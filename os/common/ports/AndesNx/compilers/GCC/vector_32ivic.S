!********************************************************************************************************
!
!                               (c) Copyright 2005-2014, Andes Techonology
!                                          All Rights Reserved
!
!                                           NDS32 Generic Port
!                                            GNU C Compiler
!
!********************************************************************************************************

!********************************************************************************************************
! INCLUDE ASSEMBLY CONSTANTS
!********************************************************************************************************

#include "n12_def.h"
#include "common.h"
		
	.global ANS_Int_Vectors
	.global ANS_Int_Vectors_End	
	
!********************************************************************************************************
! 	Interrupt Entry Table
!********************************************************************************************************

	.section .nds32_init, "ax"

ANS_Int_Vectors:
	b	OS_Init_Nds32			!  (0) Trap Reset/NMI
	VNOP          !HANDLE_TLB_Fill		        !  (1) Trap TLB fill
	VECTOR	HANDLE_PTE_Not_Present		!  (2) Trap PTE not present
	VECTOR	HANDLE_TLB_Misc		        !  (3) Trap TLB misc
	VECTOR	HANDLE_TLB_VLPT_Miss		!  (4) Trap TLB VLPT miss
	VECTOR	HANDLE_Machine_Error		!  (5) Trap Machine error
	VECTOR	HANDLE_Debug_Related		!  (6) Trap Debug related
	VECTOR	HANDLE_General_Exception	!  (7) Trap General exception
	VECTOR	HANDLE_Syscall			    !  (8) Syscall

    VECTOR  HANDLE_HW0           !  (9) Interrupt HW0   
    VECTOR  HANDLE_HW1           ! (10) Interrupt HW1
    VECTOR  HANDLE_HW2           ! (11) Interrupt HW2
    VECTOR  HANDLE_HW3           ! (12) Interrupt HW3
    VECTOR  HANDLE_HW4           ! (13) Interrupt HW4
    VECTOR  HANDLE_HW5           ! (14) Interrupt HW5
    VECTOR  HANDLE_HW6           ! (15) Interrupt HW6    
    VECTOR  HANDLE_HW7           ! (16) Interrupt HW7
    VECTOR  HANDLE_HW8           ! (17) Interrupt HW8
    VECTOR  HANDLE_HW9           ! (18) Interrupt HW9
    VECTOR  HANDLE_HW10          ! (19) Interrupt HW10
    VECTOR  HANDLE_HW11          ! (20) Interrupt HW11
    VECTOR  HANDLE_HW12          ! (21) Interrupt HW12
    VECTOR  HANDLE_HW13          ! (22) Interrupt HW13
    VECTOR  HANDLE_HW14          ! (23) Interrupt HW14
    VECTOR  HANDLE_HW15          ! (24) Interrupt HW15
    VECTOR  HANDLE_HW16          ! (25) Interrupt HW16
    VECTOR  HANDLE_HW17          ! (26) Interrupt HW17
    VECTOR  HANDLE_HW18          ! (27) Interrupt HW18
    VECTOR  HANDLE_HW19          ! (28) Interrupt HW19
    VECTOR  HANDLE_HW20          ! (29) Interrupt HW20
    VECTOR  HANDLE_HW21          ! (30) Interrupt HW21
    VECTOR  HANDLE_HW22          ! (31) Interrupt HW22
    VECTOR  HANDLE_HW23          ! (32) Interrupt HW23
    VECTOR  HANDLE_HW24          ! (33) Interrupt HW24
    VECTOR  HANDLE_HW25          ! (34) Interrupt HW25
    VECTOR  HANDLE_HW26          ! (35) Interrupt HW26
    VECTOR  HANDLE_HW27          ! (36) Interrupt HW27
    VECTOR  HANDLE_HW28          ! (37) Interrupt HW28
    VECTOR  HANDLE_HW29          ! (38) Interrupt HW29
    VECTOR  HANDLE_HW30          ! (39) Interrupt HW30
    VECTOR  HANDLE_HW31          ! (40) Interrupt HW31
ANS_Int_Vectors_End:


	.section .ramtext
HANDLE_HW0:
	IntJmp Vector00
HANDLE_HW18:
	IntJmp Vector18
HANDLE_HW31:
	IntJmp Vector31

	.section .text
HANDLE_TLB_Fill:
die1:
	b die1
HANDLE_PTE_Not_Present:
die2:
	b die2
HANDLE_TLB_Misc:
die3:
	b die3
HANDLE_TLB_VLPT_Miss:
die4:
	b die4
HANDLE_Machine_Error:
die5:
	b die5
HANDLE_Debug_Related:
die6:
	b die6
HANDLE_General_Exception:
die7:
	b die7
HANDLE_Syscall:
die8:
	b die8

! Interrupt wrapper function

HANDLE_HW1:
	IntJmp Vector01
HANDLE_HW2:
	IntJmp Vector02
HANDLE_HW3:
	IntJmp Vector03
HANDLE_HW4:
	IntJmp Vector04
HANDLE_HW5:
	IntJmp Vector05
HANDLE_HW6:
	IntJmp Vector06
HANDLE_HW7:
	IntJmp Vector07
	
HANDLE_HW8:
	IntJmp Vector08
HANDLE_HW9:
	IntJmp Vector09
HANDLE_HW10:
	IntJmp Vector10
HANDLE_HW11:
	IntJmp Vector11
HANDLE_HW12:
	IntJmp Vector12
HANDLE_HW13:
	IntJmp Vector13
HANDLE_HW14:
	IntJmp Vector14
HANDLE_HW15:
	IntJmp Vector15

HANDLE_HW16:
	IntJmp Vector16
HANDLE_HW17:
	IntJmp Vector17
HANDLE_HW19:
	IntJmp Vector19
HANDLE_HW20:
	IntJmp Vector20
HANDLE_HW21:
	IntJmp Vector21
HANDLE_HW22:
	IntJmp Vector22
HANDLE_HW23:
	IntJmp Vector23

HANDLE_HW24:
	IntJmp Vector24
HANDLE_HW25:
	IntJmp Vector25
HANDLE_HW26:
	IntJmp Vector26
HANDLE_HW27:
	IntJmp Vector27
HANDLE_HW28:
	IntJmp Vector28
HANDLE_HW29:
	IntJmp Vector29
HANDLE_HW30:
	IntJmp Vector30



	.section .bss
.global OSIntNesting
	.data
	.align 3
	.skip 4
OSIntNesting:
	.end


