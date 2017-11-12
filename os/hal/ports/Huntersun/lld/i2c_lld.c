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
 * @file    hs66xx/i2c_lld.c
 * @brief   I2C Driver subsystem low level driver source.
 *
 * @addtogroup I2C
 * @{
 */
#include <string.h>
#include "ch.h"
#include "hal.h"

#if HAL_USE_I2C || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/
#define I2C_INTR_DEFAULT_MASK		(I2C_INTR_RX_FULL | \
					 I2C_INTR_TX_EMPTY | \
					 I2C_INTR_TX_ABRT | \
					 I2C_INTR_STOP_DET)

#define I2C_INTR_SLAVE_MASK             (I2C_INTR_RD_REQ | \
                                         I2C_INTR_TX_ABRT | \
                                         I2C_INTR_RX_FULL | \
                                         I2C_INTR_STOP_DET)

#define I2C_ENBALE_CLOCK(i2cp)       \
  if (&I2CD0 == i2cp) {\
    cpmEnableI2C0();\
  }

#define I2C_DISBALE_CLOCK(i2cp)       \
  if (&I2CD0 == i2cp) {\
    cpmDisableI2C0();\
  }

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   I2C0 driver identifier.
 */
#if HS_I2C_USE_I2C0 || defined(__DOXYGEN__)
I2CDriver I2CD0;
#endif

#define EEPROM_PAGE_SIZE 32

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
#if ((I2C_USE_DMA == TRUE) && (HS_DMA_REQUIRED == TRUE))

/**
 * @brief  i2c DMA complete callback function
 */
static void i2c_dma_complete(I2CDriver *i2cp, hs_dma_cb_para_t *var)
{
  (void)i2cp;
  if (var->oper_res != DMA_TRANS_OPER_OK) {
    return;
  }
}


static void i2c_rx_dma_config(I2CDriver *i2cp) {
  HS_I2C_Type *dp = i2cp->i2c;
  hs_dma_config_t cfg;
  i2cp->dma_rx = dmaStreamAllocate(HS_I2C_I2C0_DMA_PRIORITY,
				     (hs_dmaisr_t)i2c_dma_complete,
				     (void *)i2cp);
  if (i2cp->dma_rx) {
    /* config DMA */
    cfg.slave_id = I2C_RX_DMA_ID;
    cfg.direction = DMA_DEV_TO_MEM;    
    
    cfg.src_addr_width = DMA_SLAVE_BUSWIDTH_8BITS;
    cfg.dst_addr_width = DMA_SLAVE_BUSWIDTH_8BITS;
    cfg.src_burst = DMA_BURST_LEN_1UNITS;
    cfg.dst_burst = DMA_BURST_LEN_1UNITS;
    cfg.dev_flow_ctrl = FALSE;	
    cfg.lli_en = 0;
    
    dmaStreamSetMode(i2cp->dma_rx, &cfg);
    
    dp->DMA_RDLR = 0;
  } 
}

static void i2c_tx_dma_config(I2CDriver *i2cp) {
  HS_I2C_Type *dp = i2cp->i2c;
   hs_dma_config_t cfg;

   i2cp->dma_tx = dmaStreamAllocate(HS_I2C_I2C0_DMA_PRIORITY,
				   (hs_dmaisr_t)i2c_dma_complete,
				   (void *)i2cp);
   if (i2cp->dma_tx) {
     cfg.slave_id = I2C_TX_DMA_ID;
     cfg.direction = DMA_MEM_TO_DEV;  	    
     
     cfg.src_addr_width = DMA_SLAVE_BUSWIDTH_8BITS;
     cfg.dst_addr_width = DMA_SLAVE_BUSWIDTH_8BITS;
     cfg.src_burst = DMA_BURST_LEN_1UNITS;
     cfg.dst_burst = DMA_BURST_LEN_1UNITS;
     cfg.dev_flow_ctrl = FALSE;	
     cfg.lli_en = 0;
     
     dmaStreamSetMode(i2cp->dma_tx, &cfg);
     
     /* set dma i2c fifo trigger level */
     dp->DMA_TDLR = i2cp->tx_fifo_depth - 1;  
   }
} 

#endif

static void i2c_address_to_buffer(uint8_t *buf, uint32_t offset, uint8_t alen)
{
  /* 8/16/24-bit address is written MSB first */
  switch(alen) {
  default: /* case 3 */
    *buf++ = offset >> 16;
  case 2:
    *buf++ = offset >> 8;
  case 1:
  case 0: /* can't happen: for better codegen */
    *buf++ = offset >> 0;
  }
}

static uint32_t i2c_lld_scl_hcnt(uint32_t ic_clk, uint32_t tSYMBOL, uint32_t tf, int cond, int offset)
{
	/*
   * DesignWare I2C core doesn't seem to have solid strategy to meet
   * the tHD;STA timing spec.  Configuring _HCNT based on tHIGH spec
   * will result in violation of the tHD;STA spec.
   */
	if (cond)
    /*
		 * Conditional expression:
		 *
		 *   IC_[FS]S_SCL_HCNT + (1+4+3) >= IC_CLK * tHIGH
		 *
		 * This is based on the manuals, and represents an ideal
		 * configuration.  The resulting I2C bus speed will be
		 * faster than any of the others.
		 *
		 * If your hardware is free from tHD;STA issue, try this one.
		 */
		return (ic_clk * tSYMBOL + 5000) / 10000 - 8 + offset;
	else
		/*
		 * Conditional expression:
		 *
		 *   IC_[FS]S_SCL_HCNT + 3 >= IC_CLK * (tHD;STA + tf)
		 *
		 * This is just experimental rule; the tHD;STA period turned
		 * out to be proportinal to (_HCNT + 3).  With this setting,
		 * we could meet both tHIGH and tHD;STA timing specs.
		 *
		 * If unsure, you'd better to take this alternative.
		 *
		 * The reason why we need to take into account "tf" here,
		 * is the same as described in i2c_lld_scl_lcnt().
		 */
		return (ic_clk * (tSYMBOL + tf) + 5000) / 10000 - 3 + offset;
}

