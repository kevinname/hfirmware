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
 * @file    hs66xx/pwm_lld.c
 * @brief   PWM Driver subsystem low level driver source.
 *
 * @addtogroup PWM
 * @{
 */

#include "ch.h"
#include "hal.h"

#if HAL_USE_PWM || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   PWMD0 driver identifier.
 */
#if HS_PWM_USE_TIM0 || defined(__DOXYGEN__)
PWMDriver PWMD0;
#endif

/**
 * @brief   PWMD1 driver identifier.
 */
#if HS_PWM_USE_TIM1 || defined(__DOXYGEN__)
PWMDriver PWMD1;
#endif

/**
 * @brief   PWMD2 driver identifier.
 */
#if HS_PWM_USE_TIM2 || defined(__DOXYGEN__)
PWMDriver PWMD2;
#endif

/**
 * @brief   PWMD3 driver identifier.
 */
#if HS_PWM_USE_TIM3 || defined(__DOXYGEN__)
PWMDriver PWMD3;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Common IRQ handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 */
static void pwm_lld_serve_interrupt(PWMDriver *pwmp) {
    uint16_t sr;
    
    sr = pwmp->tim->SR;
    sr = sr & pwmp->tim->DIER & _TIM_DIER_IRQ_MASK;
    pwmp->tim->SR = ~sr;
    
  if (((sr & TIM_SR_CC1IF) != 0) && (pwmp->config->channels[0].callback != NULL))
  pwmp->config->channels[0].callback(pwmp, pwmp->config->channels[0].arg);
  
  if (((sr & TIM_SR_CC2IF) != 0) && (pwmp->config->channels[1].callback != NULL))
  pwmp->config->channels[1].callback(pwmp, pwmp->config->channels[1].arg);
  
  if (((sr & TIM_SR_CC3IF) != 0) && (pwmp->config->channels[2].callback != NULL))
          pwmp->config->channels[2].callback(pwmp, pwmp->config->channels[2].arg);
  
  if (((sr & TIM_SR_CC4IF) != 0) && (pwmp->config->channels[3].callback != NULL))
          pwmp->config->channels[3].callback(pwmp, pwmp->config->channels[3].arg);
  
  if (((sr & TIM_SR_UIF) != 0) && (pwmp->config->callback != NULL))
          pwmp->config->callback(pwmp, NULL);
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if HS_PWM_USE_TIM0
/**
 * @brief   TIM0 interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(TIM0_IRQHandler) {

  CH_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD0);

  CH_IRQ_EPILOGUE();
}
#endif /* HS_PWM_USE_TIM0 */

#if HS_PWM_USE_TIM1
/**
 * @brief   TIM1 interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(TIM1_IRQHandler) {

  CH_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD1);

  CH_IRQ_EPILOGUE();
}
#endif /* HS_PWM_USE_TIM1 */

#if HS_PWM_USE_TIM2
/**
 * @brief   TIM2 interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(TIM2_IRQHandler) {

  CH_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD2);

  CH_IRQ_EPILOGUE();
}
#endif /* HS_PWM_USE_TIM2 */

#if HS_PWM_USE_TIM3
/**
 * @brief   TIM3 interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(TIM3_IRQHandler) {

  CH_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD3);

  CH_IRQ_EPILOGUE();
}
#endif /* HS_PWM_USE_TIM3 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level PWM driver initialization.
 *
 * @notapi
 */
void pwm_lld_init(void) {

#if HS_PWM_USE_TIM0
  /* Driver initialization.*/
  pwmObjectInit(&PWMD0);
  PWMD0.tim = HS_TIM0;
#endif /* HS_PWM_USE_TIM0 */

#if HS_PWM_USE_TIM1
  /* Driver initialization.*/
  pwmObjectInit(&PWMD1);
  PWMD1.tim = HS_TIM1;
#endif /* HS_PWM_USE_TIM1*/

#if HS_PWM_USE_TIM2
  /* Driver initialization.*/
  pwmObjectInit(&PWMD2);
  PWMD2.tim = HS_TIM2;
#endif /* HS_PWM_USE_TIM2 */
}
#if HS_PWM_USE_TIM3
  /* Driver initialization.*/
  pwmObjectInit(&PWMD3);
  PWMD3.tim = HS_TIM3;
#endif /* HS_PWM_USE_TIM0 */

#if PWM_USE_DMA
/**
 * @brief   pwm DMA common service routine.
 *
 * @param[in] IRrcd     pointer to the @p IRrcDevinfo  object
 * @param[in] error     error status
 */
void pwm_DMACallBack(void *p, hs_dma_cb_para_t *var) {
   PWMDriver *_pwmp = (PWMDriver *)p; 
   
  if(var->oper_res != DMA_TRANS_OPER_OK){
    return ;
  }else{
    _pwmp->config->pwmDMAinfo->pwmDMAcallback();
    }      
}
#endif

/**
 * @brief   Configures and activates the PWM peripheral.
 *
 * @param[in] pwmp      pointer to the @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_start(PWMDriver *pwmp) {
  uint32_t psc;
  uint16_t ccer;
  
  if (pwmp->state == PWM_STOP) {
    /* Enables the peripheral.*/
#if HS_PWM_USE_TIM0
    if (&PWMD0 == pwmp) {
      cpmEnableTIM0();
      if (0 == pwmp->config->frequency) {
        /* workaround: max frequency to TIM, bypass->upd->div1 */
        HS_PSO->TIM0_CFG = 0x14;
        HS_PSO->REG_UPD = 0x01;
        HS_PSO->TIM0_CFG = (1u << 8) | 0x15;
      }
      nvicEnableVector(IRQ_TIM0,
                       ANDES_PRIORITY_MASK(HS_PWM_TIM0_IRQ_PRIORITY));
      pwmp->clock = cpm_get_clock(HS_TIM0_CLK);
    }
#endif /* HS_PWM_USE_TIM1*/
#if HS_PWM_USE_TIM1
    if (&PWMD1 == pwmp) {
      cpmEnableTIM1();
      if (0 == pwmp->config->frequency) {
        /* workaround: max frequency to TIM, bypass->upd->div1 */
        HS_PSO->TIM1_CFG = 0x14;
        HS_PSO->REG_UPD = 0x01;
        HS_PSO->TIM1_CFG = (1u << 8) | 0x15;
      }
      nvicEnableVector(IRQ_TIM1,
                       ANDES_PRIORITY_MASK(HS_PWM_TIM1_IRQ_PRIORITY));
      pwmp->clock = cpm_get_clock(HS_TIM1_CLK);
    }
#endif /* HS_PWM_USE_TIM1 */
#if HS_PWM_USE_TIM2
    if (&PWMD2 == pwmp) {
      cpmEnableTIM2();
      if (0 == pwmp->config->frequency) {
        /* workaround: max frequency to TIM, bypass->upd->div1 */
        HS_PSO->TIM2_CFG = 0x14;
        HS_PSO->REG_UPD = 0x01;
        HS_PSO->TIM2_CFG = (1u << 8) | 0x15;
      }
      nvicEnableVector(IRQ_TIM2,
                       ANDES_PRIORITY_MASK(HS_PWM_TIM2_IRQ_PRIORITY));
      pwmp->clock = cpm_get_clock(HS_TIM2_CLK);
    }
#endif /* HS_PWM_USE_TIM2 */
#if HS_PWM_USE_TIM3
    if (&PWMD3 == pwmp) {
      cpmEnableTIM3();
      nvicEnableVector(IRQ_TIM3,
                       ANDES_PRIORITY_MASK(HS_PWM_TIM3_IRQ_PRIORITY));
      pwmp->clock = cpm_get_clock(HS_TIM3_CLK);
    }
#endif /* HS_PWM_USE_TIM3 */
    
    pwmp->tim->CR1 = TIM_CR1_ARPE;

    /* All channels configured in PWM1 mode with preload enabled and will
       stay that way until the driver is stopped.*/
    pwmp->tim->CCMR1 = _TIM_CCMR1_OC1M(6) | TIM_CCMR1_OC1PE |
                       _TIM_CCMR1_OC2M(6) | TIM_CCMR1_OC2PE;
    pwmp->tim->CCMR2 = _TIM_CCMR2_OC3M(6) | TIM_CCMR2_OC3PE |
                       _TIM_CCMR2_OC4M(6) | TIM_CCMR2_OC4PE;
  } else {
    /* Driver re-configuration scenario, it must be stopped first.*/
    pwmp->tim->CR1    = 0;                  /* Timer disabled.              */
    pwmp->tim->DIER   = pwmp->config->dier &/* DMA-related DIER settings.   */
                        ~_TIM_DIER_IRQ_MASK;
    pwmp->tim->SR     = 0;                  /* Clear eventual pending IRQs. */
    pwmp->tim->CCR[0] = 0;                  /* Comparator 1 disabled.       */
    pwmp->tim->CCR[1] = 0;                  /* Comparator 2 disabled.       */
    pwmp->tim->CCR[2] = 0;                  /* Comparator 3 disabled.       */
    pwmp->tim->CCR[3] = 0;                  /* Comparator 4 disabled.       */
    pwmp->tim->CNT  = 0;                    /* Counter reset to zero.       */
  }

  /* Configures the peripheral.*/
  if (0 == pwmp->config->frequency) {
    /* max frequency to TIM */
    psc = 0;
  }
  else {
  psc = (pwmp->clock / pwmp->config->frequency) - 1;
  chDbgAssert((psc <= 0xFFFF) &&
              ((psc + 1) * pwmp->config->frequency) == pwmp->clock,
              "pwm_lld_start(), #1,invalid frequency");
  }
  pwmp->tim->PSC  = (uint16_t)psc;
  pwmp->tim->ARR  = (uint16_t)(pwmp->period - 1);
  pwmp->tim->CR2  = pwmp->config->cr2;

  /* Output enables and polarities setup.*/
  ccer = 0;
  switch (pwmp->config->channels[0].mode & PWM_OUTPUT_MASK) {
  case PWM_OUTPUT_ACTIVE_LOW:
    ccer |= TIM_CCER_CC1P;
  case PWM_OUTPUT_ACTIVE_HIGH:
    ccer |= TIM_CCER_CC1E;
  default:
    ;
  }
  switch (pwmp->config->channels[1].mode & PWM_OUTPUT_MASK) {
  case PWM_OUTPUT_ACTIVE_LOW:
    ccer |= TIM_CCER_CC2P;
  case PWM_OUTPUT_ACTIVE_HIGH:
    ccer |= TIM_CCER_CC2E;
  default:
    ;
  }
  switch (pwmp->config->channels[2].mode & PWM_OUTPUT_MASK) {
  case PWM_OUTPUT_ACTIVE_LOW:
    ccer |= TIM_CCER_CC3P;
  case PWM_OUTPUT_ACTIVE_HIGH:
    ccer |= TIM_CCER_CC3E;
  default:
    ;
  }
  switch (pwmp->config->channels[3].mode & PWM_OUTPUT_MASK) {
  case PWM_OUTPUT_ACTIVE_LOW:
    ccer |= TIM_CCER_CC4P;
  case PWM_OUTPUT_ACTIVE_HIGH:
    ccer |= TIM_CCER_CC4E;
  default:
    ;
  }

  switch (pwmp->config->channels[0].mode & PWM_COMPLEMENTARY_OUTPUT_MASK) {
  case PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW:
    ccer |= TIM_CCER_CC1NP;
  case PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH:
    ccer |= TIM_CCER_CC1NE;
  default:
    ;
  }
  switch (pwmp->config->channels[1].mode & PWM_COMPLEMENTARY_OUTPUT_MASK) {
  case PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW:
    ccer |= TIM_CCER_CC2NP;
  case PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH:
    ccer |= TIM_CCER_CC2NE;
  default:
    ;
  }
  switch (pwmp->config->channels[2].mode & PWM_COMPLEMENTARY_OUTPUT_MASK) {
  case PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW:
    ccer |= TIM_CCER_CC3NP;
  case PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH:
    ccer |= TIM_CCER_CC3NE;
  default:
    ;
  }
#if 0
  switch (pwmp->config->channels[3].mode & PWM_COMPLEMENTARY_OUTPUT_MASK) {
  case PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW:
    ccer |= TIM_CCER_CC4NP;
  case PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH:
    ccer |= TIM_CCER_CC4NE;
  default:
    ;
  }
#endif

  pwmp->tim->CCER  = ccer;
#if PWM_USE_DMA  
  if (pwmp->config->useDMA == false) {
      pwmp->tim->EGR   = TIM_EGR_UG; }     /* Update event.                */
#else
  pwmp->tim->EGR   = TIM_EGR_UG;
#endif
  
  pwmp->tim->DIER |= pwmp->config->callback == NULL ? 0 : TIM_DIER_UIE;
  pwmp->tim->SR    = 0;                     /* Clear pending IRQs.          */
  pwmp->tim->BDTR  = pwmp->config->bdtr | TIM_BDTR_MOE;

 /*use DMA*/
#if PWM_USE_DMA   
  if (pwmp->config->useDMA == true) {
      hs_dma_config_t sconfig;
      hs_dma_burstlen_t burst_len = DMA_BURST_LEN_4UNITS;
  
      /* config TIMER dma register */
      pwmp->tim->DCR = pwmp->config->pwmDMAinfo->startRegrNum 
          | ((pwmp->config->pwmDMAinfo->RegLength-1) << 8);
      pwmp->tim->DIER |= TIM_DIER_UDE;
        
        
      /* allocate DMA channel to timer pwm */
      pwmp->pwmdma = dmaStreamAllocate(HS_PWM_DMA_PRIORITY,
          (hs_dmaisr_t)pwm_DMACallBack,
          (void *)pwmp);
      if (pwmp->pwmdma == NULL)
      return;
        
      if (pwmp->config->pwmDMAinfo->RegLength == 1)
      burst_len = DMA_BURST_LEN_1UNITS;
      else if(pwmp->config->pwmDMAinfo->RegLength == 4)
      burst_len = DMA_BURST_LEN_4UNITS;
      else if(pwmp->config->pwmDMAinfo->RegLength == 8)
      burst_len = DMA_BURST_LEN_8UNITS;

#if HS_PWM_USE_TIM0
      if (&PWMD0 == pwmp) 
	sconfig.slave_id = TIMER0_DMA_ID;
#endif
#if HS_PWM_USE_TIM1
      if(&PWMD1 == pwmp)
	sconfig.slave_id = TIMER1_DMA_ID; 
#endif
#if HS_PWM_USE_TIM2
      if(&PWMD2 == pwmp)
	sconfig.slave_id = TIMER2_DMA_ID; 
#endif
        
      sconfig.direction = DMA_MEM_TO_DEV;
      sconfig.src_addr_width = DMA_SLAVE_BUSWIDTH_16BITS;
      sconfig.dst_addr_width = DMA_SLAVE_BUSWIDTH_16BITS;
      sconfig.src_burst = burst_len;
      sconfig.dst_burst = burst_len;
      sconfig.dev_flow_ctrl = FALSE;	
      sconfig.lli_en = 0;
      dmaStreamSetMode(pwmp->pwmdma, &sconfig);
          
  }else
#endif
 {

      /* Timer configured and started.*/

      pwmp->tim->CR1   = TIM_CR1_ARPE | TIM_CR1_URS |
          TIM_CR1_CEN;	
  }
  
}

