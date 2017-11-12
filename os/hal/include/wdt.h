/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012,2013 Giovanni Di Sirio.
		 Copyright (C) 2014 HunterSun Technologies
                 zutao.min@huntersun.com.cn
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

/**
 * @file    wdt.h
 * @brief   WATCHDOG Driver macros and structures.
 *
 * @addtogroup WATCHDOG
 * @{
 */

#ifndef _WATCHDOG_H_
#define _WATCHDOG_H_

#if HAL_USE_WDT || defined(__DOXYGEN__)


#include "wdt_lld.h"

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/
#define     wdtInit()          wdt_init()
#define     wdtKickdog()       wdt_keepalive()
#define     wdtDisable()       wdt_disable()
#define     wdtSetTimeout(s)   wdt_set_timeout(s)
#define     wdtGetTimeout()    wdt_get_timeout()
#define     wdtTimeLeft()      wdt_time_left()

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#endif /* HAL_USE_WDT */

#endif /* _WATCHDOG_H_ */

/** @} */