static uint32_t i2c_lld_scl_lcnt(uint32_t ic_clk, uint32_t tLOW, uint32_t tf, int offset)
{
	/*
	 * Conditional expression:
	 *
	 *   IC_[FS]S_SCL_LCNT + 1 >= IC_CLK * (tLOW + tf)
	 *
	 * DW I2C core starts counting the SCL CNTs for the LOW period
	 * of the SCL clock (tLOW) as soon as it pulls the SCL line.
	 * In order to meet the tLOW timing spec, we need to take into
	 * account the fall time of SCL signal (tf).  Default tf value
	 * should be 0.3 us, for safety.
	 */
	return ((ic_clk * (tLOW + tf) + 5000) / 10000) - 1 + offset;
}

/**
 * @brief   Aborts an I2C transaction.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
static void i2c_lld_abort_operation(I2CDriver *i2cp) {
  HS_I2C_Type *dp = i2cp->i2c;

#if ((I2C_USE_DMA == TRUE) && (HS_DMA_REQUIRED == TRUE))
  dp->DMA_CR = 0;
  if (i2cp->dma_tx)
    dmaStreamRelease(i2cp->dma_tx);
  if (i2cp->dma_rx)
    dmaStreamRelease(i2cp->dma_rx);
#endif
  /* Stops the I2C peripheral.*/
  dp->INTR_MASK = 0;
  dp->ENABLE = 0;
}



/**
 * @brief   Set clock speed.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
static void i2c_lld_set_clock(I2CDriver *i2cp) {
  HS_I2C_Type *dp = i2cp->i2c;
  int32_t clock_speed = i2cp->config->clock_speed;
  //i2cdutycycle_t duty = i2cp->config->duty_cycle;
  uint32_t con = dp->CON;
  uint32_t clk = cpm_get_clock(HS_I2C0_CLK);

  osalDbgCheck((i2cp != NULL) && (clock_speed > 0) && (clock_speed <= 4000000));

  /* set standard and fast speed deviders for high/low periods */

  /* Standard-mode @100k */
  dp->SS_SCL_HCNT = i2c_lld_scl_hcnt(clk/1000,
				     40,/* tHD;STA = tHIGH = 4.0 us */
				     3,	/* tf = 0.3 us */
				     0,	/* 0: default, 1: Ideal */
				     0);/* No offset */
  dp->SS_SCL_LCNT = i2c_lld_scl_lcnt(clk/1000,
				     47,/* tLOW = 4.7 us */
				     3,	/* tf = 0.3 us */
				     0);/* No offset */

  /* Fast-mode @400k */
  dp->FS_SCL_HCNT = i2c_lld_scl_hcnt(clk/1000,
				     6,	/* tHD;STA = tHIGH = 0.6 us */
				     3,	/* tf = 0.3 us */
				     0,	/* 0: default, 1: Ideal */
				     0);/* No offset */
  dp->FS_SCL_LCNT = i2c_lld_scl_lcnt(clk/1000,
				     13,/* tLOW = 1.3 us */
				     3,	/* tf = 0.3 us */
				     0);/* No offset */

  if (clock_speed <= 100000) {
#if 0
    /* Configure clock_div in standard mode.*/
    osalDbgAssert(duty == STD_DUTY_CYCLE,
                "i2c_lld_set_clock(), #1",
                "Invalid standard mode duty cycle");
#endif

    /* Standard mode clock_div calculate: Tlow/Thigh = 1/1.*/
    /* Sets the Maximum Rise Time for standard mode.*/
    dp->CON &= (con & ~0x6) | I2C_CON_SPEED_STD;
  }
  else if (clock_speed <= 400000) {
#if 0
    /* Configure clock_div in fast mode.*/
    osalDbgAssert((duty == FAST_DUTY_CYCLE_2) || (duty == FAST_DUTY_CYCLE_16_9),
                "i2c_lld_set_clock(), #4",
                "Invalid fast mode duty cycle");

    if (duty == FAST_DUTY_CYCLE_2) {
      /* Fast mode clock_div calculate: Tlow/Thigh = 2/1.*/
    }
    else if (duty == FAST_DUTY_CYCLE_16_9) {
      /* Fast mode clock_div calculate: Tlow/Thigh = 16/9.*/
    }
#endif
    /* Sets the Maximum Rise Time for fast mode.*/
    dp->CON &= (con & ~0x6) | I2C_CON_SPEED_FAST;
  }
}

