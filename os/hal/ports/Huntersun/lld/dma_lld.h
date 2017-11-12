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
 * @file    HS66xx/dma.h
 * @brief   Enhanced-DMA helper driver header.
 *
 * @addtogroup HS66xx_DMA
 * @{
 */

#ifndef _HS_DMA_H_
#define _HS_DMA_H_

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

#define DW_DMA_MAX_NR_CHANNELS	8

/* flow controller */
enum dw_dma_fc {
  DW_DMA_FC_D_M2M,
  DW_DMA_FC_D_M2P,
  DW_DMA_FC_D_P2M,
  DW_DMA_FC_D_P2P,
  DW_DMA_FC_P_P2M,
  DW_DMA_FC_SP_P2P,
  DW_DMA_FC_P_M2P,
  DW_DMA_FC_DP_P2P,
};

/* bursts size */
enum dw_dma_msize {
  DW_DMA_MSIZE_1,
  DW_DMA_MSIZE_4,
  DW_DMA_MSIZE_8,
  DW_DMA_MSIZE_16,
  DW_DMA_MSIZE_32,
  DW_DMA_MSIZE_64,
  DW_DMA_MSIZE_128,
  DW_DMA_MSIZE_256,
};

enum dw_dmac_flags {
  DW_DMA_IS_CYCLIC = 0,
  DW_DMA_IS_SOFT_LLP = 1,
};

/**
 * @brief   Total number of DMA streams.
 * @note    This is the total number of streams among all the DMA units.
 */
#define HS_DMA_STREAMS           3

/**
 * @brief   Checks if a DMA priority is within the valid range.
 * @param[in] prio      DMA priority
 *
 * @retval              The check result.
 * @retval FALSE        invalid DMA priority.
 * @retval TRUE         correct DMA priority.
 */
#define HS_DMA_IS_VALID_PRIORITY(prio) (((prio) >= 0) && ((prio) <= 7))
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   DMA controller interrupt priority level setting.
 */
#if !defined(HS_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define HS_DMA_IRQ_PRIORITY             3
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * enum dma_transfer_direction - dma transfer mode and direction indicator
 * @DMA_MEM_TO_MEM: Async/Memcpy mode
 * @DMA_MEM_TO_DEV: Slave mode & From Memory to Device
 * @DMA_DEV_TO_MEM: Slave mode & From Device to Memory
 * @DMA_DEV_TO_DEV: Slave mode & From Device to Device
 */
typedef enum {
  DMA_MEM_TO_MEM,
  DMA_MEM_TO_DEV,
  DMA_DEV_TO_MEM,
  DMA_DEV_TO_DEV,
  DMA_TRANS_NONE,
} hs_dma_dir_t;

/**
 * enum dma_slave_buswidth - defines bus with of the DMA slave
 * device, source or target buses
 */
enum dma_slave_buswidth {
  DMA_SLAVE_BUSWIDTH_8BITS = 0,
  DMA_SLAVE_BUSWIDTH_16BITS,
  DMA_SLAVE_BUSWIDTH_32BITS,
  DMA_SLAVE_BUSWIDTH_64BITS,
  DMA_SLAVE_BUSWIDTH_128BITS,
  DMA_SLAVE_BUSWIDTH_256BITS,
  DMA_SLAVE_BUSWIDTH_MAXBITS,
};

enum {
  DMA_TRANS_OPER_OK = 0,
  DMA_TRANS_OPER_ERROR = 0xff,
};

typedef enum {
  DMA_STATE_IDLE,
  DMA_STATE_STOP,
  DMA_STATE_READY,
  DMA_STATE_WORK,
} hs_dma_state_t;

typedef enum {
  DMA_BURST_LEN_1UNITS = 0, /* a unit length equal to SRC/DST_TR_WIDTH */
  DMA_BURST_LEN_4UNITS,
  DMA_BURST_LEN_8UNITS,
  DMA_BURST_LEN_16UNITS,
  DMA_BURST_LEN_32UNITS,
  DMA_BURST_LEN_64UNITS,
  DMA_BURST_LEN_128UNITS,
  DMA_BURST_LEN_256UNITS,

  DMA_BURST_LEN_RESERVE,
} hs_dma_burstlen_t;

typedef enum {
  I2S_TX_DMA_ID,
  I2S_RX_DMA_ID,
  UART1_TX_DMA_ID,
  UART1_RX_DMA_ID,
  I2C_TX_DMA_ID,
  I2C_RX_DMA_ID,
  SPI0_TX_DMA_ID,
  SPI0_RX_DMA_ID,
  SPI1_TX_DMA_ID,
  SPI1_RX_DMA_ID,
  TIMER0_DMA_ID,
  TIMER1_DMA_ID,
  TIMER2_DMA_ID,
  ADC_DMA_ID,
  DMA_MAX_ID
} hs_dma_id_t;

/* LLI == Linked List Item; a.k.a. DMA block descriptor */
typedef struct dw_lli {
  /* values that are not changed by hardware */
  uint32_t sar;
  uint32_t dar;
  uint32_t llp; /* chain to next lli */
  uint32_t ctllo;
  /* values that may get written back: */
  uint32_t ctlhi;
  /* sstat and dstat can snapshot peripheral register state.
   * silicon config may discard either or both...
   */
  uint32_t sstat;
  uint32_t dstat;
} hs_dma_lli_t;

/* DMA API extensions */
struct dw_cyclic_desc {
  unsigned long periods;
  void (*period_callback)(void *param);
  void *period_callback_param;
};

typedef struct {
  uint32_t oper_res;
  uint32_t curr_src_addr;
  uint32_t curr_dst_addr;
  size_t xfer_size;
} hs_dma_cb_para_t;

/**
 * @brief   HS DMA ISR function type.
 *
 * @param[in] p         parameter for the registered function
 * @param[in] flags     pre-shifted content of the xISR register, the bits
 *                      are aligned to bit zero
 */
typedef void (*hs_dmaisr_t)(void *p, hs_dma_cb_para_t *var);

/**
 * struct dma_slave_config - dma slave channel runtime config
 * @direction: whether the data shall go in or out on this slave
 * channel, right now. DMA_TO_DEVICE and DMA_FROM_DEVICE are
 * legal values, DMA_BIDIRECTIONAL is not acceptable since we
 * need to differentiate source and target addresses.
 * @src_addr: this is the physical address where DMA slave data
 * should be read (RX), if the source is memory this argument is
 * ignored.
 * @dst_addr: this is the physical address where DMA slave data
 * should be written (TX), if the source is memory this argument
 * is ignored.
 * @src_addr_width: this is the width in bytes of the source (RX)
 * register where DMA data shall be read. If the source
 * is memory this may be ignored depending on architecture.
 * Legal values: 1, 2, 4, 8.
 * @dst_addr_width: same as src_addr_width but for destination
 * target (TX) mutatis mutandis.
 * @src_maxburst: the maximum number of words (note: words, as in
 * units of the src_addr_width member, not bytes) that can be sent
 * in one burst to the device. Typically something like half the
 * FIFO depth on I/O peripherals so you don't overflow it. This
 * may or may not be applicable on memory sources.
 * @dst_maxburst: same as src_maxburst but for destination target
 * mutatis mutandis.
 * @device_fc: Flow Controller Settings. Only valid for slave channels. Fill
 * with 'true' if peripheral should be flow controller. Direction will be
 * selected at Runtime.
 * @slave_id: Slave requester id. Only valid for slave channels. The dma
 * slave peripheral will have unique id as dma requester which need to be
 * pass as slave config.
 *
 * This struct is passed in as configuration data to a DMA engine
 * in order to set up a certain channel for DMA transport at runtime.
 * The DMA device/engine has to provide support for an additional
 * command in the channel config interface, DMA_SLAVE_CONFIG
 * and this struct will then be passed in as an argument to the
 * DMA engine device_control() function.
 *
 * The rationale for adding configuration information to this struct
 * is as follows: if it is likely that most DMA slave controllers in
 * the world will support the configuration option, then make it
 * generic. If not: if it is fixed so that it be sent in static from
 * the platform data, then prefer to do that. Else, if it is neither
 * fixed at runtime, nor generic enough (such as bus mastership on
 * some CPU family and whatnot) then create a custom slave config
 * struct and pass that, then make this config a member of that
 * struct, if applicable.
 */
typedef struct dma_slave_config {
  hs_dma_dir_t direction;
  enum dma_slave_buswidth src_addr_width;
  enum dma_slave_buswidth dst_addr_width;

  hs_dma_burstlen_t src_burst;
  hs_dma_burstlen_t dst_burst;
  bool_t dev_flow_ctrl;
  hs_dma_id_t slave_id;

  bool_t lli_en;
  bool_t lli_loop_en;
  uint32_t lli_block_num;
  uint32_t lli_block_len;
  uint32_t lli_src_addr;
  uint32_t lli_dst_addr;
} hs_dma_config_t;

typedef struct {
  bool_t lli_support; /**< @brief No Linked List Pointer */
  uint32_t max_block_size; /**< @brief Maximum block size */
  hs_dma_burstlen_t max_burst_len;
} hs_dma_hwinfo_t;

/**
 * @brief   DMA stream descriptor structure.
 */
typedef struct {
  HS_DMA_CH_Type *stream; /**< @brief Associated DMA stream.  */
  HS_DMAC_Type *dmac; /**< @brief DMA controller.  */

  uint32_t state;
  uint32_t selfindex; /**< @brief Index to self in array. */
  uint32_t mask;
  uint32_t priority;

  hs_dma_config_t cfg;
  hs_dma_hwinfo_t hwinfo;
  hs_dma_lli_t *llip;

  hs_dmaisr_t dma_func; /**< @brief DMA callback function.  */
  void *dma_param; /**< @brief DMA callback parameter. */
} hs_dma_stream_t;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

#define min_t(type, x, y) ({			\
	type __min1 = (x);			\
	type __min2 = (y);			\
	__min1 < __min2 ? __min1: __min2; })

