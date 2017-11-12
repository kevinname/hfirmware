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
/*
   Concepts and parts of this file have been contributed by Fabio Utzig and
   Xo Wang.
 */

/**
 * @file    hs66xx/icu_lld.h
 * @brief   ICU Driver subsystem low level driver header.
 *
 * @addtogroup ICU
 * @{
 */

#ifndef _ICU_LLD_H_
#define _ICU_LLD_H_

#if HAL_USE_ICU || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   ICUD0 driver enable switch.
 * @details If set to @p TRUE the support for ICUD0 is included.
 */
#if !defined(HS_ICU_USE_TIM0) || defined(__DOXYGEN__)
#define HS_ICU_USE_TIM0               FALSE
#endif

/**
 * @brief   ICUD1 driver enable switch.
 * @details If set to @p TRUE the support for ICUD1 is included.
 */
#if !defined(HS_ICU_USE_TIM1) || defined(__DOXYGEN__)
#define HS_ICU_USE_TIM1               FALSE
#endif

/**
 * @brief   ICUD2 driver enable switch.
 * @details If set to @p TRUE the support for ICUD2 is included.
 */
#if !defined(HS_ICU_USE_TIM2) || defined(__DOXYGEN__)
#define HS_ICU_USE_TIM2               FALSE
#endif

/**
 * @brief   ICUD3 driver enable switch.
 * @details If set to @p TRUE the support for ICUD3 is included.
 */
#if !defined(HS_ICU_USE_TIM3) || defined(__DOXYGEN__)
#define HS_ICU_USE_TIM3               FALSE
#endif

/**
 * @brief   ICUD0 interrupt priority level setting.
 */
#if !defined(HS_ICU_TIM0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define HS_ICU_TIM0_IRQ_PRIORITY         3
#endif

/**
 * @brief   ICUD1 interrupt priority level setting.
 */
#if !defined(HS_ICU_TIM1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define HS_ICU_TIM1_IRQ_PRIORITY         3
#endif

/**
 * @brief   ICUD2 interrupt priority level setting.
 */
#if !defined(HS_ICU_TIM2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define HS_ICU_TIM2_IRQ_PRIORITY         3
#endif

/**
 * @brief   ICUD3 interrupt priority level setting.
 */
#if !defined(HS_ICU_TIM3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define HS_ICU_TIM3_IRQ_PRIORITY         3
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !HS_ICU_USE_TIM0 && !HS_ICU_USE_TIM1 &&                              \
    !HS_ICU_USE_TIM2 && !HS_ICU_USE_TIM3
#error "ICU driver activated but no TIM peripheral assigned"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief ICU driver mode.
 */
typedef enum {
  ICU_INPUT_ACTIVE_HIGH = 0,        /**< Trigger on rising edge.            */
  ICU_INPUT_ACTIVE_LOW = 1,         /**< Trigger on falling edge.           */
} icumode_t;

/**
 * @brief   ICU frequency type.
 */
typedef uint32_t icufreq_t;

/**
 * @brief   ICU channel type.
 */
typedef enum {
  ICU_CHANNEL_1 = 0,              /**< Use TIMxCH1.      */
  ICU_CHANNEL_2 = 1,              /**< Use TIMxCH2.      */
} icuchannel_t;

/**
 * @brief   ICU counter type.
 */
typedef uint16_t icucnt_t;

#if ICU_USE_DMA

/**
 * @brief   PWM DMA priority (0..3|lowest..highest).
 * @note    The priority level is used for pwm DMA channels but
 *          because of the channels ordering the RX channel has always priority
 *          over the TX channel.
 */
#if !defined(HS_IR_RC_PWM_DMA_PRIORITY) || defined(__DOXYGEN__)
#define  HS_ICU_DMA_PRIORITY     1
#endif

