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
#include "bthc_uapi.h"

#if 1
//#include "datatype.h"
#ifndef HANDLE
#define HANDLE void *
#endif
#ifndef UINT8
#define UINT8   unsigned char
#endif
#ifndef UINT16
#define UINT16  unsigned short
#endif
#ifndef UINT32
#define UINT32	unsigned long
#endif

/* https://www.bluetooth.org/Technical/AssignedNumbers/A2MP.htm, Was named
   Controller type */
typedef enum _tHCI_Device_Type {
	HCI_DEVICE_TYPE_BREDR =                                                  0x00,
	HCI_DEVICE_TYPE_WIFI =                                                   0x01,
	HCI_DEVICE_TYPE_UWB =                                                    0x02,
	HCI_DEVICE_TYPE_UDP_FAKE_PAL =                                           0xEF,
	HCI_DEVICE_TYPE_IVT_FAKE_PAL =                                           0xF0,
	HCI_DEVICE_TYPE_UNKNOWN =                                                0xFF
} tHCI_Device_Type;

/*---------------------------------------------------------------------------*/
/* 						HCI Transport Layer Interface		       	  	     */
/*---------------------------------------------------------------------------*/
typedef int (Func_Receive) 				(HANDLE host_hdl, UINT8 *hdr, UINT8 hdr_len, UINT8 *pdu, UINT32 pdu_len);

typedef struct _TL_CallbackStru
{
	Func_Receive *Receive;	
} TL_CallbackStru;

typedef int (Func_Startup) 				(HANDLE tl_hdl, TL_CallbackStru *pCallback, HANDLE host_hdl);
typedef void (Func_Shutdown) 			(HANDLE tl_hdl);
typedef int (Func_Send) 				(HANDLE tl_hdl, UINT8 *pBuf, UINT32 len);
typedef int (Func_StartSco) 			(HANDLE tl_hdl, UINT16 sco_hdl);
typedef void (Func_StopSco) 			(HANDLE tl_hdl, UINT16 sco_hdl);
typedef unsigned char (Func_GetType) 	(HANDLE tl_hdl);

typedef struct _TransportLayerStru
{
	Func_GetType *GetType;
	Func_Startup *Startup;
	Func_Shutdown *Shutdown;
	Func_Send *Send;
	Func_StartSco *StartSco;
	Func_StopSco *StopSco;
} TransportLayerStru;

extern void App_GAP_RegisterTL(HANDLE tl_handle, TransportLayerStru *func);
extern void App_GAP_UnRegisterTL(HANDLE tl_handle);
#endif

struct AppTLInstStru {
  HANDLE		 host_hdl;
  Func_Receive *rcv_func;
};

static TransportLayerStru tl_funcs;
static struct AppTLInstStru s_hTLInst;

/* a simple Lost Package Concealment */
static bt_host_config_t *p_host_cfg;
static bt_host_status_t *p_host_sts;

__ONCHIP_VHCI__ void vhci_h4_hc2host_packet(uint8_t *data, uint32_t pdu_len, uint8_t *header, uint8_t head_len)
{
  unsigned char packet_type = *header;
  if (s_hTLInst.rcv_func) {
    /* data doesn't contain header */
    s_hTLInst.rcv_func(s_hTLInst.host_hdl, header, head_len, data, pdu_len);
  }
  //debug("h4 host<-hc: rx %02x in %d+%d\n", packet_type, head_len, pdu_len);
  printf_packet(packet_type, 1/*in*/, header+1, data, pdu_len);
  
#if HS_USE_PRODUCT_TEST
  if (*header == HCI_TLPKT_EVENT) {
    if ((*data == 0x02 /*HCI_INQUIRY_RESULT_EVENT*/) ||
        (*data == 0x22 /*HCI_INQUIRY_RESULT_EVENT_WITH_RSSI*/) ||
        (*data == 0x2F /*HCI_EXTENDED_INQUIRY_RESULT_EVENT*/))
      p_host_sts->gap_inquiry_result_cnt++;
  }
#endif

  HCI_Generic_Acknowledge_Complete_Tx(packet_type, pdu_len);
}

int AppTL_GetType(HANDLE tl_hdl)
{
  (void)tl_hdl;
  return HCI_DEVICE_TYPE_BREDR;
}

int AppTL_Startup(HANDLE tl_hdl, TL_CallbackStru *cb_func, HANDLE host_hdl)
{
  (void)tl_hdl;
  s_hTLInst.host_hdl = host_hdl;
  s_hTLInst.rcv_func = cb_func->Receive;
  return 0;
}

