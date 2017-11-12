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
 * @file    hs66xx/spi_lld.c
 * @brief   SPI Driver subsystem low level driver source template.
 *
 * @addtogroup SPI
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#if HAL_USE_SPI || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/
/**
 * @brief   SPI1 driver identifier.
 */
#if HS_SPI_USE_SPI0 || defined(__DOXYGEN__)
SPIDriver SPID0;
#endif

/**
 * @brief   SPI1 driver identifier.
 */
#if HS_SPI_USE_SPI1 || defined(__DOXYGEN__)
SPIDriver SPID1;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
/**
 * @brief   RX DMA common service routine.
 *
 * @param[in] spip     pointer to the @p SPIDriver object
 * @param[in] error     error status
 */
static void spi_lld_serve_rx_end_irq(SPIDriver *spip, hs_dma_cb_para_t *var) {
  /* DMA errors handling.*/
#if defined(HS_SPI_DMA_ERROR_HOOK)
  HS_SPI_DMA_ERROR_HOOK(var);
#else
  (void)var;
#endif

	dmaStreamDisable(spip->dmarx);
	spip->rxstate = SPI_RX_COMPLETE;
  if(spip->rxstate != SPI_RX_ACTIVE && spip->txstate != SPI_TX_ACTIVE){
    osalSysUnlockFromISR();
    _spi_isr_code(spip);
    osalSysLockFromISR();
  }
}

/**
 * @brief   TX DMA common service routine.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 * @param[in] error     error status
 */
static void spi_lld_serve_tx_end_irq(SPIDriver *spip, hs_dma_cb_para_t *var) {
    /* DMA errors handling.*/
#if defined(HS_SPI_DMA_ERROR_HOOK)
    HS_SPI_DMA_ERROR_HOOK(var);
#else
    (void)var;
#endif

  dmaStreamDisable(spip->dmatx);
	spip->txstate = SPI_TX_COMPLETE;

  if(spip->rxstate != SPI_RX_ACTIVE && spip->txstate != SPI_TX_ACTIVE){
    osalSysUnlockFromISR();
    _spi_isr_code(spip);
    osalSysLockFromISR();
  }
}

/**
 * @brief   spi common service routine.
 *
 * @param[in] spi     pointer to the @p SPIDriver object
 */
