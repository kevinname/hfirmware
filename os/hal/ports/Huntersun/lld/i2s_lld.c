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
 * @file    hs66xx/i2s_lld.c
 * @brief   audio interface Driver subsystem low level driver source template.
 *
 * @addtogroup I2S
 * @{
 */

#include <string.h>
#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#if HAL_USE_I2S || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/


/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   I2S driver identifier.
 */
#if HS_I2S_USE_I2S0 && !defined(__DOXYGEN__)
I2SDriver I2SD = {0};
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/


/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

uint32_t _i2s_get_bitw(hs_i2s_bitwidth_t bw)
{
  uint32_t bit_w = 16;

  switch(bw)
  {
    case I2S_BITWIDTH_12BIT:
      bit_w = 12;
      break;

    case I2S_BITWIDTH_16BIT:
      bit_w = 16;
      break;

    case I2S_BITWIDTH_20BIT:
      bit_w = 20;
      break;

    case I2S_BITWIDTH_24BIT:
      bit_w = 24;
      break;

    case I2S_BITWIDTH_32BIT:
      bit_w = 32;
      break;

    default:
      bit_w = 16;
      break;
  }

  return bit_w;
}

static void _i2s_set_ccr(HS_I2S_Type *i2s, hs_i2s_bitwidth_t width)
{
  uint32_t bit_w;

  switch(width)
  {
    case I2S_BITWIDTH_16BIT:
      bit_w = 0;
      break;

    case I2S_BITWIDTH_24BIT:
      bit_w = 1;
      break;

    case I2S_BITWIDTH_32BIT:
      bit_w = 2;
      break;

    default:
      bit_w = 0;
      break;
  }

  __i2s_set_wss(i2s, bit_w);
  __i2s_set_sclkGate(i2s, 0);
}


static void _i2s_hw_init(HS_I2S_Type *i2s, hs_i2s_config_t *cfgp, hs_audio_streamdir_t dir)
{
  __i2s_set_enable(i2s, 0);
  __i2s_set_i2smode(i2s, cfgp->i2s_mode);
  _i2s_set_ccr(i2s, cfgp->ws_width);

  if(dir == AUDIO_STREAM_PLAYBACK)
  {
    __i2s_set_txsamplewidth(i2s, cfgp->sample_width);
    __i2s_enable_tx(i2s, 0);
    __i2s_enable_txChn(i2s, 0);
    __i2s_set_txFifoDepth(i2s, 7);
  }
  else
  {
    __i2s_set_rxsamplewidth(i2s, cfgp->sample_width);
    __i2s_enable_rx(i2s, 0);
    __i2s_enable_rxChn(i2s, 0);
    __i2s_set_rxFifoDepth(i2s, 7);
  }

  __i2s_set_workmode(i2s, I2S_WORKMODE_SLAVE); //cfgp->work_mode);

  //__i2s_mask_allInt(i2s);
  __i2s_unmask_allInt(i2s);
  __i2s_set_enable(i2s, 1);
}


static void _i2s_record_buf_full(void *p, hs_dma_cb_para_t *var)
{
  I2SDriver *i2sp = (I2SDriver *)p;
  hs_audio_stream_t *stmp = i2sp->pRec;

  if(var->oper_res != DMA_TRANS_OPER_OK)
  {
    return ;
  }

  audio_update_hw_ptr(stmp, 1);

  __audio_wakeup(stmp);
}

static void _i2s_play_buf_empty(void *p, hs_dma_cb_para_t *var)
{
  I2SDriver *i2sp = (I2SDriver *)p;
  hs_audio_stream_t *stmp = i2sp->pPly;

  if(var->oper_res != DMA_TRANS_OPER_OK)
  {
    return ;
  }

  audio_update_hw_ptr(stmp, 1);
  __audio_wakeup(stmp);
}

static void _i2s_setup_dmacfg(I2SDriver *i2sp, hs_dma_config_t *dmaCfg, hs_dma_dir_t dir)
{
  dmaCfg->direction = dir;

  if(i2sp->rx_cfgp->sample_width == I2S_BITWIDTH_24BIT ||
     i2sp->tx_cfgp->sample_width == I2S_BITWIDTH_24BIT)
  {
    dmaCfg->src_addr_width = DMA_SLAVE_BUSWIDTH_32BITS;
    dmaCfg->dst_addr_width = DMA_SLAVE_BUSWIDTH_32BITS;
  }
  else
  {
    dmaCfg->src_addr_width = DMA_SLAVE_BUSWIDTH_16BITS;
    dmaCfg->dst_addr_width = DMA_SLAVE_BUSWIDTH_16BITS;
  }

  dmaCfg->src_burst = DMA_BURST_LEN_1UNITS;
  dmaCfg->dst_burst = DMA_BURST_LEN_1UNITS;
  dmaCfg->dev_flow_ctrl = FALSE;

  dmaCfg->lli_en = 1;
  dmaCfg->lli_loop_en = TRUE;

  if(dir == DMA_DEV_TO_MEM)
  {
    dmaCfg->lli_src_addr = (uint32_t)&i2sp->rx_i2s->RXDMA;
    dmaCfg->lli_dst_addr = (uint32_t)i2sp->prxbuf;

    dmaCfg->lli_block_num = i2sp->rx_cfgp->frame_num;
    dmaCfg->lli_block_len = i2sp->rx_cfgp->frame_len;

    dmaCfg->slave_id = I2S_RX_DMA_ID;
  }
  else if(dir == DMA_MEM_TO_DEV)
  {
    dmaCfg->lli_src_addr = (uint32_t)i2sp->ptxbuf;
    dmaCfg->lli_dst_addr = (uint32_t)&i2sp->tx_i2s->TXDMA;

    dmaCfg->lli_block_num = i2sp->tx_cfgp->frame_num;
    dmaCfg->lli_block_len = i2sp->tx_cfgp->frame_len;

    dmaCfg->slave_id = I2S_TX_DMA_ID;
  }
}

static void _i2s_record_start(I2SDriver *i2sp)
{
  __i2s_enable_rx(i2sp->rx_i2s, 0);
  __i2s_enable_rxChn(i2sp->rx_i2s, 0);

  __i2s_reset_rxDma(i2sp->rx_i2s);
  __i2s_reset_allRxFifo(i2sp->rx_i2s);
  __i2s_flush_rxFifo(i2sp->rx_i2s);

  __i2s_enable_rx(i2sp->rx_i2s, 1);
  __i2s_enable_rxChn(i2sp->rx_i2s, 1);
}

static void _i2s_play_start(I2SDriver *i2sp)
{
  __i2s_enable_tx(i2sp->tx_i2s, 0);
  __i2s_enable_txChn(i2sp->tx_i2s, 0);

  __i2s_reset_txDma(i2sp->tx_i2s);
  __i2s_reset_allTxFifo(i2sp->tx_i2s);
  __i2s_flush_txFifo(i2sp->tx_i2s);

  __i2s_enable_tx(i2sp->tx_i2s, 1);
  __i2s_enable_txChn(i2sp->tx_i2s, 1);
}

static int32_t _i2s_allocDma(I2SDriver *i2sp, hs_audio_streamdir_t dir)
{
  hs_dma_config_t dmaCfg;
  hs_dma_dir_t dma_dir;
  hs_i2s_config_t *cfgp;

  uint32_t size;
  uint8_t *buf;
  if(dir == AUDIO_STREAM_PLAYBACK)
  {
    cfgp = i2sp->tx_cfgp;
    
    dma_dir = DMA_MEM_TO_DEV;
    buf = i2sp->ptxbuf;
  }
  else
  {
    cfgp = i2sp->rx_cfgp;
    
    dma_dir = DMA_DEV_TO_MEM;
    buf = i2sp->prxbuf;
  }

  size = cfgp->frame_len * cfgp->frame_num;  
  memset(buf, 0, size);

  _i2s_setup_dmacfg(i2sp, &dmaCfg, dma_dir);
  if(dir == AUDIO_STREAM_PLAYBACK)
  {
    i2sp->pdmatx = dmaStreamAllocate(HS_I2S_DMA_PRIORITY,
                    (hs_dmaisr_t)_i2s_play_buf_empty, (void *)i2sp);
    if(i2sp->pdmatx == NULL)
      return -2;

    dmaStreamSetMode(i2sp->pdmatx, &dmaCfg);
  }
  else
  {
    i2sp->pdmarx = dmaStreamAllocate(HS_I2S_DMA_PRIORITY,
                      (hs_dmaisr_t)_i2s_record_buf_full, (void *)i2sp);
    if(i2sp->pdmarx == NULL)
      return -2;

    dmaStreamSetMode(i2sp->pdmarx, &dmaCfg);
  }

  return 0;
}

static void _i2s_setup_hwparams(I2SDriver *i2sp, hs_audio_stream_t *streamp, hs_audio_streamdir_t dir)
{
  hs_i2s_config_t  *cfgp;

  streamp->dir = dir;
  if(dir == AUDIO_STREAM_PLAYBACK)
  {
    streamp->dma_start = (uint32_t)i2sp->ptxbuf;
    cfgp = i2sp->tx_cfgp;
  }
  else
  {
    streamp->dma_start = (uint32_t)i2sp->prxbuf;
    cfgp = i2sp->rx_cfgp;
  }

  streamp->buffer_size = cfgp->frame_len * cfgp->frame_num;
  streamp->dma_end = streamp->dma_start + streamp->buffer_size;

  if(cfgp->i2s_mode == I2S_PCMMODE_STEREO)
  {
    streamp->frame_bits = _i2s_get_bitw(cfgp->sample_width) * 2;
  }
  else
  {
    streamp->frame_bits = _i2s_get_bitw(cfgp->sample_width);
  }
  streamp->frame_t_us = 1000000u / cfgp->sample_rate;
  streamp->min_aglin = streamp->frame_bits / 8;
  streamp->hw_ptr = 0;
  streamp->period_size = cfgp->frame_len * 8 / streamp->frame_bits;

  streamp->start_threshold = 0;
  streamp->boundary = streamp->buffer_size * 8 / streamp->frame_bits;
  streamp->app_ptr = 0;
  streamp->min_oper_len = streamp->min_aglin;
  streamp->silence_filled = 0;
  streamp->prompt_dis = 0;

  #if HS_I2S_USE_STATISTIC
  memset(&streamp->performance, 0, sizeof(hs_audio_debug_t));
  #endif
}


/**
 * @brief   Common IRQ handler.
 *
 * @param[in] i2sp       communication channel associated to the I2S
 */
static void serve_interrupt(I2SDriver *i2sp)
{
	(void)i2sp;
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/
/**
 * @brief   I2S interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(I2S_IRQHandler) {

  CH_IRQ_PROLOGUE();

  serve_interrupt(&I2SD);

  CH_IRQ_EPILOGUE();
}
/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level I2S driver initialization.
 *
 * @notapi
 */
void i2s_lld_init(void)
{
#if HS_I2S_USE_I2S0
  I2SD.rx_i2s = HS_I2S_RX;
  I2SD.tx_i2s = HS_I2S_TX;

  I2SD.rx_cfgp = NULL;
  I2SD.tx_cfgp = NULL;

  I2SD.prxbuf = NULL;
  I2SD.ptxbuf = NULL;

  I2SD.pdmarx = NULL;
  I2SD.pdmatx = NULL;

  I2SD.rx_state = I2S_STATE_STOP;
  I2SD.tx_state = I2S_STATE_STOP;
#endif /* HS_I2S_USE_I2S0 */
}

/**
 * @brief   Configures and activates the I2S peripheral.
 *
 * @param[in] i2sp      pointer to the @p I2SDriver object
 *
 * @notapi
 */
void i2s_lld_start(I2SDriver *i2sp)
{
  if((i2sp->rx_state != I2S_STATE_STOP) || (i2sp->tx_state != I2S_STATE_STOP))
  {
    return ;
  }

  cpmEnableI2S();
  cpmResetI2S();

  i2sp->rx_state = I2S_STATE_READY;
  i2sp->tx_state = I2S_STATE_READY;
}

/**
 * @brief   Deactivates the I2S peripheral.
 *
 * @param[in] i2sp      pointer to the @p I2SDriver object
 *
 * @notapi
 */
void i2s_lld_stop(I2SDriver *i2sp)
{
  i2s_lld_record_stop(i2sp);
  i2s_lld_play_stop(i2sp);

  __i2s_mask_allInt(i2sp->rx_i2s);
  __i2s_mask_allInt(i2sp->tx_i2s);

  nvicDisableVector(IRQ_I2S);
  cpmDisableI2S();

  i2sp->rx_state = I2S_STATE_STOP;
  i2sp->tx_state = I2S_STATE_STOP;
}

int32_t i2s_lld_record_start(I2SDriver *i2sp, hs_i2s_config_t *rx_cfgp, hs_audio_stream_t *pRec)
{
  int32_t res;

  if(i2sp->rx_state < I2S_STATE_READY)
    return -1;

  if(i2sp->prxbuf == NULL){
    if((i2sp->prxbuf = osBmemAlloc(rx_cfgp->frame_len * rx_cfgp->frame_num)) == NULL)
    {
      return -1;
    }

    memset(i2sp->prxbuf, 0, rx_cfgp->frame_len * rx_cfgp->frame_num);
  }

  i2sp->rx_cfgp = rx_cfgp;
  i2sp->pRec = pRec;

  res = _i2s_allocDma(i2sp, AUDIO_STREAM_RECORD);
  if(res < 0)
    return -1;

  _lld_enableRxI2s();
  _i2s_hw_init(i2sp->rx_i2s, rx_cfgp, AUDIO_STREAM_RECORD);
  _i2s_setup_hwparams(i2sp, pRec, AUDIO_STREAM_RECORD);

  dmaStreamStartByLli(i2sp->pdmarx);
  _i2s_record_start(i2sp);

  i2sp->rx_state = I2S_STATE_WORKING;
  return 0;
}

int32_t i2s_lld_play_start(I2SDriver *i2sp, hs_i2s_config_t *tx_cfgp, hs_audio_stream_t *pPly)
{
  int32_t res;

  if(i2sp->tx_state < I2S_STATE_READY)
    return -1;

  if(i2sp->ptxbuf == NULL)
  {
    if((i2sp->ptxbuf = osBmemAlloc(tx_cfgp->frame_len * tx_cfgp->frame_num)) == NULL)
    {
      //uint32_t cnt, size;
      
      //cnt = osBmemGetInfo(&size);
      //hs_printf("dma memory pool free size: 0x%X(%d), alloc size:0x%X!\r\n", size, cnt, (tx_cfgp->frame_len * tx_cfgp->frame_num));
      return -1;
    }

    memset(i2sp->ptxbuf, 0, tx_cfgp->frame_len * tx_cfgp->frame_num);
  }

  i2sp->tx_cfgp = tx_cfgp;
  i2sp->pPly = pPly;

  res = _i2s_allocDma(i2sp, AUDIO_STREAM_PLAYBACK);
  if(res < 0)
  {
    osBmemFree(i2sp->ptxbuf);
    return -1;
  }

  _lld_enableTxI2s();
  _i2s_hw_init(i2sp->tx_i2s, tx_cfgp, AUDIO_STREAM_PLAYBACK);
  _i2s_setup_hwparams(i2sp, pPly, AUDIO_STREAM_PLAYBACK);

  _i2s_play_start(i2sp);

  i2sp->tx_state = I2S_STATE_WORKING;
  return 0;
}

int32_t i2s_lld_record_stop(I2SDriver *i2sp)
{
  if(i2sp->pdmarx)
  {
    dmaStreamDisable(i2sp->pdmarx);
  }

  if(i2sp->prxbuf != NULL){
    osBmemFree(i2sp->prxbuf);
    i2sp->prxbuf = NULL;
  }

  __i2s_enable_rx(i2sp->rx_i2s, 0);
  __i2s_enable_rxChn(i2sp->rx_i2s, 0);

  __i2s_set_enable(i2sp->rx_i2s, 0);

  dmaStreamRelease(i2sp->pdmarx);
  i2sp->pdmarx = NULL;

  _lld_disableRxI2s();
  i2sp->rx_state = I2S_STATE_READY;

  return 0;
}

int32_t i2s_lld_play_stop(I2SDriver *i2sp)
{
  if(i2sp->pdmatx)
  {
    dmaStreamDisable(i2sp->pdmatx);
  }

  if(i2sp->ptxbuf != NULL)
  {
    osBmemFree(i2sp->ptxbuf);
    i2sp->ptxbuf = 0;
  }

  __i2s_enable_tx(i2sp->tx_i2s, 0);
  __i2s_enable_txChn(i2sp->tx_i2s, 0);

  __i2s_set_enable(i2sp->tx_i2s, 0);

  dmaStreamRelease(i2sp->pdmatx);
  i2sp->pdmatx = NULL;

  _lld_disableTxI2s();
  i2sp->tx_state = I2S_STATE_READY;

  return 0;
}

#endif /* HAL_USE_I2S */

/** @} */
