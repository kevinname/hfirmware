#include <nds32_intrinsic.h>
#include "nds32_regs.h"
#include "n12_def.h"
#include "context.h" 
#include "hal.h" 
#include "chprintf.h" 

#define __KEEP_POWER  0   

#if __KEEP_POWER
#define __PD_SIGNAL__     ((1U<<1) | (1U<<5) | (0xf << 7) | (3 << 12) | (1 << 16) | (0xf << 21));
#else
#define __PD_SIGNAL__     ((1U<<1) | (1U<<5) | (0xf << 7) | (3 << 12) | (1 << 16) | (0xf << 21) | (3u << 27));
#endif

void __cpu_s_context(void); 
void hs_boardInit(void);
void hs_boardUninit(void);

void cpu_init(void)
{
  unsigned int tmp;
  
  nds32_icache_flush();
  nds32_dcache_invalidate();

  tmp = __nds32__mfsr(NDS32_SR_MMU_CTL);
  tmp &= ~MMU_CTL_MSK;
  tmp |= MMU_CTL_INIT_CACHE_ON;
  __nds32__mtsr(tmp, NDS32_SR_MMU_CTL);

  tmp = __nds32__mfsr(NDS32_SR_CACHE_CTL);
  tmp &= ~CACHE_CTL_MSK;
  tmp |= CACHE_CTL_INIT;
  tmp |= 3;  
  __nds32__mtsr(tmp, NDS32_SR_CACHE_CTL);
}

