/*
    ChibiOS/RT - Copyright (C) 2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    audio_repair.c
 * @brief   clock repair of audio system .
 *
 * @addtogroup audio
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "string.h"
#include "chprintf.h"

#if HS_AUDIO_USE_REPAIR || defined(__DOXYGEN__)

#if 0//HS_AUDIO_USE_SWREPAIR

#define REPAIR_POINT_NUM          64
#define REPAIR_THRESHOLD_VALUE    10

#define __audio_rp_GetThrowData(value, i, nextValue)  \
  ((value * (REPAIR_POINT_NUM - (i/2 + 1)) + nextValue * (i/2 + 1)) / REPAIR_POINT_NUM)

#define __audio_rp_GetIncreaseData(value, i, lstValue)  \
  ((value * (REPAIR_POINT_NUM - (i/2 + 1)) + lstValue * (i/2 + 1)) / REPAIR_POINT_NUM)

#define __audio_frame2byte(stmp, frame)   ((frame) * stmp->frame_bits / 8)
  

void _audio_rp_swThrowPoint(hs_audio_stream_t *stmp)
{
  int16_t i, j, *ptr;
  int32_t temp, next_left, next_right;

  if(stmp->app_ptr < REPAIR_POINT_NUM)
  {
    return ;
  }
  
  ptr = (int16_t *)(stmp->dma_start + __audio_frame2byte(stmp, (stmp->app_ptr - REPAIR_POINT_NUM)));

  for(i=0; i<REPAIR_POINT_NUM-1; i++)
  {
    if(stmp->frame_bits == 16)
    {
      j = 2 * i;
      temp = ptr[i];
      next_left = ptr[i+1];
      ptr[i] = __audio_rp_GetThrowData(temp, j, next_left);
    }
    else
    {
      j = 2 * i;
      temp = ptr[j];
      next_left = ptr[j+2];
      ptr[j] = __audio_rp_GetThrowData(temp, j, next_left);

      j += 1;
      temp = ptr[j];
      next_right = ptr[j+2];
      ptr[j] = __audio_rp_GetThrowData(temp, j, next_right);
    }
  }

  stmp->app_ptr -= 1;
}

void _audio_rp_swIncreasePoint(hs_audio_stream_t *stmp)
{
  int16_t i, j, *ptr;
  int32_t temp, last_left, last_right;
  
  if((stmp->app_ptr < REPAIR_POINT_NUM) || (stmp->app_ptr == stmp->boundary))
  {
    return ;
  }
  
  ptr = (int16_t *)(stmp->dma_start + __audio_frame2byte(stmp, (stmp->app_ptr - REPAIR_POINT_NUM)));

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

void hs_audio_autoRepair(hs_audio_stream_t *stmp)
{
  uint32_t u32Level;

  u32Level = audioGetBufferLevel();
  if(u32Level == 0)
  {
    return ;
  }  

  if((u32Level > stmp->start_threshold) 
    && ((u32Level - stmp->start_threshold) >= REPAIR_THRESHOLD_VALUE))
  {
    _audio_rp_swThrowPoint(stmp);
  }

  if((u32Level < stmp->start_threshold) 
    && ((stmp->start_threshold - u32Level) >= REPAIR_THRESHOLD_VALUE))
  {
    _audio_rp_swIncreasePoint(stmp);
  }
}

#endif

#if HS_AUDIO_USE_HWREPAIR

#define HWREPAIR_FIELD_NUM          6

#define HWREPAIR_SUIT_FIELD1        (HWREPAIR_FIELD_NUM / 2 + 1)
#define HWREPAIR_SUIT_FIELD0        (HWREPAIR_FIELD_NUM / 2 - 1)

#define __field_size(stmp)                    ((stmp)->boundary / HWREPAIR_FIELD_NUM)
#define __get_field(stmp, lvl)                ((lvl) / __field_size(stmp))
#define __chk_fieldInSuit(field)              (((field) <= HWREPAIR_SUIT_FIELD1) && ((field) >= HWREPAIR_SUIT_FIELD0))
#define __chk_fieldInQuick(field)             ((field) < HWREPAIR_SUIT_FIELD0)
#define __chk_fieldInSlow(field)              ((field) > HWREPAIR_SUIT_FIELD1)

#if HS_AUDIO_HWREPAIR_WSCLK
static uint32_t g_u32CurrClk;
#endif

void _audio_rp_hwAdjust(int step)
{
  if(step == 0xff)
    return ;
  
  #if HS_AUDIO_HWREPAIR_WSCLK
  HS_CODEC_Type *codec = HS_CODEC;
  uint32_t div, odd;

  if(g_u32CurrClk == 0)
    return ;

  div = (g_u32CurrClk >> 24) & 0xff;
  odd = (g_u32CurrClk >> 23) & 1;

  if(step != 0)
  {
    div = div * 2 - odd;
    div -= step;
    if(div % 2)
    {
      div = div / 2 + 1;
      odd = 1;
    }
    else
    {
      div = div / 2;
      odd = 0;
    }
  }
  
  __codec_set_bitsval(codec->CLK_CTRL_2, 23, 23, odd);
  __codec_set_bitsval(codec->CLK_CTRL_2, 24, 31, div);
  #endif

  #if HS_AUDIO_HWREPAIR_WORKCLK
  HS_PSO_CPM_Type *pstPso = HS_PSO;

  if(step == 0)
    __codec_set_bitval(pstPso->CODEC_CFG, 9, 0);
  else if(step > 0)
    __codec_set_bitsval(pstPso->CODEC_CFG, 8, 31, 0xfff03);
  else
    __codec_set_bitsval(pstPso->CODEC_CFG, 8, 31, 0x3ff02);
  
  #endif
}

void hs_audio_autoRepair(hs_audio_stream_t *stmp)
{
  uint32_t u32Level, u32Field;
  int step;

  #if HS_AUDIO_HWREPAIR_WSCLK
  if(stmp->state == AUDIO_PREPARE)
  {
    g_u32CurrClk  = HS_CODEC->CLK_CTRL_2;
    return ;
  }
  #endif

  u32Level = audioGetBufferLevel();

  chMtxLock(&stmp->mutex);
  u32Field = __get_field(stmp, u32Level);  

  if(__chk_fieldInSuit(u32Field))
    step = 0xff;
  else if(__chk_fieldInQuick(u32Field))
    step = u32Field - HWREPAIR_SUIT_FIELD0;
  else if(__chk_fieldInSlow(u32Field))
    step = 0;

  _audio_rp_hwAdjust(step);
  chMtxUnlock(&stmp->mutex);
}


#endif

#endif /* HS_AUDIO_USE_REPAIR */

/** @} */