/**
 * @brief   Deactivates the PWM peripheral.
 *
 * @param[in] pwmp      pointer to the @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_stop(PWMDriver *pwmp) {

    if (pwmp->state == PWM_READY) {
        /* Resets the peripheral.*/
    pwmp->tim->CR1  = 0;                    /* Timer disabled.              */
    pwmp->tim->DIER = 0;                    /* All IRQs disabled.           */
    pwmp->tim->SR   = 0;                    /* Clear eventual pending IRQs. */
    pwmp->tim->BDTR = 0;
    pwmp->tim->CCER = 0;
    pwmp->tim->ARR =  0;
    pwmp->tim->DCR = 0;
    pwmp->tim->PSC = 0;
    pwmp->tim->CCMR1 = 0;
    pwmp->tim->CCMR2 = 0;
    
    /* Disables the peripheral.*/
#if HS_PWM_USE_TIM0
    if (&PWMD0 == pwmp) {
      nvicDisableVector(IRQ_TIM0);
      cpmDisableTIM0();
    }
#endif /* HS_PWM_USE_TIM0 */
#if HS_PWM_USE_TIM1
    if (&PWMD1 == pwmp) {
      nvicDisableVector(IRQ_TIM1);
      cpmDisableTIM1();
    }
#endif /* HS_PWM_USE_TIM1 */
#if HS_PWM_USE_TIM2
    if (&PWMD2 == pwmp) {
      nvicDisableVector(IRQ_TIM2);
      cpmDisableTIM2();
    }
