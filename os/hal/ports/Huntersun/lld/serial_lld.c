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
 * @file    hs66xx/serial_lld.c
 * @brief   Serial Driver subsystem low level driver source.
 *
 * @addtogroup SERIAL
 * @{
 */

#include "ch.h"
#include "hal.h"

#if HAL_USE_SERIAL || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define HAL_SERIAL_USE_DMA FALSE

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief UART0 serial driver identifier.*/
#if HS_SERIAL_USE_UART0 && !defined(__DOXYGEN__)
SerialDriver SD0;
#endif

/** @brief UART1 serial driver identifier.*/
#if HS_SERIAL_USE_UART1 && !defined(__DOXYGEN__)
SerialDriver SD1;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   Driver default configuration.
 */
static const SerialConfig default_config = {
  SERIAL_DEFAULT_BITRATE,
  FALSE,
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   UART initialization.
 * @details This function must be invoked with interrupts disabled.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 * @param[in] config    the architecture-dependent serial driver configuration
 */
static void uart_init(SerialDriver *sdp, const SerialConfig *config) {
  HS_UART_Type *u = sdp->uart;

#define MODE_X_DIV 16
  /* Compute divisor value. Normally, we should simply return:
   *   NS16550_CLK / MODE_X_DIV / baudrate
   * but we need to round that value by adding 0.5.
   * Rounding is especially important at high baud rates.
   */
  int baud_divisor =  (sdp->clock + (config->speed * (MODE_X_DIV / 2))) /
    (MODE_X_DIV * config->speed);

  u->IER = 0x00;
  u->LCR = UART_LCR_DLAB;
  u->DLL = 0;
  u->DLH = 0;

  u->LCR = 0x00;
  if (HS_UART0 != u) {
    if (config->ctsrts)
      u->MCR = UART_MCR_RTS;
    else
      u->MCR = 0;
    u->FCR = UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT |
      UART_FCR_FIFO_EN | UART_FCR_TRIGGER_1;
  } else {
    u->MCR = 0x00;
    u->FCR = UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT;
  }

  /* Baud rate setting.*/
  u->LCR = UART_LCR_DLAB;
  u->DLL = baud_divisor & 0xff;
  u->DLH = (baud_divisor >> 8) & 0xff;
  /* 8 data, 1 stop, no parity */
  u->LCR = UART_LCR_8N1;

  /* enable interrupts */
  if (config->ctsrts)
    u->IER = /*UART_IER_THRI | */UART_IER_RLSI | UART_IER_RDI | UART_IER_MSI;
  else
    u->IER = /*UART_IER_THRI | */UART_IER_RLSI | UART_IER_RDI;
  sdp->can_send = TRUE;
}

/**
 * @brief   UART de-initialization.
 * @details This function must be invoked with interrupts disabled.
 *
 * @param[in] u         pointer to an UART I/O block
 */
static void uart_deinit(HS_UART_Type *u) {
  u->LCR = 0x00;
  u->IER = 0x00;
  u->FCR = UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT;
}

/**
 * @brief   Error handling routine.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 * @param[in] lsr       UART LSR register value
 */
static void set_error(SerialDriver *sdp, uint32_t lsr) {
  eventflags_t sts = 0;

  if (lsr & UART_LSR_OE)
    sts |= SD_OVERRUN_ERROR;
  if (lsr & UART_LSR_PE)
    sts |= SD_PARITY_ERROR;
  if (lsr & UART_LSR_FE)
    sts |= SD_FRAMING_ERROR;
  if (lsr & UART_LSR_BI)
    sts |= SD_BREAK_DETECTED;
  chSysLockFromISR();
  chnAddFlagsI(sdp, sts);
  chSysUnlockFromISR();
}

/**
 * @brief   Common IRQ handler.
 *
 * @param[in] sdp       communication channel associated to the UART
 */
static void serve_interrupt(SerialDriver *sdp) {
  HS_UART_Type *u = sdp->uart;
  uint8_t msr;

  while (TRUE) {
    switch (u->IIR & UART_IIR_ID) {
    case UART_IIR_NO_INT:
      return;
    case UART_IIR_BDI: //busy detect
      /* The DesignWare APB UART has an Busy Detect (0x07)
       * interrupt meaning an LCR write attempt occured while the
       * UART was busy. The interrupt must be cleared by reading
       * the UART status register (USR) and the LCR re-written. */
      u->USR;      
      break;
    case UART_IIR_MSI:
      msr = u->MSR;
      if (msr & UART_MSR_DCTS) {
	if (msr & UART_MSR_CTS) {
	  sdp->can_send = TRUE;
	  /* re-enable transmit interrupt to send if oqueue is not empty */
          chSysLockFromISR();
          if (!chOQIsEmptyI(&sdp->oqueue))
	    u->IER |= UART_IER_THRI;
          chSysUnlockFromISR();
	} else {
	  sdp->can_send = FALSE;
	}
      }
      break;
    case UART_IIR_RLSI:
      set_error(sdp, u->LSR);
      break;
    case UART_IIR_CTI: //receive timeout
    case UART_IIR_RDI: //received data
      chSysLockFromISR();
      if (chIQIsEmptyI(&sdp->iqueue))
        chnAddFlagsI(sdp, CHN_INPUT_AVAILABLE);
      chSysUnlockFromISR();
      while (u->LSR & UART_LSR_DR) {
        chSysLockFromISR();
	    if (chIQPutI(&sdp->iqueue, u->RBR) < Q_OK)
	      chnAddFlagsI(sdp, SD_OVERRUN_ERROR);
        chSysUnlockFromISR();
      }
      break;
    case UART_IIR_THRI: //tx: THR or tFIFO empty
      {
        int i = ((u->CPR >> 16) & 0x0f) * 16; //FIFO size
    	/* disable transmit interrupt if hardware flow control disallows send */
    	if (!sdp->can_send) {
    	  u->IER &= ~UART_IER_THRI;
    	  break;
    	}
    	
        if (HS_UART0 == u)
          i = 0;
        do {
          msg_t b;
    
          chSysLockFromISR();
          b = chOQGetI(&sdp->oqueue);
          chSysUnlockFromISR();
          if (b < Q_OK) {
    	    /* disable transmit interrupt if oqueue become empty */
            u->IER &= ~UART_IER_THRI;
            chSysLockFromISR();
            chnAddFlagsI(sdp, CHN_OUTPUT_EMPTY);
            chSysUnlockFromISR();
            break;
          }
          u->THR = b;
        } while (--i > 0);
      }
      break;
    default:
      (void) u->THR;
      (void) u->RBR;
    }
  }
}

#if HS_DMA_REQUIRED && HAL_SERIAL_USE_DMA && HS_SERIAL_USE_UART1
/**
 * @brief   Trigger DMA to transfer from output queue to serial port
 */
static void serial_dma_tx_trigger(SerialDriver *sdp)
{
  OutputQueue *oqp = &sdp->oqueue;
  size_t size;

  if (!chOQIsEmptyI(oqp)) {
    sdp->tx_running = TRUE;
    if (oqp->q_rdptr < oqp->q_wrptr)
      size = oqp->q_wrptr - oqp->q_rdptr;
    else
      size = oqp->q_top - oqp->q_rdptr; //don't wrap
    dmaStreamStart(sdp->dmatx, oqp->q_rdptr, &sdp->uart->THR, size);
  }
}

/**
 * @brief   DMA tx complete callback function
 */
static void serial_dma_tx_complete(SerialDriver *sdp, hs_dma_cb_para_t *var)
{
  OutputQueue *oqp = &sdp->oqueue;
  size_t size;

  if (var->oper_res != DMA_TRANS_OPER_OK) {
    return;
  }

  size = var->xfer_size;
  oqp->q_counter += size;
  oqp->q_rdptr += size;
  if (oqp->q_rdptr >= oqp->q_top)
    oqp->q_rdptr = oqp->q_buffer;
  if (notempty(&oqp->q_waiting))
    chSchReadyI(fifo_remove(&oqp->q_waiting))->p_u.rdymsg = Q_OK;

  sdp->tx_running = FALSE;
  serial_dma_tx_trigger(sdp);
}
#endif /* HAL_SERIAL_USE_DMA */

/**
 * @brief   Attempts a TX FIFO preload.
 */
static void preload(SerialDriver *sdp) {
  HS_UART_Type *u = sdp->uart;

  if (!sdp->can_send)
    return;

#if HS_DMA_REQUIRED && HAL_SERIAL_USE_DMA
  if (sdp->dmatx) {
    if (!sdp->tx_running)
      serial_dma_tx_trigger(sdp);
    return;
  }
#endif

  if (u->LSR & UART_LSR_THRE) {
    int i = ((u->CPR >> 16) & 0x0f) * 16; //FIFO size
    if (HS_UART0 == u)
      i = 0;
    do {
      msg_t b = chOQGetI(&sdp->oqueue);
      if (b < Q_OK) {
        chnAddFlagsI(sdp, CHN_OUTPUT_EMPTY);
        return;
      }
      u->THR = b;
    } while (--i > 0);
  }
  u->IER |= UART_IER_THRI;
}

#if HS_SERIAL_USE_UART0 || defined(__DOXYGEN__)
static void notify0(io_queue_t *qp) {

  (void)qp;
  preload(&SD0);
}
#endif

#if HS_SERIAL_USE_UART1 || defined(__DOXYGEN__)
static void notify1(io_queue_t *qp) {

  (void)qp;
  preload(&SD1);
}
#endif

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/
#if HS_SERIAL_USE_UART0 || defined(__DOXYGEN__)
/**
 * @brief   UART0 interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(UART0_IRQHandler) {

  CH_IRQ_PROLOGUE();

  serve_interrupt(&SD0);

  CH_IRQ_EPILOGUE();
}
#endif

#if HS_SERIAL_USE_UART1 || defined(__DOXYGEN__)
/**
 * @brief   UART1 interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(UART1_IRQHandler) {

  CH_IRQ_PROLOGUE();

  serve_interrupt(&SD1);

  CH_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level serial driver initialization.
 *
 * @notapi
 */
void sd_lld_init(void) {

#if HS_SERIAL_USE_UART0
  sdObjectInit(&SD0, NULL, notify0);
  SD0.uart = HS_UART0;
#endif

#if HS_SERIAL_USE_UART1
  sdObjectInit(&SD1, NULL, notify1);
  SD1.uart = HS_UART1;
#endif
}

/**
 * @brief   Low level serial driver configuration and (re)start.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 * @param[in] config    the architecture-dependent serial driver configuration.
 *                      If this parameter is set to @p NULL then a default
 *                      configuration is used.
 *
 * @notapi
 */
void sd_lld_start(SerialDriver *sdp, const SerialConfig *config) {

  if (config == NULL)
    config = &default_config;

  if (sdp->state == SD_STOP) {
    /* Enables the peripheral.*/
#if HS_SERIAL_USE_UART0
    if (&SD0 == sdp) {
      cpmEnableUART0();
      cpmResetUART0();
      SD0.clock = cpm_get_clock(HS_UART0_CLK);
      nvicEnableVector(IRQ_UART0,
                       ANDES_PRIORITY_MASK(HS_SERIAL_UART0_IRQ_PRIORITY));
      /* locked in the upper layer, in serial.c */
      chSysUnlock();
      sdp->ib = chHeapAlloc(NULL, 64);
      sdp->ob = chHeapAlloc(NULL, 64);
      chIQObjectInit(&sdp->iqueue, sdp->ib, 64, NULL, sdp);
      chOQObjectInit(&sdp->oqueue, sdp->ob, 64, notify0, sdp);
      chSysLock();
    }
#endif
#if HS_SERIAL_USE_UART1
    if (&SD1 == sdp) {
      cpmEnableUART1();
      cpmResetUART1();
      SD1.clock = cpm_get_clock(HS_UART1_CLK);
      nvicEnableVector(IRQ_UART1,
                       ANDES_PRIORITY_MASK(HS_SERIAL_UART1_IRQ_PRIORITY));
      /* locked in the upper layer, in serial.c */
      chSysUnlock();
      sdp->ib = chHeapAlloc(NULL, SERIAL_BUFFERS_SIZE);
      sdp->ob = chHeapAlloc(NULL, SERIAL_BUFFERS_SIZE);
      chIQObjectInit(&sdp->iqueue, sdp->ib, SERIAL_BUFFERS_SIZE, NULL, sdp);
      chOQObjectInit(&sdp->oqueue, sdp->ob, SERIAL_BUFFERS_SIZE, notify1, sdp);      
      
      chSysLock();

      chIQResetI(&sdp->iqueue);
      chOQResetI(&sdp->oqueue);
      //sdp->dmatx = NULL;
      
#if HS_DMA_REQUIRED && HAL_SERIAL_USE_DMA
      sdp->dmatx = dmaStreamAllocate(HS_SERIAL_UART1_DMA_PRIORITY,
              (hs_dmaisr_t)serial_dma_tx_complete,
              (void *)sdp);
      if (sdp->dmatx) {
        hs_dma_config_t cfg;
        cfg.slave_id = UART1_TX_DMA_ID;
        cfg.direction = DMA_MEM_TO_DEV;
        cfg.src_addr_width = DMA_SLAVE_BUSWIDTH_8BITS;
        cfg.dst_addr_width = DMA_SLAVE_BUSWIDTH_8BITS;
        cfg.src_burst = DMA_BURST_LEN_1UNITS;
        cfg.dst_burst = DMA_BURST_LEN_1UNITS;
        cfg.dev_flow_ctrl = FALSE;	
        cfg.lli_en = 0;
        dmaStreamSetMode(sdp->dmatx, &cfg);
      }
#endif
    }
#endif
  }
  /* Configures the peripheral.*/
  uart_init(sdp, config);
}

/**
 * @brief   Low level serial driver stop.
 * @details De-initializes the UART, stops the associated clock, resets the
 *          interrupt vector.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 *
 * @notapi
 */
void sd_lld_stop(SerialDriver *sdp) {

  if (sdp->state == SD_READY) {
    /* Resets the peripheral.*/
    uart_deinit(sdp->uart);

    chIQResetI(&sdp->iqueue);
    chOQResetI(&sdp->oqueue);

    /* locked in the upper layer, in serial.c */
    chSysUnlock();
    chHeapFree(sdp->ib);
    chHeapFree(sdp->ob);
    chSysLock();

    /* Disables the peripheral.*/
#if HS_SERIAL_USE_UART0
    if (&SD0 == sdp) {
      nvicDisableVector(IRQ_UART0);
      cpmDisableUART0();
      return;
    }
#endif
#if HS_SERIAL_USE_UART1
    if (&SD1 == sdp) {
#if HS_DMA_REQUIRED && HAL_SERIAL_USE_DMA
      if (sdp->dmatx) {
        dmaStreamRelease(sdp->dmatx);
        sdp->dmatx = NULL;
      }
#endif
      nvicDisableVector(IRQ_UART1);
      cpmDisableUART1();
      return;
    }
#endif
  }
}

#endif /* HAL_USE_SERIAL */

/** @} */
