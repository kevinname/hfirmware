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
 * @file    hs66xx/cpm_lld.h=c
 * @brief   clock & pmu driver source.
 *
 * @addtogroup HAL
 * @{
 */

#include "ch.h"
#include "hal.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

__STATIC_INLINE void delay_nops(uint16_t cnt)
{
  while (cnt-- > 0) __NOP();
}

__STATIC_INLINE void pso_reg_update() {
   HS_PSO->REG_UPD = 0x01;
}

__STATIC_INLINE void pmu_cpm_reg_update() {
   HS_PMU_CPM->UPD = 0x01;
}

__STATIC_INLINE void pmu_reg_update() {
   HS_PMU->BASIC |= (1u<<30);
}

static inline uint32_t get_xtal_clock() {
  if(CPM_GetXtalSel())
    return XCLK_CLOCK_24MHz;
  else
    return XCLK_CLOCK_16MHz;
}

static inline uint32_t get_master_clock() {
  /* judge xtal-clk  or pll-clk */
  if (HS_PMU_CPM->PLL_SRC_CFG&0x1)
    return PLL_CLOCK_192M;
  else
    return get_xtal_clock();
}

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/
typedef struct pmu_tune_shadow_s {
  uint32_t rc_rxadc  : 4;
  uint32_t rc_auadc  : 4;
  uint32_t rc_txdac  : 4;
  uint32_t rc_rxfil  : 4;
  uint32_t rc_rxtia  : 4;
  uint32_t ldo_lobuf : 2;
  uint32_t ldo_xtal  : 2;
  uint32_t ldo_vco   : 2;
  uint32_t rsv       : 4;
  uint32_t valid_rc  : 1;
  uint32_t valid_ldo : 1;
} pmu_tune_shadow_t;
static pmu_tune_shadow_t m_pmu_tune_shadow;

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   start system pll module when first start PLL
 *
 * @api
 */
void cpm_start_pll(void) {
  uint32_t i=20000;

  //HS_PMU->EXT = 0;
  HS_PMU->BASIC |= 1u<<30;
  delay_nops(32*2);

  /* first reset pll */
  cpm_reset_pll();
  
  /* donot gate system PLL output to CPU */
  HS_PMU->ANA_CON &= ~(1<<25);

  /* after reset over MUST BE > 1us to wait */
  delay_nops(1*32); 

  /* Start AFC to calibrate PLL */
  HS_ANA->COMMON_CFG[0] |= (1u <<9);
  /* wait 1 us at least */
  delay_nops(32*2);
  HS_ANA->COMMON_CFG[0] &= ~(1u <<9);

  /* wait 220us at lease */
  delay_nops(32*220);
  /* Waiting for PLL lock signal */
  while(i--) {
    if ((HS_PMU->BASIC & 0x80000000) == 0x80000000)
       break;
  }
}

/**
 * @brief reset system pll module when NOT first start PLL .
 *
 * @api
 */

void cpm_reset_pll(void) {
  /* 1. reset pll */
  HS_PMU->BASIC &= ~(1u<<1);
  pmu_reg_update();
  delay_nops(32);

  /* 2. power on pll: pd_sys_pll = 0 */
  HS_PMU->BASIC &= ~(1u<<0);
  pmu_reg_update();

  /* 3. not reset, MUST BE delay >5us */
  delay_nops(32*6);
  HS_PMU->BASIC |= (1u<<1);
  pmu_reg_update();
}

/**
 * @brief   stop system PLL module
 *
 * @api
 */
void cpm_stop_pll(void) {
  /* 1. clear pll lock signal write bt_phy spi reg 0x6d 13-bit=1 */
  volatile uint32_t *ptr = (uint32_t *)0x4000F058;
  *ptr |= (1<<29);
  delay_nops(32*1);

  /* 2. reset pll */
  HS_PMU->BASIC &= ~(1u<<1);
  pmu_reg_update();
  delay_nops(32*5);

  /* 3. power down pll */
  HS_PMU->BASIC |= (1<<0);
  pmu_reg_update();
  delay_nops(32*1);

  /* donot reset and write bt_phy spi reg 0x6d 13-bit=0 */
  //HS_PMU->BASIC |= (1<<1);
  pmu_reg_update();
  *ptr &= ~(1<<29);
}

/**
 * @brief   Platform early initialization.
 * @note    All the involved constants come from the file @p board.h.
 * @note    This function is meant to be invoked early during the system
 *          initialization, it is usually invoked from the file
 *          @p board.c.
 *
 * @special
 */
void cpm_init_clock(void) {
  uint32_t coeff_frc, coeff_int;
  sys_clk_t system_clk;
  uint32_t cpu_cfg;

  /* get XTAL val 16M OR 32M */
  system_clk.xtal_clk = get_xtal_clock();

  /* always change to xtal clock in case of watchdog reset */
  cpm_switch_to_xtal();

  /* start pll */
  cpm_start_pll();
  system_clk.pll_clk = PLL_CLOCK_192M;
  cpm_switch_to_pll();

  /* don't set divider to 1; and 0 means bypass */
  cpu_cfg = 0;
#if __SYSTEM_CLOCK__ != 1
  if (system_clk.pll_clk != CPU_DEFAULT_CLOCK)
    cpu_cfg |= (DIV_ROUND(system_clk.pll_clk, CPU_DEFAULT_CLOCK) << 8);
  if (CPU_DEFAULT_CLOCK != AHB_DEFAULT_CLOCK)
    cpu_cfg |= (DIV_ROUND(CPU_DEFAULT_CLOCK,  AHB_DEFAULT_CLOCK) << 16);
  if (CPU_DEFAULT_CLOCK != RAM_DEFAULT_CLOCK)
    cpu_cfg |= (DIV_ROUND(CPU_DEFAULT_CLOCK,  RAM_DEFAULT_CLOCK) << 24);
#endif
  HS_PSO->CPU_CFG = cpu_cfg | (1 << 7) | (0xf << 1);
  __WFD();

  HS_PSO->APB_CFG = (DIV_ROUND(system_clk.pll_clk, APB_DEFAULT_CLOCK)/2 << 8) | 0x5;
  HS_PSO->SFLASH_CFG = (DIV_ROUND(system_clk.pll_clk, SF_DEFAULT_CLOCK) << 8) | 0x1d;  //delay bypass
  __WFD();

  HS_PSO->USB_CFG =  0x14;
  HS_PMU->ANA_CON &= ~(1<<25);

  HS_PSO->SD_CFG = ((DIV_ROUND(system_clk.pll_clk, SD_DEFAULT_CLOCK)/2) << 8) | 0x05;
  HS_PSO->SD_DRV_CFG = (1<<0)|(1<<1)|(((DIV_ROUND(system_clk.pll_clk, SD_DEFAULT_CLOCK)/4)+1)<<8); //smp clock delay bypass and div clock delay 1/4 clock

  HS_PSO->TIM0_CFG = (DIV_ROUND(system_clk.xtal_clk, TIMER_DEFAULT_CLOCK) << 8) | 0x15;
  HS_PSO->TIM1_CFG = (DIV_ROUND(system_clk.xtal_clk, TIMER_DEFAULT_CLOCK) << 8) | 0x15;
  HS_PSO->TIM2_CFG = (DIV_ROUND(system_clk.xtal_clk, TIMER_DEFAULT_CLOCK) << 8) | 0x15;

  coeff_int = get_xtal_clock()/UART_DEFAULT_CLOCK;
  coeff_frc = DIV_ROUND((get_xtal_clock()-UART_DEFAULT_CLOCK*coeff_int) << CPM_UART_DIV_COEFF_FRC_Len,
                        UART_DEFAULT_CLOCK);

  HS_PSO->UART0_CFG =  (coeff_int << 8) | (coeff_frc << 16) | 0x1d;
  HS_PSO->UART1_CFG =  (coeff_int << 8) | (coeff_frc << 16) | 0x1d;

  HS_PSO->I2C0_CFG =   (DIV_ROUND(system_clk.pll_clk, I2C_DEFAULT_CLOCK)/2 << 8) | 0x15;
  HS_PSO->I2S_CFG =    0x04;                   //unreset
  HS_PSO->CODEC_CFG =  0x04;                   //unreset
  HS_PSO->BTPHY_CFG =  0x2a04;  //LPO source default from xclk divider
  HS_PSO->SPI0_CFG =   4;//0x10;                   //no divider gate_en=0
  HS_PSO->SPI1_CFG =   4;//(DIV_ROUND(system_clk.pll_clk, SPI1_DEFAULT_CLOCK) << 11) | 0x15; //gate_en=0
  HS_PSO->WDT_CFG =    0x00;

  HS_PSO->AHB_GATE_CFG = 0x0a;                 //dma & gpio:
  HS_PSO->CPM_GATE_CFG = 0xa0;
  HS_PSO->CPM_ANA_CFG = 0x04;

  pso_reg_update();
  pmu_reg_update();
}

/**
 * @brief   Get the clock.
 * @param[in] idx       index to clock source
 * @return              clock in Hz
 *
 * @api
 */
uint32_t cpm_get_clock(hs_clk_t idx) {
  uint32_t cfg = 0, div = 4;
  uint32_t top_clk = get_master_clock();

  switch (idx) {
  case HS_PLL_CLK:
    return PLL_CLOCK_192M;

  case HS_CPU_CLK:
    cfg = HS_PSO->CPU_CFG;
    div = CPM_BFEXT(DIV_COEFF, cfg);
    break;

  case HS_AHB_CLK:
    cfg = HS_PSO->CPU_CFG;
    div = CPM_BFEXT(AHB_DIV_COEFF, cfg);
    break;

  case HS_RAM_CLK:
    cfg = HS_PSO->CPU_CFG;
    div = CPM_BFEXT(RAM_DIV_COEFF, cfg);
    break;

  case HS_APB_CLK:
  case HS_SPI0_CLK:
  case HS_SPI1_CLK:
    cfg = HS_PSO->APB_CFG;
    div = CPM_BFEXT(APB_DIV_COEFF, cfg)*2;
    break;

  case HS_SF_CLK:
    cfg = HS_PSO->SFLASH_CFG;
    if (CPM_DIV_EN(cfg))
      div = CPM_BFEXT(SF_DIV_COEFF, cfg);
    break;

  case HS_USB_CLK:
    /* derived from system PLL */
    if (HS_PMU_CPM->PLL_SRC_CFG & 0x1)
      return 48000000;
    else
      return 0;

  case HS_SD_CLK:
    cfg = HS_PSO->SD_CFG;
    if (CPM_DIV_EN(cfg))
      div = CPM_BFEXT(DIV_COEFF, cfg)*2;
    break;

  case HS_TIM0_CLK:
    cfg = HS_PSO->TIM0_CFG;
    if (CPM_DIV_EN(cfg))
      div = CPM_BFEXT(DIV_COEFF, cfg);
    top_clk = get_xtal_clock();
    break;

  case HS_TIM1_CLK:
    cfg = HS_PSO->TIM1_CFG;
    if (CPM_DIV_EN(cfg))
      div = CPM_BFEXT(DIV_COEFF, cfg);
    top_clk = get_xtal_clock();
    break;

  case HS_TIM2_CLK:
    cfg = HS_PSO->TIM2_CFG;
    if (CPM_DIV_EN(cfg))
      div = CPM_BFEXT(DIV_COEFF, cfg);
    top_clk = get_xtal_clock();
    break;

  case HS_XTAL_CLK:
    return get_xtal_clock();

  case HS_UART0_CLK:
  case HS_UART1_CLK:
    {
      uint32_t coeff_frc, coeff_int;
      top_clk = get_xtal_clock();

      if (HS_UART0_CLK == idx)
        cfg = HS_PSO->UART0_CFG;
      else
        cfg = HS_PSO->UART1_CFG;

      coeff_frc = CPM_BFEXT(UART_DIV_COEFF_FRC, cfg);
      coeff_int = CPM_BFEXT(UART_DIV_COEFF_INT, cfg);
      if (CPM_DIV_EN(cfg))
	      return ((top_clk ) / ((coeff_int << CPM_UART_DIV_COEFF_FRC_Len) + coeff_frc)) << CPM_UART_DIV_COEFF_FRC_Len;
      else
	      return (top_clk);
    }

  case HS_I2C0_CLK:
    cfg = HS_PSO->I2C0_CFG;
    if (CPM_DIV_EN(cfg))
      div = CPM_BFEXT(I2C_DIV_COEFF, cfg)*2;
    break;

  case HS_LPO_CLK:
  case HS_RTC_CLK:
  case HS_WDT_CLK:
    return 32000;

  case HS_CODEC_MCLK:
  case HS_BTBB_CLK:
    /* derived from system PLL */
    if (HS_PMU_CPM->PLL_SRC_CFG & 0x1)
      return 24000000;
    return 0;

  default:
    return 0;
  }

  if (0 == div)
    div = 1;
  return top_clk / div;
}

/**
 * @brief   Set the clock.
 * @param[in] idx index to clock source
 *            hz  clock in hz
 *
 * @api
 */
void cpm_set_clock(hs_clk_t idx, uint32_t hz) {
    uint32_t mclk = get_master_clock();
	uint8_t div = DIV_ROUND(mclk, hz);
	switch (idx) {
	case HS_CPU_CLK:
      HS_PSO->CPU_CFG = CPM_BFINS(DIV_COEFF, div, HS_PSO->CPU_CFG) | CPM_BIT(DIV_EN);
	  break;
    case HS_RAM_CLK:
      HS_PSO->CPU_CFG = CPM_BFINS(RAM_DIV_COEFF, div, HS_PSO->CPU_CFG) | CPM_BIT(DIV_EN);
	  break;
	case HS_AHB_CLK:
      HS_PSO->CPU_CFG = CPM_BFINS(AHB_DIV_COEFF, div, HS_PSO->CPU_CFG) | CPM_BIT(DIV_EN);
      break;
	case HS_APB_CLK:
	  HS_PSO->APB_CFG = CPM_BFINS(APB_DIV_COEFF, div/2, HS_PSO->APB_CFG) | CPM_BIT(DIV_EN);
	  pso_reg_update();
      return;
  case HS_SF_CLK:
      HS_PSO->SFLASH_CFG = (div << 8) | 0x1d;  //delay bypass
      __WFD();
	  break;
	default: return;
	}
	__WFD();
    return;
}


/**
 * @brief   Set master clock source from system PLL
 *
 * @api
 */
void cpm_switch_to_pll(void) {
	HS_PMU_CPM->PLL_SRC_CFG = 0x1;
	__WFD();
}

/**
 * @brief   Set master clock source from main crystal
 *
 * @api
 */
void cpm_switch_to_xtal(void) {
	HS_PMU_CPM->PLL_SRC_CFG = 0x0;
	__WFD();
}

/**
 * @brief   Set sf clock delay
 * @param[in] half_cycle  = 1/2 * 1/PLL_CLK 0:bypass 1~8: n half cycle
 *           half_cycle between 0 to 8
 * @api
 */
void cpm_set_sf_clock_delay(uint8_t half_cycle) {
	uint32_t val = half_cycle;
	if (half_cycle == 0)
		HS_PSO->SFLASH_CFG |= CPM_BIT(SF_DELAY_BYPASS);
	else if (half_cycle < 9) {
		HS_PSO->SFLASH_CFG &= (~(CPM_BIT(SF_DELAY_BYPASS)));
		HS_PSO->SFLASH_CFG = CPM_BFINS(SF_DELAY_OFFSET, val-1, HS_PSO->SFLASH_CFG);
	}else{
		/* if half_cycle>8 defalt 3 */
		HS_PSO->SFLASH_CFG &= (~(CPM_BIT(SF_DELAY_BYPASS)));
		HS_PSO->SFLASH_CFG = CPM_BFINS(SF_DELAY_OFFSET, 3, HS_PSO->SFLASH_CFG);
	}
}

/**
 * @brief   Set sd sample clock and driver clock delay and mclk phase
 * @param[in]  sd_dev_clk
 *
 * @api
 */
void cpm_set_sd_dev_clock(sd_dev_clk_t *sd_dev_clk) {

	int32_t drv_val = sd_dev_clk->drv_half_cycles;
	int32_t smp_val = sd_dev_clk->smp_half_cycles;

	if (sd_dev_clk->mclk_phase == SD_CLK_PHASE_0)
		HS_PSO->SD_DRV_CFG &= (~(CPM_BIT(SD_DEV_MCLK_INV)));
	else if(sd_dev_clk->mclk_phase == SD_CLK_PHASE_180)
		HS_PSO->SD_DRV_CFG |= CPM_BIT(SD_DEV_MCLK_INV);

	if (smp_val == -1)
		HS_PSO->SD_DRV_CFG |= CPM_BIT(SD_DEV_SMP_DELAY_BYPASS);
	else if (smp_val < 8) {
		HS_PSO->SD_DRV_CFG &= (~(CPM_BIT(SD_DEV_SMP_DELAY_BYPASS)));
		HS_PSO->SD_DRV_CFG = CPM_BFINS(SD_DEV_SMP_DELAY_OFFSET, smp_val-1, HS_PSO->SD_DRV_CFG);
	}else{
		/* if half_cycle>8 defalt bypass */
		HS_PSO->SD_DRV_CFG |= CPM_BIT(SD_DEV_SMP_DELAY_BYPASS);
	}

	if (drv_val == -1)
		HS_PSO->SD_DRV_CFG |= CPM_BIT(SD_DEV_DRV_DELAY_BYPASS);
	else if (drv_val < 8) {
		HS_PSO->SD_DRV_CFG &= (~(CPM_BIT(SD_DEV_DRV_DELAY_BYPASS)));
		HS_PSO->SD_DRV_CFG = CPM_BFINS(SD_DEV_DRV_DELAY_OFFSET, drv_val-1, HS_PSO->SD_DRV_CFG);
	}else{
		/* if half_cycle>8 defalt 3 */
		HS_PSO->SD_DRV_CFG &= (~CPM_BIT(SD_DEV_DRV_DELAY_BYPASS));
		HS_PSO->SD_DRV_CFG = CPM_BFINS(SD_DEV_DRV_DELAY_OFFSET, 3, HS_PSO->SD_DRV_CFG);
	}

  pso_reg_update();
}

__ONCHIP_CODE__ void cpm_delay_us(uint32_t us)
{
  //chSysPolledDelayX(us * (cpm_get_clock(HS_CPU_CLK) / 1000000));
  chSysPolledDelayX(us * (CPU_DEFAULT_CLOCK / 1000000));
}

void cpm_reset_system(void)
{
  //cpmResetAPB();
  __hal_set_bitsval(HS_PMU->ANA_CON, 27, 28, 0);
  
  if(CPM_GetXtalSel())
    __hal_set_bitval(HS_ANA->SDM_CFG, 16, 0);
  else
    __hal_set_bitval(HS_ANA->SDM_CFG, 16, 1);

  __hal_set_bitsval(HS_PMU->PADC_CON[0], 5, 10, 5);
}

/**
 * @brief   Set sflash inner or outter
 * @param[in]  location
 *
 * @api
 */
void sf_inner_select(sf_loc_t loc) {
	if (loc == SF_LOC_INNER)
		HS_PMU->BASIC |= (1<<3);
	else if (loc == SF_LOC_OUTER)
		HS_PMU->BASIC &= ~(1<<3);

    pmu_reg_update();
}

void pmu_xclkon_insleep(int on){
  if(on)
    HS_PMU->BASIC &= ~(1<<2);
  else
    HS_PMU->BASIC |= (1<<2);
}

uint32_t pmu_get_powerPinStatus(void)
{
  return ((HS_PMU->PMU_CLOCK_MUX >> 1) & 1);
}

void pmu_deep_sleep(void)
{
  __hal_set_bitsval(HS_PMU->BASIC, 4, 5, HS_PIN_DEEP_SLEEP_WAKEUP);
  pmu_reg_update();

  HS_PMU->PSO_PM_CON = 0x40080402;
}

void pmu_chip_poweroff(void)
{
  HS_PMU->BASIC |= (1u << 7);
  HS_PMU->CHIP_PM_PIN |= (1u << 13);

  pmu_reg_update();  
}

uint32_t pmu_ana_get(uint32_t start, uint32_t end)
{
  volatile uint32_t *ptr = &HS_ANA->COMMON_PACK[0];
  uint32_t val, bit_len, low_len, word_width = sizeof(uint32_t) * 8;

  bit_len = end - start + 1;  
  if(bit_len > word_width)
    return 0;

  low_len = word_width - start % word_width;
  low_len = low_len > bit_len ? bit_len : low_len;
  
  ptr += start / word_width;
  val = (*ptr >> (start % word_width) ) & ((1u << low_len) - 1);
  
  if(bit_len > low_len)
  {
    ptr ++;
    val |= (*ptr & ((1 << (bit_len -  low_len)) - 1)) << low_len;
  }

  return val;
}

void pmu_ana_set(uint32_t start, uint32_t end, uint32_t val)
{
  volatile uint32_t *ptr = &HS_ANA->COMMON_PACK[0];
  uint32_t bit_len, low_offset, low_len, word_width = sizeof(uint32_t) * 8;

  bit_len = end - start + 1;  
  if(bit_len > word_width)
    return ;

  low_offset = start % word_width;
  low_len = word_width - low_offset;

  low_len = low_len > bit_len ? bit_len : low_len;
  
  ptr += start / word_width;
  __hal_set_bitsval(*ptr, low_offset, (low_offset + low_len - 1), val);

  if(bit_len > low_len)
  {
    ptr ++;
    val >>= low_len;

    __hal_set_bitsval(*ptr, 0, (bit_len - low_len), val);
  }
}

static const short rxadc_rc_tab[16] = {356, 346, 335, 324, 313, 302, 292, 281, 270, 259, 248, 238, 227, 216, 205, 194}; //4b'1000
static const short auadc_rc_tab[16] = {356, 346, 335, 324, 313, 302, 292, 281, 270, 259, 248, 238, 227, 216, 205, 194}; //4b'1000
static const short txdac_rc_tab[16] = {378, 360, 342, 324, 306, 288, 270, 252, 234, 216, 198, 180, 162, 144, 126, 108}; //4b'0110
static const short rxfil_rc_tab[ 8] = {360, 330, 300, 270, 240, 210, 180, 150}; //3b'011? b'111
static const short rxtia_rc_tab[ 8] = {405, 371, 337, 303, 270, 236, 202, 168}; //        b'100

/* rc table search policy by lihongwei */
static int _search_rc_tab(short value, const short *table, int size)
{
  int i = 0;
  do{
    if(table[i] < value)
      break;
  }while(++i < size);
  
  if(i == size)
    return i - 1;
  
  if(i == 0)
    return i;
  
  if(table[i - 1] - value < value - table[i])
    return i - 1;
  
  return i;
}

/* RC calibration for rxadc, auadc, txdac, rxfil, rxtia */
void pmu_cali_rc(void)
{
  if (m_pmu_tune_shadow.valid_rc == 0) {
    uint32_t saved0, saved1, saved2;
    short rc_cnt;

    saved0 = HS_ANA->PD_CFG[0];
    saved1 = HS_PMU->ANA_CON;
    saved2 = HS_ANA->PD_CFG[2];

    /* over voltage protect */
    HS_ANA->REGS.RXADC_SHRT = 1;

    /* [8]pd_ldo_v1p5_ana, [7]pd_ldp_v1p5_adda */
    HS_PMU->ANA_CON &= ~((1 << 8) | (1 << 7));
    /* [20][4]pd_rxadc_i [18][2]pd_rxadc_biasgen */
    HS_ANA->PD_CFG[2] &= ~((1 << 20) | (1 << 18) | (1 << 4) | (1 << 2));
    /* [12]pd_rxadc_dacbias */
    HS_ANA->PD_CFG[0] &= ~(1 << 12);

    /* wait 20us for ldo output stability */
    cpm_delay_us(20);

    /* start RC calibration to get rc_cnt = t1+t2 */
    HS_ANA->RC_CALIB_CNS = (1 << 0); //sw set, hw clear
    while (HS_ANA->RC_CALIB_CNS & (1 << 0))
      ;
    rc_cnt = (HS_ANA->RC_CALIB_CNS >> 16) / 2;

    /* post */
    HS_ANA->REGS.RXADC_SHRT = 0;
    HS_ANA->PD_CFG[0] = saved0;
    HS_PMU->ANA_CON   = saved1;
    HS_ANA->PD_CFG[2] = saved2;

    m_pmu_tune_shadow.rc_rxadc = _search_rc_tab(rc_cnt, rxadc_rc_tab, 16);
    m_pmu_tune_shadow.rc_auadc = _search_rc_tab(rc_cnt, auadc_rc_tab, 16);
    m_pmu_tune_shadow.rc_txdac = _search_rc_tab(rc_cnt, txdac_rc_tab, 16);
    m_pmu_tune_shadow.rc_rxfil = _search_rc_tab(rc_cnt, rxfil_rc_tab, 8);
    m_pmu_tune_shadow.rc_rxtia = _search_rc_tab(rc_cnt, rxtia_rc_tab, 8);
    m_pmu_tune_shadow.valid_rc = 1;
  }

  /* [7:4]rxadc_rctune */
  __hal_set_bitsval(HS_ANA->COMMON_CFG[0], 4, 7,   m_pmu_tune_shadow.rc_rxadc);
  /* reg<16:13>RCtune         =auadc_rctune */
  HS_ANA->REGS.RC_TUNE =                           m_pmu_tune_shadow.rc_auadc;
  /* reg<142:139>txdac_bw_cal =txdac_rctune */
  HS_ANA->REGS.TXDAC_BW_CAL =                      m_pmu_tune_shadow.rc_txdac;
  /* [6:4]ctune_fil           =rxfil_rctune */
  __hal_set_bitsval(HS_ANA->RX_FIL_CFG, 4, 6,      m_pmu_tune_shadow.rc_rxfil);
  /* [18:16]cf_tune           =rxtia_rctune */
  __hal_set_bitsval(HS_ANA->COMMON_CFG[1], 16, 18, m_pmu_tune_shadow.rc_rxtia);
}

/* low noise LDO calibration for lobuf, xtal, vco */
void pmu_cali_ldo(void)
{
  if (m_pmu_tune_shadow.valid_ldo == 0) {
    uint32_t saved0, saved1, saved2, ctrl;

    saved0 = HS_PMU->ANA_CON;
    saved1 = HS_ANA->PD_CFG[1];
    saved2 = HS_ANA->DBG_IDX;

    /* [23]sys_pll_gt_32m, required by ldo_calib_comp_ts; [8]pd_ldo_v1p5_ana by yangguang */
    HS_PMU->ANA_CON &= ~((1 << 23) | (1 << 8));

    /* [22][6]pd_ldo_v1p3_lobuf */
    /* pd_ldoxtal is on by fsm */
    /* [28][12]pd_bt_ldovco */
    HS_ANA->PD_CFG[1] &= ~((1 << 28) | (1 << 22) | (1 << 12) | (1 << 6));

    HS_ANA->REGS.PD_LDOCALI_CAMP = 0;
    HS_ANA->REGS.LDOCALI_SWAP = 0;

    /* wait 80us for ldo output stability */
    cpm_delay_us(100);

    HS_ANA->LDO_CALIB_CNS = 0 << 8; //LDO settle time =10us + counter in 32MHz
    /* enable LDO calibration to control reg<268:267> reg<257:256> reg<45:44> by fsm */
    HS_ANA->LDO_CALIB_CNS |= (1 << 0); //ldo_calib_en: 1=fsm
    /* wait 2~3 cycles in 32MHz, so hw will clear ldo_calib_done */
    cpm_delay_us(2);
    while ((HS_ANA->LDO_CALIB_CNS & (1 << 31)) == 0) //ldo_calib_done
      ;

    /* sw delay for 10us*4step*3ldo to avoid hw bug */
    cpm_delay_us(120);

    /* duplicate the LDO calibration values from fsm to reg, in case of cali module ugly output on 32MHz reset */
    HS_ANA->DBG_IDX = 7;
    ctrl = HS_ANA->DBG_RDATA;

    /* post */
    HS_ANA->LDO_CALIB_CNS &= ~(1 << 0); //ldo_calib_en: 0=reg
    HS_ANA->REGS.PD_LDOCALI_CAMP = 1;
    HS_PMU->ANA_CON   = saved0;
    HS_ANA->PD_CFG[1] = saved1;
    HS_ANA->DBG_IDX   = saved2;

    m_pmu_tune_shadow.ldo_lobuf = (ctrl >> 4) & 0x3;
    m_pmu_tune_shadow.ldo_xtal  = (ctrl >> 2) & 0x3;
    m_pmu_tune_shadow.ldo_vco   = (ctrl >> 0) & 0x3;
    m_pmu_tune_shadow.valid_ldo = 1;
  }

  /* reg<45:44> */
  HS_ANA->REGS.LDO_LOBUF_CTRL = m_pmu_tune_shadow.ldo_lobuf;
  /* reg<257:256> */
  HS_ANA->REGS.CON_LDO_XTAL   = m_pmu_tune_shadow.ldo_xtal;
  /* reg<268:267> */
  HS_ANA->REGS.CON_LDO_VCO    = m_pmu_tune_shadow.ldo_vco;
}

/** @} */

