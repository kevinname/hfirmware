! Switch to interruption level \lv
	.macro IntlSwitch lv

	! mfsr    $r0, $PSW
	! li      $r1, #~PSW_mskINTL
	!	and     $r0, $r0, $r1
	! ori     $r0, $r0, #(\lv << PSW_offINTL)
	! mtsr    $r0, $PSW
	! isb

  mfsr      $r0, $PSW
#ifdef CONFIG_HWZOL
    /* Also enable ZOL (PSW.AEN) */
    xori    $r0, $r0, #((1 << 13) | (1 << 1))
#else
    addi    $r0, $r0, #-2
#endif
    mtsr    $r0, $PSW
	! isb

	.endm

	/* align $sp to 8B boundary */
	.macro align_sp_8 R0="$r2", R1="$r3"
    move    \R0, $sp        !keep original $sp to be pushed
#if !defined(__NDS32_ISA_V3M__) || 1 //could be optimized out
 #ifndef __NDS32_EXT_PERF__
    andi    \R1, $sp, #4    ! R1 = $sp.bit2 // 0 or 4
    subri   \R1, \R1, #4    ! R1 = 4 - R1   // 4 or 0
    sub     $sp, $sp, \R1   ! $sp -= R1     //-4 or 0
    push    \R0
 #else
    addi    $sp, $sp, #-4   ! $sp -= 4
    bclr    $sp, $sp, #2    ! $sp.bit2 = 0
    swi     \R0, [$sp]
 #endif
#endif
	.endm

	.macro push_d0d1
#if (defined(__NDS32_ISA_V2__) && defined(__NDS32_DX_REGS__)) || (!defined(__NDS32_ISA_V2__) && (defined(__NDS32_DIV__)||defined(__NDS32_MAC__)))
    mfusr   $r1, $d1.lo
    mfusr   $r2, $d1.hi
    mfusr   $r3, $d0.lo
    mfusr   $r4, $d0.hi
    pushm   $r1, $r4
#endif
	.endm

	.macro pop_d0d1
#if (defined(__NDS32_ISA_V2__) && defined(__NDS32_DX_REGS__)) || (!defined(__NDS32_ISA_V2__) && (defined(__NDS32_DIV__)||defined(__NDS32_MAC__)))
    popm    $r1, $r4
    mtusr   $r1, $d1.lo
    mtusr   $r2, $d1.hi
    mtusr   $r3, $d0.lo
    mtusr   $r4, $d0.hi
#endif
	.endm
	.macro push_ifc_lp
#ifdef __NDS32_EXT_IFC__
# ifndef CONFIG_NO_NDS32_EXT_IFC
    mfusr   $r0, $IFC_LP
    push    $r0
# endif
#endif
	.endm
	.macro pop_ifc_lp
#ifdef __NDS32_EXT_IFC__
# ifndef CONFIG_NO_NDS32_EXT_IFC
    pop     $r0
    mtusr   $r0, $IFC_LP
# endif
#endif
	.endm
	.macro push_zol
#ifdef CONFIG_HWZOL
    mfusr   $r1, $LB
    mfusr   $r2, $LE
    mfusr   $r3, $LC
    pushm   $r1, $r3
#endif
	.endm
	.macro pop_zol
#ifdef CONFIG_HWZOL
    popm    $r1, $r3
    mtusr   $r1, $LB
    mtusr   $r2, $LE
    mtusr   $r3, $LC
#endif
	.endm
	.macro push_sp
#ifdef __NDS32_EXT_IFC__
# ifndef CONFIG_NO_NDS32_EXT_IFC
    push $sp
# else
    addi $sp, $sp, #-4
    push $sp
# endif
#else
    addi $sp, $sp, #-4
    push $sp
