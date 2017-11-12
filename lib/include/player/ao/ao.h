/*
    audio plaer - Copyright (C) 2012~2016 HunterSun Technologies
                  pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/audio/dec/dec.h
 * @brief   include file.
 * @details 
 *
 * @addtogroup  decoder
 * @details 
 * @{
 */
#ifndef __LIB_AUDIO_AO_H__
#define __LIB_AUDIO_AO_H__

#if HS_USE_PLAYER

#include "hal.h"

typedef struct
{
  hs_audio_config_t   stI2sCfg;
  hs_audio_cbfun_t    pfnAudioEvent;
  int                 audioSrc; 
  int                 isBlock;
  int                 isStart;
  int                 volume;
}hs_ao_t;


hs_ao_t *hs_ao_create(int eWorkMode);
void     hs_ao_start(hs_ao_t *pstAo);
void     hs_ao_destroy(hs_ao_t *pstAo);
int      hs_ao_fetchData(uint8_t *pu8Buf, uint32_t u32Len);
void     hs_ao_stop(hs_ao_t *pstAo);
void     hs_ao_setVol(hs_ao_t *pstAo, int vol);
void     hs_ao_setSample(int samp);

#endif

#endif
