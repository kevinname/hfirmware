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
#include "lib.h"
#include "stdlib.h"

#if PLATFORM_ADC_USE_ADC0 

typedef struct {
  BaseSequentialStream *chp;
  osSemaphoreId sem_id;
  osThreadId pThd;
  const adc_chn_para_t *pstAdcChn;
  adc_channel_t chn_d;
  uint32_t test_cnt;
  uint8_t disp;
}_adctest_arg_t;

typedef struct {
  const char *name;
  uint16_t chnMap;
  uint8_t cnt;
  const adc_attr_t stAdcAttr;
}_adctest_case_t;

static uint16_t g_u16TestData = 0x5c;
static uint16_t g_u16ChnMap;
static osSemaphoreId adc_semid;
static adc_attr_t *g_pstAdcAttr = NULL;
static osThreadId g_pChnThd[ADC_CHANNEL_NUM];
static uint8_t  g_u8AdcStart;

static const _adctest_case_t g_stAdcTestCase[] = 
{
  /* case 0 */
  {
    "ONCE-RIGHT-SW-SMALL_FIRST-POS-DISCONT-ANALOG-REG",
    0x30, 10,
    {
      ADC_CONT_MODE_ONLY_ONCE, ADC_DATA_ALIGN_RIGHT, ADC_TRIGER_MODE_SOFTWARE, 
      ADC_CHN_ORDER_SMALL_FIRST, ADC_HDTRIGER_SOURCE_TM0_1, ADC_TRIGER_EDGE_POS,
      ADC_DISC_MODE_DISCONT, ADC_INPUT_SRC_ANALOG, 0, ADC_TEST_MODE_REG
    }
  },

  /* case 1 */
  {
    "ONCE-RIGHT-SW-BIG_FIRST-POS-DISCONT-ANALOG-REG",
    0x13, 10,
    {
      ADC_CONT_MODE_ONLY_ONCE, ADC_DATA_ALIGN_RIGHT, ADC_TRIGER_MODE_SOFTWARE, 
      ADC_CHN_ORDER_BIG_FIRST, ADC_HDTRIGER_SOURCE_TM0_1, ADC_TRIGER_EDGE_POS,
      ADC_DISC_MODE_DISCONT, ADC_INPUT_SRC_ANALOG, 0, ADC_TEST_MODE_REG
    }
  },

  /* case 2 */
  {
    "ONCE-RIGHT-SW-SMALL_FIRST-POS-CONT-ANALOG-REG",
    0x74, 10,
    {
      ADC_CONT_MODE_ONLY_ONCE, ADC_DATA_ALIGN_RIGHT, ADC_TRIGER_MODE_SOFTWARE, 
      ADC_CHN_ORDER_SMALL_FIRST, ADC_HDTRIGER_SOURCE_TM0_1, ADC_TRIGER_EDGE_POS,
      ADC_DISC_MODE_CONT, ADC_INPUT_SRC_ANALOG, 0, ADC_TEST_MODE_REG
    }
  },

  /* case 3 */
  {
    "ONCE-RIGHT-SW-SMALL_FIRST-NEG-DISCONT-ANALOG-REG",
    0x65, 10,
    {
      ADC_CONT_MODE_ONLY_ONCE, ADC_DATA_ALIGN_RIGHT, ADC_TRIGER_MODE_SOFTWARE, 
      ADC_CHN_ORDER_SMALL_FIRST, ADC_HDTRIGER_SOURCE_TM0_1, ADC_TRIGER_EDGE_NEG,
      ADC_DISC_MODE_DISCONT, ADC_INPUT_SRC_ANALOG, 0, ADC_TEST_MODE_REG
    }
  },

  /* case 4 */
  {
    "ONCE-RIGHT-SW-SMALL_FIRST-NEG-DISCONT-FILTER-REG",
    0x74, 10,
    {
      ADC_CONT_MODE_ONLY_ONCE, ADC_DATA_ALIGN_RIGHT, ADC_TRIGER_MODE_SOFTWARE, 
      ADC_CHN_ORDER_SMALL_FIRST, ADC_HDTRIGER_SOURCE_TM0_1, ADC_TRIGER_EDGE_NEG,
      ADC_DISC_MODE_DISCONT, ADC_INPUT_SRC_FILTER, 0, ADC_TEST_MODE_REG
    }
  },

  /* case 5 */
  {
    "ONCE-RIGHT-SW-SMALL_FIRST-NEG-DISCONT-FILTER-SWAP-REG",
    0x47, 10,
    {
      ADC_CONT_MODE_ONLY_ONCE, ADC_DATA_ALIGN_RIGHT, ADC_TRIGER_MODE_SOFTWARE, 
      ADC_CHN_ORDER_SMALL_FIRST, ADC_HDTRIGER_SOURCE_TM0_1, ADC_TRIGER_EDGE_NEG,
      ADC_DISC_MODE_DISCONT, ADC_INPUT_SRC_FILTER, 1, ADC_TEST_MODE_REG
    }
  },

  /* case 6 */
  {
    "ONCE-LEFT-SW-SMALL_FIRST-NEG-DISCONT-FILTER-SWAP-REG",
    0x26, 10,
    {
      ADC_CONT_MODE_ONLY_ONCE, ADC_DATA_ALIGN_LEFT, ADC_TRIGER_MODE_SOFTWARE, 
      ADC_CHN_ORDER_SMALL_FIRST, ADC_HDTRIGER_SOURCE_TM0_1, ADC_TRIGER_EDGE_NEG,
      ADC_DISC_MODE_DISCONT, ADC_INPUT_SRC_FILTER, 1, ADC_TEST_MODE_REG
    }
  },

  /* case 7 */
  {
    "ONCE-RIGHT-SW-SMALL_FIRST-DUAL-CONT-FILTER-SWAP-REG",
    0x17, 10,
    {
      ADC_CONT_MODE_ONLY_ONCE, ADC_DATA_ALIGN_RIGHT, ADC_TRIGER_MODE_SOFTWARE, 
      ADC_CHN_ORDER_SMALL_FIRST, ADC_HDTRIGER_SOURCE_TM0_1, ADC_TRIGER_EDGE_DUAL,
      ADC_DISC_MODE_CONT, ADC_INPUT_SRC_FILTER, 1, ADC_TEST_MODE_REG
    }
  },

  /* case 8 */
  {
    "ONCE-RIGHT-SW-SMALL_FIRST-NEG-DISCONT-FILTER-SWAP-REG",
    0x176, 10,
    {
      ADC_CONT_MODE_ONLY_ONCE, ADC_DATA_ALIGN_RIGHT, ADC_TRIGER_MODE_SOFTWARE, 
      ADC_CHN_ORDER_SMALL_FIRST, ADC_HDTRIGER_SOURCE_TM0_1, ADC_TRIGER_EDGE_NEG,
      ADC_DISC_MODE_DISCONT, ADC_INPUT_SRC_FILTER, 1, ADC_TEST_MODE_REG
    }
  },

  /* case 9 */
  {
    "ONCE-RIGHT-SW-SMALL_FIRST-DUAL-CONT-FILTER-SWAP-REG",
    0x137, 10,
    {
      ADC_CONT_MODE_ONLY_ONCE, ADC_DATA_ALIGN_RIGHT, ADC_TRIGER_MODE_SOFTWARE, 
      ADC_CHN_ORDER_SMALL_FIRST, ADC_HDTRIGER_SOURCE_TM0_1, ADC_TRIGER_EDGE_DUAL,
      ADC_DISC_MODE_CONT, ADC_INPUT_SRC_FILTER, 1, ADC_TEST_MODE_REG
    }
  },

  /* case 10 */
  {
    "ONCE-RIGHT-HW-SMALL_FIRST-TM2_1-POS-DISCONT-ANALOG-REG",
    0x30, 10,
    {
      ADC_CONT_MODE_ONLY_ONCE, ADC_DATA_ALIGN_RIGHT, ADC_TRIGER_MODE_HARDWARE, 
      ADC_CHN_ORDER_SMALL_FIRST, ADC_HDTRIGER_SOURCE_TM2_1, ADC_TRIGER_EDGE_POS,
      ADC_DISC_MODE_DISCONT, ADC_INPUT_SRC_ANALOG, 0, ADC_TEST_MODE_REG
    }
  },

  /* case 11 */
  {
    "ONCE-RIGHT-HW-BIG_FIRST-TM2_0-POS-DISCONT-ANALOG-REG",
    0x13, 10,
    {
      ADC_CONT_MODE_ONLY_ONCE, ADC_DATA_ALIGN_RIGHT, ADC_TRIGER_MODE_HARDWARE, 
      ADC_CHN_ORDER_BIG_FIRST, ADC_HDTRIGER_SOURCE_TM2_0, ADC_TRIGER_EDGE_POS,
      ADC_DISC_MODE_DISCONT, ADC_INPUT_SRC_ANALOG, 0, ADC_TEST_MODE_REG
    }
  },

  /* case 12 */
  {
    "ONCE-RIGHT-HW-SMALL_FIRST-TM2_2-POS-CONT-ANALOG-REG",
    0xf6, 10,
    {
      ADC_CONT_MODE_ONLY_ONCE, ADC_DATA_ALIGN_RIGHT, ADC_TRIGER_MODE_HARDWARE, 
      ADC_CHN_ORDER_SMALL_FIRST, ADC_HDTRIGER_SOURCE_TM2_2, ADC_TRIGER_EDGE_POS,
      ADC_DISC_MODE_CONT, ADC_INPUT_SRC_ANALOG, 0, ADC_TEST_MODE_REG
    }
  },

  /* case 13 */
  {
    "ONCE-RIGHT-HW-SMALL_FIRST-TM2_3-NEG-DISCONT-ANALOG-REG",
    0x65, 10,
    {
      ADC_CONT_MODE_ONLY_ONCE, ADC_DATA_ALIGN_RIGHT, ADC_TRIGER_MODE_HARDWARE, 
      ADC_CHN_ORDER_SMALL_FIRST, ADC_HDTRIGER_SOURCE_TM2_3, ADC_TRIGER_EDGE_NEG,
      ADC_DISC_MODE_DISCONT, ADC_INPUT_SRC_ANALOG, 0, ADC_TEST_MODE_REG
    }
  },

  /* case 14 */
  {
    "ONCE-RIGHT-HW-SMALL_FIRST-TM2_1-NEG-DISCONT-FILTER-REG",
    0x71, 10,
    {
      ADC_CONT_MODE_ONLY_ONCE, ADC_DATA_ALIGN_RIGHT, ADC_TRIGER_MODE_HARDWARE, 
      ADC_CHN_ORDER_SMALL_FIRST, ADC_HDTRIGER_SOURCE_TM2_1, ADC_TRIGER_EDGE_NEG,
      ADC_DISC_MODE_DISCONT, ADC_INPUT_SRC_FILTER, 0, ADC_TEST_MODE_REG
    }
  },

  /* case 15 */
  {
    "ONCE-RIGHT-HW-SMALL_FIRST-TM2_1-NEG-DISCONT-FILTER-SWAP-REG",
    0x47, 10,
    {
      ADC_CONT_MODE_ONLY_ONCE, ADC_DATA_ALIGN_RIGHT, ADC_TRIGER_MODE_HARDWARE, 
      ADC_CHN_ORDER_SMALL_FIRST, ADC_HDTRIGER_SOURCE_TM2_1, ADC_TRIGER_EDGE_NEG,
      ADC_DISC_MODE_DISCONT, ADC_INPUT_SRC_FILTER, 1, ADC_TEST_MODE_REG
    }
  },

  /* case 16 */
  {
    "ONCE-LEFT-HW-SMALL_FIRST-TM2_1-NEG-DISCONT-FILTER-SWAP-REG",
    0x26, 10,
    {
      ADC_CONT_MODE_ONLY_ONCE, ADC_DATA_ALIGN_LEFT, ADC_TRIGER_MODE_HARDWARE, 
      ADC_CHN_ORDER_SMALL_FIRST, ADC_HDTRIGER_SOURCE_TM2_1, ADC_TRIGER_EDGE_NEG,
      ADC_DISC_MODE_DISCONT, ADC_INPUT_SRC_FILTER, 1, ADC_TEST_MODE_REG
    }
  },

  /* case 17 */
  {
    "ONCE-RIGHT-HW-SMALL_FIRST-TM2_2-DUAL-CONT-FILTER-SWAP-REG",
    0x17, 10,
    {
      ADC_CONT_MODE_ONLY_ONCE, ADC_DATA_ALIGN_RIGHT, ADC_TRIGER_MODE_HARDWARE, 
      ADC_CHN_ORDER_SMALL_FIRST, ADC_HDTRIGER_SOURCE_TM2_2, ADC_TRIGER_EDGE_DUAL,
      ADC_DISC_MODE_CONT, ADC_INPUT_SRC_FILTER, 1, ADC_TEST_MODE_REG
    }
  },

  /* case 18 */
  {
    "ONCE-RIGHT-HW-SMALL_FIRST-TM2_2-POS-CONT-ANALOG-REG",
    0x177, 10,
    {
      ADC_CONT_MODE_ONLY_ONCE, ADC_DATA_ALIGN_RIGHT, ADC_TRIGER_MODE_HARDWARE, 
      ADC_CHN_ORDER_SMALL_FIRST, ADC_HDTRIGER_SOURCE_TM2_2, ADC_TRIGER_EDGE_POS,
      ADC_DISC_MODE_CONT, ADC_INPUT_SRC_ANALOG, 0, ADC_TEST_MODE_REG
    }
  },

  /* case 19 */
  {
    "ONCE-RIGHT-HW-SMALL_FIRST-TM2_3-NEG-DISCONT-ANALOG-REG",
    0x167, 10,
    {
      ADC_CONT_MODE_ONLY_ONCE, ADC_DATA_ALIGN_RIGHT, ADC_TRIGER_MODE_HARDWARE, 
      ADC_CHN_ORDER_SMALL_FIRST, ADC_HDTRIGER_SOURCE_TM2_3, ADC_TRIGER_EDGE_NEG,
      ADC_DISC_MODE_DISCONT, ADC_INPUT_SRC_ANALOG, 0, ADC_TEST_MODE_REG
    }
  },

  /* case 20 */
  {
    "MULTI-RIGHT-SW-BIG_FIRST-POS-DISCONT-ANALOG-REG",
    0x13, 10,
    {
      ADC_CONT_MODE_MULTI_NUM, ADC_DATA_ALIGN_RIGHT, ADC_TRIGER_MODE_SOFTWARE, 
      ADC_CHN_ORDER_BIG_FIRST, ADC_HDTRIGER_SOURCE_TM2_1, ADC_TRIGER_EDGE_POS,
      ADC_DISC_MODE_DISCONT, ADC_INPUT_SRC_ANALOG, 0, ADC_TEST_MODE_REG
    }
  },

  /* case 21 */
  {
    "MULTI-RIGHT-SW-SMALL_FIRST-POS-DISCONT-ANALOG-REG",
    0x35, 10,
    {
      ADC_CONT_MODE_MULTI_NUM, ADC_DATA_ALIGN_RIGHT, ADC_TRIGER_MODE_SOFTWARE, 
      ADC_CHN_ORDER_SMALL_FIRST, ADC_HDTRIGER_SOURCE_TM2_1, ADC_TRIGER_EDGE_POS,
      ADC_DISC_MODE_DISCONT, ADC_INPUT_SRC_ANALOG, 0, ADC_TEST_MODE_REG
    }
  },

  /* case 22 */
  {
    "MULTI-RIGHT-HW-SMALL_FIRST-TM2_3-NEG-DISCONT-ANALOG-REG",
    0x65, 10,
    {
      ADC_CONT_MODE_MULTI_NUM, ADC_DATA_ALIGN_RIGHT, ADC_TRIGER_MODE_HARDWARE, 
      ADC_CHN_ORDER_SMALL_FIRST, ADC_HDTRIGER_SOURCE_TM2_3, ADC_TRIGER_EDGE_NEG,
      ADC_DISC_MODE_DISCONT, ADC_INPUT_SRC_ANALOG, 0, ADC_TEST_MODE_REG
    }
  },

  /* case 23 */
  {
    "MULTI-RIGHT-HW-SMALL_FIRST-TM2_1-NEG-DISCONT-FILTER-REG",
    0x77, 10,
    {
      ADC_CONT_MODE_MULTI_NUM, ADC_DATA_ALIGN_RIGHT, ADC_TRIGER_MODE_HARDWARE, 
      ADC_CHN_ORDER_SMALL_FIRST, ADC_HDTRIGER_SOURCE_TM2_1, ADC_TRIGER_EDGE_NEG,
      ADC_DISC_MODE_DISCONT, ADC_INPUT_SRC_FILTER, 0, ADC_TEST_MODE_REG
    }
  },

};