#endif
	.endm

	.macro SAVE_FPU_REGS_00
    fsdi.bi $fd3, [$sp], -8
    fsdi.bi $fd2, [$sp], -8
    fsdi.bi $fd1, [$sp], -8
    fsdi    $fd0, [$sp+0]
	.endm

	.macro SAVE_FPU_REGS_01
    fsdi.bi $fd7, [$sp], -8
    fsdi.bi $fd6, [$sp], -8
    fsdi.bi $fd5, [$sp], -8
    fsdi.bi $fd4, [$sp], -8
    SAVE_FPU_REGS_00
	.endm

	.macro SAVE_FPU_REGS_02
    fsdi.bi $fd15, [$sp], -8
    fsdi.bi $fd14, [$sp], -8
    fsdi.bi $fd13, [$sp], -8
    fsdi.bi $fd12, [$sp], -8
    fsdi.bi $fd11, [$sp], -8
    fsdi.bi $fd10, [$sp], -8
    fsdi.bi $fd9, [$sp], -8
    fsdi.bi $fd8, [$sp], -8
    SAVE_FPU_REGS_01
	.endm

	.macro SAVE_FPU_REGS_03
    fsdi.bi $fd31, [$sp], -8
    fsdi.bi $fd30, [$sp], -8
    fsdi.bi $fd29, [$sp], -8
    fsdi.bi $fd28, [$sp], -8
    fsdi.bi $fd27, [$sp], -8
    fsdi.bi $fd26, [$sp], -8
    fsdi.bi $fd25, [$sp], -8
    fsdi.bi $fd24, [$sp], -8
    fsdi.bi $fd23, [$sp], -8
    fsdi.bi $fd22, [$sp], -8
    fsdi.bi $fd21, [$sp], -8
    fsdi.bi $fd20, [$sp], -8
    fsdi.bi $fd19, [$sp], -8
    fsdi.bi $fd18, [$sp], -8
    fsdi.bi $fd17, [$sp], -8
    fsdi.bi $fd16, [$sp], -8
    SAVE_FPU_REGS_02
	.endm

	.macro push_fpu
#if defined(__NDS32_EXT_FPU_CONFIG_0__)
    addi    $sp, $sp, -8
    SAVE_FPU_REGS_00
#elif defined(__NDS32_EXT_FPU_CONFIG_1__)
    addi    $sp, $sp, -8
    SAVE_FPU_REGS_01
#elif defined(__NDS32_EXT_FPU_CONFIG_2__)
    addi    $sp, $sp, -8
    SAVE_FPU_REGS_02
#elif defined(__NDS32_EXT_FPU_CONFIG_3__)
    addi    $sp, $sp, -8
    SAVE_FPU_REGS_03
#else
#endif
	.endm

	.macro RESTORE_FPU_REGS_00
    fldi.bi $fd0, [$sp], 8
    fldi.bi $fd1, [$sp], 8
    fldi.bi $fd2, [$sp], 8
    fldi.bi $fd3, [$sp], 8
	.endm

	.macro RESTORE_FPU_REGS_01
    RESTORE_FPU_REGS_00
    fldi.bi $fd4, [$sp], 8
    fldi.bi $fd5, [$sp], 8
    fldi.bi $fd6, [$sp], 8
    fldi.bi $fd7, [$sp], 8
	.endm

	.macro RESTORE_FPU_REGS_02
    RESTORE_FPU_REGS_01
    fldi.bi $fd8, [$sp], 8
    fldi.bi $fd9, [$sp], 8
    fldi.bi $fd10, [$sp], 8
    fldi.bi $fd11, [$sp], 8
    fldi.bi $fd12, [$sp], 8
    fldi.bi $fd13, [$sp], 8
    fldi.bi $fd14, [$sp], 8
    fldi.bi $fd15, [$sp], 8
	.endm

	.macro RESTORE_FPU_REGS_03
    RESTORE_FPU_REGS_02
    fldi.bi $fd16, [$sp], 8
    fldi.bi $fd17, [$sp], 8
    fldi.bi $fd18, [$sp], 8
    fldi.bi $fd19, [$sp], 8
    fldi.bi $fd20, [$sp], 8
    fldi.bi $fd21, [$sp], 8
    fldi.bi $fd22, [$sp], 8
    fldi.bi $fd23, [$sp], 8
    fldi.bi $fd24, [$sp], 8
    fldi.bi $fd25, [$sp], 8
    fldi.bi $fd26, [$sp], 8
    fldi.bi $fd27, [$sp], 8
    fldi.bi $fd28, [$sp], 8
    fldi.bi $fd29, [$sp], 8
    fldi.bi $fd30, [$sp], 8
    fldi.bi $fd31, [$sp], 8
	.endm

	.macro pop_fpu
