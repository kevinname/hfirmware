/*
    bootloader - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    config/tone/cfg_audio.c
 * @brief   audio play and record.
 * @details 
 *
 * @addtogroup  sbc
 * @details 
 * @{
 */
#include "lib.h"
#include "cfg_audio.h"
#include "cfg_tone.h"

#if HS_USE_TONE

static hs_audio_config_t *g_pstAudPlyCfg, *g_pstAudRecCfg;
static uint8_t g_u8RecRunning, g_u8PlyRunning;

static hs_audio_config_t * _cfg_toneNewAudio(uint32_t u32Rate, hs_i2s_pcmmode_t enMode)
{
  hs_audio_config_t * pstCfg;

  pstCfg = (hs_audio_config_t *)hs_malloc(sizeof(hs_audio_config_t), __MT_Z_GENERAL);
  if(!pstCfg)
  {
    return NULL;
  }

  pstCfg->frame_len = I2S_PLY_BLOCK_SIZE;
  pstCfg->frame_num = 4;

  pstCfg->i2s_mode = enMode;
  pstCfg->sample_rate = (hs_i2s_sample_t)u32Rate;
  pstCfg->sample_width = I2S_BITWIDTH_16BIT;
  pstCfg->work_mode = I2S_WORKMODE_SLAVE;
  pstCfg->ws_width = I2S_BITWIDTH_32BIT;

  return pstCfg;
}

static void _cfg_tonePlyCallBack(hs_audio_event_t enEvent)
{
  if(enEvent == AUDIO_EVENT_STOPPED)
  {
    g_u8PlyRunning = CFG_AUDIO_STOP;
  }

  if(enEvent == AUDIO_EVENT_RESUME)
  {
    //audioPlayStart(&g_pstAudPlyCfg, 0);
    //audioPlyRegisterEvent(hs_audio_ply_event);
    //audioPlayStop();
  }
}

static void _cfg_toneRecCallBack(hs_audio_event_t enEvent)
{
  if(enEvent == AUDIO_EVENT_STOPPED)
  {
    g_u8RecRunning = CFG_AUDIO_STOP;
  }

  if(enEvent == AUDIO_EVENT_RESUME)
  {
    //audioRecordStart(&g_pstAudPlyCfg, 0);
    //audioRecRegisterEvent(hs_audio_ply_event);
    //audioRecordStop();
  }
}

void hs_cfg_toneAudioStart(uint32_t u32Rate, uint8_t u8Mode, int16_t s16ToneVolume)
{
  (void)s16ToneVolume;
  hs_cfg_res_t enRes;
  hs_cfg_tone_info_t stToneInfo;
  uint16_t u16MonoMode;
  int res;
    
  g_pstAudPlyCfg = _cfg_toneNewAudio(u32Rate, (hs_i2s_pcmmode_t)u8Mode);

  if(u8Mode == TONE_AUDIO_MODE_MONO)
  {  
    enRes = hs_cfg_getDataByIndex(HS_CFG_TONE_INFO, (uint8_t *)&stToneInfo, sizeof(hs_cfg_tone_info_t));
    if(enRes != HS_CFG_OK)
      u16MonoMode = TRACK_LL;
    else
      u16MonoMode = stToneInfo.u8SndMonoMode;
  }
  
  audioPlayAcquire();
  res = audioPlayStart(g_pstAudPlyCfg, 0, _cfg_tonePlyCallBack);
  if(res != 0)
  {
    hs_printf("tone start error!\r\n");
    hs_free(g_pstAudPlyCfg);
    g_pstAudPlyCfg = NULL;
    audioPlayRelease();
    return ;
  }
  
  audioPlayPromptDisable();
  audioSetPlaySource(AUDIO_PLAY_RAM);
  
  g_u8PlyRunning = CFG_AUDIO_RUNNING;
  hs_audio_volRestore(AVOL_DEV_TON);

  if(u8Mode == TONE_AUDIO_MODE_MONO)
    audioInvertI2sOutput(u16MonoMode);  //LL

  audioPlayUnmute();
  msleep(100);
  audioPlayRelease();
}

hs_cfg_res_t hs_cfg_toneTxStream(uint8_t *u8Buf, uint32_t u32Len)
{
  uint32_t u32RealLen;
  uint8_t *pu8Ptr;

  do
  {
    u32RealLen = audioPlyGetDataBuffer(&pu8Ptr, u32Len, TIME_INFINITE);
    memcpy(pu8Ptr, u8Buf, u32RealLen);
    audioPlySendDataDone(pu8Ptr, u32RealLen);
    u32Len -= u32RealLen;
    u8Buf += u32RealLen;

    if(g_u8PlyRunning == CFG_AUDIO_STOP)
    {
      return HS_CFG_ERROR;
    }
  }while(u32Len);

  return HS_CFG_OK;
}

void hs_cfg_toneAudioStop(uint8_t u8Interrupt)
{
  uint8_t u8state = g_u8PlyRunning;
  
  g_u8PlyRunning = CFG_AUDIO_STOP;

  /* avoid no sound if a playing tone's end point is interrupted by MP3 player */
  if((u8Interrupt == 0) && (u8state == CFG_AUDIO_RUNNING))
  {
    audioPlayStop();
  }

  if(g_pstAudPlyCfg)
  {
    hs_free(g_pstAudPlyCfg);
    g_pstAudPlyCfg = NULL;
  }  
}

void hs_cfg_toneRecStart(uint32_t u32Rate, uint8_t u8Mode, int16_t s16ToneVolume)
{  
  g_pstAudRecCfg = _cfg_toneNewAudio(u32Rate, (hs_i2s_pcmmode_t)u8Mode);
  
  audioStart();
  audioSetRecordSource(AUDIO_RECORD_MIC);
  
  audioRecordStart(g_pstAudRecCfg, _cfg_toneRecCallBack);
  
  audioRecordSetVolume(s16ToneVolume);
  g_u8RecRunning = CFG_AUDIO_RUNNING;
  
}

hs_cfg_res_t hs_cfg_toneRxStream(uint8_t *u8Buf, uint32_t u32Len)
{
  uint32_t u32RealLen;
  uint8_t *pu8Ptr;

  do
  {
    u32RealLen = audioRecGetDataBuffer(&pu8Ptr, u32Len, TIME_INFINITE);
    memcpy(u8Buf, pu8Ptr, u32RealLen);
    audioRecGetDataDone(pu8Ptr, u32RealLen);
    
    u32Len -= u32RealLen;
    u8Buf += u32RealLen;

    if(g_u8RecRunning == CFG_AUDIO_STOP)
    {
      return HS_CFG_ERROR;
    }
  }while(u32Len);

  return HS_CFG_OK;
}

void hs_cfg_toneRecStop(uint8_t u8Interrupt)
{  
  g_u8RecRunning = CFG_AUDIO_STOP;

  if(u8Interrupt == 0)
  {
    audioRecordStop();
  }

  if(g_pstAudRecCfg)
  {
    hs_free(g_pstAudRecCfg);
    g_pstAudRecCfg = NULL;
  }
}

#endif /* HS_USE_TONE */


/** @} */
