/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio.

    This file is part of ChibiOS.

    ChibiOS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file    chcore_n10.c
 * @brief   Andes-N architecture port code.
 *
 * @addtogroup AndesN10_CORE
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "nds32_defs.h"
#include "tick_timer.h"
#include "string.h"

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/
struct port_extctx *ctxp = NULL;
/*===========================================================================*/
/* Module local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/
static struct port_extctx *ctxp_swi;
/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module interrupt handlers.                                                */
/*===========================================================================*/
/**
 * @brief   System Timer vector.
 * @details This interrupt is used as system tick.
 * @note    The timer must be initialized in the startup code.
 */
CH_IRQ_HANDLER(TICK_IRQHandler) {

  CH_IRQ_PROLOGUE();

  chSysLockFromISR();
  port_systick_clearIrq();
  chSysTimerHandlerI();
  chSysUnlockFromISR();

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   PendSV vector.
 * @details The PendSV vector is used for exception mode re-entering after a
 *          context switch.
 * @note    The PendSV vector is only used in compact kernel mode.
 */
/*lint -save -e9075 [8.4] All symbols are invoked from asm context.*/
CH_IRQ_HANDLER(SWI_IRQHandler) {  

  SR_CLRB32(NDS32_SR_INT_PEND, INT_PEND_offSWI);  

#if CORTEX_USE_FPU
  /* Enforcing unstacking of the FP part of the context.*/
  FPU->FPCCR &= ~FPU_FPCCR_LSPACT_Msk;
#endif

  /* The port_extctx structure is pointed by the PSP register.*/
  ctxp_swi = (struct port_extctx *)__get_PSP();

  /* Discarding the current exception context and positioning the stack to
     point to the real one.*/
  ctxp_swi ++;

  /* Writing back the modified PSP value.*/
  __set_PSP((uint32_t)ctxp_swi);

  ctxp = NULL;
}

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Exception exit redirection to _port_switch_from_isr().
 */
void _port_irq_epilogue(void) {

  if(ctxp == NULL)
    return ;
  else
    ctxp->ipc = (regarm_t)0;

  port_lock_from_isr();
  if (OSIntNesting <= 1) {    
    struct port_extctx *ctxp_tmp = &ctxp[1];
    memcpy(ctxp, ctxp_tmp, sizeof(struct port_extctx));

    /* The exit sequence is different depending on if a preemption is
       required or not.*/
    if (chSchIsPreemptionRequired()) {
      /* Preemption is required we need to enforce a context switch.*/
      ctxp->ipc = (regarm_t)_port_switch_from_isr;
      ctxp->ipsw &= ~0x8001u;
    }
    else {
      /* Preemption not required, we just need to exit the exception
         atomically.*/
      //ctxp->ipc = (regarm_t)_port_exit_from_isr;
      ctxp = NULL;
    }

    /* Note, returning without unlocking is intentional, this is done in
       order to keep the rest of the context switch atomic.*/
    return;
  }
  else {
    ctxp = NULL;
  }

  port_unlock_from_isr();
}

/** @} */
