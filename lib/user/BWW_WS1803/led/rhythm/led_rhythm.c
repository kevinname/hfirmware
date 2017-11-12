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

#define RHYTHM_MAXVAL                 0x100u
#define LED_RHYTHM_MODE_CHANGED       (HS_CFG_EVENT_USER_BEGAN + 0x10)

#define MARQUEE_TYPE_NUM              7

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

  uint32_t            u32ModeCnt;
  uint32_t            u32FunMap;
}hs_arhythmcfg_t;

typedef struct
{
  uint8_t  u8Color[3];
}hs_rgbinfo_t;

static hs_arhythm_t g_stAudioRhythm =
{
  2,      /* rhythm led number */
  1,      /* rgb enable */
  1,      /* led active level */
  20,     /* led flash period, default: 20ms */

  /* rhytm led info */
  {
    { 0, 3, PA4, 0 },
    { 0, 2, PA3, 0 },    
    { 0, 0, PA0, 0 },
    { 0, 0, PB0, 0 },  
  },

  /* rgb led info */
  {
    { 0, 0, PA5, 0 },   /* red */
    { 0, 1, PB4, 0 },   /* green */
    { 0, 2, PB7, 0 },   /* blue */
  },  
  
  100,    /* frame flash peroid, default: 100ms */
  RHYTHM_MAXVAL, /* max volume */
};

static const uint8_t g_u8RGB[][3] =
{
  {1, 0, 0},   /* red */
  {0, 1, 0},   /* green */
  {0, 0, 1},   /* blue */
  {1, 1, 0},   /* yellow */
  {1, 0, 1},   /* purple */
  {0, 1, 1},
};


static const uint8_t g_u8Marquee[MARQUEE_TYPE_NUM][2] =
{
  {0, 1},   
  {1, 0},   
  {0, 0},   
  {0, 1},   
  {1, 1},   
  {0, 1},
  {0, 0},
};


static hs_arhythmcfg_t *g_pstARCfg;

static void _led_gpioOut(uint8_t u8LedIndex, uint8_t val)
{
  ioportid_t pGpio;

  if(u8LedIndex < PAL_IOPORTS_WIDTH)
  {
    pGpio = IOPORT0;
  }
  else
  {
    u8LedIndex = u8LedIndex - PAL_IOPORTS_WIDTH;
    pGpio = IOPORT1;
  }

  palWritePad(pGpio, u8LedIndex, val);
}

void _led_flashRhythm(hs_arhythmcfg_t *pstARCfg)
{
  uint16_t u16Rms, u16LedSize, u16CurrSize;
  int16_t s16TmpP;
  uint8_t i, u8CurrPos, u8RhyPinNum, u8PwmIdx, u8ChnIdx;

  u16Rms = audioGetDacRms();
  u8RhyPinNum = pstARCfg->pstAudR->u8RhyPinNum > 4 ? 4 : pstARCfg->pstAudR->u8RhyPinNum;

  if((pstARCfg->u32FunMap & 2) == 0)
  {
    for(i=0; i<u8RhyPinNum; i++)
    {
      u8PwmIdx = pstARCfg->pstAudR->stRhyLed[i].u8PwmIdx;
      u8ChnIdx = pstARCfg->pstAudR->stRhyLed[i].u8ChnIdx;      

      hs_pwm_set(pstARCfg->pwmHandler[u8PwmIdx], u8ChnIdx, 0);
    }
    
    return ;
  }

  if(u16Rms > 0x10)
  {
    if(u16Rms > (pstARCfg->pstAudR->u32MaxVol * 2))
      pstARCfg->pstAudR->u32MaxVol = u16Rms;

    if(u16Rms > pstARCfg->pstAudR->u32MaxVol)
    {
      s16TmpP = (u16Rms - pstARCfg->pstAudR->u32MaxVol) / (pstARCfg->pstAudR->u32MaxVol / 3);
      s16TmpP = s16TmpP > 3 ? 3 : s16TmpP;
      s16TmpP = 3 - s16TmpP + 2;
      if((pstARCfg->u32Cnt % s16TmpP) == 0)
        u16Rms = 0;
    }

    if((pstARCfg->u32Cnt % 20) == 0)
      pstARCfg->pstAudR->u32MaxVol = RHYTHM_MAXVAL;
      
    u16LedSize = pstARCfg->pstAudR->u32MaxVol / (u8RhyPinNum + 1);
    u8CurrPos = u16Rms / u16LedSize;
    u16CurrSize = u16Rms % u16LedSize;
    
    for(i=0; i<u8RhyPinNum; i++)
    {
      u8PwmIdx = pstARCfg->pstAudR->stRhyLed[u8RhyPinNum - 1 - i].u8PwmIdx;
      u8ChnIdx = pstARCfg->pstAudR->stRhyLed[u8RhyPinNum - 1 - i].u8ChnIdx;
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
      u16LedSize = (pstARCfg->u32Cnt / 4) % MARQUEE_TYPE_NUM;      

      for(i=0; i<u8RhyPinNum; i++)
      {
        u8PwmIdx = pstARCfg->pstAudR->stRhyLed[i].u8PwmIdx;
        u8ChnIdx = pstARCfg->pstAudR->stRhyLed[i].u8ChnIdx;      

        u16CurrSize = g_u8Marquee[u16LedSize][i] == 0 ? 0 : pstARCfg->pstAudR->u8LedPeriod;
        hs_pwm_set(pstARCfg->pwmHandler[u8PwmIdx], u8ChnIdx, u16CurrSize);
      }
    }
  }
}

void _led_flashRgb(hs_arhythmcfg_t *pstARCfg)
{
  uint32_t j, u32Num = sizeof(g_u8RGB) / 3;
  uint32_t u32Idx;
  uint8_t u8Val;

  if((pstARCfg->u32Cnt % 16) != 0)
    return ;

  u32Idx = (pstARCfg->u32Cnt / 16) % u32Num;
  for(j=0; j<3; j++)
  {
    u8Val  = (pstARCfg->u32FunMap & 1) == 0 ? 
      (!pstARCfg->pstAudR->u8LedActiveLvl) : g_u8RGB[u32Idx][j];
    _led_gpioOut(pstARCfg->pstAudR->stRgbLed[j].u8PinIdx, u8Val);
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
    stLedPad.u8PadMode = PAD_FUNC_GPIO;

    hs_pad_config(&stLedPad);
  }
}

void _led_modeChange(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;
  (void)parg;
  hs_arhythmcfg_t *pstARCfg = g_pstARCfg;
  uint8_t u8Mode;

  if(!pstARCfg)
    return ;

  pstARCfg->u32ModeCnt += 1;
  u8Mode = pstARCfg->u32ModeCnt % 3;
  if(u8Mode == 0)
    pstARCfg->u32FunMap = 3;
  else if(u8Mode == 1)
    pstARCfg->u32FunMap = 2;
  else
    pstARCfg->u32FunMap = 0;
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

  g_pstARCfg->u32FunMap = 3;
  hs_cfg_sysListenMsg(LED_RHYTHM_MODE_CHANGED,  _led_modeChange);
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
