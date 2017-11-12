#ifndef __ANDES_IRQ_H_
#define __ANDES_IRQ_H_

#include <stdint.h>

typedef enum
{
  IRQ_BTBB          = 0,
  IRQ_CODEC         = 1,
  IRQ_DMA           = 2,
  IRQ_GPIO_COMBO    = 3,
  IRQ_TIMER_COMBO   = 4,
  IRQ_USBHOST       = 5,
  IRQ_SDHOST        = 6,
  IRQ_RTC           = 7,
  IRQ_UART0         = 8,
  IRQ_UART1         = 9,
  IRQ_I2C           = 10,
  IRQ_I2S           = 11,
  IRQ_ADC           = 12,
  IRQ_SPIM0         = 13,
  IRQ_SPIM1         = 14,
  IRQ_BTPHY         = 15,

  IRQ_I2S_RX        = 16,
  IRQ_I2S_TX        = 17,
  IRQ_SYSTICK       = 18,
  IRQ_CODEC_TX      = 19,
  IRQ_GPIO0         = 20,
  IRQ_GPIO1         = 21,
                    
  IRQ_TIM0          = 22,
  IRQ_TIM1          = 23,
  IRQ_TIM2          = 24,

  IRQ_USB_ACTIVE    = 25,
  IRQ_WDT           = 26,
  IRQ_SFLASH        = 27,

  IRQ_6200_RF_SPI   = 28,
  IRQ_6200_RF       = 29,
  IRQ_USB_DMA       = 30,
  IRQ_SWINT         = 31,
} IRQn_Type;

#define IRQ_TICK_VECTOR       IRQ_SYSTICK
#define IRQ_SWI_VECTOR        IRQ_SWINT

#define IRQ_EDGE_TRIGGER	1
#define IRQ_LEVEL_TRIGGER	0

#define IRQ_ACTIVE_HIGH   1
#define IRQ_ACTIVE_LOW    0


extern uint32_t OSIntNesting;



void hal_intc_init();
void hal_intc_swi_enable();
void hal_intc_swi_disable();
void hal_intc_swi_clean();
void hal_intc_swi_trigger();

uint32_t hal_intc_irq_mask(int _irqs_);
void hal_intc_irq_unmask(uint32_t _irqs_);
void hal_intc_irq_clean(uint32_t _irqs_);
void hal_intc_irq_clean_all();
void hal_intc_irq_enable(uint32_t _irqs_);
void hal_intc_irq_disable(uint32_t _irqs_);
void hal_intc_irq_disable_all();
void hal_intc_irq_set_priority(uint32_t _prio1_, uint32_t _prio2_);
void hal_intc_irq_config(uint32_t _irqs_, uint32_t _edge_, uint32_t _falling_);
uint32_t hal_intc_get_all_pend();


#endif
