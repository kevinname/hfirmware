/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio
                 Copyright (C) 2014 HunterSun Technologies

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
#include "lib.h"

#if HAL_USE_PWM && HAL_USE_ICU

uint16_t *pwm_dma_mem = NULL;
uint16_t *icu_dma_mem = NULL;

#define TEST_CASE_SIZE    (10)
#define UPDATE_REG_NUM    (4)
#define DMA_TRANFER_BLOCK (TEST_CASE_SIZE*UPDATE_REG_NUM)
#define BUF_SIZE        (TEST_CASE_SIZE*UPDATE_REG_NUM*4)
static void pwmpcb(PWMDriver *pwmp, void *arg) {

  (void)pwmp;
  (void)arg;
 // palClearPad(IOPORT1, GPIO1_LED1);
}

static void pwmc1cb(PWMDriver *pwmp, void *arg) {

  (void)pwmp;
  (void)arg;
 // palSetPad(IOPORT1, GPIO1_LED1);
}

static const PWMConfig pwm_pio_cfg = {
  100000,                                    /* 100kHz PWM clock frequency.   */
  100,                                    /* Initial PWM period 1mS.       */
  pwmpcb,
  {
   {PWM_OUTPUT_ACTIVE_HIGH, pwmc1cb, NULL},
   {PWM_OUTPUT_DISABLED, NULL, NULL},
   {PWM_OUTPUT_DISABLED, NULL, NULL},
   {PWM_OUTPUT_DISABLED, NULL, NULL}
  },
  0,
  0,
  0
#if PWM_USE_DMA
  ,
  FALSE,
  0
#endif
};

#if PWM_USE_DMA
bool pwm_dma_com = FALSE;

static void pwm_dma_callback(void) {
     pwm_dma_com = TRUE;
}

static PWMDMAinfo pwm_dma_info = {
  DMA_TRANFER_BLOCK,  /* only 10 update */
  11,    /* update ARR RCR CCR1 CCR2 */
  NULL,
  UPDATE_REG_NUM,
  pwm_dma_callback
};

static const PWMConfig pwm_dma_cfg = {
  100000,                                    /* 100kHz PWM clock frequency.   */
  100,                                       /* Initial PWM period 1mS.       */
  pwmpcb,
  {
   {PWM_OUTPUT_ACTIVE_HIGH, pwmc1cb, NULL},
   {PWM_OUTPUT_DISABLED, NULL, NULL},
   {PWM_OUTPUT_DISABLED, NULL, NULL},
   {PWM_OUTPUT_DISABLED, NULL, NULL}
  },
  0,
  0,
  0
#if PWM_USE_DMA
  ,
  TRUE,
  &pwm_dma_info
#endif
};
#endif
static icucnt_t last_width =0, last_period = 0;

static void icuwidthcb(ICUDriver *icup) {
 // palSetPad(IOPORT1, GPIO1_LED2);
  last_width = icuGetWidth(icup);
#if ICU_USE_DMA
 // icu_pio_mem[num] = last_width;
 // num++;
#endif
}

static void icuperiodcb(ICUDriver *icup) {

 // palClearPad(IOPORT1, GPIO1_LED2);
  last_period = icuGetPeriod(icup);
#if ICU_USE_DMA  
 // icu_pio_mem[num] = last_period;
 // num++;
#endif  
}

#if ICU_USE_DMA
bool icu_dma_com = FALSE;
void icu_dma_callback(void*p) {
	 (void)p;
     icu_dma_com = TRUE;
}
static ICUDMAinfo icu_dma_info = {
  DMA_TRANFER_BLOCK,  /* only 10 update */
  13,      /* update CCR1 CCR2 CCR3 CCR4 */
  NULL,
  UPDATE_REG_NUM,
  TRUE,
  FALSE,
  icu_dma_callback
}; 
#endif

static const ICUConfig icu_dma_cfg = {
  ICU_INPUT_ACTIVE_HIGH,
  1000000,                                    /* 1MHz ICU clock frequency.   */
  icuwidthcb,
  icuperiodcb,
  NULL,
  ICU_CHANNEL_1,
  0
#if ICU_USE_DMA
  ,
  TRUE,
  &icu_dma_info
#endif
};

static const ICUConfig icu_pio_cfg = {
  ICU_INPUT_ACTIVE_HIGH,
  100000,                                    /* 10MHz ICU clock frequency.   */
  icuwidthcb,
  icuperiodcb,
  NULL,
  ICU_CHANNEL_1,
  0
#if ICU_USE_DMA
  ,
  FALSE,
  &icu_dma_info
#endif
};

