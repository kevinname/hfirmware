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
 * @file    audio.c
 * @brief   audio system Driver code.
 *
 * @addtogroup audio
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "string.h"
#include "chprintf.h"


#if HAL_USE_AUDIO || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

static hs_audio_t g_audio_info;

static hs_audio_config_t g_stDefaultCfg =
{
  I2S_SAMPLE_48K,
  I2S_BITWIDTH_16BIT,
  I2S_BITWIDTH_32BIT,
  I2S_WORKMODE_MASTER,
  I2S_PCMMODE_STEREO,

  I2S_PLY_BLOCK_SIZE / 2,
  I2S_PLY_BLOCK_NUM,
};

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

void hs_audio_algInit(void);
int  hs_audio_algAecStart(hs_audio_t *pstAudio, pfnAlgInit_t pfnAecInit, 
                    pfnAlgAec_t pfnAecCal, pfnAlgUninit_t pfnAecUninit);
void hs_audio_algAecStop(void);
int  hs_audio_algAecSaveTxData(uint8_t *pbuf, uint32_t len);
int  hs_audio_algAecDoData(uint8_t *pbuf, uint32_t len);

int  hs_audio_algAnsStart(hs_audio_t *pstAudio, pfnAlgInit_t pfnAnsInit, 
                    pfnAlgAns_t pfnAnsCal, pfnAlgUninit_t  pfnAnsUninit);
void hs_audio_algAnsStop(void);
int  hs_audio_algAnsDoData(uint8_t *pbuf, uint32_t len);
long hs_audio_calDrc(int16_t xin);


#define REPAIR_POINT_NUM          60

#define __audio_rp_GetThrowData(value, i, nextValue)  \
  ((value * (REPAIR_POINT_NUM - (i/2 + 1)) + nextValue * (i/2 + 1)) / REPAIR_POINT_NUM)

#define __audio_rp_GetIncreaseData(value, i, lstValue)  \
  ((value * (REPAIR_POINT_NUM - (i/2 + 1)) + lstValue * (i/2 + 1)) / REPAIR_POINT_NUM)


/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
uint32_t _audio_frame2byte(hs_audio_stream_t *stmp, uint32_t frame)
{
  return frame * stmp->frame_bits / 8;
}

uint32_t _audio_byte2frame(hs_audio_stream_t *stmp, uint32_t byte)
{
  return byte * 8 / stmp->frame_bits;
}

void _audio_rp_swIncreasePoint(hs_audio_stream_t *stmp)
{
  int16_t i, j, *ptr;
  int32_t temp, last_left, last_right;  
  
  ptr = (int16_t *)(stmp->dma_start + _audio_frame2byte(stmp, (stmp->app_ptr - REPAIR_POINT_NUM)));

  last_left = ptr[0];
  last_right = ptr[1];
  for(i=1; i<REPAIR_POINT_NUM; i++)
  {
    if(stmp->frame_bits == 16)
    {
      j = 2 * i;
      temp = ptr[i];
      ptr[i] = __audio_rp_GetIncreaseData(temp, j, last_left);
      last_left = temp;
    }
    else
    {
      j = 2 * i;
      temp = ptr[j];
      ptr[j] = __audio_rp_GetIncreaseData(temp, j, last_left);
      last_left = temp;

      j += 1;
      temp = ptr[j];
      ptr[j] = __audio_rp_GetIncreaseData(temp, j, last_right);
      last_right = temp;
    }
  }

  if(stmp->frame_bits == 16)
  {
    ptr[REPAIR_POINT_NUM] = last_left;
  }
  else
  {
    ptr[REPAIR_POINT_NUM*2] = last_left;
    ptr[REPAIR_POINT_NUM*2 + 1] = last_right;
  }  
  
  stmp->app_ptr += 1;
  if(stmp->app_ptr >= stmp->boundary)
    stmp->app_ptr -= stmp->boundary;
}

void _audio_get_pointer(hs_audio_stream_t *stmp, uint32_t *src, uint32_t *dst)
{
  if(src != NULL)
  {
    if(stmp->dir == AUDIO_STREAM_PLAYBACK)
      dmaGetCurSrcAddr(g_audio_info.pi2s->pdmatx, (*src));
    else
      dmaGetCurSrcAddr(g_audio_info.pi2s->pdmarx, (*src));
  }

  if(dst != NULL)
  {
    if(stmp->dir == AUDIO_STREAM_PLAYBACK)
      dmaGetCurDstAddr(g_audio_info.pi2s->pdmatx, (*dst));
    else
      dmaGetCurDstAddr(g_audio_info.pi2s->pdmarx, (*dst));
  }
}

void _audio_keep_silence(hs_audio_stream_t *stmp, uint32_t new_hw_ptr)
{
  uint8_t *ptr;

  ptr = (uint8_t *)stmp->dma_start + _audio_frame2byte(stmp, stmp->hw_ptr);
  if(new_hw_ptr > stmp->hw_ptr)
  {
    memset(ptr, stmp->silence_filled, _audio_frame2byte(stmp, new_hw_ptr - stmp->hw_ptr));
  }
  else
  {
    memset(ptr, stmp->silence_filled, _audio_frame2byte(stmp, stmp->boundary - stmp->hw_ptr));

    ptr = (uint8_t *)stmp->dma_start;
    memset(ptr, stmp->silence_filled, _audio_frame2byte(stmp, new_hw_ptr));
  }
}

