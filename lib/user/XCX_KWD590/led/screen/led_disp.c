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

#define LED_PINNUM  6
#define LED_DIGNUM  4

#if __BOARD_KWD590__
#define LED_PIN1    PA15
#define LED_PIN2    PA13
#define LED_PIN3    PA12
#define LED_PIN4    PA11
#define LED_PIN5    PA9
#define LED_PIN6    PA7
#else //auevbv2
#define LED_PIN6    PA11
#define LED_PIN5    PA12
#define LED_PIN4    PA15
#define LED_PIN3    PB1
#define LED_PIN2    PB2
#define LED_PIN1    PB3
#endif

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
  LED_PIN1, LED_PIN2, LED_PIN3, LED_PIN4, LED_PIN5, LED_PIN6,
};

volatile uint8_t g_u8FunPin[LED_FUNC_MAX] = 
{
  0x63,
};

volatile uint8_t g_u8DigPin[LED_DIGNUM][LED_DIG_MAX] = 
{
  /*  a     b     c     d     e     f     g */
  {0x00, 0x43, 0x53, 0x00, 0x00, 0x00, 0x00},
  {0x21, 0x31, 0x41, 0x51, 0x61, 0x32, 0x42},
  {0x12, 0x13, 0x14, 0x15, 0x16, 0x52, 0x62},
  {0x23, 0x24, 0x25, 0x26, 0x34, 0x35, 0x36}
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
volatile uint8_t g_u8Dig_Idle[] = {LED_DIG_D};
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

static void _led_normalDisp(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;
  (void)parg;

  g_stScreenPara.u8OnTime = LED_ON_TIME;
  g_stScreenPara.u8Period = 1;
}

static void _led_lowDisp(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;
  (void)parg;

  g_stScreenPara.u8OnTime = 1;
  g_stScreenPara.u8Period = 5;
}

static void _led_changeDisp(uint16_t u16Msg, void *parg)
{
  uint32_t arg = (uint32_t)parg;

  if(!arg)
    _led_normalDisp(u16Msg, parg);
  else
    _led_lowDisp(u16Msg, parg);
}

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


static __ONCHIP_CODE__ void _led_pinOutOn(uint8_t u8Pin)
{
  uint16_t u16PadIdx;
  
  u16PadIdx = g_u8LedPin[(u8Pin&0xf) - 1];
  _led_gpioDir(u16PadIdx, 1);
  _led_gpioOut(u16PadIdx, 0);

  u16PadIdx = g_u8LedPin[((u8Pin >> 4)&0xf) - 1];
  _led_gpioDir(u16PadIdx, 1);
  _led_gpioOut(u16PadIdx, 1);
}

static __ONCHIP_CODE__ void _led_pinOutOff(uint8_t u8Pin)
{
  uint16_t u16PadIdx;
  
  u16PadIdx = g_u8LedPin[(u8Pin&0xf) - 1];
  _led_gpioDir(u16PadIdx, 0);

  u16PadIdx = g_u8LedPin[((u8Pin >> 4)&0xf) - 1];
  _led_gpioDir(u16PadIdx, 0);
}

static __ONCHIP_CODE__ void _led_on(uint8_t u8Pin)
{
  _led_pinOutOn(u8Pin);
  
  cpm_delay_us(g_stScreenPara.u8OnTime);
  
  _led_pinOutOff(u8Pin);
}

static __ONCHIP_CODE__ void _led_digDisp(uint8_t idx, uint8_t val)
{
  volatile hs_led_diginfo_t *pstDig;
  int i;
  
  val %= g_u16DigNum; idx %= LED_DIGNUM;
  pstDig = &g_stDig[val];
  for(i=0; i<pstDig->len; i++)
    _led_on(g_u8DigPin[idx][pstDig->pu8Ptr[i]]);
}

__ONCHIP_CODE__ void hs_led_frameDisp(void)
{
  hs_led_frame_t *pstFrame = &g_stLed.stFrame;
  int i;
  uint32_t u32Func;
  static uint8_t m_hi_idx = 1;
  uint8_t lo_bitmap = 0;

  if(!g_stScreenPara.u8Period)
    return ;
  
  g_stScreenPara.u32Cnt += 1;
  if((g_stScreenPara.u32Cnt % g_stScreenPara.u8Period) != 0)
    return ;

  if(!pstFrame)
    return ;

  if(hs_pmu_isSleeping())
    return ;

  /* translate display to LED ctrl-pairs */
  for(i=0; i<LED_FUNC_MAX; i++) {
    if (pstFrame->u32Function & (0x1u<<i)) {
      uint8_t pair = g_u8FunPin[i];
      if ((0 != (pair >> 4)) && ((m_hi_idx+0) == (pair >> 4)))
        lo_bitmap |= (1u << (pair & 0x0f));
    }
  }
  for(i=0; i<LED_DIGNUM; i++) {
    int ii;
    hs_led_diginfo_t *pstDig;
    uint8_t val = pstFrame->u8Digit[i];
    pstDig = (hs_led_diginfo_t *)&g_stDig[val];
    for (ii=0; ii<pstDig->len; ii++) {
      uint8_t pair = g_u8DigPin[i][pstDig->pu8Ptr[ii]];
      if ((0 != (pair >> 4)) && ((m_hi_idx+0) == (pair >> 4)))
        lo_bitmap |= (1u << (pair & 0x0f));
    }
  }

  /* set intput to clear all */
#if __BOARD_KWD590__
  HS_GPIO0->DATAOUT &= ~((1u << LED_PIN1) | (1u << LED_PIN2) | (1u << LED_PIN3) | (1u << LED_PIN4) | (1u << LED_PIN5) | (1u << LED_PIN6));
  HS_GPIO0->OUTENCLR = (1u << LED_PIN1) | (1u << LED_PIN2) | (1u << LED_PIN3) | (1u << LED_PIN4) | (1u << LED_PIN5) | (1u << LED_PIN6);
#else //auevbv2
  HS_GPIO0->OUTENCLR = (1u << 15) | (1u << 12) | (1u << 11);
  HS_GPIO1->OUTENCLR = (1u << 3) | (1u << 2) | (1u << 1);
#endif

  /* pair: output hi */
#if __BOARD_KWD590__
  HS_GPIO0->DATAOUT |= (1u << g_u8LedPin[m_hi_idx-1]);
  HS_GPIO0->OUTENSET = (1u << g_u8LedPin[m_hi_idx-1]);
#else
  switch (m_hi_idx) {
  case 1:
    HS_GPIO1->DATAOUT |= (1u << 3);
    HS_GPIO1->OUTENSET = (1u << 3);
    break;
  case 2:
    HS_GPIO1->DATAOUT |= (1u << 2);
    HS_GPIO1->OUTENSET = (1u << 2);
    break;
  case 3:
    HS_GPIO1->DATAOUT |= (1u << 1);
    HS_GPIO1->OUTENSET = (1u << 1);
    break;
  case 4:
    HS_GPIO0->DATAOUT |= (1u << 15);
    HS_GPIO0->OUTENSET = (1u << 15);
    break;
  case 5:
    HS_GPIO0->DATAOUT |= (1u << 12);
    HS_GPIO0->OUTENSET = (1u << 12);
    break;
  case 6:
    HS_GPIO0->DATAOUT |= (1u << 11);
    HS_GPIO0->OUTENSET = (1u << 11);
    break;
  }
#endif
  m_hi_idx++;
  if (m_hi_idx > LED_PINNUM)
    m_hi_idx = 1;

  /* pair: output lo */
#if __BOARD_KWD590__
  for (i = 1; i <= LED_PINNUM; i++) {
    if (lo_bitmap & (1u << i)) {
      HS_GPIO0->DATAOUT &= ~(1u << g_u8LedPin[i-1]);
      HS_GPIO0->OUTENSET = (1u << g_u8LedPin[i-1]);
    }
  }
#else
  if (lo_bitmap & (1u << 1)) {
    HS_GPIO1->DATAOUT &= ~(1u << 3);
    HS_GPIO1->OUTENSET = (1u << 3);
  }
  if (lo_bitmap & (1u << 2)) {
    HS_GPIO1->DATAOUT &= ~(1u << 2);
    HS_GPIO1->OUTENSET = (1u << 2);
  }
  if (lo_bitmap & (1u << 3)) {
    HS_GPIO1->DATAOUT &= ~(1u << 1);
    HS_GPIO1->OUTENSET = (1u << 1);
  }
  if (lo_bitmap & (1u << 4)) {
    HS_GPIO0->DATAOUT &= ~(1u << 15);
    HS_GPIO0->OUTENSET = (1u << 15);
  }
  if (lo_bitmap & (1u << 5)) {
    HS_GPIO0->DATAOUT &= ~(1u << 12);
    HS_GPIO0->OUTENSET = (1u << 12);
  }
  if (lo_bitmap & (1u << 6)) {
    HS_GPIO0->DATAOUT &= ~(1u << 11);
    HS_GPIO0->OUTENSET = (1u << 11);
  }
#endif
  return;

  u32Func = pstFrame->u32Function;
  for(i=0; i<LED_FUNC_MAX; i++)
  {
    if(u32Func & 1)
      _led_on(g_u8FunPin[i]);

    u32Func >>= 1;
  }

  for(i=0; i<LED_DIGNUM; i++)
    _led_digDisp(i, pstFrame->u8Digit[i]);
}

void hs_led_initDisp(uint32_t freq)
{
  int i; 
  
  memset(&g_stLed, 0, sizeof(hs_led_t)); 
  for (i=0; i<4; i++)
    g_stLed.stFrame.u8Digit[i] = CHAR_off;
  g_stLed.u32Ms = 1000 / freq;

  g_stScreenPara.u8OnTime = LED_ON_TIME;
  g_stScreenPara.u8Period = 1;
  g_stScreenPara.u32Cnt   = 0;

  hs_padinfo_t stPad = {LED_PIN1, PAD_FUNC_GPIO, CFG_PAD_DIR_INPUT, CFG_PAD_PULL_NO, 0};
  for(i=0; i<LED_PINNUM; i++)
  {
    stPad.u16PadIdx = g_u8LedPin[i];
    hs_pad_config(&stPad);
  }

  hs_cfg_sysListenMsg(HS_CFG_EVENT_BT_SCO_STATUS,  _led_changeDisp);
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

uint8_t hs_led_getDig(uint8_t idx)
{
  idx %= LED_DIGNUM;  
  return g_stLed.stFrame.u8Digit[idx];
}

void hs_led_setBlank(void)
{
  int i;
  for (i=0; i<4; i++)
    g_stLed.stFrame.u8Digit[i] = CHAR_off;
  g_stLed.stFrame.u32Function = 0;
}

#endif

/** @} */
