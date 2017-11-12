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

/*
    PengJiang, 20140910
    NOTE:
    The host supports storage device only
*/

#include <string.h>

#include "ch.h"
#include "hal.h"
#include "usb_host.h"

#if HAL_USE_USB_HOST_STORAGE


void usb_h_start(USBHostDriver *usbhp)
{
  usb_h_lld_start(usbhp);
}

void usb_h_stop(USBHostDriver *usbhp)
{
  osalMutexLock(&usbhp->mutex);
  usb_h_lld_stop(usbhp);
  osalMutexUnlock(&usbhp->mutex);
}

//get block size and block number
bool_t usb_h_getdiskinfo(USBHostDriver *usbhp, DISK_BLOCK_INFO* blk_info)
{
    if(usbhp->blkinfo.blk_cnt==0&&usbhp->blkinfo.blk_size==0)
    {
        if(!usb_h_lld_scsi_readcapacity(usbhp))
        {
            return FALSE;
        }
    }
    blk_info->blk_cnt = usbhp->blkinfo.blk_cnt;
    blk_info->blk_size = usbhp->blkinfo.blk_size;
    return TRUE;
}

bool_t usb_h_readdisk(USBHostDriver *usbhp, uint32_t start_blk, uint16_t numBlk, uint8_t *buf)
{
  bool_t res;
  
  osalMutexLock(&usbhp->mutex);
  res = usb_h_lld_scsi_read10(usbhp, start_blk, numBlk, buf);
  osalMutexUnlock(&usbhp->mutex);

  return res;
}

bool_t usb_h_writedisk(USBHostDriver *usbhp, uint32_t start_blk, uint16_t numBlk, const uint8_t *buf)
{
  bool_t res;
  
  osalMutexLock(&usbhp->mutex);
  res = usb_h_lld_scsi_write10(usbhp, start_blk, numBlk, (uint8_t*)buf);
  osalMutexUnlock(&usbhp->mutex);

  return res;
}

#endif