uint32_t _audio_play_avail(hs_audio_stream_t *stmp)
{
  uint32_t frame;

  if(stmp->app_ptr >= stmp->hw_ptr)
    frame = stmp->app_ptr - stmp->hw_ptr;
  else
    frame = stmp->boundary + stmp->app_ptr - stmp->hw_ptr;

  return frame;
}

int32_t _audio_play_inavail(hs_audio_stream_t *stmp)
{
  return stmp->boundary - _audio_play_avail(stmp) - 1;
}

int32_t _audio_record_avail(hs_audio_stream_t *stmp)
{
  uint32_t frame;

  if(stmp->hw_ptr >= stmp->app_ptr)
    frame = stmp->hw_ptr - stmp->app_ptr;
  else
    frame = stmp->boundary + stmp->hw_ptr - stmp->app_ptr;

  return frame;
}

static void _audio_wakeup(void *p)
{
  hs_audio_stream_t *stmp = (hs_audio_stream_t *)p;

  chSysLockFromISR();
  __audio_wakeup(stmp);
  chSysUnlockFromISR();
}

static int32_t _audio_new_event(hs_audio_event_cb_t **ppEvent)
{
  hs_audio_event_cb_t *pTmpEvent;

  pTmpEvent = (hs_audio_event_cb_t *)chHeapAlloc(NULL, sizeof(hs_audio_event_cb_t));
  if(pTmpEvent == NULL)
  {
    return -2;
  }
  
  pTmpEvent->pfnCallback = NULL;
  pTmpEvent->pstNextCB = *ppEvent;
  *ppEvent = pTmpEvent;

  return 0;
}

static void _audio_do_event(hs_audio_stream_t *stmp, hs_audio_event_cb_t **ppEvent, hs_audio_event_t event)
{
  hs_audio_event_cb_t *TmpEvent;

  if(*ppEvent != NULL)
  {
    if((event == AUDIO_EVENT_STOPPED) && ((*ppEvent)->pfnCallback != NULL))
      (*ppEvent)->pfnCallback(event);

    if(event == AUDIO_EVENT_RESUME)
    {
      hs_audio_event_cb_t *this = *ppEvent;

      if((*ppEvent)->pstNextCB != NULL)
      {
        TmpEvent = (*ppEvent)->pstNextCB;
        /*
         * if the last thread who used audio not register event,
         * check the last of the last thread. loop until found
         * the event of a thread register.
         */
        while((TmpEvent->pfnCallback == NULL) && (TmpEvent->pstNextCB != NULL))
        {
          chHeapFree((void *)TmpEvent);

          TmpEvent = TmpEvent->pstNextCB;
        };

        *ppEvent = TmpEvent->pstNextCB;
      }
      else
      {
        TmpEvent = NULL;
        *ppEvent = NULL;
      }

      chHeapFree((void *)this);

      if(TmpEvent != NULL)
      {
        if(TmpEvent->pfnCallback != NULL)
        {
          chMtxUnlock(&stmp->mutex);
          TmpEvent->pfnCallback(event);
          chMtxLock(&stmp->mutex);
        }

        chHeapFree((void *)TmpEvent);
      }
    }
  }
}

void _audio_rec_stop(void)
{
  hs_audio_stream_t *stmp = &g_audio_info.rec;

  stmp->state = AUDIO_READY;
  i2s_lld_record_stop(g_audio_info.pi2s);
  codec_lld_record_stop(g_audio_info.pcodec);

  chSysLock();
  __audio_wakeup(stmp);
  chSysUnlock();
}

