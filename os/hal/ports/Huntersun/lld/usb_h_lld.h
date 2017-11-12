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

#ifndef _USB_H_LLD_H_
#define _USB_H_LLD_H_

#if HAL_USE_USB_HOST_STORAGE
#include "usb_storage.h"
#include "ff.h"

typedef enum {
  USB_H_UNKNOWNDEV = 0,
  USB_H_STOP     = 1,
  USB_H_POWERED  = 2,
  USB_H_ADDRESSED= 3,
  USB_H_READY    = 4,
  USB_H_SUSPEND  = 5,
  USB_H_CONNECTED= 6,
  USB_H_DISCONNECTED= 7,
}usb_h_state_t;

typedef enum {
  GET_DEV_DESC   = 0,
  SET_DEV_ADDR   = 1,
  GET_CFG_DESC   = 2,
  SET_CFG        = 3,
  GET_MAXLUN     = 4,
  SET_INF        = 5,
}usb_cmd_idx;

typedef struct _DISK_BLOCK_INFO {
    uint32_t blk_cnt;
    uint32_t blk_size;//in bytes
}DISK_BLOCK_INFO;

#define USB_H_MAILBOX_SIZE 4

typedef struct _USBHostDriver {
  USBDriver                     *usbp;
  usb_h_state_t                 host_state;
  uint16_t                      inEpPktSiz;
  uint16_t                      outEpPktSiz;
  uint8_t                       maxlun;
  uint8_t                       inEpNum;
  uint8_t                       outEpNum;
  bulk_cbw                      cbw;
  bulk_csw                      csw;
  DISK_BLOCK_INFO               blkinfo;
  HS_USB_Type                   *hs_usb;

  thread_t                      *usbhost_thread;
  thread_t                      *usbenum_thread;
  thread_t                      *usboutep_thread;
  thread_t                      *usbinep_thread;

  //FATFS                         *usbh_fs;

  mailbox_t                     mbox;
  msg_t                         message[USB_H_MAILBOX_SIZE];
  mutex_t                       mutex;


  uint32_t                       magicNum;//used for cbw and csw

}USBHostDriver;

extern USBHostDriver USBHOST0;


#define BULK_IN_DMA_CH  0
#define BULK_OUT_DMA_CH 1

#define DEFAULT_DEV_ADDR 0x02

#define CBW_DMA_MODE    DMA_MODE_NONE
#define CSW_DMA_MODE    DMA_MODE_NONE
#define CMD_DMA_MODE    DMA_MODE_NONE
#define R_DMA_MODE      DMA_MODE_1
#define W_DMA_MODE      DMA_MODE_1

#define FS_DRV_USB_H    1  //pengjiang

#define CBW_SIZ         31
#define CSW_SIZ         13

#define USBHREG         HS_USB

#define swap16(x)       ((((x) & 0xFF00) >> 8) | (((x) & 0x00FF) << 8))
#define swap32(x)       ((((x) & 0xFF000000)>>24) | (((x) & 0x00FF0000)>>8) |\
                           (((x) & 0x0000FF00)<<8 ) | (((x) & 0x000000FF)<<24) )


#define RETURN_IF_HOST_NOT_READY(usbhp) {\
  if((usbhp)->host_state != USB_H_READY) \
  return;                                \
}

#define RETURN_FALSE_IF_HOST_NOT_READY(usbhp) {\
  if((usbhp)->host_state != USB_H_READY) \
  return FALSE;                                \
}

#ifdef __cplusplus
extern "C" {
#endif
    void usbhostInit(void);
    void usb_h_lld_start(USBHostDriver *usbhp) ;
    void usb_h_lld_stop(USBHostDriver *usbhp);
    void hs66xx_usb_host_irq(uint8_t usb, uint16_t tx, uint16_t rx) ;
    void hs66xx_usb_host_dma_irq(void);
    uint8_t usb_h_lld_scsi_testunitready(USBHostDriver *usbhp);
    uint8_t usb_h_lld_scsi_readcapacity(USBHostDriver *usbhp);
    uint8_t usb_h_lld_scsi_write10(USBHostDriver *usbhp, uint32_t start_blk, uint16_t blk_num, uint8_t* buf);
    uint8_t usb_h_lld_scsi_read10(USBHostDriver *usbhp, uint32_t start_blk, uint16_t blk_num, uint8_t* buf);
    uint8_t usb_h_lld_scsi_inquiry(USBHostDriver *usbhp);
    uint8_t usb_h_lld_scsi_readcapacities(USBHostDriver *usbhp);
#ifdef __cplusplus
}
#endif

#endif

#endif
