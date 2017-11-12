/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio
                 Copyright (C) 2014 HunterSun Technologies
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

/**
 * @file    hs66xx/hal_lld.c
 * @brief   HAL Driver subsystem low level driver source template.
 *
 * @addtogroup HAL
 * @{
 */

#include "ch.h"
#include "hal.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level HAL driver initialization.
 *
 * @notapi
 */
void hal_lld_init(void) {  
  
#if defined(HS_DMA_REQUIRED)
  dmaInit();
#endif

  cpmEnableBTPHY();

  /* Programmable voltage detector enable.*/
}

/**
 * @brief  Setup the microcontroller system
 *         Power on all RAM and misc before setup stack.
 * @note   This function should be used only after reset.
 *
 * @notapi
 */
void SystemInit(void) {
#if 0
  HS_PMU->SCRATCH1 = 0;
  HS_PMU->SCRATCH1 = (HS_PMU->RAM_PM_CON[0]&0x1ffff) | ((HS_PMU->REMAP&0x7fff)<<17);
  /* power on RAM if its power status is off */
  if (0 == (HS_PMU->RAM_PM_CON[0] & (1 << 16))) {
    /* PSO is off when enter deep sleep, so set it on before update PMU registers */
    HS_PMU->PSO_PM_CON    = ((HS_PMU->PSO_PM_CON)   &0xfffffffc) | PMU_POWER_ON;
    HS_PMU->RAM_PM_CON[0] = ((HS_PMU->RAM_PM_CON[0])&0xfffffffc) | PMU_POWER_ON;
    HS_PMU->RAM_PM_CON[1] = ((HS_PMU->RAM_PM_CON[1])&0xfffffffc) | PMU_POWER_ON;
    HS_PMU->RAM_PM_CON[2] = ((HS_PMU->RAM_PM_CON[2])&0xfffffffc) | PMU_POWER_ON;
    HS_PMU->RAM_PM_CON[3] = ((HS_PMU->RAM_PM_CON[3])&0xfffffffc) | PMU_POWER_ON;
    HS_PMU->RAM_PM_CON[4] = ((HS_PMU->RAM_PM_CON[4])&0xfffffffc) | PMU_POWER_ON;
    HS_PMU->RAM_PM_CON[5] = ((HS_PMU->RAM_PM_CON[5])&0xfffffffc) | PMU_POWER_ON;
    HS_PMU->RAM_PM_CON[6] = ((HS_PMU->RAM_PM_CON[6])&0xfffffffc) | PMU_POWER_ON;
    HS_PMU->RAM_PM_CON[7] = ((HS_PMU->RAM_PM_CON[7])&0xfffffffc) | PMU_POWER_ON;
    HS_PMU->RAM_PM_CON[8] = ((HS_PMU->RAM_PM_CON[8])&0xfffffffc) | PMU_POWER_ON;
    HS_PMU->RAM_PM_CON[9] = ((HS_PMU->RAM_PM_CON[9])&0xfffffffc) | PMU_POWER_ON;
    /* update */
    HS_PMU->BASIC |= (1 << 24);

    /* wait ram power-on complete */
    while (!(HS_PMU->RAM_PM_CON[0] & HS_PMU->RAM_PM_CON[1] & HS_PMU->RAM_PM_CON[2] 
     & HS_PMU->RAM_PM_CON[3] & HS_PMU->RAM_PM_CON[4] & HS_PMU->RAM_PM_CON[5] 
           & HS_PMU->RAM_PM_CON[6] & HS_PMU->RAM_PM_CON[7] & HS_PMU->RAM_PM_CON[8] 
     & HS_PMU->RAM_PM_CON[9] & 0x10000));

  }
#endif
  cpmEnableMisc();
  cpmResetBTPHY();

  /* 32 nops delay without variable in stack */
  __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
  __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
  __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
  __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
}

/** @} */
