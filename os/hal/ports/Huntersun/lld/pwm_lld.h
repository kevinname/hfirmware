/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio
                 Copyright (C) 2014 HunterSun Technologies
                 wei.lu@huntersun.com.cn

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

/**
 * @file    hs66xx/pwm_lld.h
 * @brief   PWM Driver subsystem low level driver header.
 *
 * @addtogroup PWM
 * @{
 */

#ifndef _PWM_LLD_H_
#define _PWM_LLD_H_

#if HAL_USE_PWM || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   Number of PWM channels per PWM driver.
 */
#define PWM_CHANNELS                        4

/**
 * @brief   Complementary output modes mask.
 */
#define PWM_COMPLEMENTARY_OUTPUT_MASK           0xF0

/**
 * @brief   Complementary output not driven.
 */
#define PWM_COMPLEMENTARY_OUTPUT_DISABLED       0x00

/**
 * @brief   Complementary output, active is logic level one.
 */
#define PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH    0x10

/**
 * @brief   Complementary output, active is logic level zero.
 */
#define PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW     0x20

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   PWMD0 driver enable switch.
 * @details If set to @p TRUE the support for PWMD0 is included.
 */
#if !defined(HS_PWM_USE_TIM0) || defined(__DOXYGEN__)
#define HS_PWM_USE_TIM0               FALSE
#endif

/**
 * @brief   PWMD1 driver enable switch.
 * @details If set to @p TRUE the support for PWMD1 is included.
 */
#if !defined(HS_PWM_USE_TIM1) || defined(__DOXYGEN__)
#define HS_PWM_USE_TIM1               FALSE
#endif

/**
 * @brief   PWMD2 driver enable switch.
 * @details If set to @p TRUE the support for PWMD2 is included.
 */
#if !defined(HS_PWM_USE_TIM2) || defined(__DOXYGEN__)
#define HS_PWM_USE_TIM2               FALSE
#endif

/**
 * @brief   PWMD0 interrupt priority level setting.
 */
#if !defined(HS_PWM_TIM0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define HS_PWM_TIM0_IRQ_PRIORITY         3
#endif

/**
 * @brief   PWMD1 interrupt priority level setting.
 */
#if !defined(HS_PWM_TIM1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define HS_PWM_TIM1_IRQ_PRIORITY         3
#endif

/**
 * @brief   PWMD2 interrupt priority level setting.
 */
#if !defined(HS_PWM_TIM2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define HS_PWM_TIM2_IRQ_PRIORITY         3
#endif

#if PWM_USE_DMA
/**
 * @brief   PWM DMA priority (0..3|lowest..highest).
 * @note    The priority level is used for pwm DMA channels but
 *          because of the channels ordering the RX channel has always priority
 *          over the TX channel.
 */
#if !defined(HS_IR_RC_PWM_DMA_PRIORITY) || defined(__DOXYGEN__)
#define  HS_PWM_DMA_PRIORITY     1
#endif
#endif
/** @} */

/** @} */

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief PWM mode type.
 */
typedef uint32_t pwmmode_t;

/**
 * @brief   PWM channel type.
 */
typedef uint8_t pwmchannel_t;

/**
 * @brief   PWM counter type.
 */
typedef uint32_t pwmcnt_t;

#if PWM_USE_DMA
typedef void (*pwmDMAcallback_t)(void);
#endif

/**
 * @brief   PWM driver channel configuration structure.
 * @note    Some architectures may not be able to support the channel mode
 *          or the callback, in this case the fields are ignored.
 */
typedef struct {
  /**
   * @brief Channel active logic level.
   */
  pwmmode_t                 mode;	
  /**
   * @brief Channel callback pointer.
   * @note  This callback is invoked on the channel compare event. If set to
   *        @p NULL then the callback is disabled.
   */
  pwmcallback_t             callback;

  void                      *arg;
  /* End of the mandatory fields.*/
} PWMChannelConfig;

#if PWM_USE_DMA
/**
 * @brief   PWM driver DMA configuration structure.
 * @note    PWM DMA is used to auto update timer registers 
 */
typedef struct {
  /**
   * @brief auto update timer register blockSize
   */
  uint8_t                   blockSize;	
  /**
   * @brief auto update timer register start number
   */
  uint8_t                   startRegrNum;
  /**
   * @brief auto update register value in memory address.
   */
  void                      *memAddr;
  /**
   * @brief auto update start register to stop register length.
   */
  uint8_t                    RegLength;
  /**
   * @brief pwm dma callback function.
   */
   pwmDMAcallback_t            pwmDMAcallback;    
} PWMDMAinfo;
#endif

