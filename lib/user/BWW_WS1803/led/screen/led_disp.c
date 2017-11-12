/*
    application - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    app/led_disp.c
 * @brief   led application.
 * @details 
 *
 * @addtogroup  app
 * @details 
 * @{
 */

#include "led_disp.h"
#include "led_engine.h"

#if HS_USE_LEDDISP

#define LED_PINNUM  7
#define LED_DIGNUM  4

#define LED_PIN1    PA7
#define LED_PIN2    PA9
#define LED_PIN3    PA11
#define LED_PIN4    PA12
#define LED_PIN5    PB2
#define LED_PIN6    PB5
#define LED_PIN7    PB6

enum
{
  LED_DIG_A         = 0x00u,
  LED_DIG_B         ,
  LED_DIG_C         ,
  LED_DIG_D         ,
  LED_DIG_E         ,
  LED_DIG_F         ,
  LED_DIG_G         ,

  LED_DIG_MAX
};

typedef struct 
{
  uint32_t    u32Function;
  uint8_t     u8Digit[LED_DIGNUM];
}hs_led_frame_t;


typedef struct 
{
  osTimerId       pstTimer;  
  hs_led_frame_t  stFrame;
  uint32_t        u32Ms;

  virtual_timer_t stVt;
}hs_led_t;

typedef struct
{
  volatile uint8_t *pu8Ptr;
  uint8_t len;
}hs_led_diginfo_t;

typedef struct
{
  uint8_t  u8OnTime;       /* unit: us */
  uint8_t  u8Period;

  uint32_t u32Cnt;
}hs_ledscreen_t;

static hs_led_t g_stLed;
static hs_ledscreen_t g_stScreenPara;

volatile uint8_t g_u8LedPin[LED_PINNUM] = 
{
  LED_PIN1, LED_PIN2, LED_PIN3, LED_PIN4, LED_PIN5, LED_PIN6, LED_PIN7
};

volatile uint8_t g_u8FunPin[LED_FUNC_MAX] = 
{
  0x16, 0x36, 0x62, 0x15, 0x34, 0x73, 0x37
};

volatile uint8_t g_u8DigPin[LED_DIGNUM][LED_DIG_MAX] = 
{
  {0x12, 0x13, 0x41, 0x51, 0x14, 0x21, 0x31},
  {0x23, 0x24, 0x52, 0x26, 0x25, 0x32, 0x42},
  {0x54, 0x35, 0x45, 0x61, 0x63, 0x43, 0x53},
  {0x76, 0x67, 0x56, 0x64, 0x46, 0x65, 0x57}
};

volatile uint8_t g_u8Dig_0[] = {LED_DIG_A, LED_DIG_B, LED_DIG_C, LED_DIG_D, LED_DIG_E, LED_DIG_F};
volatile uint8_t g_u8Dig_1[] = {LED_DIG_B, LED_DIG_C};
volatile uint8_t g_u8Dig_2[] = {LED_DIG_A, LED_DIG_B, LED_DIG_D, LED_DIG_E, LED_DIG_G};
volatile uint8_t g_u8Dig_3[] = {LED_DIG_A, LED_DIG_B, LED_DIG_C, LED_DIG_D, LED_DIG_G};
volatile uint8_t g_u8Dig_4[] = {LED_DIG_B, LED_DIG_C, LED_DIG_F, LED_DIG_G};
volatile uint8_t g_u8Dig_5[] = {LED_DIG_A, LED_DIG_C, LED_DIG_D, LED_DIG_F, LED_DIG_G};
volatile uint8_t g_u8Dig_6[] = {LED_DIG_A, LED_DIG_C, LED_DIG_D, LED_DIG_E, LED_DIG_F, LED_DIG_G};
volatile uint8_t g_u8Dig_7[] = {LED_DIG_A, LED_DIG_B, LED_DIG_C};
volatile uint8_t g_u8Dig_8[] = {LED_DIG_A, LED_DIG_B, LED_DIG_C, LED_DIG_D, LED_DIG_E, LED_DIG_F, LED_DIG_G};
volatile uint8_t g_u8Dig_9[] = {LED_DIG_A, LED_DIG_B, LED_DIG_C, LED_DIG_D, LED_DIG_F, LED_DIG_G};
volatile uint8_t g_u8Dig_Idle[] = {LED_DIG_G};
volatile uint8_t g_u8Dig_b[] = {LED_DIG_C, LED_DIG_D, LED_DIG_E, LED_DIG_F, LED_DIG_G};
volatile uint8_t g_u8Dig_L[] = {LED_DIG_D, LED_DIG_E, LED_DIG_F};
volatile uint8_t g_u8Dig_u[] = {LED_DIG_B, LED_DIG_C, LED_DIG_D, LED_DIG_E, LED_DIG_F};
volatile uint8_t g_u8Dig_e[] = {LED_DIG_A, LED_DIG_D, LED_DIG_E, LED_DIG_F, LED_DIG_G};
volatile uint8_t g_u8Dig_P[] = {LED_DIG_A, LED_DIG_B, LED_DIG_E, LED_DIG_F, LED_DIG_G};
volatile uint8_t g_u8Dig_H[] = {LED_DIG_B, LED_DIG_C, LED_DIG_E, LED_DIG_F, LED_DIG_G};
volatile uint8_t g_u8Dig_A[] = {LED_DIG_A, LED_DIG_B, LED_DIG_C, LED_DIG_E, LED_DIG_F, LED_DIG_G};
volatile uint8_t g_u8Dig_Pd[] = {};