float const g_fGain[4] = {0.5, 1, 2, 4};

float _adcTest_calVal(_adctest_arg_t *parg, uint16_t data)
{
  uint8_t gain;//, chopper_en;
  int16_t sdata;
  float vcm, vi, vf, mvoffset, gainMode;
  
  if(!parg->pstAdcChn)
  {
    gain       = parg->chn_d > 2 ? 1 : 3;
    //chopper_en = 0;
    vcm        = (parg->chn_d > 0 ? 8 : 5) * 125;
  }
  else
  {
    gain       = parg->pstAdcChn->gtune;
    //chopper_en = parg->pstAdcChn->en_chop;
    vcm        = parg->pstAdcChn->sel_vcm * 125;
  }

  sdata  = hs_adc_getInteger(data);
  hs_adc_getAdjust(&mvoffset, &gainMode);

  //hs_printf("\r\nOffset: %f, %f\r\n", mvoffset, gainMode);
  
  vf = sdata * 1000.0 / 2048;  
  vi = (vf - mvoffset) * gainMode / g_fGain[gain] + vcm;
  
  if(parg->chn_d == ADC_CHANNEL_CHIP_BATTERY)
    vi = vi * 3;
  
  return vi;
}

static void _adcTest_Thd(void *arg)
{
  _adctest_arg_t *parg = (_adctest_arg_t *)arg;
  uint32_t data, cnt = 0;

  chRegSetThreadName("adcTest");
  while(1)
  {
    data = adcGetChnData(ADC_HANDLER, parg->chn_d, TIME_INFINITE);
    if(data == 0)
    {
      msleep(10);
      continue;
    }

    if(parg->disp)
    {
      oshalSemaphoreWait(adc_semid, -1);
      chprintf(parg->chp, "\r\nChn[%d-%04d], %d, %fmV ", parg->chn_d, cnt, data, _adcTest_calVal(parg, data));
      oshalSemaphoreRelease(adc_semid);
    }

    if(0) //((data & 0x3ff) != (g_u16TestData + parg->chn_d + 1))
    {
      oshalSemaphoreWait(adc_semid, -1);
      chprintf(parg->chp, "\r\nChn[%d]: Reg: 0x%x != 0x%x ", parg->chn_d, data, (g_u16TestData + parg->chn_d + 1));
      oshalSemaphoreRelease(adc_semid);
    }

    parg->test_cnt -= 1;
    cnt += 1;
    if((parg->test_cnt == 0) || (g_u8AdcStart == 0))
      break;
  }

  if(parg->disp)
  {
    oshalSemaphoreWait(adc_semid, -1);
    chprintf(parg->chp, "\r\nChn[%d] test over, have been tested count: 0x%x!\r\n", parg->chn_d, cnt);
    oshalSemaphoreRelease(adc_semid);
  }

  if(!parg->disp)
	  chprintf(parg->chp, "%d", parg->chn_d);

  oshalSignalSet(parg->pThd, (1u << parg->chn_d));
  g_u16ChnMap &= ~(1u << parg->chn_d);
  hs_free(arg);
}

