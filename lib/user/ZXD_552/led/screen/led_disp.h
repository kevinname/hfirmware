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
  LED_FUNC_PLAY     = 0x00u,
  LED_FUNC_PAUSE    ,
  LED_FUNC_USB      ,
  LED_FUNC_SD       ,
  LED_FUNC_COLON    ,
  LED_FUNC_FM       ,
  LED_FUNC_MP3      ,

  LED_FUNC_MAX
}hs_led_funidx_t;

void hs_led_initDisp(uint32_t freq);
void hs_led_setFunc(hs_led_funidx_t idx, hs_led_switch_t st);
void hs_led_setDig(uint8_t idx, uint8_t val);
void hs_led_setFuncReverse(hs_led_funidx_t idx);
__ONCHIP_CODE__ void hs_led_frameDisp(void);

#endif

#endif
 /** @} */
