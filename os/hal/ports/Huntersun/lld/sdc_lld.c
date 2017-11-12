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
 * @file    hs66xx/sdc_lld.c
 * @brief   SDC Driver subsystem low level driver source.
 *
 * @addtogroup SDC
 * @{
 */

#include <string.h>
#include "ch.h"
#include "hal.h"

#if HAL_USE_SDC || defined(__DOXYGEN__)

#define HAL_SDC_USE_POLL FALSE

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/* Common flag combinations */
#define DW_MCI_DATA_ERROR_FLAGS	(SDHC_INT_DTO | SDHC_INT_DCRC | \
				 SDHC_INT_HTO | SDHC_INT_SBE  | \
				 SDHC_INT_EBE)
#define DW_MCI_CMD_ERROR_FLAGS	(SDHC_INT_RTO | SDHC_INT_RCRC | \
				 SDHC_INT_RESP_ERR)
#define DW_MCI_ERROR_FLAGS	(DW_MCI_DATA_ERROR_FLAGS | \
				 DW_MCI_CMD_ERROR_FLAGS  | SDHC_INT_HLE)

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   SDCD0 driver identifier.
 */
SDCDriver SDCD0;

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   Buffer for temporary storage during unaligned transfers.
 */
static union _tmp_buf{
  uint32_t  alignment;
  uint8_t   buf[MMCSD_BLOCK_SIZE];
} *u = 0;

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Setup card clock.
 *
 * @param[in] cclk      card clock in Hz
 *
 * @notapi
 */
static void sdc_lld_setup_clock(SDCDriver *sdcp, uint32_t cclk) {

  uint32_t div, clk;

  clk = cpm_get_clock(HS_SD_CLK);
  div = clk / cclk;
  if (clk % cclk && clk > cclk)
    /* prevent over-clocking the card */
    div += 1;
  div = (clk != cclk) ? DIV_ROUND_UP(div, 2) : 0;

  /* disable clock and inform CIU */
  HS_SDHC->CLKENA = 0;
  HS_SDHC->CLKSRC = 0;
  sdc_lld_send_cmd_none(sdcp, SDHC_CMD_UPD_CLK, 0);

  /* set clock to desired speed */
  HS_SDHC->CLKDIV = div;
  sdc_lld_send_cmd_none(sdcp, SDHC_CMD_UPD_CLK, 0);

  /* enable clock; only low power if no SDIO */
  HS_SDHC->CLKENA  = (SDHC_CLKEN_ENABLE << sdcp->id) |
    (SDHC_CLKEN_LOW_PWR << sdcp->id);
  sdc_lld_send_cmd_none(sdcp, SDHC_CMD_UPD_CLK, 0);
}

/**
 * @brief   Setup IDMAC descriptor to start transfer data in DMA
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[out] buf      pointer to the DMA buffer
 * @param[in] n         number of blocks to read
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   operation succeeded.
 * @retval CH_FAILED    operation failed.
 *
 * @notapi
 */
