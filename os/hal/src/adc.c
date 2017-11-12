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
 * @file    adc.c
 * @brief   ADC Driver code.
 *
 * @addtogroup ADC
 * @{
 */

#include "ch.h"
#include "hal.h"

#if (HAL_USE_ADC == TRUE) || defined(__DOXYGEN__)

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
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   ADC Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void adcInit(void) {

  adc_lld_init();
}

/**
 * @brief   Initializes the standard part of a @p ADCDriver structure.
 *
 * @param[out] adcp     pointer to the @p ADCDriver object
 *
 * @init
 */
void adcObjectInit(ADCDriver *adcp) {
  int i;
  
  adcp->state    = ADC_STOP;
  adcp->channel_mask = 0;
  
  for(i=0; i<ADC_CHANNEL_NUM; i++)
  {
    adcp->thread[i]   = NULL;
  }
  
#if ADC_USE_MUTUAL_EXCLUSION
#if CH_CFG_USE_MUTEXES
  chMtxObjectInit(&adcp->mutex);
#else
  chSemObjectInit(&adcp->semaphore, 1);
#endif
#endif /* ADC_USE_MUTUAL_EXCLUSION */
}

/**
 * @brief   Configures and activates the ADC peripheral.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 * @param[in] config    pointer to the @p ADCConfig object. Depending on
 *                      the implementation the value can be @p NULL.
 *
 * @api
 */
int adcStart(ADCDriver *adcp, const adc_attr_t *config) {
  int res;

  osalDbgCheck(adcp != NULL);
  if(adcp == NULL)
  {
    return ADC_ERROR_NULLPTR;
  }

  osalSysLock();
  if((adcp->state != ADC_STOP) && (adcp->state != ADC_READY))
  {
    osalSysUnlock();
    return ADC_ERROR_STATE;
  }

  res = adc_lld_start(adcp, config);
  adcp->state = ADC_READY;
  osalSysUnlock();

  return res;
}

/**
 * @brief   Deactivates the ADC peripheral.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @api
 */
int adcStop(ADCDriver *adcp) {

  osalDbgCheck(adcp != NULL);
  if(adcp == NULL)
  {
    return ADC_ERROR_NULLPTR;
  }

  osalSysLock();
  if(adcp->state == ADC_STOP)
  {
    osalSysUnlock();
    return 0;
  }
  
  adc_lld_stop(adcp);
  adcp->state = ADC_STOP;
  osalSysUnlock();

  return 0;
}

int adcSetTiming(ADCDriver *adcp, const adc_timing_t *pTiming)
{
  int res;
  
  osalSysLock();   
  if(adcp->state < ADC_READY)
  {
    osalSysUnlock();
    return ADC_ERROR_STATE;
  }
  
  res = adc_lld_set_timing(adcp, pTiming);
  osalSysUnlock();
  
  return res;
}

int adcAddChannel(ADCDriver *adcp, adc_channel_t chn, const adc_chn_para_t *pChnPara)
{
  int res;
  
  osalSysLock();   
  if((adcp->state != ADC_READY)
    && (adcp->state != ADC_ACTIVE))
  {
    osalSysUnlock();
    return ADC_ERROR_STATE;
  }
  
  res = adc_lld_add_channel(adcp, chn, pChnPara);
  osalSysUnlock();
  
  return res;
}

int adcDeleteChannel(ADCDriver *adcp, adc_channel_t chn)
{
  int res;
  
  osalSysLock();   
  if((adcp->state != ADC_READY)
    && (adcp->state != ADC_ACTIVE))
  {
    osalSysUnlock();
    return ADC_ERROR_STATE;
  }
  
  res = adc_lld_delete_channel(adcp, chn);
  osalSysUnlock();
  
  return res;
}

uint32_t adcGetChnData(ADCDriver *adcp, adc_channel_t chn, systime_t timeout)
{
  int res;
  
  osalSysLock();   
  if(adcp->state != ADC_ACTIVE)
  {
    osalSysUnlock();
    return 0;
  }
  
  res = adc_lld_getChnData(adcp, chn, timeout);
  osalSysUnlock();
  
  return res;
}

/**
 * @brief   Starts an ADC conversion.
 * @details Starts an asynchronous conversion operation.
 * @note    The buffer is organized as a matrix of M*N elements where M is the
 *          channels number configured into the conversion group and N is the
 *          buffer depth. The samples are sequentially written into the buffer
 *          with no gaps.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 * @param[in] grpp      pointer to a @p ADCConversionGroup object
 * @param[in] samples   to convert count
 * @param[in] depth     buffer depth (matrix rows number). The buffer depth
 *                      must be one or an even number.
 *
 * @api
 */
int adcStartConversion(ADCDriver *adcp) {
  int res;
  
  osalSysLock();  
  res = adcStartConversionI(adcp);
  osalSysUnlock();

  return res;
}

/**
 * @brief   Starts an ADC conversion.
 * @details Starts an asynchronous conversion operation.
 * @post    The callbacks associated to the conversion group will be invoked
 *          on buffer fill and error events.
 * @note    The buffer is organized as a matrix of M*N elements where M is the
 *          channels number configured into the conversion group and N is the
 *          buffer depth. The samples are sequentially written into the buffer
 *          with no gaps.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 * @param[in] grpp      pointer to a @p ADCConversionGroup object
 * @param[out] samples  pointer to the samples buffer
 * @param[in] depth     buffer depth (matrix rows number). The buffer depth
 *                      must be one or an even number.
 *
 * @iclass
 */
int adcStartConversionI(ADCDriver *adcp) {

  osalDbgCheckClassI();
  osalDbgCheck((adcp != NULL));
  if(adcp == NULL)
  {
    return ADC_ERROR_NULLPTR;
  }

  if((adcp->state != ADC_READY) 
    && (adcp->state != ADC_ACTIVE))
  {
    return ADC_ERROR_STATE;
  }

  adcp->state = ADC_ACTIVE;
  return adc_lld_start_conversion(adcp);
}

int adcStartConversionWithDMA(ADCDriver *adcp, uint8_t *pDmaBuf, uint32_t DmaLen) {

  osalDbgCheck((adcp != NULL));
  if(adcp == NULL)
  {
    return ADC_ERROR_NULLPTR;
  }

  if((adcp->state != ADC_READY) 
    && (adcp->state != ADC_ACTIVE))
  {
    return ADC_ERROR_STATE;
  }

  adcp->state = ADC_DMAACTIVE;
  return adc_lld_start_conversionWithDMA(adcp, pDmaBuf, DmaLen);
}

int adcDmaWaitForDone(ADCDriver *adcp, systime_t timeout) {

  int res;
  
  osalDbgCheck((adcp != NULL));
  if(adcp == NULL)
  {
    return ADC_ERROR_NULLPTR;
  }

  if(adcp->state != ADC_DMAACTIVE)
  {
    return ADC_ERROR_STATE;
  }

  osalSysLock();  
  res = adc_lld_dmaWaitForDone(adcp, timeout);
  osalSysUnlock();

  adcp->state = ADC_READY;
  return res;
}

/**
 * @brief   Stops an ongoing conversion.
 * @details This function stops the currently ongoing conversion and returns
 *          the driver in the @p ADC_READY state. If there was no conversion
 *          being processed then the function does nothing.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @api
 */
int adcStopConversion(ADCDriver *adcp) {
  int res = -4;

  osalSysLock();  
  res = adcStopConversionI(adcp);
  osalSysUnlock();

  return res;
}

/**
 * @brief   Stops an ongoing conversion.
 * @details This function stops the currently ongoing conversion and returns
 *          the driver in the @p ADC_READY state. If there was no conversion
 *          being processed then the function does nothing.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @iclass
 */
int adcStopConversionI(ADCDriver *adcp) {
  int res = -4;
  
  osalDbgCheckClassI();
  osalDbgCheck(adcp != NULL);
  if(adcp == NULL)
  {
    return ADC_ERROR_NULLPTR;
  }

  if((adcp->state != ADC_READY)
    && (adcp->state != ADC_ACTIVE)
    && (adcp->state != ADC_DMAACTIVE))
  {
    return ADC_ERROR_STATE;
  }

  res = adc_lld_stop_conversion(adcp);
  adcp->state = ADC_READY;

  return res;
}


#if ADC_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
/**
 * @brief   Gains exclusive access to the ADC peripheral.
 * @details This function tries to gain ownership to the ADC bus, if the bus
 *          is already being used then the invoking thread is queued.
 * @pre     In order to use this function the option
 *          @p ADC_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @api
 */
void adcAcquireBus(ADCDriver *adcp) {

  osalDbgCheck(adcp != NULL);
  if(adcp == NULL)
  {
    return ;
  }

#if CH_CFG_USE_MUTEXES
  chMtxLock(&adcp->mutex);
#elif CH_CFG_USE_SEMAPHORES
  chSemWait(&adcp->semaphore);
#endif
}

/**
 * @brief   Releases exclusive access to the ADC peripheral.
 * @pre     In order to use this function the option
 *          @p ADC_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @api
 */
void adcReleaseBus(ADCDriver *adcp) {

  osalDbgCheck(adcp != NULL);
  if(adcp == NULL)
  {
    return ;
  }

#if CH_CFG_USE_MUTEXES
  chMtxUnlock(&adcp->mutex);
#elif CH_CFG_USE_SEMAPHORES
  chSemSignal(&adcp->semaphore);
#endif
}
#endif /* ADC_USE_MUTUAL_EXCLUSION */

#endif /* HAL_USE_ADC */

/** @} */