/**
 * @brief   Set operation mode of I2C hardware.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
static void i2c_lld_set_opmode(I2CDriver *i2cp) {
  HS_I2C_Type *dp = i2cp->i2c;
  i2copmode_t opmode = i2cp->config->op_mode;

  switch (opmode) {
  case OPMODE_I2C_MASTER:
    dp->CON = I2C_CON_MASTER | I2C_CON_SLAVE_DISABLE | I2C_CON_RESTART_EN;
    break;
  case OPMODE_I2C_SLAVE:
	  dp->CON = I2C_CON_RESTART_EN;
    break;
  case OPMODE_SMBUS_DEVICE:
    dp->CON = I2C_CON_10BITADDR_SLAVE | I2C_CON_RESTART_EN;
    break;
  case OPMODE_SMBUS_HOST:
    dp->CON = I2C_CON_MASTER | I2C_CON_SLAVE_DISABLE | I2C_CON_10BITADDR_MASTER | I2C_CON_RESTART_EN;
    break;
  }
}

/**
 * @brief   Common ISR code to clear interrupt cause
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
static uint32_t i2c_lld_read_clear_intrbits(I2CDriver *i2cp) {
  HS_I2C_Type *dp = i2cp->i2c;
  uint32_t dummy, stat = dp->INTR_STAT;

  /* Do not use the IC_CLR_INTR register to clear interrupts. */
  if (stat & I2C_INTR_RX_UNDER) {
    i2cp->errors |= I2CD_OVERRUN;
    dummy = dp->CLR_RX_UNDER;
  }
  if (stat & I2C_INTR_RX_OVER) {
    i2cp->errors |= I2CD_OVERRUN;
    dummy = dp->CLR_RX_OVER;
  }
  if (stat & I2C_INTR_TX_OVER) {
    i2cp->errors |= I2CD_OVERRUN;
    dummy = dp->CLR_TX_OVER;
  }
  if (stat & I2C_INTR_RD_REQ)
    dummy = dp->CLR_RD_REQ;
  if (stat & I2C_INTR_TX_ABRT) {
    /*
     * The IC_TX_ABRT_SOURCE register is cleared whenever
     * the IC_CLR_TX_ABRT is read.  Preserve it beforehand.
     */
    dummy = dp->TX_ABRT_SOURCE;
    if (dummy & I2C_TX_ARB_LOST)
      i2cp->errors |= I2CD_ARBITRATION_LOST;
    if (dummy & 0x1f/*I2C_TX_ABRT_xxx_NOACK*/)
      i2cp->errors |= I2CD_ACK_FAILURE;
    if (dummy & 0xfe0)
      i2cp->errors |= I2CD_BUS_ERROR; /* it is trigged by wrong sw behaviours */
    dummy = dp->CLR_TX_ABRT;
  }
  if (stat & I2C_INTR_RX_DONE)
    dummy = dp->CLR_RX_DONE;
  if (stat & I2C_INTR_ACTIVITY)
    dummy = dp->CLR_ACTIVITY;
  if (stat & I2C_INTR_STOP_DET)
    dummy = dp->CLR_STOP_DET;
  if (stat & I2C_INTR_START_DET)
    dummy = dp->CLR_START_DET;
  if (stat & I2C_INTR_GEN_CALL)
    dummy = dp->CLR_GEN_CALL;

  (void)dummy;
  return stat;
}

