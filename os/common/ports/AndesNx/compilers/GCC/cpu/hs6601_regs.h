/*****************************************************************************
 *
 *            Copyright Andes Technology Corporation 2014
 *                         All Rights Reserved.
 *
 *  Revision History:
 *
 *    Jan.11.2014     Created.
 ****************************************************************************/

#ifndef __HS6601_REGS_H__ 
#define __HS6601_REGS_H__ 


/*****************************************************************************
 * IRQ Vector
 ****************************************************************************/
 

#define REG32(reg)               (  *( (volatile uint32_t *) (reg) ) )


/*****************************************************************************
 * ExLM - hs6601 AHB
 * **************************************************************************/
#define EILM_BASE       0x30000000
#define EDLM_BASE       0x10000000
#define SPI_MEM_BASE    0x50000000

#define SR_CLRB32(reg, bit) \
{     \
	int mask = __nds32__mfsr(reg)& ~(1<<bit); \
        __nds32__mtsr(mask, reg);           \
	__nds32__dsb(); \
}

#define SR_SETB32(reg,bit)\
{\
	int mask = __nds32__mfsr(reg)|(1<<bit);\
	__nds32__mtsr(mask, reg);     \
	__nds32__dsb();               \
}

#define SR_SETBS32(reg, s, e, bitval)      \
do{                                                 \
  uint32_t mask;                                    \
  mask = ((1u<<((e)-(s)+1)) - 1) << (s);              \
  __nds32__mtsr( ((__nds32__mfsr((reg)) & ~mask) | (((bitval)<<(s)) & mask)), reg);   \
  __nds32__dsb();       \
}while(0)

#if 0
#define SR_GETBS32(reg, s, e)\
{\
  uint32_t mask;                                    \
  mask = ((1u<<((e)-(s)+1)) - 1) << (s);              \
  (__nds32__mfsr((reg)) & mask) >> (s);   \
}
#else
static inline uint32_t SR_GETBS32(int reg, int s, int e)
{
	uint32_t mask;
	mask = ((1u<<((e)-(s)+1)) - 1) << (s);

	return (__nds32__mfsr((reg)) & mask) >> (s);
}
#endif

#endif /* __HS6601_REGS_INC__ */
