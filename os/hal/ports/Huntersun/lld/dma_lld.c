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
 * @file    HS66xx/dma.c
 * @brief   Enhanced DMA helper driver code.
 *
 * @addtogroup HS66xx_DMA
 * @details DMA sharing helper driver. In the HS66xx the DMA streams are a
 *          shared resource, this driver allows to allocate and free DMA
 *          streams at runtime in order to allow all the other device
 *          drivers to coordinate the access to the resource.
 * @note    The DMA ISR handlers are all declared into this module because
 *          sharing, the various device drivers can associate a callback to
 *          ISRs when allocating streams.
 * @{
 */

#include <string.h>
#include "ch.h"
#include "hal.h"


/* The following macro is only defined if some driver requiring DMA services
   has been enabled.*/
#if defined(HS_DMA_REQUIRED) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   DMA streams descriptors.
 * @details This table keeps the association between an unique stream
 *          identifier and the involved physical registers.
 */
static hs_dma_stream_t _hs_dma_streams[HS_DMA_STREAMS];

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   Mask of the allocated streams.
 */
static uint16_t dma_streams_mask;
static uint16_t dma_nr_channels;
static uint32_t all_chan_mask;

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
static void _dma_setup_lli(hs_dma_stream_t *dmastp, uint32_t ctrllo)
{
  hs_dma_config_t *cfgp = &dmastp->cfg;
  hs_dma_lli_t *llip = dmastp->llip;
  uint32_t i;

  for(i=0; i<cfgp->lli_block_num; i++)
  {
    switch(cfgp->direction)
    {
      case DMA_MEM_TO_MEM:
        llip[i].sar = cfgp->lli_src_addr + i * cfgp->lli_block_len;
        llip[i].dar = cfgp->lli_dst_addr + i * cfgp->lli_block_len;
        llip[i].ctlhi = cfgp->lli_block_len / dmaBitWidthToBytes(cfgp->dst_addr_width);
        break;
      case DMA_MEM_TO_DEV:
        llip[i].sar = cfgp->lli_src_addr + i * cfgp->lli_block_len;
        llip[i].dar = cfgp->lli_dst_addr;
        llip[i].ctlhi = cfgp->lli_block_len / dmaBitWidthToBytes(cfgp->dst_addr_width);
        break;
      case DMA_DEV_TO_MEM:
        llip[i].sar = cfgp->lli_src_addr;
        llip[i].dar = cfgp->lli_dst_addr + i * cfgp->lli_block_len;
        llip[i].ctlhi = cfgp->lli_block_len / dmaBitWidthToBytes(cfgp->src_addr_width);
        break;
      default:
        llip[i].llp = 0;
        return ;
    }

    llip[i].ctllo = ctrllo;
    llip[i].llp = (uint32_t)&llip[i+1];
  }

  if(cfgp->lli_loop_en)
  {
    llip[cfgp->lli_block_num-1].llp = (uint32_t)&llip[0];
  }
  else
  {
    llip[cfgp->lli_block_num-1].llp = 0;
  }
}

static hs_dma_stream_t * _dma_getIdleChannel(void)
{
  uint16_t i, idx = 0;

  for(i=0; i<dma_nr_channels; i++)
  {
    if((((1u << i ) & dma_streams_mask) == 0)
      || (_hs_dma_streams[i].state == DMA_STATE_IDLE))
    {
      idx = i;
      break;
    }
  }

  if(i<dma_nr_channels)
  {
    dma_streams_mask |= 1u << idx;
    return &_hs_dma_streams[idx];
  }

  return NULL;
}

static void _dma_releaseChannel(hs_dma_stream_t *pStream)
{
  uint16_t i, idx = 0;

  for(i=0; i<dma_nr_channels; i++)
  {
    if((((1u << i ) & dma_streams_mask))
      && ((uint32_t)&_hs_dma_streams[i] == (uint32_t)pStream))
    {
      idx = i;
      break;
    }
  }

  if(i<dma_nr_channels)
  {
    dma_streams_mask &= ~(1u << idx);
    _hs_dma_streams[idx].state = DMA_STATE_IDLE;
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

static void dma_save_interrupt(void){
  uint32_t i, status_xfer, status_err, status_block;
  HS_DMAC_Type *dmac = HS_DMAC;
  hs_dma_cb_para_t cb_para;
  hs_dma_stream_t *dmastp;

  chSysLockFromISR();
  if (dmac->STATUS_INT) {
    /* BUG: Unexpected interrupts pending, try to recover */
    dmaStreamClearBits(dmac->MASK.XFER, (1 << 8) - 1);
    dmaStreamClearBits(dmac->MASK.SRC_TRAN, (1 << 8) - 1);
    dmaStreamClearBits(dmac->MASK.DST_TRAN, (1 << 8) - 1);
  }

  status_xfer = dmac->RAW.XFER;
  status_block = dmac->RAW.BLOCK;
  status_err = dmac->RAW.ERROR;

  for (i = 0; i < dma_nr_channels; i++)
  {
    dmastp = &_hs_dma_streams[i];
    cb_para.oper_res = DMA_TRANS_OPER_OK;

    if (status_err & (1 << i))
    {
      dmastp->dmac->CLEAR.ERROR = dmastp->mask;
      cb_para.oper_res = DMA_TRANS_OPER_ERROR;
    }

    cb_para.curr_src_addr = dmastp->stream->SAR;
    cb_para.curr_dst_addr = dmastp->stream->DAR;
    cb_para.xfer_size = dmastp->stream->CTL_HI & 0x0FFF;

    if ((status_xfer & (1 << i)) || (status_block & (1 << i)))
    {
      if (dmastp->dma_func)
      {
        dmastp->dmac->CLEAR.XFER = dmastp->mask;
        dmastp->dmac->CLEAR.BLOCK = dmastp->mask;
        dmastp->dmac->CLEAR.SRC_TRAN = dmastp->mask;
        dmastp->dmac->CLEAR.DST_TRAN = dmastp->mask;
        dmastp->dma_func(dmastp->dma_param, &cb_para);
      }

      if(status_xfer & (1 << i))
        dmastp->state = DMA_STATE_READY;
    }
  }
  chSysUnlockFromISR();
}

/**
 * @brief   DMAC interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(DMAC_IRQHandler) {
  CH_IRQ_PROLOGUE();
  dma_save_interrupt();
  CH_IRQ_EPILOGUE();
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   HS DMA helper initialization.
 *
 * @init
 */
void dmaInit(void) {
  int i;
  HS_DMAC_Type *dmac = HS_DMAC;

  cpmEnableDMA();
  dma_nr_channels = HS_DMA_STREAMS;
  all_chan_mask = (1 << dma_nr_channels) - 1;

  /* disable */
  dmac->CFG = 0;
  /* clear bits */
  dmaStreamClearBits(dmac->MASK.XFER, all_chan_mask);
  dmaStreamClearBits(dmac->MASK.SRC_TRAN, all_chan_mask);
  dmaStreamClearBits(dmac->MASK.DST_TRAN, all_chan_mask);
  dmaStreamClearBits(dmac->MASK.ERROR, all_chan_mask);
  while (dmac->CFG & DW_CFG_DMA_EN)
    ;

  /* disable BLOCK interrupts as well */
  dmaStreamClearBits(dmac->MASK.BLOCK, all_chan_mask);

  dma_streams_mask = 0;
  for (i = 0; i < dma_nr_channels; i++) {
    hs_dma_stream_t *dmastp = &_hs_dma_streams[i];
    int r = dma_nr_channels - i - 1;

    dmastp->stream = HS_DMA_CH0 + i;
    dmastp->dmac = dmac;

    dmastp->selfindex = i;
    dmastp->mask = 1 << i;
    /* 7 is highest priority & 0 is lowest. */
    dmastp->priority = i;

    dmastp->hwinfo.max_block_size = (4 << ((dmac->MAX_BLK_SIZE >> 4 * i) & 0xf)) - 1;
    dmastp->hwinfo.lli_support = (dmac->DWC_PARAMS[r] >> DWC_PARAMS_MBLK_EN & 0x1);
    dmastp->hwinfo.max_burst_len = (hs_dma_burstlen_t)((((dmac->DWC_PARAMS[r] >> DWC_PARAMS_MAX_MULT_SIZE) & 0x7) + 2) << 1);

    dmaStreamClearBits(dmac->CH_EN, dmastp->mask);
    dmastp->dma_func = NULL;
    dmastp->dma_param = NULL;
    dmastp->llip = NULL;

    dmastp->state = DMA_STATE_IDLE;
  }

  /* Clear all interrupts on all channels. */
  dmac->CLEAR.XFER = all_chan_mask;
  dmac->CLEAR.BLOCK = all_chan_mask;
  dmac->CLEAR.SRC_TRAN = all_chan_mask;
  dmac->CLEAR.DST_TRAN = all_chan_mask;
  dmac->CLEAR.ERROR = all_chan_mask;

  /* enable */
  dmac->CFG = DW_CFG_DMA_EN;

  cpmDisableDMA();
}

/**
 * @brief   Allocates a DMA stream.
 * @details The stream is allocated and, if required, the DMA clock enabled.
 *          The function also enables the IRQ vector associated to the stream
 *          and initializes its priority.
 * @pre     The stream must not be already in use or an error is returned.
 * @post    The stream is allocated and the default ISR handler redirected
 *          to the specified function.
 * @post    The stream ISR vector is enabled and its priority configured.
 * @post    The stream must be freed using @p dmaStreamRelease() before it can
 *          be reused with another peripheral.
 * @post    The stream is in its post-reset state.
 * @note    This function can be invoked in both ISR or thread context.
 *
 * @param[in] priority  IRQ priority mask for the DMA stream
 * @param[in] func      handling function pointer, can be @p NULL
 * @param[in] param     a parameter to be passed to the handling function
 * @return              dma stream handle.
 * @retval NULL         no idle dma channel to be used.
 *
 * @special
 */
hs_dma_stream_t * dmaStreamAllocate(uint32_t priority, hs_dmaisr_t func, void *param)
{
  hs_dma_stream_t *dmastp;

  /* Enabling DMA clocks required by the current streams set.*/
  if (dma_streams_mask == 0)
  {
    cpmEnableDMA();
    nvicEnableVector(IRQ_DMA, ANDES_PRIORITY_MASK(HS_DMA_IRQ_PRIORITY));
  }

  dmastp = _dma_getIdleChannel();
  if(dmastp == NULL)
    return NULL;

  /* Marks the stream as allocated.*/
  dmastp->dma_func  = func;
  dmastp->dma_param = param;
  dmastp->priority = priority;

  /* Putting the stream in a safe state.*/
  dmaStreamDisable(dmastp);

  dmastp->state = DMA_STATE_STOP;

  return dmastp;
}

/**
 * @brief   Releases a DMA stream.
 * @details The stream is freed and, if required, the DMA clock disabled.
 *          Trying to release a unallocated stream is an illegal operation
 *          and is trapped if assertions are enabled.
 * @pre     The stream must have been allocated using @p dmaStreamAllocate().
 * @post    The stream is again available.
 * @note    This function can be invoked in both ISR or thread context.
 *
 * @param[in] dmastp    pointer to a hs_dma_stream_t structure
 *
 * @special
 */
void dmaStreamRelease(hs_dma_stream_t *dmastp)
{
  if(dmastp == NULL)
    return ;

  _dma_releaseChannel(dmastp);

  /* Shutting down clocks that are no more required, if any.*/
  dmaStreamDisable(dmastp);
  if (dma_streams_mask == 0)
  {
    nvicDisableVector(IRQ_DMA);
    cpmDisableDMA();
  }

  if(dmastp->llip != NULL)
  {
    /* locked in the upper layer, in audio.c */
    osBmemFree(dmastp->llip);
    dmastp->llip = NULL;
  }

  if (dmastp->dma_func)
  {
    /* Disable interrupts */
    dmaStreamClearBits(dmastp->dmac->MASK.XFER, dmastp->mask);
    dmaStreamClearBits(dmastp->dmac->MASK.BLOCK, dmastp->mask);
    dmaStreamClearBits(dmastp->dmac->MASK.ERROR, dmastp->mask);
  }

  dmastp->dma_func = NULL;
  dmastp->dma_param = NULL;
}

/**
 * @brief   Programs the stream mode settings.
 * @note    This function can be invoked in both ISR or thread context.
 * @pre     The stream must have been allocated using @p dmaStreamAllocate().
 * @post    After use the stream can be released using @p dmaStreamRelease().
 *
 * @param[in] dmastp    pointer to a hs_dma_stream_t structure
 * @param[in] sconfig   dma slave config
 *
 * @special
 */
void dmaStreamSetMode(hs_dma_stream_t *dmastp, hs_dma_config_t *sconfig)
{
  uint32_t ctllo = 0, cfghi = 0, cfglo;

  if(dmastp == NULL)
    return ;

  if(sconfig != NULL)
    memcpy(&dmastp->cfg, sconfig, sizeof(hs_dma_config_t));

  if(sconfig->lli_en)
  {
    ctllo = DWC_CTLL_LLP_D_EN | DWC_CTLL_LLP_S_EN;
    /* locked in the upper layer, in audio.c */
    dmastp->llip = (hs_dma_lli_t *)osBmemAlloc(sizeof(hs_dma_lli_t) * sconfig->lli_block_num);
  }
  ctllo |= DWC_CTLL_DST_MSIZE(sconfig->dst_burst)
         | DWC_CTLL_SRC_MSIZE(sconfig->src_burst);
  cfglo = DWC_CFGL_CH_PRIOR(dmastp->priority);

  switch (sconfig->direction)
  {
    case DMA_MEM_TO_MEM:
      ctllo |= 0
            | DWC_CTLL_DST_WIDTH(sconfig->dst_addr_width)
            | DWC_CTLL_SRC_WIDTH(sconfig->src_addr_width)
            | DWC_CTLL_DMS(1)
            | DWC_CTLL_SMS(0)
            | DWC_CTLL_DST_INC
            | DWC_CTLL_SRC_INC
            | DWC_CTLL_FC_M2M;
      cfghi = DWC_CFGH_FCMODE;
      cfglo |= DWC_CFGL_HS_SRC | DWC_CFGL_HS_DST;
      return;
    case DMA_MEM_TO_DEV:
      ctllo |= 0
            | DWC_CTLL_DST_WIDTH(sconfig->dst_addr_width)
            | DWC_CTLL_SRC_WIDTH(sconfig->src_addr_width)
            | DWC_CTLL_DMS(1)
            | DWC_CTLL_SMS(0)
            | DWC_CTLL_DST_FIX
            | DWC_CTLL_SRC_INC
            | (sconfig->dev_flow_ctrl ? DWC_CTLL_FC(DW_DMA_FC_P_M2P) :
  	                                    DWC_CTLL_FC(DW_DMA_FC_D_M2P));

      cfghi = DWC_CFGH_DST_PER(sconfig->slave_id) | DWC_CFGH_FCMODE ;
      cfglo |= DWC_CFGL_HS_SRC;
      break;

    case DMA_DEV_TO_MEM:
      ctllo |= 0
            | DWC_CTLL_DST_WIDTH(sconfig->dst_addr_width)
            | DWC_CTLL_SRC_WIDTH(sconfig->src_addr_width)
            | DWC_CTLL_DMS(0)
            | DWC_CTLL_SMS(1)
            | DWC_CTLL_DST_INC
            | DWC_CTLL_SRC_FIX
            | (sconfig->dev_flow_ctrl ? DWC_CTLL_FC(DW_DMA_FC_P_P2M) :
  	                                    DWC_CTLL_FC(DW_DMA_FC_D_P2M));

      cfghi = DWC_CFGH_SRC_PER(sconfig->slave_id) | DWC_CFGH_FCMODE ;
      cfglo |= DWC_CFGL_HS_DST;
      break;
    default:
    	break;
  }

  ctllo |= DWC_CTLL_INT_EN;

  if(sconfig->lli_en)
  {
    _dma_setup_lli(dmastp, ctllo);
#if defined(__nds32__)
    nds32_dcache_clean();
#endif
    dmastp->stream->LLP = (uint32_t)dmastp->llip;
    dmastp->stream->CTL_HI = dmastp->llip->ctlhi;
  }
  else
  {
    dmastp->stream->LLP = 0;
  }

  dmastp->stream->CTL_LO = ctllo;
  dmastp->stream->CFG_LO = cfglo;
  dmastp->stream->CFG_HI = cfghi;

  if (dmastp->dma_func)
  {
    /* Enable interrupts */
    dmaStreamSetBits(dmastp->dmac->MASK.XFER, dmastp->mask);
    dmaStreamSetBits(dmastp->dmac->MASK.BLOCK, dmastp->mask);
    dmaStreamSetBits(dmastp->dmac->MASK.ERROR, dmastp->mask);
  }

  dmastp->state = DMA_STATE_READY;
}

/**
 * @brief   Starts a memory to memory operation using the specified stream.
 * @note    The default transfer data mode is "byte to byte" but it can be
 *          changed by specifying extra options in the @p mode parameter.
 * @pre     The stream must have been allocated using @p dmaStreamAllocate().
 * @post    After use the stream can be released using @p dmaStreamRelease().
 *
 * @param[in] dmastp    pointer to a hs_dma_stream_t structure
 * @param[in] src       source address
 * @param[in] dst       destination address
 * @param[in] n         number of data units to copy
 */
void dmaStartMemCopy(hs_dma_stream_t *dmastp, uint8_t *src, uint8_t *dst, size_t n) {
  uint32_t src_width, dst_width, ctllo;
  size_t xfer_count;

  src_width = dst_width = DMA_SLAVE_BUSWIDTH_8BITS;
  xfer_count = min(n >> src_width, dmastp->hwinfo.max_block_size);

  ctllo = dmastp->stream->CTL_LO
    | DWC_CTLL_DST_MSIZE(DW_DMA_MSIZE_1)
    | DWC_CTLL_SRC_MSIZE(DW_DMA_MSIZE_1)
    | DWC_CTLL_DST_WIDTH(dst_width)
    | DWC_CTLL_SRC_WIDTH(src_width)
    | DWC_CTLL_DST_INC
    | DWC_CTLL_SRC_INC;

  ctllo &= ~(7 << 20);

  dmastp->stream->CTL_LO = ctllo | DWC_CTLL_INT_EN;
  dmastp->stream->CTL_HI = xfer_count;

  dmastp->stream->CFG_LO = DWC_CFGL_CH_PRIOR(dmastp->priority);
  dmastp->stream->CFG_HI = DWC_CFGH_FIFO_MODE;

  dmastp->stream->SAR = (uint32_t)src;
  dmastp->stream->DAR = (uint32_t)dst;

  if (dmastp->dma_func) {
    /* Enable interrupts */
    dmaStreamSetBits(dmastp->dmac->MASK.XFER, dmastp->mask);
    dmaStreamSetBits(dmastp->dmac->MASK.ERROR, dmastp->mask);
  }

  dmaStreamEnable(dmastp);
}

/**
 * @brief   Polled wait for DMA transfer end.
 * @pre     The stream must have been allocated using @p dmaStreamAllocate().
 * @post    After use the stream can be released using @p dmaStreamRelease().
 *
 * @param[in] dmastp    pointer to a hs_dma_stream_t structure
 */
void dmaWaitCompletion(hs_dma_stream_t *dmastp)
{
  int16_t cnt = 10;

  while ((0 == (dmastp->dmac->RAW.XFER & dmastp->mask)) && (cnt-- > 0))
    chThdSleepS(MS2ST(100));

  dmastp->dmac->CLEAR.XFER = dmastp->mask;
  dmaStreamDisable(dmastp);
}

#endif /* HS_DMA_REQUIRED */

/** @} */