/**
 * @brief   Common ISR code to read received data in PIO
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
static void i2c_lld_read_pio(I2CDriver *i2cp) {
  HS_I2C_Type *dp = i2cp->i2c;
  struct i2c_msg *msgs = i2cp->msgs;
  uint32_t len, rx_valid;
  uint8_t *buf;
  uint32_t intr_mask;
  
  intr_mask = I2C_INTR_RX_FULL | I2C_INTR_STOP_DET | I2C_INTR_TX_ABRT;
  dp->INTR_MASK = intr_mask;
  
  for (; i2cp->msg_read_idx < NUM_I2C_MSG; i2cp->msg_read_idx++) {
    if (!(msgs[i2cp->msg_read_idx].flags & I2C_M_RD))
      continue;
    if (!(i2cp->status & STATUS_READ_IN_PROGRESS)) {
      len = msgs[i2cp->msg_read_idx].len;
      buf = msgs[i2cp->msg_read_idx].buf;
    } else {
      len = i2cp->rxbytes;
      buf = i2cp->rxbuf;
    }

    rx_valid = dp->RXFLR;
    for (; len > 0 && rx_valid > 0; len--, rx_valid--)
      *buf++ = dp->DATA_CMD;

    if (len > 0) {
      i2cp->status |= STATUS_READ_IN_PROGRESS;
      i2cp->rxbytes = len;
      i2cp->rxbuf = buf;
      return;
    } else {
      i2cp->status &= ~STATUS_READ_IN_PROGRESS;
    }
    
  }
}

/**
 * @brief   Common ISR code to write data or read trigger in PIO
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
static void i2c_lld_xfer_msg_pio(I2CDriver *i2cp) {
  HS_I2C_Type *dp = i2cp->i2c;
  struct i2c_msg *msgs = i2cp->msgs;
  uint32_t intr_mask, tx_limit, rx_limit;
  uint8_t *buf = i2cp->txbuf;
  uint32_t len = i2cp->txbytes;

  intr_mask = I2C_INTR_DEFAULT_MASK;

  for (; i2cp->msg_write_idx < NUM_I2C_MSG; i2cp->msg_write_idx++) {
    if ((msgs[i2cp->msg_write_idx].flags & I2C_M_RD) &&
        1/*(msgs[i2cp->msg_write_idx].len > 0)*/) {
      dp->CON1 = msgs[i2cp->msg_write_idx].len | I2C_CON1_RX_ENABLE | I2C_CON1_READBYTES_UPDATE;
      
#if ((I2C_USE_DMA == TRUE) && (HS_DMA_REQUIRED == TRUE)) 
    dp->DMA_CR = I2C_DMA_CR_RDMAE;
    dmaStreamStart(i2cp->dma_rx, &dp->DATA_CMD, msgs[i2cp->msg_write_idx].buf, msgs[i2cp->msg_write_idx].len);
    dp->INTR_MASK = I2C_INTR_STOP_DET;
#else
    intr_mask &= ~I2C_INTR_TX_EMPTY;
    intr_mask &= ~I2C_INTR_STOP_DET;
    /* Enable interrupts */
    dp->INTR_MASK = intr_mask;
#endif
    return;
    }

    if (0 == msgs[i2cp->msg_write_idx].len) {
      //i2cp->errors = I2CD_ERROR_NULLPTR;
      //break;
    }

    if (!(i2cp->status & STATUS_WRITE_IN_PROGRESS)) {
      /* new i2c_msg */
      buf = msgs[i2cp->msg_write_idx].buf;
      len = msgs[i2cp->msg_write_idx].len;
    }

    tx_limit = i2cp->tx_fifo_depth - dp->TXFLR;
    rx_limit = i2cp->rx_fifo_depth - dp->RXFLR;

    while (len > 0 && tx_limit > 0 && rx_limit > 0) {
      if (msgs[i2cp->msg_write_idx].flags & I2C_M_RD) {
        ;
      } else {
	    dp->DATA_CMD = *buf++;
      }
        tx_limit--; len--;
    }

    i2cp->txbuf = buf;
    i2cp->txbytes = len;

    if (len > 0) {
      /* more bytes to be written */
      i2cp->status |= STATUS_WRITE_IN_PROGRESS;
      break;
    } else
      i2cp->status &= ~STATUS_WRITE_IN_PROGRESS;
  }

  /*
   * If i2c_msg index search is completed, we don't need TX_EMPTY
   * interrupt any more.
   */
  if (i2cp->msg_write_idx == NUM_I2C_MSG) {
    intr_mask &= ~I2C_INTR_TX_EMPTY;
    if (i2cp->config->con_mode == CONMODE_I2C_AUTO_WR)
      i2cp->msg_write_idx = 0;
  }
  if (i2cp->errors)
    intr_mask = 0;
  dp->INTR_MASK = intr_mask;
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/
/**
 * @brief   Common IRQ handler.
 *
 * @param[in] i2cp       communication channel associated to the I2C
 */
static void i2c_serve_interrupt(I2CDriver *i2cp) {
  uint32_t stat;

  if ((0 == HS_I2C0->ENABLE) || (0 == (HS_I2C0->RAW_INTR_STAT & ~I2C_INTR_ACTIVITY)))
    return;

  stat = i2c_lld_read_clear_intrbits(i2cp);

  if (stat & I2C_INTR_RD_REQ) {
    i2c_lld_xfer_msg_pio(i2cp);
  }

  /*
   * Anytime TX_ABRT is set, the contents of the tx/rx
   * buffers are flushed.  Make sure to skip them.
   */
  if (stat & I2C_INTR_TX_ABRT) {
    i2c_lld_abort_operation(i2cp);
    _i2c_wakeup_isr(i2cp);
     return;
  }

  if (stat & I2C_INTR_RX_FULL) {
    i2c_lld_read_pio(i2cp);
  }

  if (stat & I2C_INTR_TX_EMPTY) {
    i2c_lld_xfer_msg_pio(i2cp);
  }

  if (stat & I2C_INTR_STOP_DET) {
    if (i2cp->config->con_mode == CONMODE_I2C_NO_AUTO_WR)
      _i2c_wakeup_isr(i2cp);
    else if (i2cp->config->con_mode == CONMODE_I2C_AUTO_WR) {
      //dp->INTR_MASK = intr_mask;
    	i2cp->i2c->CON1 |= I2C_CON1_CLEAR_I2C_ENABLE;
      _i2c_wakeup_isr(i2cp);
    }
  }
  
  return;  
}

#if HS_I2C_USE_I2C0 || defined(__DOXYGEN__)
/**
 * @brief   I2C0 interrupt handler.
 *
 * @notapi
 */

OSAL_IRQ_HANDLER(I2C0_IRQHandler)
{
  OSAL_IRQ_PROLOGUE();
  i2c_serve_interrupt(&I2CD0);
  OSAL_IRQ_EPILOGUE();  
}
#endif /* HS_I2C_USE_I2C0 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level I2C driver initialization.
 *
 * @notapi
 */
void i2c_lld_init(void) {

#if HS_I2C_USE_I2C0
  i2cObjectInit(&I2CD0);
  I2CD0.thread = NULL;
  I2CD0.i2c    = HS_I2C0;
  //chDbgAssert(HS_I2C0->COMP_PARAM_1 == 0x44570140, __FUNCTION__, "wrong IP configration");
  osalDbgAssert(HS_I2C0->COMP_TYPE == 0x44570140, "wrong IP type");
  I2CD0.tx_fifo_depth = ((HS_I2C0->COMP_PARAM_1 >> 16) & 0xff) + 1;
  I2CD0.rx_fifo_depth = ((HS_I2C0->COMP_PARAM_1 >> 8) & 0xff) + 1;

  HS_I2C0->INTR_MASK = 0;
#endif /* HS_I2C_USE_I2C0 */
}

