#ifndef _USBSUBDEV_SERIAL_H_
#define _USBSUBDEV_SERIAL_H_

#include "ch.h"
#include "hal.h"
#include "usbdev.h"

#if HAL_USE_USB_SERIAL

#define USBD1_SERIAL_BULK_OUT_EP        1 //audio out endpoint
#define USBD1_SERIAL_BULK_IN_EP         USBD1_SERIAL_BULK_OUT_EP //audio in endpoint
#define USBD1_SERIAL_INT_IN_EP          2 //audio in endpoint

extern const USBDescriptor strings_serial[];
extern const SerialUSBConfig serusbcfg;
extern const If_Num dev_if_num_serial;
//extern const As_Ep_Num dev_ep_num_serial;
extern const USBDescriptor device_descriptor_serial;
extern const uint8_t configuration_descriptor_data[];
extern const USBDescriptor configuration_descriptor_serial;

void usb_event_serial(USBDriver *usbp, usbevent_t event) ;
#endif

#endif