#ifndef _USBSUBDEV_AUDIO_H_
#define _USBSUBDEV_AUDIO_H_

#include "ch.h"
#include "hal.h"
#include "usbdev.h"

#if HAL_USE_USB_AUDIO
extern const as_if_info if_info[2];
extern const If_Num dev_if_num_audio;
extern const As_Ep_Num dev_ep_num_audio;
void usb_event_audio(USBDriver *usbp, usbevent_t event) ;
#endif

#if HAL_USE_USB_AUDIO||HAL_USE_USB_STORAGE
extern const USBDescriptor device_descriptor_composite;
extern const USBDescriptor configuration_descriptor_composite_all;
extern const USBDescriptor configuration_descriptor_composite_audio;
extern const USBDescriptor configuration_descriptor_composite_stor;
extern const USBDescriptor strings_composite[];

#endif


#endif