/**
 * @brief   Configures and activates the I2C peripheral.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
int i2c_lld_start(I2CDriver *i2cp) {
  HS_I2C_Type *dp = i2cp->i2c;

  if(i2cp->config == NULL) {
    return -1;
  }

  /* Enables the peripheral.*/
#if HS_I2C_USE_I2C0
  if (&I2CD0 == i2cp) {
    cpmResetI2C0();
    cpmEnableI2C0();
    nvicEnableVector(IRQ_I2C, ANDES_PRIORITY_MASK(HS_I2C_I2C0_IRQ_PRIORITY));
  }
#endif /* HS_I2C_USE_I2C0 */

  /* Configures the peripheral.*/

  /* Disable the adapter */
  dp->ENABLE = 0;

  /* Setup I2C parameters.*/
  i2c_lld_set_opmode(i2cp);
  i2c_lld_set_clock(i2cp);

  /* Configure Tx/Rx FIFO threshold levels */
  dp->TX_TL = i2cp->tx_fifo_depth - 1;
  dp->RX_TL = 0;

  #if ((I2C_USE_DMA == TRUE) && (HS_DMA_REQUIRED == TRUE))
  i2c_tx_dma_config(i2cp);
  i2c_rx_dma_config(i2cp);
  #endif
  
  /* i2c enter SLAVE mode */
  if (i2cp->config->op_mode == OPMODE_I2C_SLAVE) 
    /* setting I2C slave addr */
    dp->SAR = I2C_SLAVE_DEFAULT_ADDR;

  /* disable interrupts */
  dp->INTR_MASK = 0;

  if (&I2CD0 == i2cp) {
    cpmDisableI2C0();
  }
  
  return 0;
}

/**
 * @brief   Deactivates the I2C peripheral.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
int i2c_lld_stop(I2CDriver *i2cp) {

  if (i2cp->state != I2C_STOP) {
    /* Resets the peripheral.*/

    /* I2C disable.*/
    i2c_lld_abort_operation(i2cp);

    /* Disables the peripheral.*/
#if HS_I2C_USE_I2C0
    if (&I2CD0 == i2cp) {
      nvicDisableVector(IRQ_I2C);
      cpmDisableI2C0();
    }
#endif /* HS_I2C_USE_I2C0 */
  }

  return 0;
}

/**
 * @brief   Receives data via the I2C bus as master.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 * @param[in] addr      slave device address
 * @param[out] rxbuf    pointer to the receive buffer
 * @param[in] rxbytes   number of bytes to be received
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The operation status.
 * @retval RDY_OK       if the function succeeded.
 * @retval RDY_RESET    if one or more I2C errors occurred, the errors can
 *                      be retrieved using @p i2cGetErrors().
 * @retval RDY_TIMEOUT  if a timeout occurred before operation end. <b>After a
 *                      timeout the driver must be stopped and restarted
 *                      because the bus is in an uncertain state</b>.
 *
 * @notapi
 */
msg_t i2c_lld_master_receive_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                     uint8_t *rxbuf, size_t rxbytes,
                                     systime_t timeout) {

  HS_I2C_Type *dp = i2cp->i2c;
  systime_t start, end;
  msg_t ret;

  I2C_ENBALE_CLOCK(i2cp);

  /* Releases the lock from high level driver.*/
  osalSysUnlock();

  /* Initializes driver fields, LSB = 1 -> receive.*/
  i2cp->addr = (addr << 1) | 0x01;
  i2cp->errors = 0;
  i2cp->status = STATUS_IDLE;
  i2cp->msg_write_idx = 0;
  i2cp->msg_read_idx = 0;
  i2cp->msgs[0].buf = rxbuf;
  i2cp->msgs[0].len = rxbytes;
  i2cp->msgs[0].flags = I2C_M_RD;
  i2cp->msgs[1].buf = NULL;
  i2cp->msgs[1].len = 0;
  i2cp->msgs[1].flags = 0;

  /* Calculating the time window for the timeout on the busy bus condition.*/
  start = osalOsGetSystemTimeX();
  end = start + OSAL_MS2ST(HS_I2C_BUSY_TIMEOUT);

  /* Waits until BUSY flag is reset and the STOP from the previous operation
     is completed, alternatively for a timeout condition.*/
  while (true) {
    osalSysLock();

    if ((dp->STATUS & I2C_STATUS_ACTIVITY) == 0) break;
    //if ((dp->RAW_INTR_STAT & I2C_INTR_STOP_DET) == 0) break;

    if (!osalOsIsTimeWithinX(osalOsGetSystemTimeX(), start, end)) {
      I2C_DISBALE_CLOCK(i2cp);
      return MSG_TIMEOUT;
    }

    osalSysUnlock();
  }

  dp->ENABLE = 0;
  /* set the slave (target) address */
  dp->TAR = addr;

  if (i2cp->config->con_mode == CONMODE_I2C_NO_AUTO_WR)
    dp->CON1 = rxbytes | I2C_CON1_RX_ENABLE | I2C_CON1_READBYTES_UPDATE;
  else {
    dp->CON1 = (i2cp->config->timer_control_channel<<20) | rxbytes | I2C_CON1_RX_ENABLE | I2C_CON1_READBYTES_UPDATE;
    
    if (i2cp->config->i2c_callback)
      i2cp->i2c_callback = i2cp->config->i2c_callback;
 
  }

#if ((I2C_USE_DMA == TRUE) && (HS_DMA_REQUIRED == TRUE))  
  dp->DMA_CR = I2C_DMA_CR_RDMAE;
  dmaStreamStart(i2cp->dma_rx, &dp->DATA_CMD, rxbuf, rxbytes);
  dp->INTR_MASK = I2C_INTR_STOP_DET;
#else
  /* Enable interrupts */
  dp->INTR_MASK = I2C_INTR_DEFAULT_MASK;   
#endif
  
  if (i2cp->config->con_mode == CONMODE_I2C_NO_AUTO_WR)
    /* Enable the adapter */
    dp->ENABLE = 1;

  /* Waits for the operation completion or a timeout.*/
  ret = osalThreadSuspendTimeoutS(&i2cp->thread, timeout);
  I2C_DISBALE_CLOCK(i2cp);
  return ret;
}

