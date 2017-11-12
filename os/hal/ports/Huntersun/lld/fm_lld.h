/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio
                 Copyright (C) 2014 HunterSun Technologies
                 hongwei.li@huntersun.com.cn

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
 * @file    hs66xx/fm_lld.h
 * @brief   FM Driver subsystem low level driver header.
 *
 * @addtogroup FM
 * @{
 */

#ifndef _FM_LLD_H_
#define _FM_LLD_H_

#if HAL_USE_FM || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/


/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/


typedef struct {
  int8_t                    chhc_th;
  int8_t                    lrhc_th;
  int8_t                    rssi_th;
  int8_t                    rssi_th_stereo;
  int8_t                    snr_th;
  int8_t                    snr_th_stereo;
} fm_th_t;

/**
 * @brief Driver configuration structure.
 */
typedef struct {
  int                       freq_current; //kHz
  int                       freq_max;     //kHz
  int                       freq_min;     //kHz
  int                       step;         //kHz
  fm_th_t                   th;
} FMConfig;

/**
 * @brief   Type of a structure representing an FM driver.
 */
typedef struct FMDriver FMDriver;

/**
 * @brief Structure representing an fm driver.
 */
struct FMDriver {
  /**
   * @brief   Driver state.
   */
  uint32_t                  state;
  /**
   * @brief   Current configuration data.
   */
  const FMConfig            *config;
  /**
   * @brief   Error flags.
   */
  uint32_t                  errors;
  /**
   * @brief   Mutex protecting the bus.
   */
  mutex_t                   mutex;

  /* End of the mandatory fields.*/
  /**
   * @brief   Thread waiting for I/O completion.
   */
  thread_t                  *thread;

  int                       freq_current; //kHz
  int                       freq_max;     //kHz
  int                       freq_min;     //kHz
  int                       step;         //kHz

  fm_th_t                   th;
  //int8_t                    stereo_th;    //obsoleted since fm stereonew alg 2016.12.09
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/
#define fm_lld_get_errors(fmp) ((fmp)->errors)
#define MODE_FM           (1 << 0)
#define MODE_STEREO       (0 << 4)
#define MODE_MONO         (1 << 4)

#define BT_MHZ_MIN  2400  //mHZ
#define BT_MHZ_MAX  2600  //mHZ
/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(__DOXYGEN__)
#if HAL_USE_FM
extern FMDriver FMD0;
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void fm_lld_init(void);
  void fm_lld_start(FMDriver *fmp);
  void fm_lld_stop(FMDriver *fmp);
  int fm_lld_set_freq(FMDriver *fmp, int frequency);
  int fm_lld_get_freq(FMDriver *fmp);
  int8_t fm_lld_get_rssi(FMDriver *fmp);
  int8_t fm_lld_get_snr(FMDriver *fmp);
  int fm_lld_scan_next(FMDriver *fmp);
  int fm_lld_scan_perv(FMDriver *fmp);
  uint32_t fm_lld_get_hwctx(FMDriver *fmp);
  void fm_lld_set_hwctx(FMDriver *fmp, uint32_t th);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_FM */

#endif /* _FM_LLD_H_ */

/** @} */
