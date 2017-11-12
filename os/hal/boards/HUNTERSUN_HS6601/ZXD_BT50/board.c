/*
    ChibiOS - Copyright (C) 2006-2014 Giovanni Di Sirio
              Copyright (C) 2014-2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "lib.h"

#if HAL_USE_CODEC || defined(__DOXYGEN__)

static const hs_codec_eqpara_t g_stDefaultEq =
{
  {80, 320, 640, 1280, 2560, 5120, 10240},
  {0,  3,   3,   0,    -3,   -3,    0},
};

static const hs_codec_boardinfo_t g_stCodecInfo = 
{
  HS_PIN_AUDIO_PA, 
  0,                    /* PA on level */
  
  0,                    /* drv gain, 0: -6db, 1: 0db */
  1,                    /* drv output, 0: differential output,  1: single-end output */
  TRACK_RR,             /* mic select, 28pin-package: TRACK_LL, 48pin-package: TRACK_RR */

  1,                    /* pga gain for linein */
  5,                    /* pga gain for mic */

  1,                    /* package select, Current packets of HS6601A2 all select '1' */

  0,                    /* mixed left and right of dac enable */
  0,                    /* mixed left and right of adc enable */

  0,                   	/* dac <=  26db */ 
  -42,                  /* dac >= -47db */

  24,                   /* adc <=  24db */
  -20,                  /* adc >= -47db */

  -9,                   /* hfp max gain of playing */
  24,                    /* adc default gain, just for hfp */

  &g_stDefaultEq  ,
};
#endif

static const hs_drvhal_cfg_t g_stDrvhalCfg =
{
  1,                  /* when low power, 0-really power-down 1-deep sleep instead of power-down */
  HS_POWER_MODE,      /* HS_POWER_MODE_BUTTON: button;  HS_POWER_MODE_SWITCH: switch */

  HS_PIN_SD_DETECT,   /* sd detect pin define */
  0,                  /* detect level */

  HS_PIN_AUX_PLUGIN,  /* aux detect pin define */
  0,                  /* detect level */

  HS_PIN_ADC_KEY,     /* adc key pin define */
  HS_PIN_ADC_KEY_EX,  /* extend adc key pin define */
  
  1,                  /* 0-disable auto init sd-card */  
  1,                  /* sdio drv capacity, 0~3 */

  1,                  /* 0-all usb device disable in sdk, include host and device */
  1,                  /* 0-disable usb-device */  

  30,                 /* unit: s; the time to power down when reporting low-power */

  ADC_GAIN_MULTI_1,   /* the gain of adc key, default ADC_GAIN_MULTI_1 */
  50.0,               /* adc key voltage range field, when a key press-down */

  HS_BATTERY_FULL_ALARM, 
  HS_BATTERY_EMPTY_ALARM,

  HS_TEMPERATURE_MAX_ALARM,
  HS_TEMPERATURE_MIN_ALARM,
};

static const SFConfig g_stSfCfg = 
{
    NULL,
    WIDTH_8BIT,
    SELECT_CS0,
    CLKMODE_0,
    LOW_ACTIVE,
    64000000,
};

#if HAL_USE_SDC || defined(__DOXYGEN__)
static const SDCConfig g_stSdCfg = 
{
  24000000
};
#endif