#if defined(__NDS32_EXT_FPU_CONFIG_0__)
    RESTORE_FPU_REGS_00
#elif defined(__NDS32_EXT_FPU_CONFIG_1__)
    RESTORE_FPU_REGS_01
#elif defined(__NDS32_EXT_FPU_CONFIG_2__)
    RESTORE_FPU_REGS_02
#elif defined(__NDS32_EXT_FPU_CONFIG_3__)
    RESTORE_FPU_REGS_03
#else
#endif
	.endm




	.macro SAVE_ALL
    pushm $r2, $r30
    push_d0d1
    push_ifc_lp
    push_zol
    mfsr    $r0, $PSW
    mfsr    $r1, $IPC
    mfsr    $r2, $IPSW
    pushm   $r0, $r2

    /* Descend interrupt level */
    align_sp_8
    push_fpu
	.endm

	.macro RESTORE_ALL
    pop_fpu
    lwi     $sp, [$sp]
    popm    $r0, $r2  /* popm $r1 ~ $r2*/
    mtsr    $r0, $PSW
    mtsr    $r1, $IPC
    mtsr    $r2, $IPSW
    pop_zol
    pop_ifc_lp
    pop_d0d1
    popm	$r2, $r30
	.endm

	.macro CALLER_SAVE
    !pushm $r2, $r30
    pushm   $r2, $r5
    pushm   $r15,$r30   /* full: 16 gpr, reduce: 6 gpr*/
    push_d0d1           /* 4 reg, optional */
    push_ifc_lp         /* 1 reg, optional */
    push_zol
    mfsr    $r0, $PSW
    mfsr    $r1, $IPC
    mfsr    $r2, $IPSW
    pushm   $r0, $r2    /* 3 reg */

    !push_sp            /* 1 or 2 reg, depend on ifc on/off */
    !align_sp_8          /* already 8-byte align, needn't do again*/
    !push_fpu            /* optional */
	.endm

  .macro CALLER_RESTORE
    !pop_fpu
    !lwi     $sp, [$sp]
    popm    $r0, $r2        /* popm $r1 ~ $r2*/
    mtsr    $r1, $IPC
    mtsr    $r2, $IPSW
    pop_zol
    pop_ifc_lp
    pop_d0d1
    popm    $r15, $r30
    popm    $r2,  $r5
  .endm

	.macro WEAK_DEFAULT weak_sym
    .weak \weak_sym
  .endm

  .macro CallFn fn
    push    $lp
    addi    $sp, $sp, -4  ! keep 8 byte align
    bal     \fn
    addi    $sp, $sp, 4
    pop     $lp
  .endm

! Define standard NDS32 vector table entry point of interruption vectors
	.macro VECTOR handler
	.align 4
    pushm   $r0, $r1
    sethi   $r0, hi20(\handler)
    ori     $r0, $r0, lo12(\handler)
    jral    $r0, $r0
  .endm

  .macro VNOP
  .align 4
    ISB
    ISB
    ISB
    ISB
  .endm

	.macro IntJmp Handler
    CALLER_SAVE    
    IntlSwitch #0

    la      $r0, OSIntNesting
    lbi     $r1, [$r0]
    addi    $r1, $r1, 1
    swi     $r1, [$r0]

    addi10.sp #-104
    la      $r0, ctxp    
    swi     $sp, [$r0]

    mfsr $r0,$misc_ctl
    ori $r0,$r0,#0x10
    mtsr $r0,$misc_ctl
    dsb

    setgie.e

    sethi   $r0, hi20(\Handler)
    ori     $r0, $r0, lo12(\Handler)
    jral    $r0

    setgie.d

    la      $r0, OSIntNesting
    lbi     $r0, [$r0]
    slti45 $r0,#0x2
    beqzs8  #0x12

    mfsr $r0,$misc_ctl
    bitci $r0,$r0,#0x10
    mtsr $r0,$misc_ctl
    dsb
    
    la      $r0, ctxp
    lw      $r0, [$r0]
    bnez    $r0, #4
    addi10.sp #104

    la      $r0, OSIntNesting
    lbi     $r1, [$r0]
    addi    $r1, $r1, -1
    swi     $r1, [$r0]  

    CALLER_RESTORE
    popm    $r0, $r1
    iret
	.endm





