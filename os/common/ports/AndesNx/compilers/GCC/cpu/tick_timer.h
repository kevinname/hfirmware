#ifndef __TICK_TIMER_H_
#define __TICK_TIMER_H_

#include <stdint.h>

#define KHz         1000
#define MHz         1000000

#define MB_PCLK                 (32 * KHz)
#define MB_CNTCLK               (100 * KHz)

void port_systick_clearIrq(void);
void port_systick_init(void);

#endif
