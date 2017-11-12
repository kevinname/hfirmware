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
 * @file    compilers/GCC/chcoreasm_n10.s
 * @brief   AndesNx architecture port low level code.
 *
 * @addtogroup AndesNx_GCC_CORE
 * @{
 */

#if !defined(FALSE) || defined(__DOXYGEN__)
#define FALSE   0
#endif

#if !defined(TRUE) || defined(__DOXYGEN__)
#define TRUE    1
#endif

#define _FROM_ASM_
#include "chconf.h"

#if !defined(__DOXYGEN__)

                .section .ramtext

/*--------------------------------------------------------------------------*
 * Performs a context switch between two threads.
 *--------------------------------------------------------------------------*/
                .globl  _port_switch
_port_switch:
                smw.adm $r6, [$sp], $r15, #0xa

                dsb
                swi	$sp, [$r1+#12]
                lwi $sp, [$r0+#12]
                dsb

                lmw.bim $r6, [$sp], $r15, #0xa
                ret

				.section .text
/*--------------------------------------------------------------------------*
 * Start a thread by invoking its work function.
 *
 * Threads execution starts here, the code leaves the system critical zone
 * and then jumps into the thread function passed in register R4. The
 * register R5 contains the thread parameter. The function chThdExit() is
 * called on thread function return.
 *--------------------------------------------------------------------------*/
                .globl  _port_thread_start
_port_thread_start:
#if CH_DBG_SYSTEM_STATE_CHECK
                jal      _dbg_check_unlock
#endif
#if CH_DBG_STATISTICS
                jal      _stats_stop_measure_crit_thd
#endif
				setgie.e
                mov55   r0, r7
                jral    r6
                movi    r0, #0              /* MSG_OK */
                jal      chThdExit
                ret

/*--------------------------------------------------------------------------*
 * Post-IRQ switch code.
 *
 * Exception handlers return here for context switching.
 *--------------------------------------------------------------------------*/
                .globl  _port_switch_from_isr
_port_switch_from_isr:
#if CH_DBG_STATISTICS
                jal      _stats_start_measure_crit_thd
#endif
#if CH_DBG_SYSTEM_STATE_CHECK
                jal      _dbg_check_lock
#endif
                jal      chSchDoReschedule
#if CH_DBG_SYSTEM_STATE_CHECK
                jal      _dbg_check_unlock
#endif
#if CH_DBG_STATISTICS
                jal      _stats_stop_measure_crit_thd
#endif

                .globl  _port_exit_from_isr
_port_exit_from_isr:
                mfsr $r0,$int_pend
                bset $r0,$r0,#0x10
				mtsr $r0,$int_pend
				isb
				setgie.e

.L1:            b       .L1

				.end

#endif /* !defined(__DOXYGEN__) */

/** @} */
