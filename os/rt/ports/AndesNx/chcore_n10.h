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
 * @file    chcore_N10.h
 * @brief   andes N10 architecture port macros and structures.
 *
 * @addtogroup andes N10_CORE
 * @{
 */

#ifndef _CHCORE_N10_H_
#define _CHCORE_N10_H_

#include <nds32_intrinsic.h>
#include "irq.h"
#include "hs6601_regs.h"
#include "core_an10.h"
#include "tick_timer.h"
#include "nvic.h"

#include "mcuconf.h"
/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/**
 * @brief   This port supports a realtime counter.
 */
#define PORT_SUPPORTS_RT                TRUE


/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   Stack size for the system idle thread.
 * @details This size depends on the idle thread implementation, usually
 *          the idle thread should take no more space than those reserved
 *          by @p PORT_INT_REQUIRED_STACK.
 * @note    In this port it is set to 16 because the idle thread does have
 *          a stack frame when compiling without optimizations. You may
 *          reduce this value to zero when compiling with optimizations.
 */
#if !defined(PORT_IDLE_THREAD_STACK_SIZE) || defined(__DOXYGEN__)
#define PORT_IDLE_THREAD_STACK_SIZE     256    //16
#endif

/**
 * @brief   Per-thread stack overhead for interrupts servicing.
 * @details This constant is used in the calculation of the correct working
 *          area size.
 * @note    In this port this value is conservatively set to 64 because the
 *          function @p chSchDoReschedule() can have a stack frame, especially
 *          with compiler optimizations disabled. The value can be reduced
 *          when compiler optimizations are enabled.
 */
#if !defined(PORT_INT_REQUIRED_STACK) || defined(__DOXYGEN__)
#define PORT_INT_REQUIRED_STACK         256
#endif

/**
 * @brief   Enables the use of the WFI instruction in the idle thread loop.
 */
#if !defined(ANDES_ENABLE_WFI_IDLE)
#define ANDES_ENABLE_WFI_IDLE          TRUE
#endif

/**
 * @brief   FPU support in context switch.
 * @details Activating this option activates the FPU support in the kernel.
 */
#if !defined(ANDES_USE_FPU)
#define ANDES_USE_FPU                  ANDES_HAS_FPU
#elif (ANDES_USE_FPU == TRUE) && (ANDES_HAS_FPU == FALSE)
/* This setting requires an FPU presence check in case it is externally
   redefined.*/
#error "the selected core does not have an FPU"
#endif

/**
 * @brief   Simplified priority handling flag.
 * @details Activating this option makes the Kernel work in compact mode.
 *          In compact mode interrupts are disabled globally instead of
 *          raising the priority mask to some intermediate level.
 */
#if !defined(ANDES_SIMPLIFIED_PRIORITY)
#define ANDES_SIMPLIFIED_PRIORITY      FALSE
#endif

/**
 * @brief   SVCALL handler priority.
 * @note    The default SVCALL handler priority is defaulted to
 *          @p ANDES_MAXIMUM_PRIORITY+1, this reserves the
 *          @p ANDES_MAXIMUM_PRIORITY priority level as fast interrupts
 *          priority level.
 */
#if !defined(ANDES_PRIORITY_SVCALL)
#define ANDES_PRIORITY_SVCALL          (ANDES_MAXIMUM_PRIORITY + 1U)
#elif !PORT_IRQ_IS_VALID_PRIORITY(ANDES_PRIORITY_SVCALL)
/* If it is externally redefined then better perform a validity check on it.*/
#error "invalid priority level specified for ANDES_PRIORITY_SVCALL"
#endif

/**
 * @brief   NVIC VTOR initialization expression.
 */
#if !defined(ANDES_VTOR_INIT) || defined(__DOXYGEN__)
#define ANDES_VTOR_INIT                0x00000000U
#endif

/**
 * @brief   NVIC PRIGROUP initialization expression.
 * @details The default assigns all available priority bits as preemption
 *          priority with no sub-priority.
 */
#if !defined(ANDES_PRIGROUP_INIT) || defined(__DOXYGEN__)
#define ANDES_PRIGROUP_INIT            (7 - ANDES_PRIORITY_BITS)
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/
#define PORT_ARCHITECTURE_ANDES_N1086

/**
 * @brief   Name of the implemented architecture.
 */
#define PORT_ARCHITECTURE_NAME          "ANDES-N1086"

/**
 * @brief   Name of the architecture variant.
 */
#define PORT_CORE_VARIANT_NAME          "ANDES-N1086"

/**
 * @brief   Port-specific information string.
 */
#if (ANDES_SIMPLIFIED_PRIORITY == FALSE) || defined(__DOXYGEN__)
#define PORT_INFO                       "Advanced kernel mode"
#else
#define PORT_INFO                       "Compact kernel mode"
#endif
/** @} */

#if (ANDES_SIMPLIFIED_PRIORITY == FALSE) || defined(__DOXYGEN__)
/**
 * @brief   Maximum usable priority for normal ISRs.
 */
#define ANDES_MAX_KERNEL_PRIORITY      (ANDES_PRIORITY_SVCALL + 1U)

/**
 * @brief   BASEPRI level within kernel lock.
 */
#define ANDES_BASEPRI_KERNEL                                               \
  ANDES_PRIO_MASK(ANDES_MAX_KERNEL_PRIORITY)
#else

#define ANDES_MAX_KERNEL_PRIORITY      0U
#endif

/**
 * @brief   PendSV priority level.
 * @note    This priority is enforced to be equal to
 *          @p ANDES_MAX_KERNEL_PRIORITY, this handler always have the
 *          highest priority that cannot preempt the kernel.
 */
#define ANDES_PRIORITY_SWI          0

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/* The following code is not processed when the file is included from an
   asm module.*/
#if !defined(_FROM_ASM_)

/* The documentation of the following declarations is in chconf.h in order
   to not have duplicated structure names into the documentation.*/
#if !defined(__DOXYGEN__)
struct port_extctx {
  regarm_t      psw;
  regarm_t      ipc;
  regarm_t      ipsw;  
  regarm_t      ifc_lp;
  regarm_t      lp;   //r30;
  regarm_t      gp;   //r29;
  regarm_t      fp;   //r28;
  regarm_t      r27;
  regarm_t      r26;
  regarm_t      r25;
  regarm_t      r24;
  regarm_t      r23;
  regarm_t      r22;
  regarm_t      r21;
  regarm_t      r20;
  regarm_t      r19;
  regarm_t      r18;
  regarm_t      r17;
  regarm_t      r16;
  regarm_t      r15; 
  
  regarm_t      r5;
  regarm_t      r4;
  regarm_t      r3;
  regarm_t      r2;
  regarm_t      r1;
  regarm_t      r0;
};

struct port_intctx {
  /* smw.adm $r6, [$sp], $r15, #0xa */
  /* lmw.bim $r6, [$sp], $r15, #0xa */
  regarm_t      r6;
  regarm_t      r7;
  regarm_t      r8;
  regarm_t      r9;
  regarm_t      r10;
  regarm_t      r11;
  regarm_t      r12;
  regarm_t      r13;
  regarm_t      r14;
  regarm_t      r15;
  regarm_t      fp;   //r27
  regarm_t      lp;   //r30;
};
#endif /* !defined(__DOXYGEN__) */

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/
/**
 * @brief   Platform dependent part of the @p chThdCreateI() API.
 * @details This code usually setup the context switching frame represented
 *          by an @p port_intctx structure.
 */
#define PORT_SETUP_CONTEXT(tp, workspace, wsize, pf, arg) {                 \
  (tp)->p_ctx.r13 = (struct port_intctx *)((uint8_t *)(workspace) +         \
                                           (size_t)(wsize) -                \
                                           sizeof(struct port_intctx));     \
  (tp)->p_ctx.r13->r6 = (regarm_t)(pf);                                     \
  (tp)->p_ctx.r13->r7 = (regarm_t)(arg);                                    \
  (tp)->p_ctx.r13->lp = (regarm_t)_port_thread_start;                       \
}