void AppTL_Shutdown(HANDLE tl_hdl)
{
  (void)tl_hdl;
  s_hTLInst.host_hdl = NULL;
  s_hTLInst.rcv_func = NULL;
}

/* int vhci_h4_host2hc_packet(uint8_t packet_type, uint8_t *packet, int size) */
__ONCHIP_VHCI__ int AppTL_Send(HANDLE tl_hdl, UINT8 *packet, UINT32 size)
{
  (void)tl_hdl;
  uint8_t hdr_len;
  uint16_t pdu_len;
  uint8_t *pdu_buf, *head_buf;
  uint8_t packet_type = *packet++;

  head_buf = packet;
  switch (packet_type) {
  case HCI_TLPKT_COMMAND:
    packet += 2; //skip cmd_opcode in 2B
    pdu_len = *packet++; //skip cmd_param_len in 1B
    /* it is required by HC's command queue in HC: pdu_len includes HCI command's header, but pdu doesn't */
    hdr_len = 3; //cmd_hdr: opcode + param_len
    break;
  case HCI_TLPKT_ACLDATA:
    packet += 2; //skip acl_handler in 2B
    pdu_len = *packet++;
    pdu_len |= (*packet++ << 8); //skip acl_data_len in 2B
    hdr_len = 0; //4 acl_hdr: handle + data_len
    break;
  case HCI_TLPKT_SCODATA:
    packet += 2; //skip sco_handler in 2B
    pdu_len = *packet++; //skip sco_data_len in 1B
    hdr_len = 0; //3 sco_hdr: handle + data_len
    break;
  default:
    /*Get unknown first byte, discard it*/
    pdu_len = 0;
    debug("h4 host->hc: unknown packet_type %d\n", packet_type);
    return 0;
  }

  p_host_sts->vhci_host2hc_busy_cnt = 0;
  while (1) {
    /* it is required by HC's command queue in HC: HCI_Generic_Get_Rx_Buf() includes HCI command's header, but pdu doesn't */
    pdu_buf = (uint8_t *)HCI_Generic_Get_Rx_Buf(packet_type, pdu_len + hdr_len, head_buf);
	
    if (NULL == pdu_buf) {
      if (HCI_TLPKT_SCODATA == packet_type) {
        p_host_sts->lpc_sco_pkt_tx_miss_cnt++;
        /* discard SCO packet if there is no room in HC or SCO is disconnected */
        return 0;
      }
      debug("h4 host->hc: wait hc ... type=%d len=%d\n", packet_type, pdu_len);
      osDelay(5);
      p_host_sts->vhci_host2hc_busy_cnt++;
      if (p_host_sts->vhci_host2hc_busy_cnt > 20) {
        p_host_sts->vhci_host2hc_busy_cnt = 0;
        /* discard Command or ACL packet if busy is too long */
        return 0;
      }
    } else {
      break;
    }
  };

  if (pdu_len > 0)
    memcpy(pdu_buf, packet, pdu_len);
  //debug("h4 host->hc: tx %02x in %d\n", packet_type, size);
  printf_packet(packet_type, 0/*in*/, head_buf, pdu_buf, pdu_len);
  HCI_Generic_Commit_Rx_Buf(packet_type);

  return size;
}

int AppTL_StartSco(HANDLE tl_hdl, UINT16 sco_hdl)
{
  (void)tl_hdl;
  (void)sco_hdl;
  return 0;
}

void AppTL_StopSco(HANDLE tl_hdl, UINT16 sco_hdl)
{
  (void)tl_hdl;
  (void)sco_hdl;
}

void InitTransportLayer(void)
{
  bt_host_get_config(&p_host_cfg);
  bt_host_get_status(&p_host_sts);

  /* Register TL to the Stack */
  memset(&s_hTLInst, 0, sizeof(struct AppTLInstStru));
  tl_funcs.GetType = (Func_GetType *)AppTL_GetType;
  tl_funcs.Startup = AppTL_Startup;
  tl_funcs.Shutdown = AppTL_Shutdown;
  tl_funcs.Send = (Func_Send *)AppTL_Send;
  tl_funcs.StartSco = AppTL_StartSco;
  tl_funcs.StopSco = AppTL_StopSco;
  App_GAP_RegisterTL((HANDLE)&s_hTLInst, &tl_funcs);
}

void FreeTransportLayer(void)
{
    App_GAP_UnRegisterTL((HANDLE)&s_hTLInst);
    memset(&s_hTLInst, 0, sizeof(struct AppTLInstStru));
    memset(&tl_funcs, 0, sizeof(TransportLayerStru));
    
}
