/*
    ChibiOS - Copyright (C) 2006-2014 Giovanni Di Sirio

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

#ifndef _BOARD_H_
#define _BOARD_H_

#include "stdint.h"

#define __DEVELOPE_BOARD__          0
#define __EVB_BOARD__               0
#define __DEMO_BOARD__              0
#define __NEWEVB_BOARD__            1

#define __CHIP_PACKAGE_48PIN_       0

enum {
  PA0         = 0,
  PA1         ,
  PA2         ,
  PA3         ,
  PA4         ,
  PA5         ,
  PA6         ,
  PA7         ,
  PA8         ,
  PA9         ,
  PA10        ,
  PA11        ,
  PA12        ,
  PA13        ,
  PA14        ,
  PA15        ,
  
  PB0         ,
  PB1         ,
  PB2         ,
  PB3         ,
  PB4         ,
  PB5         ,
  PB6         ,
  PB7         ,

  PIN_NUM
};

enum {

  HS_POWER_MODE_BUTTON    = 0,
  HS_POWER_MODE_SWITCH    ,
};

#if __DEVELOPE_BOARD__

#if __CHIP_PACKAGE_48PIN_
#define HS_PIN_AUX_PLUGIN         PB7
#define HS_PIN_SD_DETECT          PA3
#define HS_PIN_DEEP_SLEEP_WAKEUP  PA0     /* PA0~3 */
#define HS_PIN_AUDIO_PA           PB6     //PB7

#define HS_PIN_UART1_RX           PA4
#define HS_PIN_UART1_TX           PB5

#define HS_CODEC_DRV_MODE         0       /* 0: differential output,  1: single-end output */
#else
#define HS_PIN_AUX_PLUGIN         PA13
#define HS_PIN_SD_DETECT          PA10    //PA14     //PA3
#define HS_PIN_DEEP_SLEEP_WAKEUP  PA0     /* PA0~3 */
#define HS_PIN_AUDIO_PA           PB6     //PB7

#define HS_PIN_UART1_RX           PA7
#define HS_PIN_UART1_TX           PA2

#define HS_CODEC_DRV_MODE         1       /* 0: differential output,  1: single-end output */
#endif

#define HS_PIN_ADC_KEY            PA5     /* PA1~PA5 */
#define HS_PIN_ADC_KEY_EX         PA0     /* PA1~PA5 */

#define HS_ADCKEY_MAXNUM          8

#define HS_ADCKEY1_VOLT           1000
#define HS_ADCKEY2_VOLT           1000
#define HS_ADCKEY3_VOLT           1000
#define HS_ADCKEY4_VOLT           1000
#define HS_ADCKEY5_VOLT           1000
#define HS_ADCKEY6_VOLT           1000    /* if voltage >= 950, the key can't be identify */
#define HS_ADCKEY7_VOLT           1000    /* if voltage >= 950, the key can't be identify */
#define HS_ADCKEY8_VOLT           1000

#endif

#if __EVB_BOARD__
/*
 * EVB Board Pin Define start 
 */
#define HS_PIN_AUX_PLUGIN         PA4
#define HS_PIN_SD_DETECT          PA11
#define HS_PIN_DEEP_SLEEP_WAKEUP  PA0     /* PA0~3 */
#define HS_PIN_AUDIO_PA           PA5     //PB7

#define HS_PIN_UART1_RX           PA7
#define HS_PIN_UART1_TX           PA9


#define HS_CODEC_DRV_MODE         0       /* 0: differential output,  1: single-end output */

/*
 * { 
 * 
 * Just for adc in board
 */
#define HS_PIN_ADC_KEY            PA3     /* PA1~PA5 */
#define HS_PIN_ADC_KEY_EX         PA0     /* PA1~PA5 */

#define HS_ADCKEY_MAXNUM          8

#define HS_ADCKEY1_VOLT           28
#define HS_ADCKEY2_VOLT           245
#define HS_ADCKEY3_VOLT           431
#define HS_ADCKEY4_VOLT           615
#define HS_ADCKEY5_VOLT           764
#define HS_ADCKEY6_VOLT           1000    /* if voltage >= 950, the key can't be identify */
#define HS_ADCKEY7_VOLT           1000    /* if voltage >= 950, the key can't be identify */
#define HS_ADCKEY8_VOLT           1000    /* if voltage >= 950, the key can't be identify */
/* } */

#endif

#if __DEMO_BOARD__