static bool_t sdc_lld_setup_idmac(SDCDriver *sdcp, uint8_t *buf, uint32_t n) {

  struct idmac_desc *desc;
  uint8_t *tbuf = buf;
  uint32_t sg_length = (n * MMCSD_BLOCK_SIZE + 4096)/4096;
  (void)sdcp;

  //if(sdcp->sg_length < sg_length){
    if(sdcp->sg != NULL)
      osBmemFree(sdcp->sg);

    sdcp->sg = (struct idmac_desc *)osBmemAlloc(sg_length * sizeof(struct idmac_desc));
    if(sdcp->sg  == 0){
      sdcp->sg = 0;
      sdcp->sg_length = 0;
      return HAL_FAILED;
    }

     sdcp->sg_length = sg_length;
  //}
  /* dw_mci_translate_sglist */
  desc = sdcp->sg;

  /* Setting up data transfer.*/
  HS_SDHC->BYTCNT = n * MMCSD_BLOCK_SIZE;
  HS_SDHC->BLKSIZ = MMCSD_BLOCK_SIZE;

  osalDbgAssert(n * MMCSD_BLOCK_SIZE <= 4096 * 4, "too large");
  /* Enable the DMA interface */
  HS_SDHC->CTRL |= SDHC_CTRL_DMA_ENABLE;

  while (TRUE) {
    /* buffer size in 13-bit, 0x1FFF = 8191 */
    uint32_t len = min(n * MMCSD_BLOCK_SIZE, 4096);
    /* Set the OWN bit and disable interrupts for this descriptor */
    desc->des0 = IDMAC_DES0_OWN | IDMAC_DES0_DIC | IDMAC_DES0_CH;
    /* Buffer length */
    desc->des1 = len & 0x1fff;
    /* Physical address to DMA to/from */
    desc->des2 = (uint32_t)tbuf;

    n -= len / MMCSD_BLOCK_SIZE;
    if (n == 0)
      break;
    desc->des3 = (uint32_t)(desc + 1);
    tbuf += len;
    desc++;
  }

  /* Set first descriptor */
  sdcp->sg->des0 |= IDMAC_DES0_FD;
  /* Set last descriptor */
  desc->des0 &= ~(IDMAC_DES0_CH | IDMAC_DES0_DIC);
  desc->des0 |= IDMAC_DES0_LD;

  desc->des3 = (uint32_t)sdcp->sg;
  HS_SDHC->DBADDR = (uint32_t)sdcp->sg;
#if defined(__nds32__)
  nds32_dcache_flush();
#endif

  /* if DMA transfer is done, DTO (Data Transfer Over) might not done */
  HS_SDHC->INTMASK = SDHC_INT_DATA_OVER;// | DW_MCI_DATA_ERROR_FLAGS;
  HS_SDHC->IDINTEN = SDHC_IDMAC_INT_NI;// | SDHC_IDMAC_INT_RI | SDHC_IDMAC_INT_TI;
#if !HAL_SD_USE_POLL
  HS_SDHC->CTRL |= SDHC_CTRL_INT_ENABLE;
#endif

  /* Select IDMAC interface */
  HS_SDHC->CTRL |= SDHC_CTRL_USE_IDMAC;
  /* Enable the IDMAC */
  HS_SDHC->BMOD |= SDHC_IDMAC_ENABLE | SDHC_IDMAC_FB;

  /* Start it running */
  HS_SDHC->PLDMND = 1;
  return HAL_SUCCESS;
}

/**
 * @brief   Prepares card to handle read transaction.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] startblk  first block to read
 * @param[in] n         number of blocks to read
 * @param[in] resp      pointer to the response buffer
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   operation succeeded.
 * @retval CH_FAILED    operation failed.
 *
 * @notapi
 */
static bool_t sdc_lld_prepare_read(SDCDriver *sdcp, uint32_t startblk,
                                   uint32_t n, uint32_t *resp) {

  uint32_t cmd;

  /* Driver handles data in 512 bytes blocks (just like HC cards). But if we
     have not HC card than we must convert address from blocks to bytes.*/
  if (!(sdcp->cardmode & SDC_MODE_HIGH_CAPACITY))
    startblk *= MMCSD_BLOCK_SIZE;

  if (n > 1) {
    /* Send read multiple blocks command to card with auto stop. */
    cmd = MMCSD_CMD_READ_MULTIPLE_BLOCK | SDHC_CMD_SEND_STOP | SDHC_CMD_DAT_EXP;
    if (sdc_lld_send_cmd_short_crc(sdcp, cmd,
                                   startblk, resp) || MMCSD_R1_ERROR(resp[0]))
      return HAL_FAILED;
  }
  else{
    /* Send read single block command.*/
    cmd = MMCSD_CMD_READ_SINGLE_BLOCK | SDHC_CMD_DAT_EXP;
    if (sdc_lld_send_cmd_short_crc(sdcp, cmd,
                                   startblk, resp) || MMCSD_R1_ERROR(resp[0]))
      return HAL_FAILED;
  }

  return HAL_SUCCESS;
}

/**
 * @brief   Prepares card to handle write transaction.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] startblk  first block to read
 * @param[in] n         number of blocks to write
 * @param[in] resp      pointer to the response buffer
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   operation succeeded.
 * @retval CH_FAILED    operation failed.
 *
 * @notapi
 */