#define max_t(type, x, y) ({			\
	type __max1 = (x);			\
	type __max2 = (y);			\
	__max1 > __max2 ? __max1: __max2; })

#define dmaStreamClearBits(reg, mask) (reg) = (((mask) << 8) | 0)
#define dmaStreamSetBits(reg, mask)   (reg) = (((mask) << 8) | (mask))

/* width type: dma_slave_buswidth */
#define dmaBitWidthToBytes(width)     (1u<<width)

#define dmaGetCurSrcAddr(dmastp, src)     (src) = (dmastp)->stream->SAR
#define dmaGetCurDstAddr(dmastp, dst)     (dst) = (dmastp)->stream->DAR

/**
 * @name    Macro Functions
 * @{
 */
/**
 * @brief   Associates a memory source and destination to a DMA stream.
 * @note    This function can be invoked in both ISR or thread context.
 * @pre     The stream must have been allocated using @p dmaStreamAllocate().
 * @post    After use the stream can be released using @p dmaStreamRelease().
 *
 * @param[in] dmastp    pointer to a hs_dma_stream_t structure
 * @param[in] src_addr  source address
 * @param[in] dst_addr  destination address
 *
 * @special
 */
#define dmaStreamSetAddress(dmastp, src_addr, dst_addr) {                   \
  (dmastp)->stream->SAR  = (uint32_t)(src_addr);                            \
  (dmastp)->stream->DAR  = (uint32_t)(dst_addr);                            \
}

