/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio
                 Copyright (C) 2014 HunterSun Technologies
                 hongwei.li@huntersun.com.cn

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
 * @file    hs66xx/spi_lld.h
 * @brief   SPI Driver subsystem low level driver header template.
 *
 * @addtogroup SPI
 * @{
 */

#ifndef _SPI_LLD_H_
#define _SPI_LLD_H_

#if HAL_USE_SPI || defined(__DOXYGEN__)

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
 * @brief   SPI driver enable switch.
 * @details If set to @p TRUE the support for SPI0 is included.
 */
#if !defined(HS_SPI_USE_SPI0) || defined(__DOXYGEN__)
#define HS_SPI_USE_SPI0               TRUE
#endif
/**
 * @brief   SPI driver enable switch.
 * @details If set to @p TRUE the support for SPI1 is included.
 */
#if !defined(HS_SPI_USE_SPI1) || defined(__DOXYGEN__)
#define HS_SPI_USE_SPI1               TRUE
#endif

#if !defined(HS_SPI_SPI0_IRQ_PRIORITY)
  #define HS_SPI_SPI0_IRQ_PRIORITY  3
#endif

#if !defined(HS_SPI_SPI1_IRQ_PRIORITY)
#define HS_SPI_SPI1_IRQ_PRIORITY  3
#endif

 /** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a structure representing an SPI driver.
 */
typedef struct SPIDriver SPIDriver;

#define SPI_SLAVE_PROTOCOL_DATA_NUM    0x8
#define SPI_SLAVE_PROTOCOL_CMD_NUM     0x4
#define SPI_SLAVE_PROTOCOL_NUM         (SPI_SLAVE_PROTOCOL_DATA_NUM+SPI_SLAVE_PROTOCOL_CMD_NUM)
#define SPI_SLAVE_READ_DIR        0x85
#define SPI_SLAVE_WRITE_DIR       0x34
#define SPI_CMD_READ_CHIP_ID      0x1
#define SPI_CMD_LOOP_TEST         0x2

typedef enum {
  SPI_MASTER_MODE = 1,
  SPI_SLAVE_MODE,
}spi_dev_role_t;

typedef enum {
  PRO_INT_STATUS  = 1,       
  PRO_CMD_STATUS  ,
  PRO_READ_STATUS ,
  PRO_READ_CMP_STATUS,
  PRO_WRITE_STATUS,
  PRO_WRITE_CMP_STATUS,
}spi_prot_status_t;
/**
 * @brief   SPI notification callback type.
 *
 * @param[in] spip      pointer to the @p SPIDriver object triggering the
 *                      callback
 */
typedef struct {
  uint8_t op_dir;
  uint8_t cmd;
  uint8_t cmd_num;
  uint8_t *data;
  uint32_t data_num;
  uint32_t dev_status;
}spislavecallback_par_t;

typedef void (*spicallback_t)(SPIDriver *spip);
typedef void (*spislavecallback_t)(SPIDriver *spip, spislavecallback_par_t *par);
/**
 * @brief   Driver configuration structure.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
typedef struct {
  /**
   * @brief Operation complete callback.
   */
  spicallback_t         end_cb;
  spislavecallback_t    slave_cb;
  spislavecallback_par_t *paras;
  /* End of the mandatory fields.*/
  uint32_t			    speed;
  uint32_t              mode;
  uint32_t              cs_gpio_index;
  uint32_t              frame_size;
  spi_dev_role_t        role;
} SPIConfig;

/**
 * @brief   Structure representing an SPI driver.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
struct SPIDriver {
  /**
   * @brief Driver state.
   */
  spistate_t            state;
	/**
   * @brief Transmitter state.
   */
  spitxstate_t          txstate;
  /**
   * @brief Receiver state.
   */
  spirxstate_t          rxstate;
  /**
   * @brief Current configuration data.
   */
  const SPIConfig       *config;
#if (SPI_USE_WAIT == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   Waiting thread.
   */
  thread_reference_t        thread;
#endif
#if (SPI_USE_MUTUAL_EXCLUSION == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   Mutex protecting the peripheral.
   */
  mutex_t                   mutex;
#endif
#if defined(SPI_DRIVER_EXT_FIELDS)
  SPI_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/

	/**
	* @brief Pointer to the spi registers block.
	*/
  HS_SPI_Type          	*spi;
  /**
   * @brief Clock frequency for the associated spi
   */
  uint32_t           	 clock;
  /**
   * @brief Receive DMA channel.
   */
  hs_dma_stream_t       *dmarx;
  /**
   * @brief Transmit DMA channel.
   */
  hs_dma_stream_t       *dmatx;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/* SPI0 interface register and address map */