void _audio_ply_stop(void)
{
  hs_audio_stream_t *stmp = &g_audio_info.ply;
  uint32_t timeout = 10;

  stmp->state = AUDIO_READY;
  if(stmp->app_ptr < stmp->period_size)
    stmp->app_ptr += stmp->period_size;
  
  while((stmp->app_ptr <= stmp->hw_ptr) && (timeout--))
    audio_update_hw_ptr(stmp, 0);

  while(1)
  {
    audio_update_hw_ptr(stmp, 0);
    if((stmp->app_ptr <= stmp->hw_ptr) || (timeout++ > 5))
    {
      chSysLock();
      __audio_wakeup(stmp);
      chSysUnlock();

      dmaStreamDisable(g_audio_info.pi2s->pdmatx);
      stmp->app_ptr = stmp->hw_ptr = 0;
      break;
    }
  }

  codec_lld_play_stop(g_audio_info.pcodec);
  i2s_lld_play_stop(g_audio_info.pi2s);
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/
void audio_update_hw_ptr(hs_audio_stream_t *stmp, uint8_t int_flag)
{
  uint32_t pos;
  uint32_t new_hw_ptr;
  syssts_t sts;
  #if HS_I2S_USE_STATISTIC
  uint32_t new_pos;
  #endif

  (void)int_flag;
  
  sts = chSysGetStatusAndLockX();
  if(stmp->state <= AUDIO_PREPARE)
  {
    pos = 0;
  }
  else
  {
    if(stmp->dir == AUDIO_STREAM_PLAYBACK)
      _audio_get_pointer(stmp, &pos, NULL);
    else
      _audio_get_pointer(stmp, NULL, &pos);

    pos -= stmp->dma_start;
    pos -= pos % stmp->min_aglin;
  }
  new_hw_ptr = _audio_byte2frame(stmp, pos);

  if((uint32_t)new_hw_ptr > stmp->boundary)
  {

    chSysRestoreStatusX(sts);
    #if HS_I2S_USE_STATISTIC
    stmp->performance.hw_error_cnt++;
    #endif

    while(1);
    return ;
  }

  #if HS_I2S_USE_STATISTIC
  if(int_flag)
  {
    stmp->performance.hw_int_cnt++;
  }
  #endif

  if((stmp->dir == AUDIO_STREAM_PLAYBACK) && ((uint32_t)new_hw_ptr != stmp->hw_ptr))
    _audio_keep_silence(stmp, new_hw_ptr);

  #if HS_I2S_USE_STATISTIC
  if((stmp->state != AUDIO_PAUSE) && (stmp->dir == AUDIO_STREAM_PLAYBACK))
  {
    if(new_hw_ptr >= stmp->hw_ptr)
    {
      if ((new_hw_ptr > stmp->app_ptr) && (stmp->app_ptr > stmp->hw_ptr))
      {
        stmp->performance.cpu_slow_cnt ++;
      }
    }
    else
    {
      new_pos = new_hw_ptr + stmp->boundary;
      if((new_pos > stmp->app_ptr) && (stmp->app_ptr > stmp->hw_ptr))
      {
        stmp->performance.cpu_slow_cnt ++;
      }
    }
  }

  if(stmp->dir == AUDIO_STREAM_RECORD)
  {
    if((stmp->app_ptr < new_hw_ptr) && (new_hw_ptr < stmp->hw_ptr))
    {
      stmp->performance.cpu_slow_cnt ++;
    }

    if((new_hw_ptr > stmp->app_ptr) && (stmp->app_ptr > stmp->hw_ptr))
    {
      stmp->performance.cpu_slow_cnt ++;
    }
  }
  #endif

  stmp->hw_ptr = new_hw_ptr;
  chSysRestoreStatusX(sts);
}

/**
 * @brief   audio Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void audioInit(void)
{
  i2s_lld_init();
  codec_lld_init();

  g_audio_info.pcodec = &CODECD;
  g_audio_info.pi2s = &I2SD;

  g_audio_info.rec.state = AUDIO_STOP;
  g_audio_info.ply.state = AUDIO_STOP;

  g_audio_info.rx_event = NULL;
  g_audio_info.tx_event = NULL;

  chMtxObjectInit(&g_audio_info.ply.mutex);
  chMtxObjectInit(&g_audio_info.rec.mutex);
}

/**
 * @brief   Configures and activates the audio system.
 *
 * @param[in] config    pointer to the @p hs_audio_config_t object
 *
 * @api
 */
void audioStart(void)
{
  if((g_audio_info.rec.state != AUDIO_STOP) || (g_audio_info.ply.state != AUDIO_STOP))
  {
    return ;
  }

  g_audio_info.rec.state = AUDIO_READY;
  g_audio_info.ply.state = AUDIO_READY;

  i2s_lld_start(g_audio_info.pi2s);
  codec_lld_start(g_audio_info.pcodec);
  hs_audio_algInit();
}

/**
 * @brief Deactivates the audio.
 * @note  Deactivating the peripheral also enforces a release of the slave
 *        select line.
 *
 *
 * @api
 */
void audioStop(void)
{
  i2s_lld_stop(g_audio_info.pi2s);
  codec_lld_stop(g_audio_info.pcodec);

  g_audio_info.rec.state = AUDIO_STOP;
  g_audio_info.ply.state = AUDIO_STOP;
}

int32_t audioAecStart(pfnAlgInit_t pfnAecInit, pfnAlgAec_t pfnAecCal, pfnAlgUninit_t pfnAecUninit)
{
  return hs_audio_algAecStart(&g_audio_info, pfnAecInit, pfnAecCal, pfnAecUninit);
}

int32_t audioAnsStart(pfnAlgInit_t pfnAnsInit, pfnAlgAns_t pfnAnsCal, pfnAlgUninit_t pfnAnsUninit)
{
  return hs_audio_algAnsStart(&g_audio_info, pfnAnsInit, pfnAnsCal, pfnAnsUninit);
}

void audioAecStop(void)
{
  hs_audio_algAecStop();
}

void audioAnsStop(void)
{
  hs_audio_algAnsStop();
}

int32_t audioRecordStart(hs_audio_config_t *cfgp, hs_audio_cbfun_t event_fun)
{
  hs_audio_stream_t *stmp = &g_audio_info.rec;
  hs_audio_config_t *pTmpCfg = cfgp;
  int32_t s32Res = -1;

  chMtxLock(&stmp->mutex);
  if(stmp->state != AUDIO_READY)
  {
    if(stmp->state == AUDIO_RUNING)
    {
      _audio_do_event(stmp, &g_audio_info.rx_event, AUDIO_EVENT_STOPPED);
      _audio_rec_stop();
    }
    else
    {
      chMtxUnlock(&stmp->mutex);
      return -1;
    }
  }

  if(cfgp == NULL)
  {
    pTmpCfg = &g_stDefaultCfg;
  }

  codec_lld_record_start(g_audio_info.pcodec, pTmpCfg);
  s32Res = i2s_lld_record_start(g_audio_info.pi2s, pTmpCfg, &g_audio_info.rec);
  if(s32Res != 0)
  {
    codec_lld_record_stop(g_audio_info.pcodec);
    chMtxUnlock(&stmp->mutex);
    return s32Res;
  }

  s32Res = _audio_new_event(&g_audio_info.rx_event);
  if(s32Res != 0)
  {
    _audio_rec_stop();
    chMtxUnlock(&stmp->mutex);
    return s32Res;
  }

  stmp->state = AUDIO_RUNING;
  g_audio_info.rx_event->pfnCallback = event_fun;
  chMtxUnlock(&stmp->mutex);

  return 0;
}
void audioRecordStop(void)
{
  hs_audio_stream_t *stmp = &g_audio_info.rec;
  hs_audio_state_t state = stmp->state;

  (void)state;
  chMtxLock(&stmp->mutex);
  _audio_rec_stop();
  _audio_do_event(stmp, &g_audio_info.rx_event, AUDIO_EVENT_RESUME);

  hs_audio_algAecStop();
  hs_audio_algAnsStop();
  chMtxUnlock(&stmp->mutex);
  
  #if HS_I2S_USE_STATISTIC
  if(state == AUDIO_RUNING)
  {
    audio_stc("\r\n[rec]cpu get buffer count: %d"
              "\r\n[rec]cpu return buffer count: %d"
              "\r\n[rec]cpu return buffer losed count: %d"
              "\r\n[rec]cpu have been slowed count: %d"
              "\r\n[rec]i2s have been slowed count: %d "
              "\r\n[rec]hardware run error count: %d"
              "\r\n[rec]hardware interrupt count: %d\r\n",
                  stmp->performance.get_buffer_cnt,
                  stmp->performance.return_buffer_cnt,
                  stmp->performance.return_lose_cnt,
                  stmp->performance.cpu_slow_cnt,
                  stmp->performance.i2s_slow_cnt,
                  stmp->performance.hw_error_cnt,
                  stmp->performance.hw_int_cnt);
  }
  #endif
}


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
uint32_t audioRecGetDataBuffer(uint8_t **ppData, uint32_t size, systime_t time)
{
  uint32_t len, avail, frame;
  hs_audio_stream_t *stmp = &g_audio_info.rec;
  virtual_timer_t vt;

  chVTObjectInit(&vt);
  if(g_audio_info.rec.state != AUDIO_RUNING)
  {
    return 0;
  }

  chMtxLock(&stmp->mutex);
  #if HS_I2S_USE_STATISTIC
  stmp->performance.get_buffer_cnt++;
  #endif

  nds32_dcache_flush();
  if (time != TIME_INFINITE)
  {
    chVTSet(&vt, time, _audio_wakeup, (void *)stmp);
  }

  audio_update_hw_ptr(stmp, 0);
  avail = _audio_record_avail(stmp);
  frame = _audio_byte2frame(stmp, size);
  if(frame > stmp->boundary)
  {
    frame = avail;
  }

  if((stmp->app_ptr + frame) > stmp->boundary)
  {
    frame = stmp->boundary - stmp->app_ptr;
  }

  if(frame > avail)
  {
    __audio_wait_for_interrupt(stmp, time, vt);
    avail = _audio_record_avail(stmp);

    if(frame > avail)
    {
      frame = avail;
    }

    #if HS_I2S_USE_STATISTIC
    stmp->performance.i2s_slow_cnt++;
    #endif
  }
  else
  {
    __audio_nowait_reset(time, vt);
  }

  *ppData = (uint8_t *)(stmp->dma_start + _audio_frame2byte(stmp, stmp->app_ptr));
  len = _audio_frame2byte(stmp, frame); 

  hs_audio_algAecDoData(*ppData, len);
  len = hs_audio_algAnsDoData(*ppData, len);
  
  chMtxUnlock(&stmp->mutex);

  return len;
}

/*
 * @brief               get saved recording data from buffer done
 *
 * @param[in] pData     recording data buffer pointer
 * @param[in] size      length have been read
 *                      .
 *
 */
void audioRecGetDataDone(uint8_t *pData, uint32_t size)
{
  uint32_t get_ptr;
  hs_audio_stream_t *stmp = &g_audio_info.rec;

  if(g_audio_info.rec.state != AUDIO_RUNING)
    return ;

  if(size == 0)
    return ;

  chMtxLock(&stmp->mutex);

  #if HS_I2S_USE_STATISTIC
  stmp->performance.return_buffer_cnt++;
  #endif

  get_ptr = (uint32_t)pData;
  if(_audio_byte2frame(stmp, get_ptr - stmp->dma_start) == stmp->app_ptr)
  {
    stmp->app_ptr += _audio_byte2frame(stmp, size);

    if(stmp->app_ptr >= stmp->boundary)
      stmp->app_ptr %= stmp->boundary;
  }
  else
  {
    #if HS_I2S_USE_STATISTIC
    stmp->performance.return_lose_cnt++;
    #endif
  }

  chMtxUnlock(&stmp->mutex);
}

void audioRecClrBuffer(void)
{
  hs_audio_stream_t *stmp = &g_audio_info.rec;
  
  if(stmp->state != AUDIO_RUNING)
    return ;

  audio_update_hw_ptr(stmp, 0);
  stmp->app_ptr = stmp->hw_ptr;
}

/*
 * @param[in] start_threshold: the point of sample to start to play
 */
int32_t audioPlayStart(hs_audio_config_t *cfgp, uint32_t start_threshold, hs_audio_cbfun_t event_fun)
{
  hs_audio_stream_t *stmp = &g_audio_info.ply;
  hs_audio_config_t *pTmpCfg = cfgp;
  int32_t res = -1;

  if((stmp->state != AUDIO_READY) && (stmp->prompt_dis != 0))
    return res;
  
  if(stmp->state != AUDIO_READY)
  {
    if(stmp->state >= AUDIO_PREPARE)
    {
      _audio_do_event(stmp, &g_audio_info.tx_event, AUDIO_EVENT_STOPPED);
      _audio_ply_stop();
    }
    else
    {
      return -1;
    }
  }

  if(cfgp == NULL)
  {
    pTmpCfg = &g_stDefaultCfg;
  }

  codec_lld_play_start(g_audio_info.pcodec, pTmpCfg);
  res = i2s_lld_play_start(g_audio_info.pi2s, pTmpCfg, stmp);
  if(res != 0)
  {
    codec_lld_play_stop(g_audio_info.pcodec);
    return res;
  }

  res = _audio_new_event(&g_audio_info.tx_event);
  if(res != 0)
  {
    _audio_ply_stop();
    return res;
  }

  if(start_threshold >= stmp->boundary)
    start_threshold = stmp->boundary / 2;

  stmp->start_threshold = start_threshold;
  g_audio_info.tx_event->pfnCallback = event_fun;
  stmp->state = AUDIO_PREPARE;
  stmp->auto_repair = AUDIO_AUTOREPAIR_DISABLE;

#if 0
  int i;
  for(i=0; i<DRC_FRAME_SIZE; i++)
    stmp->gain[i] = (1u << 15);
#endif

  return 0;
}

void audioPlayPromptDisable(void)
{
  hs_audio_stream_t *stmp = &g_audio_info.ply;

  stmp->prompt_dis = 1;
}

void audioPlayStop(void)
{
  hs_audio_stream_t *stmp = &g_audio_info.ply;

  chMtxLock(&stmp->mutex);
  if(stmp->state <= AUDIO_PREPARE)
  {
    codec_lld_play_stop(g_audio_info.pcodec);
    i2s_lld_play_stop(g_audio_info.pi2s);

    stmp->state = AUDIO_READY;

    _audio_do_event(stmp, &g_audio_info.tx_event, AUDIO_EVENT_RESUME);
    chMtxUnlock(&stmp->mutex);
    return ;
  }

  _audio_ply_stop();
  _audio_do_event(stmp, &g_audio_info.tx_event, AUDIO_EVENT_RESUME);
  chMtxUnlock(&stmp->mutex);

  #if HS_I2S_USE_STATISTIC
  audio_stc("\r\n[ply]cpu get buffer count: %d"
            "\r\n[ply]cpu return buffer count: %d"
            "\r\n[ply]cpu return buffer losed count: %d"
            "\r\n[ply]cpu have been slowed count: %d"
            "\r\n[ply]i2s have been slowed count: %d "
            "\r\n[ply]hardware run error count: %d"
            "\r\n[ply]hardware interrupt count: %d\r\n",
                stmp->performance.get_buffer_cnt,
                stmp->performance.return_buffer_cnt,
                stmp->performance.return_lose_cnt,
                stmp->performance.cpu_slow_cnt,
                stmp->performance.i2s_slow_cnt,
                stmp->performance.hw_error_cnt,
                stmp->performance.hw_int_cnt);
  #endif
}

hs_audio_stream_t * audioGetStream(hs_audio_streamdir_t dir)
{
  hs_audio_stream_t *stmp;

  stmp = dir == AUDIO_STREAM_PLAYBACK ? &g_audio_info.ply : &g_audio_info.rec;
  return stmp;
}

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
uint32_t audioPlyGetDataBuffer(uint8_t **ppData, uint32_t size, systime_t time)
{
  hs_audio_stream_t *stmp = &g_audio_info.ply;
  virtual_timer_t vt;
  uint32_t len, inavail, frame;

  chVTObjectInit(&vt);

  chMtxLock(&stmp->mutex);
  if(g_audio_info.ply.state < AUDIO_PREPARE)
  {
    return 0;
  }
  
  if(stmp->state == AUDIO_PAUSE)
  {
    return 0;
  }
  
  #if HS_I2S_USE_STATISTIC
  stmp->performance.get_buffer_cnt++;
  #endif

  if(time != TIME_INFINITE)
  {
    chVTSet(&vt, time, _audio_wakeup, (void *)stmp);
  }

  audio_update_hw_ptr(stmp, 0);
  inavail = _audio_play_inavail(stmp);
  frame = _audio_byte2frame(stmp, size);
  if(frame > stmp->boundary)
  {
    frame = inavail;
  }

  if((stmp->app_ptr + frame) > stmp->boundary)
  {
    frame = stmp->boundary - stmp->app_ptr;
  }

  if(frame > inavail)
  {
    __audio_wait_for_interrupt(stmp, time, vt);

    inavail = _audio_play_inavail(stmp);
    if(frame > inavail)
      frame = inavail;

    #if HS_I2S_USE_STATISTIC
    stmp->performance.i2s_slow_cnt++;
    #endif
  }
  else
  {
    __audio_nowait_reset(time, vt);
  }

  *ppData = (uint8_t *)(stmp->dma_start + _audio_frame2byte(stmp, stmp->app_ptr));
  len = _audio_frame2byte(stmp, frame);

  return len;
}

void audioPlyCpyData(uint8_t *dstData, uint8_t *srcData, uint32_t len)
{
#if 1
  memcpy(dstData, srcData, len);
#else
  hs_audio_stream_t *stmp = &g_audio_info.ply;
  uint16_t i, temp_len;
  int32_t temp;
  int16_t xin[DRC_FRAME_SIZE];

  while(len>1)
  {
    temp_len = len > (DRC_FRAME_SIZE * 2) ? DRC_FRAME_SIZE : (len/2);

    for(i=0; i<temp_len; i++)
    {
      xin[i] = (int32_t)((srcData[2 * i+1] << 8) + srcData[2 * i]);
      temp = xin[i] * stmp->gain[i];
      temp >>= 15;

      dstData[2 * i]     = temp;
      dstData[2 * i + 1] = temp >> 8;
    }
    
    for(i=0; i<temp_len; i++)
      stmp->gain[i] = hs_audio_calDrc(xin[i]);

    srcData += temp_len * 2;
    dstData += temp_len * 2;
    len     -= temp_len * 2;
  }
#endif
}

/*
 * @brief               save the data to playing buffer done
 *
 * @param[in] pData     playing data buffer pointer
 * @param[in] size      length have been write
 *                      .
 *
 */
void audioPlySendDataDone(uint8_t *pData, uint32_t size)
{
  uint32_t get_ptr;
  hs_audio_stream_t *stmp = &g_audio_info.ply;

  if(stmp->state < AUDIO_PREPARE)
  {
    chMtxUnlock(&stmp->mutex);
    return ;
  }

  if(size == 0)
  {
    chMtxUnlock(&stmp->mutex);
    return ;  
  }  

  #if HS_I2S_USE_STATISTIC
  stmp->performance.return_buffer_cnt++;
  #endif

  get_ptr = (uint32_t)pData;
  
#if defined(__nds32__)
  nds32_dcache_clean();
#endif
  if(_audio_byte2frame(stmp, get_ptr - stmp->dma_start) == stmp->app_ptr)
  {
    stmp->app_ptr += _audio_byte2frame(stmp, size);

    if ((stmp->app_ptr >= stmp->start_threshold) && (stmp->state == AUDIO_PREPARE))
    {
      i2s_lld_play_triger(g_audio_info.pi2s);
      stmp->state = AUDIO_RUNING;
    }

    if(stmp->app_ptr >= stmp->boundary)
      stmp->app_ptr %= stmp->boundary;
  }
  else
  {
    #if HS_I2S_USE_STATISTIC
    stmp->performance.return_lose_cnt++;
    #endif
  }

  audio_update_hw_ptr(stmp, 0);
  if(_audio_play_avail(stmp) < (stmp->boundary / 3))
  {
    if((stmp->app_ptr >= REPAIR_POINT_NUM) && (stmp->app_ptr < (stmp->boundary - 1)))
    {
      _audio_rp_swIncreasePoint(stmp);
      size += stmp->frame_bits / 8;
    }    
  }
  
  hs_audio_algAecSaveTxData(pData, size);
  
  chMtxUnlock(&stmp->mutex);
  if(stmp->auto_repair == AUDIO_AUTOREPAIR_ENABLE)
  {
    #if HS_AUDIO_USE_REPAIR
    hs_audio_autoRepair(stmp);
    #endif
  }
}

uint32_t audioGetBufferLevel(void)
{
  hs_audio_stream_t *stmp = &g_audio_info.ply;
  uint32_t buffer_level;

  if(stmp->state != AUDIO_RUNING)
    return 0;

  chMtxLock(&stmp->mutex);
  audio_update_hw_ptr(stmp, 0);
  buffer_level = _audio_play_avail(stmp);
  chMtxUnlock(&stmp->mutex);

  return buffer_level;
}

void audioPlySetAutoRepair(hs_audio_autorepair_t auto_r)
{
  hs_audio_stream_t *stmp = &g_audio_info.ply;

  if(stmp->state < AUDIO_PREPARE)
    return ;

  stmp->auto_repair = auto_r;
}

void audioPlayAcquire(void)
{
  chMtxLock(&g_audio_info.ply.mutex);
}

void audioPlayRelease(void)
{
  chMtxUnlock(&g_audio_info.ply.mutex);
}

void audioPlayPause(void)
{
  hs_audio_stream_t *stmp = &g_audio_info.ply;

  if(stmp->state == AUDIO_PAUSE)
    return ;

  if(stmp->state < AUDIO_RUNING)
    return ;

  chMtxLock(&stmp->mutex);
  stmp->state = AUDIO_PAUSE;

  while(1)
  {
    if((stmp->app_ptr + stmp->period_size) >= stmp->boundary)
      stmp->app_ptr = (stmp->app_ptr + stmp->period_size) % stmp->boundary;

    if(stmp->app_ptr <= stmp->hw_ptr)
    {
      dmaStreamDisable(g_audio_info.pi2s->pdmatx);
      stmp->app_ptr = stmp->hw_ptr = 0;
      break;
    }

    chThdSleep(MS2ST(10));

  }

  chMtxUnlock(&stmp->mutex);
}
void audioPlayResume(void)
{
  hs_audio_stream_t *stmp = &g_audio_info.ply;

  if(stmp->state != AUDIO_PAUSE)
    return ;

  chMtxLock(&stmp->mutex);
  g_audio_info.ply.state = AUDIO_PREPARE;
  chMtxUnlock(&stmp->mutex);
}

/* rc calibration */
void audioRxAdcCalibration()
{
  codec_lld_rxAdc_calibration();
}

void audioAuAdcCalibration()
{
  codec_lld_auAdc_calibration();
}

void audioTxDacCalibration()
{
  codec_lld_txDac_calibration();
}

void audioRxFilterCalibration()
{
  codec_lld_rxFilter_calibration();
}

void audioRxTiaCalibration()
{
  codec_lld_rxTia_calibration();
}


void audioMicbiasCalibration()
{
  codec_lld_micbias_calibration();
}

/* audio control function */
void audioRecordSetVolume(int db)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_RECORD_SET_VOLUME, &db);
}

