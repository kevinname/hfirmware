/*
    fm tx demo - Copyright (C) 2012~2016 HunterSun Technologies
                 wei.lu@huntersun.com.cn
 */

/**
 * @file    hs6760_main.c
 * @brief   hs6760 FM tx demo with PWM as clock input
 * @details 
 *
 * @addtogroup  user
 * @details 
 * @{
 */

#include "lib.h"
#include "hs6760.h"

#if HAL_USE_I2C && HS_USE_HS6760

static const PWMConfig m_pwm_cfg_fmclk12mhz = {
  0,                               /* 0 means no div in TIM from XTAL@24MHz */
  2,                               /* period in 2-cycle (min)  */
  NULL,
  {
   {PWM_OUTPUT_ACTIVE_HIGH, NULL, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL, NULL},
  },
  0,
  0,
  0
#if PWM_USE_DMA
  ,
  FALSE,
  0
#endif
};

void hs_fmtx_init(void)
{
  /* drvhal's pwm cannot setup output@12MHz to HS6760 */
  pwmStart(&PWMD0, &m_pwm_cfg_fmclk12mhz);
  pwmEnableChannel(&PWMD0, 3/*channel*/, 1/*width*/);

  /* FM transmitter is always open, so no pwmStop() */
  #if __BOARD_KWD590__
  hs6760_fm_open(XTAL_12M);
  #else //auevbv2
  hs6760_fm_open(XTAL_24M);
  #endif
}

#endif
