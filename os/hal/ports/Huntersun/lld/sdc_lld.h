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
 * @file    hs66xx/sdc_lld.h
 * @brief   SDC Driver subsystem low level driver header.
 *
 * @addtogroup SDC
 * @{
 */

#ifndef _SDC_LLD_H_
#define _SDC_LLD_H_

#if HAL_USE_SDC || defined(__DOXYGEN__)

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
 * @brief   SDHC interrupt priority level setting.
 */
#if !defined(HS_SDC_SDHC_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define HS_SDC_SDHC_IRQ_PRIORITY         3
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if HS_SDC_SDHC_IRQ_PRIORITY > 3
#error "Invalid IRQ priority assigned to SDHC"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of card flags.
 */
typedef uint32_t sdcmode_t;

/**
 * @brief   SDC Driver condition flags type.
 */
typedef uint32_t sdcflags_t;

/**
 * @brief   Type of a structure representing an SDC driver.
 */
typedef struct SDCDriver SDCDriver;

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {
  uint32_t                  speed;
} SDCConfig;

/**
 * @brief   @p SDCDriver specific methods.
 */
#define _sdc_driver_methods                                                 \
  _mmcsd_block_device_methods

/**
 * @extends MMCSDBlockDeviceVMT
 *
 * @brief   @p SDCDriver virtual methods table.
 */
struct SDCDriverVMT {
  _sdc_driver_methods
};

struct idmac_desc {
	uint32_t	des0;	/* Control Descriptor */
#define IDMAC_DES0_DIC	(1UL << 1)
#define IDMAC_DES0_LD	(1UL << 2)
#define IDMAC_DES0_FD	(1UL << 3)
#define IDMAC_DES0_CH	(1UL << 4)
#define IDMAC_DES0_ER	(1UL << 5)
#define IDMAC_DES0_CES	(1UL << 30)
#define IDMAC_DES0_OWN	(1UL << 31)

	uint32_t	des1;	/* Buffer sizes */
#define IDMAC_SET_BUFFER1_SIZE(d, s) \
	((d)->des1 = ((d)->des1 & 0x03ffe000) | ((s) & 0x1fff))

	uint32_t	des2;	/* buffer 1 physical address */

	uint32_t	des3;	/* buffer 2 physical address */
};

/**
 * @brief   Structure representing an SDC driver.
 */
struct SDCDriver {
  /**
   * @brief Virtual Methods Table.
   */
  const struct SDCDriverVMT *vmt;
  _mmcsd_block_device_data
  /**
   * @brief Current configuration data.
   */
  const SDCConfig           *config;
  /**
   * @brief Various flags regarding the mounted card.
   */
  sdcmode_t                 cardmode;
  /**
   * @brief Errors flags.
   */
  sdcflags_t                errors;
  /**
   * @brief Card RCA.
   */
  uint32_t                  rca;
  /* End of the mandatory fields.*/
  /**
   * @brief Thread waiting for I/O completion IRQ.
   */
  thread_t                    *thread;
  /**
   * @brief     Pointer to the SDIO registers block.
   * @note      Used only for dubugging purpose.
   */
#if CH_DBG_ENABLE_ASSERTS
  HS_SDHC_Type              *sdhc;
#endif
  /**
   * @brief ID of card, currently one card only
   */
  uint16_t                   id;
  
  uint16_t                   sg_length;
  struct idmac_desc          *sg;

#if CH_CFG_USE_MUTEXES || defined(__DOXYGEN__)
  /* @brief Mutex protecting the bus. */ 
  mutex_t               mutex;
#endif
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @name    R1 response utilities
 * @{
 */
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(__DOXYGEN__)
extern SDCDriver SDCD0;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void sdc_lld_init(void);
  void sdc_lld_start(SDCDriver *sdcp);
  void sdc_lld_stop(SDCDriver *sdcp);
  void sdc_lld_start_clk(SDCDriver *sdcp);
  void sdc_lld_set_data_clk(SDCDriver *sdcp);
  void sdc_lld_stop_clk(SDCDriver *sdcp);
  void sdc_lld_set_bus_mode(SDCDriver *sdcp, sdcbusmode_t mode);
  void sdc_lld_send_cmd_none(SDCDriver *sdcp, uint32_t cmd, uint32_t arg);
  bool_t sdc_lld_send_cmd_short(SDCDriver *sdcp, uint32_t cmd, uint32_t arg,
                                uint32_t *resp);
  bool_t sdc_lld_send_cmd_short_crc(SDCDriver *sdcp, uint32_t cmd, uint32_t arg,
                                    uint32_t *resp);
  bool_t sdc_lld_send_cmd_long_crc(SDCDriver *sdcp, uint32_t cmd, uint32_t arg,
                                   uint32_t *resp);
  bool_t sdc_lld_read(SDCDriver *sdcp, uint32_t startblk,
                      uint8_t *buf, uint32_t n);
  bool_t sdc_lld_write(SDCDriver *sdcp, uint32_t startblk,
                       const uint8_t *buf, uint32_t n);
  bool_t sdc_lld_sync(SDCDriver *sdcp);
  bool_t sdc_lld_is_card_inserted(SDCDriver *sdcp);
  bool_t sdc_lld_is_write_protected(SDCDriver *sdcp);
  sdcbusmode_t sdc_lld_get_bus_width(SDCDriver *sdcp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SDC */

#endif /* _SDC_LLD_H_ */

/** @} */