/**
 * @brief   Transmits data via the I2C bus as master.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 * @param[in] addr      slave device address
 * @param[in] txbuf     pointer to the transmit buffer
 * @param[in] txbytes   number of bytes to be transmitted
 * @param[out] rxbuf    pointer to the receive buffer
 * @param[in] rxbytes   number of bytes to be received
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The operation status.
 * @retval RDY_OK       if the function succeeded.
 * @retval RDY_RESET    if one or more I2C errors occurred, the errors can
 *                      be retrieved using @p i2cGetErrors().
 * @retval RDY_TIMEOUT  if a timeout occurred before operation end. <b>After a
 *                      timeout the driver must be stopped and restarted
 *                      because the bus is in an uncertain state</b>.
 *
 * @notapi
 */
msg_t i2c_lld_master_transmit_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                      const uint8_t *txbuf, size_t txbytes,
                                      uint8_t *rxbuf, size_t rxbytes,
                                      systime_t timeout) {
  HS_I2C_Type *dp = i2cp->i2c;
  systime_t start, end;
  msg_t ret;

  I2C_ENBALE_CLOCK(i2cp) 

  /* Releases the lock from high level driver.*/
  osalSysUnlock();

  /* Initializes driver fields, LSB = 0 -> write.*/
  i2cp->addr = addr << 1;
  i2cp->errors = 0;
  i2cp->status = STATUS_IDLE;
  i2cp->msg_write_idx = 0;
  i2cp->msg_read_idx = 0;
  i2cp->msgs[0].buf = (uint8_t *)txbuf;
  i2cp->msgs[0].len = txbytes;
  i2cp->msgs[0].flags = 0;
  i2cp->msgs[1].buf = rxbuf;
  i2cp->msgs[1].len = rxbytes;
  i2cp->msgs[1].flags = rxbuf ? I2C_M_RD : 0;

  /* Calculating the time window for the timeout on the busy bus condition.*/
  start = osalOsGetSystemTimeX();
  end = start + OSAL_MS2ST(HS_I2C_BUSY_TIMEOUT);

  /* Waits until BUSY flag is reset and the STOP from the previous operation
     is completed, alternatively for a timeout condition.*/
  while (true) {
    osalSysLock();

    if ((dp->STATUS & I2C_STATUS_ACTIVITY) == 0) break;
    //if ((dp->RAW_INTR_STAT & I2C_INTR_STOP_DET) == 0) break;

    if (!osalOsIsTimeWithinX(osalOsGetSystemTimeX(), start, end)) {
      I2C_DISBALE_CLOCK(i2cp);
      return MSG_TIMEOUT;
    }

    osalSysUnlock();
  }

  dp->ENABLE = 0;
  /* set the slave (target) address */
  dp->TAR = addr;
 
  if (i2cp->config->con_mode == CONMODE_I2C_NO_AUTO_WR)
    dp->CON1 = 0;
  else {
    dp->CON1 = (i2cp->config->timer_control_channel<<20);     
    if (i2cp->config->i2c_callback)
      i2cp->i2c_callback = i2cp->config->i2c_callback;
  }

#if ((I2C_USE_DMA == TRUE) && (HS_DMA_REQUIRED == TRUE))
  if (i2cp->dma_tx) {
    dp->DMA_CR = I2C_DMA_CR_TDMAE;
    dmaStreamStart(i2cp->dma_tx, txbuf, &dp->DATA_CMD, txbytes);
    dp->INTR_MASK = I2C_INTR_STOP_DET | I2C_INTR_TX_ABRT;
  }
#else
  /* Enable interrupts */
  dp->INTR_MASK = I2C_INTR_DEFAULT_MASK;
#endif

  if (i2cp->config->con_mode == CONMODE_I2C_NO_AUTO_WR)
    /* Enable the adapter */
    dp->ENABLE = 1;

  /* Waits for the operation completion or a timeout.*/
  ret = osalThreadSuspendTimeoutS(&i2cp->thread, timeout);
  I2C_DISBALE_CLOCK(i2cp);
  return ret;
}


