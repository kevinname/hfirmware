#include <nds32_intrinsic.h>
#include "n12_def.h"
#include "nds32_defs.h" 
#include "stdint.h"
#include "hs6601_regs.h"
#include "irq.h"

/********************************
 * 	HAL Level : Interrupt
 ********************************/
/* 32IVIC without SOC INTC */

/*
 *	mask/unmask priority >= _irqs_ interrupts
 *	used in ISR & gie diable
 */
uint32_t hal_intc_irq_mask(int _irqs_)
{
  uint32_t prv_msk = __nds32__mfsr(NDS32_SR_INT_MASK2);
	
  if (_irqs_ == -1 )
  {
    __nds32__mtsr(0, NDS32_SR_INT_MASK2);
  }
  else if (_irqs_ < 32 )
  {
    SR_CLRB32(NDS32_SR_INT_MASK2,_irqs_);
  }
  else
  {
    return -1;
  }
  
  return prv_msk;
}

void hal_intc_irq_unmask(uint32_t _msk_)
{
	__nds32__mtsr( _msk_ , NDS32_SR_INT_MASK2);
}

#if 0
void hal_intc_irq_clean(uint32_t _irqs_)
{
	if ( _irqs_ == IRQ_SWI_VECTOR )
		SR_CLRB32(NDS32_SR_INT_PEND, INT_PEND_offSWI);
	/* PEND2 is W1C */
	SR_SETB32(NDS32_SR_INT_PEND2,_irqs_);
}
#endif

void hal_intc_irq_clean_all()
{
	__nds32__mtsr(-1,NDS32_SR_INT_PEND2);
}

void hal_intc_irq_disable(uint32_t _irqs_)
{
	SR_CLRB32(NDS32_SR_INT_MASK2,_irqs_);
}

void hal_intc_irq_disable_all()
{
	__nds32__mtsr(0x0,NDS32_SR_INT_MASK2);
}

void hal_intc_irq_enable(uint32_t _irqs_)
{
	SR_SETB32(NDS32_SR_INT_MASK2,_irqs_);
}

void hal_intc_irq_set_priority( uint32_t _prio1_, uint32_t _prio2_ )
{
	__nds32__mtsr(_prio1_, NDS32_SR_INT_PRI);
	__nds32__mtsr(_prio2_, NDS32_SR_INT_PRI2);
}

void hal_intc_swi_enable()
{
	SR_SETB32(NDS32_SR_INT_MASK2,IRQ_SWI_VECTOR); 
}

void hal_intc_swi_disable()
{
	SR_CLRB32(NDS32_SR_INT_MASK2,IRQ_SWI_VECTOR);
}

void hal_intc_swi_clean()
{
	SR_CLRB32(NDS32_SR_INT_PEND, INT_PEND_offSWI);  
}

void hal_intc_swi_trigger()
{
	SR_SETB32(NDS32_SR_INT_PEND,INT_PEND_offSWI);
}

uint32_t hal_intc_get_all_pend()
{
	return __nds32__mfsr(NDS32_SR_INT_PEND2); 
}

void hal_intc_init()
{
  hal_intc_swi_enable();
  hal_intc_irq_set_priority(0xFFFFFFFF,0xFFFFFFFF);

  __nds32__mtsr(__nds32__mfsr(NDS32_SR_IVB) | 0, NDS32_SR_IVB);
  //__nds32__mtsr(__nds32__mfsr(NDS32_SR_IVB) | 0x50000000, NDS32_SR_IVB);
}



