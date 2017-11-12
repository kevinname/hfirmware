!********************************************************************************************************
!
!                               (c) Copyright 2015-2018, Huntersun Techonology
!                                          All Rights Reserved
!
!                                           NDS32 Generic Port
!                                            GNU C Compiler
!
!********************************************************************************************************

!********************************************************************************************************
! INCLUDE ASSEMBLY CONSTANTS
!********************************************************************************************************

#include <nds32_init.inc>
#include "n12_def.h"
#include "common.h"		

#define PMU_PM_CON0		    0x400e001c
#define PMU_PM_CON1		    0x400e0020
#define PMU_PM_CON          PMU_PM_CON0
#define PMU_PSO_CON		    0x400e0004
#define PMU_PSO_CON_VAL		0x40080402

#define PSO_CPU_CFG		0x40001004
#define CPU_RESET_MASK		(1U << 7)

	.global __cpu_c_context
	.global __cpu_s_context

	.section .text
	
__cpu_c_context:	
	sethi $r8, hi20(PMU_PM_CON)
	ori   $r8, $r8, lo12(PMU_PM_CON)
	lw    $r0, [$r8]
	beqz  $r0, __c_context_exit

	mov55 $sp, $r0
	dsb
	
	popm  $r0, $r7
	mtsr  $r0, $int_mask
	mtsr  $r1, $int_mask2
	mtsr  $r2, $int_ctrl
	mtsr  $r3, $ivb
	mtsr  $r4, $sp_priv
	mtsr  $r5, $pfm_ctl
	mtsr  $r6, $int_pri
	mtsr  $r7, $int_pri2

	movi  $r0, 0
	sw    $r0, [$r8]
	
	RESTORE_ALL
	popm  $r0, $r1
	ret

__c_context_exit:
	ret

__cpu_s_context:
	pushm $r0, $r1
	SAVE_ALL

	mfsr  $r0, $int_mask
	mfsr  $r1, $int_mask2
	mfsr  $r2, $int_ctrl
	mfsr  $r3, $ivb
	mfsr  $r4, $sp_priv
	mfsr  $r5, $pfm_ctl
	mfsr  $r6, $int_pri
	mfsr  $r7, $int_pri2
	pushm $r0, $r7

	sethi $r0, hi20(PMU_PM_CON)
	ori   $r0, $r0, lo12(PMU_PM_CON)
	sw    $sp, [$r0]

	jal pmu_deep_sleep
	!sethi $r0, hi20(PMU_PSO_CON)
	!ori   $r0, $r0, lo12(PMU_PSO_CON)
	!sethi $r1, hi20(PMU_PSO_CON_VAL)
	!ori   $r1, $r0, lo12(PMU_PSO_CON_VAL)
	
	!reset mcu for test restore the sleeping context
	!movi  $r2, 1
	!slli  $r2, $r2, 7
	!sethi $r0, hi20(PSO_CPU_CFG)
	!ori   $r0, $r0, lo12(PSO_CPU_CFG)
	!lw    $r1, [$r0]
	!xor   $r1, $r1, $r2
	!sw    $r1, [$r0]
	!or    $r1, $r1, $r2
	!sw    $r1, [$r0]

	nop
	nop
	nop
	nop
	nop
	
.die:   b .die
	

