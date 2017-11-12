/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio
                 Copyright (C) 2014 HunterSun Technologies
                 zutao.min@huntersun.com.cn

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

#include <string.h>
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "chrtclib.h"
#include "stdlib.h"

/*************************************************
gpio test: 
     gpio number     direction   logic
                     1 : input      1: high
                     0 : output     0: low
example:
    gpio  10         1                     
    gpio  20         0           0      
*************************************************/
bool_t gpio_changed= FALSE;
uint8_t gpio_change_val = 0;

static uint8_t read_gpio_val(uint8_t gpio_num){
  uint8_t val = 0;
  
  if (gpio_num < PAL_IOPORTS_WIDTH) {
    palSetPadMode(IOPORT0, gpio_num, PAL_MODE_INPUT|PAL_MODE_ALTERNATE(PAD_FUNC_GPIO)|PAL_MODE_DRIVE_CAP(3));
	val = (palReadPort(IOPORT0) >> gpio_num)&0x1;
  }else if(gpio_num < 2*PAL_IOPORTS_WIDTH) {
    palSetPadMode(IOPORT1, gpio_num-PAL_IOPORTS_WIDTH, PAL_MODE_INPUT|PAL_MODE_ALTERNATE(PAD_FUNC_GPIO)|PAL_MODE_DRIVE_CAP(3));
	val = (palReadPort(IOPORT1) >> (gpio_num-PAL_IOPORTS_WIDTH))&0x1;	
  }
  
  return val;
}

static void print_gpio_level(ioportid_t port, uint8_t pad) {
  uint8_t num = (port == IOPORT0)? pad : pad+16;
  gpio_change_val = read_gpio_val(num);
  gpio_changed = TRUE;
}
        
void cmd_gpio(BaseSequentialStream *chp, int argc, char *argv[]) {
	
  uint8_t gpio_num, direction, val, lev, err=0;

  if(argc == 1)
  {
    if(strcmp(argv[0], "?") == 0)
      chprintf(chp, "gpio pin dir [val]\r\n\tdir 0-output 1-input\r\n");
  }
          
  if (argc == 0) {
	  /* default test gpio 0 and gpio 16 */
	  palSetPadMode(IOPORT0, 1, PAL_MODE_INPUT|PAL_MODE_ALTERNATE(PAD_FUNC_GPIO)|PAL_MODE_DRIVE_CAP(3));
	  palSetPadMode(IOPORT1, 1, PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_GPIO)|PAL_MODE_DRIVE_CAP(3));
	  
	  palClearPad(IOPORT1, 1);
	  val = (palReadPort(IOPORT0) >> 1)&0x1;
	  if (val)
		  err = 1;
	  palSetPad(IOPORT1, 1);
	  val = (palReadPort(IOPORT0) >> 1)&0x1;
	  if (!val)
		  err = 1;
	  if(err)		  
		  chprintf(chp, "GPIO FAIL.\r\n");

	  else	  
		  chprintf(chp, "GPIO PASS.\r\n");
	  return;
  }
  gpio_num  = atoi(argv[0]) & 0xff;
  direction = atoi(argv[1]) & 0xff;
  
  if (gpio_num >= 2*PAL_IOPORTS_WIDTH)
     goto ERROR;
  
  if (direction == 1) {
      val = read_gpio_val(gpio_num); 
      chprintf(chp, "read gpio%d is %s\r\n", gpio_num, val?"high":"low");
      
      /* test gpio interrupt trigger */
      if (argc == 3) {
        lev = atoi(argv[2]) & 0xff;
        palRegEvent(gpio_num, (hs_gpio_inter_t)lev, (ioevent_t)print_gpio_level);
        //while(!gpio_changed);
        //gpio_changed = TRUE;
        //chprintf(chp, "read gpio%d is %s\r\n", gpio_num, gpio_change_val?"high":"low");
      }
  } else if (direction == 0) {
	  
	  val = atoi(argv[2]) & 0xff;	  
	  if (gpio_num < PAL_IOPORTS_WIDTH) {
		  palSetPadMode(IOPORT0, gpio_num, PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_GPIO)|PAL_MODE_DRIVE_CAP(3));
		  if (val)
			  palSetPad(IOPORT0, gpio_num);
		  else
			  palClearPad(IOPORT0, gpio_num);
	  } else if (gpio_num < 2*PAL_IOPORTS_WIDTH) {
		  palSetPadMode(IOPORT1, gpio_num-PAL_IOPORTS_WIDTH, PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_GPIO)|PAL_MODE_DRIVE_CAP(3));
		  if (val)
			  palSetPad(IOPORT1, gpio_num-PAL_IOPORTS_WIDTH);
		  else
			  palClearPad(IOPORT1, gpio_num-PAL_IOPORTS_WIDTH);
	  } else
		  goto ERROR;	  
  } else 
	  goto ERROR;	  
  chThdSleepMilliseconds(100);
  return;
   
ERROR:
  
  chprintf(chp, "Usage: gpio [number] [direction] [level]\r\n"); 

  chprintf(chp, "Example: gpio 10 0 1 -- set gpio 10 output = 1\r\n" 
                "Example: gpio 20 1 [0,1,2,3]  -- set gpio 10 input\r\n");
}

