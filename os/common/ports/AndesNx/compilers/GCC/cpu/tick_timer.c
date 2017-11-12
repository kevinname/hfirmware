#include <nds32_intrinsic.h>
#include "n12_def.h"
#include "nds32_defs.h" 
#include "tick_timer.h" 
#include "hs6601.h"
#include "irq.h"
#include "nvic.h"
#include "chconf.h"
#include "mcuconf.h"


static HS_SYS_Type *tickTm;

void port_systick_clearIrq(void)
{
  tickTm->SYS_TICK &= ~(1u << 24);
  //hal_intc_irq_clean(IRQ_TICK_VECTOR);
  tickTm->SYS_TICK |= 1u << 24;
}

void port_systick_init(void)
{
  tickTm = HS_SYS;
  port_systick_clearIrq();
  nvicEnableVector(IRQ_TICK_VECTOR, HS_TICK_IRQ_PRIORITY);

  tickTm->SYS_TICK  = MB_PCLK / CH_CFG_ST_FREQUENCY;
  tickTm->SYS_TICK |= 1u << 24;
}







