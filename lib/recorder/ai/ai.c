/*
    audio player - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/audio/dec/dec.c
 * @brief   source file.
 * @details 
 *
 * @addtogroup  decoder
 * @details 
 * @{
 */

#include "lib.h"
#include "ai.h"

#define RECORDER_AI_BLOCKSIZE       2400
#define RECORDER_AI_BLOCKNUM        4


static void _ai_initPara(hs_ai_t *pstAi, hs_rec_cfg_t *pstRecCfg)
{
  hs_audio_config_t *pstCfg = &pstAi->stI2sCfg;
  
  pstCfg->sample_rate  = pstRecCfg->u32Sample;

  pstCfg->frame_len    = RECORDER_AI_BLOCKSIZE;
  pstCfg->frame_num    = RECORDER_AI_BLOCKNUM;

  pstCfg->sample_width = I2S_BITWIDTH_16BIT;
  pstCfg->ws_width     = I2S_BITWIDTH_32BIT;
  pstCfg->work_mode    = I2S_WORKMODE_MASTER;
  pstCfg->i2s_mode     = pstRecCfg->u16Mode;
}

void _ai_start(hs_ai_t *pstAi, hs_rec_cfg_t *pstRecCfg)
{
  int res;
  
  if((!pstAi) || (pstAi->u8IsStart == 1))
    return ;

  res = audioRecordStart(&pstAi->stI2sCfg, NULL);
  if(res != 0)
  {
    hs_printf("$");
    audioPlayRelease();
    return ;
  }

  audioRecordMute();
  audioSetRecordSource(AUDIO_RECORD_MIC);   
  audioSetTestMode(0);  
  audioInvertI2sInput(TRACK_RR);
  pmu_ana_set(10, 12, 4);
  audioRecordSetVolume(pstRecCfg->s16Gain);
  pstAi->u8IsStart = 1;

  msleep(100);
  audioRecordUnmute();
}

void _ai_stop(hs_ai_t *pstAi)
{
  if(pstAi->u8IsStart)
  {
    pstAi->u8IsStart = 0;
    audioRecordStop();    
  }
}

uint32_t hs_ai_fetchData(hs_ai_t *pstAi, uint8_t *pu8Buf, uint32_t u32Len)
{
  uint32_t u32Tmp, u32GetLen = 0;
  uint8_t *pData;

  if ((!pstAi) || (!pstAi->u8IsStart))
    return u32GetLen;

  while(u32Len > 0)
  {
    u32Tmp = audioRecGetDataBuffer(&pData, u32Len, TIME_INFINITE);
    memcpy(pu8Buf, pData, u32Tmp);
    audioRecGetDataDone(pData, u32Tmp);

    if(u32Tmp == 0)
      break;

    u32Len    -= u32Tmp;
    pu8Buf    += u32Tmp;
    u32GetLen += u32Tmp;
  }

  return u32GetLen;
}

hs_ai_t *hs_ai_create(hs_rec_cfg_t *pstRecCfg)
{
  hs_ai_t *pstAi;  

  pstAi = (hs_ai_t *)hs_malloc(sizeof(hs_ai_t), __MT_Z_GENERAL);
  if(!pstAi)
    return NULL;

  _ai_initPara(pstAi, pstRecCfg);
  _ai_start(pstAi, pstRecCfg);
  
  return pstAi;
}

void hs_ai_destroy(hs_ai_t *pstAi)
{
  if(!pstAi)
    return ;

  _ai_stop(pstAi);
  hs_free(pstAi);
}

