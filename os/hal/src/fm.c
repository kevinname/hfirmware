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
   Concepts and parts of this file have been contributed by Uladzimir Pylinsky
   aka barthess.
 */

/**
 * @file    fm.c
 * @brief   FM Driver code.
 *
 * @addtogroup FM
 * @{
 */
#include "ch.h"
#include "hal.h"

#if HAL_USE_FM || HAL_USE_EXTERNAL_FM || defined(__DOXYGEN__)

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
 * @brief   FM Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void fmInit(void) {
  fm_lld_init();
}

/**
 * @brief   Initializes the standard part of a @p FMDriver structure.
 *
 * @param[out] fmp     pointer to the @p FMDriver object
 *
 * @init
 */
void fmObjectInit(FMDriver *fmp) {

  fmp->state  = FM_STOP;
  fmp->config = NULL;

  /* a temporary mutual exclusion in each original API;
     fmAcquireBus()/fmReleaseBus will be a better choice. */
  osalMutexObjectInit(&fmp->mutex);

#if defined(FM_DRIVER_EXT_INIT_HOOK)
  FM_DRIVER_EXT_INIT_HOOK(fmp);
#endif
}

/**
 * @brief   Configures and activates the FM peripheral.
 *
 * @param[in] fmp      pointer to the @p FMDriver object
 * @param[in] config    pointer to the @p FMConfig object
 *
 * @api
 */
int fmStart(FMDriver *fmp, const FMConfig *config) {

  chDbgCheck((fmp != NULL) && (config != NULL));
  if (fmp == NULL) {
    return FMD_ERROR_NULLPTR;
  }

  //chSysLock();
  chDbgAssert((fmp->state == FM_STOP) || (fmp->state == FM_READY) ||
              (fmp->state == FM_LOCKED),
              "fmStart(), #1");
  if ((fmp->state != FM_STOP) && (fmp->state != FM_READY) &&
      (fmp->state != FM_LOCKED)) {
    //chSysUnlock();
    return FMD_ERROR_STATE;
  }

  fmp->config = config;
  fm_lld_start(fmp);

  fmp->state = FM_READY;
  //chSysUnlock();
  return 0;
}

/**
 * @brief   Deactivates the FM peripheral.
 *
 * @param[in] fmp      pointer to the @p FMDriver object
 *
 * @api
 */
int fmStop(FMDriver *fmp) {

  chDbgCheck(fmp != NULL);
  if (fmp == NULL) {
    return FMD_ERROR_NULLPTR;
  }

  //chSysLock();
  chDbgAssert((fmp->state == FM_STOP) || (fmp->state == FM_READY) ||
              (fmp->state == FM_LOCKED),
              "fmStop(), #1");
  if ((fmp->state != FM_STOP) && (fmp->state != FM_READY) &&
      (fmp->state != FM_LOCKED)) {
    //chSysUnlock();
    return FMD_ERROR_STATE;
  }

  fm_lld_stop(fmp);

  fmp->state = FM_STOP;
  //chSysUnlock();
  return 0;
}


/**
 * @brief   set FM frequency.
 *
 * @param[in] fmp      pointer to the @p FMDriver object
 * @param[in] frequency   fm frequency
 ¡Á
 * @notapi
 */
int fmSetFrequency(FMDriver *fmp, int frequency){
  chDbgCheck(fmp != NULL);

  osalMutexLock(&fmp->mutex);
  fm_lld_set_freq(fmp, frequency);
  osalMutexUnlock(&fmp->mutex);
  return 0;
}

/**
 * @brief   get FM frequency.
 *
 * @param[in] fmp      pointer to the @p FMDriver object
 *
 * @notapi
 */
int fmGetFrequency(FMDriver *fmp){
  int frequency;
  chDbgCheck(fmp != NULL);

  osalMutexLock(&fmp->mutex);
  frequency = fm_lld_get_freq(fmp);
  osalMutexUnlock(&fmp->mutex);
  return frequency;
}

/**
 * @brief   get FM signal strength
 *
 * @param[in] fmp      pointer to the @p FMDriver object
 ¡Á
 * @notapi
 */
int8_t fmGetRssi(FMDriver *fmp){
  chDbgCheck(fmp != NULL);

  return fm_lld_get_rssi(fmp);
}

/**
 * @brief   get FM signal noise ratio
 *
 * @param[in] fmp      pointer to the @p FMDriver object
 ¡Á
 * @notapi
 */
int8_t fmGetSnr(FMDriver *fmp){
  chDbgCheck(fmp != NULL);

  return fm_lld_get_snr(fmp);
}

/**
 * @brief   scan fm fm.
 *
 * @param[in] fmp      pointer to the @p FMDriver object
 *
 * @notapi
 */
int fmScanNext(FMDriver *fmp)
{
  int frequency;
  chDbgCheck(fmp != NULL);

  osalMutexLock(&fmp->mutex);
  frequency = fm_lld_scan_next(fmp);
  osalMutexUnlock(&fmp->mutex);
  return frequency;
}

/**
 * @brief   scan fm fm.
 *
 * @param[in] fmp      pointer to the @p FMDriver object
 *
 * @notapi
 */
int fmScanPrev(FMDriver *fmp)
{
  int frequency;
  chDbgCheck(fmp != NULL);

  osalMutexLock(&fmp->mutex);
  frequency = fm_lld_scan_perv(fmp);
  osalMutexUnlock(&fmp->mutex);
  return frequency;
}

/**
 * @brief   get FM phy threshold after scan a channel.
 *
 * @param[in] fmp      pointer to the @p FMDriver object
 *
 * @notapi
 */
uint32_t fmGetContext(FMDriver *fmp)
{
  chDbgCheck(fmp != NULL);

  return fm_lld_get_hwctx(fmp);
}

/**
 * @brief   set FM phy threshold after set a channel.
 *
 * @param[in] fmp      pointer to the @p FMDriver object
 *
 * @notapi
 */
void fmSetContext(FMDriver *fmp, uint32_t th)
{
  chDbgCheck(fmp != NULL);

  fm_lld_set_hwctx(fmp, th);
}

#endif /* HAL_USE_FM */

/** @} */