typedef void  (*icuDMAcallback_t)(void*p);

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
  uint8_t                   startRegNum;
  /**
   * @brief auto update register value in memory address.
   */
  void                      *memAddr;
  /**
   * @brief auto update start register to stop register length.
   */
  uint8_t                    RegLength;
  /**
   * @brief update per.
   */
  bool                       periodU;
  /**
   * @brief auto update start register to stop register length.
   */
  bool                       widthU;
  /**
   * @brief pwm dma callback function.
   */
  icuDMAcallback_t          callback;  
} ICUDMAinfo;
#endif

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {
  /**
   * @brief   Driver mode.
   */
  icumode_t                 mode;
  /**
   * @brief   Timer clock in Hz.
   * @note    The low level can use assertions in order to catch invalid
   *          frequency specifications.
   */
  icufreq_t                 frequency;
  /**
   * @brief   Callback for pulse width measurement.
   */
  icucallback_t             width_cb;
  /**
   * @brief   Callback for cycle period measurement.
   */
  icucallback_t             period_cb;
  /**
   * @brief   Callback for timer overflow.
   */
  icucallback_t             overflow_cb;
  /* End of the mandatory fields.*/
  /**
   * @brief   Timer input channel to be used.
   * @note    Only inputs TIMx 1 and 2 are supported.
   */
  icuchannel_t              channel;
  /**
   * @brief TIM DIER register initialization data.
   * @note  The value of this field should normally be equal to zero.
   * @note  Only the DMA-related bits can be specified in this field.
   */
  uint32_t                  dier;

#if ICU_USE_DMA        
   /**
    * @brief USE DMA update timer registers .
    * @note this value is TRUE or FALSE
    */
   bool_t                   useDMA;
   /**
    * @brief pwm usb DMA info .
    */	
   ICUDMAinfo	            *icuDMAinfo;
#endif     
} ICUConfig;

/**
 * @brief   Structure representing an ICU driver.
 */
struct ICUDriver {
  /**
   * @brief Driver state.
   */
  icustate_t                state;
  /**
   * @brief Current configuration data.
   */
  const ICUConfig           *config;
#if defined(ICU_DRIVER_EXT_FIELDS)
  ICU_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief Timer base clock.
   */
  uint32_t                  clock;
  /**
   * @brief Pointer to the TIMx registers block.
   */
  HS_TIM_Type               *tim;
  /**
   * @brief CCR register used for width capture.
   */
  volatile uint32_t         *wccrp;
  /**
   * @brief CCR register used for period capture.
   */
  volatile uint32_t         *pccrp;
        
#if ICU_USE_DMA          
    /**
   * @brief icu DMA channel.
   */
  hs_dma_stream_t           *icudma;
#endif        
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Returns the width of the latest pulse.
 * @details The pulse width is defined as number of ticks between the start
 *          edge and the stop edge.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 * @return              The number of ticks.
 *
 * @notapi
 */
#define icu_lld_get_width(icup) (*((icup)->wccrp) + 1)

/**
 * @brief   Returns the width of the latest cycle.
 * @details The cycle width is defined as number of ticks between a start
 *          edge and the next start edge.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 * @return              The number of ticks.
 *
 * @notapi
 */
#define icu_lld_get_period(icup) (*((icup)->pccrp) + 1)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if HS_ICU_USE_TIM0 && !defined(__DOXYGEN__)
extern ICUDriver ICUD0;
#endif

#if HS_ICU_USE_TIM1 && !defined(__DOXYGEN__)
extern ICUDriver ICUD1;
#endif

#if HS_ICU_USE_TIM2 && !defined(__DOXYGEN__)
extern ICUDriver ICUD2;
#endif

#if HS_ICU_USE_TIM3 && !defined(__DOXYGEN__)
extern ICUDriver ICUD3;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void icu_lld_init(void);
  void icu_lld_start(ICUDriver *icup);
  void icu_lld_stop(ICUDriver *icup);
  void icu_lld_enable(ICUDriver *icup);
  void icu_lld_disable(ICUDriver *icup);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_ICU */

#endif /* _ICU_LLD_H_ */

/** @} */