#define HS_PIN_AUX_PLUGIN         PB7
#define HS_PIN_SD_DETECT          PA3
#define HS_PIN_DEEP_SLEEP_WAKEUP  PA0     /* PA0~3 */
#define HS_PIN_AUDIO_PA           PB6

#define HS_PIN_UART1_RX           PA5
#define HS_PIN_UART1_TX           PA7

#define HS_CODEC_DRV_MODE         0       /* 0: differential output,  1: single-end output */

#define HS_PIN_ADC_KEY            PA2     /* PA1~PA5 */
#define HS_PIN_ADC_KEY_EX         PA0     /* PA1~PA5 */

#define HS_ADCKEY_MAXNUM          8

#define HS_ADCKEY1_VOLT           520     /* mode */
#define HS_ADCKEY2_VOLT           167     /* play / stop */
#define HS_ADCKEY3_VOLT           20      /* v++ */
#define HS_ADCKEY4_VOLT           314     /* v-- */
#define HS_ADCKEY5_VOLT           421     /* function */
#define HS_ADCKEY6_VOLT           1000    /* if voltage >= 950, the key can't be identify */
#define HS_ADCKEY7_VOLT           1000    /* if voltage >= 950, the key can't be identify */
#define HS_ADCKEY8_VOLT           1000    /* if voltage >= 950, the key can't be identify */

#endif

#if __NEWEVB_BOARD__
/*
 * EVB Board Pin Define start 
 */
#define HS_PIN_AUX_PLUGIN         PA0

#if __CHIP_PACKAGE_48PIN_
#define HS_PIN_SD_DETECT          PA13    //PA13    // PA10
#else
#define HS_PIN_SD_DETECT          PA10
#endif
#define HS_PIN_DEEP_SLEEP_WAKEUP  PA0     /* PA0~3 */
#define HS_PIN_AUDIO_PA           PB6

#if __CHIP_PACKAGE_48PIN_
#define HS_PIN_UART1_RX           PA9
#define HS_PIN_UART1_TX           PA7
#else
#define HS_PIN_UART1_RX           PB7
#define HS_PIN_UART1_TX           PA1
#endif

#define HS_CODEC_DRV_MODE         1       /* 0: differential output,  1: single-end output */

/*
 * { 
 * 
 * Just for adc in board
 */

#if __CHIP_PACKAGE_48PIN_
#define HS_PIN_ADC_KEY            PA3     /* PA1~PA5 */
#else
#define HS_PIN_ADC_KEY            PA2
#endif
#define HS_PIN_ADC_KEY_EX         PA0     /* PA1~PA5 */

#define HS_ADCKEY_MAXNUM          8

#define HS_ADCKEY1_VOLT           191     /* mode */
#define HS_ADCKEY2_VOLT           361     /* play / stop */
#define HS_ADCKEY3_VOLT           555     /* v++ */
#define HS_ADCKEY4_VOLT           20      /* v-- */
#define HS_ADCKEY5_VOLT           735     /* function */
#define HS_ADCKEY6_VOLT           1000    /* if voltage >= 950, the key can't be identify */
#define HS_ADCKEY7_VOLT           1000    /* if voltage >= 950, the key can't be identify */
#define HS_ADCKEY8_VOLT           1000    /* if voltage >= 950, the key can't be identify */
/* } */

#endif

/*
 * { 
 * 
 * Just for adc in board
 */

#define HS_BATTERY_FULL_ALARM     4100.0  /* when battery voltage >= the value, alarmed! unit: mv */
#define HS_BATTERY_EMPTY_ALARM    3000.0  /* when battery voltage <= the value, alarmed! unit: mv */

#define HS_TEMPERATURE_MAX_ALARM  80.0    /* when chip temperature >= the value, alarmed! unit: ¡æ */
#define HS_TEMPERATURE_MIN_ALARM  -30.0   /* when chip temperature <= the value, alarmed! unit: ¡æ */
/*
 * }
 */

#ifndef HS_POWER_MODE
/* 
 * if on button mode, 1->0(3s)->1 power off, 1->0(3s) power on. it can't be changed 
 * if on switch mode, 1-power off, 0-power on. it can't be changed 
 */
#define HS_POWER_MODE             HS_POWER_MODE_BUTTON    
#endif

/*
 * Board identifier.
 */
#define BOARD_NAME                "Huntersun HS6601"


#ifdef __cplusplus
extern "C" {
#endif

void hs_boardInit(void);
void hs_boardUninit(void);

float hs_boardGetKeyVolt(uint8_t u8Idx);
void boardKickWatchDog(void);

#ifdef __cplusplus
}
#endif

#endif /* _BOARD_H_ */
