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
 * @file    fm.h
 * @brief   FM Driver macros and structures.
 *
 * @addtogroup FM
 * @{
 */

#ifndef _FM_H_
#define _FM_H_

#if HAL_USE_FM || HAL_USE_EXTERNAL_FM || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/
#define FMD_ERROR_NULLPTR          -10
#define FMD_ERROR_STATE            -11

/**
 * @name    FM bus error conditions
 * @{
 */
#define FMD_NO_ERROR               0x00   /**< @brief No error.            */
#define FMD_BUS_ERROR              0x01   /**< @brief Bus Error.           */
#define FMD_ARBITRATION_LOST       0x02   /**< @brief Arbitration Lost.    */
#define FMD_ACK_FAILURE            0x04   /**< @brief Acknowledge Failure. */
#define FMD_OVERRUN                0x08   /**< @brief Overrun/Underrun.    */
#define FMD_PEC_ERROR              0x10   /**< @brief PEC Error in
                                                reception.                  */
#define FMD_TIMEOUT                0x20   /**< @brief Hardware timeout.    */
#define FMD_SMB_ALERT              0x40   /**< @brief SMBus Alert.         */
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   Enables the mutual exclusion APIs on the FM bus.
 */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/


/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  FM_UNINIT = 0,                           /**< Not initialized.           */
  FM_STOP = 1,                             /**< Stopped.                   */
  FM_READY = 2,                            /**< Ready.                     */
  FM_ACTIVE_TX = 3,                        /**< Transmitting.              */
  FM_ACTIVE_RX = 4,                        /**< Receiving.                 */
  FM_LOCKED = 5                            /**> Bus or driver locked.      */
} fmstate_t;

typedef enum
{
  FM_INNER = 0,
  FM_EXTERNEL,
}FM_SELECTION_T;

#include "fm_lld.h"

#if HAL_USE_EXTERNAL_FM
#include "external_fm_lld.h"
#endif
/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/



/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void fmInit(void);
  void fmObjectInit(FMDriver *fmp);
  int  fmStart(FMDriver *fmp, const FMConfig *config);
  int  fmStop(FMDriver *fmp);
  int fmSetFrequency(FMDriver *fmp, int frequency);
  int fmGetFrequency(FMDriver *fmp);
  int fmScanPrev(FMDriver *fmp);
  int fmScanNext(FMDriver *fmp);
  int8_t fmGetRssi(FMDriver *fmp);
  int8_t fmGetSnr(FMDriver *fmp);
  uint32_t fmGetContext(FMDriver *fmp);
  void fmSetContext(FMDriver *fmp, uint32_t th);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_FM */

#endif /* _FM_H_ */

/** @} */