volatile hs_led_diginfo_t g_stDig[] = 
{
  {g_u8Dig_0, sizeof(g_u8Dig_0)},
  {g_u8Dig_1, sizeof(g_u8Dig_1)},
  {g_u8Dig_2, sizeof(g_u8Dig_2)},
  {g_u8Dig_3, sizeof(g_u8Dig_3)},
  {g_u8Dig_4, sizeof(g_u8Dig_4)},
  {g_u8Dig_5, sizeof(g_u8Dig_5)},
  {g_u8Dig_6, sizeof(g_u8Dig_6)},
  {g_u8Dig_7, sizeof(g_u8Dig_7)},
  {g_u8Dig_8, sizeof(g_u8Dig_8)},
  {g_u8Dig_9, sizeof(g_u8Dig_9)},

  {g_u8Dig_Idle, sizeof(g_u8Dig_Idle)},   /* 10 - */
  {g_u8Dig_b, sizeof(g_u8Dig_b)},         /* 11 b */
  {g_u8Dig_L, sizeof(g_u8Dig_L)},         /* 12 L */
  {g_u8Dig_u, sizeof(g_u8Dig_u)},         /* 13 u */
  {g_u8Dig_e, sizeof(g_u8Dig_e)},         /* 14 e */
  {g_u8Dig_P, sizeof(g_u8Dig_P)},         /* 15 P */
  {g_u8Dig_H, sizeof(g_u8Dig_H)},         /* 16 H */
  {g_u8Dig_A, sizeof(g_u8Dig_A)},         /* 17 A */
  {g_u8Dig_Pd, 0}                         /* 18 power down */
  
};

volatile uint16_t g_u16DigNum = sizeof(g_stDig) / sizeof(hs_led_diginfo_t);


#define __led_getFuncMask(fidx)         (1u << (fidx))
#define __led_clrFunc(u8Func, fidx)     ((u8Func) &= (~ __led_getFuncMask(fidx)))
#define __led_setFunc(u8Func, fidx)     ((u8Func) |= __led_getFuncMask(fidx))

static __ONCHIP_CODE__ void _led_gpioOut(uint8_t u8PinIndex, uint8_t val)
{
  ioportid_t pGpio;

  if(u8PinIndex < PAL_IOPORTS_WIDTH)
  {
    pGpio = IOPORT0;
  }
  else
  {
    u8PinIndex = u8PinIndex - PAL_IOPORTS_WIDTH;
    pGpio = IOPORT1;
  }

  palWritePad(pGpio, u8PinIndex, val);
}

static __ONCHIP_CODE__ void _led_gpioDir(uint8_t u8PinIndex, uint8_t output)
{
  ioportid_t pGpio;

  if(u8PinIndex < PAL_IOPORTS_WIDTH)
  {
    pGpio = IOPORT0;
  }
  else
  {
    u8PinIndex = u8PinIndex - PAL_IOPORTS_WIDTH;
    pGpio = IOPORT1;
  }

  if(output)
    pGpio->OUTENSET = 1u << u8PinIndex;
  else
    pGpio->OUTENCLR = 1u << u8PinIndex;
}

