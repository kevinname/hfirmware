/*
    drvhal - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/drvhal/pwm.h
 * @brief   pwm include file.
 * @details 
 *
 * @addtogroup  drvhal
 * @details 
 * @{
 */
#ifndef __LIB_PWM_H__
#define __LIB_PWM_H__

#include "lib.h"

typedef uint32_t hs_pwmhandle_t;

#ifdef __cplusplus
extern "C" {
#endif

#if HAL_USE_PWM

/*
 * @brief                   open a pwm and init
 *                      
 * @param[in] u8Idx         pwm index. [0, 1]
 * @param[in] period        period of this pwm. 
 * @param[in] u8ActiveLvl   the active level. 0 or 1
 *                      .
 * @return                  a pwm handler
 */
hs_pwmhandle_t hs_pwm_init(uint8_t u8Idx, uint16_t u16Period, uint8_t u8ActiveLvl);

/*
 * @brief               set a channel of the pwm
 *                      
 * @param[in] ehandle   pwm handler
 * @param[in] u8ChnIdx  channel index. [0, 3]
 * @param[in] hWidth    the active level with of the channel in the pwm.
 *                      .
 * @return              0-ok other-error
 */
int hs_pwm_set(hs_pwmhandle_t ehandle, uint8_t u8ChnIdx, uint32_t u32HiWidth);

/*
 * @brief               close the pwm
 *                      
 * @param[in] ehandle   pwm handler
 *                      .
 */
void hs_pwm_uninit(hs_pwmhandle_t ehandle);

#endif

#ifdef __cplusplus
}
#endif

#endif
 /** @} */