msg_t i2c_lld_master_write_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                      const uint8_t *txbuf, size_t txbytes,
                                      systime_t timeout) {
  HS_I2C_Type *dp = i2cp->i2c;
  systime_t start, end;
  msg_t ret;

  I2C_ENBALE_CLOCK(i2cp) 

  /* Releases the lock from high level driver.*/
  osalSysUnlock();

  /* Initializes driver fields, LSB = 0 -> write.*/
  i2cp->addr = addr << 1;
  i2cp->errors = 0;
  i2cp->status = STATUS_IDLE;
  i2cp->msg_write_idx = 0;
  i2cp->msg_read_idx = 0;
  i2cp->msgs[0].buf = (uint8_t *)txbuf;
  i2cp->msgs[0].len = txbytes;
  i2cp->msgs[0].flags = 0;
  i2cp->msgs[1].buf = NULL;
  i2cp->msgs[1].len = 0;
  i2cp->msgs[1].flags = 0;

  /* Calculating the time window for the timeout on the busy bus condition.*/
  start = osalOsGetSystemTimeX();
  end = start + OSAL_MS2ST(HS_I2C_BUSY_TIMEOUT);

  /* Waits until BUSY flag is reset and the STOP from the previous operation
     is completed, alternatively for a timeout condition.*/
  while (true) {
    osalSysLock();

    if ((dp->STATUS & I2C_STATUS_ACTIVITY) == 0) break;
    //if ((dp->RAW_INTR_STAT & I2C_INTR_STOP_DET) == 0) break;

    if (!osalOsIsTimeWithinX(osalOsGetSystemTimeX(), start, end)) {
      I2C_DISBALE_CLOCK(i2cp);
      return MSG_TIMEOUT;
    }

    osalSysUnlock();
  }

  dp->ENABLE = 0;
  /* set the slave (target) address */
  dp->TAR = addr;

  if (i2cp->config->con_mode == CONMODE_I2C_NO_AUTO_WR)
    dp->CON1 = 0;
  else {
    dp->CON1 = (i2cp->config->timer_control_channel<<20);     
    if (i2cp->config->i2c_callback)
      i2cp->i2c_callback = i2cp->config->i2c_callback;
  }

#if ((I2C_USE_DMA == TRUE) && (HS_DMA_REQUIRED == TRUE))
  if (i2cp->dma_tx) {
    dp->DMA_CR = I2C_DMA_CR_TDMAE;
    dmaStreamStart(i2cp->dma_tx, txbuf, &dp->DATA_CMD, txbytes);
    dp->INTR_MASK = I2C_INTR_STOP_DET | I2C_INTR_TX_ABRT;
  }
#else
  /* Enable interrupts */
  dp->INTR_MASK = I2C_INTR_DEFAULT_MASK;
#endif

  if (i2cp->config->con_mode == CONMODE_I2C_NO_AUTO_WR)
    /* Enable the adapter */
    dp->ENABLE = 1;
 
  /* Waits for the operation completion or a timeout.*/
  ret = osalThreadSuspendTimeoutS(&i2cp->thread, timeout);
  I2C_DISBALE_CLOCK(i2cp);
  return ret;
}


msg_t i2c_lld_slave_receive_timeout(I2CDriver *i2cp, const uint8_t *rxbuf, 
				    size_t rxbytes, systime_t timeout) {
  HS_I2C_Type *dp = i2cp->i2c;
  systime_t start, end;
  msg_t ret;

  I2C_ENBALE_CLOCK(i2cp);

  /* Releases the lock from high level driver.*/
  osalSysUnlock();

  /* Initializes driver fields, LSB = 1 -> receive.*/
  i2cp->addr = 0;
  i2cp->errors = 0;
  i2cp->status = STATUS_IDLE;
  i2cp->msg_write_idx = 0;
  i2cp->msg_read_idx = 0;
  i2cp->msgs[0].buf = (uint8_t *)rxbuf;
  i2cp->msgs[0].len = rxbytes;
  i2cp->msgs[0].flags = I2C_M_RD;
  i2cp->msgs[1].buf = NULL;
  i2cp->msgs[1].len = 0;
  i2cp->msgs[1].flags = 0;

  /* Calculating the time window for the timeout on the busy bus condition.*/
  start = osalOsGetSystemTimeX();
  end = start + OSAL_MS2ST(HS_I2C_BUSY_TIMEOUT);

  /* Waits until BUSY flag is reset and the STOP from the previous operation
     is completed, alternatively for a timeout condition.*/
  while (true) {
    osalSysLock();

    if ((dp->STATUS & I2C_STATUS_ACTIVITY) == 0) break;
    //if ((dp->RAW_INTR_STAT & I2C_INTR_STOP_DET) == 0) break;

    if (!osalOsIsTimeWithinX(osalOsGetSystemTimeX(), start, end)) {
      I2C_DISBALE_CLOCK(i2cp);
      return MSG_TIMEOUT;
    }

    osalSysUnlock();
  }

  /* Enable the adapter */
  dp->ENABLE = 1;
  /* Enable interrupts */
  dp->INTR_MASK = I2C_INTR_SLAVE_MASK;

  /* Waits for the operation completion or a timeout.*/
  ret = osalThreadSuspendTimeoutS(&i2cp->thread, timeout);
  I2C_DISBALE_CLOCK(i2cp);
  return ret;
}