void _adc_help_attr(BaseSequentialStream *chp)
{
  chprintf(chp, "Format: adcattr [attribute1 attribute2 testdata]\r\n"  
                  "attribute1[0xabcdefg]:\r\n\t"
                    "a - cont mode.   0-only once, 1-multi number\r\n\t"
                    "b - triger mode. 0-software, 1-hardware\r\n\t"
                    "c - data align.  0-right, 1-left\r\n\t"
                    "d - channel order. 0-small first, 1-big first\r\n\t"
                    "e - hardware triger source. 0~3-TM0:0~3, 4~7-TM1:4~7,8~11-TM2:0~3, 12-EXT\r\n\t"
                    "f - triger edage. 0-pos, 1-neg,2-hual, 3-no used\r\n\t"
                    "g - disc mode. 0-continue, 1-discontinue\r\n"     
                  "attribute2[0xabc]:\r\n\t"
                    "a - sarq_bypass. sardata from: 0-CIC filtr, 1-analog input\r\n\t"
                    "b - swap_enable. 0-disable, 1-enable\r\n\t"
                    "c - test_mode.  0-from adc, 1-from register\r\n"
                  "testdata:\r\n\t"
                    "if test_mode == 1, this field validate\r\n"
                  "if no arg, the command will not change adc attribute.\r\nThis cmd should be run first!\r\n\r\n"); 
}

