#ifndef _USB_DEV_H_
#define _USB_DEV_H_

#include "usbdef.h"
#if HAL_USE_USB

typedef int32_t (*hs_pfnUsbDevInit_t)(const void * pArg);
typedef void    (*hs_pfnUsbDevUninit_t)(void);

typedef struct 
{
  const USBConfig          *pstConfig;
  hs_pfnUsbDevInit_t       pDevInit;
  hs_pfnUsbDevUninit_t     pDevUninit;
  const void               *pArg;
}hs_usbdev_t;

typedef enum
{
  USB_DEVTYPE_SERIAL           = 0,
  USB_DEVTYPE_AUDIO_STORAGE    ,
  USB_DEVTYPE_HS6200           ,

  USB_DEVTYPE_HOST_DISK        ,
  USB_DEVTYPE_NODEV            ,
}hs_usbdevtype_t;

int  hs_usb_open(hs_usbdevtype_t dev);
int  hs_usb_close(hs_usbdevtype_t dev);

void hs_usb_scanDisk(void);
void hs_usb_serialOpen(uint16_t u16Idx, void *parg) __attribute__((used));
void hs_usb_serialClose(uint16_t u16Idx, void *parg) __attribute__((used));
#endif

#endif
