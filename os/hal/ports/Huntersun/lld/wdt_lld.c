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
 * @file    hs66xx/wdt_lld.c
 * @brief   HS66xx WATCHDOG low level driver code.
 *
 * @addtogroup WATCHDOG
 * @{
 */

#include "ch.h"
#include "hal.h"

#if HAL_USE_WDT || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

static inline uint32_t wdt_top_in_seconds(uint8_t top)
{
	/*
	 * There are 16 possible timeout values in 0..15 where the number of
	 * cycles is 2 ^ (16 + i) and the watchdog counts down.
	 */
	return (1 << (16 + top)) / HS_WDTCLK;
}

#if 0//unused routine
static INLINE uint32_t wdt_is_enabled(void)
{
	return HS_WDT->CR & WDT_CONTROL_REG_WDT_EN_MASK;
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Get Watchdog Current Counter Value
 *
 * @return  Current Counter Value.
 * @notapi
 */
uint32_t wdt_time_left(void){
	return HS_WDT->CCVR / HS_WDTCLK;	
}

/**
 * @brief   Get Watchdog Timeout value
 *
 * @return   Watchdog Timeout value.
 * @notapi
 */

uint32_t wdt_get_timeout(void) {

	int top = HS_WDT->TORR & 0xF;

	return wdt_top_in_seconds(top);	
}

/**
 * @brief   Set Watchdog Timeout value
 *
 * @param[in] seconds   seconds Timeout TO set to Watchdog
 *
 * @return    The real Watchdog Timeout value TO set to Watchdog
 *
 * @notapi
 */

uint32_t wdt_set_timeout(uint32_t seconds) {
	int i, top_val = HS_WDT_MAX_TOP;

	/*
	 * Iterate over the timeout values until we find the closest match. We
	 * always look for >=.
	 */
	for (i = 0; i <= HS_WDT_MAX_TOP; ++i)
		if (wdt_top_in_seconds(i) >= seconds) {
			top_val = i;
			break;
		}

	/* Set the new value in the watchdog. */
	HS_WDT->TORR = top_val << 4 | top_val;

	return wdt_top_in_seconds(top_val);	
}

void wdt_disable(void) {
  HS_WDT->CLOCK_EN = 0;
}


/**
 * @brief   Kickoff the dog
 *
 * @notapi
 */

void wdt_keepalive(void) {
  HS_WDT->CRR = WDT_COUNTER_RESTART_KICK_VALUE;	
}

/**
 * @brief   Watchdog driver initialization
 *
 * @notapi
 */

void wdt_init(void) {

	uint32_t reg;
	
	cpmEnableWDT();

	/* clear wdt interrupt */
	reg = HS_WDT->EOI;
	reg = 0;
        
	/*
	 * The watchdog is not currently enabled. Set the timeout to
	 * the maximum and then start it.
	 */
	wdt_set_timeout(HS_WDT_DEFAULT_TOP);
	
	/* Set Clock Enable */
	HS_WDT->CLOCK_EN = WDT_CLOCK_EN_VALUE;
	
	/* Timeout period is default = */
	/* This is used to select the number of pclk cycles 
	   for which the system reset stays asserted*/
	reg |= WDT_CR_RPL_256;
	
	/* First generate an interrupt and if it isnot cleared by the time a 
	   second timeout occurs then generate a system reset*/
	//reg |= WDT_CR_RMOD_INT;
	/* Generate a system reset without interrupt */
	reg &= ~WDT_CR_RMOD_INT;

	/* Once watchdog is enabled, it can be cleared only by a system reset */
	reg |= WDT_CR_EN;
	
	HS_WDT->CR = reg;
	
	/* enable watchdog interrupt */
	nvicEnableVector(IRQ_WDT, ANDES_PRIORITY_MASK(HS_WDT_IRQ_PRIORITY));
		
	return ;
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/**
 * @brief   WATCHDOG interrupt handler.
 *
 * @notapi
 */

CH_IRQ_HANDLER(WDT_IRQHandler) {

  CH_IRQ_PROLOGUE();

  /* don't clear watchdog interrupt here, but the next startup */
	
  CH_IRQ_EPILOGUE();	
}

#endif /* HAL_USE_WDT */

/** @} */
