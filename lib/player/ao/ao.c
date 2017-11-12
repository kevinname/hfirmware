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

#if HS_USE_PLAYER

#define PLAYER_AO_BLOCKSIZE       2400
#define PLAYER_AO_BLOCKNUM        4
#define PLAYER_AO_THREH           ((PLAYER_AO_BLOCKSIZE * PLAYER_AO_BLOCKNUM) / 4 - 100)


static hs_ao_t *g_pstAo;

void _ao_event(hs_audio_event_t enEvent)
{
  if(!g_pstAo)
    return ;
  
  if(enEvent == AUDIO_EVENT_RESUME) 
    hs_ao_start(g_pstAo);
}

void _ao_musicEvent(hs_audio_event_t enEvent)
{
  if(!g_pstAo)
    return ;
  
  if(enEvent == AUDIO_EVENT_STOPPED)
  {
    g_pstAo->isBlock = 1;
    g_pstAo->isStart = 0;
  }

  if(enEvent == AUDIO_EVENT_RESUME)
  {    
    hs_ao_start(g_pstAo);
  }
}

static void _ao_initPara(hs_ao_t *pstAo, hs_playermode_t eWorkMode)
{
  hs_audio_config_t *pstCfg = &pstAo->stI2sCfg;
  
  if(eWorkMode == PLAYER_WORKMODE_MP3)
  {
    pstAo->pfnAudioEvent = _ao_musicEvent;
    pstAo->audioSrc      = AUDIO_PLAY_RAM;    
    pstCfg->sample_rate  = I2S_SAMPLE_44P1K;

    pstCfg->frame_len    = PLAYER_AO_BLOCKSIZE;
    pstCfg->frame_num    = PLAYER_AO_BLOCKNUM;
  }
  else if(eWorkMode == PLAYER_WORKMODE_FM)
  {
    pstAo->pfnAudioEvent = _ao_musicEvent;
    pstAo->audioSrc      = AUDIO_PLAY_FM;    
    pstCfg->sample_rate  = I2S_SAMPLE_32K;    

    pstCfg->frame_len    = PLAYER_AO_BLOCKSIZE / 6;
    pstCfg->frame_num    = PLAYER_AO_BLOCKNUM;
  }
  else if(eWorkMode == PLAYER_WORKMODE_LINEIN)
  {
    pstAo->pfnAudioEvent = _ao_musicEvent;
    pstAo->audioSrc      = AUDIO_PLAY_RAM;//AUDIO_PLAY_LINEIN;    
    pstCfg->sample_rate  = I2S_SAMPLE_48K;    

    pstCfg->frame_len    = PLAYER_AO_BLOCKSIZE / 2;
    pstCfg->frame_num    = PLAYER_AO_BLOCKNUM;
  }

  pstCfg->sample_width = I2S_BITWIDTH_16BIT;
  pstCfg->ws_width     = I2S_BITWIDTH_32BIT;
  pstCfg->work_mode    = I2S_WORKMODE_MASTER;
  pstCfg->i2s_mode     = I2S_PCMMODE_STEREO;
}

void hs_ao_start(hs_ao_t *pstAo)
{
  int res;
  
  if((!pstAo) || (pstAo->isStart == 1))
    return ;

  audioPlayAcquire();
  pstAo->isBlock = 0;
  res = audioPlayStart(&pstAo->stI2sCfg, PLAYER_AO_THREH, pstAo->pfnAudioEvent);
  if(res != 0)
  {
    hs_printf("@");
    audioPlayRelease();
    return ;
  }
    
  audioSetPlaySource(pstAo->audioSrc);   
  audioSetTestMode(0); 
  
  if(pstAo->stI2sCfg.i2s_mode == I2S_PCMMODE_STEREO)
    audioInvertI2sOutput(TRACK_LR);
  else
    audioInvertI2sOutput(TRACK_LL);   
  
  pstAo->isStart = 1;

  msleep(100);
  //audioPlayUnmute();
  hs_audio_volRestore(AVOL_DEV_NOR);  
  audioPlayRelease();
}

void hs_ao_setSample(int samp)
{
  hs_ao_t *pstAo = g_pstAo;
  
  if(!pstAo)
    return ;

  pstAo->stI2sCfg.sample_rate = samp;

  if(pstAo->isStart == 1)
    audioSetPlaySample(samp);
}

int hs_ao_fetchData(uint8_t *pu8Buf, uint32_t u32Len)
{
  systime_t timeout = MS2ST(10);
  uint32_t tmp;
  uint8_t *pData;
  
  if(!g_pstAo)
    return 0;

  if((!g_pstAo->isStart) && (!g_pstAo->isBlock))
  {
    #if 0
    hs_ao_start(g_pstAo);
    if(!g_pstAo->isStart)
      return 0;
    #else
    while(!g_pstAo->isStart)
    {
      msleep(100);
      hs_ao_start(g_pstAo);
    }
    #endif
  }

  if(u32Len % 4)
	  u32Len -= u32Len % 4;

  while(u32Len > 0) 
  {
    if(g_pstAo->isBlock)
    {
      chThdSleepMilliseconds(10);
      continue;
    }

    if(!g_pstAo->isStart)
      return 0;
    
    tmp = u32Len > PLAYER_AO_BLOCKSIZE ? PLAYER_AO_BLOCKSIZE : u32Len;
    tmp = audioPlyGetDataBuffer(&pData, tmp, timeout);
    audioPlyCpyData(pData, pu8Buf, tmp);
    audioPlySendDataDone(pData, tmp);  

    if(0 == audioGetBufferLevel())
      hs_printf("************************\r\n");
    
    pu8Buf += tmp;
    u32Len -= tmp;
  }

  return 0;
}

hs_ao_t *hs_ao_create(int eWorkMode)
{
  hs_ao_t *pstAo;  

  pstAo = (hs_ao_t *)hs_malloc(sizeof(hs_ao_t), __MT_Z_GENERAL);
  if(!pstAo)
    return NULL;

  _ao_initPara(pstAo, eWorkMode);
  pstAo->volume = -20;

  g_pstAo = pstAo;
  return pstAo;
}

void hs_ao_destroy(hs_ao_t *pstAo)
{
  if(!pstAo)
    return ;

  hs_ao_stop(pstAo);
  hs_free(pstAo);
  g_pstAo = NULL;
}

void hs_ao_stop(hs_ao_t *pstAo)
{
  if(pstAo->isStart)
  {
    pstAo->isStart = 0;
    audioPlayStop();    
  }
}

void hs_ao_setVol(hs_ao_t *pstAo, int vol)
{
  if(pstAo)
    pstAo->volume = vol;
  
  hs_audio_volSet(AVOL_DEV_NOR, vol);
}

#endif

