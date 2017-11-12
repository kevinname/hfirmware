/*
    ChibiOS/RT - Copyright (C) 2014 HunterSun Technologies
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

#if HAL_USE_WDT
/*************************************************
wdt test: 
    wdt kick      : kick the watchdog, system run OK
example:
  
*************************************************/

void cmd_wdt(BaseSequentialStream *chp, int argc, char *argv[]) {
  #if HAL_USE_WDT
  if (argc < 1) {
      goto ERROR;
  }

  if (strcmp(argv[0], "kick") == 0) {
     wdtKickdog();
  }

  if (strcmp(argv[0], "init") == 0) {
     wdtInit();
  }

  if (strcmp(argv[0], "stop") == 0) {
     wdtDisable();
  }
  
  return;
   
ERROR:
  
  chprintf(chp, "Usage: wdt kick \r\n"); 

  chprintf(chp, "Example: wdt kick    -- kick the watchdog \r\n");
  #else
  (void)chp;
  (void)argc;
  (void)argv;
  #endif
}
#endif