NOINLINE void cpu_enter_sleep(int xclk_on)
{
  unsigned int tmp, tim;
  
  hs_boardUninit();
  HS_SYS->SYS_TICK &= ~(1u << 24);
  __nds32__setgie_dis();
  cpm_switch_to_xtal();
  cpm_stop_pll();

  #if HAL_USE_WDT
  wdt_disable();
  #endif

  pmu_xclkon_insleep(xclk_on);
  
#if __KEEP_POWER 
  HS_SF->INTR_MASK = 0;
  HS_PSO->BTPHY_CFG |= (CPM_BUS_GATE | (1 << 12) | (1 << 10) | (1 << 8));
  cpmDisableGPIO();
  cpmDisableI2C0();
  cpmDisableUSB();
  cpmDisableSPI0();
  cpmDisableSPI1();
  cpmDisableTIM0();
  cpmDisableTIM1();
  cpmDisableTIM2();
  cpmDisableUART0();
  cpmDisableRTC();

  HS_ANA->PD_CFG[0] |= ((1u<<12) | (1u<<15) | (3u<<20));
  #if 0
  HS_ANA->PD_CFG[1] |= ((1u<<4) | (1u<<12) | (1u<<28));
  #else
  HS_ANA->PD_CFG[1] &= ~(1u<<28);
  HS_ANA->PD_CFG[1] |= ((1u<<4) | (1u<<12));
  #endif
  
  HS_ANA->AU_ANA_CFG[0] |= 0xf;  
  HS_SYS->USB_CTRL  |= (1u<<16);
  
  HS_PSO->CPU_CFG = 0x1f1f049e;
  __WFD();
  HS_PSO->CPU_CFG = 0x1f1f1f9e;
  __WFD();
  
  HS_PSO->CPM_ANA_CFG |= 2;  
  HS_PSO->REG_UPD = 0x01; 

  //HS_PMU->EXT &= ~(1u<<4);
  HS_PMU->EXT |= 0x7 << 5;
  HS_PMU->ANA_CON |= __PD_SIGNAL__; 
  HS_PMU->BASIC  |= (1u<<30); 

  /* keep always off LDOs of RF PLL enter sniff mode: [0]pd_bt_ldodiv, [1]pd_bt_ldommd, [3]pd_bt_synth_ldocp, [5]pd_bt_ldopfd, [6]pd_ldo_v1p3_lobuf, [12]pd_bt_ldovco */
  HS_ANA->PD_CFG[1] &= ~((1 << 28) | (1 << 22) | (1 << 21) | (1 << 19) | (1 << 17) | (1 << 16)); //xxx_flag
  HS_ANA->PD_CFG[1] |= ((1 << 12) | (1 <<  6) | (1 <<  5) | (1 <<  3) | (1 <<  1) | (1 <<  0)); //xxx_reg
  /* keep always on BT RX blocks: [12]pd_rxfil, [11]pd_rxmixer, [10]pd_rxtia, [9]pd_rxgm, [8]pd_rxlna, [5]pd_rxadc_q, [4]pd_rxadc_i, [2]pd_rxadc_biasgen */
  HS_ANA->PD_CFG[2] &= ~((1 << 28) | (1 << 27) | (1 << 26) | (1 << 25) | (1 << 24) | (1 << 21) | (1 << 20) | (1 << 18)); //xxx_flag
  HS_ANA->PD_CFG[2] |= ((1 << 12) | (1 << 11) | (1 << 10) | (1 <<  9) | (1 <<  8) | (1 <<  5) | (1 <<  4) | (1 <<  2)); //xxx_reg
  /* [23][22]pd_tca [21]pd_rfcstgm [12]pd_rxadc_dacbias */
  HS_ANA->PD_CFG[0] &=~(1<<23);
  HS_ANA->PD_CFG[0] |= ((1 << 22) | (1 << 21) | (1 << 12));
#if 0
  /* keep always on BT TX blocks: [10]pd_txpa, [9]pd_txum, [7]pd_txdac */
  HS_ANA->PD_CFG[1] &= ~((1 << 25) | (1 << 23)); //xxx_flag
  HS_ANA->PD_CFG[1] &= ~((1 <<  9) | (1 <<  7)); //xxx_reg
#else
  HS_ANA->PD_CFG[1] &= ~((1 << 25)); //pd_txum always on
  HS_ANA->PD_CFG[1] |= ((1 <<  9));
#endif

  __WFI();  

  HS_PSO->CPU_CFG = 0x49e;
  __WFD();
  HS_PSO->CPU_CFG = 0x9e;
  __WFD();

  cpm_switch_to_pll();

  //HS_PMU->EXT |= (1u<<4);
  HS_PMU->EXT &= ~(0x7 << 5);
  HS_PMU->ANA_CON &= ~__PD_SIGNAL__;  
  HS_PMU->BASIC  |= (1u<<30);  

  HS_PSO->CPM_ANA_CFG &= ~2u;

  HS_SYS->USB_CTRL  &= ~(1u<<16);
  HS_ANA->PD_CFG[0] &= ~((1u<<12) | (1u<<15) | (3u<<20));
  HS_ANA->PD_CFG[1] &= ~((1u<<4) | (1u<<12) | (1u<<28));  
  
  cpmEnableGPIO();

  /* keep always on LDOs of RF PLL: [0]pd_bt_ldodiv, [1]pd_bt_ldommd, [3]pd_bt_synth_ldocp, [5]pd_bt_ldopfd, [6]pd_ldo_v1p3_lobuf, [12]pd_bt_ldovco */
  HS_ANA->PD_CFG[1] &= ~((1 << 28) | (1 << 22) | (1 << 21) | (1 << 19) | (1 << 17) | (1 << 16)); //xxx_flag
  HS_ANA->PD_CFG[1] &= ~((1 << 12) | (1 <<  6) | (1 <<  5) | (1 <<  3) | (1 <<  1) | (1 <<  0)); //xxx_reg
  /* keep always on BT RX blocks: [12]pd_rxfil, [11]pd_rxmixer, [10]pd_rxtia, [9]pd_rxgm, [8]pd_rxlna, [5]pd_rxadc_q, [4]pd_rxadc_i, [2]pd_rxadc_biasgen */
  HS_ANA->PD_CFG[2] &= ~((1 << 28) | (1 << 27) | (1 << 26) | (1 << 25) | (1 << 24) | (1 << 21) | (1 << 20) | (1 << 18)); //xxx_flag
  HS_ANA->PD_CFG[2] &= ~((1 << 12) | (1 << 11) | (1 << 10) | (1 <<  9) | (1 <<  8) | (1 <<  5) | (1 <<  4) | (1 <<  2)); //xxx_reg
  /* [23][22]pd_tca [21]pd_rfcstgm [12]pd_rxadc_dacbias */
  HS_ANA->PD_CFG[0] &= ~((1 << 23) | (1 << 22) | (1 << 21) | (1 << 12));
#if 0
  /* keep always on BT TX blocks: [10]pd_txpa, [9]pd_txum, [7]pd_txdac */
  HS_ANA->PD_CFG[1] &= ~((1 << 25) | (1 << 23)); //xxx_flag
  HS_ANA->PD_CFG[1] &= ~((1 <<  9) | (1 <<  7)); //xxx_reg
#else
  HS_ANA->PD_CFG[1] &= ~((1 << 25)); //pd_txum always on
  HS_ANA->PD_CFG[1] &= ~((1 <<  9));
#endif  

#else
  HS_PMU->ANA_CON |= __PD_SIGNAL__; 
  HS_PMU->EXT |= (7u << 1) | (0xfu << 5);
  HS_PMU->EXT &= ~(1u<<4);  
  
  __cpu_s_context();

  //HS_PMU->EXT |= (1u<<4);  
  HS_PMU->ANA_CON &= ~__PD_SIGNAL__;  
  
  cpu_init();
#endif  

  tmp = chSysGetRealtimeCounterX();
  cpm_reset_system();
  port_systick_init();    
  hs_boardInit();
  __nds32__setgie_en();

  #if HAL_USE_WDT
  wdtInit();
  #endif
  
  tim = (chSysGetRealtimeCounterX() - tmp) / 192;
  hs_printf("Wakeup Time: %d us\r\n", tim);
}


NOINLINE void cpu_enter_sleepNoWakeup(void)
{
  hs_boardUninit();
  __nds32__setgie_dis();
  cpm_switch_to_xtal();
  cpm_stop_pll();

  pmu_xclkon_insleep(0);
  
  HS_PMU->ANA_CON |= (3 << 1) | (0xfu << 7); 
  HS_PMU->POWER_PIN &= ~(1u << 12);
  
  pmu_deep_sleep();  
}