void cmd_pin(BaseSequentialStream *chp, int argc, char *argv[]) {
  char *action= "";
  int error = 0, pin;

  if (argc < 2) {
    error = 1;
    goto label_out;
  }

  pin = atoi(argv[0]);
  action = argv[1];
  if ((pin < 0) || (pin > 31)) {
    /* dump all pin-mux */
    uint32_t data = (HS_GPIO1->DATA << 16) | HS_GPIO0->DATA;
    uint32_t out = (HS_GPIO1->DATAOUT << 16) | HS_GPIO0->DATAOUT;
    for (pin = 0; pin < 24; pin++) {
      uint32_t padc = HS_PMU->PADC_CON[pin];
      chprintf(chp, "pin%02d: 0x%08lx mux=%2u input=%u pull=%u drv=%u; data=%u out=%u\r\n",
               pin, padc, padc>>5, padc&0x1, (padc>>1)&0x3, (padc>>3)&0x3,
               (data>>pin)&0x1, (out>>pin)&0x1);
    }
    return;
  }

  argc--;
  if (strncmp(action, "set", strlen("set")) == 0)
  {
    if (pin < 16) {
      palSetPadMode(IOPORT0, pin, PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_GPIO)|PAL_MODE_DRIVE_CAP(3));
      palSetPad(IOPORT0, pin);
    } else {
      pin -= 16;
      palSetPadMode(IOPORT1, pin, PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_GPIO)|PAL_MODE_DRIVE_CAP(3));
      palSetPad(IOPORT1, pin);
    }
  }
  else if (strncmp(action, "clr", strlen("clr")) == 0)
  {
    if (pin < 16) {
      palSetPadMode(IOPORT0, pin, PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_GPIO)|PAL_MODE_DRIVE_CAP(3));
      palClearPad(IOPORT0, pin);
    } else {
      pin -= 16;
      palSetPadMode(IOPORT1, pin, PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_GPIO)|PAL_MODE_DRIVE_CAP(3));
      palClearPad(IOPORT1, pin);
    }
  }
  else if (strncmp(action, "get", strlen("get")) == 0)
  {
    int level;
    if (pin < 16) {
      palSetPadMode(IOPORT0, pin, PAL_MODE_INPUT|PAL_MODE_ALTERNATE(PAD_FUNC_GPIO)|PAL_MODE_DRIVE_CAP(3));
      level = palReadPad(IOPORT0, pin);
    } else {
      pin -= 16;
      palSetPadMode(IOPORT1, pin, PAL_MODE_INPUT|PAL_MODE_ALTERNATE(PAD_FUNC_GPIO)|PAL_MODE_DRIVE_CAP(3));
      level = palReadPad(IOPORT1, pin);
    }
    chprintf(chp, "pin%d is %s\r\n", pin, level ? "High" : "Low");
  }
  else if (strncmp(action, "got", strlen("got")) == 0)
  {
    int level;
    if (pin < 16) {
      level = HS_GPIO0->DATAOUT & (1 << pin) ? 1 : 0;
    } else {
      pin -= 16;
      level = HS_GPIO1->DATAOUT & (1 << pin) ? 1 : 0;
    }
    chprintf(chp, "pin%d in DATAOUT is %s\r\n", pin, level ? "High" : "Low");
  }
  else if (strncmp(action, "mux", strlen("mux")) == 0)
  {
    int level, func=5, input=1, pull=0, cap=3;
    uint32_t mode;
    if (argc >= 2)
      func = atoi(argv[2]);
    if (argc >= 3)
      input = atoi(argv[3]);
    if (argc >= 4)
      pull = atoi(argv[4]);
    if (argc >= 5)
      cap = atoi(argv[5]);
    mode = PAL_MODE_ALTERNATE(func) | PAL_MODE_DRIVE_CAP(cap);
    mode |= input ? PAL_HS_MODE_INPUT : PAL_HS_MODE_OUTPUT;
    if (pull == 0)
      mode |= PAL_HS_PUDR_NOPUPDR;
    else if (pull == 1)
      mode |= PAL_HS_PUDR_PULLUP;
    else if (pull == 2)
      mode |= PAL_HS_PUDR_PULLDOWN;
    if (pin < 16) {
      palSetPadMode(IOPORT0, pin, mode);
      level = palReadPad(IOPORT0, pin);
    } else {
      pin -= 16;
      palSetPadMode(IOPORT1, pin, mode);
      level = palReadPad(IOPORT1, pin);
    }
    chprintf(chp, "pin%d as func%d, current is %s\r\n", pin, func, level ? "High" : "Low");
  }
  else
  {
    error = 1;
  }

 label_out:
  if (error) {
    chprintf(chp, "Usage: pin num(dec) action [params]\r\n");
    chprintf(chp, "                    set            \r\n");
    chprintf(chp, "                    clr            \r\n");
    chprintf(chp, "                    get            \r\n");
    chprintf(chp, "                    got   <-DATAOUT\r\n");
    chprintf(chp, "                    mux    [func=5] [input=1] [pull:no=0|up=1|down=2] [cap=3] \r\n");
  }
}
