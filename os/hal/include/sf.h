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
 * @file    sf.h
 * @brief   SPI flash interface Driver macros and structures.
 *
 * @addtogroup SF
 * @{
 */

#ifndef _SF_H_
#define _SF_H_

#if HAL_USE_SF || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/
#define SF_RES_ERROR_STATE            -10
#define SF_RES_ERROR_NULLPTR          -11
/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    SF configuration options
 * @{
 */
/**
 * @brief   Enables the @p sfAcquireBus() and @p sfReleaseBus() APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(SF_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define SF_USE_MUTUAL_EXCLUSION    TRUE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if SF_USE_MUTUAL_EXCLUSION && !CH_CFG_USE_MUTEXES && !CH_CFG_USE_SEMAPHORES
#error "SF_USE_MUTUAL_EXCLUSION requires CH_USE_MUTEXES and/or CH_USE_SEMAPHORES"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  SF_UNINIT = 0,                   /**< Not initialized.                   */
  SF_STOP   = 1,                   /**< Stopped.                           */
  SF_READY  = 2,                   /**< Ready.                             */
  SF_FOUND  = 3,                   /**< flash have found                   */
  SF_WORKING= 4,                   /**< spi flash interface working        */
} sfstate_t;

#include "sf_lld.h"

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

  void sfInit(void);
  void sfObjectInit(SFDriver *sfp);
  int sfStart(SFDriver *sfp, const SFConfig *config);
  int sfStop(SFDriver *sfp);
  int sfProbe(SFDriver *sfp, systime_t to);
  int sfErase(SFDriver *sfp, uint32_t offset, size_t len, systime_t to);
  int sfRead(SFDriver *sfp, uint32_t offset, void *rdbuf, size_t len, systime_t to);
  int sfWrite(SFDriver *sfp, uint32_t offset, void *wrbuf, size_t len, systime_t to);

  uint32_t sfReadStatus(SFDriver *sfp, systime_t to);
  int sfQuadEn(SFDriver *sfp, sfqe_t qe, systime_t to);
  
  int sfDeepPD(SFDriver *sfp, systime_t to);
  int sfReleasePD(SFDriver *sfp, systime_t to);

#if SF_USE_MUTUAL_EXCLUSION
  void sfAcquireBus(SFDriver *sfp);
  void sfReleaseBus(SFDriver *sfp);
#endif /* SF_USE_MUTUAL_EXCLUSION */
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SF */

#endif /* _SF_H_ */

/** @} */
