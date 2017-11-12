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
 * @file    audio.h
 * @brief   audio Driver macros and structures.
 *
 * @addtogroup AUDIO
 * @{
 */

#ifndef _AUDIO_H_
#define _AUDIO_H_

#if HAL_USE_AUDIO || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/
#define DRC_FRAME_SIZE    16
/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/


/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !defined(HAL_USE_I2S) || defined(__DOXYGEN__)
#define HAL_USE_I2S                 TRUE
#endif

#if !defined(HAL_USE_CODEC) || defined(__DOXYGEN__)
#define HAL_USE_CODEC               TRUE
#endif

#if !defined(HS_AUDIO_USE_DEBUG) || defined(__DOXYGEN__)
#define HS_AUDIO_USE_DEBUG          FALSE
#endif

#if !defined(HS_AUDIO_USE_REPAIR) || defined(__DOXYGEN__)
#define HS_AUDIO_USE_REPAIR         TRUE
#endif

#if !defined(HS_I2S_USE_I2S0) || defined(__DOXYGEN__)
#define HS_I2S_USE_I2S0             TRUE
#endif

#if !defined(HS_I2S_USE_STATISTIC) || defined(__DOXYGEN__)
#define HS_I2S_USE_STATISTIC        TRUE
#endif

#if !defined(HS_CODEC_USE_DRV) || defined(__DOXYGEN__)
#define HS_CODEC_USE_DRV            TRUE
#endif

#if !defined(HS_CODEC_USE_INSIDE) || defined(__DOXYGEN__)
#define HS_CODEC_USE_INSIDE            TRUE
#endif

#if !defined(HS_CODEC_USE_WM8753) || defined(__DOXYGEN__)
#define HS_CODEC_USE_WM8753            TRUE
#endif

#if HS_AUDIO_USE_REPAIR
  #define HS_AUDIO_USE_HWREPAIR  1
  #define HS_AUDIO_USE_SWREPAIR  0

  #if HS_AUDIO_USE_HWREPAIR && HS_AUDIO_USE_SWREPAIR
  #error "Can't select the two method at the same time!"
  #endif

  #if !HS_AUDIO_USE_HWREPAIR && !HS_AUDIO_USE_SWREPAIR
  #error "Must select a method to do auto-repair!"
  #endif

  #if HS_AUDIO_USE_HWREPAIR
    #define HS_AUDIO_HWREPAIR_WORKCLK   1
    #define HS_AUDIO_HWREPAIR_WSCLK     0

    #if HS_AUDIO_HWREPAIR_WORKCLK && HS_AUDIO_HWREPAIR_WSCLK
    #error "Can't select the two method at the same time!"
    #endif

    #if !HS_AUDIO_HWREPAIR_WORKCLK && !HS_AUDIO_HWREPAIR_WSCLK
    #error "Must select a method to do auto-repair!"
    #endif
  #endif
#endif /* end of HS_AUDIO_USE_REPAIR */

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/
typedef enum
{
  AUDIO_PLY_SRC_MP3   = 0,
  AUDIO_PLY_SRC_AVRCP,
  AUDIO_PLY_SRC_HFP,
  AUDIO_PLY_SRC_USB,
  AUDIO_PLY_SRC_TONE,
  AUDIO_PLY_SRC_FM,
  AUDIO_PLY_SRC_MAX,
}hs_audio_ply_src_t;

typedef enum
{
  AUDIO_REC_SRC_LINEIN = 0,
  AUDIO_REC_SRC_xxx,
  AUDIO_REC_SRC_HFP,
  AUDIO_REC_SRC_USB,
  AUDIO_REC_SRC_RING,
  AUDIO_REC_SRC_MAX = 6,
}hs_audio_rec_src_t;

typedef enum
{
  AUDIO_AUTOREPAIR_DISABLE            = 0,
  AUDIO_AUTOREPAIR_ENABLE             ,
}hs_audio_autorepair_t;

typedef enum
{
  AUDIO_EVENT_STOPPED                  = 0,
  AUDIO_EVENT_RESUME                   ,

  AUDIO_EVENT_BULT
}hs_audio_event_t;

enum
{
  AUDIO_EVENT_NO_DEL                  = 0,
  AUDIO_EVENT_DEL                     ,
};

 typedef enum
{
  AUDIO_IDLE                      = 0,
  AUDIO_STOP                      ,
  AUDIO_READY                     ,
  AUDIO_PREPARE                   ,
  AUDIO_RUNING                    ,
  AUDIO_PAUSE                     ,
}hs_audio_state_t;

enum
{
  TRACK_LR                        = 0,
  TRACK_RL                        ,
  TRACK_LL                        ,
  TRACK_RR                        ,
};

typedef enum
{
  AUDIO_STREAM_PLAYBACK       = 0x80,
  AUDIO_STREAM_RECORD         ,
}hs_audio_streamdir_t;

#if HS_I2S_USE_STATISTIC
typedef struct
{
  uint32_t           get_buffer_cnt;
  uint32_t           return_buffer_cnt;
  uint32_t           return_lose_cnt;

  uint32_t           cpu_slow_cnt;
  uint32_t           i2s_slow_cnt;

  uint32_t           hw_error_cnt;
  uint32_t           hw_int_cnt;
}hs_audio_debug_t;
#endif

typedef void (*hs_audio_cbfun_t)(hs_audio_event_t enEvent);

typedef struct hsAudio_event_s  hs_audio_event_cb_t;
struct hsAudio_event_s
{
  hs_audio_cbfun_t      pfnCallback;
  hs_audio_event_cb_t  *pstNextCB;
};


typedef struct
{
  /* hardware params */
  uint32_t           dma_start;
  uint32_t           dma_end;
  uint32_t           buffer_size;
  uint32_t           period_size;

  uint32_t           frame_bits;
  uint32_t           frame_t_us;
  uint32_t           min_aglin;

  uint32_t           hw_ptr;

  /* software params */
  uint32_t           start_threshold;
  uint32_t           boundary;
  uint32_t           app_ptr;
  uint32_t           min_oper_len;

  uint32_t           silence_filled;
  uint32_t           prompt_dis;
#if 0
  int32_t            gain[DRC_FRAME_SIZE];
#endif

  hs_audio_streamdir_t    dir;
  hs_audio_state_t        state;
  hs_audio_autorepair_t   auto_repair;

  /* performance debug params */
  #if HS_I2S_USE_STATISTIC
  hs_audio_debug_t      performance;
  #endif

  /* locking & scheduling */
  thread_t                *thread;
  mutex_t                 mutex;
}hs_audio_stream_t;

#include "i2s_lld.h"
typedef hs_i2s_config_t hs_audio_config_t;
#include "codec_lld.h"

typedef struct
{
  CODECDriver       *pcodec;
  I2SDriver         *pi2s;
  hs_audio_config_t *pcfg;

  hs_audio_stream_t   rec;
  hs_audio_stream_t   ply;

  hs_audio_event_cb_t *rx_event;
  hs_audio_event_cb_t *tx_event;
}hs_audio_t;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/
#if HS_AUDIO_USE_DEBUG && CH_DBG_ENABLE_PRINTF
#include <stdio.h>
#define audio_dbg(fmt,args...)	printf(fmt, ##args)
#else
#define audio_dbg(fmt,args...)
#endif

#if HS_I2S_USE_STATISTIC && CH_DBG_ENABLE_PRINTF
#include <stdio.h>
#define audio_stc(fmt,args...)	printf(fmt, ##args)
#else
#define audio_stc(fmt,args...)
#endif

#define __audio_wait_for_interrupt(stmp, time, vt)    \
do                                                    \
{                                                     \
  chSysLock();                                        \
  stmp->thread = chThdGetSelfX();                     \
  chSchGoSleepS(CH_STATE_SUSPENDED);                  \
  if ((time != TIME_INFINITE) && chVTIsArmedI(&vt))   \
    chVTResetI(&vt);                                  \
  chSysUnlock();                                      \
}while(0)

#define __audio_nowait_reset(time, vt)                \
do                                                    \
{                                                     \
  if ((time != TIME_INFINITE) && chVTIsArmed(&vt))   \
    chVTReset(&vt);                                  \
}while(0)

#define __audio_wakeup(stmp)                          \
do                                                    \
{                                                     \
  if (stmp->thread)                                   \
  {                                                   \
    thread_t *tp = stmp->thread;                      \
    stmp->thread = NULL;                              \
    tp->p_u.rdymsg = MSG_TIMEOUT;                     \
    chSchReadyI(tp);                                  \
  }                                                   \
}while(0)                                             \

typedef short (*pfnAlgAec_t)(short , short );
typedef void  (*pfnAlgAns_t)(short *, short *);
typedef void  (*pfnAlgInit_t)(void);
typedef void  (*pfnAlgUninit_t)(void);


/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/
#define audioGetCodecStatus()       codec_lld_getStatus(&CODECD)
#define audioSetPgaGain(gain_idx)   codec_analog_set_pga_gain(gain_idx)
#define audioSetMixerEn(volume)     codec_lld_setMixer(volume)
#define audioMixerIsEn()            codec_lld_isMixerEn()

#ifdef __cplusplus
extern "C" {
#endif

void audio_update_hw_ptr(hs_audio_stream_t *stmp, uint8_t int_flag);

void audioInit(void);
void audioStart(void);
void audioStop(void);

int32_t audioAecStart(pfnAlgInit_t pfnAecInit, pfnAlgAec_t pfnAecCal, pfnAlgUninit_t pfnAecUninit);
int32_t audioAnsStart(pfnAlgInit_t pfnAnsInit, pfnAlgAns_t pfnAnsCal, pfnAlgUninit_t pfnAnsUninit);
void audioAecStop(void);
void audioAnsStop(void);

int32_t audioRecordStart(hs_audio_config_t *cfgp, hs_audio_cbfun_t event_fun);
void audioRecordStop(void);

/*
 * @param[in] start_threshold: the point of sample to start to play
 */
int32_t audioPlayStart(hs_audio_config_t *cfgp, uint32_t start_threshold, hs_audio_cbfun_t event_fun);
void audioPlayStop(void);

hs_audio_stream_t * audioGetStream(hs_audio_streamdir_t dir);

/*
 * @brief               get buffer have been saved recording data
 *                      when have not enough data, this function will be block
 * @param[out] ppData   recording data buffer pointer
 * @param[in] size      length to be read
 * @param[in] time      the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The real length can be read.
 */
uint32_t audioRecGetDataBuffer(uint8_t **ppData, uint32_t size, systime_t time);

/*
 * @brief               get saved recording data from buffer done
 *
 * @param[in] pData     recording data buffer pointer
 * @param[in] size      length have been read
 *                      .
 *
 */
void audioRecGetDataDone(uint8_t *pData, uint32_t size);

/*
 * @brief               get buffer to be written for playing data
 *                      when have not enough buffer, this function will be block
 * @param[out] ppData   playing data buffer pointer
 * @param[in] size      length to be read
 * @param[in] time      the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The real length can be written.
 */
uint32_t audioPlyGetDataBuffer(uint8_t **ppData, uint32_t size, systime_t time);

/*
 * @brief               save the data to playing buffer done
 *
 * @param[in] pData     playing data buffer pointer
 * @param[in] size      length have been write
 *                      .
 *
 */
void audioPlySendDataDone(uint8_t *pData, uint32_t size);

/*
 * return the point of data to be playing in buffer
 */
uint32_t audioGetBufferLevel(void);

/*
 * @brief               Enable or disable to auto-repair
 *                      when sample rate different to the source.
 *                      This function can be called behind of audioPlayStart()
 *
 * @param[in] auto_r    AUDIO_AUTOREPAIR_DISABLE - disable
 *                      AUDIO_AUTOREPAIR_ENABLE  - enable
 */
void audioPlySetAutoRepair(hs_audio_autorepair_t auto_r);

void hs_audio_autoRepair(hs_audio_stream_t *stmp);
void audioPlayAcquire(void);
void audioPlayRelease(void);

/* rc calibration */
void audioRxAdcCalibration();
void audioAuAdcCalibration();
void audioTxDacCalibration();
void audioRxFilterCalibration();
void audioRxTiaCalibration();
void audioMicbiasCalibration();

/* audio control function */
void audioPlayPause(void);
void audioPlayResume(void);

void audioRecordSetVolume(int db);
int audioRecordGetVolumeMin(hs_audio_rec_src_t src);
int audioRecordGetVolumeMax(hs_audio_rec_src_t src);
void audioRecordMute(void);
void audioRecordUnmute(void);
void audioPlaySetVolume(int db);
void audioPlaySetVolumeLeft(int db);
void audioPlaySetVolumeRight(int db);
int audioPlayGetVolumeMax(hs_audio_ply_src_t src);
int audioPlayGetVolumeMin(hs_audio_ply_src_t src);
void audioPlayMute(void);
void audioPlayUnmute(void);
void audioSetCodecSel(int sel);
void audioSetRecordSource(int source);
void audioSetPlaySource(int source);
void audioSetPlaySample(int sample);
void audioSetRecordSample(int sample);
void audioSetShortFir(int enable);
void audioSetAdcDrcMode(int mode);
void audioSetAdcDrcLimiter(int limiter);
void audioSetAdcDrcAgc(int limiter);
void audioSetDacDrcMode(int mode);
void audioSetDacDrcLimiter(int limiter);
void audioSetDacDrcAgc(int limiter);
void audioSetAdcMix(int enable);
void audioSetDacMix(int enable);
void audioSetDacMode(int mode);
void audioSetRecMode(int mode);
void audioSetPlyMode(int mode);
void audioInvertI2sInput(int sel);
void audioInvertI2sOutput(int sel);
void audioSetEqEnable(int enable);
void audioSetBand1Coeff(uint16_t coeff);
void audioSetBand1Gain(uint8_t gain);
void audioSetBand2Coeff(uint32_t coeff);
void audioSetBand2Gain(uint8_t gain);
void audioSetBand3Coeff(uint32_t coeff);
void audioSetBand3Gain(uint8_t gain);
void audioSetBand4Coeff(uint32_t coeff);
void audioSetBand4Gain(uint8_t gain);
void audioSetBand5Coeff(uint32_t coeff);
void audioSetBand5Gain(uint8_t gain);
void audioSetBand6Coeff(uint32_t coeff);
void audioSetBand6Gain(uint8_t gain);
void audioSetBand7Coeff(uint16_t coeff);
void audioSetBand7Gain(uint8_t gain);
void audioSetTestMode(uint8_t mode);
void audioSetI2sConnCtrl(uint8_t enable);
int audioGetDacRms();
int audioGetAecDelay();
void audioAecDelayMeasureInit(int threshold);
void audioSetDacPeakDetect(uint8_t enable);
void audioSetBTCfg(void);
void audioPlyCpyData(uint8_t *srcData, uint8_t *dstData, uint32_t len);
uint32_t audioGetPlaySampleRate(void);
void audioPlayPromptDisable(void);


#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_AUDIO */

#endif /* _AUDIO_H_ */

/** @} */
