/*
    drvhal - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/drvhal/audio/lib_audio.c
 * @brief   lib_audio file.
 * @details 
 *
 * @addtogroup  lib drvhal
 * @details 
 * @{
 */

#include "lib.h"
#include "math.h"
#include "lib_aec.h"
#include "lib_ans.h"
#include "cfg_tone.h"


#if HAL_USE_AUDIO

typedef struct
{
  int16_t  s16Dev[AVOL_DEV_NUM];
}hs_audio_var_t;

static hs_audio_var_t g_stAudioVar;

const hs_codec_boardinfo_t *hs_boardGetCodecInfo(void);

int hs_audio_enAec(void)
{
  return audioAecStart(hs_aec_init, hs_aec_process, hs_aec_uninit);
}

int hs_audio_enAns(void)
{
  return audioAnsStart(hs_ans_init, hs_ans_process, hs_ans_uninit);
}

void hs_audio_volSet(hs_avol_dev_t eDev, int16_t s16Vol)
{
  hs_audio_ply_src_t src = eDev == AVOL_DEV_HFP ? AUDIO_PLY_SRC_HFP : 0; 
  int tmpVol;
  
  if(eDev >= AVOL_DEV_NUM)
    return ;  

  tmpVol = audioPlayGetVolumeMax(src);
  if(s16Vol >= tmpVol)
  {
    s16Vol = tmpVol;
    g_stAudioVar.s16Dev[eDev] = s16Vol;
    hs_led_disp((eDev << 12) | LED_DISP_VOLUME);

    if(eDev != AVOL_DEV_HFP)
      hs_cfg_toneDoEvent(HS_CFG_SYS_EVENT, HS_CFG_EVENT_VOL_MAXALARM, 0);
  }

  tmpVol = audioPlayGetVolumeMin(src);
  if(s16Vol <= tmpVol)
  {
    s16Vol = tmpVol;
    g_stAudioVar.s16Dev[eDev] = s16Vol;
    hs_led_disp((eDev << 12) | LED_DISP_VOLUME);
    
    if(eDev != AVOL_DEV_HFP)
      hs_cfg_toneDoEvent(HS_CFG_SYS_EVENT, HS_CFG_EVENT_VOL_MINALARM, 0);

    audioPlaySetVolume(s16Vol);
    hs_cfg_setDataByIndex(HS_CFG_PROD_AUDIO, (uint8_t *)&g_stAudioVar, sizeof(hs_audio_var_t), 0);  
    
    audioPlayMute();
    return ;
  }
  else
  {
    audioPlayUnmute();
  }  

  g_stAudioVar.s16Dev[eDev] = s16Vol;
  hs_led_disp((eDev << 12) | LED_DISP_VOLUME);
  audioPlaySetVolume(s16Vol);
  hs_cfg_setDataByIndex(HS_CFG_PROD_AUDIO, (uint8_t *)&g_stAudioVar, sizeof(hs_audio_var_t), 0);  
}

int16_t hs_audio_volGet(hs_avol_dev_t eDev)
{
  if(eDev >= AVOL_DEV_NUM)
    return 0;

  return g_stAudioVar.s16Dev[eDev];
}

uint16_t hs_audio_volGetLvl(hs_avol_dev_t eDev)
{
  hs_audio_ply_src_t src = eDev == AVOL_DEV_HFP ? AUDIO_PLY_SRC_HFP : 0; 
  int16_t s16Max, s16Min;
  uint16_t u16Vol;
  
  if(eDev >= AVOL_DEV_NUM)
    return 0;

  s16Max = audioPlayGetVolumeMax(src);
  s16Min = audioPlayGetVolumeMin(src);

  u16Vol = s16Max - s16Min;
  u16Vol = (g_stAudioVar.s16Dev[eDev] - s16Min) * 32 / u16Vol;

  return u16Vol;  
}

void hs_audio_volRestore(hs_avol_dev_t eDev)
{
  const hs_codec_boardinfo_t *pstInfo;
  
  if(eDev >= AVOL_DEV_NUM)
    return ;

  if(eDev != AVOL_DEV_HFP)
  {
    pstInfo = hs_boardGetCodecInfo();
    hs_codec_setEq(pstInfo->pstDefaultEq);
  }
  
  audioPlaySetVolume(g_stAudioVar.s16Dev[eDev]);
  if(g_stAudioVar.s16Dev[eDev] <= audioPlayGetVolumeMin(0))
    audioPlayMute();
  else
    audioPlayUnmute();
}

void hs_audio_init(void)
{
  int i;

  audioStart();
  
  if(HS_CFG_OK != hs_cfg_getDataByIndex(HS_CFG_PROD_AUDIO, (uint8_t *)&g_stAudioVar, sizeof(hs_audio_var_t)))
  {
    for(i=0; i<AVOL_DEV_NUM; i++)
      g_stAudioVar.s16Dev[i] = 0;
  }
}

#endif

/** @} */