void cmd_tim(BaseSequentialStream *chp, int argc, char *argv[]){
  (void)argc;
  bool error = FALSE;
  
  PWMDriver *pwmd = 
#if HS_PWM_USE_TIM0
  &PWMD0;
#elif HS_PWM_USE_TIM1
  &PWMD1;
#elif HS_PWM_USE_TIM2
  &PWMD2;
#endif
  
  ICUDriver *icud =
#if HS_ICU_USE_TIM0
  &ICUD0;          
#elif HS_ICU_USE_TIM1
  &ICUD1;
#elif HS_ICU_USE_TIM2
  &ICUD2;
#endif
            
  /*
   * Initializes the PWM driver 1 and ICU driver 2.
   * GPIO0 5 is the PWM output.
   * GPIO0 11 is the ICU input.
   * The two pins have to be externally connected together.
   */
  // ICU TIMER 1
  palSetPadMode(IOPORT0, 6, PAL_MODE_INPUT | PAL_MODE_ALTERNATE(PAD_FUNC_TIMER2_3) | PAL_MODE_DRIVE_CAP(3));
  // PWM TIMER 2
  palSetPadMode(IOPORT1, 1, PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_TIMER2_3) | PAL_MODE_DRIVE_CAP(3));

#if ICU_USE_DMA && PWM_USE_DMA
  uint16_t i;
  if (!strcmp(argv[0], "dma")) {

    /* alloc icu_dma_mem and pwm_dma_mem */
	if ((pwm_dma_mem = (uint16_t*)hs_malloc(BUF_SIZE, __MT_DMA)) == NULL) {
		error = TRUE;
		goto end;
	}

	if ((icu_dma_mem = (uint16_t*)hs_malloc(BUF_SIZE, __MT_DMA)) == NULL) {
		error = TRUE;
		goto end;
	}

	pwm_dma_info.memAddr = pwm_dma_mem;
	icu_dma_info.memAddr = icu_dma_mem;

    pwmStart(pwmd, &pwm_dma_cfg);
    icuStart(icud, &icu_dma_cfg);

    /* init dma mem val
       period form 1 2 3 to 10 ms
       duty form 10 20 to 100
    */
    for (i=0; i<TEST_CASE_SIZE; i++) {
    	*(pwm_dma_mem+i*4+0) = 10+10*i - 1;
    	*(pwm_dma_mem+i*4+1) = 0;
    	*(pwm_dma_mem+i*4+2) = PWM_PERCENTAGE_TO_WIDTH(pwmd, 100+100*i);
    	*(pwm_dma_mem+i*4+3) = 0;
    }
    
    memset(icu_dma_mem, 0x0, BUF_SIZE);

    icuEnable(icud);
    pwmEnableChannel(pwmd, 0, 0);
    
    chThdSleepMilliseconds(3000);

    /*
    *   channel 0 and stops the drivers.
    */
    pwmDisableChannel(pwmd, 0);
    pwmStop(pwmd);
    icuDisable(icud);
    icuStop(icud);
    if (pwm_dma_mem)
      hs_free(pwm_dma_mem);
    if (icu_dma_mem)
      hs_free(icu_dma_mem);

    return;
  } else
#endif
  if (!strcmp(argv[0], "pio")) {
	msleep(100);

    pwmStart(pwmd, &pwm_pio_cfg);  
    icuStart(icud, &icu_pio_cfg);
    icuEnable(icud);

    msleep(100);
    /*
    * Starts the PWM channel 0 using 70% duty cycle.
    */
    pwmEnableChannel(pwmd, 0, PWM_PERCENTAGE_TO_WIDTH(pwmd, 7000));

    msleep(200);

    if ((last_period != 100) || (last_width != 70)) {
      //chprintf(chp, "(period,duty): (100,70) -> error (%d,%d)\r\n", last_period, last_width);
      chprintf(chp, "(period,duty): (100,50) -> error (%d,%d)\r\n", last_period, last_width);
      error = TRUE;
    }

    /*
    * Changes the PWM channel 0 to 50% duty cycle.
    */
    pwmEnableChannel(pwmd, 0, PWM_PERCENTAGE_TO_WIDTH(pwmd, 5000));

    msleep(100);

    if ((last_period != 100) || (last_width != 50)) {
      chprintf(chp, "(period,duty): (100,50) -> error (%d,%d)\r\n", last_period, last_width);
      error = TRUE;
    }

    /*
     * Changes the PWM channel 0 to 25% duty cycle.
     */
    pwmEnableChannel(pwmd, 0, PWM_PERCENTAGE_TO_WIDTH(pwmd, 2000));
  
    msleep(100);

    if ((last_period != 100) || (last_width != 20)) {
      chprintf(chp, "(period,duty): (100,20) -> error (%d,%d)\r\n", last_period, last_width);
      error = TRUE;
  }
#if 0
    /*
     * Changes PWM period to half second the duty cycle becomes 50%
     * implicitly.
     */
    pwmChangePeriod(pwmd, 5);

    msleep(100);

    if ((last_period != 50) || (last_width != 20)) {
      chprintf(chp, "(period,duty): (50,20) -> error (%d,%d)\r\n", last_period, last_width);
      error = TRUE;
    }
#endif
    /*
     * Disables channel 0 and stops the drivers.
     */
    pwmDisableChannel(pwmd, 0);
    pwmStop(pwmd);
    icuDisable(icud);
    icuStop(icud);

#if ICU_USE_DMA && PWM_USE_DMA
end:
#endif
    if (!error)
      chprintf(chp, "ICU & PWM PASS\r\n");
    else
      chprintf(chp, "ICU & PWM FAIL\r\n");
  }

}     

#else
void cmd_tim(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)chp;
  (void)argc;
  (void)argv;
}
#endif
