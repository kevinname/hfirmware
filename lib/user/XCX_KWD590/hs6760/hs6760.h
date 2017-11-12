/*
    application - Copyright (C) 2012~2016 HunterSun Technologies
                 yufeng.yao@huntersun.com.cn
 */

/**
 * @file    user/hs6760.h
 * @brief   HS6760 FM transmitter APIs.
 * @details 
 *
 * @addtogroup  user
 * @details 
 * @{
 */
#ifndef __HS6760_H__
#define __HS6760_H__

#ifdef __cplusplus
extern "C" {
#endif

#if HAL_USE_I2C && HS_USE_HS6760

#include "lib_i2c.h"

typedef enum {
  XTAL_32K768 = 0,
  XTAL_7M6,
  XTAL_12M,
  XTAL_24M,
} hs6760_xtal_t;

typedef enum {
  DEV_75K = 0,
  DEV_50K,
  DEV_22K5,
} hs6760_freq_dev_t;

typedef enum {
  NORMAL_MODE = 0,
  MUTE_MODE,
  SLEEP_MODE,
} hs6760_mode_t;

hs_i2c_handler_t hs6760_fm_open(hs6760_xtal_t xtal);
int hs6760_fm_close(void);
int hs6760_fm_set_freq(int freq_dmhz);    //1039=103.9MHz
int hs6760_fm_set_mode(hs6760_mode_t mode);

void hs_fmtx_init(void);

#endif

#ifdef __cplusplus
}
#endif

#endif
 /** @} */
