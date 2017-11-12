/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio
                 Copyright (C) 2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn

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
#include <stdlib.h> //atoi
#include "lib.h"
#include "context.h"

void cmd_pmuSetVoltage(BaseSequentialStream *chp, int argc, char *argv[]) 
{
  uint32_t index, val;
  HS_ANA_Type *pstAna = HS_ANA;
  HS_PMU_Type *pstPmu = HS_PMU;

  if (argc != 1) {
    chprintf(chp, "Usage: pmuvolt index\r\n\tindex:\r\n\t  1:2.2v 2:1.3v 3:1.3v 4:1.5v 5:1.7v 6:1.7v 7:2.5v\r\n");
    return;
  }
  
  index = atoll(argv[0]);

  pstAna->PD_CFG[0] = 0;
  pstAna->PD_CFG[1] = 0;
  pstAna->PD_CFG[2] = 0;
  pstAna->PD_CFG[3] = 0;

  pstPmu->ANA_CON = 0;
  
  pmu_ana_set(76, 76, 1);
  pmu_ana_set(73, 75, index);

  val = pmu_ana_get(73, 76);
  
  chprintf(chp, "Set pmu voltage over! \r\n[76, 73]'s value is 0x%02x !\r\n\r\n", val);  
}


void cmd_rfpllSetVoltage(BaseSequentialStream *chp, int argc, char *argv[]) 
{
  uint32_t index, val;
  //HS_ANA_Type *pstAna = HS_ANA;
  //HS_PMU_Type *pstPmu = HS_PMU;

  if (argc != 1) {
    chprintf(chp, "Usage: rfpllvolt index\r\n\tindex:\r\n\t  1:1.3v 2:1.3v 3:1.3v 4:1.6v 5:1.5v 6:1.3v\r\n");
    return;
  }
  
  index = atoll(argv[0]);

  #if 0
  pstAna->PD_CFG[0] = 0;
  pstAna->PD_CFG[1] = 0;
  pstAna->PD_CFG[2] = 0;
  pstAna->PD_CFG[3] = 0;
  pstPmu->ANA_CON = 0;
  #endif
  
  pmu_ana_set(298, 298, 1);
  pmu_ana_set(295, 297, index);

  val = pmu_ana_get(295, 298);
  
  chprintf(chp, "Set rf pll voltage over! \r\n[298, 295]'s value is 0x%02x !\r\n\r\n", val);  
}

void cmd_pllClock(BaseSequentialStream *chp, int argc, char *argv[]) 
{
  uint32_t index, val;
  HS_ANA_Type *pstAna = HS_ANA;
  HS_PMU_Type *pstPmu = HS_PMU;

  if (argc != 1) {
    chprintf(chp, "Usage: pllclk index\r\n\tindex:\r\n\t  0: System PLL\r\n\t  1: BT rf PLL\r\n\t  2:FM PLL\r\n");
    return;
  }
  
  index = atoll(argv[0]);

  pstAna->PD_CFG[0] = 0;
  pstAna->PD_CFG[1] = 0;
  pstAna->PD_CFG[2] = 0;
  pstAna->PD_CFG[3] = 0;
  pstPmu->ANA_CON = 0xe0000000;
  
  switch(index)
  {
    case 0:
      pmu_ana_set(294, 294, 1);
      __hal_set_bitsval(pstAna->COMMON_CFG[1], 20, 22, 4);
      break;

    case 1:
      pmu_ana_set(289, 289, 0);
      __hal_set_bitsval(pstAna->COMMON_CFG[1], 20, 22, 2);
      break;

    case 2:
      pstAna->FM_CFG[0] |= 1u << 8;
      __hal_set_bitsval(pstAna->COMMON_CFG[1], 20, 22, 1);
      break;

    default:
      chprintf(chp, "paramter error!\r\n\r\n");  
  }

  val = pmu_ana_get(295, 298);
  
  chprintf(chp, "Select pll over! \r\n\r\n", val);  
}