#define SPI0_MASTER_OFFSET	16
#define SPI0_MODE_OFFSET	17
#define SPI0_RESET_OFFSET	19
#define SPI0_MSB_OFFSET	20
#define SPI0_RX_TRIGGER_LEVEL_OFFSET	26
#define SPI0_RX_FIFO_RESET_OFFSET	28
#define SPI0_TX_FIFO_RESET_OFFSET	29
#define SPI0_RX_FIFO_ENABLE_OFFSET	30
#define SPI0_TX_FIFO_ENABLE_OFFSET	31
#define SPI0_TX_EMPTY_INT_EN_OFFSET   4
#define SPI0_TX_EMPTY_STATUS_OFFSET   5
#define SPI0_RX_TRIGGER_INT_EN_OFFSET 6
#define SPI0_RX_TRIGGER_STATUS_OFFSET 7
#define SPI0_RX_BYTE_CNT_OFFSET       8
#define SPI0_TX_BYTE_CNT_OFFSET       16
#define SPI0_INT_EN_OFFSET            0

/* Bit fields in CTRL */
#define SPI0_MASTER (1 << SPI0_MASTER_OFFSET)
#define SPI0_RESET	(1 << SPI0_RESET_OFFSET)
#define SPI0_MSB	(1 << SPI0_MSB_OFFSET)
#define SPI0_TX_FIFO_RESET	(1 << SPI0_TX_FIFO_RESET_OFFSET)
#define SPI0_RX_FIFO_RESET	(1 << SPI0_RX_FIFO_RESET_OFFSET)
#define SPI0_TX_FIFO_ENABLE	((unsigned int)1 << SPI0_TX_FIFO_ENABLE_OFFSET)
#define SPI0_RX_FIFO_ENABLE	((unsigned int)1 << SPI0_RX_FIFO_ENABLE_OFFSET)
#define SPI0_SET_TRIGGER_MASK (~(3ul << SPI0_RX_TRIGGER_LEVEL_OFFSET))
#define SPI0_SET_TRIGGER_LEVEL(n) (((n)&0x3) << SPI0_RX_TRIGGER_LEVEL_OFFSET)
#define SPI0_SET_TX_EMPTY_INT  (1 << SPI0_TX_EMPTY_INT_EN_OFFSET)
#define SPI0_SET_RX_TRIGGER_INT (1 << SPI0_RX_TRIGGER_INT_EN_OFFSET)
#define SPI0_INT_EN_MASK             (1 << SPI0_INT_EN_OFFSET)
#define SPI0_TX_EMPTY_STATUS_MASK (1ul << SPI0_TX_EMPTY_STATUS_OFFSET)
#define SPI0_RX_TRIGGER_STATUS_MASK (1ul << SPI0_RX_TRIGGER_STATUS_OFFSET)
#define SPI0_RX_BYTE_CNT_MASK  (0xff << SPI0_RX_BYTE_CNT_OFFSET)
#define SPI0_TX_BYTE_CNT_MAKE  (0xff << SPI0_TX_BYTE_CNT_OFFSET)
#define SPI0_BUSY	            (1 << 24)

#if !defined(HS_SPI_SPI0_DMA_PRIORITY) || defined(__DOXYGEN__)
#define HS_SPI_SPI0_DMA_PRIORITY      1
#endif

#if !defined(HS_SPI_SPI1_DMA_PRIORITY) || defined(__DOXYGEN__)
#define HS_SPI_SPI1_DMA_PRIORITY      1
#endif


/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/
#if HS_SPI_USE_SPI0 && !defined(__DOXYGEN__)
extern SPIDriver SPID0;
#endif

#if HS_SPI_USE_SPI1 && !defined(__DOXYGEN__)
extern SPIDriver SPID1;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void spi_lld_init(void);
  void spi_lld_start(SPIDriver *spip);
  void spi_lld_stop(SPIDriver *spip);
  void spi_lld_select(SPIDriver *spip);
  void spi_lld_unselect(SPIDriver *spip);
  void spi_lld_ignore(SPIDriver *spip, size_t n);
  void spi_lld_exchange(SPIDriver *spip, size_t n,
                        const void *txbuf, void *rxbuf);
  void spi_lld_send(SPIDriver *spip, size_t n, const void *txbuf);
  void spi_lld_receive(SPIDriver *spip, size_t n, void *rxbuf);
  uint16_t spi_lld_polled_exchange(SPIDriver *spip, uint16_t frame);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SPI */

#endif /* _SPI_LLD_H_ */

/** @} */