/**
 * @brief   Sets the number of transfers to be performed.
 * @note    This function can be invoked in both ISR or thread context.
 * @pre     The stream must have been allocated using @p dmaStreamAllocate().
 * @post    After use the stream can be released using @p dmaStreamRelease().
 *
 * @param[in] dmastp    pointer to a hs_dma_stream_t structure
 * @param[in] size      transfer size in byte
 *
 * @special
 */
#define dmaStreamSetTransactionSize(dmastp, size) {                         \
  (dmastp)->stream->CTL_HI  = (uint32_t)(size);                             \
}

/**
 * @brief   Returns the number of transfers to be performed.
 * @note    This function can be invoked in both ISR or thread context.
 * @pre     The stream must have been allocated using @p dmaStreamAllocate().
 * @post    After use the stream can be released using @p dmaStreamRelease().
 *
 * @param[in] dmastp    pointer to a hs_dma_stream_t structure
 * @return              The number of transfers to be performed.
 *
 * @special
 */
#define dmaStreamGetTransactionSize(dmastp) ((size_t)((dmastp)->stream->CTL_HI))

/**
 * @brief   DMA stream enable.
 * @note    This function can be invoked in both ISR or thread context.
 * @pre     The stream must have been allocated using @p dmaStreamAllocate().
 * @post    After use the stream can be released using @p dmaStreamRelease().
 *
 * @param[in] dmastp    pointer to a hs_dma_stream_t structure
 *
 * @special
 */