#endif /* HS_PWM_USE_TIM2 */

#if HS_PWM_USE_TIM3
    if (&PWMD3 == pwmp) {
      nvicDisableVector(IRQ_TIM3);
      cpmDisableTIM3();
    }
#endif /* HS_PWM_USE_TIM3 */
  }
#if PWM_USE_DMA         
    if ((pwmp->config->useDMA == true) && (pwmp->pwmdma != NULL))
    dmaStreamRelease(pwmp->pwmdma);
#endif
}

/**
 * @brief   Enables a PWM channel.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @post    The channel is active using the specified configuration.
 * @note    Depending on the hardware implementation this function has
 *          effect starting on the next cycle (recommended implementation)
 *          or immediately (fallback implementation).
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] channel   PWM channel identifier (0...PWM_CHANNELS-1)
 * @param[in] width     PWM pulse width as clock pulses number
 *
 * @notapi
 */
void pwm_lld_enable_channel(PWMDriver *pwmp,
                            pwmchannel_t channel,
                            pwmcnt_t width) {
#if PWM_USE_DMA       
    if ((pwmp->config->useDMA == true) && (pwmp->pwmdma != NULL)){
        dmaStreamStart(pwmp->pwmdma, pwmp->config->pwmDMAinfo->memAddr, 
            &pwmp->tim->DMAR, pwmp->config->pwmDMAinfo->blockSize);
        
   //       pwmp->tim->CCR[channel] = width-1;
          pwmp->tim->CR1   = TIM_CR1_CEN | TIM_CR1_ARPE;	
          pwmp->tim->EGR   = TIM_EGR_UG;      /* Update event.                */

    }else
#endif
    {
         pwmp->tim->CCR[channel] = width;}                 /* New duty cycle.      */
  
    /* If there is a callback defined for the channel then the associated
     interrupt must be enabled.*/
  if (pwmp->config->channels[channel].callback != NULL) {
    uint32_t dier = pwmp->tim->DIER;
    /* If the IRQ is not already enabled care must be taken to clear it,
       it is probably already pending because the timer is running.*/
    if ((dier & (2 << channel)) == 0) {
      pwmp->tim->DIER = dier | (2 << channel);
      pwmp->tim->SR   = ~(2 << channel);
    }
  }
  else {
    pwmp->tim->DIER &= ~(2 << channel);
    pwmp->tim->SR   = ~(2 << channel);
  }
}

/**
 * @brief   Disables a PWM channel.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @post    The channel is disabled and its output line returned to the
 *          idle state.
 * @note    Depending on the hardware implementation this function has
 *          effect starting on the next cycle (recommended implementation)
 *          or immediately (fallback implementation).
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] channel   PWM channel identifier (0...PWM_CHANNELS-1)
 *
 * @notapi
 */
void pwm_lld_disable_channel(PWMDriver *pwmp, pwmchannel_t channel) {

  pwmp->tim->CCR[channel] = 0;
  pwmp->tim->DIER &= ~(2 << channel);

#if PWM_USE_DMA 
  if ((pwmp->config->useDMA == true) && (pwmp->pwmdma != NULL)) {
      dmaStreamDisable(pwmp->pwmdma);
  }
#endif
}

#endif /* HAL_USE_PWM */

/** @} */