/**
 * @brief   Computes the thread working area global size.
 * @note    There is no need to perform alignments in this macro.
 */
#define PORT_WA_SIZE(n) (sizeof(struct port_intctx) +                       \
                         sizeof(struct port_extctx) +                       \
                         ((size_t)(n)) + ((size_t)(PORT_INT_REQUIRED_STACK)))                         

#define PORT_IRQ_PROLOGUE()
#define PORT_IRQ_EPILOGUE()       _port_irq_epilogue();

/**
 * @brief   IRQ handler function declaration.
 * @note    @p id can be a function name or a vector number depending on the
 *          port implementation.
 */
#define PORT_IRQ_HANDLER(id) void id(void)

/**
 * @brief   Fast IRQ handler function declaration.
 * @note    @p id can be a function name or a vector number depending on the
 *          port implementation.
 */
#define PORT_FAST_IRQ_HANDLER(id) void id(void)

/**
 * @brief   Performs a context switch between two threads.
 * @details This is the most critical code in any port, this function
 *          is responsible for the context switch between 2 threads.
 * @note    The implementation of this code affects <b>directly</b> the context
 *          switch performance so optimize here as much as you can.
 *
 * @param[in] ntp       the thread to be switched in
 * @param[in] otp       the thread to be switched out
 */
