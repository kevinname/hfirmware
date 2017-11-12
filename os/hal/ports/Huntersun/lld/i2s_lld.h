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
 * @file    hs66xx/i2s_lld.h
 * @brief   audio interface Driver subsystem low level driver header template.
 *
 * @addtogroup I2S
 * @{
 */

#ifndef _I2S_LLD_H_
#define _I2S_LLD_H_

#if HAL_USE_I2S || defined(__DOXYGEN__)

#include "dma_lld.h"
#include "audio.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

#define HS_I2S_MCLK               24000000

#define I2S_REC_BLOCK_SIZE        (2400)
#define I2S_REC_BLOCK_NUM         3
#define I2S_REC_BUFFER_SIZE       (I2S_REC_BLOCK_SIZE * I2S_REC_BLOCK_NUM)

#define I2S_PLY_BLOCK_SIZE        (2400)
#define I2S_PLY_BLOCK_NUM         5
#define I2S_PLY_BUFFER_SIZE       (I2S_PLY_BLOCK_SIZE * I2S_PLY_BLOCK_NUM)

#define I2S_MONO_MODE_OFFSET  4

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

#define CLOCK_DIV_EN     (1<<0)

/**
 * @name    Configuration options
 * @{
 */


/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/


/**
 * @brief   I2S interrupt priority level setting.
 */
#if !defined(HS_I2S_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define HS_I2S_IRQ_PRIORITY        3
#endif

#if !defined(HS_I2S_DMA_PRIORITY) || defined(__DOXYGEN__)
#define HS_I2S_DMA_PRIORITY        3
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/
typedef enum
{
  I2S_STATE_IDLE        = 0,
  I2S_STATE_STOP        ,
  I2S_STATE_READY       ,
  I2S_STATE_WORKING     ,

  I2S_STATE_PAUSE       ,
}hs_i2s_state_t;

typedef enum
{
  I2S_WORKMODE_SLAVE        = 0,
  I2S_WORKMODE_MASTER       ,
}hs_i2s_workmode_t;

typedef enum
{
  I2S_PCMMODE_STEREO        = 0,
  I2S_PCMMODE_MONO          ,
}hs_i2s_pcmmode_t;

typedef enum
{
  I2S_BITWIDTH_IGNORE     = 0,
  I2S_BITWIDTH_12BIT      ,
  I2S_BITWIDTH_16BIT      ,
  I2S_BITWIDTH_20BIT      ,
  I2S_BITWIDTH_24BIT      ,
  I2S_BITWIDTH_32BIT      ,
  I2S_BITWIDTH_64BIT      ,
  I2S_BITWIDTH_128BIT     ,
  I2S_BITWIDTH_192BIT     ,
}hs_i2s_bitwidth_t;

typedef enum
{
  I2S_SAMPLE_8K           = 8000,
  I2S_SAMPLE_11K          = 11025,
  I2S_SAMPLE_12K          = 12000,
  I2S_SAMPLE_16K          = 16000,
  I2S_SAMPLE_22K          = 22050,
  I2S_SAMPLE_24K          = 24000,
  I2S_SAMPLE_32K          = 32000,
  I2S_SAMPLE_44P1K        = 44100,
  I2S_SAMPLE_48K          = 48000,
  I2S_SAMPLE_96K          = 96000,
}hs_i2s_sample_t;

/**
 * @brief   Type of a structure representing an I2S driver.
 */
//typedef struct I2SDriver I2SDriver;

/**
 * @brief   I2S notification callback type.
 *
 * @param[in] i2sp      pointer to the @p I2SDriver object triggering the
 *                      callback
 */
//typedef void (*i2scallback_t)(I2SDriver *i2sp);

/**
 * @brief   Driver configuration structure.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
typedef struct
{
  hs_i2s_sample_t     sample_rate;
  hs_i2s_bitwidth_t   sample_width;
  hs_i2s_bitwidth_t   ws_width;
  hs_i2s_workmode_t   work_mode;
  hs_i2s_pcmmode_t    i2s_mode;

  uint32_t            frame_len;
  uint32_t            frame_num;
} hs_i2s_config_t;


/**
 * @brief   Structure representing an I2S driver.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
typedef struct
{
  hs_i2s_state_t         rx_state;
  hs_i2s_state_t         tx_state;

  hs_i2s_config_t        *rx_cfgp;
  hs_i2s_config_t        *tx_cfgp;

  HS_I2S_Type            *rx_i2s;
  HS_I2S_Type            *tx_i2s;

  hs_dma_stream_t        *pdmarx;
  hs_dma_stream_t        *pdmatx;

  hs_audio_stream_t       *pRec;
  hs_audio_stream_t       *pPly;

  uint8_t                *prxbuf;
  uint8_t                *ptxbuf;
}I2SDriver;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/


#define __i2s_clr_bit(val, bit)            (val) &= ~(1u<<bit)
#define __i2s_set_bit(val, bit)            (val) |= (1u<<bit)

#define __i2s_set_bitval(val, bit, bitval)        \
do{                                                 \
  uint32_t mask;                                    \
  mask = 1u<<(bit);                                 \
  (val) = ((val)&~mask) | (((bitval)<<(bit))&mask); \
}while(0)

#define __i2s_set_bitsval(val, s, e, bitval)      \
do{                                                 \
  uint32_t mask;                                    \
  mask = ((1u<<((e)-(s)+1)) - 1) << (s);              \
  (val) = ((val)&~mask) | (((bitval)<<(s))&mask);   \
}while(0)

#define __i2s_set_rxsamplewidth(i2sp, width)      (i2sp)->CHN_R0.RCR = (width)
#define __i2s_set_txsamplewidth(i2sp, width)      (i2sp)->CHN_R0.TCR = (width)
#define __i2s_set_workmode(i2sp, mode)            __i2s_set_bitval((i2sp)->CER, 0, mode)
#define __i2s_set_i2smode(i2sp, mode)             __i2s_set_bitval((i2sp)->IER, 4, mode)

#define __i2s_set_enable(i2sp, en)                __i2s_set_bitval((i2sp)->IER, 0, en)

#define __i2s_enable_rx(i2sp, en)                 (i2sp)->IRER = (en)
#define __i2s_enable_tx(i2sp, en)                 (i2sp)->ITER = (en)

#define __i2s_enable_rxChn(i2sp, en)              (i2sp)->CHN_R0.RER = (en)
#define __i2s_enable_txChn(i2sp, en)              (i2sp)->CHN_R0.TER = (en)

#define __i2s_set_rxFifoDepth(i2sp, depth)        (i2sp)->CHN_R0.RFCR = (depth)
#define __i2s_set_txFifoDepth(i2sp, depth)        (i2sp)->CHN_R0.TFCR = (depth)

#define __i2s_mask_allInt(i2sp)                   (i2sp)->CHN_R0.IMR = 0x33
#define __i2s_unmask_allInt(i2sp)                 (i2sp)->CHN_R0.IMR = 0

#define __i2s_flush_rxFifo(i2sp)                  (i2sp)->CHN_R0.RFF = 1
#define __i2s_flush_txFifo(i2sp)                  (i2sp)->CHN_R0.TFF = 1

#define __i2s_reset_allRxFifo(i2sp)               (i2sp)->RXFFR = 1
#define __i2s_reset_allTxFifo(i2sp)               (i2sp)->TXFFR = 1

#define __i2s_set_wss(i2sp, val)                  __i2s_set_bitsval((i2sp)->CCR, 3, 4, val)   /*(i2sp)->i2s->CCR |= (val) << 3*/
#define __i2s_set_sclkGate(i2sp, val)             __i2s_set_bitsval((i2sp)->CCR, 0, 2, val)   /*(i2sp)->i2s->CCR |= (val) << 0*/

#define __i2s_reset_rxDma(i2sp)                   (i2sp)->RRXDMA = 1
#define __i2s_reset_txDma(i2sp)                   (i2sp)->RTXDMA = 1

/* output function */
#define i2s_lld_play_triger(i2sp)                 dmaStreamStartByLli((i2sp)->pdmatx);
/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if HS_I2S_USE_I2S0 && !defined(__DOXYGEN__)
extern I2SDriver I2SD;
#endif


#ifdef __cplusplus
extern "C" {
#endif

void i2s_lld_init(void);
void i2s_lld_start(I2SDriver *i2sp) ;
void i2s_lld_stop(I2SDriver *i2sp);

int32_t i2s_lld_record_start(I2SDriver *i2sp, hs_i2s_config_t *rx_cfgp, hs_audio_stream_t *pRec);
int32_t i2s_lld_record_stop(I2SDriver *i2sp);

int32_t i2s_lld_play_start(I2SDriver *i2sp, hs_i2s_config_t *tx_cfgp, hs_audio_stream_t *pPly);
int32_t i2s_lld_play_stop(I2SDriver *i2sp);

#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_I2S */

#endif /* _I2S_LLD_H_ */

/** @} */
