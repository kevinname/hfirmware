/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio
              Copyright (C) 2015 Huntersun Technologies
              wei.lu@huntersun.com.cn

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <string.h>
#include "bt_os.h"    //os-related
#include "bt_config.h"

#define RD_TIMEOUT MS2ST(1000)
#define MAX_PKT_SIZE 1024

static SerialDriver *hci_device;
static uint8_t *s_RxBuffer;
static thread_t *thd_bt_dongle;

extern SerialUSBDriver SDU1;

void vhci_h4_hc2pc_packet(uint8_t *data, uint32_t pdu_len, uint8_t *header, uint8_t head_len)
{
  unsigned char packet_type = *header;

  sdWrite(hci_device, header, head_len);
  sdWrite(hci_device,   data,  pdu_len);
  HCI_Generic_Acknowledge_Complete_Tx(packet_type, pdu_len);
}

static THD_FUNCTION(vhci_h4_pc2hc_process, arg)
{
  (void)arg;
  uint16_t pdu_len;
  uint8_t *pdu_buf, *head_buf;
  uint8_t packet_type;
  uint32_t tot = 0;
  uint32_t res;
  uint32_t pdu_length, hdr_len = 1;

  while (1) {
    /*Get the first byte, because the first byte indicate the packet type.*/
    do {
      res = sdReadTimeout(hci_device, &s_RxBuffer[0], 1, RD_TIMEOUT);
      if (res <= 0) {
        osDelay(10);
        /* try USB serial */
        if ((&SDU1 != (SerialUSBDriver *)hci_device) && sduIsReady(&USBD1))
          hci_device = (SerialDriver *)(&SDU1);
      }
    } while (res <= 0);

    packet_type = s_RxBuffer[0];
    switch (packet_type) {
    case HCI_TLPKT_COMMAND:
      pdu_length = 4;
      break;
    case HCI_TLPKT_ACLDATA:
      pdu_length = 5;
      break;
    case HCI_TLPKT_SCODATA:
      pdu_length = 4;
      break;
    case HCI_TLPKT_EVENT:
      pdu_length = 3;
      break;
    default:
      /*Get unknown first byte, discard it*/
      pdu_length = 0;
      continue;
    }

    tot = 1;
    /*Get the header of packet, as we got the header, so the following will coms very soon, so we do not sleep here*/
    do {
      res = sdReadTimeout(hci_device, &s_RxBuffer[tot], pdu_length-tot, RD_TIMEOUT);
      if (res > 0) {
        tot += res;
      }
    } while (tot < pdu_length);

    /*get the length of the total packet*/
    switch (s_RxBuffer[0]) {
    case HCI_TLPKT_COMMAND:
      /* it is required by HC's command queue in HC: HCI_Generic_Get_Rx_Buf() includes HCI command's header, but pdu doesn't */
      hdr_len = 3;
      pdu_len = s_RxBuffer[3];
      pdu_length += pdu_len;
      break;
    case HCI_TLPKT_ACLDATA:
      hdr_len = 0;
      pdu_len = s_RxBuffer[3] + (s_RxBuffer[4] << 8);
      pdu_length += pdu_len;
      break;
    case HCI_TLPKT_SCODATA:
      hdr_len = 0;
      pdu_len = s_RxBuffer[3];
      pdu_length += pdu_len;
      break;
    case HCI_TLPKT_EVENT:
      hdr_len = 0;
      pdu_len = s_RxBuffer[2];
      pdu_length += pdu_len;
      break;
    default:
      /*can not be here*/
      pdu_length = 0;
      continue;
    }

    /* avoid buffer overflow by corrupted packet */
    if ((pdu_length - tot) > MAX_PKT_SIZE)
      continue;

    /*get the total packet*/
    while (pdu_length - tot > 0) {
      res = sdReadTimeout(hci_device, &s_RxBuffer[tot], pdu_length - tot, RD_TIMEOUT);
      if (res > 0) {
        tot += res;
      }
      if (tot < pdu_length) {
        /*we just read part of packet, so sleep a short time*/
        osDelay(5);
      } else {
        /*read full packet*/
        break;
      }
    }

    head_buf = &s_RxBuffer[1];
    /* it is required by HC's command queue in HC: HCI_Generic_Get_Rx_Buf() includes HCI command's header, but pdu doesn't */
    pdu_buf = (uint8_t *)HCI_Generic_Get_Rx_Buf(packet_type, pdu_len + hdr_len, head_buf);
    if (NULL == pdu_buf)
      continue;
    if (pdu_len > 0)
      memcpy(pdu_buf, &s_RxBuffer[pdu_length - pdu_len], pdu_len);
    HCI_Generic_Commit_Rx_Buf(packet_type);
  }
}

void bt_usb_start(uint8_t dev)
{
  s_RxBuffer = chHeapAlloc(NULL, MAX_PKT_SIZE);
  if (NULL == s_RxBuffer)
    return;

  hci_device = &SD0;
  if (dev) {
    osDelay(5000);
    if (sduIsReady(&USBD1))
        hci_device = (SerialDriver *)(&SDU1);
  }

  HCI_Generic_Register_Tx_Callback(vhci_h4_hc2pc_packet);

  thd_bt_dongle = chThdCreateFromHeap(NULL, 512,
                                      NORMALPRIO,
                                      vhci_h4_pc2hc_process,
                                      NULL);
}

void bt_usb_stop(void)
{
  if(!s_RxBuffer)
    return ;

  oshalThreadTerminate(thd_bt_dongle);

  chHeapFree(s_RxBuffer);
  s_RxBuffer = NULL;
}


