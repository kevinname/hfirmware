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
#ifndef __LIB_AUDIO_AI_H__
#define __LIB_AUDIO_AI_H__

#include "hal.h"
#include "recorder.h"

typedef struct
{
  hs_audio_config_t   stI2sCfg;
  
  uint8_t             u8IsStart;
}hs_ai_t;

uint32_t hs_ai_fetchData(hs_ai_t *pstAi, uint8_t *pu8Buf, uint32_t u32Len);
hs_ai_t *hs_ai_create(hs_rec_cfg_t *pstRecCfg);
void hs_ai_destroy(hs_ai_t *pstAi);

#endif
