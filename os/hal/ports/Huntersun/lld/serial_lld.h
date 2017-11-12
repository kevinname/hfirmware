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
 * @file    hs66xx/serial_lld.h
 * @brief   Serial Driver subsystem low level driver header.
 *
 * @addtogroup SERIAL
 * @{
 */

#ifndef _SERIAL_LLD_H_
#define _SERIAL_LLD_H_

#if HAL_USE_SERIAL || defined(__DOXYGEN__)

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
 * @brief   UART0 driver enable switch.
 * @details If set to @p TRUE the support for UART0 is included.
 */
#if !defined(HS_SERIAL_USE_UART0) || defined(__DOXYGEN__)
#define HS_SERIAL_USE_UART0             FALSE
#endif

/**
 * @brief   UART1 driver enable switch.
 * @details If set to @p TRUE the support for UART1 is included.
 */
#if !defined(HS_SERIAL_USE_UART1) || defined(__DOXYGEN__)
#define HS_SERIAL_USE_UART1             FALSE
#endif

/**
 * @brief   UART0 interrupt priority level setting.
 */
#if !defined(HS_SERIAL_UART0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define HS_SERIAL_UART0_IRQ_PRIORITY    3
#endif

/**
 * @brief   UART1 interrupt priority level setting.
 */
#if !defined(HS_SERIAL_UART1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define HS_SERIAL_UART1_IRQ_PRIORITY    3
#endif

/**
 * @brief   UART1 DMA priority level setting.
 */
#if !defined(HS_SERIAL_UART1_DMA_PRIORITY) || defined(__DOXYGEN__)
#define HS_SERIAL_UART1_DMA_PRIORITY    0
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if HS_SERIAL_USE_UART0 && !HS_HAS_UART0
#error "UART0 not present in the selected device"
#endif

#if HS_SERIAL_USE_UART1 && !HS_HAS_UART1
#error "UART1 not present in the selected device"
#endif

#if !HS_SERIAL_USE_UART0 && !HS_SERIAL_USE_UART1
#error "SERIAL driver activated but no UART peripheral assigned"
#endif


/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Generic Serial Driver configuration structure.
 * @details An instance of this structure must be passed to @p sdStart()
 *          in order to configure and start a serial driver operations.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
typedef struct {
  /**
   * @brief Bit rate.
   */
  uint32_t                  speed;
  /* End of the mandatory fields.*/
  /**
   * @brief Hardware flow control.
   */
  bool_t                    ctsrts;
} SerialConfig;

/**
 * @brief @p SerialDriver specific data.
 */
#define _serial_driver_data                                                 \
  _base_asynchronous_channel_data                                           \
  /* Driver state.*/                                                        \
  sdstate_t                 state;                                          \
  /* Input queue.*/                                                         \
  input_queue_t             iqueue;                                         \
  /* Output queue.*/                                                        \
  output_queue_t            oqueue;                                         \
  /* Input circular buffer.*/                                               \
  uint8_t                   *ib;                                            \
  /* Output circular buffer.*/                                              \
  uint8_t                   *ob;                                            \
  /* End of the mandatory fields.*/                                         \
  /* Pointer to the UART registers block.*/                                 \
  HS_UART_Type              *uart;                                          \
  /* Clock frequency for the associated UART.*/                             \
  uint32_t                  clock;                                          \
  /* Allow UART to send or not according to MSR.*/                          \
  bool_t                    can_send;                                       \
  /* Don't sumbit DMA if DMA is running.*/                                  \
  bool_t                    tx_running;                                     \
  /*Transmit DMA channel.*/                                                 \
  //hs_dma_stream_t           *dmatx;
  


/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if HS_SERIAL_USE_UART0 && !defined(__DOXYGEN__)
extern SerialDriver SD0;
#endif
#if HS_SERIAL_USE_UART1 && !defined(__DOXYGEN__)
extern SerialDriver SD1;
#endif

#ifdef __cplusplus
extern "C" {
#endif
CH_IRQ_HANDLER(UART1_IRQHandler);

  void sd_lld_init(void);
  void sd_lld_start(SerialDriver *sdp, const SerialConfig *config);
  void sd_lld_stop(SerialDriver *sdp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SERIAL */

#endif /* _SERIAL_LLD_H_ */

/** @} */