static void serve_spi_irq(SPIDriver *spip) {
  HS_SPI_Type *s = spip->spi;
  uint8_t i = 0;
  uint8_t rxbytes = (s->STAT>>8)&0xff;
  spislavecallback_par_t *paras = spip->config->paras;  
  if (spip->config->role == SPI_SLAVE_MODE) {
          
    if (s->STAT & SPI0_RX_TRIGGER_STATUS_MASK) {
      s->STAT &= ~SPI0_RX_TRIGGER_STATUS_MASK;

      while(rxbytes--) {
      if (paras->dev_status == PRO_INT_STATUS) {        
        if (!paras->cmd_num) {
           paras->op_dir = s->RDATA;
           
           if ((paras->op_dir == SPI_SLAVE_READ_DIR) || (paras->op_dir == SPI_SLAVE_WRITE_DIR)) {
             paras->dev_status = PRO_CMD_STATUS;
             paras->cmd_num++;
           }
           
           continue;
        }
      }
      
      if (paras->dev_status == PRO_CMD_STATUS) {
        if (paras->cmd_num == 1) {
          paras->cmd = s->RDATA;
          spip->config->slave_cb(spip, paras);
          paras->cmd_num++;
          continue;
         }

         i = s->RDATA;
         if (++paras->cmd_num == SPI_SLAVE_PROTOCOL_CMD_NUM) {
           if (paras->op_dir == SPI_SLAVE_READ_DIR)
             paras->dev_status = PRO_READ_STATUS;      
           else if (paras->op_dir == SPI_SLAVE_WRITE_DIR)
             paras->dev_status = PRO_WRITE_STATUS;
           continue;
         }
       }

      if (paras->dev_status == PRO_WRITE_STATUS) {
        if (paras->data_num < SPI_SLAVE_PROTOCOL_DATA_NUM) {
           *(paras->data++) = s->RDATA;
           if(++paras->data_num == SPI_SLAVE_PROTOCOL_DATA_NUM) {
             paras->data -=  SPI_SLAVE_PROTOCOL_DATA_NUM;
             paras->dev_status = PRO_WRITE_CMP_STATUS;
             spip->config->slave_cb(spip, paras);
             break;
           }
        } 
      }
      }
    }

    if (s->STAT & SPI0_TX_EMPTY_STATUS_MASK) {
      s->STAT &= ~SPI0_TX_EMPTY_STATUS_MASK;
      
      if (paras->dev_status == PRO_READ_STATUS) {
    	s->CTR &= ~SPI0_RX_FIFO_ENABLE;
    	s->CTR |=  SPI0_TX_FIFO_ENABLE;

    	for (i=0; i<SPI_SLAVE_PROTOCOL_DATA_NUM; i++) {
          s->WDATA = *paras->data++;
          paras->dev_status = PRO_READ_CMP_STATUS;
    	}
#if 0
        if (++paras->data_num == SPI_SLAVE_PROTOCOL_DATA_NUM) {
          paras->dev_status = PRO_READ_CMP_STATUS;
          paras->data -=  SPI_SLAVE_PROTOCOL_DATA_NUM;
          spip->config->slave_cb(spip, paras);
          s->CTR &= ~SPI0_TX_FIFO_ENABLE;
          s->CTR |=  SPI0_RX_FIFO_ENABLE;
        }
#endif
      }
    }      

  }
  
  return;
}

/**
 * @brief   SPI initialization.
 * @details This function must be invoked with interrupts disabled.
 *
 * @param[in] spip     pointer to the @p SPIDriver object
 */

static void spi_start(SPIDriver *spip)
{
    HS_SPI_Type *s = spip->spi;
	const SPIConfig *config = spip->config;
    uint32_t clk_div;
	uint32_t ctrl = 0;

	clk_div = (spip->clock >> 1) / config->speed - 1;
	if (clk_div < 3)
		clk_div = 3;
    ctrl = s->CTR;
	ctrl = ctrl | SPI0_RESET | SPI0_TX_FIFO_RESET | SPI0_RX_FIFO_RESET;

	/* reset spi hardware, tx fifo and rx fifo */
	s->CTR = ctrl;

	/* clear rx fifo and tx fifo reset*/
	ctrl = ctrl & (~SPI0_TX_FIFO_RESET & ~SPI0_RX_FIFO_RESET);
	s->CTR = ctrl;

	ctrl = (ctrl & 0xffff0000) | clk_div;
	s->CTR = ctrl;

	ctrl |= SPI0_MSB;
    
    if (spip->config->role == SPI_MASTER_MODE)
      ctrl |= SPI0_MASTER;
    else
      ctrl &= ~SPI0_MASTER; //slave mode
    
	s->CTR = ctrl;

	if (spip->config->role == SPI_MASTER_MODE)
	  ctrl |= SPI0_TX_FIFO_ENABLE | SPI0_RX_FIFO_ENABLE;
	else {
	  ctrl |= SPI0_RX_FIFO_ENABLE;
	  ctrl &= ~SPI0_TX_FIFO_ENABLE;
	}
	s->CTR = ctrl;

	ctrl = (ctrl & 0xFFDDFFFF) | ((config->mode & 0x01) << 17) | (((config->mode >> 1) & 0x01) << 21);
	s->CTR = ctrl;
    
  if (spip->config->role == SPI_MASTER_MODE) {
    //dma transfer
    s->DMACR = 0x03;
    s->DMATDLR = (32 >> 1);  //tx data level = fifo_depth / 2
    s->DMARDLR = 0;          //rx data level
  } else if (spip->config->role == SPI_SLAVE_MODE) {/* slave interrupt */
    s->CTR &= SPI0_SET_TRIGGER_MASK;
    s->CTR |= SPI0_SET_TRIGGER_LEVEL(0); /* rx 1 byte trigger */
    //s->CTR |= (1ul<<22);
    s->STAT |= SPI0_SET_TX_EMPTY_INT | SPI0_SET_RX_TRIGGER_INT;
    s->STAT &= ~SPI0_INT_EN_MASK;
    s->CTR &= ~SPI0_RESET;
    
    if (&SPID0 == spip)
      nvicEnableVector(IRQ_SPIM0, ANDES_PRIORITY_MASK(HS_SPI_SPI0_IRQ_PRIORITY));
    else
      nvicEnableVector(IRQ_SPIM1, ANDES_PRIORITY_MASK(HS_SPI_SPI1_IRQ_PRIORITY));
  }
}

