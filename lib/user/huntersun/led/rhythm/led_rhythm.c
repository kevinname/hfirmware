/*
    application - Copyright (C) 2012~2017 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/user/rhythm/led_rhythm.c
 * @brief   user function
 * @details 
 *
 * @addtogroup  lib user's function
 * @details 
 * @{
 */
#include "lib.h"

#define RHYTHM_MAXVAL       0x500u

typedef struct
{
  uint8_t   u8PwmIdx;
  uint8_t   u8ChnIdx;
  uint8_t   u8PinIdx;
  uint8_t   u8Reserve;
}hs_ledinfo_t;

typedef struct
{
  uint8_t         u8RhyPinNum; 
  uint8_t         u8RgbEnable;
  uint8_t         u8LedActiveLvl;  
  uint8_t         u8LedPeriod;

  hs_ledinfo_t    stRhyLed[4];
  hs_ledinfo_t    stRgbLed[3];
  
  uint16_t        u16FrameTm;
  uint32_t        u32MaxVol;
}hs_arhythm_t;

typedef struct
{
  osTimerId           pstTimer;
  hs_pwmhandle_t      pwmHandler[3];
  hs_arhythm_t       *pstAudR;
  uint32_t            u32Cnt;
}hs_arhythmcfg_t;

typedef struct
{
  uint8_t  u8Color[3];
}hs_rgbinfo_t;

static hs_arhythm_t g_stAudioRhythm =
{
  0,      /* rhythm led number */
  0,      /* rgb enable */
  0,      /* led active level */
  20,     /* led flash period, default: 20ms */

  /* rhytm led info */
  {
    { 2, 1, PB5, 0 },
    { 0, 3, PA5, 0 },
    { 0, 1, PA4, 0 },
    { 0, 0, PB4, 0 },  
  },

  /* rgb led info */
  {
    { 2, 1, PB5, 0 },   /* red */
    { 0, 3, PA0, 0 },   /* green */
    { 0, 1, PA0, 0 },   /* blue */
  },  
  
  100,    /* frame flash peroid, default: 100ms */
  RHYTHM_MAXVAL, /* max volume */
};

static const uint8_t g_u8RGB[][3] =
{
  {255, 0  , 0  },   /* red */
  {0  , 255, 0  },   /* green */
  {0  , 0  , 255},   /* blue */
  {255, 255, 0  },   /* yellow */
  {128, 0  , 128},   /* purple */
  {64 , 128, 128},
  {203, 28 , 121},
  {33 , 214, 181}
};

static hs_arhythmcfg_t *g_pstARCfg;

void _led_flashRhythm(hs_arhythmcfg_t *pstARCfg)
{
  uint16_t u16Rms, u16LedSize, u16CurrSize;
  uint8_t i, u8CurrPos, u8RhyPinNum, u8PwmIdx, u8ChnIdx;

  u16Rms = audioGetDacRms();
  u8RhyPinNum = pstARCfg->pstAudR->u8RhyPinNum > 4 ? 4 : pstARCfg->pstAudR->u8RhyPinNum;

  if(u16Rms > 0)
  {
    if(u16Rms > pstARCfg->pstAudR->u32MaxVol)
      pstARCfg->pstAudR->u32MaxVol = u16Rms;
      
    u16LedSize = pstARCfg->pstAudR->u32MaxVol / (u8RhyPinNum + 1);
    u8CurrPos = u16Rms / u16LedSize;
    u16CurrSize = u16Rms % u16LedSize;
    
    for(i=0; i<u8RhyPinNum; i++)
    {
      u8PwmIdx = pstARCfg->pstAudR->stRhyLed[i].u8PwmIdx;
      u8ChnIdx = pstARCfg->pstAudR->stRhyLed[i].u8ChnIdx;
      if(i < u8CurrPos)
        hs_pwm_set(pstARCfg->pwmHandler[u8PwmIdx], u8ChnIdx, pstARCfg->pstAudR->u8LedPeriod);
      else if(i > u8CurrPos)
        hs_pwm_set(pstARCfg->pwmHandler[u8PwmIdx], u8ChnIdx, 0);
      else
      {
        u16CurrSize = u16CurrSize * pstARCfg->pstAudR->u8LedPeriod / u16LedSize;
        hs_pwm_set(pstARCfg->pwmHandler[u8PwmIdx], u8ChnIdx, u16CurrSize);
      }
    }
  }
  else
  {
    pstARCfg->pstAudR->u32MaxVol = RHYTHM_MAXVAL;
    
    if((pstARCfg->u32Cnt % 4) == 0)
    {
      u16LedSize = (pstARCfg->u32Cnt / 4) % (1u << u8RhyPinNum);

      for(i=0; i<u8RhyPinNum; i++)
      {
        u8PwmIdx = pstARCfg->pstAudR->stRhyLed[i].u8PwmIdx;
        u8ChnIdx = pstARCfg->pstAudR->stRhyLed[i].u8ChnIdx;      

        u16CurrSize = (u16LedSize & (1u << i)) == 0 ? 0 : pstARCfg->pstAudR->u8LedPeriod;
        hs_pwm_set(pstARCfg->pwmHandler[u8PwmIdx], u8ChnIdx, u16CurrSize);
      }
    }
  }
}

