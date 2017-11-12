/*
    drvhal - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/drvhal/pwm.c
 * @brief   pwm file.
 * @details 
 *
 * @addtogroup  drvhal
 * @details 
 * @{
 */

#include "lib.h"

#if HAL_USE_PWM

#define PWM_US_CYCLE        1ul
#define PWM_FREQUENCY       (PWM_US_CYCLE * 1000 * 1000)
#define PWM_CHK_FLAG        0x55aa

typedef struct
{
  PWMDriver    *pstPwm;
  PWMConfig     stPwmCfg;  

  uint32_t      u32Period;     /* unit : ms */

  uint16_t      u16ActiveLvl;
  uint16_t      u16ChkFlg;
}hs_pwmpara_t;

static inline PWMDriver *_pwm_getHandler(uint8_t u8PwmIdx)
{
  PWMDriver *pstPwm;

  pstPwm = 
    #if HS_PWM_USE_TIM0
    (u8PwmIdx == 0) ? &PWMD0 : 
    #endif
      #if HS_PWM_USE_TIM1
      (u8PwmIdx == 1) ? &PWMD1 : 
      #endif
        #if HS_PWM_USE_TIM2
        (u8PwmIdx == 2) ? &PWMD2 : 
        #endif
          NULL;
        
  return pstPwm;
}

static void _pwm_initConfig(hs_pwmpara_t *pstHandle)
{
  PWMConfig *pstConfig = &pstHandle->stPwmCfg;
  uint16_t i, u16ActiveLvl;

  u16ActiveLvl = pstHandle->u16ActiveLvl == 0 ? 
                         PWM_OUTPUT_ACTIVE_LOW : 
                         PWM_OUTPUT_ACTIVE_HIGH;
                         
  pstConfig->frequency = PWM_FREQUENCY;
  pstConfig->period    = pstHandle->u32Period * PWM_US_CYCLE;
  for(i=0; i<PWM_CHANNELS; i++)
    pstConfig->channels[i].mode = u16ActiveLvl;    
}

/*
 * @brief                   open a pwm and init
 *                      
 * @param[in] u8Idx         pwm index. [0, 1]
 * @param[in] period        period of this pwm. 
 * @param[in] u8ActiveLvl   the active level. 0 or 1
 *                      .
 * @return                  a pwm handler
 */
hs_pwmhandle_t hs_pwm_init(uint8_t u8Idx, uint16_t u16Period, uint8_t u8ActiveLvl)
{
  hs_pwmpara_t *pstHandle;
  int res;

  pstHandle = hs_malloc(sizeof(hs_pwmpara_t), __MT_Z_GENERAL);
  if(!pstHandle)
    return 0;

  pstHandle->pstPwm = _pwm_getHandler(u8Idx);
  if(!pstHandle->pstPwm)
  {
    hs_free(pstHandle);
    return 0;
  }

  pstHandle->u16ChkFlg = PWM_CHK_FLAG;
  pstHandle->u16ActiveLvl = u8ActiveLvl;
  pstHandle->u32Period  = u16Period;
  _pwm_initConfig(pstHandle);  

  pwmStop(pstHandle->pstPwm);
  res = pwmStart(pstHandle->pstPwm, &pstHandle->stPwmCfg);
  if(res != 0)
  {
    hs_free(pstHandle);
    return 0;
  }

  return (hs_pwmhandle_t)pstHandle;
}

/*
 * @brief               set a channel of the pwm
 *                      
 * @param[in] ehandle   pwm handler
 * @param[in] u8ChnIdx  channel index. [0, 3]
 * @param[in] hWidth    the active level with of the channel in the pwm, unit:ms.
 *                      .
 * @return              0-ok other-error
 */
int hs_pwm_set(hs_pwmhandle_t ehandle, uint8_t u8ChnIdx, uint32_t u32HiWidth)
{
  hs_pwmpara_t *pstHandle = (hs_pwmpara_t *)ehandle;
  
  if ((!pstHandle) || (pstHandle->u16ChkFlg != PWM_CHK_FLAG)
    || (u8ChnIdx >= PWM_CHANNELS))
    return -1;

  pwmEnableChannel(pstHandle->pstPwm, u8ChnIdx, u32HiWidth * PWM_US_CYCLE);
  return 0;
}

/*
 * @brief               close the pwm
 *                      
 * @param[in] ehandle   pwm handler
 *                      .
 */
void hs_pwm_uninit(hs_pwmhandle_t ehandle)
{
  hs_pwmpara_t *pstHandle = (hs_pwmpara_t *)ehandle;
  
  if ((!pstHandle) || (pstHandle->u16ChkFlg != PWM_CHK_FLAG))
    return ;

  pstHandle->u16ChkFlg = 0;
  pwmStop(pstHandle->pstPwm);
  hs_free(pstHandle);  
}

#endif

/** @} */