static const hs_padinfo_t g_stPadDefault[] = 
{
  //{HS_PIN_UART1_TX,   PAD_FUNC_UART1_TX,  CFG_PAD_DIR_OUTPUT,   CFG_PAD_PULL_UP,    3},
  //{HS_PIN_UART1_RX,   PAD_FUNC_UART1_RX,  CFG_PAD_DIR_INPUT,    CFG_PAD_PULL_UP,    3},

  /* sd card */
  {PA6,               PAD_FUNC_SD_USB,    CFG_PAD_DIR_INPUT,    CFG_PAD_PULL_UP,      3},
  {PA8,               PAD_FUNC_SD_USB,    CFG_PAD_DIR_INPUT,    CFG_PAD_PULL_UP,      3},
  {PA10,              PAD_FUNC_SD_USB,    CFG_PAD_DIR_INPUT,    CFG_PAD_PULL_UP,      3},

  /* usb */
  {PA14,              PAD_FUNC_SD_USB,    CFG_PAD_DIR_INPUT,    CFG_PAD_PULL_DOWN,    3},
  {PB0,               PAD_FUNC_SD_USB,    CFG_PAD_DIR_INPUT,    CFG_PAD_PULL_DOWN,    3},
  {PB1,               PAD_FUNC_SD_USB,    CFG_PAD_DIR_INPUT,    CFG_PAD_PULL_UP,    3},
  
  {PB2,               PAD_FUNC_GPIO,      CFG_PAD_DIR_OUTPUT,    CFG_PAD_PULL_UP,      3},

  {PA2,               PAD_FUNC_GPIO,      CFG_PAD_DIR_INPUT,    CFG_PAD_PULL_UP,      3},
  {PA3,               PAD_FUNC_GPIO,      CFG_PAD_DIR_INPUT,    CFG_PAD_PULL_UP,      3},
  {PA4,               PAD_FUNC_GPIO,      CFG_PAD_DIR_INPUT,    CFG_PAD_PULL_UP,      3},
};

static const float g_fKeyVoltage[HS_ADCKEY_MAXNUM] = /* unit: mv */
{
  HS_ADCKEY1_VOLT, HS_ADCKEY2_VOLT,
  HS_ADCKEY3_VOLT, HS_ADCKEY4_VOLT,
  HS_ADCKEY5_VOLT, HS_ADCKEY6_VOLT,
  HS_ADCKEY7_VOLT, HS_ADCKEY8_VOLT,
};

void _board_pinmuxInit(void) 
{
  uint32_t i, u32PinNum = sizeof(g_stPadDefault) / sizeof(hs_padinfo_t);
  HS_PMU_Type *pstPmu = HS_PMU;

  for(i=0; i<u32PinNum; i++)
    hs_pad_config(&g_stPadDefault[i]);
  
  hs_pad_init();
  pstPmu->PADC_CSF &= ~(3u << 3);
}

/* analog circuit calibration routines will take effect XTAL, audio CODEC, BT, FM etc */
void _board_anaCalibration(void)
{
  pmu_cali_rc();
  pmu_cali_ldo();
}

void hs_boardInit(void)
{
  _board_anaCalibration();
  _board_pinmuxInit();
  
  sfStart(&SFD, &g_stSfCfg);
  sfProbe(&SFD, TIME_INFINITE);
  
  #if HAL_USE_SERIAL && HS_SERIAL_USE_UART1
  sdStart(&SD1, NULL);
  #endif

  #if HAL_USE_SDC
  sdcStart(&SDCD0, &g_stSdCfg);
  #endif

  #if HAL_USE_AUDIO
  audioStart();
  #endif
  
  hs_drvhal_init();
}

void hs_boardUninit(void)
{
  hs_drvhal_uninit(); 
    
  sfStop(&SFD);

  #if HAL_USE_SDC
  sdcStop(&SDCD0);
  #endif  
}

NOINLINE float hs_boardGetKeyVolt(uint8_t u8Idx)
{
  if(u8Idx >= HS_ADCKEY_MAXNUM)
    return 1000.0;

  return g_fKeyVoltage[u8Idx];
}

NOINLINE void boardKickWatchDog(void)
{
  #if HAL_USE_WDT || defined(__DOXYGEN__)
  wdtKickdog();
  #else
  __NOP();
  #endif
}

#if HAL_USE_CODEC || defined(__DOXYGEN__)
NOINLINE const hs_codec_boardinfo_t *hs_boardGetCodecInfo(void)
{
  return &g_stCodecInfo;
}
#endif

NOINLINE const hs_drvhal_cfg_t *hs_boardGetDrvCfg(void)
{
  return &g_stDrvhalCfg;
}


/*
 * }
 */

