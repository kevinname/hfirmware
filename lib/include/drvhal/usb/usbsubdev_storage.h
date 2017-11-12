#ifndef _USBSUBDEV_STORAGE_H_
#define _USBSUBDEV_STORAGE_H_

#include "ch.h"
#include "hal.h"
#include "usbdev.h"

#if HAL_USE_USB_STORAGE
extern const If_Num dev_if_num_storage;
extern  const USBEndpointConfig ep2config_bulk;
extern  USBInEndpointState ep2instate_bulk;
extern  USBOutEndpointState ep2outstate_bulk;
extern  const Fifo_Cfg fifo_cfg_bulk[];
void usb_event_storage(USBDriver *usbp, usbevent_t event);
#endif

#endif