void _adc_help_timing(BaseSequentialStream *chp)
{
  chprintf(chp, "Format: adctiming para1 para2\r\n"  
                  "para1[0xabcdef]:\r\n\t"
                    "a  - t1.   0~0xf,   default: 0x06 \r\n\t"
                    "b  - t2.   0~0xf,   default: 0x06 \r\n\t"
                    "c  - t3.   0~0xf,   default: 0x01 \r\n\t"
                    "d  - t4.   0~0xf,   default: 0x04 \r\n\t"
                    "ef - cic_cycle. 0~0xff, default: 0x40\r\n\t"
                    "g  - smp_cycle. 0~0xf,  default: 0x01\r\n\t"
                    "h  - rst_cycle. 0~0xf,  default: 0x01\r\n"                    
                  "para2[0xabc00de]:\r\n\t"
                    "adc - t5.   0~0xfff, default: 0x348\r\n\t"
                    "de  - t6.   0~0x3f,  default: 0x18 \r\n"
                  "\r\n"); 
}

void _adc_help_chnAttr(BaseSequentialStream *chp)
{
  chprintf(chp, "Format: adcchn chn [para1 para2] test_cnt\r\n"  
                  "chn: channel-id, 0~7\r\n"
                  "para1[0xabcdefgh]:\r\n\t"
                    "a - en_r2r.       \r\n\t"
                    "b - en_chop.      \r\n\t"
                    "c - gtune. gain: 0-0.5 1-1 2-2 3-4 \r\n\t"
                    "d - ldoctrl. 0-2.3v 1-2.4v 2-2.5v 3-2.6v \r\n\t"
                    "e - sel_fchop. 0-1/32*fs 1-1/16*fs 2-1/8*fs 3-1/4*fs  fs=24MHz \r\n\t"
                    "f - sel_inp. 0~7 \r\n\t"
                    "g - sel_vcm. 0-0v 1-0.125v 2-0.25v ... 8-1.0v ... 14-1.75v 15-1.875v\r\n\t"
                    "h - en_count_sar. \r\n"
                  "para2[0xabc]:\r\n\t"
                    "a - en_dem_sar.   \r\n\t"
                    "b - sar_buf.  \r\n\t"
                    "c - en_sar_ckdelay. \r\n"
                  "\r\n"); 
}

