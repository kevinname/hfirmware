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
 * @file    hs66xx/i2c_lld.h
 * @brief   I2C Driver subsystem low level driver header.
 *
 * @addtogroup I2C
 * @{
 */

#ifndef _I2C_LLD_H_
#define _I2C_LLD_H_

#if HAL_USE_I2C || defined(__DOXYGEN__)
#include "dma_lld.h"
/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/
#define EEPROM_PAGE_SIZE        32
#define I2C_SLAVE_DEFAULT_ADDR  0x62
/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   I2C1 driver enable switch.
 * @details If set to @p TRUE the support for I2C1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(HS_I2C_USE_I2C0) || defined(__DOXYGEN__)
#define HS_I2C_USE_I2C0               TRUE
#endif

/**
 * @brief   I2C timeout on busy condition in milliseconds.
 */
#if !defined(STM32_I2C_BUSY_TIMEOUT) || defined(__DOXYGEN__)
#define HS_I2C_BUSY_TIMEOUT            50
#endif

#if !defined(HS_I2C_I2C0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define HS_I2C_I2C0_IRQ_PRIORITY      3
#endif

#if !defined(HS_I2C_I2C0_DMA_PRIORITY) || defined(__DOXYGEN__)
#define HS_I2C_I2C0_DMA_PRIORITY      1
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type representing I2C address.
 */
typedef uint16_t i2caddr_t;

/**
 * @brief   Type of I2C Driver condition flags.
 */
typedef uint32_t i2cflags_t;

/**
 * @brief   Supported modes for the I2C bus.
 */
typedef enum {
  OPMODE_I2C_MASTER = 1,
  OPMODE_I2C_SLAVE = 2,
  OPMODE_SMBUS_DEVICE = 3,
  OPMODE_SMBUS_HOST = 4,
} i2copmode_t;

typedef enum {
  CONMODE_I2C_AUTO_WR = 1,
  CONMODE_I2C_NO_AUTO_WR,
}i2cconmode_t;

typedef enum {
  I2C_DMA_TRANS_DIR_TX = 1,
  I2C_DMA_TRANS_DIR_RX = 2,
}i2cdmatransdire_t;

/**
 * @brief   Supported duty cycle modes for the I2C bus.
 */
typedef enum {
  STD_DUTY_CYCLE = 1,
  FAST_DUTY_CYCLE_2 = 2,
  FAST_DUTY_CYCLE_16_9 = 3,
} i2cdutycycle_t;

/**
 * @brief   Type of a structure representing an I2C driver.
 */
typedef struct I2CDriver I2CDriver;

/**
 * @brief   I2C notification callback type.
 *
 * @param[in] i2cp      pointer to a @p I2CDriver object
 */

typedef void (*i2ccallback_t)(void *arg);

/**
 * @brief   Driver configuration structure.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */

/**
 * @brief Driver configuration structure.
 */
typedef struct {
  i2copmode_t     op_mode;       /**< @brief Specifies the I2C mode.        */
  uint32_t        clock_speed;   /**< @brief Specifies the clock frequency.
                                      @note Must be set to a value lower
                                      than 400kHz.                          */
  //i2cdutycycle_t  duty_cycle;    /**< @brief Specifies the I2C fast mode
  //                                    duty cycle.                           */
  i2cconmode_t  con_mode;         /**< @brief Specifies the I2C read and write control mode */
  uint32_t    timer_control_channel; 

  /* i2c callback */
  i2ccallback_t i2c_callback;

} I2CConfig;



struct i2c_msg {
#define I2C_M_RD            0x0001	/* read data, from slave to master */
  /**
   * @brief   Flags to tx or rx.
   */
  uint8_t                   flags;
  /**
   * @brief   Pointer to the buffer to tx/rx.
   */
  uint8_t                   *buf;
  /**
   * @brief   Number of bytes of data to tx/rx.
   */
  size_t                    len;
};

/**
 * @brief Structure representing an I2C driver.
 */
struct I2CDriver {
  /**
   * @brief   Driver state.
   */
  i2cstate_t                state;
  /**
   * @brief   Current configuration data.
   */
  const I2CConfig           *config;
  /**
   * @brief   Error flags.
   */
  i2cflags_t                errors;
#if I2C_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
  /**
   * @brief   Mutex protecting the bus.
   */
  mutex_t                     mutex;

#endif /* I2C_USE_MUTUAL_EXCLUSION */
#if defined(I2C_DRIVER_EXT_FIELDS)
  I2C_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief   Thread waiting for I/O completion.
   */
  thread_t                  *thread;
  /**
   * @brief     Current slave address without R/W bit.
   */
  i2caddr_t                 addr;
  uint32_t                  tx_fifo_depth;
  uint32_t                  rx_fifo_depth;
#define NUM_I2C_MSG      2
  struct i2c_msg            msgs[NUM_I2C_MSG];
#define STATUS_IDLE   0x0
#define STATUS_WRITE_IN_PROGRESS	0x1
#define STATUS_READ_IN_PROGRESS   0x2
  /**
   * @brief   I2C master status for multi-message transfer.
   */
  uint32_t                  status;
  /**
   * @brief   the element index of the current rx message in the msgs array.
   */
  uint8_t                   msg_read_idx;
  /**
   * @brief   the element index of the current tx message in the msgs array.
   */
  uint8_t                   msg_write_idx;
  /**
   * @brief   Current pointer to the buffer with data to send.
   */
  uint8_t                   *txbuf;
  /**
   * @brief   Remains number of bytes of data to send.
   */
  size_t                    txbytes;
  /**
   * @brief   Current pointer to the buffer to put received data.
   */
  uint8_t                   *rxbuf;
  /**
   * @brief   Remains number of bytes of data to receive.
   */
  size_t                    rxbytes;
  /**
   * @brief     Pointer to the I2Cx registers block.
   */
  HS_I2C_Type               *i2c;  

  hs_dma_stream_t           *dma_tx;
  hs_dma_stream_t           *dma_rx;

  i2ccallback_t i2c_callback;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Get errors from I2C driver.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
#define i2c_lld_get_errors(i2cp) ((i2cp)->errors)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(__DOXYGEN__)
#if HS_I2C_USE_I2C0
extern I2CDriver I2CD0;
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void i2c_lld_init(void);
  int i2c_lld_start(I2CDriver *i2cp);
  int i2c_lld_stop(I2CDriver *i2cp);
  msg_t i2c_lld_master_transmit_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                        const uint8_t *txbuf, size_t txbytes,
                                        uint8_t *rxbuf, size_t rxbytes,
                                        systime_t timeout);
  msg_t i2c_lld_master_receive_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                       uint8_t *rxbuf, size_t rxbytes,
                                       systime_t timeout);
  /* read/write EEPROM-like I2C memory */
  msg_t i2c_lld_master_readmem_timeout(I2CDriver *i2cp, i2caddr_t addr,
				       uint32_t offset, uint8_t alen,
				       uint8_t *rxbuf, size_t rxbytes,
				       systime_t timeout);
  msg_t i2c_lld_master_writemem_timeout(I2CDriver *i2cp, i2caddr_t addr,
					uint32_t offset, uint8_t alen,
					const uint8_t *txbuf, size_t txbytes,
					systime_t timeout);
  msg_t i2c_lld_slave_receive_timeout(I2CDriver *i2cp, const uint8_t *rxbuf, 
				      size_t rxbytes, systime_t timeout);
  msg_t i2c_lld_slave_transmit_timeout(I2CDriver *i2cp, const uint8_t *rxbuf, 
				       size_t rxbytes, systime_t timeout);      

#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_I2C */

#endif /* _I2C_LLD_H_ */

/** @} */
