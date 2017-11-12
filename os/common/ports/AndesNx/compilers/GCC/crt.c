
#include "hal.h" 

#include <nds32_intrinsic.h>
#include "n12_def.h"
#include "string.h"
#include "nds32_defs.h" 



extern unsigned char __main_stack__[];
extern unsigned char __ilm_base__[];

void _nds32_init_mem(void) __attribute__((no_prologue, optimize("Os")));
void _nds32_init_mem()
{
  __nds32__mtsr((unsigned int)__ilm_base__|0x1, NDS32_SR_ILMB);
  __nds32__isb();

  __nds32__mtsr((unsigned int)__main_stack__, NDS32_SR_SP_PRIV);
  
  //if (HS_ANA->REGS.LDO_DIG_ADJ != 0x3) 
  //{
      /* set DVDD12 to 1.3V */
  //    HS_ANA->REGS.LDO_DIG_ADJ = 0x3;
  //    cpmResetPSO();
  //};
}

void CRT_Mem_Init(void)
{
  #define MEMCPY(des, src, n)     __builtin_memcpy ((des), (src), (n))
  extern char __rw_lma_start[];
  extern char __rw_lma_end[];
  extern char __rw_vma_start[];  
  
  unsigned int size = __rw_lma_end - __rw_lma_start;
  MEMCPY(__rw_vma_start, __rw_lma_start, size);

  #ifdef RUN_IN_FLASH
  extern char __code_lma_start[];
  extern char __code_vma_start[];
  extern char __code_lma_end[];

  memset(__code_vma_start, 0, (__rw_vma_start - __code_vma_start));
  
  size = __code_lma_end - __code_lma_start;
  MEMCPY(__code_vma_start, __code_lma_start, size);

  extern char __init_lma_start[];
  extern char __vector_base__[];
  extern char __init_lma_end[];
  
  size = __init_lma_end - __init_lma_start;
  MEMCPY(__vector_base__, __init_lma_start, size);
  #endif
}

void CRT_Init(void)
{
	/*
 	 * Memory init
 	 */	 
	CRT_Mem_Init();

}

void _unhandled_exception(void) 
{
  while(1);
}

void Vector00(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector01(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector02(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector03(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector04(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector05(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector06(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector07(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector08(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector09(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector10(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector11(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector12(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector13(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector14(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector15(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector16(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector17(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector18(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector19(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector20(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector21(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector22(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector23(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector24(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector25(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector26(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector27(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector28(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector29(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector30(void) __attribute__((weak, alias("_unhandled_exception")));
void Vector31(void) __attribute__((weak, alias("_unhandled_exception")));