int audioRecordGetVolumeMax(hs_audio_rec_src_t src)
{
  int max = src;
  
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_RECORD_GET_VOLUME_MAX, &max);
  return max;
}

int audioRecordGetVolumeMin(hs_audio_rec_src_t src)
{
  int min = src;

  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_RECORD_GET_VOLUME_MIN, &src);
  return min;
}

void audioRecordSetVolumeLeft(int db)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_RECORD_SET_VOLUME_LEFT, &db);
}

void audioRecordSetVolumeRight(int db)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_RECORD_SET_VOLUME_RIGHT, &db);
}

void audioRecordMute(void)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_RECORD_MUTE, NULL);
}

void audioRecordUnmute(void)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_RECORD_UNMUTE, NULL);
}

void audioPlaySetVolume(int db)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_PLAY_SET_VOLUME, &db);
}

int audioPlayGetVolumeMax(hs_audio_ply_src_t src)
{
  int max = src;
  
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_PLAY_GET_VOLUME_MAX, &max);
  return max;
}

int audioPlayGetVolumeMin(hs_audio_ply_src_t src)
{
  int min = src;

  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_PLAY_GET_VOLUME_MIN, &min);
  return min;
}

void audioPlaySetVolumeLeft(int db)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_PLAY_SET_VOLUME_LEFT, &db);
}

