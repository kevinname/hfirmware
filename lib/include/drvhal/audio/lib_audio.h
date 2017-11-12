/*
    drvhal - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/drvhal/audio/lib_audio.h
 * @brief   codec include file.
 * @details 
 *
 * @addtogroup  lib drvhal
 * @details 
 * @{
 */
#ifndef __LIB_AUDIO_H__
#define __LIB_AUDIO_H__

#include "lib.h"
#include "lib_codec.h"

#if HAL_USE_AUDIO

typedef enum
{
  AVOL_DEV_NOR    = 0,
  AVOL_DEV_HFP    ,
  AVOL_DEV_TON    ,

  AVOL_DEV_NUM
}hs_avol_dev_t;

#ifdef __cplusplus
extern "C" {
#endif

void hs_audio_rhythmStart(uint16_t msg, void *parg);
void hs_audio_rhythmStop(uint16_t msg, void *parg);

int hs_audio_enAec(void);
int hs_audio_enAns(void);

void     hs_audio_volSet(hs_avol_dev_t eDev, int16_t s16Vol);
int16_t  hs_audio_volGet(hs_avol_dev_t eDev);
uint16_t hs_audio_volGetLvl(hs_avol_dev_t eDev);
void     hs_audio_volRestore(hs_avol_dev_t eDev);
void     hs_audio_init(void);

#ifdef __cplusplus
}
#endif

#endif

#endif
 /** @} */