/**************************************************************************/
static __ONCHIP_CODE__ void _led_pinOutAllOff(void)
{
  uint16_t i;

  for(i=0; i<LED_PINNUM; i++)
    _led_gpioDir(g_u8LedPin[i], 0);
}

static __ONCHIP_CODE__ void _led_groupOn(uint8_t u8Idx, uint32_t u32Map)
{
  uint16_t i;

  _led_gpioDir(g_u8LedPin[u8Idx - 1], 1);
  _led_gpioOut(g_u8LedPin[u8Idx - 1], 1);

  
  for(i=1; i<=PIN_NUM; i++)
  {
    u32Map >>= 1;
    
    if(u32Map & 1)
    {
      _led_gpioDir(g_u8LedPin[i - 1], 1);
      _led_gpioOut(g_u8LedPin[i - 1], 0);
    }    
  }
}

static __ONCHIP_CODE__ uint32_t _led_searchPin(uint8_t u8Idx)
{
  hs_led_frame_t *pstFrame = &g_stLed.stFrame;
  volatile hs_led_diginfo_t *pstDig;
  uint32_t u32BitMap = 0, u32Func;
  uint16_t i, j;
  
  u32Func = pstFrame->u32Function;
  for(i=0; i<LED_FUNC_MAX; i++)
  {
    if ((u32Func & 1) && ((g_u8FunPin[i] >> 4) == u8Idx))
      u32BitMap |= 1u << (g_u8FunPin[i] & 0xf);

    u32Func >>= 1;
  }

  for(i=0; i<LED_DIGNUM; i++)
  {
    pstDig = &g_stDig[pstFrame->u8Digit[i]];
    for(j=0; j<pstDig->len; j++)
    {
      if((g_u8DigPin[i][pstDig->pu8Ptr[j]] >> 4) == u8Idx)
        u32BitMap |= 1u << (g_u8DigPin[i][pstDig->pu8Ptr[j]] & 0xf);
    }
  }

  return u32BitMap;
}

__ONCHIP_CODE__ void hs_led_frameDisp(void)
{
  uint32_t idx, u32Map;

  if(!g_stScreenPara.u8Period)
    return ;
  
  g_stScreenPara.u32Cnt += 1;
  if((g_stScreenPara.u32Cnt % g_stScreenPara.u8Period) != 0)
    return ;

  _led_pinOutAllOff();
  if(hs_pmu_isSleeping())
    return ;
  
  idx = (g_stScreenPara.u32Cnt / g_stScreenPara.u8Period) % LED_PINNUM + 1;
  u32Map = _led_searchPin(idx);
  _led_groupOn(idx, u32Map);
}

void hs_led_initDisp(uint32_t freq)
{
  int i; 
  
  memset(&g_stLed, 0, sizeof(hs_led_t)); 
  g_stLed.u32Ms = 1000 / freq;

  g_stScreenPara.u8OnTime = LED_ON_TIME;
  g_stScreenPara.u8Period = 2;
  g_stScreenPara.u32Cnt   = 0;

  hs_padinfo_t stPad = {LED_PIN1, PAD_FUNC_GPIO, CFG_PAD_DIR_INPUT, CFG_PAD_PULL_NO, 3};
  for(i=0; i<LED_PINNUM; i++)
  {
    stPad.u16PadIdx = g_u8LedPin[i];
    hs_pad_config(&stPad);
  }
}

void hs_led_setFunc(hs_led_funidx_t idx, hs_led_switch_t st)
{
  if(st == LED_ON)
    __led_setFunc(g_stLed.stFrame.u32Function, idx);
  else
    __led_clrFunc(g_stLed.stFrame.u32Function, idx);
}

void hs_led_setFuncReverse(hs_led_funidx_t idx)
{
  if(g_stLed.stFrame.u32Function & __led_getFuncMask(idx))
    __led_clrFunc(g_stLed.stFrame.u32Function, idx);
  else
    __led_setFunc(g_stLed.stFrame.u32Function, idx);
}

void hs_led_setDig(uint8_t idx, uint8_t val)
{
  val %= g_u16DigNum; idx %= LED_DIGNUM;  
  g_stLed.stFrame.u8Digit[idx] = val;
}

#endif

/** @} */