void audioPlaySetVolumeRight(int db)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_PLAY_SET_VOLUME_RIGHT, &db);
}

void audioPlayMute(void)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_PLAY_MUTE, NULL);
}

void audioPlayUnmute(void)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_PLAY_UNMUTE, NULL);
}

void audioSetCodecSel(int sel)
{
 	codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_CODEC_SEL, (void *)&sel);
}

void audioSetRecordSource(int source)
{
 	codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_RECORD_SOURCE, (void *)&source);
}

void audioSetPlaySource(int source)
{
 	codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_PLAY_SOURCE, (void *)&source);
}

void audioSetRecordSample(int sample)
{
 	codec_lld_ctrl(g_audio_info.pcodec, AUDIO_RECORD_SET_SAMPLE, (void *)&sample);
}

void audioSetPlaySample(int sample)
{
 	codec_lld_ctrl(g_audio_info.pcodec, AUDIO_PLAY_SET_SAMPLE, (void *)&sample);
}

void audioSetShortFir(int enable)
{
 	codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_SHORT_FIR, (void *)&enable);
}

void audioSetAdcDrcMode(int mode)
{
 	codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_ADC_DRC_MODE, (void *)&mode);
}

void audioSetAdcDrcLimiter(int limiter)
{
 	codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_ADC_DRC_LIMITER, (void *)&limiter);
}