msg_t i2c_lld_slave_transmit_timeout(I2CDriver *i2cp, const uint8_t *txbuf, 
				     size_t txbytes, systime_t timeout){
  HS_I2C_Type *dp = i2cp->i2c;
  systime_t start, end;
  msg_t ret;

  I2C_ENBALE_CLOCK(i2cp); 

  /* Releases the lock from high level driver.*/
  osalSysUnlock();

  /* Initializes driver fields, LSB = 0 -> write.*/
  i2cp->addr = 0;
  i2cp->errors = 0;
  i2cp->status = STATUS_IDLE;
  i2cp->msg_write_idx = 0;
  i2cp->msg_read_idx = 0;
  i2cp->msgs[0].buf = (uint8_t *)txbuf;
  i2cp->msgs[0].len = txbytes;
  i2cp->msgs[0].flags = 0;
  i2cp->msgs[1].buf = NULL;
  i2cp->msgs[1].len = 0;
  i2cp->msgs[1].flags = 0;

  /* Calculating the time window for the timeout on the busy bus condition.*/
  start = osalOsGetSystemTimeX();
  end = start + OSAL_MS2ST(HS_I2C_BUSY_TIMEOUT);

  /* Waits until BUSY flag is reset and the STOP from the previous operation
     is completed, alternatively for a timeout condition.*/
  while (true) {
    osalSysLock();

    if ((dp->STATUS & I2C_STATUS_ACTIVITY) == 0) break;
    //if ((dp->RAW_INTR_STAT & I2C_INTR_STOP_DET) == 0) break;

    if (!osalOsIsTimeWithinX(osalOsGetSystemTimeX(), start, end)) {
      I2C_DISBALE_CLOCK(i2cp);
      return MSG_TIMEOUT;
    }

    osalSysUnlock();
  }

  /* Enable the adapter */
  dp->ENABLE = 1;
  /* Enable interrupts */
  dp->INTR_MASK = I2C_INTR_SLAVE_MASK; 

  /* Waits for the operation completion or a timeout.*/
  ret = osalThreadSuspendTimeoutS(&i2cp->thread, timeout);
  I2C_DISBALE_CLOCK(i2cp);
  return ret;
}

/**
 * @brief   Read from i2c memory via the I2C bus as master.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 * @param[in] addr      slave device address
 * @param[in] offset    offset address to i2c memory
 * @param[in] alen      the length of offset address in byte (max to 3-byte)
 * @param[out] rxbuf    pointer to the receive buffer
 * @param[in] rxbytes   number of bytes to be received
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The operation status.
 * @retval RDY_OK       if the function succeeded.
 * @retval RDY_RESET    if one or more I2C errors occurred, the errors can
 *                      be retrieved using @p i2cGetErrors().
 * @retval RDY_TIMEOUT  if a timeout occurred before operation end. <b>After a
 *                      timeout the driver must be stopped and restarted
 *                      because the bus is in an uncertain state</b>.
 *
 * @notapi
 */
msg_t i2c_lld_master_readmem_timeout(I2CDriver *i2cp, i2caddr_t addr,
				     uint32_t offset, uint8_t alen,
				     uint8_t *rxbuf, size_t rxbytes,
				     systime_t timeout) {
  uint8_t abuf[3];
  i2c_address_to_buffer(abuf, offset, alen);
  
#if 0
  msg_t msg;
  msg = i2c_lld_master_write_timeout(i2cp, addr, abuf, alen, timeout);
  if (msg != MSG_OK)
    return msg;
  return i2c_lld_master_receive_timeout(i2cp, addr, rxbuf, rxbytes, timeout);
#endif  

  return i2c_lld_master_transmit_timeout(i2cp, addr, abuf, alen,
					 rxbuf, rxbytes, timeout);
}

/**
 * @brief   Write to i2c memory via the I2C bus as master.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 * @param[in] addr      slave device address
 * @param[in] offset    offset address to i2c memory
 * @param[in] alen      the length of offset address in byte (max to 3-byte)
 * @param[out] txbuf    pointer to the transmit buffer
 * @param[in] txbytes   number of bytes to be transmitted
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The operation status.
 * @retval RDY_OK       if the function succeeded.
 * @retval RDY_RESET    if one or more I2C errors occurred, the errors can
 *                      be retrieved using @p i2cGetErrors().
 * @retval RDY_TIMEOUT  if a timeout occurred before operation end. <b>After a
 *                      timeout the driver must be stopped and restarted
 *                      because the bus is in an uncertain state</b>.
 *
 * @notapi
 */
msg_t i2c_lld_master_writemem_timeout(I2CDriver *i2cp, i2caddr_t addr,
				      uint32_t offset, uint8_t alen,
                                      const uint8_t *txbuf, size_t txbytes,
                                      systime_t timeout) {  
  uint8_t abuf[3];
  msg_t msg;
  uint8_t *eep_mem;
   
  //I2C_ENBALE_CLOCK(i2cp);
  i2c_address_to_buffer(abuf, offset, alen);

  if ((eep_mem = (uint8_t*)osBmemAlloc(EEPROM_PAGE_SIZE)) == NULL)
    return MSG_TIMEOUT;
  
  /* 1. combine addr and data */
  memcpy(eep_mem, abuf, alen);
  memcpy(eep_mem+alen, txbuf, txbytes);
  //nds32_dcache_flush();
  
  /* 2. write data to mem  */
  msg = i2c_lld_master_write_timeout(i2cp, addr, eep_mem, txbytes+alen, timeout);
  osBmemFree(eep_mem);
  return msg;
}

#endif /* HAL_USE_I2C */

/** @} */