/**
 * @brief   SPI de-initialization.
 * @details This function must be invoked with interrupts disabled.
 *
 * @param[in] spip     pointer to the @p SPIDriver object
 */
static void spi_stop(SPIDriver *spip) {
  HS_SPI_Type *s = spip->spi;

  s->CTR = s->CTR | SPI0_RESET;
  
  if (spip->config->role == SPI_SLAVE_MODE) {
    if (&SPID0 == spip)
      nvicDisableVector(IRQ_SPIM0);
    else
      nvicDisableVector(IRQ_SPIM1);
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if HS_SPI_USE_SPI0
/**
 * @brief   SPI0 IRQ handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPI0_IRQHandler) {

  CH_IRQ_PROLOGUE();

  serve_spi_irq(&SPID0);

  CH_IRQ_EPILOGUE();
}
#endif /* HS_SPI_USE_SPI1 */

#if HS_SPI_USE_SPI1
/**
 * @brief   SPI1 IRQ handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(SPI1_IRQHandler) {

  CH_IRQ_PROLOGUE();

  serve_spi_irq(&SPID1);

  CH_IRQ_EPILOGUE();
}
#endif /* HS_SPI_USE_SPI1 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level SPI driver initialization.
 *
 * @notapi
 */
void spi_lld_init(void) {
	/* Driver initialization.*/
#if HS_SPI_USE_SPI0
  spiObjectInit(&SPID0);
	SPID0.spi = HS_SPI0;
	SPID0.dmarx = NULL;
	SPID0.dmatx = NULL;
#endif /* HS_SPI_USE_SPI0 */

	/* Driver initialization.*/
#if HS_SPI_USE_SPI1
  spiObjectInit(&SPID1);
	SPID1.spi = HS_SPI1;
	SPID1.dmarx = NULL;
	SPID1.dmatx = NULL;
#endif /* HS_SPI_USE_SPI1 */
}

/**
* @brief   Configures and activates the SPI peripheral.
*
* @param[in] spip      pointer to the @p SPIDriver object
*
* @notapi
*/
void spi_lld_start(SPIDriver *spip) {
	hs_dma_config_t sconfig;

  if (spip->state == SPI_STOP) {
		/* Enables the peripheral.*/
#if HS_SPI_USE_SPI0
    if (&SPID0 == spip) {
      if (spip->config->role == SPI_MASTER_MODE) {
	    spip->dmarx = dmaStreamAllocate(HS_SPI_SPI0_DMA_PRIORITY,
                                      (hs_dmaisr_t)spi_lld_serve_rx_end_irq,
                                      (void *)spip);
        chDbgAssert(spip->dmarx != NULL, "stream already allocated");
        spip->dmatx = dmaStreamAllocate(HS_SPI_SPI0_DMA_PRIORITY,
                                      (hs_dmaisr_t)spi_lld_serve_tx_end_irq,
                                      (void *)spip);
        chDbgAssert(spip->dmatx != NULL, "stream already allocated");
      }
      
      cpmEnableSPI0();
      SPID0.clock = cpm_get_clock(HS_SPI0_CLK);
      /*
      nvicEnableVector(IRQ_SPIM0, CORTEX_PRIORITY_MASK(HS_SPI_SPI0_IRQ_PRIORITY));
      */
    }
#endif /* HS_SPI_USE_SPI0 */

    /* Enables the peripheral.*/
#if HS_SPI_USE_SPI1
    if (&SPID1 == spip) {
      if (spip->config->role == SPI_MASTER_MODE) {      
	    spip->dmarx = dmaStreamAllocate(HS_SPI_SPI1_DMA_PRIORITY,
                                      (hs_dmaisr_t)spi_lld_serve_rx_end_irq,
                                      (void *)spip);
        chDbgAssert(spip->dmarx != NULL, "stream already allocated");
        spip->dmatx = dmaStreamAllocate(HS_SPI_SPI1_DMA_PRIORITY,
                                      (hs_dmaisr_t)spi_lld_serve_tx_end_irq,
                                      (void *)spip);
        chDbgAssert(spip->dmatx != NULL, "stream already allocated");
      }
      cpmEnableSPI1();
      SPID1.clock = cpm_get_clock(HS_SPI1_CLK);
      /*
      nvicEnableVector(IRQ_SPIM1, CORTEX_PRIORITY_MASK(HS_SPI_SPI1_IRQ_PRIORITY));
      */
    }
#endif /* HS_SPI_USE_SPI1 */
    
  if (spip->config->role == SPI_MASTER_MODE) {
    /* Static DMA setup */
    sconfig.direction = DMA_DEV_TO_MEM;
    sconfig.src_addr_width = DMA_SLAVE_BUSWIDTH_8BITS;
    sconfig.dst_addr_width = DMA_SLAVE_BUSWIDTH_8BITS;
    sconfig.src_burst = DMA_BURST_LEN_1UNITS;
    sconfig.dst_burst = DMA_BURST_LEN_1UNITS;
    sconfig.dev_flow_ctrl = FALSE;
#if HS_SPI_USE_SPI0
    if (&SPID0 == spip){
    	sconfig.slave_id = SPI0_RX_DMA_ID;
		}
#endif
#if HS_SPI_USE_SPI1
    if (&SPID1 == spip) {
			sconfig.slave_id = SPI1_RX_DMA_ID;
		}
#endif
    sconfig.lli_en = 0;
    dmaStreamSetMode(spip->dmarx, &sconfig);

    sconfig.direction = DMA_MEM_TO_DEV;
    sconfig.src_addr_width = DMA_SLAVE_BUSWIDTH_8BITS;
    sconfig.dst_addr_width = DMA_SLAVE_BUSWIDTH_8BITS;
    sconfig.src_burst = DMA_BURST_LEN_1UNITS;
    sconfig.dst_burst = DMA_BURST_LEN_1UNITS;
    sconfig.dev_flow_ctrl = FALSE;
#if HS_SPI_USE_SPI0
    if (&SPID0 == spip){
    	sconfig.slave_id = SPI0_TX_DMA_ID;
		}
#endif
#if HS_SPI_USE_SPI1
    if (&SPID1 == spip) {
			sconfig.slave_id = SPI1_TX_DMA_ID;
		}
#endif
    sconfig.lli_en = 0;
    dmaStreamSetMode(spip->dmatx, &sconfig);
  }
  }
  
  /* Configures the peripheral.*/
  spip->rxstate = SPI_RX_IDLE;
  spip->txstate = SPI_TX_IDLE;
  spi_start(spip);
}

/**
* @brief   Deactivates the SPI peripheral.
*
* @param[in] spip      pointer to the @p SPIDriver object
*
* @notapi
*/
void spi_lld_stop(SPIDriver *spip) {

  if (spip->state == SPI_READY) {
	spi_stop(spip);
    if (spip->config->role == SPI_MASTER_MODE) {
      /* Resets the peripheral.*/
	  dmaStreamRelease(spip->dmarx);
      dmaStreamRelease(spip->dmatx);
    }
    
    /* Disables the peripheral.*/
#if HS_SPI_USE_SPI0
    if (&SPID0 == spip) {
      //nvicDisableVector(SPI0_IRQn);
     cpmDisableSPI0();
    }
#endif /* HS_SPI_USE_SPI1 */
#if HS_SPI_USE_SPI1
    if (&SPID1 == spip) {
      //nvicDisableVector(SPI1_IRQn);
      cpmDisableSPI1();
    }
#endif /* HS_SPI_USE_SPI1 */
  }
}

static void set_gpio_out(uint8_t index, uint8_t val)
{
  HS_GPIO_Type *pGpio;
  uint16_t tmp;

  if(index < 16){
    tmp = index;
    pGpio = IOPORT0;
  }
  else{
    tmp = index - 16;
    pGpio = IOPORT1;
  }

  if((val & 1) == 0) {
    palClearPad(pGpio, tmp);
  }
  else {
    palSetPad(pGpio, tmp);
  }
}


/**
* @brief   Asserts the slave select signal and prepares for transfers.
*
* @param[in] spip      pointer to the @p SPIDriver object
*
* @notapi
*/
void spi_lld_select(SPIDriver *spip) {
  const SPIConfig *config = spip->config;

#if HS_SPI_USE_SPI0
  if (&SPID0 == spip) {
    if(config->cs_gpio_index < 32){
      set_gpio_out(config->cs_gpio_index, 0);
    }
  }
#endif /* HS_SPI_USE_SPI1 */
#if HS_SPI_USE_SPI1
  if (&SPID1 == spip) {
    if(config->cs_gpio_index < 32){
      set_gpio_out(config->cs_gpio_index, 0);
    }
  }
#endif /* HS_SPI_USE_SPI1 */
}

/**
* @brief   Deasserts the slave select signal.
* @details The previously selected peripheral is unselected.
*
* @param[in] spip      pointer to the @p SPIDriver object
*
* @notapi
*/
void spi_lld_unselect(SPIDriver *spip) {
  const SPIConfig *config = spip->config;

#if HS_SPI_USE_SPI0
  if (&SPID0 == spip) {
    if(config->cs_gpio_index < 32){
      set_gpio_out(config->cs_gpio_index, 1);
    }
  }
#endif /* HS_SPI_USE_SPI1 */
#if HS_SPI_USE_SPI1
  if (&SPID1 == spip) {
    if(config->cs_gpio_index < 32){
      set_gpio_out(config->cs_gpio_index, 1);
    }
  }
#endif /* HS_SPI_USE_SPI1 */
}

/**
* @brief   Ignores data on the SPI bus.
* @details This asynchronous function starts the transmission of a series of
*          idle words on the SPI bus and ignores the received data.
* @post    At the end of the operation the configured callback is invoked.
*
* @param[in] spip      pointer to the @p SPIDriver object
* @param[in] n         number of words to be ignored
*
* @notapi
*/
void spi_lld_ignore(SPIDriver *spip, size_t n) {
  HS_SPI_Type *s = spip->spi;

  s->CTR &= ~SPI0_RESET;
  s->CTR |= SPI0_TX_FIFO_ENABLE;
  dmaStreamStart(spip->dmatx, spip, &s->WDATA, n);
}

int spi_xfer(SPIDriver *spip, size_t n, const void *txbuf, void *rxbuf)
{
	HS_SPI_Type *s = spip->spi;

  s->CTR &= ~SPI0_RESET;
  s->CTR |= (SPI0_RX_FIFO_ENABLE | SPI0_TX_FIFO_ENABLE);

  dmaStreamStart(spip->dmarx, &s->RDATA, rxbuf, n);
  dmaStreamStart(spip->dmatx, txbuf, &s->WDATA, n);

	return 0;
}

/**
* @brief   Exchanges data on the SPI bus.
* @details This asynchronous function starts a simultaneous transmit/receive
*          operation.
* @post    At the end of the operation the configured callback is invoked.
* @note    The buffers are organized as uint8_t arrays for data sizes below or
*          equal to 8 bits else it is organized as uint16_t arrays.
*
* @param[in] spip      pointer to the @p SPIDriver object
* @param[in] n         number of words to be exchanged
* @param[in] txbuf     the pointer to the transmit buffer
* @param[out] rxbuf    the pointer to the receive buffer
*
* @notapi
*/
void spi_lld_exchange(SPIDriver *spip, size_t n,
                      const void *txbuf, void *rxbuf) {
  spip->rxstate = SPI_RX_ACTIVE;
  spip->txstate = SPI_TX_ACTIVE;

  spi_xfer(spip, n, txbuf, rxbuf);
}

/**
* @brief   Sends data over the SPI bus.
* @details This asynchronous function starts a transmit operation.
* @post    At the end of the operation the configured callback is invoked.
* @note    The buffers are organized as uint8_t arrays for data sizes below or
*          equal to 8 bits else it is organized as uint16_t arrays.
*
* @param[in] spip      pointer to the @p SPIDriver object
* @param[in] n         number of words to send
* @param[in] txbuf     the pointer to the transmit buffer
*
* @notapi
*/
void spi_lld_send(SPIDriver *spip, size_t n, const void *txbuf) {
  HS_SPI_Type *s = spip->spi;

  s->CTR &= ~SPI0_RESET;
  s->CTR |= SPI0_TX_FIFO_ENABLE;

  spip->txstate = SPI_TX_ACTIVE;
  dmaStreamStart(spip->dmatx, txbuf, &s->WDATA, n);
}

/**
* @brief   Receives data from the SPI bus.
* @details This asynchronous function starts a receive operation.
* @post    At the end of the operation the configured callback is invoked.
* @note    The buffers are organized as uint8_t arrays for data sizes below or
*          equal to 8 bits else it is organized as uint16_t arrays.
*
* @param[in] spip      pointer to the @p SPIDriver object
* @param[in] n         number of words to receive
* @param[out] rxbuf    the pointer to the receive buffer
*
* @notapi
*/
void spi_lld_receive(SPIDriver *spip, size_t n, void *rxbuf) {
  HS_SPI_Type *s = spip->spi;

  s->CTR &= ~SPI0_RESET;
  s->CTR |= SPI0_RX_FIFO_ENABLE;

  spip->rxstate = SPI_RX_ACTIVE;
  spip->txstate = SPI_TX_ACTIVE;
  dmaStreamStart(spip->dmarx, &s->RDATA, rxbuf, n);
  dmaStreamStart(spip->dmatx, rxbuf, &s->WDATA, n);
}

/**
* @brief   Exchanges one frame using a polled wait.
* @details This synchronous function exchanges one frame using a polled
*          synchronization method. This function is useful when exchanging
*          small amount of data on high speed channels, usually in this
*          situation is much more efficient just wait for completion using
*          polling than suspending the thread waiting for an interrupt.
*
* @param[in] spip      pointer to the @p SPIDriver object
* @param[in] frame     the data frame to send over the SPI bus
* @return              The received data frame from the SPI bus.
*/
uint16_t spi_lld_polled_exchange(SPIDriver *spip, uint16_t frame) {
  HS_SPI_Type *s = spip->spi;
  unsigned char buf[2] = {frame & 0xff, frame >> 8};

  s->CTR &= ~SPI0_RESET;
  s->CTR |= (SPI0_RX_FIFO_ENABLE | SPI0_TX_FIFO_ENABLE);
  s->WDATA = buf[0];
  buf[0] = s->RDATA;
  s->WDATA = buf[1];
  buf[1] = s->RDATA;

  return (buf[1] << 8) + buf[0];
}

#endif /* HAL_USE_SPI */

/** @} */
