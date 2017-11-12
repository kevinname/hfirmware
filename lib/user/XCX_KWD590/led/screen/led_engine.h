/*
    application - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    app/led_engine.h
 * @brief   led application.
 * @details 
 *
 * @addtogroup  app
 * @details 
 * @{
 */
#ifndef __APP_LED_ENGINE_H__
#define __APP_LED_ENGINE_H__

#include "lib.h"
#include "led_disp.h"

#if HS_USE_LEDDISP

#define LED_ON_TIME         5          /* unit: us */

#define LED_DISP_MASK       0xfff
typedef enum
{
  LED_DISP_MODE_CHANGE    = 0,
  LED_DISP_MODE_IDLE      ,
  LED_DISP_VOLUME         ,
  LED_DISP_FREQ           ,
  LED_DISP_MUSICIDX       ,
  LED_DISP_PHONE_NUM      ,

  LED_DISP_NUM
}hs_led_dispType_t;

void hs_led_init(void);
void hs_led_disp(hs_led_dispType_t eMsg);


#endif

#endif
 /** @} */
