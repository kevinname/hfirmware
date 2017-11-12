/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio
                 Copyright (C) 2014 HunterSun Technologies
                 zutao.min@huntersun.com.cn

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

/**
 * @file    HS66xx/wdt_lld.h
 * @brief   HS66xx WATCHDOG low level driver header.
 *
 * @addtogroup WATCHDOG
 * @{
 */

#ifndef _WDT_LLD_H_
#define _WDT_LLD_H_

#if HAL_USE_WDT || defined(__DOXYGEN__)

/**
 * @brief   SDHC interrupt priority level setting.
 */
#if !defined(HS_WDT_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define HS_WDT_IRQ_PRIORITY         2
#endif

/* The maximum TOP (timeout period) value that can 
 * be set in the watchdog. 
 */
#define HS_WDT_MAX_TOP		15

/* The default TOP (timeout period) value that can 
 * be set in the watchdog. 
 */
#define HS_WDT_DEFAULT_TOP	4

#define HS_WDTCLK               (cpm_get_clock(HS_WDT_CLK))

/**
 * @brief   WATCHDOG ports subsystem initialization.
 *
 * @notapi
 */
#define wdt_lld_init(void) wdt_init(void)

#ifdef __cplusplus
extern "C" {
#endif
  void wdt_init(void);
  void wdt_keepalive(void);
  void wdt_disable(void);
  uint32_t wdt_set_timeout(uint32_t seconds);
  uint32_t wdt_get_timeout(void);
  uint32_t wdt_time_left(void);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_WDT */

#endif /* _WDT_LLD_H_ */

/** @} */
