/*
    application - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    app/led_disp.h
 * @brief   led application.
 * @details 
 *
 * @addtogroup  app
 * @details 
 * @{
 */
#ifndef __APP_LED_DISP_H__
#define __APP_LED_DISP_H__

#include "lib.h"

#if HS_USE_LEDDISP

typedef enum
{
  LED_OFF           = 0,
  LED_ON
}hs_led_switch_t;

typedef enum
{
  LED_FUNC_DOT      = 0x00u,
  LED_FUNC_MAX      ,

  LED_FUNC_PLAY     = 0x08u,
  LED_FUNC_PAUSE    ,
  LED_FUNC_USB      ,
  LED_FUNC_SD       ,
  LED_FUNC_COLON    ,
  LED_FUNC_FM       ,
  LED_FUNC_MP3      ,
}hs_led_funidx_t;

#define CHAR_off  18
#define CHAR_sign 10
#define CHAR_b    11
#define CHAR_L    12
#define CHAR_u    13
#define CHAR_e    14
#define CHAR_P    15
#define CHAR_H    16
#define CHAR_A    17
#define CHAR_t    19
#define CHAR_S    20
#define CHAR_d    21
#define CHAR_X    22

void hs_led_initDisp(uint32_t freq);
void hs_led_setFunc(hs_led_funidx_t idx, hs_led_switch_t st);
void hs_led_setDig(uint8_t idx, uint8_t val);
void hs_led_setFuncReverse(hs_led_funidx_t idx);
uint8_t hs_led_getDig(uint8_t idx);
void hs_led_setBlank(void);
__ONCHIP_CODE__ void hs_led_frameDisp(void);

#endif

#endif
 /** @} */
