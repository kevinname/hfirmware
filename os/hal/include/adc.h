/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012,2013 Giovanni Di Sirio.
                 Copyright (C) 2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn

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
 * @file    adc.h
 * @brief   ADC Driver macros and structures.
 *
 * @addtogroup ADC
 * @{
 */

#ifndef _ADC_H_
#define _ADC_H_

#if HAL_USE_ADC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/
#define ADC_ERROR_NULLPTR          -10
#define ADC_ERROR_STATE            -11
/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/


/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/
#if !defined(ADC_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define ADC_USE_MUTUAL_EXCLUSION    TRUE
#endif


#if ADC_USE_MUTUAL_EXCLUSION && !CH_CFG_USE_MUTEXES && !CH_CFG_USE_SEMAPHORES
#error "ADC_USE_MUTUAL_EXCLUSION requires CH_USE_MUTEXES and/or CH_USE_SEMAPHORES"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  ADC_UNINIT = 0,                           /**< Not initialized.           */
  ADC_STOP = 1,                             /**< Stopped.                   */
  ADC_READY = 2,                            /**< Ready.                     */
  ADC_ACTIVE = 3,                           /**< Converting.                */
  ADC_DMAACTIVE = 4,                        /**< DMA Conversion .           */
  ADC_ERROR = 5                             /**< Conversion complete.       */
} adcstate_t;

#include "adc_lld.h"

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

#define adcSetTestData(val)       _adc_SetTestData(val)

#if PLATFORM_ADC_USE_ADC0 || defined(__DOXYGEN__)
#define adcGetAdjustRatio()       (ADCD0.adjust_ratio)
#else
#define adcGetAdjustRatio()       0
#endif

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void adcInit(void);
  void adcObjectInit(ADCDriver *adcp);
  int adcStart(ADCDriver *adcp, const adc_attr_t *config);
  int adcStop(ADCDriver *adcp);

  int adcSetTiming(ADCDriver *adcp, const adc_timing_t *pTiming);

  int adcAddChannel(ADCDriver *adcp, adc_channel_t chn, const adc_chn_para_t *pChnPara);
  int adcDeleteChannel(ADCDriver *adcp, adc_channel_t chn);

  uint32_t adcGetChnData(ADCDriver *adcp, adc_channel_t chn, systime_t timeout);
  
  int adcStartConversion(ADCDriver *adcp);
  int adcStartConversionI(ADCDriver *adcp);

  int adcStartConversionWithDMA(ADCDriver *adcp, uint8_t *pDmaBuf, uint32_t DmaLen);
  int adcDmaWaitForDone(ADCDriver *adcp, systime_t timeout);
  
  int adcStopConversion(ADCDriver *adcp);
  int adcStopConversionI(ADCDriver *adcp);
  
#if ADC_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
  void adcAcquireBus(ADCDriver *adcp);
  void adcReleaseBus(ADCDriver *adcp);
#endif /* ADC_USE_MUTUAL_EXCLUSION */
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_ADC */

#endif /* _ADC_H_ */

/** @} */