#if (CH_DBG_ENABLE_STACK_CHECK == FALSE) || defined(__DOXYGEN__)
#define port_switch(ntp, otp) _port_switch(ntp, otp)
#else
#define port_switch(ntp, otp) {                                             \
  struct port_intctx *r13 = (struct port_intctx *)__get_PSP();              \
  if ((stkalign_t *)(r13 - 1) < (otp)->p_stklimit) {                        \
    chSysHalt("stack overflow");                                            \
  }                                                                         \
  _port_switch(ntp, otp);                                                   \
}
#endif

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void _port_irq_epilogue(void) __ONCHIP_CODE__;
  void _port_switch(thread_t *ntp, thread_t *otp);
  void _port_thread_start(void);
  void _port_switch_from_isr(void);
  void _port_exit_from_isr(void);
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

/**
 * @brief   Port-related initialization code.
 */
static inline void port_init(void) {
  extern char __vector_base__[];
  /* Initialization of the vector table and priority related settings.*/
  __nds32__mtsr(__nds32__mfsr(NDS32_SR_IVB) | (uint32_t)__vector_base__, NDS32_SR_IVB);

  /* Initializing priority grouping.*/
  hal_intc_irq_set_priority(0xFFFFFFFF,0xFFFFFFFF);
  hal_intc_irq_clean_all();

  port_systick_init();

  /* DWT cycle counter enable.*/
  SR_SETB32(NDS32_SR_PRUSR_ACC_CTL, 1);
  __nds32__mtsr(0x4408007, NDS32_SR_PFM_CTL); // ins, total cycles and I$ miss

  /* Initialization of the system vectors used by the port.*/
  nvicEnableVector(IRQ_SWINT, ANDES_PRIORITY_SWI);  
}

/**
 * @brief   Returns a word encoding the current interrupts status.
 *
 * @return              The interrupts status.
 */
static inline syssts_t port_get_irq_status(void) {
  
  return (__nds32__mfsr(NDS32_SR_PSW) & 1);
}

/**
 * @brief   Checks the interrupt status.
 *
 * @param[in] sts       the interrupt status word
 *
 * @return              The interrupt status.
 * @retvel false        the word specified a disabled interrupts status.
 * @retvel true         the word specified an enabled interrupts status.
 */
static inline bool port_irq_enabled(syssts_t sts) {

  return (sts & (syssts_t)1) == (syssts_t)1;
}

/**
 * @brief   Determines the current execution context.
 *
 * @return              The execution context.
 * @retval false        not running in ISR mode.
 * @retval true         running in ISR mode.
 */
static inline bool port_is_isr_context(void) {

  return (bool)(OSIntNesting > 0U);
}

/**
 * @brief   Kernel-lock action.
 * @details In this port this function raises the base priority to kernel
 *          level.
 */
static inline void port_lock(void) {
  __disable_irq();
}

/**
 * @brief   Kernel-unlock action.
 * @details In this port this function lowers the base priority to user
 *          level.
 */
static inline void port_unlock(void) {
  __enable_irq();
}

/**
 * @brief   Kernel-lock action from an interrupt handler.
 * @details In this port this function raises the base priority to kernel
 *          level.
 * @note    Same as @p port_lock() in this port.
 */
static inline void port_lock_from_isr(void) {

  port_lock();
}

/**
 * @brief   Kernel-unlock action from an interrupt handler.
 * @details In this port this function lowers the base priority to user
 *          level.
 * @note    Same as @p port_unlock() in this port.
 */
static inline void port_unlock_from_isr(void) {

  port_unlock();
}

/**
 * @brief   Disables all the interrupt sources.
 * @note    In this port it disables all the interrupt sources by raising
 *          the priority mask to level 0.
 */
static inline void port_disable(void) {

  __disable_irq();
}

/**
 * @brief   Disables the interrupt sources below kernel-level priority.
 * @note    Interrupt sources above kernel level remains enabled.
 * @note    In this port it raises/lowers the base priority to kernel level.
 */
static inline void port_suspend(void) {
  __disable_irq();
}

/**
 * @brief   Enables all the interrupt sources.
 * @note    In this port it lowers the base priority to user level.
 */
static inline void port_enable(void) {
  __enable_irq();
}

/**
 * @brief   Enters an architecture-dependent IRQ-waiting mode.
 * @details The function is meant to return when an interrupt becomes pending.
 *          The simplest implementation is an empty function or macro but this
 *          would not take advantage of architecture-specific power saving
 *          modes.
 * @note    Implemented as an inlined @p WFI instruction.
 */
static inline void port_wait_for_interrupt(void) {

#if ANDES_ENABLE_WFI_IDLE == TRUE
  __WFI();
#endif
}

/**
 * @brief   Returns the current value of the realtime counter.
 *
 * @return              The realtime counter value.
 */
static inline rtcnt_t port_rt_get_counter_value(void) {

  return __nds32__mfsr(NDS32_SR_PFMC1);
}

#endif /* !defined(_FROM_ASM_) */

#endif /* _CHCORE_V7M_H_ */

/** @} */