static bool_t sdc_lld_prepare_write(SDCDriver *sdcp, uint32_t startblk,
                                    uint32_t n, uint32_t *resp) {

  uint32_t cmd;

  /* Driver handles data in 512 bytes blocks (just like HC cards). But if we
     have not HC card than we must convert address from blocks to bytes.*/
  if (!(sdcp->cardmode & SDC_MODE_HIGH_CAPACITY))
    startblk *= MMCSD_BLOCK_SIZE;

  if (n > 1) {
    /* Write multiple blocks command with auto stop. */
    cmd = MMCSD_CMD_WRITE_MULTIPLE_BLOCK | SDHC_CMD_SEND_STOP | SDHC_CMD_DAT_WR | SDHC_CMD_DAT_EXP | SDHC_CMD_HOLD;
    if (sdc_lld_send_cmd_short_crc(sdcp, cmd,
                                   startblk, resp) || MMCSD_R1_ERROR(resp[0]))
      return HAL_FAILED;
  }
  else{
    /* Write single block command.*/
    cmd = MMCSD_CMD_WRITE_BLOCK | SDHC_CMD_DAT_WR | SDHC_CMD_DAT_EXP | SDHC_CMD_HOLD;
    if (sdc_lld_send_cmd_short_crc(sdcp, cmd,
                                   startblk, resp) || MMCSD_R1_ERROR(resp[0]))
      return HAL_FAILED;
  }

  return HAL_SUCCESS;
}

/**
 * @brief   Wait end of data transaction and performs finalizations.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] n         number of blocks in transaction
 * @param[in] resp      pointer to the response buffer
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   operation succeeded.
 * @retval CH_FAILED    operation failed.
 */
static bool_t sdc_lld_wait_transaction_end(SDCDriver *sdcp, uint32_t n,
                                           uint32_t *resp) {

  uint32_t idsts, rintsts;
  (void)resp;

  /* Note the mask is checked before going to sleep because the interrupt
     may have occurred before reaching the critical zone.*/
  chSysLock();
#if !HAL_SDC_USE_POLL
  if (HS_SDHC->INTMASK != 0) {
    osalDbgAssert(sdcp->thread == NULL, "not NULL");
    sdcp->thread = currp;
    chSchGoSleepS(CH_STATE_SUSPENDED);
    chDbgAssert(sdcp->thread == NULL, "not NULL");
  }
#else
  while ((HS_SDHC->RINTSTS & SDHC_INT_DATA_OVER) == 0)
    ;
#endif

#if 1
  //chDbgAssert(((HS_SDHC->IDSTS >> 13) & 0x0f) == 0, "IDMAC not idle");
  if(((HS_SDHC->IDSTS >> 13) & 0x0f) != 0)
  {
    if(sdcp->sg != NULL)
    {
      osBmemFree(sdcp->sg);
      sdcp->sg = NULL;
    }

    chSysUnlock();
    return HAL_FAILED;
  }
#else
  /* Wait until IDMAC become idle.*/
  while (((HS_SDHC->IDSTS >> 13) & 0x0f) != 0) {
    HS_SDHC->PLDMND = 1;
  }
#endif

  /* DMA event flags must be manually cleared.*/
  idsts = HS_SDHC->IDSTS;
  HS_SDHC->IDSTS = idsts;
  rintsts = HS_SDHC->RINTSTS;
  HS_SDHC->RINTSTS = rintsts;

  /* Disable and reset the IDMAC interface */
  HS_SDHC->CTRL &= ~SDHC_CTRL_USE_IDMAC;
  HS_SDHC->CTRL |= SDHC_CTRL_DMA_RESET;
  //* Stop the IDMAC running */
  HS_SDHC->BMOD &= ~(SDHC_IDMAC_ENABLE | SDHC_IDMAC_FB);

  chSysUnlock();


  /* Make sure that data transfer over & no abnormal interrupt */
  if ((rintsts & (SDHC_INT_DATA_OVER | SDHC_INT_DCRC)) == 0) {

    if(sdcp->sg != NULL)
    {
      osBmemFree(sdcp->sg);
      sdcp->sg = NULL;
    }
  
    return HAL_FAILED;
  }
  if ((idsts & SDHC_IDMAC_INT_AI) != 0) {

    if(sdcp->sg != NULL)
    {
      osBmemFree(sdcp->sg);
      sdcp->sg = NULL;
    }
    
    return HAL_FAILED;
  }

  /* Finalize transaction.*/
  if (n > 1) {
#if 1
    uint32_t tmp = 0;

    /* wait auto stop done */
    while ( ((rintsts & SDHC_INT_ACD) == 0) && (tmp++ < 0x10000))
      rintsts = HS_SDHC->RINTSTS;
    HS_SDHC->RINTSTS = rintsts;
#else
    return sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_STOP_TRANSMISSION, 0, resp);
#endif
  }

  if(sdcp->sg != NULL)
  {
    osBmemFree(sdcp->sg);
    sdcp->sg = NULL;
  }

  return HAL_SUCCESS;
}

