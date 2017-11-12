/*
 ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

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
/*
 PengJiang, 20140910
 */

#include <string.h>

#include "ch.h"
#include "hal.h"
#include "lib.h"

#if HAL_USE_USB_HOST_STORAGE

#include "usb_lld.h"
#include "usb_h_lld.h"


USBHostDriver USBHOST0;

#define USB_H_DBG 0

#if USB_H_DBG
#include <stdio.h>
extern SerialDriver SD1;
//#define USBH_PRINTF(...)  chprintf((BaseSequentialStream *)&SD1, __VA_ARGS__)
#define USBH_PRINTF(fmt,args...) do { hs_printf("[%06u] ", osKernelSysTick()); hs_printf(fmt, ##args); } while (0)
inline void usb_h_dbg_printdata(uint8_t* buf, uint32_t len)
{
  uint32_t i;
  for(i=0;i<len;i++) {
    hs_printf("%02x ",buf[i]);
    if((i%16)==15)
    {
      hs_printf("\n\r");
    }
  }
}
#else
#define USBH_PRINTF(fmt,args...)
inline void usb_h_dbg_printdata(uint8_t* buf, uint32_t len)
{
  (void)buf;
  (void)len;
}
#endif

static const uint8_t usb_cmd_setup_data[][8] =
{
  { 0x80, 0x06, 0x00, 0x01, 0x00, 0x00, 0x12, 0x00}, //GET_DEV_DESC
  { 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, //SET_DEV_ADDR
  { 0x80, 0x06, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00}, //GET_CFG_DESC
  { 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, //SET_CFG
  { 0xA1, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00}, //GET_MAXLUN
  { 0x01, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // SET_INF
};

static THD_FUNCTION(UsbHostThread, arg);

void usbhostInit(void)
{
  USBHOST0.usbp = &USBD1;
  USBHOST0.hs_usb = HS_USB;
  USBHOST0.magicNum = 0x12345678;
  USBHOST0.usbenum_thread = NULL;
  USBHOST0.usbinep_thread = NULL;
  USBHOST0.usboutep_thread = NULL;

  osalMutexObjectInit(&USBHOST0.mutex);
}

static void usb_h_lld_parameters_init(USBHostDriver *usbhp)
{
  usbhp->inEpNum = 0;
  usbhp->outEpNum = 0;
  usbhp->inEpPktSiz = 0;
  usbhp->outEpPktSiz = 0;
  usbhp->maxlun = 0;
  hsusb_ep_select(0);
  USBHREG->USB_TXFIFO = 0x6000;
  USBHREG->USB_RXFIFO = 0x6000;
  USBHREG->USB_INTTXEN = 0x0000;
  USBHREG->USB_INTRXEN = 0x0000;
  hsusb_ep_int_enable(usbhp->usbp, 0, TRUE);
  USBHREG->USB_ADDR = 0x00;
}

void usb_h_lld_start(USBHostDriver *usbhp)
{
  usbhp->host_state = USB_H_STOP;

  chMBObjectInit(&usbhp->mbox, usbhp->message, USB_H_MAILBOX_SIZE);

  usbhp->usbhost_thread = chThdCreateFromHeap(NULL, 1024, NORMALPRIO, UsbHostThread, (void*)&USBHOST0);
  osalDbgAssert(usbhp->usbhost_thread != NULL, "chThdCreateFromHeap failed!");
  chRegSetThreadNameX(usbhp->usbhost_thread, "usbhost_thread");

  nvicDisableVector(IRQ_USBHOST);
  cpmDisableUSB();
  cpmResetUSB();
  cpmEnableUSB();
  usb_lld_enable_int();

  //usb_lld_enable_dma_int();
  USBHREG->USB_INTUSBEN = 0xf7;
  usb_lld_connect_bus(usbhp->usbp);
  USBHREG->USB_DEVCTL |= HSUSB_DEVCTL_SESSION; //start a session
  usbhp->host_state = USB_H_POWERED;
}

void usb_h_lld_stop(USBHostDriver *usbhp)
{
  usbhp->host_state = USB_H_STOP;
  
  nvicDisableVector(IRQ_USBHOST);
  //nvicDisableVector(IRQ_USB_DMA);

  //usb_lld_disconnect_bus(usbhp->usbp);
  //cpmDisableUSB();
  if (usbhp->usbenum_thread != NULL) {
    chSysLock();
    chSchReadyI(usbhp->usbenum_thread);
    chSysUnlock();
  }
  if (usbhp->usboutep_thread != NULL) {
    chSysLock();
    chSchReadyI(usbhp->usboutep_thread);
    chSysUnlock();
  }
  if (usbhp->usbinep_thread != NULL) {
    chSysLock();
    chSchReadyI(usbhp->usbinep_thread);
    chSysUnlock();
  }
  if (usbhp->usbenum_thread || usbhp->usboutep_thread || usbhp->usbinep_thread) {
    usbhp->usbenum_thread = usbhp->usboutep_thread = usbhp->usbinep_thread = NULL;
  }

  chMBPost(&usbhp->mbox, HSUSB_INTR_RELEASE, TIME_INFINITE);
  chThdWait(usbhp->usbhost_thread);  
}

uint8_t get_bulk_eps_info(USBHostDriver *usbhp, uint8_t* desc, uint16_t len)
{
  uint32_t pos = 0;
  uint8_t blk_len = 0;
  while(pos<len)
  {
    blk_len = desc[pos];
    if(desc[pos+1] == USB_DESCRIPTOR_INTERFACE
        &&desc[pos+4] == 2 // 2 bulk endpoints
        &&desc[pos+5] == MASS_STORAGE_CLASS_CODE
        &&desc[pos+6] == MASS_STORAGE_SUBCLASS_SCSI
        &&desc[pos+7] == MASS_STORAGE_IFPROTOCAL_BULK
    )
    {
      uint8_t i = 0;
      for(i=0;i<2;i++)
      {
        pos+=blk_len;
        if(desc[pos+1] != USB_DESCRIPTOR_ENDPOINT)
        {
          return FALSE;
        }
        if(desc[pos+2]&0x80)
        {
          usbhp->inEpNum = desc[pos+2]&0x0f;
          usbhp->inEpPktSiz = desc[pos+4]+(desc[pos+5]<<8);
        }
        else
        {
          usbhp->outEpNum = desc[pos+2]&0x0f;
          usbhp->outEpPktSiz = desc[pos+4]+(desc[pos+5]<<8);
        }
        blk_len = desc[pos];
      }
      if(usbhp->inEpPktSiz != 64||usbhp->outEpPktSiz != 64)
      {
        return FALSE;
      }
      else
      {
        return TRUE;
      }

    }
    else
    {
      pos+=blk_len;
      continue;
    }
  }
  return FALSE;
}

static bool_t usb_h_lld_unmount_fs()
{
#if HAL_USE_FATFS
  if (f_mount(NULL, _T("1:"), 0) != FR_OK) {
    return FALSE;
  }
  else {
    return TRUE;
  }
#else
  return FALSE;
#endif
}

//on_off: TRUE, trigger reset signal; FALSE, end reset signal
void usb_h_lld_send_reset_signal(USBHostDriver *usbhp)
{
  (void)usbhp;
  /* assert HSUSB_POWER_RESET bit in HSUSB_INTR_CONNECT's ISR */
  USBHREG->USB_POWER |= HSUSB_POWER_RESET;
  chThdSleepMilliseconds(200);
  USBHREG->USB_POWER &= ~HSUSB_POWER_RESET;
  chThdSleepMilliseconds(200);
}

static uint8_t usb_h_lld_send_usb_setup_pkt(USBHostDriver *usbhp)
{
  uint16_t csr0 = 0;
  chSysLock();
  hsusb_ep_select(0);
  usb_packet_write_from_buffer(usbhp->usbp, 0, usbhp->usbp->setup, 8);
  csr0 = USBHREG->USB_CSR0;
  csr0 |= (HSUSB_CSR0_H_SETUPPKT|HSUSB_CSR0_TXPKTRDY);
  USBHREG->USB_CSR0 = csr0;
  usbhp->usbenum_thread = chThdGetSelfX();
  chSchGoSleepS(CH_STATE_SUSPENDED);
  chSysUnlock();
  csr0 = USBHREG->USB_CSR0;

  if ((USB_H_DISCONNECTED == usbhp->host_state) ||
      (USB_H_STOP == usbhp->host_state)) {
    return FALSE;
  }

  USBHREG->USB_CSR0 = 0;
  if(csr0&(HSUSB_CSR0_H_ERROR|HSUSB_CSR0_H_RXSTALL|HSUSB_CSR0_H_NAKTIMEOUT))
  {
    return FALSE;
  }
  return TRUE;

}

static uint8_t usb_h_lld_send_usb_getcmd(USBHostDriver *usbhp, uint8_t* buf, uint16_t len)
{
  uint16_t csr0 = 0;

  if(FALSE == usb_h_lld_send_usb_setup_pkt(usbhp))
  {
    return FALSE;
  }

  if(len)
  {
    uint16_t byteleft = len;
    while(byteleft)
    {
      uint16_t pktsize = byteleft>USB_CTRL_EP_PKT_SIZ?USB_CTRL_EP_PKT_SIZ:byteleft;
      chSysLock();
      csr0 = HSUSB_CSR0_H_REQPKT;
      USBHREG->USB_CSR0 = csr0;
      usbhp->usbenum_thread = chThdGetSelfX();
      chSchGoSleepS(CH_STATE_SUSPENDED);
      chSysUnlock();

      if ((USB_H_DISCONNECTED == usbhp->host_state) ||
          (USB_H_STOP == usbhp->host_state)) {
        return FALSE;
      }

      usb_packet_read_to_buffer(usbhp->usbp, 0, &buf[len-byteleft], pktsize);
      byteleft-=pktsize;
      csr0 = USBHREG->USB_CSR0;
      if(csr0&(HSUSB_CSR0_H_ERROR|HSUSB_CSR0_H_RXSTALL|HSUSB_CSR0_H_NAKTIMEOUT))
      {
        return FALSE;
      }
      USBHREG->USB_CSR0 = 0;
    }
  }

  chSysLock();
  csr0 = USBHREG->USB_CSR0;
  csr0 |= (HSUSB_CSR0_H_STATUSPKT|HSUSB_CSR0_TXPKTRDY);
  USBHREG->USB_CSR0 = csr0;
  usbhp->usbenum_thread = chThdGetSelfX();
  chSchGoSleepS(CH_STATE_SUSPENDED);
  chSysUnlock();

  if ((USB_H_DISCONNECTED == usbhp->host_state) ||
      (USB_H_STOP == usbhp->host_state)) {
    return FALSE;
  }

  csr0 = USBHREG->USB_CSR0;
  if(csr0&(HSUSB_CSR0_H_ERROR|HSUSB_CSR0_H_RXSTALL|HSUSB_CSR0_H_NAKTIMEOUT))
  {
    return FALSE;
  }
  USBHREG->USB_CSR0 = 0;
  return TRUE;
}

static uint8_t usb_h_lld_send_usb_setcmd(USBHostDriver *usbhp, uint8_t* buf, uint16_t len)
{
  uint16_t csr0 = 0;

  if(FALSE == usb_h_lld_send_usb_setup_pkt(usbhp))
  {
    return FALSE;
  }

  if(len)
  {
    uint16_t byteleft = len;
    while(byteleft)
    {
      uint8_t pktlen = (byteleft>=USB_CTRL_EP_PKT_SIZ)?USB_CTRL_EP_PKT_SIZ:byteleft;
      usb_packet_write_from_buffer(usbhp->usbp, 0, &buf[len-byteleft], pktlen);
      chSysLock();
      USBHREG->USB_CSR0 = HSUSB_CSR0_TXPKTRDY;
      usbhp->usbenum_thread = chThdGetSelfX();
      chSchGoSleepS(CH_STATE_SUSPENDED);
      chSysUnlock();

      if ((USB_H_DISCONNECTED == usbhp->host_state) ||
          (USB_H_STOP == usbhp->host_state)) {
        return FALSE;
      }

      byteleft-=pktlen;
      csr0 = USBHREG->USB_CSR0;
      USBHREG->USB_CSR0 = 0;
      if(csr0&(HSUSB_CSR0_H_ERROR|HSUSB_CSR0_H_RXSTALL|HSUSB_CSR0_H_NAKTIMEOUT))
      {
        return FALSE;
      }
    }
  }
  chSysLock();
  csr0 = (HSUSB_CSR0_H_STATUSPKT|HSUSB_CSR0_H_REQPKT);
  USBHREG->USB_CSR0 = csr0;
  usbhp->usbenum_thread = chThdGetSelfX();
  chSchGoSleepS(CH_STATE_SUSPENDED);
  chSysUnlock();

  if ((USB_H_DISCONNECTED == usbhp->host_state) ||
      (USB_H_STOP == usbhp->host_state)) {
    return FALSE;
  }

  csr0 = USBHREG->USB_CSR0;
  USBHREG->USB_CSR0 = 0;
  if(csr0&(HSUSB_CSR0_H_ERROR|HSUSB_CSR0_H_RXSTALL|HSUSB_CSR0_H_NAKTIMEOUT))
  {
    return FALSE;
  }
  return TRUE;
}

static uint8_t usb_h_ldd_get_dev_desc(USBHostDriver *usbhp, uint8_t* buf)
{
  memcpy(usbhp->usbp->setup, usb_cmd_setup_data[GET_DEV_DESC], 8);
  return usb_h_lld_send_usb_getcmd(usbhp, buf, 18);
}

static uint8_t usb_h_ldd_set_dev_addr(USBHostDriver *usbhp, uint8_t addr)
{
  memcpy(usbhp->usbp->setup, usb_cmd_setup_data[SET_DEV_ADDR], 8);
  usbhp->usbp->setup[2] = addr;
  return usb_h_lld_send_usb_setcmd(usbhp, NULL, 0);
}

static uint8_t usb_h_ldd_get_config_desc(USBHostDriver *usbhp, uint8_t* buf, uint16_t len)
{
  memcpy(usbhp->usbp->setup, usb_cmd_setup_data[GET_CFG_DESC], 6);
  usbhp->usbp->setup[6] = (len&0x00ff);
  usbhp->usbp->setup[7] = (len&0xff00)>>8;
  return usb_h_lld_send_usb_getcmd(usbhp, buf, len);
}

static uint8_t usb_h_ldd_set_config(USBHostDriver *usbhp, uint8_t cfg)
{
  memcpy(usbhp->usbp->setup, usb_cmd_setup_data[SET_CFG], 8);
  usbhp->usbp->setup[2] = cfg;
  return usb_h_lld_send_usb_setcmd(usbhp, NULL, 0);
}

static uint8_t usb_h_ldd_set_interface(USBHostDriver *usbhp, uint8_t altsetting)
{
  memcpy(usbhp->usbp->setup, usb_cmd_setup_data[SET_INF], 8);
  usbhp->usbp->setup[3] = altsetting;
  return usb_h_lld_send_usb_setcmd(usbhp, NULL, 0);
}

static uint8_t usb_h_ldd_get_max_lun(USBHostDriver *usbhp, uint8_t* buf)
{
  memcpy(usbhp->usbp->setup, usb_cmd_setup_data[GET_MAXLUN], 8);
  return usb_h_lld_send_usb_getcmd(usbhp, buf, 1);
}

static uint8_t usb_h_ldd_set_bulk_xfer_parameters(USBHostDriver *usbhp)
{
  USBHREG->USB_INTTXEN |= 0x0001<<usbhp->outEpNum;
  USBHREG->USB_INTRXEN |= 0x0001<<usbhp->inEpNum;
  hsusb_ep_select(usbhp->inEpNum);
  USBHREG->USB_RXFIFO = 0x6008+(usbhp->inEpPktSiz>>3);
  USBHREG->USB_RXMAXP = usbhp->inEpPktSiz>>3;
  USBHREG->USB_RXCSR = HSUSB_RXCSR_CLRDATATOG;
  hsusb_ep_select(usbhp->outEpNum);
  USBHREG->USB_TXFIFO = 0x6008+(usbhp->inEpPktSiz>>3)+(usbhp->outEpPktSiz>>3);
  USBHREG->USB_TXMAXP = usbhp->outEpPktSiz>>3;
  USBHREG->USB_TXCSR = HSUSB_TXCSR_CLRDATATOG;

  usbhp->cbw.Signature = BULK_CBW_SIGNITURE;

  return TRUE;
}

void hs66xx_usb_host_handle_connect(USBHostDriver *usbhp)
{
  uint8_t databuf[256];
  uint16_t datalen = 0;
  int i;

  usb_lld_disable_pmu_charger();

  //chSysLock();
  usbhp->host_state = USB_H_CONNECTED;
  //chSysUnlock();

  usb_h_lld_send_reset_signal(usbhp);//reset device

  /* workaround for some udisks which will Disconnect once during enumeration */
  for (i = 0; i < 100; i++) {
    uint8_t int_usb = HS_USB->USB_INTUSB;
    if (int_usb & HSUSB_INTR_DISCONNECT) {
      USBH_PRINTF("DevCtl=0x%x, IntrUSB=0x%x\n\r", USBHREG->USB_DEVCTL, int_usb);

      usb_lld_disconnect_bus(usbhp->usbp);
      chThdSleepMilliseconds(100);
      usb_lld_connect_bus(usbhp->usbp);

      while (1) {
        /* wait udisk's firmware restart for Connect again */
        int_usb = HS_USB->USB_INTUSB;
        //USBH_PRINTF("DevCtl=0x%x, IntrUSB=0x%x\n\r", USBHREG->USB_DEVCTL, int_usb);
        if (int_usb & HSUSB_INTR_CONNECT) {
          break;
        }
        chThdSleepMilliseconds(1);
      }
      /* SW generates Reset signaling after Connect again */
      usb_h_lld_send_reset_signal(usbhp);

      /* this delay is required by SAGET udisk */
      chThdSleepMilliseconds(500);
      break;
    }
    chThdSleepMilliseconds(1);
  }
  for (i = 0; i < 100; i++) {
    if ((USBHREG->USB_DEVCTL & (HSUSB_DEVCTL_FSDEV | HSUSB_DEVCTL_LSDEV)) != 0)
      break;
    chThdSleepMilliseconds(1);
  }
  NVIC_EnableIRQ(IRQ_USBHOST);
  if (i == 100) {
    USBH_PRINTF("speed neg fail\n\r");
    goto unsupported_dev;
  }

  //enum device and check if it is usb storage devce(bulk only device)
  //memset(databuf, 0xAA, 256);//test
  usb_h_lld_parameters_init(usbhp);

  if(!usb_h_ldd_get_dev_desc(usbhp, databuf))
  {
    USBH_PRINTF("get dev desc fail!\n\r");
    goto unsupported_dev;
  }
  USBH_PRINTF("Device Descriptor:\n\r");
  usb_h_dbg_printdata(databuf, 18);
  USBH_PRINTF("\n\r");

  if(!usb_h_ldd_set_dev_addr(usbhp, DEFAULT_DEV_ADDR))
  {
    USBH_PRINTF("set dev desc fail!\n\r");
    goto unsupported_dev;
  }
  USBHREG->USB_ADDR = DEFAULT_DEV_ADDR;
  //usbhp->host_state = USB_H_ADDRESSED;
  USBH_PRINTF("Device Address: %02x\n\r",DEFAULT_DEV_ADDR);
  //memset(databuf, 0xAA, 256);
  if(!usb_h_ldd_get_dev_desc(usbhp, databuf))
  {
    goto unsupported_dev;
  }
  //memset(databuf, 0xAA, 256);
  if(!usb_h_ldd_get_config_desc(usbhp, databuf, 8))
  {
    goto unsupported_dev;
  }
  datalen = databuf[2]+(databuf[3]<<8);
  //memset(databuf, 0xAA, 256);
  if(!usb_h_ldd_get_config_desc(usbhp, databuf, datalen))
  {
    goto unsupported_dev;
  }
  if(FALSE == get_bulk_eps_info(usbhp,databuf,datalen))
  {
    goto unsupported_dev;
  }
  USBH_PRINTF("Configuration Descriptor:\n\r");
  usb_h_dbg_printdata(databuf, datalen);
  USBH_PRINTF("\n\r");
  USBH_PRINTF("Device Info:\n\r");
  USBH_PRINTF("BulkIn  Ep=%d, PktSiz=%02x\n\r",usbhp->inEpNum,usbhp->inEpPktSiz);
  USBH_PRINTF("BulkOut Ep=%d, PktSiz=%02x\n\r",usbhp->outEpNum,usbhp->outEpPktSiz);
  if(!usb_h_ldd_set_config(usbhp, 1))
  {
    USBH_PRINTF("set_configuration failed\r\n");
    goto unsupported_dev;
  }

  if(!usb_h_ldd_set_interface(usbhp, 0))
  {
    USBH_PRINTF("set_interface failed\r\n");
    //goto unsupported_dev;
  }

  if (!usb_h_ldd_get_max_lun(usbhp, databuf)) {
    /* plugout during udisk enumeration */
    goto unsupported_dev;
  }
  usbhp->maxlun = databuf[0];
  USBH_PRINTF("max lun=%02x\n\r",usbhp->maxlun);

  usb_h_ldd_set_bulk_xfer_parameters(usbhp);

  if (!usb_h_lld_scsi_inquiry(usbhp)) {
    /* plugout during udisk enumeration */
    goto unsupported_dev;
  }

  for (i = 0; i < 10; i++) {
    chThdSleepMilliseconds(20);
    if (usb_h_lld_scsi_testunitready(usbhp))
      break;
  }
  if (i == 10)
    goto unsupported_dev;

  if (!usb_h_lld_scsi_readcapacity(usbhp)) {
    /* TF/SD card read has no media */
    goto unsupported_dev;
  }
  //usb_h_lld_mount_fs(usbhp);
  usbhp->host_state = USB_H_READY;

  return;

  unsupported_dev:
  USBH_PRINTF("Device NOT supported!\n\r");
  usbhp->host_state = USB_H_UNKNOWNDEV;
  return;

}

void hs66xx_usb_host_handle_disconnect(USBHostDriver *usbhp)
{
  USBH_PRINTF("Device disconnected!\n\r");
  usbhp->host_state = USB_H_DISCONNECTED; 
  if (usbhp->usbenum_thread != NULL) {
    chSysLock();
    chSchReadyI(usbhp->usbenum_thread);
    chSysUnlock();
  }
  if (usbhp->usboutep_thread != NULL) {
    chSysLock();
    chSchReadyI(usbhp->usboutep_thread);
    chSysUnlock();
  }
  if (usbhp->usbinep_thread != NULL) {
    chSysLock();
    chSchReadyI(usbhp->usbinep_thread);
    chSysUnlock();
  }
  if (usbhp->usbenum_thread || usbhp->usboutep_thread || usbhp->usbinep_thread) {
    usbhp->usbenum_thread = usbhp->usboutep_thread = usbhp->usbinep_thread = NULL;
    osDelay(10);
  }

  usb_lld_enable_pmu_charger();

  g_UsbMode = USB_MODE_UNKNOWN;
  usb_h_lld_parameters_init(usbhp);
  usb_h_lld_unmount_fs();

  //usb_lld_disconnect_bus();
}

void hs66xx_usb_host_handle_error(USBHostDriver *usbhp)
{
  usbhp->host_state = USB_H_UNKNOWNDEV;
}

void hs66xx_usb_host_handle_bulk_epout(USBHostDriver *usbhp)
{
  (void)usbhp;
  if (usbhp->usboutep_thread != NULL)
  {
    chSchReadyI(usbhp->usboutep_thread);
    usbhp->usboutep_thread = NULL;
  }
}

void hs66xx_usb_host_handle_bulk_epin(USBHostDriver *usbhp)
{
  (void)usbhp;
  if (usbhp->usbinep_thread != NULL)
  {
    chSchReadyI(usbhp->usbinep_thread);
    usbhp->usbinep_thread = NULL;
  }

}

void hs66xx_usb_host_handle_ep0(USBHostDriver *usbhp)
{
  (void)usbhp;
  if (usbhp->usbenum_thread != NULL) {
    chSchReadyI(usbhp->usbenum_thread);
    usbhp->usbenum_thread = NULL;
  }
}

void hs66xx_usb_host_irq(uint8_t usb, uint16_t tx, uint16_t rx)
{
  USBHostDriver *usbhp = &USBHOST0;
  uint16_t int_tx = tx;
  uint8_t int_usb = usb;
  uint16_t int_rx = rx;
  if(int_tx&0x0001)
  {
    hs66xx_usb_host_handle_ep0(usbhp);
  }
  if (int_tx&(0x0001<<usbhp->outEpNum))
  {
    hs66xx_usb_host_handle_bulk_epout(usbhp);
  }

  if (int_rx&(0x0001<<usbhp->inEpNum))
  {
    hs66xx_usb_host_handle_bulk_epin(usbhp);
  }

  if(int_usb)
  {
    if (int_usb & HSUSB_INTR_CONNECT) {
      /* HW should set it, and SW will clear it after 10ms~15ms */
      USBHREG->USB_POWER |= HSUSB_POWER_RESET;
      /* workaround for some udisks which will Disconnect once during enumeration */
      nvicDisableVector(IRQ_USBHOST);
    }
    chMBPostI(&usbhp->mbox, int_usb);
  }

  hs66xx_usb_host_dma_irq();
}

void hs66xx_usb_host_dma_irq(void)
{
  USBHostDriver *usbhp = &USBHOST0;
  uint8_t regval = USBDMA(0)->USB_DMA_INTR;

  if(regval&(1<<BULK_IN_DMA_CH))
  {
    if (usbhp->usbinep_thread != NULL)
    {
      chSchReadyI(usbhp->usbinep_thread);
      usbhp->usbinep_thread = NULL;
    }
  }
  if(regval&(1<<BULK_OUT_DMA_CH))
  {
    if (usbhp->usboutep_thread != NULL)
    {
      chSchReadyI(usbhp->usboutep_thread);
      usbhp->usboutep_thread = NULL;
    }
  }

}

static THD_FUNCTION(UsbHostThread, arg)
{
  USBHostDriver *usbhp = (USBHostDriver *)arg;
  msg_t msg;

  uint32_t int_usb = 0;

  while(1)
  {
    chMBFetch(&usbhp->mbox, &msg, TIME_INFINITE);

    int_usb = msg;
    USBH_PRINTF("int_usb=%x\r\n", int_usb);
    if(int_usb&HSUSB_INTR_CONNECT)
    {
      hs66xx_usb_host_handle_connect(usbhp);
      if (usbhp->host_state == USB_H_UNKNOWNDEV) {
        hs66xx_usb_host_handle_disconnect(usbhp);
        break;
      }
    }
    if(int_usb&HSUSB_INTR_DISCONNECT)
    {
      hs66xx_usb_host_handle_disconnect(usbhp);
    }
    if(int_usb&HSUSB_INTR_VBUSERROR)
    {
      hs66xx_usb_host_handle_error(usbhp);
    }
    if(int_usb&HSUSB_INTR_BABBLE)
    {
      nvicDisableVector(IRQ_USBHOST);

      usb_lld_disconnect_bus(usbhp->usbp);
      cpmDisableUSB();
      usbhp->host_state = USB_H_STOP;

      chThdSleepMilliseconds(1000);
  
      cpmResetUSB();
      cpmEnableUSB();
      usb_lld_enable_int();
      USBHREG->USB_INTUSBEN = 0xf7;
      USBHREG->USB_DEVCTL |= 0x01;
      usb_lld_connect_bus(usbhp->usbp);
      usbhp->host_state = USB_H_POWERED;
    }
    if(int_usb & HSUSB_INTR_RELEASE)
    {
      usb_lld_disconnect_bus(usbhp->usbp);
      cpmDisableUSB();
      break;
    }
  }
}

uint8_t usb_h_lld_bulk_outep(USBHostDriver *usbhp, uint8_t* buf, uint32_t len, uint8_t dma_mode)
{
  uint16_t csr = 0;
  uint32_t left = len;
  uint32_t pktsiz = 0;

  if (USB_H_STOP == usbhp->host_state) {
    goto usb_h_lld_bulk_outep_error;
  }

  hsusb_ep_select(usbhp->outEpNum);
  USBHREG->USB_TXTYPE = (0x02<<4)+usbhp->outEpNum; //bulk + out ep number
  USBHREG->USB_TXINTERVAL = 0;//never timeout
  USBHREG->USB_TXCSR = 0;

  nds32_dcache_flush();
  while(left)
  {
    hsusb_ep_select(usbhp->outEpNum);
    if (dma_mode == DMA_MODE_NONE)
    {
      pktsiz = left>usbhp->outEpPktSiz?usbhp->outEpPktSiz:left;
      usb_packet_write_from_buffer(usbhp->usbp,
          usbhp->outEpNum,
          &buf[len-left],
          pktsiz);
    }
    else if(dma_mode == DMA_MODE_0)
    {
      pktsiz = left>usbhp->outEpPktSiz?usbhp->outEpPktSiz:left;
      chSysLock();
      csr = USBHREG->USB_TXCSR;
      csr |= HSUSB_TXCSR_DMAENAB;
      csr &= ~HSUSB_TXCSR_DMAMODE;
      USBHREG->USB_TXCSR = csr;
      usb_dma_config_ch(usbhp->usbp,
          BULK_OUT_DMA_CH,
          usbhp->outEpPktSiz,
          DMA_MODE_0,
          TRUE,
          usbhp->outEpNum,
          (uint32_t)&buf[len-left],
          pktsiz);
      usbhp->usboutep_thread = chThdGetSelfX();
      chSchGoSleepS(CH_STATE_SUSPENDED);
      chSysUnlock();

      if ((USB_H_DISCONNECTED == usbhp->host_state) ||
          (USB_H_STOP == usbhp->host_state)) {
        goto usb_h_lld_bulk_outep_error;
      }

      //USBHDMAREG(BULK_OUT_DMA_CH).USB_DMA_CTL=0;
      USBDMA(BULK_OUT_DMA_CH)->USB_DMA_CTL=0;
    }
    else if(dma_mode == DMA_MODE_1)
    {
      pktsiz = len;
      chSysLock();
      csr = USBHREG->USB_TXCSR;
      csr |= HSUSB_TXCSR_DMAENAB|HSUSB_TXCSR_DMAMODE|HSUSB_TXCSR_AUTOSET;
      USBHREG->USB_TXCSR = csr;
      usb_dma_config_ch(usbhp->usbp,
          BULK_OUT_DMA_CH,
          usbhp->outEpPktSiz,
          DMA_MODE_1,
          TRUE,
          usbhp->outEpNum,
          (uint32_t)buf,
          pktsiz);
      usbhp->usboutep_thread = chThdGetSelfX();
      chSchGoSleepS(CH_STATE_SUSPENDED);
      chSysUnlock();

      if ((USB_H_DISCONNECTED == usbhp->host_state) ||
          (USB_H_STOP == usbhp->host_state)) {
        goto usb_h_lld_bulk_outep_error;
      }

      //USBHDMAREG(BULK_OUT_DMA_CH).USB_DMA_CTL=0;
      USBDMA(BULK_OUT_DMA_CH)->USB_DMA_CTL=0;
    }
    else
    {
      goto usb_h_lld_bulk_outep_error;
    }

    if(dma_mode != DMA_MODE_1)
    {
      chSysLock();
      hsusb_ep_select(usbhp->outEpNum);
      csr = USBHREG->USB_TXCSR;
      csr &= ~(HSUSB_TXCSR_AUTOSET|HSUSB_TXCSR_DMAENAB|HSUSB_TXCSR_DMAMODE);
      csr |= HSUSB_TXCSR_TXPKTRDY;

      USBHREG->USB_TXCSR = csr;
      usbhp->usboutep_thread = chThdGetSelfX();
      chSchGoSleepS(CH_STATE_SUSPENDED);
      chSysUnlock();

      if ((USB_H_DISCONNECTED == usbhp->host_state) ||
          (USB_H_STOP == usbhp->host_state)) {
        goto usb_h_lld_bulk_outep_error;
      }

      hsusb_ep_select(usbhp->outEpNum);
      csr = USBHREG->USB_TXCSR;
      if(csr&(HSUSB_TXCSR_H_RXSTALL|HSUSB_TXCSR_H_ERROR|HSUSB_TXCSR_H_NAKTIMEOUT))
      {
        goto usb_h_lld_bulk_outep_error;
      }
    }
    left-=pktsiz;
    USBHREG->USB_TXCSR = 0;
  }
  return TRUE;
  usb_h_lld_bulk_outep_error:
  USBHREG->USB_TXCSR = HSUSB_TXCSR_FLUSHFIFO;
  return FALSE;
}

uint8_t usb_h_lld_bulk_inep_dmamode1(USBHostDriver *usbhp, uint8_t* buf, uint32_t len)
{
  uint16_t csr = 0;

#if 0
  chSysLock();
  hsusb_ep_select(usbhp->inEpNum);
  USBHREG->USB_RXCSR = HSUSB_RXCSR_H_REQPKT;
  g_usbinep_thread = chThdSelf();
  chSchGoSleepS(THD_STATE_SUSPENDED);
  chSysUnlock();

  usb_packet_read_to_buffer(usbhp->usbp,
      usbhp->inEpNum,
      buf,
      64);
#endif
  chSysLock();
  hsusb_ep_select(usbhp->inEpNum);
  csr = USBHREG->USB_RXCSR;
  csr |= HSUSB_RXCSR_H_REQPKT|HSUSB_RXCSR_DMAENAB|HSUSB_RXCSR_DMAMODE|HSUSB_RXCSR_AUTOCLEAR|HSUSB_RXCSR_H_AUTOREQ;
  USBHREG->USB_RXCSR = csr;
  usb_dma_config_ch(usbhp->usbp,
      BULK_IN_DMA_CH,
      usbhp->inEpPktSiz,
      DMA_MODE_1,
      FALSE,
      usbhp->inEpNum,
      (uint32_t)&buf[0],
      len);
  usbhp->usbinep_thread = chThdGetSelfX();
  chSchGoSleepS(CH_STATE_SUSPENDED);
  chSysUnlock();

  if ((USB_H_DISCONNECTED == usbhp->host_state) ||
      (USB_H_STOP == usbhp->host_state)) {
    return FALSE;
  }

  //nds32_dcache_invalidate();
  //USBHDMAREG(BULK_IN_DMA_CH).USB_DMA_CTL=0;
  USBDMA(BULK_IN_DMA_CH)->USB_DMA_CTL=0;
  if(USBHREG->USB_RXCOUNT1 == CSW_SIZ)
  {
    usb_packet_read_to_buffer(usbhp->usbp,
        usbhp->inEpNum,
        (uint8_t*)&usbhp->csw,
        CSW_SIZ);
  }
  USBHREG->USB_RXCSR = 0;
  return TRUE;
}

uint8_t usb_h_lld_bulk_inep(USBHostDriver *usbhp, uint8_t* buf, uint32_t len, uint8_t dma_mode)
{
  uint16_t csr = 0;
  uint32_t left = len;

  if (USB_H_STOP == usbhp->host_state) {
    goto usb_h_lld_bulk_inep_error;
  }

  hsusb_ep_select(usbhp->inEpNum);
  USBHREG->USB_RXTYPE = (0x02<<4)+usbhp->inEpNum; //bulk + out ep number
  USBHREG->USB_RXINTERVAL = 0;//never timeout

  if (dma_mode == DMA_MODE_1)
  {
    nds32_dcache_flush();
    if(usb_h_lld_bulk_inep_dmamode1(usbhp, buf, len))
    {
      return TRUE;
    }
    else
    {
      goto usb_h_lld_bulk_inep_error;
    }
  }

  while(left)
  {
    uint8_t pktsiz = left>usbhp->inEpPktSiz?usbhp->inEpPktSiz:left;
    chSysLock();
    hsusb_ep_select(usbhp->inEpNum);
    USBHREG->USB_RXCSR = HSUSB_RXCSR_H_REQPKT;
    usbhp->usbinep_thread = chThdGetSelfX();
    chSchGoSleepS(CH_STATE_SUSPENDED);
    chSysUnlock();

    if ((USB_H_DISCONNECTED == usbhp->host_state) ||
        (USB_H_STOP == usbhp->host_state)) {
      goto usb_h_lld_bulk_inep_error;
    }

    hsusb_ep_select(usbhp->inEpNum);
    csr = USBHREG->USB_RXCSR;
    if(csr&(HSUSB_RXCSR_H_RXSTALL|HSUSB_RXCSR_H_ERROR|HSUSB_RXCSR_DATAERROR))
    {
      goto usb_h_lld_bulk_inep_error;
    }
    if(!(HSUSB_RXCSR_RXPKTRDY&csr))
    {
      goto usb_h_lld_bulk_inep_error;
    }

    if(dma_mode == DMA_MODE_NONE)
    {
      usb_packet_read_to_buffer(usbhp->usbp,
          usbhp->inEpNum,
          &buf[len-left],
          pktsiz);
    }
    else if (dma_mode == DMA_MODE_0)
    {
      nds32_dcache_flush();
      chSysLock();
      hsusb_ep_select(usbhp->inEpNum);
      csr = USBHREG->USB_RXCSR;
      csr |= HSUSB_RXCSR_DMAENAB;
      csr &= ~HSUSB_RXCSR_DMAMODE;
      USBHREG->USB_RXCSR = csr;
      usb_dma_config_ch(usbhp->usbp,
          BULK_IN_DMA_CH,
          usbhp->inEpPktSiz,
          DMA_MODE_0,
          FALSE,
          usbhp->inEpNum,
          (uint32_t)&buf[len-left],
          pktsiz);
      usbhp->usbinep_thread = chThdGetSelfX();
      chSchGoSleepS(CH_STATE_SUSPENDED);
      chSysUnlock();

      if ((USB_H_DISCONNECTED == usbhp->host_state) ||
          (USB_H_STOP == usbhp->host_state)) {
        goto usb_h_lld_bulk_inep_error;
      }

      //nds32_dcache_invalidate();
      //USBHDMAREG(BULK_IN_DMA_CH).USB_DMA_CTL=0;
      USBDMA(BULK_IN_DMA_CH)->USB_DMA_CTL=0;
    }
    else
    {
      goto usb_h_lld_bulk_inep_error;
    }
    USBHREG->USB_RXCSR = 0;
    left-=pktsiz;
  }
  return TRUE;
  usb_h_lld_bulk_inep_error:
  USBHREG->USB_RXCSR = HSUSB_RXCSR_FLUSHFIFO;
  return FALSE;
}

uint8_t usb_h_lld_send_cbw(USBHostDriver *usbhp)
{
  return usb_h_lld_bulk_outep(usbhp, (uint8_t*)&usbhp->cbw, 31, CBW_DMA_MODE);
}

uint8_t usb_h_lld_get_csw(USBHostDriver *usbhp)
{

  return usb_h_lld_bulk_inep(usbhp, (uint8_t*)&usbhp->csw, 13, CSW_DMA_MODE);
}

uint8_t usb_h_lld_write_bulkdata(USBHostDriver *usbhp, uint8_t* buf, uint32_t len, uint8_t dma_mode)
{
  return usb_h_lld_bulk_outep(usbhp, buf, len, dma_mode);
}

uint8_t usb_h_lld_read_bulkdata(USBHostDriver *usbhp, uint8_t* buf, uint32_t len, uint8_t dma_mode)
{
  return usb_h_lld_bulk_inep(usbhp, buf, len, dma_mode);
}

uint8_t usb_h_lld_scsi_read10(USBHostDriver *usbhp, uint32_t start_blk, uint16_t blk_num, uint8_t* buf)
{
  uint8_t ret;
  uint32_t len;
  RETURN_FALSE_IF_HOST_NOT_READY(usbhp)

  usbhp->cbw.Tag = usbhp->magicNum++;
  usbhp->cbw.DataTransferLength = blk_num*usbhp->blkinfo.blk_size;
  usbhp->cbw.Flags = 0x80;
  usbhp->cbw.Lun = 0x00;
  usbhp->cbw.Length = 0x0A;
  memset(usbhp->cbw.CDB, 0, 16);
  usbhp->cbw.CDB[0] = SCSI_READ_10;
  start_blk = swap32(start_blk);
  blk_num = swap16(blk_num);
  memcpy(&usbhp->cbw.CDB[2], &start_blk, 4);
  memcpy(&usbhp->cbw.CDB[7], &blk_num, 2);
  start_blk = swap32(start_blk);
  blk_num = swap16(blk_num);
  ret = usb_h_lld_send_cbw(usbhp);
  if(!ret)
  {
    USBH_PRINTF("send read10 cbw fail!\n\r");
    goto usb_h_lld_scsi_read10_error;
  }

  len = blk_num*usbhp->blkinfo.blk_size;
  if(R_DMA_MODE == DMA_MODE_1)
  {
    len += CSW_SIZ;
  }

  //memset(buf, 0xaa, len);
  ret = usb_h_lld_read_bulkdata(usbhp, buf, len, R_DMA_MODE);
  if(!ret)
  {
    USBH_PRINTF("read read10 data fail!\n\r");
    goto usb_h_lld_scsi_read10_error;
  }
  else
  {

    USBH_PRINTF("read read10 data:blk:%d, %d bytes\n\r",start_blk, usbhp->blkinfo.blk_size*blk_num);
    //usb_h_dbg_printdata(buf, usbhp->blkinfo.blk_size*blk_num);
  }

  if(R_DMA_MODE != DMA_MODE_1)
  {
    ret = usb_h_lld_get_csw(usbhp);
    if(!ret)
    {
      USBH_PRINTF("get read10 csw fail!\n\r");
      goto usb_h_lld_scsi_read10_error;
    }
  }
  return TRUE;

  usb_h_lld_scsi_read10_error:
  hsusb_ep_select(usbhp->inEpNum);
  //USBHDMAREG(BULK_IN_DMA_CH).USB_DMA_CTL=0;
  USBDMA(BULK_IN_DMA_CH)->USB_DMA_CTL=0;
  USBHREG->USB_RXCSR = 0;
  return FALSE;

}

uint8_t usb_h_lld_scsi_write10(USBHostDriver *usbhp, uint32_t start_blk, uint16_t blk_num, uint8_t* buf)
{
  uint8_t ret;

  RETURN_FALSE_IF_HOST_NOT_READY(usbhp)

  usbhp->cbw.Tag = usbhp->magicNum++;
  usbhp->cbw.DataTransferLength = blk_num*usbhp->blkinfo.blk_size;
  usbhp->cbw.Flags = 0x00;
  usbhp->cbw.Lun = 0x00;
  usbhp->cbw.Length = 0x0A;
  memset(usbhp->cbw.CDB, 0, 16);
  usbhp->cbw.CDB[0] = SCSI_WRITE_10;
  start_blk = swap32(start_blk);
  blk_num = swap16(blk_num);
  memcpy(&usbhp->cbw.CDB[2], &start_blk, 4);
  memcpy(&usbhp->cbw.CDB[7], &blk_num, 2);
  start_blk = swap32(start_blk);
  blk_num = swap16(blk_num);
  ret = usb_h_lld_send_cbw(usbhp);
  if(!ret)
  {
    USBH_PRINTF("send write10 cbw fail!\n\r");
    goto usb_h_lld_scsi_write10_error;
  }

  ret = usb_h_lld_write_bulkdata(usbhp, buf, blk_num*usbhp->blkinfo.blk_size, W_DMA_MODE);
  if(!ret)
  {
    USBH_PRINTF("read write10 data fail!\n\r");
    goto usb_h_lld_scsi_write10_error;
  }
  else
  {
    USBH_PRINTF("read write10 data:blk:%d, %d bytes\n\r",start_blk, usbhp->cbw.DataTransferLength);
    //usb_h_dbg_printdata(buf, 16);//usbhp->cbw.DataTransferLength);
  }

  ret = usb_h_lld_get_csw(usbhp);
  if(!ret)
  {
    USBH_PRINTF("get write10 csw fail!\n\r");
    goto usb_h_lld_scsi_write10_error;
  }
  return TRUE;
  usb_h_lld_scsi_write10_error:
  hsusb_ep_select(usbhp->outEpNum);
  //USBHDMAREG(BULK_OUT_DMA_CH).USB_DMA_CTL=0;
  USBDMA(BULK_OUT_DMA_CH)->USB_DMA_CTL=0;
  USBHREG->USB_TXCSR = 0;
  return FALSE;
}

uint8_t usb_h_lld_scsi_requestsense(USBHostDriver *usbhp)
{
  uint8_t ret;
  uint8_t cmd[6] = {0x03,0,0,0,18,0};
  uint8_t sense_data[18];

  //RETURN_FALSE_IF_HOST_NOT_READY(usbhp)

  usbhp->cbw.Tag = usbhp->magicNum++;
  usbhp->cbw.DataTransferLength = 18;
  usbhp->cbw.Flags = 0x80; /*DEVICE_TO_HOST*/
  usbhp->cbw.Lun = 0x00;
  usbhp->cbw.Length = 0x06;
  memset(usbhp->cbw.CDB, 0, 16);
  memcpy(usbhp->cbw.CDB, cmd, 6);
  ret = usb_h_lld_send_cbw(usbhp);
  if(!ret)
  {
    USBH_PRINTF("send RequestSense cbw fail!\n\r");
    return FALSE;
  }

  ret = usb_h_lld_read_bulkdata(usbhp, sense_data, 18, CMD_DMA_MODE);
  if(!ret)
  {
    USBH_PRINTF("read sense data fail!\n\r");
    //return FALSE;
  }

  ret = usb_h_lld_get_csw(usbhp);
  if(!ret || (usbhp->csw.Signature != BULK_CSW_SIGNITURE))
  {
    USBH_PRINTF("get RequestSense csw fail!\n\r");
    return FALSE;
  }

  USBH_PRINTF("RequestSense Residue=%d,Status=%d\n\r",usbhp->csw.Residue,usbhp->csw.Status);
  if(usbhp->csw.Residue == 0 && usbhp->csw.Status != 1)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

uint8_t usb_h_lld_scsi_testunitready(USBHostDriver *usbhp)
{
  uint8_t ret;

  //RETURN_FALSE_IF_HOST_NOT_READY(usbhp)

  usbhp->cbw.Tag = usbhp->magicNum++;
  usbhp->cbw.DataTransferLength = 0x00000000;
  usbhp->cbw.Flags = 0x00;
  usbhp->cbw.Lun = 0x00;
  usbhp->cbw.Length = 0x06;
  memset(usbhp->cbw.CDB, 0, 16);
  ret = usb_h_lld_send_cbw(usbhp);
  if(!ret)
  {
    USBH_PRINTF("send testunitready cbw fail!\n\r");
    return FALSE;
  }
  ret = usb_h_lld_get_csw(usbhp);
  if(!ret)
  {
    USBH_PRINTF("get testunitready csw fail!\n\r");
    return FALSE;
  }

  if(usbhp->csw.Signature!=BULK_CSW_SIGNITURE||usbhp->csw.Tag!=usbhp->cbw.Tag)
  {
    USBH_PRINTF("csw data error!\n\r");
    return FALSE;
  }

  USBH_PRINTF("testunitready Residue=%d,Status=%d\n\r",usbhp->csw.Residue,usbhp->csw.Status);
  if(usbhp->csw.Residue == 0 && usbhp->csw.Status == 0)
  {
    return TRUE;
  }
  if (usbhp->csw.Status == 1)
  {
    return usb_h_lld_scsi_requestsense(usbhp);
  }
  return FALSE;
}

uint8_t usb_h_lld_scsi_readcapacity(USBHostDriver *usbhp)
{
  uint8_t ret;
  //RETURN_FALSE_IF_HOST_NOT_READY(usbhp)

  usbhp->cbw.Tag = usbhp->magicNum++;
  usbhp->cbw.DataTransferLength = 0x00000008;
  usbhp->cbw.Flags = 0x80;
  usbhp->cbw.Lun = 0x00;
  usbhp->cbw.Length = 0x0a;
  memset(usbhp->cbw.CDB, 0, 16);
  usbhp->cbw.CDB[0] = SCSI_READ_CAPACITY;
  ret = usb_h_lld_send_cbw(usbhp);
  if(!ret)
  {
    USBH_PRINTF("send capacity cbw fail!\n\r");
    goto usb_h_lld_scsi_readcapacity_error;
  }

  ret = usb_h_lld_read_bulkdata(usbhp, (uint8_t*)&usbhp->blkinfo, 8, CMD_DMA_MODE);
  if(!ret)
  {
    USBH_PRINTF("read capacity data fail!\n\r");
    goto usb_h_lld_scsi_readcapacity_error;
  }
  else
  {
    usbhp->blkinfo.blk_cnt = swap32(usbhp->blkinfo.blk_cnt);
    usbhp->blkinfo.blk_size = swap32(usbhp->blkinfo.blk_size);
    USBH_PRINTF("capacity data: blknum=%08x(%d), blksiz=%08x\n\r", usbhp->blkinfo.blk_cnt, usbhp->blkinfo.blk_cnt, usbhp->blkinfo.blk_size);
  }

  ret = usb_h_lld_get_csw(usbhp);
  if(!ret)
  {
    USBH_PRINTF("get capacity csw fail!\n\r");
    goto usb_h_lld_scsi_readcapacity_error;
  }
  return TRUE;
  usb_h_lld_scsi_readcapacity_error:

  return FALSE;

}

uint8_t usb_h_lld_scsi_inquiry(USBHostDriver *usbhp)
{
  uint8_t ret;
  uint8_t result[36];
  //RETURN_FALSE_IF_HOST_NOT_READY(usbhp)

  usbhp->cbw.Tag = usbhp->magicNum++;
  usbhp->cbw.DataTransferLength = 0x00000024;
  usbhp->cbw.Flags = 0x80;
  usbhp->cbw.Lun = 0x00;
  usbhp->cbw.Length = 0x06;
  memset(usbhp->cbw.CDB, 0, 16);
  usbhp->cbw.CDB[0] = SCSI_INQUIRY;
  usbhp->cbw.CDB[4] = 0x24;
  ret = usb_h_lld_send_cbw(usbhp);
  if(!ret)
  {
    USBH_PRINTF("send inquiry cbw fail!\n\r");
    goto usb_h_lld_scsi_inquiry_error;
  }

  ret = usb_h_lld_read_bulkdata(usbhp, result, 36, CMD_DMA_MODE);
  if(!ret)
  {
    USBH_PRINTF("read inquiry data fail!\n\r");
    goto usb_h_lld_scsi_inquiry_error;
  }
  else
  {
    //just dummy inquiry
    char vid_pid[17];
    memcpy(vid_pid, &result[8], 8);
    vid_pid[8] = 0;
    USBH_PRINTF("MSD - Vendor ID: %s\r\n", vid_pid);

    memcpy(vid_pid, &result[16], 16);
    vid_pid[16] = 0;
    USBH_PRINTF("MSD - Product ID: %s\r\n", vid_pid);

    memcpy(vid_pid, &result[32], 4);
    vid_pid[4] = 0;
    USBH_PRINTF("MSD - Product rev: %s\r\n", vid_pid);
  }

  ret = usb_h_lld_get_csw(usbhp);
  if(!ret)
  {
    USBH_PRINTF("get capacity csw fail!\n\r");
    goto usb_h_lld_scsi_inquiry_error;
  }
  return TRUE;
  usb_h_lld_scsi_inquiry_error:

  return FALSE;
}

uint8_t usb_h_lld_scsi_readcapacities(USBHostDriver *usbhp)
{
  uint8_t ret;
  uint8_t data_buf[12];
  RETURN_FALSE_IF_HOST_NOT_READY(usbhp)

  usbhp->cbw.Tag = usbhp->magicNum++;
  usbhp->cbw.DataTransferLength = 0x000000FC;
  usbhp->cbw.Flags = 0x80;
  usbhp->cbw.Lun = 0x00;
  usbhp->cbw.Length = 0x0a;
  memset(usbhp->cbw.CDB, 0, 16);
  usbhp->cbw.CDB[0] = SCSI_READ_CAPACITIES;
  usbhp->cbw.CDB[8] = 0xFC;
  ret = usb_h_lld_send_cbw(usbhp);
  if(!ret)
  {
    USBH_PRINTF("send capacities cbw fail!\n\r");
    goto usb_h_lld_scsi_readcapacities_error;
  }

  ret = usb_h_lld_read_bulkdata(usbhp, data_buf, 12, CMD_DMA_MODE);
  if(!ret)
  {
    USBH_PRINTF("read capacities data fail!\n\r");
    goto usb_h_lld_scsi_readcapacities_error;
  }
  else
  {
    //just dummy capacities
  }

  ret = usb_h_lld_get_csw(usbhp);
  if(!ret)
  {
    USBH_PRINTF("get capacities csw fail!\n\r");
    goto usb_h_lld_scsi_readcapacities_error;
  }
  return TRUE;
  usb_h_lld_scsi_readcapacities_error:

  return FALSE;

}

#endif