void audioSetAdcDrcAgc(int limiter)
{
 	codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_ADC_DRC_AGC, (void *)&limiter);
}

void audioSetDacDrcMode(int mode)
{
 	codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_DAC_DRC_MODE, (void *)&mode);
}

void audioSetDacDrcLimiter(int limiter)
{
 	codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_DAC_DRC_LIMITER, (void *)&limiter);
}

void audioSetDacDrcAgc(int limiter)
{
 	codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_DAC_DRC_AGC, (void *)&limiter);
}

void audioSetAdcMix(int enable)
{
 	codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_ADC_MIX, (void *)&enable);
}

void audioSetDacMix(int enable)
{
 	codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_DAC_MIX, (void *)&enable);
}

void audioSetDacMode(int mode)
{
 	codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_DAC_MODE, (void *)&mode);
}

void audioInvertI2sInput(int sel)
{
 	codec_lld_ctrl(g_audio_info.pcodec, AUDIO_INVERT_I2S_INPUT, (void *)&sel);
}

void audioInvertI2sOutput(int sel)
{
 	codec_lld_ctrl(g_audio_info.pcodec, AUDIO_INVERT_I2S_OUTPUT, (void *)&sel);
}

void audioSetRecMode(int mode)
{
	chSysLock();
  __i2s_set_i2smode(g_audio_info.pi2s->rx_i2s, mode);
	chSysUnlock();
}

