/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

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
 * @file    hs66xx/icu_lld.c
 * @brief   ICU Driver subsystem low level driver source hs66xx.
 *
 * @addtogroup ICU
 * @{
 */

#include "ch.h"
#include "hal.h"

#if HAL_USE_ICU || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   ICUD0 driver identifier.
 */
#if HS_ICU_USE_TIM0 || defined(__DOXYGEN__)
ICUDriver ICUD0;
#endif

/**
 * @brief   ICUD1 driver identifier.
 */
#if HS_ICU_USE_TIM1 || defined(__DOXYGEN__)
ICUDriver ICUD1;
#endif

/**
 * @brief   ICUD2 driver identifier.
 */
#if HS_ICU_USE_TIM2 || defined(__DOXYGEN__)
ICUDriver ICUD2;
#endif

/**
 * @brief   ICUD3 driver identifier.
 */
#if HS_ICU_USE_TIM3 || defined(__DOXYGEN__)
ICUDriver ICUD3;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/
#if ICU_USE_DMA
/**
 * @brief   icu DMA common service routine.
 *
 * @param[in] IRrcd     pointer to the @p IRrcDevinfo  object
 * @param[in] error     error status
 */
void icuDMACallBack(void *p, hs_dma_cb_para_t *var) {
   ICUDriver *_icup = (ICUDriver *)p; 
   
  if(var->oper_res != DMA_TRANS_OPER_OK){
    return ;
  }else{
    _icup->config->icuDMAinfo->callback(_icup);
    }      
}
#endif
/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Shared IRQ handler.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 */
static void icu_lld_serve_interrupt(ICUDriver *icup) {
  uint16_t sr;

  sr  = icup->tim->SR;
  sr &= icup->tim->DIER & _TIM_DIER_IRQ_MASK;
  icup->tim->SR = ~sr;
  if (icup->config->channel == ICU_CHANNEL_1) {
    if ((sr & TIM_SR_CC1IF) != 0)
      _icu_isr_invoke_period_cb(icup);
    if ((sr & TIM_SR_CC2IF) != 0)
      _icu_isr_invoke_width_cb(icup);
  } else {
    if ((sr & TIM_SR_CC1IF) != 0)
      _icu_isr_invoke_width_cb(icup);
    if ((sr & TIM_SR_CC2IF) != 0)
      _icu_isr_invoke_period_cb(icup);
  }
  if ((sr & TIM_SR_UIF) != 0)
    _icu_isr_invoke_overflow_cb(icup);
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if HS_ICU_USE_TIM0
/**
 * @brief   TIM0 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra osaleck in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM0_IRQHandler) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD0);

  OSAL_IRQ_EPILOGUE();
}
#endif /* HS_ICU_USE_TIM0 */

#if HS_ICU_USE_TIM1
/**
 * @brief   TIM1 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra osaleck in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM1_IRQHandler) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD1);

  OSAL_IRQ_EPILOGUE();
}
#endif /* HS_ICU_USE_TIM1 */

#if HS_ICU_USE_TIM2
/**
 * @brief   TIM2 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra osaleck in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM2_IRQHandler) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD2);

  OSAL_IRQ_EPILOGUE();
}
#endif /* HS_ICU_USE_TIM2 */

#if HS_ICU_USE_TIM3
/**
 * @brief   TIM3 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra osaleck in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM3_IRQHandler) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD3);

  OSAL_IRQ_EPILOGUE();
}
#endif /* HS_ICU_USE_TIM3 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level ICU driver initialization.
 *
 * @notapi
 */
void icu_lld_init(void) {

#if HS_ICU_USE_TIM0
  /* Driver initialization.*/
  icuObjectInit(&ICUD0);
  ICUD0.tim = HS_TIM0;
#endif /* HS_ICU_USE_TIM0 */
#if HS_ICU_USE_TIM1
  /* Driver initialization.*/
  icuObjectInit(&ICUD1);
  ICUD1.tim = HS_TIM1;
#endif /* HS_ICU_USE_TIM1 */
#if HS_ICU_USE_TIM2
  /* Driver initialization.*/
  icuObjectInit(&ICUD2);
  ICUD2.tim = HS_TIM2;
#endif /* HS_ICU_USE_TIM2 */
#if HS_ICU_USE_TIM3
  /* Driver initialization.*/
  icuObjectInit(&ICUD3);
  ICUD3.tim = HS_TIM3;
#endif /* HS_ICU_USE_TIM3 */
}

/**
 * @brief   Configures and activates the ICU peripheral.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 *
 * @notapi
 */
void icu_lld_start(ICUDriver *icup) {
  uint32_t psc;

  if (icup->state == ICU_STOP) {
    /* Enables the peripheral.*/
#if HS_ICU_USE_TIM0
    if (&ICUD0 == icup) {
      cpmEnableTIM0();
      nvicEnableVector(IRQ_TIM0,
    		           ANDES_PRIORITY_MASK(HS_ICU_TIM0_IRQ_PRIORITY));
      icup->clock = cpm_get_clock(HS_TIM0_CLK);
    }
#endif /* HS_ICU_USE_TIM0 */
#if HS_ICU_USE_TIM1
    if (&ICUD1 == icup) {
      cpmEnableTIM1();
      nvicEnableVector(IRQ_TIM1,
    		           ANDES_PRIORITY_MASK(HS_ICU_TIM1_IRQ_PRIORITY));
      icup->clock =cpm_get_clock(HS_TIM1_CLK);
    }
#endif /* HS_ICU_USE_TIM1 */
#if HS_ICU_USE_TIM2
    if (&ICUD2 == icup) {
      cpmEnableTIM2();
      nvicEnableVector(IRQ_TIM2,
    		           ANDES_PRIORITY_MASK(HS_ICU_TIM2_IRQ_PRIORITY));
      icup->clock = cpm_get_clock(HS_TIM2_CLK);
    }
#endif /* HS_ICU_USE_TIM2 */
#if HS_ICU_USE_TIM3
    if (&ICUD3 == icup) {
      cpmEnableTIM3();
      nvicEnableVector(IRQ_TIM3,
    		           ANDES_PRIORITY_MASK(HS_ICU_TIM3_IRQ_PRIORITY));
      icup->clock = cpm_get_clock(HS_TIM3_CLK);
    }
#endif /* HS_ICU_USE_TIM3 */
  }
  else {
    /* Driver re-configuration scenario, it must be stopped first.*/
    icup->tim->CR1    = 0;                  /* Timer disabled.              */
    icup->tim->DIER   = icup->config->dier &/* DMA-related DIER settings.   */
                        ~_TIM_DIER_IRQ_MASK;
    icup->tim->SR     = 0;                  /* Clear eventual pending IRQs. */
    icup->tim->CCR[0] = 0;                  /* Comparator 1 disabled.       */
    icup->tim->CCR[1] = 0;                  /* Comparator 2 disabled.       */
    icup->tim->CNT    = 0;                  /* Counter reset to zero.       */
  }

  /* Configures the peripheral.*/
  psc = (icup->clock / icup->config->frequency) - 1;
  osalDbgAssert((psc <= 0xFFFF) &&
              ((psc + 1) * icup->config->frequency) == icup->clock,
              "icu_lld_start(), #1,invalid frequency");
  icup->tim->PSC  = (uint16_t)psc;
  icup->tim->ARR   = 0xFFFF;

  if (icup->config->channel == ICU_CHANNEL_1) {
    /* Selected input 1.
       CCMR1_CC1S = 01 = CH1 Input on TI1.
       CCMR1_CC2S = 10 = CH2 Input on TI1.*/
    icup->tim->CCMR1 = _TIM_CCMR1_CC1S(1) | _TIM_CCMR1_CC2S(2);

    /* SMCR_TS  = 101, input is TI1FP1.
       SMCR_SMS = 100, reset on rising edge.*/
    icup->tim->SMCR  = _TIM_SMCR_TS(5) | _TIM_SMCR_SMS(4);

    /* The CCER settings depend on the selected trigger mode.
       ICU_INPUT_ACTIVE_HIGH: Active on rising edge, idle on falling edge.
       ICU_INPUT_ACTIVE_LOW:  Active on falling edge, idle on rising edge.*/
    if (icup->config->mode == ICU_INPUT_ACTIVE_HIGH)
      icup->tim->CCER = TIM_CCER_CC1E |
                        TIM_CCER_CC2E | TIM_CCER_CC2P;
     // icup->tim->CCER = TIM_CCER_CC1E | TIM_CCER_CC1P | TIM_CCER_CC1NP;
    else
      icup->tim->CCER = TIM_CCER_CC1E | TIM_CCER_CC1P |
                        TIM_CCER_CC2E;

    /* Direct pointers to the capture registers in order to make reading
       data faster from within callbacks.*/
    icup->wccrp = &icup->tim->CCR[1];
    icup->pccrp = &icup->tim->CCR[0];
  } else {
    /* Selected input 2.
       CCMR1_CC1S = 10 = CH1 Input on TI2.
       CCMR1_CC2S = 01 = CH2 Input on TI2.*/
    icup->tim->CCMR1 = _TIM_CCMR1_CC1S(2) | _TIM_CCMR1_CC2S(1);

    /* SMCR_TS  = 110, input is TI2FP2.
       SMCR_SMS = 100, reset on rising edge.*/
    icup->tim->SMCR  = _TIM_SMCR_TS(6) | _TIM_SMCR_SMS(4);

    /* The CCER settings depend on the selected trigger mode.
       ICU_INPUT_ACTIVE_HIGH: Active on rising edge, idle on falling edge.
       ICU_INPUT_ACTIVE_LOW:  Active on falling edge, idle on rising edge.*/
    if (icup->config->mode == ICU_INPUT_ACTIVE_HIGH)
      icup->tim->CCER = TIM_CCER_CC1E | TIM_CCER_CC1P |
                        TIM_CCER_CC2E;
    else
      icup->tim->CCER = TIM_CCER_CC1E |
                        TIM_CCER_CC2E | TIM_CCER_CC2P;

    /* Direct pointers to the capture registers in order to make reading
       data faster from within callbacks.*/
    icup->wccrp = &icup->tim->CCR[0];
    icup->pccrp = &icup->tim->CCR[1];
  }
  
/*use DMA*/  
#if ICU_USE_DMA
  if (icup->config->useDMA) {
      hs_dma_config_t sconfig;
      hs_dma_burstlen_t burst_len = DMA_BURST_LEN_4UNITS;
  
      /* config TIMER dma register */
      icup->tim->DCR = icup->config->icuDMAinfo->startRegNum 
          | ((icup->config->icuDMAinfo->RegLength-1) << 8);
     // icup->tim->DIER |= TIM_DIER_UDE;
               
      /* allocate DMA channel to timer icu */
      icup->icudma = dmaStreamAllocate(HS_ICU_DMA_PRIORITY,
          (hs_dmaisr_t)icuDMACallBack,
          (void *)icup);
      if (icup->icudma == NULL)
      return;
        
      if (icup->config->icuDMAinfo->RegLength == 1)
      burst_len = DMA_BURST_LEN_1UNITS;
      else if(icup->config->icuDMAinfo->RegLength == 4)
      burst_len = DMA_BURST_LEN_4UNITS;
      else if(icup->config->icuDMAinfo->RegLength == 8)
      burst_len = DMA_BURST_LEN_8UNITS;

#if HS_ICU_USE_TIM0
      if (&ICUD0 == icup) 
	sconfig.slave_id = TIMER0_DMA_ID;
#endif
#if HS_ICU_USE_TIM1
      if(&ICUD1 == icup)
	sconfig.slave_id = TIMER1_DMA_ID; 
#endif
#if HS_ICU_USE_TIM2
      if(&ICUD2 == icup)
	sconfig.slave_id = TIMER2_DMA_ID; 
#endif
        
      sconfig.direction = DMA_DEV_TO_MEM;
      sconfig.src_addr_width = DMA_SLAVE_BUSWIDTH_32BITS;
      sconfig.dst_addr_width = DMA_SLAVE_BUSWIDTH_32BITS;
      sconfig.src_burst = burst_len;
      sconfig.dst_burst = burst_len;
      sconfig.dev_flow_ctrl = FALSE;	
      sconfig.lli_en = 0;
      dmaStreamSetMode(icup->icudma, &sconfig);
  }
#endif
}

/**
 * @brief   Deactivates the ICU peripheral.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 *
 * @notapi
 */
void icu_lld_stop(ICUDriver *icup) {

  if (icup->state == ICU_READY) {
    /* Resets the peripheral.*/
    icup->tim->CR1  = 0;                    /* Timer disabled.              */
    icup->tim->DIER = 0;                    /* All IRQs disabled.           */
    icup->tim->SR   = 0;                    /* Clear eventual pending IRQs. */

    /* Disables the peripheral.*/
#if HS_ICU_USE_TIM0
    if (&ICUD0 == icup) {
      nvicDisableVector(IRQ_TIM0);
      cpmDisableTIM0();
    }
#endif /* HS_ICU_USE_TIM0 */
#if HS_ICU_USE_TIM1
    if (&ICUD1 == icup) {
      nvicDisableVector(IRQ_TIM1);
      cpmDisableTIM1();
    }
#endif /* HS_ICU_USE_TIM1 */
#if HS_ICU_USE_TIM2
    if (&ICUD2 == icup) {
      nvicDisableVector(IRQ_TIM2);
      cpmDisableTIM2();
    }
#endif /* HS_ICU_USE_TIM2 */
#if HS_ICU_USE_TIM3
    if (&ICUD3 == icup) {
      nvicDisableVector(IRQ_TIM3);
      cpmDisableTIM3();
    }
#endif /* HS_ICU_USE_TIM3 */

#if ICU_USE_DMA  
    if ((icup->config->useDMA) && (icup->icudma != NULL))
      dmaStreamRelease(icup->icudma);
#endif
  }
}

/**
 * @brief   Enables the input capture.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 *
 * @notapi
 */
void icu_lld_enable(ICUDriver *icup) {

  icup->tim->SR = 0;                        /* Clear pending IRQs (if any). */
#if ICU_USE_DMA
  if ((icup->config->useDMA) && (icup->icudma != NULL)) {
    if (icup->config->channel == ICU_CHANNEL_1) {
      if (icup->config->icuDMAinfo->periodU)
        icup->tim->DIER |= TIM_DIER_CC1DE/* | TIM_DIER_CC2DE*/;
      if(icup->config->icuDMAinfo->widthU)
        icup->tim->DIER |= TIM_DIER_CC2DE /*| TIM_DIER_CC1DE*/;

    } else {
      if (icup->config->icuDMAinfo->periodU)
        icup->tim->DIER |= TIM_DIER_CC1DE;
      if(icup->config->icuDMAinfo->widthU)
        icup->tim->DIER |= TIM_DIER_CC2DE;
    }
  
  dmaStreamStart(icup->icudma, &icup->tim->DMAR, icup->config->icuDMAinfo->memAddr, 
            icup->config->icuDMAinfo->blockSize);
  
  //icup->tim->EGR   = TIM_EGR_UG;      /* Update event.  */ 
 }else {
#endif
  if (icup->config->channel == ICU_CHANNEL_1) {
    if (icup->config->period_cb != NULL)
      icup->tim->DIER |= TIM_DIER_CC1IE;
    if (icup->config->width_cb != NULL)
      icup->tim->DIER |= TIM_DIER_CC2IE;
  } else {
    if (icup->config->width_cb != NULL)
      icup->tim->DIER |= TIM_DIER_CC1IE;
    if (icup->config->period_cb != NULL)
      icup->tim->DIER |= TIM_DIER_CC2IE;
  }
  if (icup->config->overflow_cb != NULL)
    icup->tim->DIER |= TIM_DIER_UIE;
#if ICU_USE_DMA  
  }
#endif
   icup->tim->CR1 = TIM_CR1_URS | TIM_CR1_CEN;
}

/**
 * @brief   Disables the input capture.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 *
 * @notapi
 */
void icu_lld_disable(ICUDriver *icup) {

  icup->tim->CR1   = 0;                     /* Initially stopped.           */
  icup->tim->SR    = 0;                     /* Clear pending IRQs (if any). */

  /* All interrupts disabled.*/
  icup->tim->DIER &= ~_TIM_DIER_IRQ_MASK;
#if ICU_USE_DMA
    if ((icup->config->useDMA) && (icup->icudma != NULL)) {
      icup->tim->DIER &= ~(TIM_DIER_CC1DE  | TIM_DIER_CC2DE
                           |TIM_DIER_CC3DE | TIM_DIER_CC4DE);      
      dmaStreamDisable(icup->icudma);
  }
#endif    
}

#endif /* HAL_USE_ICU */

/** @} */
