#ifndef _USBSUBDEV_HS6200_H_
#define _USBSUBDEV_HS6200_H_

#include "ch.h"
#include "hal.h"
#include "usbdev.h"



#define USB_DMA_CH_HS6200_OUTEP     0
#define USB_DMA_CH_HS6200_INEP      1
#define USB_HS6200_IF_NUM_BULK      0
#define USB_HS6200_BULK_OUT_EP      2 //bulk out endpoint
#define USB_HS6200_BULK_IN_EP       USB_HS6200_BULK_OUT_EP//bulk in endpoint
#define USB_HS6200_BULK_OUT_PKTSIZ  0x40
#define USB_HS6200_BULK_IN_PKTSIZ   0x40

#define USB_VID_HS6200              0x10C4 
#define USB_PID_HS6200              0xEA61

#define USB_HS6200_STOP             0
#define USB_HS6200_READY            1

#if HAL_USE_USB_BULK_HS6200

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//USB HS6200 APIs
uint16_t Block_Write (uint8_t *Buffer, uint16_t NumBytes);
uint8_t Block_Read (uint8_t *Buffer, uint8_t NumBytes);
bool_t UsbHs6200_IsReady(void);
void UsbHs6200_SetMailBox(mailbox_t *pmbox);
void UsbHs6200_StartUsb();
void UsbHs6200_StopUsb(void);
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////


#ifdef __cplusplus
}
#endif


//other functions
#ifdef __cplusplus
extern "C" {
#endif
bool_t usb_hs6200_vendor_req_handler(USBDriver *usbp);
#ifdef __cplusplus
}
#endif


extern const If_Num dev_if_num_hs6200;
extern  const USBEndpointConfig ep2config_hs6200;
extern  USBInEndpointState ep2instate_hs6200;
extern  USBOutEndpointState ep2outstate_hs6200;
extern  const Fifo_Cfg fifo_cfg_hs6200[];
void usb_event_hs6200(USBDriver *usbp, usbevent_t event);
extern  const USBDescriptor strings_hs6200[];
extern  const USBDescriptor configuration_descriptor_hs6200;
extern  const USBDescriptor device_descriptor_hs6200;

#endif

#endif