void audioSetPlyMode(int mode)
{
	chSysLock();
  __i2s_set_i2smode(g_audio_info.pi2s->tx_i2s, mode);
	chSysUnlock();
}

void audioSetEqEnable(int enable)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_EQ, (void *)&enable);
}

void audioSetBand1Coeff(uint16_t coeff)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_BAND1_COEFF, (void *)&coeff);
}

void audioSetBand1Gain(uint8_t gain)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_BAND1_GAIN, (void *)&gain);
}

void audioSetBand2Coeff(uint32_t coeff)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_BAND2_COEFF, (void *)&coeff);
}

void audioSetBand2Gain(uint8_t gain)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_BAND2_GAIN, (void *)&gain);
}

void audioSetBand3Coeff(uint32_t coeff)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_BAND3_COEFF, (void *)&coeff);
}

void audioSetBand3Gain(uint8_t gain)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_BAND3_GAIN, (void *)&gain);
}

void audioSetBand4Coeff(uint32_t coeff)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_BAND4_COEFF, (void *)&coeff);
}

void audioSetBand4Gain(uint8_t gain)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_BAND4_GAIN, (void *)&gain);
}

void audioSetBand5Coeff(uint32_t coeff)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_BAND5_COEFF, (void *)&coeff);
}

void audioSetBand5Gain(uint8_t gain)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_BAND5_GAIN, (void *)&gain);
}