#if HAL_USE_PWM
static const PWMConfig g_adcPwmCfg = {
  10000, 
  1000, 
  NULL,
  {
   {PWM_OUTPUT_ACTIVE_HIGH, NULL, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL, NULL}
  },
  0,
  0,
  0,
  #if PWM_USE_DMA
  false,
  NULL
  #endif
};
#endif

int _adc_start_timer(BaseSequentialStream *chp, uint8_t index, uint8_t channel) 
{
  #if HAL_USE_PWM
  PWMDriver *pd;

  pd = 
    #if HS_PWM_USE_TIM0
    (index == 0) ? &PWMD0 : 
    #endif
      #if HS_PWM_USE_TIM1
      (index == 1) ? &PWMD1 : 
      #endif
        #if HS_PWM_USE_TIM2
        (index == 2) ? &PWMD2 : 
        #endif
          NULL;

  if(pd == NULL)
  {
    chprintf(chp, "[ERROR]have not found the timer%d\r\n", index);
    return -1;
  }
  
  pwmStart(pd, &g_adcPwmCfg);
  pwmEnableChannel(pd, channel, PWM_PERCENTAGE_TO_WIDTH(pd, 5000));

  return 0;
  #else
  return -1;
  #endif
}

void _adc_stop_timer(uint8_t index, uint8_t channel)
{
  #if HAL_USE_PWM
  PWMDriver *pd;

  pd = 
    #if HS_PWM_USE_TIM0
    (index == 0) ? &PWMD0 : 
    #endif
      #if HS_PWM_USE_TIM1
      (index == 1) ? &PWMD1 : 
      #endif
        #if HS_PWM_USE_TIM2
        (index == 2) ? &PWMD2 : 
        #endif
          NULL;

  if(pd == NULL)
  {
    return ;
  }

  pwmDisableChannel(pd, channel);
  pwmStop(pd);
  #endif
}

