/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio
                 Copyright (C) 2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn

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
 * @file    hs66xx/sf_lld.h
 * @brief   SPI flash interface Driver subsystem low level driver header template.
 *
 * @addtogroup SF
 * @{
 */

#ifndef _SF_LLD_H_
#define _SF_LLD_H_

#if HAL_USE_SF || defined(__DOXYGEN__)

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
 * @brief   SF driver extern.
 * @details If set to @p TRUE output the global var.
 */
#if !defined(HS_SF_USE_SF0) || defined(__DOXYGEN__)
#define HS_SF_USE_SF0             TRUE
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#define SF_INTR_MASK         0
#define SF_INTR_UNMASK       1
#define SF_INTR_CLRSTS       1

#define SF_MAILBOX_SIZE      4

#define SF_DATA_CNT(len)     ((len) << 16)
#define SF_CMD_WIDTH(bits)   ((bits) << 8)
#define SF_CMD_KEEPCS        (0u  << 6)
#define SF_CMD_READ          1
#define SF_CMD_WRITE         2

#define SF_READ_PAGE         0x100


#define SF_CMD_WREN          0x06000000
#define SF_CMD_RDSTAS        0x05000000
#define SF_CMD_RDSTAS1       0x35000000
#define SF_CMD_WRSTAS        0x01000000
#define SF_CMD_FTRD          0x0b000000
#define SF_CMD_RD            0x03000000
#define SF_CMD_PP            0x02000000
#define SF_CMD_SE            0x20000000
#define SF_CMD_BE            0xd8000000
#define SF_CMD_CE            0xc7000000
#define SF_CMD_RDID          0x9f000000
#define SF_CMD_DEEPPD        0xB9000000
#define SF_CMD_RELEASEPD     0xAB000000
#define SF_CMD_HPM           0xA3000000
#define SF_CMD_FTQRD         0x6b000000

/**
 * @brief   SF interrupt priority level setting.
 */
#if !defined(HS_SF_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define HS_SF_IRQ_PRIORITY        3
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/
/**
 * @brief   data width.
 */
typedef enum{
  WIDTH_8BIT       = 0,
  WIDTH_16BIT      ,
  WIDTH_32BIT      ,
}sfdw_t;


/**
 * @brief   chip select.
 */
typedef enum{
  SELECT_CS0       = 0,
  SELECT_CS1       ,
  SELECT_CS2       ,
  SELECT_CS3       ,
}sfcs_t;


/**
 * @brief   clock mode.
 */
typedef enum{
  CLKMODE_0       = 0,
  CLKMODE_1       ,
  CLKMODE_2       ,
  CLKMODE_3       ,
}sfmode_t;

/**
 * @brief   cs polarity.
 */
typedef enum{
  LOW_ACTIVE       = 0,
  HIGH_ACTIVE      ,
}sfcsp_t;

typedef enum{
  QUAD_DISABLE     = 0,
  QUAD_ENABLE      ,
}sfqe_t;

typedef struct {
  const char    name[32];
  uint32_t      mid;
  uint32_t      page_size;
  uint32_t      sector_size;
  uint32_t      sector_num;
  uint32_t      block_size;
  uint32_t      block_num;
} SFlashInfo_t;

/**
 * @brief   Type of a structure representing an SF driver.
 */
typedef struct SFDriver SFDriver;

/**
 * @brief   SF notification callback type.
 *
 * @param[in] sfp      pointer to the @p SFDriver object triggering the
 *                      callback
 */
typedef void (*sfcallback_t)(SFDriver *sfp);

/**
 * @brief   Driver configuration structure.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
typedef struct {
  sfcallback_t end_cb;

  /**
   * @brief data width when in little-endian system
   */
  sfdw_t data_width;
  sfcs_t cs;
  sfmode_t clk_mode;
  sfcsp_t cs_pol;

  /**
   * @brief The wanted SPI master clock
   */
  uint32_t speed;
  
  /* End of the mandatory fields.*/
} SFConfig;

/**
 * @brief   Structure representing an SF driver.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
struct SFDriver {
  /**
   * @brief Driver state.
   */
  sfstate_t            state;
  /**
   * @brief Current configuration data.
   */
  const SFConfig       *config;
  const SFlashInfo_t   *sfinfo;

  thread_t               *thread;

#if SF_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
#if CH_CFG_USE_MUTEXES || defined(__DOXYGEN__)
  /**
   * @brief Mutex protecting the bus.
   */
  mutex_t                 mutex;
#elif CH_CFG_USE_SEMAPHORES
  semaphore_t             semaphore;
#endif
#endif /* SF_USE_MUTUAL_EXCLUSION */

  HS_SF_Type            *sf;
  /**
   * @brief The clock source of this IP
   */
  uint32_t              clock;
  /**
   * @brief The true SPI master clock
   */
  uint32_t              mclk;
  uint32_t              mid;
  systime_t             timeout;

  uint32_t              fmode;
  /* End of the mandatory fields.*/
};

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if HS_SF_USE_SF0 && !defined(__DOXYGEN__)
extern SFDriver SFD;
#endif


#ifdef __cplusplus
extern "C" {
#endif
  void sf_lld_init(void);
  int sf_lld_start(SFDriver *sfp);
  int sf_lld_stop(SFDriver *sfp);

  int sf_lld_probe(SFDriver *sfp);
  int sf_lld_erase(SFDriver *sfp, uint32_t offset, size_t len);
  int sf_lld_read(SFDriver *sfp, uint32_t offset, void *rdbuf, size_t len);
  int sf_lld_write(SFDriver *sfp, uint32_t offset, void *wrbuf, size_t len);

  void sf_lld_QuadEn(SFDriver *sfp, sfqe_t qe);
  uint32_t sf_read_status(SFDriver *sfp);

  int sf_lld_deepPd(SFDriver *sfp);
  int sf_lld_releasePd(SFDriver *sfp);
  
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SF */

#endif /* _SF_LLD_H_ */

/** @} */