void audioSetBand6Coeff(uint32_t coeff)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_BAND6_COEFF, (void *)&coeff);
}

void audioSetBand6Gain(uint8_t gain)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_BAND6_GAIN, (void *)&gain);
}

void audioSetBand7Coeff(uint16_t coeff)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_BAND7_COEFF, (void *)&coeff);
}

void audioSetBand7Gain(uint8_t gain)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_BAND7_GAIN, (void *)&gain);
}

void audioSetTestMode(uint8_t mode)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_TEST_MODE, (void *)&mode);
}

void audioSetI2sConnCtrl(uint8_t enable)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_I2S_CONN_CTRL, (void *)&enable);
}

int audioGetDacRms()
{
  int rms = 0;

  if(g_audio_info.ply.state >= AUDIO_PREPARE)
    codec_lld_ctrl(g_audio_info.pcodec, AUDIO_GET_DAC_RMS, (void *)&rms);
  
  return rms;
}

int audioGetAecDelay()
{
  int delay = 0;
  
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_GET_AEC_DELAY, (void *)&delay);
  return delay;
}

void audioAecDelayMeasureInit(int threshold)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_AEC_DELAY_MEASURE_INIT, (void *)&threshold);
}

void audioSetDacPeakDetect(uint8_t enable)
{
  codec_lld_ctrl(g_audio_info.pcodec, AUDIO_SET_DAC_PEAK, (void *)&enable);
}

uint32_t audioGetPlaySampleRate(void)
{
  if((!g_audio_info.pcodec) || (!g_audio_info.pcodec->tx_cfg))
    return 0;

  if(g_audio_info.ply.state < AUDIO_PREPARE)
    return 0;

  return g_audio_info.pcodec->tx_cfg->sample_rate;
}

NOINLINE void audioSetBTCfg(void)
{
  codec_lld_setBTCfg();
}

#endif /* HAL_USE_AUDIO */

/** @} */