void _led_flashRgb(hs_arhythmcfg_t *pstARCfg)
{
  uint32_t j, u32Num = sizeof(g_u8RGB) / 3;
  uint32_t u32Width, u32Idx;
  uint8_t u8Pwm, u8Chn;

  if((pstARCfg->u32Cnt % 20) != 0)
    return ;

  u32Idx = (pstARCfg->u32Cnt / 20) % u32Num;

  for(j=0; j<3; j++)
  {
    u8Pwm = pstARCfg->pstAudR->stRgbLed[j].u8PwmIdx;
    u8Chn = pstARCfg->pstAudR->stRgbLed[j].u8ChnIdx;
    
    u32Width = g_u8RGB[u32Idx][j] * pstARCfg->pstAudR->u8LedPeriod / 256;
    hs_pwm_set(pstARCfg->pwmHandler[u8Pwm], u8Chn, u32Width);
  }
}

void _led_flash(void const *arg)
{
  hs_arhythmcfg_t *pstARCfg = (hs_arhythmcfg_t *)arg;

  pstARCfg->u32Cnt += 1;
  
  if(pstARCfg->pstAudR->u8RhyPinNum > 1)
    _led_flashRhythm(pstARCfg);

  if(pstARCfg->pstAudR->u8RgbEnable)
    _led_flashRgb(pstARCfg);
}

void _led_rhyPadInit(const hs_arhythm_t *pstAR)
{
  hs_padinfo_t stLedPad = {0, PAD_FUNC_TIMER1_CH0, CFG_PAD_DIR_OUTPUT, CFG_PAD_PULL_NO, 3};
  uint16_t i, u16PinNum;
  uint8_t u8PwmIdx;

  u16PinNum = pstAR->u8RhyPinNum > 4 ? 4 : pstAR->u8RhyPinNum;
  for(i=0; i<u16PinNum; i++)
  {
    u8PwmIdx = pstAR->stRhyLed[i].u8PwmIdx;
    stLedPad.u16PadIdx = pstAR->stRhyLed[i].u8PinIdx;  
    if(u8PwmIdx == 0)
      stLedPad.u8PadMode = PAD_FUNC_TIMER1_CH0 + (pstAR->stRhyLed[i].u8ChnIdx & 3);
    else
      stLedPad.u8PadMode = PAD_FUNC_TIMER2_3;

    hs_pad_config(&stLedPad);
  }  

  for(i=0; i<3; i++)
  {
    u8PwmIdx = pstAR->stRgbLed[i].u8PwmIdx;
    stLedPad.u16PadIdx = pstAR->stRgbLed[i].u8PinIdx;  
    if(u8PwmIdx == 0)
      stLedPad.u8PadMode = PAD_FUNC_TIMER1_CH0 + (pstAR->stRgbLed[i].u8ChnIdx & 3);
    else
      stLedPad.u8PadMode = PAD_FUNC_TIMER2_3;

    hs_pad_config(&stLedPad);
  }
}

void hs_led_flash(void)
{
  int i;
  hs_arhythm_t *pstAR = &g_stAudioRhythm;
  osTimerDef_t stTmDef;
  
  if(g_pstARCfg)
    return ;

  g_pstARCfg = (hs_arhythmcfg_t *)hs_malloc(sizeof(hs_arhythmcfg_t), __MT_Z_GENERAL);
  if(!g_pstARCfg)
    return ;

  for(i=0; i<3; i++)
  {
    if(!g_pstARCfg->pwmHandler[i])
      g_pstARCfg->pwmHandler[i] = hs_pwm_init(i, pstAR->u8LedPeriod, pstAR->u8LedActiveLvl);
  }

  g_pstARCfg->pstAudR = pstAR;
  stTmDef.ptimer = _led_flash;
  g_pstARCfg->pstTimer = oshalTimerCreate(&stTmDef, osTimerPeriodic, (void *)g_pstARCfg);
  if(!g_pstARCfg->pstTimer)
  {
    for(i=0; i<3; i++)
      hs_pwm_uninit(g_pstARCfg->pwmHandler[i]);
    hs_free(g_pstARCfg);
    g_pstARCfg = NULL;
    return ;
  }

  _led_rhyPadInit(pstAR);
  oshalTimerStart(g_pstARCfg->pstTimer, pstAR->u16FrameTm);
}

void hs_led_flashStop(void)
{
  int i;
  
  if(!g_pstARCfg)
    return ;

  oshalTimerDelete(g_pstARCfg->pstTimer);
  
  for(i=0; i<3; i++)
    hs_pwm_uninit(g_pstARCfg->pwmHandler[i]);
  
  hs_free(g_pstARCfg);
  g_pstARCfg = NULL;
}



/** @} */