int _adcSetAttr(BaseSequentialStream *chp, const adc_attr_t *pstAdcAttr)
{
  if(0 != adcStart(ADC_HANDLER, pstAdcAttr))
  {
    chprintf(chp, "Adc set attribute Error!\r\n");
    return -1;
  }

  if(pstAdcAttr->test_mode == 1)
    adcSetTestData(g_u16TestData);  

  g_u16ChnMap = 0;
  return 0;
}

int _adcAddChn(BaseSequentialStream *chp, uint32_t chn, const adc_chn_para_t *pstAdcChn, uint32_t cnt, uint8_t disp)
{
  _adctest_arg_t *pstAdcArg;
  osThreadDef_t thdDef;
  
  if(chn >= ADC_CHANNEL_NUM)
  {
    chprintf(chp, "Adc Channel id Error!\r\n");
    return -1;
  }

  pstAdcArg = hs_malloc(sizeof(_adctest_arg_t), __MT_Z_GENERAL);
  if(pstAdcArg == NULL)
  {
    chprintf(chp, "alloc memory error!\r\n");
    return -1;
  }

  thdDef.pthread   = (os_pthread)_adcTest_Thd;
  thdDef.stacksize = 512;
  thdDef.tpriority = osPriorityNormal;

  pstAdcArg->chn_d = chn;
  pstAdcArg->chp = chp;
  pstAdcArg->test_cnt = cnt;  
  pstAdcArg->pThd = curthread();
  pstAdcArg->disp = disp;
  pstAdcArg->pstAdcChn = pstAdcChn;
  g_pChnThd[chn] = oshalThreadCreate(&thdDef, pstAdcArg); 

  if(0 != adcAddChannel(ADC_HANDLER, chn, pstAdcChn))
  {
    chprintf(chp, "Adc add Channel Error!\r\n");
    oshalThreadTerminate(pstAdcArg->pThd);
    hs_free(pstAdcArg);
    return -1;
  }

  g_u16ChnMap |= 1u << chn;
  return 0;
}

void cmd_adcSetAttr(BaseSequentialStream *chp, int argc, char *argv[]) 
{  
  osSemaphoreDef_t semdef;
  uint32_t attr1, attr2;
  
  if ((argc != 0) && (argc != 2) && (argc != 3)) 
  {
    _adc_help_attr(chp);
    return;
  }

  if (argc != 0)
  {
    g_pstAdcAttr = hs_malloc(sizeof(adc_attr_t), __MT_Z_GENERAL);
    if(g_pstAdcAttr == NULL)
    {
      chprintf(chp, "alloc memory error!\r\n");
      return ;
    }
  }

  attr1 = strtoul(argv[0], NULL, 16) & 0xfffffff;
  attr2 = strtoul(argv[1], NULL, 16) & 0xfff;

  g_pstAdcAttr->cont = (attr1 >> 24) & 0xf;
  g_pstAdcAttr->hw_en = (attr1 >> 20) & 0xf;
  g_pstAdcAttr->align = (attr1 >> 16) & 0xf;
  g_pstAdcAttr->scandir = (attr1 >> 12) & 0xf;
  g_pstAdcAttr->timer_sel = (attr1 >> 8) & 0xf;
  g_pstAdcAttr->start_edge = (attr1 >> 4) & 0xf;
  g_pstAdcAttr->disen = (attr1 >> 0) & 0xf;

  g_pstAdcAttr->sarq_bypass = (attr2 >> 8) & 0xf;
  g_pstAdcAttr->swap_enable = (attr2 >> 4) & 0xf;
  g_pstAdcAttr->test_mode = (attr2 >> 0) & 0xf;

  if((g_pstAdcAttr->test_mode == 1) && (argc == 3))
    g_u16TestData = strtoul(argv[2], NULL, 16) & 0xfff;

  if(0 != _adcSetAttr(chp, g_pstAdcAttr))
  {
    chprintf(chp, "Adc set attribute error!\r\n");
    return ;
  }

  adc_semid = oshalSemaphoreCreate(&semdef, 1);
  chprintf(chp, "Adc set attribute OK!\r\n");
}

