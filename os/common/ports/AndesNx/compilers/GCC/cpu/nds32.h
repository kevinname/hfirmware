#ifndef __NDS32_BASE_H_
#define __NDS32_BASE_H_

#include <nds32_intrinsic.h>
#include "irq.h"
#include "n12_def.h"
#include "nds32_defs.h"
#include "nds32_regs.h"

#define CONFIG_CPU_ICACHE_ENABLE      1
#define CONFIG_CPU_DCACHE_ENABLE      1
//#define CONFIG_CPU_DCACHE_WRITETHROUGH 1
//#define CONFIG_CHECK_RANGE_ALIGNMENT  1

static inline void GIE_SAVE(unsigned long *var)
{
	*var = __nds32__mfsr(NDS32_SR_PSW);
	__nds32__gie_dis();
}

static inline void GIE_RESTORE(unsigned long var)
{
	if (var & PSW_mskGIE)
        __nds32__gie_en();
}

#endif