/**
 * @brief   Driver configuration structure.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
typedef struct {
  /**
   * @brief   Timer clock in Hz.
   * @note    The low level can use assertions in order to catch invalid
   *          frequency specifications.
   */
  uint32_t                  frequency;
  /**
   * @brief   PWM period in ticks.
   * @note    The low level can use assertions in order to catch invalid
   *          period specifications.
   */
  pwmcnt_t                  period;
  /**
   * @brief Periodic callback pointer.
   * @note  This callback is invoked on PWM counter reset. If set to
   *        @p NULL then the callback is disabled.
   */
  pwmcallback_t             callback;
  /**
   * @brief Channels configurations.
   */
  PWMChannelConfig          channels[PWM_CHANNELS];
  /* End of the mandatory fields.*/
  /**
   * @brief TIM CR2 register initialization data.
   * @note  The value of this field should normally be equal to zero.
   */
  uint32_t                  cr2;
  /**
   * @brief TIM BDTR (break & dead-time) register initialization data.
   * @note  The value of this field should normally be equal to zero.
   */                                                                     \
   uint32_t                 bdtr;
   /**
    * @brief TIM DIER register initialization data.
    * @note  The value of this field should normally be equal to zero.
    * @note  Only the DMA-related bits can be specified in this field.
    */
   uint32_t                 dier;
#if PWM_USE_DMA        
   /**
    * @brief USE DMA update timer registers .
    * @note this value is TRUE or FALSE
    */
   bool_t                   useDMA;
   /**
    * @brief pwm usb DMA info .
    */	
   PWMDMAinfo	            *pwmDMAinfo;
#endif        
} PWMConfig;

/**
 * @brief   Structure representing an PWM driver.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
struct PWMDriver {
  /**
   * @brief Driver state.
   */
  pwmstate_t                state;
  /**
   * @brief Current configuration data.
   */
  const PWMConfig           *config;
  /**
   * @brief   Current PWM period in ticks.
   */
  pwmcnt_t                  period;
#if defined(PWM_DRIVER_EXT_FIELDS)
  PWM_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief Timer base clock.
   */
  uint32_t                  clock;
  /**
   * @brief Pointer to the PWMx registers block.
   */
  HS_TIM_Type               *tim;
        
#if PWM_USE_DMA        
  /**
   * @brief pwm DMA channel.
   */
  hs_dma_stream_t           *pwmdma;
#endif
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Changes the period the PWM peripheral.
 * @details This function changes the period of a PWM unit that has already
 *          been activated using @p pwmStart().
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @post    The PWM unit period is changed to the new value.
 * @note    The function has effect at the next cycle start.
 * @note    If a period is specified that is shorter than the pulse width
 *          programmed in one of the channels then the behavior is not
 *          guaranteed.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] period    new cycle time in ticks
 *
 * @notapi
 */
#define pwm_lld_change_period(pwmp, period)                                 \
  ((pwmp)->tim->ARR = (uint16_t)((period) - 1))

/**
 * @brief   Returns a PWM channel status.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] channel   PWM channel identifier (0...PWM_CHANNELS-1)
 *
 * @notapi
 */
#define pwm_lld_is_channel_enabled(pwmp, channel)                             \
  (((pwmp)->tim->CCR[channel] != 0) ||                                      \
   (((pwmp)->tim->DIER & (2 << channel)) != 0))

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if HS_PWM_USE_TIM0 && !defined(__DOXYGEN__)
extern PWMDriver PWMD0;
#endif

#if HS_PWM_USE_TIM1 && !defined(__DOXYGEN__)
extern PWMDriver PWMD1;
#endif

#if HS_PWM_USE_TIM2 && !defined(__DOXYGEN__)
extern PWMDriver PWMD2;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void pwm_lld_init(void);
  void pwm_lld_start(PWMDriver *pwmp);
  void pwm_lld_stop(PWMDriver *pwmp);
  void pwm_lld_enable_channel(PWMDriver *pwmp,
                              pwmchannel_t channel,
                              pwmcnt_t width);
  void pwm_lld_disable_channel(PWMDriver *pwmp, pwmchannel_t channel);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_PWM */

#endif /* _PWM_LLD_H_ */

/** @} */