void cmd_adcSetTiming(BaseSequentialStream *chp, int argc, char *argv[]) 
{
  adc_timing_t stAdcTiming;
  uint32_t attr1, attr2;
  
  if(argc != 2)
  {
    _adc_help_timing(chp);
    return;
  }

  attr1 = strtoul(argv[0], NULL, 16);
  attr2 = strtoul(argv[1], NULL, 16) & 0xfff00ff;

  stAdcTiming.t1 = (attr1 >> 28) & 0xf;
  stAdcTiming.t2 = (attr1 >> 24) & 0xf;
  stAdcTiming.t3 = (attr1 >> 20) & 0xf;
  stAdcTiming.t4 = (attr1 >> 16) & 0xf;

  stAdcTiming.t5 = (attr2 >> 16) & 0xfff;
  stAdcTiming.t6 = (attr2 >> 0) & 0x3f;

  stAdcTiming.cic_cycle = (attr1 >> 8) & 0xff;
  stAdcTiming.smp_cycle = (attr1 >> 4) & 0xf;
  stAdcTiming.rst_cycle = (attr1 >> 0) & 0xf;

  if(0 != adcSetTiming(ADC_HANDLER, &stAdcTiming))
  {
    chprintf(chp, "Adc set timing Error!\r\n");
    return ;
  }

  chprintf(chp, "Adc set timing ok!\r\n");
}

void cmd_adcAddChn(BaseSequentialStream *chp, int argc, char *argv[]) 
{
  adc_chn_para_t *pstAdcChn;
  uint32_t chn, attr1, attr2, test_cnt;
  
  if((argc != 2) && (argc != 4))
  {
    _adc_help_chnAttr(chp);
    return;
  }

  if(argc == 4)
  {
    chn   = strtoul(argv[0], NULL, 16);
    attr1 = strtoul(argv[1], NULL, 16);
    attr2 = strtoul(argv[2], NULL, 16) & 0xfff;
    test_cnt = strtoul(argv[3], NULL, 16);

    pstAdcChn = hs_malloc(sizeof(adc_chn_para_t), __MT_Z_GENERAL);

    pstAdcChn->en_r2r    = (attr1 >> 28) & 0xf;
    pstAdcChn->en_chop   = (attr1 >> 24) & 0xf;
    pstAdcChn->gtune     = (attr1 >> 20) & 0xf;
    pstAdcChn->ldoctrl   = (attr1 >> 16) & 0xf;
    pstAdcChn->sel_fchop = (attr1 >> 12) & 0xf;
    pstAdcChn->sel_inp   = (attr1 >> 8) & 0xf;
    pstAdcChn->sel_vcm   = (attr1 >> 4) & 0xf;
    pstAdcChn->en_count_sar = attr1 & 0xf;

    pstAdcChn->en_dem_sar = (attr2 >> 8) & 0xf;
    pstAdcChn->sar_buf    = (attr2 >> 4) & 0xf;
    pstAdcChn->en_sar_ckdelay = attr2 & 0xf;
  }
  else
  {
    chn      = strtoul(argv[0], NULL, 16);
    test_cnt = strtoul(argv[1], NULL, 16);
    pstAdcChn = NULL;
  }

  if(0 != _adcAddChn(chp, chn, pstAdcChn, test_cnt, 1))
  {
    chprintf(chp, "Adc add channel error!\r\n");
    return ;
  }

  chprintf(chp, "Adc add Channel %d Ok!\r\n", chn);
}

void cmd_adcStart(BaseSequentialStream *chp, int argc, char *argv[]) 
{
  uint32_t mode, size, cnt = 0x10;
  uint16_t *dmaBuf;
  
  if ((argc != 1) && (argc != 2)) {
    chprintf(chp, "Usage: adcstart mode [cnt]\r\n\tmode: 0-normal mode, 1-dma mode\r\n");
    return;
  }

  mode = atoll(argv[0]); 
  if(argc > 1) {
    cnt = strtoul(argv[1], NULL, 16);
  }
  
  if(mode == 0) {
    if(0 != adcStartConversion(ADC_HANDLER)) {
      chprintf(chp, "Adc start error!\r\n");
      return ;
    }

    g_u8AdcStart = 1;

    if(g_pstAdcAttr->hw_en)
      _adc_start_timer(chp, g_pstAdcAttr->timer_sel / 4, g_pstAdcAttr->timer_sel % 4);
  } else {
    uint32_t i, address, chn_num, chn_mask;

    chn_num = ADC_HANDLER->channel_num;
    chn_mask = ADC_HANDLER->channel_mask;
    size = chn_num * 2 * cnt * (1 + g_pstAdcAttr->swap_enable);
    dmaBuf = (uint16_t *)hs_malloc(size, __MT_Z_DMA);
    
    if(0 != adcStartConversionWithDMA(ADC_HANDLER, (uint8_t *)dmaBuf, size)) {
      chprintf(chp, "Adc dma start error!\r\n");
      hs_free(dmaBuf);
      return ;
    }

    g_u8AdcStart = 0;
    if(g_pstAdcAttr->hw_en)
      _adc_start_timer(chp, g_pstAdcAttr->timer_sel / 4, g_pstAdcAttr->timer_sel % 4);

    if(0 != adcDmaWaitForDone(ADC_HANDLER, TIME_INFINITE)) {
      chprintf(chp, "Adc dma transfer error!\r\n");
    }

    if(g_pstAdcAttr && (g_pstAdcAttr->hw_en))
    {
      _adc_stop_timer(g_pstAdcAttr->timer_sel / 4, g_pstAdcAttr->timer_sel % 4);
    }

    address = (uint32_t)dmaBuf;
    chprintf(chp, "\r\n          \t");
    for(i=0; i<ADC_CHANNEL_NUM; i++)
    {
      if(chn_mask & 1)
        chprintf(chp, " %X   ", i);

      chn_mask >>= 1;
    }
    
    for(i=0; i<size/2; i++)
    {
      if((i%chn_num) == 0)
        chprintf(chp, "\r\n%08X:\t", address+i*2);

      chprintf(chp, "%08d ", dmaBuf[i]);
    }

    chprintf(chp, "\r\n\r\n");
    hs_free(dmaBuf);
  }
}

