/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio
              Copyright (C) 2015 Huntersun Technologies
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
 * @file    hs66xx/gpt_lld.c
 * @brief   HS66xx GPT subsystem low level driver source.
 *
 * @addtogroup GPT
 * @{
 */

#include "hal.h"

#if (HAL_USE_GPT == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   GPTD0 driver identifier.
 */
#if HS_GPT_USE_TIM0 || defined(__DOXYGEN__)
GPTDriver GPTD0;
#endif

/**
 * @brief   GPTD1 driver identifier.
 */
#if HS_GPT_USE_TIM1 || defined(__DOXYGEN__)
GPTDriver GPTD1;
#endif

/**
 * @brief   GPTD2 driver identifier.
 */
#if HS_GPT_USE_TIM2 || defined(__DOXYGEN__)
GPTDriver GPTD2;
#endif

/**
 * @brief   GPTD3 driver identifier.
 */
#if HS_GPT_USE_TIM3 || defined(__DOXYGEN__)
GPTDriver GPTD3;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Shared IRQ handler.
 *
 * @param[in] gptp      pointer to a @p GPTDriver object
 */
static void gpt_lld_serve_interrupt(GPTDriver *gptp) {

  gptp->tim->SR = 0;
  if (gptp->state == GPT_ONESHOT) {
    gptp->state = GPT_READY;                /* Back in GPT_READY state.     */
    gpt_lld_stop_timer(gptp);               /* Timer automatically stopped. */
  }

  if(gptp->config->callback)
    gptp->config->callback(gptp);
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if HS_GPT_USE_TIM0
/**
 * @brief   TIM0 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM0_IRQHandler) {

  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD0);

  OSAL_IRQ_EPILOGUE();
}
#endif /* HS_GPT_USE_TIM0 */

#if HS_GPT_USE_TIM1
/**
 * @brief   TIM1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM1_IRQHandler) {

  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD1);

  OSAL_IRQ_EPILOGUE();
}
#endif /* HS_GPT_USE_TIM1 */

#if HS_GPT_USE_TIM2
/**
 * @brief   TIM2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM2_IRQHandler) {

  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD2);

  OSAL_IRQ_EPILOGUE();
}
#endif /* HS_GPT_USE_TIM2 */

#if HS_GPT_USE_TIM3
/**
 * @brief   TIM3 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIM3_IRQHandler) {

  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD3);

  OSAL_IRQ_EPILOGUE();
}
#endif /* HS_GPT_USE_TIM3 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level GPT driver initialization.
 *
 * @notapi
 */
void gpt_lld_init(void) {

#if HS_GPT_USE_TIM0
  /* Driver initialization.*/
  gptObjectInit(&GPTD0);
  GPTD0.tim = HS_TIM0;
#endif

#if HS_GPT_USE_TIM1
  /* Driver initialization.*/
  gptObjectInit(&GPTD1);
  GPTD1.tim = HS_TIM1;
#endif

#if HS_GPT_USE_TIM2
  /* Driver initialization.*/
  gptObjectInit(&GPTD2);
  GPTD2.tim = HS_TIM2;
#endif

#if HS_GPT_USE_TIM3
  /* Driver initialization.*/
  gptObjectInit(&GPTD3);
  GPTD3.tim = HS_TIM3;
#endif
}

/**
 * @brief   Configures and activates the GPT peripheral.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 *
 * @notapi
 */
void gpt_lld_start(GPTDriver *gptp) {
  uint16_t psc;

  if (gptp->state == GPT_STOP) {
    /* Enables the peripheral.*/
#if HS_GPT_USE_TIM0
    if (&GPTD0 == gptp) {
      cpmEnableTIM0();
      nvicEnableVector(IRQ_TIM0,
                       ANDES_PRIORITY_MASK(HS_GPT_TIM0_IRQ_PRIORITY));
      gptp->clock = cpm_get_clock(HS_TIM0_CLK);
    }
#endif /* HS_GPT_USE_TIM0 */
#if HS_GPT_USE_TIM1
    if (&GPTD1 == gptp) {
      cpmEnableTIM1();
      nvicEnableVector(IRQ_TIM1,
                       ANDES_PRIORITY_MASK(HS_GPT_TIM1_IRQ_PRIORITY));
      gptp->clock = cpm_get_clock(HS_TIM1_CLK);
    }
#endif /* HS_GPT_USE_TIM1 */
#if HS_GPT_USE_TIM2
    if (&GPTD2 == gptp) {
      cpmEnableTIM2();
      nvicEnableVector(IRQ_TIM2,
                       ANDES_PRIORITY_MASK(HS_GPT_TIM2_IRQ_PRIORITY));
      gptp->clock = cpm_get_clock(HS_TIM2_CLK);
    }
#endif /* HS_GPT_USE_TIM2 */
#if HS_GPT_USE_TIM3
    if (&GPTD3 == gptp) {
      cpmEnableTIM3();
      nvicEnableVector(IRQ_TIM3,
                       ANDES_PRIORITY_MASK(HS_GPT_TIM3_IRQ_PRIORITY));
      gptp->clock = cpm_get_clock(HS_TIM3_CLK);
    }
#endif /* HS_GPT_USE_TIM1 */
  }
  /* Configures the peripheral.*/
  /* Prescaler value calculation.*/
  psc = (uint16_t)((gptp->clock / gptp->config->frequency) - 1);
  osalDbgAssert(((uint32_t)(psc + 1) * gptp->config->frequency) == gptp->clock,
                "invalid frequency");

  /* Timer configuration.*/
  gptp->tim->CR1  = 0;                          /* Initially stopped.       */
  gptp->tim->CR2  = gptp->config->cr2;          /* DMA on UE (if any).      */
  gptp->tim->PSC  = psc;                        /* Prescaler value.         */
  gptp->tim->DIER = gptp->config->dier &        /* DMA-related DIER bits.   */
                    _TIM_DIER_IRQ_MASK;
  gptp->tim->SR   = 0;                          /* Clear pending IRQs.      */
}

/**
 * @brief   Deactivates the GPT peripheral.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 *
 * @notapi
 */
void gpt_lld_stop(GPTDriver *gptp) {

  if (gptp->state == GPT_READY) {
    /* Resets the peripheral.*/
    gptp->tim->CR1  = 0;                        /* Timer disabled.          */
    gptp->tim->DIER = 0;                        /* All IRQs disabled.       */
    gptp->tim->SR   = 0;                        /* Clear pending IRQs.      */

    /* Disables the peripheral.*/
#if HS_GPT_USE_TIM0
    if (&GPTD0 == gptp) {
      nvicDisableVector(IRQ_TIM0);
      cpmDisableTIM0();
    }
#endif /* HS_GPT_USE_TIM0 */
#if HS_GPT_USE_TIM1
    if (&GPTD1 == gptp) {
      nvicDisableVector(IRQ_TIM1);
      cpmDisableTIM1();
    }
#endif /* HS_GPT_USE_TIM1 */
#if HS_GPT_USE_TIM2
    if (&GPTD2 == gptp) {
      nvicDisableVector(IRQ_TIM2);
      cpmDisableTIM2();
    }
#endif /* HS_GPT_USE_TIM2 */
#if HS_GPT_USE_TIM3
    if (&GPTD3 == gptp) {
      nvicDisableVector(IRQ_TIM3);
      cpmDisableTIM3();
    }
#endif /* HS_GPT_USE_TIM3 */
  }
}

/**
 * @brief   Starts the timer in continuous mode.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 * @param[in] interval  period in ticks
 *
 * @notapi
 */
void gpt_lld_start_timer(GPTDriver *gptp, gptcnt_t interval) {

  gptp->tim->ARR   = (uint32_t)(interval - 1);  /* Time constant.           */
  gptp->tim->EGR   = TIM_EGR_UG;                /* Update event.            */
  gptp->tim->CNT   = 0;                         /* Reset counter.           */

  /* NOTE: After generating the UG event it takes several clock cycles before
     SR bit 0 goes to 1. This is because the clearing of CNT has been inserted
     before the clearing of SR, to give it some time.*/
  gptp->tim->SR    = 0;                         /* Clear pending IRQs.      */
  gptp->tim->DIER |= TIM_DIER_UIE;              /* Update Event IRQ enabled.*/
  gptp->tim->CR1   = TIM_CR1_URS | TIM_CR1_CEN;
}

/**
 * @brief   Stops the timer.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 *
 * @notapi
 */
void gpt_lld_stop_timer(GPTDriver *gptp) {

  gptp->tim->CR1   = 0;                         /* Initially stopped.       */
  gptp->tim->SR    = 0;                         /* Clear pending IRQs.      */

  /* All interrupts disabled.*/
  gptp->tim->DIER &= ~_TIM_DIER_IRQ_MASK;
}

/**
 * @brief   Starts the timer in one shot mode and waits for completion.
 * @details This function specifically polls the timer waiting for completion
 *          in order to not have extra delays caused by interrupt servicing,
 *          this function is only recommended for short delays.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 * @param[in] interval  time interval in ticks
 *
 * @notapi
 */
void gpt_lld_polled_delay(GPTDriver *gptp, gptcnt_t interval) {
  int i;

  gptp->tim->ARR  = (uint32_t)(interval - 1);   /* Time constant.           */
  gptp->tim->EGR  = TIM_EGR_UG;                 /* Update event.            */
  for(i = 0; (i < 1000) && !(gptp->tim->SR & TIM_SR_UIF); i++);   /* Wait UIF set */
  gptp->tim->SR   = 0;                          /* Clear pending IRQs.      */
  for(i = 0; (i < 1000) && (gptp->tim->SR != 0); i++);            /* Wait IRQs clear */
  gptp->tim->CR1  = TIM_CR1_OPM | TIM_CR1_URS | TIM_CR1_CEN;
  while (!(gptp->tim->SR & TIM_SR_UIF))
    ;
  gptp->tim->SR   = 0;
  for(i = 0; (i < 1000) && (gptp->tim->SR != 0); i++);            /* Wait IRQs clear */
}

#endif /* HAL_USE_GPT == TRUE */

/** @} */
