/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio
                 Copyright (C) 2014 HunterSun Technologies
                 wei.lu@huntersun.com.cn

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    hs66xx/pal_lld.c
 * @brief   HS66xx PAL low level driver code.
 *
 * @addtogroup PAL
 * @{
 */

#include "ch.h"
#include "hal.h"

#if HAL_USE_PAL || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/
static ioevent_t gpio_event[PAL_IOPORTS_WIDTH * 2];
/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static void initgpio(HS_GPIO_Type *gpiop) {
  /* nTRST cannot reset GPIO */
  gpiop->INTENCLR = 0xffff;
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/
static void gpio_serve_interrupt(void)
{
  uint32_t i, src, src0, src1;  
  ioportid_t gio0 = IOPORT0;
  ioportid_t gio1 = IOPORT1;

  src = src0 = gio0->INTSTATUS & 0xffff;  
  for(i=0; i<PAL_IOPORTS_WIDTH; i++){
    if((src & 1) && (gpio_event[i] != NULL)){
      gpio_event[i](gio0, i);
    }

    src >>= 1;
  }
  gio0->INTSTATUS = src0;

  src = src1 = gio1->INTSTATUS & 0xffff;
  for(i=0; i<PAL_IOPORTS_WIDTH; i++){
    if((src & 1) && (gpio_event[PAL_IOPORTS_WIDTH+i] != NULL)){
      gpio_event[PAL_IOPORTS_WIDTH + i](gio1, i);
    }

    src >>= 1;
  }
  gio1->INTSTATUS = src1;  
}

CH_IRQ_HANDLER(GPIO_IRQHandler) {

  CH_IRQ_PROLOGUE();

  chSysLockFromISR();
  gpio_serve_interrupt();
  chSysUnlockFromISR();

  CH_IRQ_EPILOGUE();
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   HS66xx I/O ports configuration.
 *
 * @param[in] config    the HS66xx ports configuration
 *
 * @notapi
 */
void _pal_lld_init(void) {
  int i;
  /*
   * Enables the PAL related clocks.
   */
  cpmEnableGPIO();

  /*
   * Initial PAL setup.
   */
  initgpio(HS_GPIO0);
  initgpio(HS_GPIO1);

  for(i=0; i<PAL_IOPORTS_WIDTH * 2; i++)
    gpio_event[i] = NULL;
}

void _pal_lld_regevent(uint8_t pad, hs_gpio_inter_t lvl, ioevent_t event)
{
  ioportid_t gio;
  
  if(pad >= PAL_IOPORTS_WIDTH * 2)
    return ;

  if(event != NULL)
  {
    gpio_event[pad] = event;
    nvicEnableVector(IRQ_GPIO_COMBO, ANDES_PRIORITY_MASK(HS_GPIO_INTR_PRIORITY));
  }

  gio = pad < PAL_IOPORTS_WIDTH ? IOPORT0 : IOPORT1;
  pad = pad >= PAL_IOPORTS_WIDTH ? (pad - PAL_IOPORTS_WIDTH) : pad;

  gio->INTENCLR = 1u << pad ;
  gio->INTTYPECLR = ((1u <<(pad+16)) | (1u << pad)); /* first clear daul-edge */
  gio->INTPOLCLR = 1u << pad;

  switch(lvl) {
  case FALLING_EDGE:
    gio->INTTYPESET |= 1u << pad;
    break;
  case RISING_EDGE:
    gio->INTTYPESET |= 1u << pad;
    gio->INTPOLSET |= 1u << pad;
    break;
  case DUAL_EDGE:
    gio->INTTYPESET |= ((1u << pad)|(1u << (pad+16))); 
    break;
  case LOW_LEVEL:
    break;
  case HIGH_LEVEL:
    gio->INTPOLSET |= 1u << pad;
    break;
  default:
    return;
  }
  
  gio->INTENSET |= 1u << pad ;
}

/**
 * @brief   Pads mode setup.
 * @details This function programs a pads group belonging to the same port
 *          with the specified mode.
 * @note    @p PAL_MODE_UNCONNECTED is implemented as push pull at minimum
 *          speed.
 *
 * @param[in] port      the port identifier
 * @param[in] pad       pad number within the port
 * @param[in] mode      the mode
 *
 * @notapi
 */
void _pal_lld_setgroupmode(ioportid_t port,
                           ioportmask_t mask,
                           iomode_t mode) 
{
  uint32_t afrm, pupdrm, moderm, drcaprm, reg_num, reg_val, sd_pl_num = 6;
  uint16_t pad = 0 ;
  
	/* mode select */
  moderm = mode & PAL_HS_MODE_MASK;
	/* Alternate function */
  afrm = mode & PAL_HS_ALTERNATE_MASK;

	/* drive capability function */
  drcaprm = (mode & PAL_HS_DRCAP_MASK);

	/* power up or down - 300k */
  pupdrm = (mode & PAL_HS_PUDR_MASK);
    
  reg_val = moderm | afrm | drcaprm | pupdrm;

  do {
    if (pad > 15)
      break;
      
    if((mask&0x1) != 0) {
      reg_num = (port == IOPORT0) ? pad : (pad+16);
      
      HS_PMU->PADC_CON[reg_num] = reg_val;

      /* if use SD function set strong pull up */        
      switch (reg_num) {
        case 6:
          sd_pl_num = 0; 
          break;
        case 8:
          sd_pl_num = 1; 
          break;      
        case 10:
          sd_pl_num = 2; 
          break;      
        case 11:
          sd_pl_num = 3; 
          break;
        case 12:
          sd_pl_num = 4; 
          break;      
        case 13:
          sd_pl_num = 5; 
          break;      
        default:
          sd_pl_num = 6;
      }
         
      if (((afrm >> 5) == PAD_FUNC_SD_USB) && (sd_pl_num<6))
        HS_PMU->GPIO_PL_UP_30K |= (1<<sd_pl_num);       
      else if (sd_pl_num<6)
        HS_PMU->GPIO_PL_UP_30K &= ~(1<<sd_pl_num);

      if ((reg_num == 1) && ((afrm >> 5) != PAD_FUNC_JTAG))
        HS_PMU->RESET_EN = 0;
      else if ((reg_num == 1) && ((afrm >> 5) == PAD_FUNC_JTAG))
        HS_PMU->RESET_EN = 1;
        
	    /* if use gpio function  0x7 is alternate gpio */
      if ((afrm >> 5) == PAD_FUNC_GPIO) {
        uint16_t mask1 = (uint16_t)1<<pad;
        iomode_t real_mode = moderm + pupdrm;
    
        switch (real_mode) {
          case PAL_MODE_INPUT: 
          case PAL_MODE_INPUT_PULLUP:
          case PAL_MODE_INPUT_PULLDOWN:
            port->OUTENCLR = mask1;
            break;

          /* if output is selected, uses pull as initial output value */
          case PAL_MODE_OUTPUT_PULLUP:
            port->DATAOUT |= mask1;
            port->OUTENSET = mask1;
            break;
          case PAL_MODE_OUTPUT_PULLDOWN:
            port->DATAOUT &= ~mask1;
          case PAL_MODE_OUTPUT:
            port->OUTENSET = mask1;
            break;
        }
      }
    }
    
    pad += 1;
    mask >>= 1;
  }while(mask); 
}
#endif /* HAL_USE_PAL */

/** @} */