void cmd_adcStop(BaseSequentialStream *chp, int argc, char *argv[]) 
{
  (void)argc;
  (void)argv;
  uint32_t i;

  g_u8AdcStart = 0;

  if(g_u16ChnMap)
    oshalSignalWait(g_u16ChnMap, 0);  
  
  adcStopConversion(ADC_HANDLER);
  adcStop(ADC_HANDLER); 

  if(g_pstAdcAttr && (g_pstAdcAttr->hw_en))
  {
    _adc_stop_timer(g_pstAdcAttr->timer_sel / 4, g_pstAdcAttr->timer_sel % 4);
  }

  for(i=0; i<ADC_CHANNEL_NUM; i++)
  {
    if(g_pChnThd[i])
    {
      oshalThreadTerminate(g_pChnThd[i]);
      g_pChnThd[i] = NULL;
    }
  }

  hs_free(g_pstAdcAttr);
  g_pstAdcAttr = NULL;

  chprintf(chp, "Adc stopped!\r\n");
}

void cmd_adcScan(BaseSequentialStream *chp, int argc, char *argv[]) 
{
  osSemaphoreDef_t semdef;
  osEvent evt;
  const _adctest_case_t *pstTestCase = g_stAdcTestCase;
  uint32_t i, j, num = sizeof(g_stAdcTestCase) / sizeof(_adctest_case_t);
  uint32_t map; 
  
  (void)argc;
  (void)argv;

  adc_semid = oshalSemaphoreCreate(&semdef, 1);
  for(i=0; i<num; i++)
  {
    chprintf(chp, "[case %d]:%s ", i, pstTestCase->name);

    msleep(5);
    
    if(0 != _adcSetAttr(chp, &pstTestCase->stAdcAttr))
    {
      chprintf(chp, "set attribute error!\r\n");
      pstTestCase ++;
      continue;
    }

    map = pstTestCase->chnMap;
    for(j=0; j<ADC_CHANNEL_NUM; j++)
    {
      if((map & 1) == 0)
      {
        map >>= 1;
        continue;
      }
      
      if(0 != _adcAddChn(chp, j, NULL, pstTestCase->cnt, 0))
      {
        chprintf(chp, "add chn %d error!\r\n", j);
      }

      map >>= 1;
      msleep(1);
    }

    g_u8AdcStart = 1;
    if(0 != adcStartConversion(ADC_HANDLER))
    {
      chprintf(chp, "set start conv error!\r\n");
      pstTestCase ++;
      continue;
    }  

    if(pstTestCase->stAdcAttr.hw_en)
    {
      _adc_start_timer(chp, pstTestCase->stAdcAttr.timer_sel / 4, pstTestCase->stAdcAttr.timer_sel % 4);
      oshalSignalWait(pstTestCase->chnMap, -1);
      _adc_stop_timer(pstTestCase->stAdcAttr.timer_sel / 4, pstTestCase->stAdcAttr.timer_sel % 4);
    }
    else
    {
      while(1)
      {
        evt = oshalSignalWait(pstTestCase->chnMap, 100);
        if(evt.status == osEventTimeout)
          adcStartConversion(ADC_HANDLER);
        else
          break;
      }
    }

    g_u8AdcStart = 0;
    adcStopConversion(ADC_HANDLER);
    adcStop(ADC_HANDLER);

    if(pstTestCase->stAdcAttr.hw_en)
    {
      _adc_stop_timer(g_pstAdcAttr->timer_sel / 4, g_pstAdcAttr->timer_sel % 4);
    }

    for(j=0; j<ADC_CHANNEL_NUM; j++)
    {
      if(g_pChnThd[j])
      {
        oshalThreadTerminate(g_pChnThd[j]);
        g_pChnThd[j] = NULL;
      }
    }

    pstTestCase ++;
    chprintf(chp, " test over!\r\n");
  }
}

#endif
