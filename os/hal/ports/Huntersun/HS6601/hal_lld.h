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
 * @file    hs66xx/hal_lld.h
 * @brief   HAL subsystem low level driver header template.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef _HAL_LLD_H_
#define _HAL_LLD_H_

#include "hs6601.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   Defines the support for realtime counters in the HAL.
 */
#define HAL_IMPLEMENTS_COUNTERS TRUE

/**
 * @name    Platform identification
 * @{
 */
#define PLATFORM_NAME           "HS6601 Bluetooth Soundbox"
/** @} */

/*===========================================================================*/
/* Platform capabilities.                                                    */
/*===========================================================================*/

/**
 * @name    HS66xx capabilities
 * @{
 */
/* GPIO attributes.*/
#define HS_HAS_GPIO0         TRUE
#define HS_HAS_GPIO1         TRUE

#if 0
/* I2C attributes.*/
#define HS_HAS_I2C0          TRUE
#define HS_I2C0_RX_DMA_ID    5
#define HS_I2C0_TX_DMA_ID    4

/* RTC attributes.*/
#define HS_HAS_RTC           TRUE

/* SDIO attributes.*/
#define HS_HAS_SDIO          TRUE

/* SPI attributes.*/
#define HS_HAS_SPI0          TRUE
#define HS_SPI0_RX_DMA_ID    7
#define HS_SPI0_TX_DMA_ID    6

#define HS_HAS_SPI1          TRUE
#define HS_SPI1_RX_DMA_ID    9
#define HS_SPI1_TX_DMA_ID    8
#endif

/* UART attributes.*/
#define HS_HAS_UART0         TRUE

#define HS_HAS_UART1         TRUE
#define HS_UART1_RX_DMA_ID   3
#define HS_UART1_TX_DMA_ID   2

#if 0
#define HS_I2S_RX_DMA_ID     1
#define HS_I2S_TX_DMA_ID     0

/* USB attributes.*/
#define HS_HAS_USB           FALSE
#define HS_HAS_OTG           TRUE
#endif

/** @} */

/*===========================================================================*/
/* Platform specific friendly IRQ names.                                     */
/*===========================================================================*/

/**
 * @name    IRQ VECTOR names
 * @{
 */
#define BT_IRQHandler           Vector00
#define CODEC_IRQHandler        Vector01
#define DMAC_IRQHandler         Vector02
#define GPIO_IRQHandler         Vector03
#define TIM_IRQHandler          Vector04
#define OTG_IRQHandler          Vector05
#define SDHC_IRQHandler         Vector06
#define RTC_IRQHandler          Vector07
#define UART0_IRQHandler        Vector08
#define UART1_IRQHandler        Vector09
#define I2C0_IRQHandler         Vector10
#define I2S_IRQHandler          Vector11
#define ADC_IRQHandler          Vector12
#define SPI0_IRQHandler         Vector13
#define SPI1_IRQHandler         Vector14
#define PHY_IRQHandler          Vector15  //15

#define TICK_IRQHandler         Vector18

#define TIM0_IRQHandler         Vector22
#define TIM1_IRQHandler         Vector23
#define TIM2_IRQHandler         Vector24

#define WDT_IRQHandler          Vector26
#define SF_IRQHandler           Vector27

#define MAC6200_SPI_IRQHandler  Vector28
#define MAC6200_RF_IRQHandler   Vector29

#define USBDMA_IRQHandler       Vector30
#define SWI_IRQHandler          Vector31 
/** @} */

void TICK_IRQHandler(void) __ONCHIP_CODE__;
void BT_IRQHandler(void) __attribute__ ((section (".bttext"), optimize(s), noinline));
void SWI_IRQHandler(void) __ONCHIP_CODE__;
/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*
 * Configuration-related checks.
 */
#if !defined(HS6601_MCUCONF)
#error "Using a wrong mcuconf.h file, HS6601_MCUCONF not defined"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type representing a system clock frequency.
 */
typedef uint32_t halclock_t;

/**
 * @brief   Type of the realtime free counter value.
 */
typedef uint32_t halrtcnt_t;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define DIV_ROUND(n,d) (((n) + ((d) / 2)) / (d))
#define min(a, b) (((a) < (b)) ? (a) : (b))

#define __hal_set_bitval(val, bit, bitval)          \
do{                                                 \
  uint32_t __mask;                                    \
  __mask = 1u<<(bit);                                 \
  (val) = ((val)&~__mask) | (((bitval)<<(bit))&__mask); \
}while(0)

#define __hal_set_bitsval(val, s, e, bitval)        \
do{                                                 \
  uint32_t __mask;                                    \
  if((e-s)==31)                                      \
  __mask = 0xFFFFFFFF;                              \
  else                                                \
  __mask = ((1u<<((e)-(s)+1)) - 1) << (s);            \
  (val) = ((val)&~__mask) | (((bitval)<<(s))&__mask);   \
}while(0)


/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/
#include "nvic.h"
#include "cpm_lld.h"

#ifdef __cplusplus
extern "C" {
#endif
  void hal_lld_init(void);
  void hs_clock_init(void);
#ifdef __cplusplus
}
#endif

#endif /* _HAL_LLD_H_ */

/** @} */
