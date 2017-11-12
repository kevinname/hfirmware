/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012,2013 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes ChibiOS/RT, without being obliged to provide
    the source code for any proprietary components. See the file exception.txt
    for full details of how and when the exception can be applied.
*/

#ifndef _USB_HOST_H_
#define _USB_HOST_H_

#include "usb_h_lld.h"

#if HAL_USE_USB_HOST_STORAGE



#define usb_h_get_disk_status(usbhp)  ((usbhp)->host_state)
#define usb_h_is_disk_ready(usbhp)    ((usbhp)->host_state==USB_H_READY)

#define usbDiskReady(usbhp)           ((usbhp)->host_state==USB_H_READY)

#ifdef __cplusplus
extern "C" {
#endif
void usb_h_start(USBHostDriver *usbhp);
void usb_h_stop(USBHostDriver *usbhp);
bool_t usb_h_getdiskinfo(USBHostDriver *usbhp, DISK_BLOCK_INFO* blk_info);
bool_t usb_h_readdisk(USBHostDriver *usbhp, uint32_t start_blk, uint16_t numBlk, uint8_t *buf);
bool_t usb_h_writedisk(USBHostDriver *usbhp, uint32_t start_blk, uint16_t numBlk, const uint8_t *buf);
#ifdef __cplusplus
}
#endif



#endif
#endif


