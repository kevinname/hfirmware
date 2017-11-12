/*
    speaker - Copyright (C) 2012~2017 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    speaker/speaker.c
 * @brief   speaker file.
 * @details 
 *
 * @addtogroup  speaker
 * @details 
 * @{
 */


#include "lib.h"

typedef struct
{
  uint32_t    u32Samp;
  uint8_t     u8ChnNum;
  uint8_t     u8Volume;
  uint8_t     u8PgaGain;
  int8_t      s8AdcVol;
}hs_speaker_cfg_t;

typedef struct
{
  uint8_t             u8AplyOwner;

  osTimerId           pstTimer;
  
  hs_speaker_cfg_t    stSpkCfg;
  hs_audio_config_t   stAcfg;
}hs_speaker_t;

static hs_speaker_t *g_pstSpeaker;

static void _speaker_audioCb(hs_audio_event_t enEvent)
{
  if(enEvent == AUDIO_EVENT_STOPPED)
  {
    if(g_pstSpeaker)
      g_pstSpeaker->u8AplyOwner = 0;
  }

  if(enEvent == AUDIO_EVENT_RESUME)
  {
    ;
  }
}


void _speaker_audioStart(hs_speaker_t *pstSpeaker)
{
  uint32_t u32Samp;

  audioRecordStop();
  
  audioPlayAcquire();  
  u32Samp = audioGetPlaySampleRate();
  if(u32Samp == 0)
  {
    pstSpeaker->u8AplyOwner = 1;
    pstSpeaker->stAcfg.sample_rate  = pstSpeaker->stSpkCfg.u32Samp;
    audioPlayStart(&pstSpeaker->stAcfg, 0, _speaker_audioCb);
    audioSetTestMode(0);
    if(pstSpeaker->stAcfg.i2s_mode == I2S_PCMMODE_STEREO)
      audioInvertI2sOutput(TRACK_LR);
    else
      audioInvertI2sOutput(TRACK_LL);     
  }
  else
  {
    pstSpeaker->u8AplyOwner = 0;
    pstSpeaker->stAcfg.sample_rate = u32Samp;  
    audioPlayMute();
  }
  audioPlayRelease();
  
  audioRecordStart(&pstSpeaker->stAcfg, NULL);
  audioSetRecordSource(AUDIO_RECORD_MIC);
  audioSetAdcMix(1);
  pmu_ana_set(10, 12, pstSpeaker->stSpkCfg.u8PgaGain);
  audioRecordSetVolume(pstSpeaker->stSpkCfg.s8AdcVol);

  audioSetMixerEn(pstSpeaker->stSpkCfg.u8Volume);

  msleep(100);
  hs_audio_volRestore(AVOL_DEV_NOR);  
}

void _speaker_checkSamp(void const *arg)
{
  hs_speaker_t *pstSpeaker = (hs_speaker_t *)arg;

  if(!audioMixerIsEn())
    _speaker_audioStart(pstSpeaker);
}

void _speaker_initPara(hs_speaker_t *pstSpeaker)
{
  hs_speaker_cfg_t *pstSpkCfg = &pstSpeaker->stSpkCfg;
  osTimerDef_t stTmDef; 

  if(HS_CFG_OK != hs_cfg_getDataByIndex(HS_CFG_SPEAKER_INFO, (uint8_t *)pstSpkCfg, sizeof(hs_speaker_cfg_t)))
  {
    pstSpkCfg->u32Samp   = I2S_SAMPLE_48K;
    pstSpkCfg->u8ChnNum  = 2;
    pstSpkCfg->u8Volume  = 0x80;
    pstSpkCfg->u8PgaGain = 5;
    pstSpkCfg->s8AdcVol  = 0;
  }
  pstSpkCfg->u8PgaGain %= 7;
  
  pstSpeaker->stAcfg.sample_rate  = pstSpkCfg->u32Samp;
  pstSpeaker->stAcfg.sample_width = I2S_BITWIDTH_16BIT;
  pstSpeaker->stAcfg.ws_width     = I2S_BITWIDTH_32BIT;
  pstSpeaker->stAcfg.work_mode    = I2S_WORKMODE_SLAVE;
  pstSpeaker->stAcfg.i2s_mode     = pstSpkCfg->u8ChnNum == 2 ? I2S_PCMMODE_STEREO : I2S_PCMMODE_MONO;
  pstSpeaker->stAcfg.frame_len    = 256;
  pstSpeaker->stAcfg.frame_num    = 3;

  stTmDef.ptimer = _speaker_checkSamp;
  pstSpeaker->pstTimer = oshalTimerCreate(&stTmDef, osTimerPeriodic, (void *)pstSpeaker);
  
}

void _speaker_destroy(hs_speaker_t *pstSpeaker)
{
  audioSetMixerEn(0);
  
  if(pstSpeaker->u8AplyOwner)
    audioPlayStop();

  audioRecordStop();
  oshalTimerDelete(pstSpeaker->pstTimer);
  pstSpeaker->pstTimer = NULL;

  hs_free(pstSpeaker);
}

void hs_speaker_sw(uint16_t u16Idx, void *parg)
{
  (void)u16Idx;
  (void)parg;
  hs_speaker_t *pstSpeaker;  

  if(g_pstSpeaker)
  {
    _speaker_destroy(g_pstSpeaker);
    g_pstSpeaker = NULL;
    return ;
  }

  pstSpeaker = (hs_speaker_t *)hs_malloc(sizeof(hs_speaker_t), __MT_Z_GENERAL);
  if(!pstSpeaker)
    return ;

  g_pstSpeaker = pstSpeaker;

  _speaker_initPara(pstSpeaker);
  _speaker_audioStart(pstSpeaker); 
  
  oshalTimerStart(pstSpeaker->pstTimer, 100);
}

__USED bool hs_speaker_isWorking(void)
{
  return (g_pstSpeaker != NULL);
}


