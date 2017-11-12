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
 * @file    hs6601/cpm_lld.h
 * @brief   clock & pmu helper driver header.
 *
 * @addtogroup HAL
 * @{
 */
#ifndef _HAL_CPM_H_
#define _HAL_CPM_H_

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/
#define __SYSTEM_CLOCK__        1

#if __SYSTEM_CLOCK__ == 1
#define  CPU_DEFAULT_CLOCK      192000000
#define  AHB_DEFAULT_CLOCK      96000000
#define  RAM_DEFAULT_CLOCK      96000000
#define  APB_DEFAULT_CLOCK      96000000
#elif __SYSTEM_CLOCK__ == 2
#define  CPU_DEFAULT_CLOCK      96000000
#define  AHB_DEFAULT_CLOCK      96000000
#define  RAM_DEFAULT_CLOCK      96000000
#define  APB_DEFAULT_CLOCK      96000000
#elif __SYSTEM_CLOCK__ == 3
#define  CPU_DEFAULT_CLOCK      192000000
#define  AHB_DEFAULT_CLOCK      48000000
#define  RAM_DEFAULT_CLOCK      48000000
#define  APB_DEFAULT_CLOCK      48000000
#elif __SYSTEM_CLOCK__ == 4
#define  CPU_DEFAULT_CLOCK      48000000
#define  AHB_DEFAULT_CLOCK      48000000
#define  RAM_DEFAULT_CLOCK      48000000
#define  APB_DEFAULT_CLOCK      48000000
#endif

#define  SF_DEFAULT_CLOCK       96000000
#define  SD_DEFAULT_CLOCK       48000000
#define  TIMER_DEFAULT_CLOCK    8000000
#define  I2C_DEFAULT_CLOCK      12000000
#define  SPI1_DEFAULT_CLOCK     48000000
#define  UART_DEFAULT_CLOCK     (921600 * 4)

#define  PLL_CLOCK_192M         192000000
#define  XCLK_CLOCK_16MHz       16000000
#define  XCLK_CLOCK_24MHz       24000000

typedef enum {
	SF_LOC_INNER = 0x1,
	SF_LOC_OUTER,
}sf_loc_t;

typedef enum {
	SD_CLK_PHASE_0,
	SD_CLK_PHASE_180,
}sd_dev_mclk_phase_t;

typedef struct {
	sd_dev_mclk_phase_t mclk_phase;
	int8_t     smp_half_cycles;
	int8_t     drv_half_cycles;
}sd_dev_clk_t;

typedef struct {
	uint8_t  num;
	uint8_t  edge; //0-posedge,1-negedge
	bool     hold;
	uint8_t  level; //0 - active low, 1 - active high
	bool     reset_en;
	uint8_t  on_cycles;  //power on delay
    uint8_t  reset_cycles; //power off delay
}power_pin_conf_t;

typedef struct {
	uint32_t pll_clk;
	uint32_t xtal_clk;
}sys_clk_t;

typedef enum {
  HS_CPU_CLK,
  HS_AHB_CLK,
  HS_APB_CLK,
  HS_RAM_CLK,
  HS_SF_CLK,
  HS_USB_CLK,
  HS_SD_CLK,
  HS_TIM0_CLK,
  HS_TIM1_CLK,
  HS_TIM2_CLK,
  HS_UART0_CLK,
  HS_UART1_CLK,
  HS_I2C0_CLK,
  HS_LPO_CLK,
  HS_SPI0_CLK, //same with APB clock
  HS_SPI1_CLK,
  HS_CODEC_MCLK, //24MHz alway
  HS_BTBB_CLK,   //24MHz always

  HS_RTC_CLK,
  HS_WDT_CLK,
  HS_PLL_CLK,    //pll clock 96M or 128M
  HS_XTAL_CLK,   //xtal in 16M or 32M
} hs_clk_t;

typedef bool            bool_t;

#define CPM_DIV_COEFF_Pos           8
#define CPM_DIV_COEFF_Msk           (0x1F << CPM_DIV_COEFF_Pos)

#define CPM_AHB_DIV_COEFF_Pos       16
#define CPM_AHB_DIV_COEFF_Msk       (0x1F << CPM_AHB_DIV_COEFF_Pos)

#define CPM_RAM_DIV_COEFF_Pos       24
#define CPM_RAM_DIV_COEFF_Msk       (0x1F << CPM_RAM_DIV_COEFF_Pos)

#define CPM_APB_DIV_COEFF_Pos       8
#define CPM_APB_DIV_COEFF_Msk       (0x1F << CPM_APB_DIV_COEFF_Pos)

#define CPM_SF_DIV_COEFF_Pos        8
#define CPM_SF_DIV_COEFF_Msk        (0x0F << CPM_SF_DIV_COEFF_Pos)

#define CPM_SF_DELAY_OFFSET_Pos     16
#define CPM_SF_DELAY_OFFSET_Msk     (0x07 << CPM_SF_DELAY_OFFSET_Pos)
#define CPM_SF_DELAY_BYPASS_Pos     3

#define CPM_SD_DEV_DRV_DELAY_BYPASS_Pos   1
#define CPM_SD_DEV_DRV_DELAY_OFFSET_Pos   8
#define CPM_SD_DEV_DRV_DELAY_OFFSET_Msk   (0x07 << CPM_SD_DEV_DRV_DELAY_OFFSET_Pos)
#define CPM_SD_DEV_SMP_DELAY_BYPASS_Pos   2
#define CPM_SD_DEV_SMP_DELAY_OFFSET_Pos   16
#define CPM_SD_DEV_SMP_DELAY_OFFSET_Msk   (0x07 << CPM_SD_DEV_SMP_DELAY_OFFSET_Pos)
#define CPM_SD_DEV_MCLK_INV_Pos           0

#define CPM_I2C_DIV_COEFF_Pos       8
#define CPM_I2C_DIV_COEFF_Msk       (0x3F << CPM_I2C_DIV_COEFF_Pos)

#define CPM_UART_DIV_COEFF_FRC_Pos  16
#define CPM_UART_DIV_COEFF_FRC_Msk  (0x3F << CPM_UART_DIV_COEFF_FRC_Pos)
#define CPM_UART_DIV_COEFF_FRC_Len  6
#define CPM_UART_DIV_COEFF_INT_Pos  8
#define CPM_UART_DIV_COEFF_INT_Msk  (0x3F << CPM_UART_DIV_COEFF_INT_Pos)

#define CPM_DIV_EN_Pos              0

/*
 * Bit manipulation macros
 */
#define CPM_BIT(name)				\
  (1 << CPM_##name##_Pos)
/* bit field generate */
#define CPM_BF(name,value)			\
  (((value) << CPM_##name##_Pos) & CPM_##name##_Msk)
/* bit field extract from value */
#define CPM_BFEXT(name,value)			\
  (((value) & CPM_##name##_Msk) >> CPM_##name##_Pos)
/* bit field insert value into old */
#define CPM_BFINS(name,value,old)		\
  (((old) & ~CPM_##name##_Msk) | CPM_BF(name,value))

__STATIC_INLINE bool_t CPM_DIV_EN(uint32_t value)
{
  return value & CPM_BIT(DIV_EN) ? TRUE : FALSE;
}

#define CPM_MOD_RST   (1 << 4)
#define CPM_MOD_GATE  (1 << 3)
#define CPM_BUS_RST   (1 << 2)
#define CPM_BUS_GATE  (1 << 1)
#define CPM_CLK_DEN   (1 << 0)

#define RESET_STATUS_POWER_UP (1<<1)
#define RESET_STATUS_SYS      (1<<2)
#define RESET_STATUS_WDT      (1<<0)

#define PMU_POWER_ON    0x1
#define PMU_POWER_OFF   0x2

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/
/**
 * @brief   Generate software reset mcu chip
 *
 * @api
 */
#define cpmResetMCU() \
do { \
  HS_PMU->OPT_RESET_CON &= ~(1 << 25); \
  HS_PMU->OPT_RESET_CON |= (1 << 25); \
} while (0)

/**
 * @brief   Generate software reset cpu core
 *
 * @api
 */
#define cpmResetCPU() \
do { \
  HS_PSO->CPU_CFG &= ~(1 << 7); \
  HS_PSO->CPU_CFG |= (1 << 7); \
} while (0)


#define cpmResetAPB() \
do { \
  HS_PSO->APB_CFG &= ~(1 << 2); \
  HS_PSO->APB_CFG |= (1 << 2); \
  __NOP(); __NOP(); __NOP(); __NOP(); \
  __NOP(); __NOP(); __NOP(); __NOP(); \
} while (0)


/**
 * @brief   Generate software reset pso domain ip
 *
 * @api
 */
#define cpmResetPSO() \
do { \
  HS_PMU->OPT_RESET_CON &= ~(1 << 26); \
  HS_PMU->OPT_RESET_CON |= (1 << 26); \
} while (0)

/**
 * @name    SF peripherals specific CPM operations
 * @{
 */
/**
 * @brief   Enables the SF peripheral clock.
 *
 * @api
 */
#define cpmEnableSF() \
  do {\
    HS_PSO->SFLASH_CFG &= ~(CPM_BUS_GATE); \
    HS_PSO->SFLASH_CFG |= (CPM_CLK_DEN); \
  }while(0)

/**
 * @brief   Disables the SF peripheral clock.
 *
 * @api
 */
#define cpmDisableSF() \
  do {\
    HS_PSO->SFLASH_CFG |= (CPM_BUS_GATE); \
    HS_PSO->SFLASH_CFG &= ~(CPM_CLK_DEN); \
  }while(0)

/**
 * @brief   Resets the SF peripheral.
 *
 * @api
 */
#define cpmResetSF() \
do { \
  HS_PSO->SFLASH_CFG &= ~(CPM_BUS_RST); \
  HS_PSO->SFLASH_CFG |= (CPM_BUS_RST); \
} while (0)
/** @} */

/**
 * @name    I2S peripherals specific CPM operations
 * @{
 */
/**
 * @brief   Enables the I2S peripheral clock.
 *
 * @api
 */
#define cpmEnableI2S() HS_PSO->I2S_CFG &= ~(CPM_BUS_GATE)

/**
 * @brief   Disables the I2S peripheral clock.
 *
 * @api
 */
#define cpmDisableI2S() HS_PSO->I2S_CFG |= (CPM_BUS_GATE)

/**
 * @brief   Resets the I2S peripheral.
 *
 * @api
 */
#define cpmResetI2S() \
do { \
  HS_PSO->I2S_CFG &= ~(CPM_BUS_RST); \
  HS_PSO->I2S_CFG |= (CPM_BUS_RST); \
} while (0)
/** @} */

/**
 * @name    CODEC peripherals specific CPM operations
 * @{
 */
/**
 * @brief   Enables the CODEC peripheral clock and 24MHz,12MHz from analog
 *
 * @api
 */
#define cpmEnableCODEC() \
do { \
  HS_PSO->CODEC_CFG &= ~(CPM_BUS_GATE); \
  HS_PSO->CODEC_CFG &= ~((1<<3) | (1 <<4));    \
  HS_PMU->ANA_CON &= ~((1 << 22) | (1 << 21)); \
} while (0)

/**
 * @brief   Disables the CODEC peripheral clock and 12MHz from analog
 *
 * @api
 */
#define cpmDisableCODEC() \
do { \
  /* clocks & power */ \
  HS_PSO->CODEC_CFG |= (CPM_BUS_GATE); \
  HS_PSO->CODEC_CFG |= ((1<<3) | (1 <<4)); \
} while (0)

/**
 * @brief   Resets the CODEC peripheral.
 *
 * @api
 */
#define cpmResetCODEC() \
do { \
  HS_PSO->CODEC_CFG &= ~(CPM_BUS_RST); \
  HS_PSO->CODEC_CFG |= (CPM_BUS_RST); \
} while (0)
/** @} */

/**
 * @name    BTPHY peripherals specific CPM operations
 * @{
 */
/**
 * @brief   Enables the BTPHY peripheral clock and 48,32,24MHz from analog
 *
 * @api
 */
#define cpmEnableBTPHY() \
do { \
  /* clocks to BTPHY:        48M, 32M, 24M */ \
  HS_PSO->BTPHY_CFG &= ~(CPM_BUS_GATE | (1 << 12) | (1 << 10) | (1 << 8)); \
  /* clocks from system PLL: 48M, 32M, 24M_dig, 24M_ana */ \
  HS_PMU->ANA_CON &= ~((1 << 24) | (1 << 23) | (1 << 22) | (1 << 21)); \
  /* FIXME: [28][12]pd_bt_ldovco_flag/reg */ \
  HS_ANA->PD_CFG[1] &= ~((1 << 28) | (1 << 12)); \
  /* [21]pd_rfsctgm [20]pd_dcoc by xuebaiqing; for btrx & fm by wangjianting */ \
  HS_ANA->PD_CFG[0] &= ~((1 << 21) | (1 << 20));  \
  /* [4]pd_bt_synth_vc_det_reg, no its flag by xuebaiqing */ \
  HS_ANA->PD_CFG[1] &= ~(1 << 4); \
  /* [12]pd_rxadc_dacbias  */ \
  HS_ANA->PD_CFG[0] &= ~(1 << 12);  \
  /* [15]pd_bt_mmd_fm by wangxin  */ \
  HS_ANA->PD_CFG[0] &= ~(1 << 15);  \
  /* powers: [16]pd_topldo_v1p8 [13]rfldo_v2p8 [10]pd_v2p5ldo [9-7]ldo_v1p5_rf/ana/adda */ \
  HS_PMU->ANA_CON &= ~((1 << 16) | (1 << 13) | (1 << 10) | (1 << 9) | (1 << 8) | (1 << 7)); \
  /* ?SAR:   [30][18]cali_pkdect_flag/reg b'0=no cali */                        \
  /* ?SAR:   [29]cali_rxatten b'0=no cali */                                    \
  /* ?       [27]gsel_txdect b'1=high gain, obsoleted */                        \
  /* TIA:    [25]fre_sel b'1 [23]lp_cp_sel b'1=complex filter; b'0=LPF for calibration */ \
  /* TIA:    [22]modsel_tia b'1=BT b'0=FM */                            \
  /* ?       [21]b'0 */                                                 \
  /* RFPLL:  [20]b'0 */                                                 \
  /* LNA:    [15:14]in_lna b'10= */                                     \
  /* SAR:    [11]gain_agcamp b'0=x1 */                                  \
  /* ?SAR:   [10]seliq_pkdif b'0=I */                                   \
  /* RFPLL:  [8]bt_cp_sw_lvl_en b'1 */                                  \
  /* RXADC:  [7:4]rxadc_rctune: b1000 */                                \
  HS_ANA->COMMON_CFG[0] = (0 << 30) | (0 << 29) | (1 << 25) | (1 << 23) | (1 << 22) | (2 << 14) | (1 << 8) | (8 << 4); \
  /*         [18:16]sel_ifagc: obsoleted */ \
  /* Filter: [13:12]swap_fil b'00=complex filter */ \
  /*         [9:8]gtune_fil b'000 */ \
  /*         [6:4]ctune_fil b'011 */ \
  /*         [0]modsel_fil  b'1=BT b'0=FM */ \
  HS_ANA->RX_FIL_CFG = (0 << 12) | (0 << 8) | (3 << 4) | (1 << 0); \
  /* TIA:    [18:16]cf_tune b'100 */ \
  /* GP-ADC: [8:4]ana_test_en b'00000 */ \
  /* LNA:    [1:0]rxlna_vbn b'01 */ \
  HS_ANA->COMMON_CFG[1] = (4 << 16) | (1 << 0); \
  /* sanity set in case of reset out of fm mode */ \
  HS_BTPHY->FM_MODE = 0; \
  HS_BTPHY->IF_REG = 0x300; \
  HS_BTPHY->DCNOTCH_K_SEL = 3; \
  HS_ANA->VTRACK_CFG = (0 << 4) | (0x4 << 0); \
  HS_ANA->VCO_AFC_CFG[0] &= ~(1 << 12); \
} while (0)

/**
 * @brief   Disables the BTPHY peripheral clock.
 *
 * @api
 */
#define cpmDisableBTPHY() \
do { \
  HS_PMU->ANA_CON |= ((1 << 13) | (1 << 9) | (1 << 8) | (1 << 7));	      \
  /* [28][12]pd_bt_ldovco_flag/reg */ \
  HS_ANA->PD_CFG[1] |= ((1 << 28) | (1 << 12)); \
  /* [12]pd_rxadc_dacbias */ \
  HS_ANA->PD_CFG[0] |= (1 << 12); \
  HS_PSO->BTPHY_CFG |= (CPM_BUS_GATE | (1 << 12) | (1 << 10) | (1 << 8)); \
} while (0)

/**
 * @brief   Resets the BTPHY peripheral.
 *
 * @api
 */
#define cpmResetBTPHY() \
do { \
  HS_PSO->BTPHY_CFG &= ~((CPM_BUS_RST) | (1 << 9) | (1 << 11));  \
  HS_PSO->BTPHY_CFG |= ((CPM_BUS_RST) | (1 << 9) | (1 << 11));   \
} while (0)
/** @} */

/**
 * @name    FM peripherals specific CPM operations
 * @{
 */
/**
 * @brief   Enables the FM peripheral clock and 48,32,24MHz from analog
 *
 * @api
 */
#define cpmEnableFM() \
do { \
  /* clocks to BTPHY:        48M, 32M, 24M */ \
  HS_PSO->BTPHY_CFG &= ~(CPM_BUS_GATE | (1 << 12) | (1 << 10) | (1 << 8)); \
  /* clocks from system PLL: 48M, 32M, 24M_dig, 24M_ana */ \
  HS_PMU->ANA_CON &= ~((1 << 24) | (1 << 23) | (1 << 22) | (1 << 21)); \
  /* powers: [16]pd_topldo_v1p8 [13]rfldo_v2p8 [10]pd_v2p5ldo [9-7]ldo_v1p5_rf/ana/adda */ \
  HS_PMU->ANA_CON &= ~((1 << 16) | (1 << 13) | (1 << 10) | (1 << 9) | (1 << 8) | (1 << 7)); \
} while (0)

/**
 * @brief   Disables the FM peripheral clock.
 *
 * @api
 */
#define cpmDisableFM() \
do { \
} while (0)

/**
 * @brief   Resets the FM peripheral.
 *
 * @api
 */
#define cpmResetFM()
/** @} */

/**
 * @name    WDT peripherals specific CPM operations
 * @{
 */
/**
 * @brief   Enables the WDT peripheral clock.
 *
 * @api
 */
#define cpmEnableWDT() HS_PSO->WDT_CFG &= ~(CPM_BUS_GATE)

/**
 * @brief   Disables the WDT peripheral clock.
 *
 * @api
 */
#define cpmDisableWDT() HS_PSO->WDT_CFG |= (CPM_BUS_GATE)

/**
 * @brief   Resets the WDT peripheral.
 *
 * @api
 */
#define cpmResetWDT()
/** @} */

/**
 * @name    ADC peripherals specific CPM operations
 * @{
 */
/**
 * @brief   Enables the ADC0 peripheral clock.
 *
 * @api
 */
#define cpmEnableADC0()

/**
 * @brief   Disables the ADC0 peripheral clock.
 *
 * @api
 */
#define cpmDisableADC0()

/**
 * @brief   Resets the ADC0 peripheral.
 *
 * @api
 */
#define cpmResetADC0()
/** @} */

/**
 * @name    DMA peripheral specific CPM operations
 * @{
 */
/**
 * @brief   Enables the DMA peripheral clock.
 *
 * @api
 */
#define cpmEnableDMA() HS_PSO->AHB_GATE_CFG &= ~(1 << 2)

/**
 * @brief   Disables the DMA peripheral clock.
 *
 * @api
 */
#define cpmDisableDMA() HS_PSO->AHB_GATE_CFG |= (1 << 2)

/**
 * @brief   Resets the DMA peripheral.
 *
 * @api
 */
#define cpmResetDMA() \
do { \
  HS_PSO->AHB_GATE_CFG &= ~(1 <<3); \
  HS_PSO->AHB_GATE_CFG |= (1 << 3); \
} while (0)
/** @} */

/**
 * @name    GPIO peripheral specific CPM operations
 * @{
 */
/**
 * @brief   Enables the GPIO peripheral clock.
 *
 * @api
 */
#define cpmEnableGPIO() HS_PSO->AHB_GATE_CFG &= ~(1 << 0);

/**
 * @brief   Disables the GPIO peripheral clock.
 *
 * @api
 */
#define cpmDisableGPIO() HS_PSO->AHB_GATE_CFG |= (1 << 0);

/**
 * @brief   Resets the GPIO peripheral.
 *
 * @api
 */
#define cpmResetGPIO() \
do { \
  HS_PSO->AHB_GATE_CFG &= ~(1 <<1); \
  HS_PSO->AHB_GATE_CFG |= (1 << 1); \
} while (0)
/** @} */

/**
 * @name    I2C peripherals specific CPM operations
 * @{
 */
/**
 * @brief   Enables the I2C0 peripheral clock.
 *
 * @api
 */
#define cpmEnableI2C0() \
  do{\
    HS_PSO->I2C0_CFG &= ~(CPM_BUS_GATE);\
    HS_PSO->I2C0_CFG |= CPM_CLK_DEN;\
  }while(0)
  

/**
 * @brief   Disables the I2C0 peripheral clock.
 *
 * @api
 */
#define cpmDisableI2C0() \
  do {\
    HS_PSO->I2C0_CFG |= (CPM_BUS_GATE);\
    HS_PSO->I2C0_CFG &= ~(CPM_CLK_DEN);\
  }while(0)

/**
 * @brief   Resets the I2C0 peripheral.
 *
 * @api
 */
#define cpmResetI2C0() \
do { \
  HS_PSO->I2C0_CFG &= ~(CPM_MOD_RST | CPM_BUS_RST); \
  HS_PSO->I2C0_CFG |= (CPM_MOD_RST | CPM_BUS_RST); \
} while (0)
/** @} */

/**
 * @name    OTG peripherals specific CPM operations
 * @{
 */
/**
 * @brief   Enables the USB peripheral clock.
 *
 * @api
 */
#define cpmEnableUSB() \
do { \
  HS_PSO->USB_CFG &= ~(CPM_MOD_GATE | CPM_BUS_GATE); \
  /* 48MHz shared with BT */ \
  HS_PMU->ANA_CON &= ~(1 << 24); \
} while (0);

/**
 * @brief   Disables the USB peripheral clock.
 *
 * @api
 */
#define cpmDisableUSB() \
do { \
  HS_PSO->USB_CFG |= (CPM_MOD_GATE | CPM_BUS_GATE); \
} while (0);

/**
 * @brief   Resets the USB peripheral.
 *
 * @api
 */
#define cpmResetUSB() \
do { \
  HS_PSO->USB_CFG &= ~(CPM_MOD_RST | CPM_BUS_RST); \
  HS_PSO->USB_CFG |= (CPM_MOD_RST | CPM_BUS_RST); \
} while (0)
/** @} */

/**
 * @name    SDHC peripheral specific CPM operations
 * @{
 */
/**
 * @brief   Enables the SDHC peripheral clock.
 *
 * @api
 */
#define cpmEnableSDHC() \
  do {\
    HS_PSO->SD_CFG &= ~(CPM_BUS_GATE); \
    HS_PSO->SD_CFG |= (CPM_CLK_DEN); \
  }while(0)

/**
 * @brief   Disables the SDHC peripheral clock.
 *
 * @api
 */
#define cpmDisableSDHC() \
  do {\
    HS_PSO->SD_CFG |= (CPM_BUS_GATE); \
    HS_PSO->SD_CFG &= ~(CPM_CLK_DEN); \
  }while(0)
    

/**
 * @brief   Resets the SDHC peripheral.
 * @note    Not supported in this family, does nothing.
 *
 * @api
 */
#define cpmResetSDHC() \
do { \
  HS_PSO->SD_CFG &= ~(1 << 2); \
  HS_PSO->SD_CFG |= (1 << 2); \
} while (0)
/** @} */

/**
 * @name    SPI peripherals specific CPM operations
 * @{
 */
/**
 * @brief   Enables the SPI0 peripheral clock.
 *
 * @api
 */
#define cpmEnableSPI0() HS_PSO->SPI0_CFG &= ~(CPM_BUS_GATE)

/**
 * @brief   Disables the SPI0 peripheral clock.
 *
 * @api
 */
#define cpmDisableSPI0() HS_PSO->SPI0_CFG |= (CPM_BUS_GATE)

/**
 * @brief   Resets the SPI0 peripheral.
 *
 * @api
 */
#define cpmResetSPI0() \
do { \
  HS_PSO->SPI0_CFG &= ~(CPM_BUS_RST); \
  HS_PSO->SPI0_CFG |= (CPM_BUS_RST); \
} while (0)

/**
 * @brief   Enables the SPI1 peripheral clock.
 *
 * @api
 */
#define cpmEnableSPI1() HS_PSO->SPI1_CFG &= ~(CPM_BUS_GATE)

/**
 * @brief   Disables the SPI1 peripheral clock.
 *
 * @api
 */
#define cpmDisableSPI1() HS_PSO->SPI1_CFG |= (CPM_BUS_GATE)

/**
 * @brief   Resets the SPI1 peripheral.
 *
 * @api
 */
#define cpmResetSPI1() \
do { \
  HS_PSO->SPI1_CFG &= ~(CPM_BUS_RST); \
  HS_PSO->SPI1_CFG |= (CPM_BUS_RST); \
} while (0)
/** @} */

/**
 * @name    TIM peripherals specific CPM operations
 * @{
 */
/**
 * @brief   Enables the TIM0 peripheral clock.
 *
 * @api
 */
#define cpmEnableTIM0() \
  do {\
    HS_PSO->TIM0_CFG &= ~(CPM_BUS_GATE); \
    HS_PSO->TIM0_CFG |= (CPM_CLK_DEN);   \
  }while(0)

/**
 * @brief   Disables the TIM0 peripheral clock.
 *
 * @api
 */
#define cpmDisableTIM0() \
  do {\
    HS_PSO->TIM0_CFG |= (CPM_BUS_GATE);   \
    HS_PSO->TIM0_CFG &= ~(CPM_CLK_DEN);    \
  }while(0)

/**
 * @brief   Resets the TIM0 peripheral.
 *
 * @api
 */
#define cpmResetTIM0() \
do { \
  HS_PSO->TIM0_CFG &= ~(CPM_MOD_RST | CPM_BUS_RST); \
  HS_PSO->TIM0_CFG |= (CPM_MOD_RST | CPM_BUS_RST); \
} while (0)

/**
 * @brief   Enables the TIM1 peripheral clock.
 *
 * @api
 */
#define cpmEnableTIM1() \
  do {\
    HS_PSO->TIM1_CFG &= ~(CPM_BUS_GATE); \
    HS_PSO->TIM1_CFG |= (CPM_CLK_DEN);   \
  }while(0)

/**
 * @brief   Disables the TIM1 peripheral clock.
 *
 * @api
 */
#define cpmDisableTIM1() \
  do {\
    HS_PSO->TIM1_CFG |= (CPM_BUS_GATE);   \
    HS_PSO->TIM1_CFG &= ~(CPM_CLK_DEN);    \
  }while(0)

/**
 * @brief   Resets the TIM1 peripheral.
 *
 * @api
 */
#define cpmResetTIM1() \
do { \
  HS_PSO->TIM1_CFG &= ~(CPM_MOD_RST | CPM_BUS_RST); \
  HS_PSO->TIM1_CFG |= (CPM_MOD_RST | CPM_BUS_RST); \
} while (0)

/**
 * @brief   Enables the TIM2 peripheral clock.
 *
 * @api
 */
#define cpmEnableTIM2() \
  do {\
    HS_PSO->TIM2_CFG &= ~(CPM_BUS_GATE); \
    HS_PSO->TIM2_CFG |= (CPM_CLK_DEN);   \
  }while(0)

/**
 * @brief   Disables the TIM2 peripheral clock.
 *
 * @api
 */
#define cpmDisableTIM2() \
  do {\
    HS_PSO->TIM2_CFG |= (CPM_BUS_GATE);   \
    HS_PSO->TIM2_CFG &= ~(CPM_CLK_DEN);    \
  }while(0)

/**
 * @brief   Resets the TIM2 peripheral.
 *
 * @api
 */
#define cpmResetTIM2() \
do { \
  HS_PSO->TIM2_CFG &= ~(CPM_MOD_RST | CPM_BUS_RST); \
  HS_PSO->TIM2_CFG |= (CPM_MOD_RST | CPM_BUS_RST); \
} while (0)
/** @} */

/**
 * @name    UART peripherals specific CPM operations
 * @{
 */
/**
 * @brief   Enables the UART0 peripheral clock.
 *
 * @api
 */
#define cpmEnableUART0() \
  do { \
    HS_PSO->UART0_CFG &= ~(CPM_BUS_GATE);  \
    HS_PSO->UART0_CFG |= (CPM_CLK_DEN);  \
  }while(0)

/**
 * @brief   Disables the UART0 peripheral clock.
 *
 * @api
 */
#define cpmDisableUART0()  \
  do { \
    HS_PSO->UART0_CFG |= (CPM_BUS_GATE);  \
    HS_PSO->UART0_CFG &= ~(CPM_CLK_DEN);  \
  }while(0)

/**
 * @brief   Resets the UART0 peripheral.
 *
 * @api
 */
#define cpmResetUART0() \
do { \
  HS_PSO->UART0_CFG &= ~(CPM_MOD_RST | CPM_BUS_RST); \
  HS_PSO->UART0_CFG |= (CPM_MOD_RST | CPM_BUS_RST); \
} while (0)

/**
 * @brief   Enables the UART1 peripheral clock.
 *
 * @api
 */
#define cpmEnableUART1() \
  do { \
    HS_PSO->UART1_CFG &= ~(CPM_BUS_GATE);  \
    HS_PSO->UART1_CFG |= (CPM_CLK_DEN);  \
  }while(0)

/**
 * @brief   Disables the UART1 peripheral clock.
 *
 * @api
 */
#define cpmDisableUART1()  \
  do { \
    HS_PSO->UART1_CFG |= (CPM_BUS_GATE);  \
    HS_PSO->UART1_CFG &= ~(CPM_CLK_DEN);  \
  }while(0)

/**
 * @brief   Resets the UART1 peripheral.
 *
 * @api
 */
#define cpmResetUART1() \
do { \
  HS_PSO->UART1_CFG &= ~(CPM_MOD_RST | CPM_BUS_RST); \
  HS_PSO->UART1_CFG |= (CPM_MOD_RST | CPM_BUS_RST); \
} while (0)

/**
 * @brief   Enables the RAM clock Auto GATE.
 *
 * @api
 */
#define cpmEnableAutoGateRAMClock() HS_PSO->CPM_GATE_CFG |= (1 << 0)

/**
 * @brief   Disables the RAM clock Auto GATE.
 *
 * @api
 */
#define cpmDisableAutoGateRAMClock() HS_PSO->CPM_GATE_CFG &= (~(1 << 0))

/**
 * @brief   Resets the PSO_CPM peripheral.
 *
 * @api
 */
#define cpmResetPSOCPM()  \
do { \
    HS_PSO->CPM_GATE_CFG &= ~(1 << 5);		\
    HS_PSO->CPM_GATE_CFG |= (1 << 5);    \
}while (0)


/**
 * @brief   Enables RTC clock.
 * @api
 */
#define cpmEnableRTC()

/**
 * @brief   Disables RTC clock.
 * @api
 */
#define cpmDisableRTC()

/**
 * @brief   Resets the RTC peripheral.
 *
 * @api
 */
#define cpmResetRTC()  \
do { \
    HS_PMU_CPM->RTC_CFG &= (~((1<<2)|(1<<4))) ;		\
    HS_PMU_CPM->RTC_CFG |= (1<<2)|(1<<4);    \
}while (0)

/**
 * @brief    Disable pd_chg, pd_ioldo_v3p3 (normal)
 * @api
 */
#define cpmDisableMisc() HS_PMU->ANA_CON |= ((1 << 5) | (1 << 28))

/**
 * @brief    Enable pd_chg, pd_ioldo_v3p3 (normal,wakeup)
 * @api
 */
#define cpmEnableMisc() HS_PMU->ANA_CON &= ~((1 << 5) | (1 << 28) | (1 << 27))


/**
 * @brief   PMU power off PSO domain to enter deep sleep; wakeup from power pin
 * @api
 */
#define pmu_poweroff_pso() \
do { \
   HS_PMU->PSO_PM_CON = ((HS_PMU->PSO_PM_CON&0xfffffffc) | PMU_POWER_OFF | (1u<<24)); \
} while (0)

/**
 * @brief   PMU power off ram domain
 * @api
 */
#define pmu_poweroff_ram() \
do { \
   HS_PMU->RAM_PM_CON = ((HS_PMU->RAM_PM_CON&0xfffffffc) | PMU_POWER_OFF); \
   pmu_reg_update();                  \
} while (0)

/**
 * @brief   PMU power off CEVA0 domain
 * @api
 */
#define pmu_poweroff_ceva0() \
do { \
   HS_PMU->CEVA0_PM_CON = ((HS_PMU->CEVA0_PM_CON&0xfffffffc) | PMU_POWER_OFF); \
   pmu_reg_update();                  \
} while (0)


/**
 * @brief   PMU power off CEVA1  domain
 * @api
 */
#define pmu_poweroff_ceva1() \
do { \
   HS_PMU->CEVA1_PM_CON = ((HS_PMU->CEVA1_PM_CON&0xfffffffc) | PMU_POWER_OFF); \
   pmu_reg_update();                  \
} while (0)


/**
 * @brief   PMU power off PSO domain to enter deep sleep; wakeup from power pin
 * @api
 */
#define pmu_poweroff_mcu() \
do { \
  HS_PMU->BASIC |= (1<<7);               \
  pmu_reg_update();                  \
} while (0)


#define CPM_GetXtalSel()      ( (HS_PMU->GPIO_STATUS[0] >> 20  ) & 0x1u )

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void cpm_start_pll(void);
  void cpm_reset_pll(void);
  void cpm_stop_pll(void);
  void cpm_switch_to_pll(void);
  void cpm_switch_to_xtal(void);
  void cpm_init_clock(void);
  uint32_t cpm_get_clock(hs_clk_t idx);
  void cpm_set_clock(hs_clk_t idx, uint32_t hz);
  void cpm_set_sf_clock_delay(uint8_t half_cycle);
  void cpm_set_sd_dev_clock(sd_dev_clk_t *sd_dev_clk);
  void sf_inner_select(sf_loc_t loc);

  void pmu_xclkon_insleep(int on);
  uint32_t pmu_get_powerPinStatus(void);
  void pmu_deep_sleep(void);
  void pmu_chip_poweroff(void);

  uint32_t pmu_ana_get(uint32_t start, uint32_t end);
  void pmu_ana_set(uint32_t start, uint32_t end, uint32_t val);
  void pmu_cali_rc(void);
  void pmu_cali_ldo(void);

  void cpm_delay_us(uint32_t us);
  void cpm_reset_system(void);
#ifdef __cplusplus
}
#endif

#endif /* _HAL_CPM_H_ */

/** @} */
