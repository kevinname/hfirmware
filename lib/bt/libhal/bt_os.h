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

#ifndef _BT_CH_H_
#define _BT_CH_H_

#include "stdint.h"
#include "autoconf.h"

/*===========================================================================*/
/* OS-dependant                                                              */
/*===========================================================================*/

#define	TASK_STK_SIZE_BT_HOST               (384*4)
#define TASK_STK_SIZE_BT_TIMER              (128*4)
#define TASK_STK_SIZE_BT_HFP                (384*4)
#define TASK_STK_SIZE_BT_HC                 (4096+1024)
#define TASK_STK_SIZE_BT_A2DP               (384*4)

#define TASK_PRIO_BT_HOST             osPriorityNormal
#define TASK_PRIO_BT_TIMER            osPriorityNormal
#define TASK_PRIO_BT_HFP              osPriorityNormal
#define TASK_PRIO_BT_HC               osPriorityNormal
#define TASK_PRIO_BT_A2DP             osPriorityNormal

#define chThdSelf() currp

#define PRINT_TIMESTAMP() hs_printf("[%06u][%s] ", chVTGetSystemTime(), curthread()->p_name)

#if defined(CONFIG_DEBUG_SYMBOL)
#define debug(fmt,args...) do { PRINT_TIMESTAMP(); hs_printf(fmt, ##args); } while (0)
void printf_packet(uint8_t packet_type, uint8_t in, uint8_t *header, uint8_t *packet, uint16_t len);
#else
#define debug(fmt,args...)
#define printf_packet(packet_type, in, header, packet, len)
#endif

#if defined(__nds32__)
#define __ONCHIP_VHCI__  __attribute__ ((section (".bttext"), optimize(s), noinline))
#else
#define __ONCHIP_VHCI__
#endif


void * hs_bthc_malloc(uint32_t size);
void   hs_bthc_free(void *p);

int    hs_bthc_start(uint8_t mode);
void   hs_bthc_stop(void);

int    hs_bthost_start(uint8_t mode);
void   hs_bthost_stop(uint8_t mode);

#endif