#define dmaStreamEnable(dmastp) {                                           \
  dmaStreamSetBits((dmastp)->dmac->CH_EN, (dmastp)->mask);                  \
  dmastp->state = DMA_STATE_WORK;                                           \
}

/**
 * @brief   DMA stream start to transfer without lli.
 * @param[in] dmastp    pointer to a hs_dma_stream_t structure
 *
 * @special
 */
#define dmaStreamStart(dmastp, src_addr, dst_addr, size) {                  \
  dmaStreamSetAddress(dmastp, src_addr, dst_addr);                          \
  dmaStreamSetTransactionSize(dmastp, size);                                \
  dmaStreamEnable(dmastp);                                                  \
}

/**
 * @brief   DMA stream start to transfer with lli.
 * @param[in] dmastp    pointer to a hs_dma_stream_t structure
 *
 * @special
 */
#define dmaStreamStartByLli(dmastp) {                                       \
  dmaStreamEnable(dmastp);                                                  \
}

/**
 * @brief   DMA stream disable.
 * @details The function disables the specified stream, waits for the disable
 *          operation to complete and then clears any pending interrupt.
 * @note    This function can be invoked in both ISR or thread context.
 * @note    Interrupts enabling flags are set to zero after this call, see
 *          bug 3607518.
 * @pre     The stream must have been allocated using @p dmaStreamAllocate().
 * @post    After use the stream can be released using @p dmaStreamRelease().
 *
 * @param[in] dmastp    pointer to a hs_dma_stream_t structure
 *
 * @special
 */
#if 1
#define dmaStreamDisable(dmastp) {                                          \
  dmaStreamClearBits((dmastp)->dmac->CH_EN, (dmastp)->mask);                \
  uint32_t tmp = (dmastp)->dmac->CH_EN;                                     \
  while (tmp & dmastp->mask)                                                \
      tmp = (dmastp)->dmac->CH_EN;                                          \
  dmaStreamClearInterrupt(dmastp);                                          \
  dmastp->state = DMA_STATE_READY;                                           \
}
#else
#define dmaStreamDisable(dmastp) {                                          \
  dmaStreamClearBits((dmastp)->dmac->CH_EN, (dmastp)->mask);                \
  while ((dmastp)->dmac->CH_EN & dmastp->mask)                              \
      ;                                                                     \
  dmaStreamClearInterrupt(dmastp);                                          \
  dmastp->state = DMA_STATE_READY;                                           \
}
#endif

/**
 * @brief   DMA stream interrupt sources clear.
 * @note    This function can be invoked in both ISR or thread context.
 * @pre     The stream must have been allocated using @p dmaStreamAllocate().
 * @post    After use the stream can be released using @p dmaStreamRelease().
 *
 * @param[in] dmastp    pointer to a hs_dma_stream_t structure
 *
 * @special
 */
#define dmaStreamClearInterrupt(dmastp) {                                   \
  (dmastp)->dmac->CLEAR.XFER = (dmastp)->mask;                              \
  (dmastp)->dmac->CLEAR.ERROR = (dmastp)->mask;                             \
}
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
void dmaInit(void);
hs_dma_stream_t * dmaStreamAllocate(uint32_t priority, hs_dmaisr_t func,
    void *param);
void dmaStreamRelease(hs_dma_stream_t *dmastp);
void dmaStreamSetMode(hs_dma_stream_t *dmastp, hs_dma_config_t *sconfig);

void dmaStartMemCopy(hs_dma_stream_t *dmastp, uint8_t *src, uint8_t *dst,
    size_t n);
void dmaWaitCompletion(hs_dma_stream_t *dmastp);
#ifdef __cplusplus
}
#endif

#endif /* _HS_DMA_H_ */

/** @} */