/**
 * @brief   Gets SDC errors.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] sta       value of the STA register
 *
 * @notapi
 */
static void sdc_lld_collect_errors(SDCDriver *sdcp, uint32_t sta) {
  uint32_t errors = SDC_NO_ERROR;

  if (sta & SDHC_INT_RCRC)
    errors |= SDC_CMD_CRC_ERROR;
  if (sta & SDHC_INT_DCRC)
    errors |= SDC_DATA_CRC_ERROR;
  if (sta & SDHC_INT_RTO)
    errors |= SDC_COMMAND_TIMEOUT;
  if (sta & SDHC_INT_DTO)
    errors |= SDC_DATA_TIMEOUT;
  if (sta & SDHC_INT_HTO) //FIXME
    errors |= SDC_TX_UNDERRUN;
  if (sta & SDHC_INT_FRUN)
    errors |= SDC_RX_OVERRUN;
  if (sta & SDHC_INT_SBE)
    errors |= SDC_STARTBIT_ERROR;

  sdcp->errors |= errors;
}

/**
 * @brief   Performs clean transaction stopping in case of errors.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] n         number of blocks in transaction
 * @param[in] resp      pointer to the response buffer
 *
 * @notapi
 */
static void sdc_lld_error_cleanup(SDCDriver *sdcp,
                                  uint32_t n,
                                  uint32_t *resp) {
  uint32_t sta;
  (void)resp;

  sta = HS_SDHC->IDSTS;
  HS_SDHC->IDSTS = sta;

  sta = HS_SDHC->RINTSTS;
  HS_SDHC->RINTSTS = sta;
  sdc_lld_collect_errors(sdcp, sta);
  if (n > 1) {
#if 1
    uint32_t tmp = 0;;
    /* wait auto stop done */
    while (((sta & SDHC_INT_ACD) == 0) && (tmp++ < 0x10000))
      sta = HS_SDHC->RINTSTS;
    HS_SDHC->RINTSTS = sta;
#else
    sdc_lld_send_cmd_short_crc(sdcp, MMCSD_CMD_STOP_TRANSMISSION, 0, resp);
#endif
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/**
 * @brief   SDIO IRQ handler.
 * @details It just wakes transaction thread. All error  handling performs in
 *          that thread.
 *
 * @isr
 */
CH_IRQ_HANDLER(SDHC_IRQHandler) {

  CH_IRQ_PROLOGUE();

  chSysLockFromISR();

  /* Disables the source but the status flags are not reset because the
     read/write functions needs to check them.*/
  HS_SDHC->IDINTEN = 0;
  HS_SDHC->INTMASK = 0;
  HS_SDHC->CTRL &= ~SDHC_CTRL_INT_ENABLE;

  if (SDCD0.thread != NULL) {
    chSchReadyI(SDCD0.thread);
    SDCD0.thread = NULL;
  }

  chSysUnlockFromISR();

  CH_IRQ_EPILOGUE();
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level SDC driver initialization.
 *
 * @notapi
 */
void sdc_lld_init(void) {

  sdcObjectInit(&SDCD0);
  SDCD0.thread = NULL;
  SDCD0.id = 0; //one card only
  SDCD0.sg_length = 0;
  SDCD0.sg = 0;
#if CH_DBG_ENABLE_ASSERTS
  SDCD0.sdhc   = HS_SDHC;
  /* no DMA inf, 0x13 is 20-bit addr, 32-bit data, AHB, 1 card, SD_MMC */
  //chDbgAssert(HS_SDHC->HCON == 0x04e44cc1, __FUNCTION__, "wrong IP configration");
#endif
}

/**
 * @brief   Configures and activates the SDC peripheral.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @notapi
 */
void sdc_lld_start(SDCDriver *sdcp) {

  uint32_t fifo_size;

  if(u == NULL){
    osalSysUnlock();
    u = (union _tmp_buf *)osBmemAlloc(sizeof(union _tmp_buf));
    osalSysLock();
  }

  if (sdcp->state == BLK_STOP) {
    cpmResetSDHC();
    cpmEnableSDHC();
    nvicEnableVector(IRQ_SDHOST, ANDES_PRIORITY_MASK(HS_SDC_SDHC_IRQ_PRIORITY));
  }

  /* Configuration, card clock is initially stopped.*/

  /* reset controller */
  HS_SDHC->CTRL   = SDHC_CTRL_RESET | SDHC_CTRL_FIFO_RESET | SDHC_CTRL_DMA_RESET;
  while (HS_SDHC->CTRL & (SDHC_CTRL_RESET | SDHC_CTRL_FIFO_RESET | SDHC_CTRL_DMA_RESET))
    ;

  HS_SDHC->RINTSTS = 0xFFFFFFFF; //clear interrupts
  HS_SDHC->INTMASK = 0; //disable all interrupt
  HS_SDHC->TMOUT   = 0xFFFFFFFF; //max timeout
  fifo_size = ((HS_SDHC->FIFOTH >> 16) & 0x0fff) + 1;
  HS_SDHC->FIFOTH  = (0x2 << 28 ) | ((fifo_size/2 - 1) << 16) | ((fifo_size/2) << 0); //burst size is 8

  /* disable clock to CIU */
  HS_SDHC->CLKENA = 0;
  HS_SDHC->CLKSRC = 0;

  /* enable interrupts: no for IDMAC */
  HS_SDHC->RINTSTS = 0xFFFFFFFF;
  //HS_SDHC->INTMASK = /*SDHC_INT_CMD_DONE | */SDHC_INT_DATA_OVER |
  //  SDHC_INT_TXDR | SDHC_INT_RXDR | DW_MCI_ERROR_FLAGS | SDHC_INT_CD;
#if !HAL_SD_USE_POLL
  HS_SDHC->CTRL    = SDHC_CTRL_INT_ENABLE;
#else
  HS_SDHC->CTRL    = 0;
#endif

  /* IDMAC */
  HS_SDHC->BMOD = SDHC_IDMAC_SWRESET;
  while (HS_SDHC->BMOD & SDHC_IDMAC_SWRESET)
    ;
  /* Mask out interrupts - get Tx & Rx complete only */
  HS_SDHC->IDSTS = 0xFFFFFFFF;
  HS_SDHC->IDINTEN = 0;//SDHC_IDMAC_INT_NI | SDHC_IDMAC_INT_RI | SDHC_IDMAC_INT_TI;

  /* FIXME: regulator enable via GPIO or SDHC */
  HS_SDHC->PWREN = 1 << sdcp->id;

  /* FIXME: get CD# or WP# via GPIO or SDHC */
  //present = HS_SDHC->CDETECT & (1 << sdcp->id) ? 0 : 1;
  //read_only = HS_SDHC->WRTPRT & (1 << sdcp->id) ? 1 : 0;
}

/**
 * @brief   Deactivates the SDC peripheral.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @notapi
 */
void sdc_lld_stop(SDCDriver *sdcp) {

  if (sdcp->state != BLK_STOP) {
    HS_SDHC->PWREN = 0;
    HS_SDHC->CLKENA = 0;
    HS_SDHC->INTMASK = 0;
    HS_SDHC->IDINTEN = 0;
    HS_SDHC->TMOUT   = 0;

    nvicDisableVector(IRQ_SDHOST);
    cpmDisableSDHC();
  }
}

/**
 * @brief   Starts the SDIO clock and sets it to init mode (400kHz or less).
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @notapi
 */
void sdc_lld_start_clk(SDCDriver *sdcp) {

  (void)sdcp;

  /*
   * This delay must be at least 74 clock sizes, or 1 ms, or the
   * time required to reach a stable voltage.
   */
  //chThdSleepMilliseconds(10);

  /* Initial clock setting: 400kHz, 1bit mode.*/
  sdc_lld_setup_clock(sdcp, 400000);
  HS_SDHC->CTYPE = SDHC_CTYPE_1BIT << sdcp->id;
}

/**
 * @brief   Sets the SDIO clock to data mode (25MHz or less).
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @notapi
 */
void sdc_lld_set_data_clk(SDCDriver *sdcp) {

  if((sdcp) && (sdcp->config))
    sdc_lld_setup_clock(sdcp, sdcp->config->speed);
  else
    sdc_lld_setup_clock(sdcp, 12000000);
}

/**
 * @brief   Stops the SDIO clock.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @notapi
 */
void sdc_lld_stop_clk(SDCDriver *sdcp) {

  (void)sdcp;

  /* disable clock and inform CIU */
  HS_SDHC->CLKENA = 0;
  HS_SDHC->CLKSRC = 0;
  sdc_lld_send_cmd_none(sdcp, SDHC_CMD_UPD_CLK, 0);
}

/**
 * @brief   Switches the bus to 4 bits mode.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] mode      bus mode
 *
 * @notapi
 */
void sdc_lld_set_bus_mode(SDCDriver *sdcp, sdcbusmode_t mode) {

  (void)sdcp;

  switch (mode) {
  case SDC_MODE_1BIT:
    HS_SDHC->CTYPE = SDHC_CTYPE_1BIT << sdcp->id;
    break;
  case SDC_MODE_4BIT:
    HS_SDHC->CTYPE = SDHC_CTYPE_4BIT << sdcp->id;
    break;
  case SDC_MODE_8BIT:
    HS_SDHC->CTYPE = SDHC_CTYPE_8BIT << sdcp->id;
    break;
  }
}

/**
 * @brief   Sends an SDIO command with no response expected.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] cmd       card command
 * @param[in] arg       command argument
 *
 * @notapi
 */
void sdc_lld_send_cmd_none(SDCDriver *sdcp, uint32_t cmd, uint32_t arg) {

  (void)sdcp;
  uint32_t tim = 0x800000;

  HS_SDHC->CMDARG = arg;
  /* FIXME: Send initialization sequence (80 clocks) before sending this command */
  if (MMCSD_CMD_GO_IDLE_STATE == cmd) {
    cmd |= SDHC_CMD_INIT;
  }
  HS_SDHC->CMD = cmd | SDHC_CMD_START;

  if (cmd & SDHC_CMD_UPD_CLK) {
    /* internal command needn't command done interrupt */
    while (((HS_SDHC->CMD & SDHC_CMD_START) != 0) && (tim--))
      ;
    HS_SDHC->RINTSTS = 0xFFFFFFFF; //clear interrupts
  } else {
    uint32_t rintsts;
    while ((((rintsts = HS_SDHC->RINTSTS) & SDHC_INT_CMD_DONE) == 0) && (tim--))
      ;
    HS_SDHC->RINTSTS = rintsts;
  }
}

/**
 * @brief   Sends an SDIO command with a short response expected.
 * @note    The CRC is not verified.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] cmd       card command
 * @param[in] arg       command argument
 * @param[out] resp     pointer to the response buffer (one word)
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   operation succeeded.
 * @retval CH_FAILED    operation failed.
 *
 * @notapi
 */
bool sdc_lld_send_cmd_short(SDCDriver *sdcp, uint32_t cmd, uint32_t arg,
                              uint32_t *resp) {

  uint32_t rintsts, tim = 0x800000;
  (void)sdcp;

  HS_SDHC->CMDARG = arg;
  HS_SDHC->CMD = cmd | SDHC_CMD_START | SDHC_CMD_RESP_EXP;
  /* command done means command sent & response received or response timeout */
  while ((((rintsts = HS_SDHC->RINTSTS) &
	  (SDHC_INT_CMD_DONE /*| SDHC_INT_ERROR*/)) == 0) && (tim--))
    ;
  HS_SDHC->RINTSTS = SDHC_INT_CMD_DONE | SDHC_INT_ERROR;//rintsts;
  if ((rintsts & SDHC_INT_ERROR) != 0) {
    sdc_lld_collect_errors(sdcp, rintsts);
    return HAL_FAILED;
  }
  *resp = HS_SDHC->RESP0;
  return HAL_SUCCESS;
}

/**
 * @brief   Sends an SDIO command with a short response expected and CRC.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] cmd       card command
 * @param[in] arg       command argument
 * @param[out] resp     pointer to the response buffer (one word)
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   operation succeeded.
 * @retval CH_FAILED    operation failed.
 *
 * @notapi
 */
bool sdc_lld_send_cmd_short_crc(SDCDriver *sdcp, uint32_t cmd, uint32_t arg,
                                  uint32_t *resp) {

  uint32_t rintsts, tim=0x800000;
  (void)sdcp;

  HS_SDHC->CMDARG = arg;
  HS_SDHC->CMD = cmd | SDHC_CMD_START | SDHC_CMD_RESP_EXP | SDHC_CMD_RESP_CRC;
  while ((((rintsts = HS_SDHC->RINTSTS) &
	  (SDHC_INT_CMD_DONE /*| SDHC_INT_ERROR*/)) == 0) && (tim--))
    ;
  HS_SDHC->RINTSTS = SDHC_INT_CMD_DONE | SDHC_INT_ERROR;//rintsts;
  if ((rintsts & SDHC_INT_ERROR) != 0) {
    sdc_lld_collect_errors(sdcp, rintsts);
    return HAL_FAILED;
  }
  *resp = HS_SDHC->RESP0;
  return HAL_SUCCESS;
}

/**
 * @brief   Sends an SDIO command with a long response expected and CRC.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] cmd       card command
 * @param[in] arg       command argument
 * @param[out] resp     pointer to the response buffer (four words)
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   operation succeeded.
 * @retval CH_FAILED    operation failed.
 *
 * @notapi
 */
bool sdc_lld_send_cmd_long_crc(SDCDriver *sdcp, uint32_t cmd, uint32_t arg,
                                 uint32_t *resp) {

  uint32_t rintsts, tim=0x800000;
  (void)sdcp;

  HS_SDHC->CMDARG = arg;
  HS_SDHC->CMD = cmd | SDHC_CMD_START | SDHC_CMD_RESP_EXP | SDHC_CMD_RESP_LONG | SDHC_CMD_RESP_CRC;
  while ((((rintsts = HS_SDHC->RINTSTS) &
	  (SDHC_INT_CMD_DONE /*| SDHC_INT_ERROR*/)) == 0) && (tim--))
    ;
  HS_SDHC->RINTSTS = SDHC_INT_CMD_DONE | SDHC_INT_ERROR;//rintsts;
  if ((rintsts & SDHC_INT_ERROR) != 0) {
    sdc_lld_collect_errors(sdcp, rintsts);
    return HAL_FAILED;
  }
  *resp++ = HS_SDHC->RESP0; //LSB
  *resp++ = HS_SDHC->RESP1;
  *resp++ = HS_SDHC->RESP2;
  *resp   = HS_SDHC->RESP3; //MSB
  return HAL_SUCCESS;
}

/**
 * @brief   Reads one or more blocks.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] startblk  first block to read
 * @param[out] buf      pointer to the read buffer
 * @param[in] n         number of blocks to read
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   operation succeeded.
 * @retval CH_FAILED    operation failed.
 *
 * @notapi
 */
bool sdc_lld_read_aligned(SDCDriver *sdcp, uint32_t startblk,
                            uint8_t *buf, uint32_t n) {
  uint32_t resp[1];

  osalDbgCheck((n < (0x1000000 / MMCSD_BLOCK_SIZE)));

  /* Checks for errors and waits for the card to be ready for reading.*/
  if (_sdc_wait_for_transfer_state(sdcp))
    return HAL_FAILED;

  if (sdc_lld_setup_idmac(sdcp, (uint8_t *)buf, n))
    return HAL_FAILED;

  /* Talk to card what we want from it.*/
  if (sdc_lld_prepare_read(sdcp, startblk, n, resp) == TRUE)
    goto error;

  if (sdc_lld_wait_transaction_end(sdcp, n, resp) == TRUE)
    goto error;

  return HAL_SUCCESS;

error:
  sdc_lld_error_cleanup(sdcp, n, resp);
  return HAL_FAILED;
}

/**
 * @brief   Writes one or more blocks.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] startblk  first block to write
 * @param[out] buf      pointer to the write buffer
 * @param[in] n         number of blocks to write
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   operation succeeded.
 * @retval CH_FAILED    operation failed.
 *
 * @notapi
 */
bool sdc_lld_write_aligned(SDCDriver *sdcp, uint32_t startblk,
                             const uint8_t *buf, uint32_t n) {
  uint32_t resp[1];

  chDbgCheck((n < (0x1000000 / MMCSD_BLOCK_SIZE)));

  /* Checks for errors and waits for the card to be ready for writing.*/
  if (_sdc_wait_for_transfer_state(sdcp))
    return HAL_FAILED;

  if (sdc_lld_setup_idmac(sdcp, (uint8_t *)buf, n))
    return HAL_FAILED;

  /* Talk to card what we want from it.*/
  if (sdc_lld_prepare_write(sdcp, startblk, n, resp) == TRUE)
    goto error;

  if (sdc_lld_wait_transaction_end(sdcp, n, resp) == TRUE)
    goto error;

  return HAL_SUCCESS;

error:
  sdc_lld_error_cleanup(sdcp, n, resp);
  return HAL_FAILED;
}

/**
 * @brief   Reads one or more blocks.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] startblk  first block to read
 * @param[out] buf      pointer to the read buffer
 * @param[in] n         number of blocks to read
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   operation succeeded.
 * @retval CH_FAILED    operation failed.
 *
 * @notapi
 */
bool sdc_lld_read(SDCDriver *sdcp, uint32_t startblk,
                    uint8_t *buf, uint32_t n) {

  if (((unsigned)buf & 3) != 0) {
    uint32_t i;
    for (i = 0; i < n; i++) {
      if (sdc_lld_read_aligned(sdcp, startblk, u->buf, 1))
        return HAL_FAILED;
      memcpy(buf, u->buf, MMCSD_BLOCK_SIZE);
      buf += MMCSD_BLOCK_SIZE;
      startblk++;
    }
    return HAL_SUCCESS;
  }
  return sdc_lld_read_aligned(sdcp, startblk, buf, n);
}

/**
 * @brief   Writes one or more blocks.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] startblk  first block to write
 * @param[out] buf      pointer to the write buffer
 * @param[in] n         number of blocks to write
 *
 * @return              The operation status.
 * @retval CH_SUCCESS  operation succeeded.
 * @retval CH_FAILED    operation failed.
 *
 * @notapi
 */
bool sdc_lld_write(SDCDriver *sdcp, uint32_t startblk,
                     const uint8_t *buf, uint32_t n) {

  if (((unsigned)buf & 3) != 0) {
    uint32_t i;
    for (i = 0; i < n; i++) {
      memcpy(u->buf, buf, MMCSD_BLOCK_SIZE);
      buf += MMCSD_BLOCK_SIZE;
      if (sdc_lld_write_aligned(sdcp, startblk, u->buf, 1))
        return HAL_FAILED;
      startblk++;
    }
    return HAL_SUCCESS;
  }
  return sdc_lld_write_aligned(sdcp, startblk, buf, n);
}

/**
 * @brief   Waits for card idle condition.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool sdc_lld_sync(SDCDriver *sdcp) {

  (void)sdcp;

  return HAL_SUCCESS;
}

/**
 * @brief   Returns the card insertion status.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @return              The card state.
 * @retval FALSE        card not inserted.
 * @retval TRUE         card inserted.
 *
 * @api
 */
bool_t sdc_lld_is_card_inserted(SDCDriver *sdcp) {

  (void)sdcp;
  uint8_t pad8_cmd, pad10_clk;

  pad8_cmd  = (HS_PMU->PADC_CON[8]  >> 5) & 0x0f;
  pad10_clk = (HS_PMU->PADC_CON[10] >> 5) & 0x0f;
  
  if((pad8_cmd != PAD_FUNC_SD_USB) || (pad10_clk != PAD_FUNC_SD_USB))
    return FALSE;

  return (HS_SDHC->CDETECT == 0);
}

/**
 * @brief   Returns the write protect status.
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @return              The card state.
 * @retval FALSE        not write protected.
 * @retval TRUE         write protected.
 *
 * @api
 */
bool_t sdc_lld_is_write_protected(SDCDriver *sdcp){

  (void)sdcp;

  int i = 0;
  while(i < 24){
    if(((HS_PMU->PADC_CON[i] >> 5) & 0x3f) == PAD_FUNC_SD_WP){
      return HS_SDHC->WRTPRT == 1? TRUE : FALSE;
    }

    i++;
  }

  return FALSE;
}

/**
 * @brief   Returns the data bus width in bit.
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @return              SDC_MODE_1/4/8BIT
 *
 * @api
 */
sdcbusmode_t sdc_lld_get_bus_width(SDCDriver *sdcp){

  (void)sdcp;
  
  /* check DAT0,1,2,3 is configured */
  if ((((HS_PMU->PADC_CON[6] >> 5) & 0x3f) == PAD_FUNC_SD_USB) &&
      (((HS_PMU->PADC_CON[11] >> 5) & 0x3f) == PAD_FUNC_SD_USB) &&
      (((HS_PMU->PADC_CON[12] >> 5) & 0x3f) == PAD_FUNC_SD_USB) &&
      (((HS_PMU->PADC_CON[13] >> 5) & 0x3f) == PAD_FUNC_SD_USB))
    return SDC_MODE_4BIT;
  else
    return SDC_MODE_1BIT;
}
#endif /* HAL_USE_SDC */

/** @} */
