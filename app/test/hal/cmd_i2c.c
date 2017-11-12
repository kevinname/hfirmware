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

#include <string.h>
#include <stdlib.h>
#include "lib.h"
#if HS_USE_HS6760
#include "hs6760.h"
static hs_i2c_handler_t m_hs6760_handle;
#endif

/*===========================================================================*/
/* I2C driver test code: write & verify EEPROM                               */
/*===========================================================================*/

#define BUF_SIZE 256
#define EEPROM_PAGE_SIZE 32
#define EEPROM_SLAVE_ADDR  0X50

#if HAL_USE_I2C
uint8_t *tx = NULL, *rx =NULL;

typedef enum {
        I2C_DMA_OP = 1,
        I2C_PIO_OP,
        I2C_PWM_OP,
}i2c_test_mode_t;

#if HAL_USE_PWM
static void pwmpcb(PWMDriver *pwmp, void *arg) {

  (void)pwmp;
  (void)arg;
}

static void pwmc1cb(PWMDriver *pwmp, void *arg) {

  (void)pwmp;
  (void)arg;
}

#if PWM_USE_DMA

#endif
static PWMConfig pwmcfg = {
  100000,                                    /* 100kHz PWM clock frequency.   */
  500,                                    /* Initial PWM period 50mS.       */
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

#endif

/* I2C interface */
static const I2CConfig i2c_master_cfg = {
    OPMODE_I2C_MASTER,
    400000,
    CONMODE_I2C_NO_AUTO_WR,
    0,
    NULL
};

static const I2CConfig i2c_slave_cfg = {
    OPMODE_I2C_SLAVE,
    400000,
    CONMODE_I2C_NO_AUTO_WR,
    0,
    NULL
};

void i2c_callback_f(void *arg) {
        (void)arg;
}

static const I2CConfig i2c_master_autowr_cfg = {
    OPMODE_I2C_MASTER,
    400000,
    CONMODE_I2C_AUTO_WR,
#if HS_PWM_USE_TIM0
      0,
#elif HS_PWM_USE_TIM1
      4,
#elif HS_PWM_USE_TIM2
      8,
#else
      0,
#endif
    i2c_callback_f
};

#endif

static bool_t i2c_test_init(const I2CConfig *cfg) {
   uint16_t i;

   if ((tx = (uint8_t*)hs_malloc(BUF_SIZE, __MT_DMA)) == NULL)
	  return false;

   if ((rx = (uint8_t*)hs_malloc(BUF_SIZE, __MT_DMA)) == NULL)
	  return false;
   
   for (i=0; i<BUF_SIZE; i++)
     tx[i] = i;

   memset(rx, 0x00, BUF_SIZE);

   i2cStart(&I2CD0, cfg);
   i2cAcquireBus(&I2CD0);
   return true;
}

static void i2c_test_uninit(void) {
    i2cReleaseBus(&I2CD0);
    i2cStop(&I2CD0);
    if (tx)
      hs_free(tx);
    if (rx)
      hs_free(rx);
}
static bool i2c_eep_autorw_test(BaseSequentialStream *chp) {
     msg_t status = MSG_OK;     
     uint8_t addr = EEPROM_SLAVE_ADDR; //EEPROM's i2c slave address
     uint16_t offset = 0x0000; //offset address in EEPROM
     size_t count = 32;
     systime_t tmo = MS2ST(100);
     uint16_t j;

     PWMDriver *pd =
 #if HS_PWM_USE_TIM0
     &PWMD0 ;
 #elif HS_PWM_USE_TIM1
     &PWMD1 ;
 #elif HS_PWM_USE_TIM2
     &PWMD2 ;
 #endif

     for(j=0; j<count; j++)
       *(tx+j) = j;
     
     chprintf(chp, "I2C test read and write EEPROM, addr: 0x%x, offset: 0x%x, count: 0x%x, ", addr, offset, count);

      /* start timer pwm */
     pwmStart(pd, &pwmcfg);
     
     pwmEnableChannel(pd, 0, PWM_PERCENTAGE_TO_WIDTH(pd, 5000));
      
     status = i2cMasterWriteMemTimeout(&I2CD0, addr, offset, 2, tx, count, tmo);
     if (status != MSG_OK) {
       pwmDisableChannel(pd, 0);
       pwmStop(pd);
       chprintf(chp,"  fail.\r\n");
       return false;
     }
     
     pwmDisableChannel(pd, 0);
    
     chThdSleepMilliseconds(100);
    
     pwmEnableChannel(pd, 0, PWM_PERCENTAGE_TO_WIDTH(pd, 5000));
     
     status = i2cMasterReadMemTimeout(&I2CD0, addr, offset, 2, rx, count, tmo);
    
    pwmDisableChannel(pd, 0);
    
    pwmStop(pd);
    
    if (status != MSG_OK) {
      chprintf(chp,"  fail.\r\n");
      return false;
    }
    
    for (j=0; j<count; j++) {
      if (tx[j] != rx[j]) {
	     chprintf(chp, "0x%04lx: 0x%02x -> 0x%02x\r\n",
		 offset+j, tx[j], rx[j]);
         chprintf(chp,"  fail.\r\n");
         return false;
      }
    }

    chprintf(chp," pass.\r\n");
    return true; 
}

static bool i2c_eep_rw_test(BaseSequentialStream *chp){
    msg_t status = MSG_OK;     
    uint8_t addr = EEPROM_SLAVE_ADDR; //EEPROM's i2c slave address
    uint16_t offset = 0x0000; //offset address in EEPROM
    size_t count = 8;
    systime_t tmo = MS2ST(100);
    uint16_t i, j;
    
    for (i = 0; i < 10; i++) {
    /* get random count, aligned to 8 pages */
    chSysLock();
    count = rand() & (BUF_SIZE - 1);
    chSysUnlock();
    count &= ~(EEPROM_PAGE_SIZE * 8 - 1);
    if ((0 == count) || (count > BUF_SIZE))
      count = EEPROM_PAGE_SIZE * 8;
       
    //chprintf(chp, "count: %d ", count);
    offset += count;

    /* fill random pattern */
    for (j=0; j < count; j++)
      tx[j] = rand();
    
    chprintf(chp, "I2C test read and write EEPROM, addr: 0x%x, offset: 0x%x, count: 0x%x, ", addr, offset, count);
    
    /* write in 3 times: 1/4, 1/4, 1/2 */
    status = i2cMasterWriteMemTimeout(&I2CD0, addr, offset, 2, tx, count/4, tmo);
    if (status != MSG_OK)
      return false;
 
    chThdSleepMilliseconds(100);
    status = i2cMasterWriteMemTimeout(&I2CD0, addr, offset+count/4, 2, tx+count/4, count/4, tmo);
    if (status != MSG_OK)
      return false;

    chThdSleepMilliseconds(100);
    status = i2cMasterWriteMemTimeout(&I2CD0, addr, offset+count/2, 2, tx+count/2, count/2, tmo);
    if (status != MSG_OK)
      return false;
 
    chThdSleepMilliseconds(100);
    status = i2cMasterReadMemTimeout(&I2CD0, addr, offset, 2, rx, count, tmo);
    if (status != MSG_OK)
      return false;     
   
    for (j=0; j<count; j++) {
      if (tx[j] != rx[j]) {
	     chprintf(chp, "0x%04lx: 0x%02x -> 0x%02x\r\n",
		 offset+j, tx[j], rx[j]);
         chprintf(chp,"  fail.\r\n");
         return false;
      }
    }
    chprintf(chp," pass.\r\n");
  }
    
    return true;
}

/* only use timer channel0 test */
static bool i2c_rw_test(i2c_test_mode_t mod, BaseSequentialStream *chp) {
    bool res;
    const I2CConfig *cfgp;
    
    if (mod == I2C_PWM_OP) {
      cfgp = &i2c_master_autowr_cfg;
    } else 
      cfgp = &i2c_master_cfg;
    
    if(!i2c_test_init(cfgp))
      return false;

    if (mod == I2C_PWM_OP)
      res = i2c_eep_autorw_test(chp);
    else
      res = i2c_eep_rw_test(chp);
    
    i2c_test_uninit();
    return res;            
}

void cmd_i2c(BaseSequentialStream *chp, int argc, char *argv[]) {
#if HAL_USE_I2C
  msg_t status = MSG_OK;
  systime_t tmo = MS2ST(100);
  bool_t res = false;
  
  /* FPGA: set pad13 pad14 i2c function */
  //palSetPadMode(IOPORT0, 13, PAL_MODE_OUTPUT| PAL_MODE_ALTERNATE(PAD_FUNC_I2C_SCK)|PAL_MODE_DRIVE_CAP(3));
  //palSetPadMode(IOPORT0, 14, PAL_MODE_INPUT | PAL_MODE_ALTERNATE(PAD_FUNC_I2C_SDA)|PAL_MODE_DRIVE_CAP(3));
#if __DEVELOPE_BOARD__
  /* SCL: PB4 =CON32.3                   extboard.J20
     SCL: PA15=CON32.24=extboard.PA8 ... extboard.J20
     SDA: PA7 =CON32.26=extboard.PA6 ... extboard.J21 */
  palSetPadMode(IOPORT0, 15, PAL_MODE_OUTPUT| PAL_MODE_ALTERNATE(PAD_FUNC_I2C_SCK)|PAL_MODE_DRIVE_CAP(3));
  palSetPadMode(IOPORT0, 7, PAL_MODE_INPUT | PAL_MODE_ALTERNATE(PAD_FUNC_I2C_SDA)|PAL_MODE_DRIVE_CAP(3));
#endif

  argc--;
  if ((strcmp(argv[0], "write") == 0) || (strcmp(argv[0], "read") == 0)) {
    size_t i, count = 8;
    uint32_t pos = 0;
    if (argc >= 1)
      pos = strtoul(argv[1], NULL, 16);
    if (argc >= 2)
      count = atoi(argv[2]);

    if ((tx = (uint8_t*)hs_malloc(count, __MT_DMA)) == NULL)
      return;
    i2cStart(&I2CD0, &i2c_master_cfg);
    i2cAcquireBus(&I2CD0);

    if (strcmp(argv[0], "write") == 0) {
      for (i=0; i<count; i++)
        tx[i] = i;
      i2cMasterWriteMemTimeout(&I2CD0, EEPROM_SLAVE_ADDR, pos, 2, tx, count, tmo);
    }
    else {
      i2cMasterReadMemTimeout(&I2CD0, EEPROM_SLAVE_ADDR, pos, 2, tx, count, tmo);
      for (i=0; i<count; i++)
          hs_printf("%02x ", tx[i]);
        hs_printf("\n");
    }

    i2cReleaseBus(&I2CD0);
    i2cStop(&I2CD0);
    hs_free(tx);
    return;
  }

  if (strcmp(argv[0], "drvhal") == 0) {
    int hz;
    size_t i, count = rand() & (BUF_SIZE - 1);

    count &= ~(EEPROM_PAGE_SIZE * 8 - 1);
    if ((0 == count) || (count > BUF_SIZE))
      count = EEPROM_PAGE_SIZE * 8;
    if ((rx = (uint8_t*)hs_malloc(count * 2, __MT_DMA)) == NULL)
      return;
    tx = rx + count;

    for (hz = 100; hz <= 1000; hz += 100) {
      int ret;
      hs_i2c_handler_t handle;
      for (i=0; i<count; i++)
        tx[i] = rand();
      handle = hs_i2c_init(hz*1000, EEPROM_SLAVE_ADDR, 7/*7-bit slave address*/, 2/*2-byte addressing*/);
      if (!handle)
        goto fail_out;
      ret = hs_i2c_write_eeprom(handle, count & (EEPROM_PAGE_SIZE-1), tx, count);
      if (ret < 0) {
        hs_i2c_uninit(handle);
        goto fail_out;
      }
      /* hs_i2c_write() doesn't support write eeprom cross page boundray */
      ret = hs_i2c_write(handle, count & (EEPROM_PAGE_SIZE-1), tx, 1);
      if (ret < 0) {
        hs_i2c_uninit(handle);
        goto fail_out;
      }
      /* eeprom requires delay after write a page */
      osDelay(10);
      ret = hs_i2c_read(handle, count & (EEPROM_PAGE_SIZE-1), rx, count);
      if (ret < 0) {
        hs_i2c_uninit(handle);
        goto fail_out;
      }
      for (i=0; i<count; i++) {
        if (rx[i] != tx[i]) {
          hs_printf("rx %02x differs tx %02x at offset %d\n", rx[i], tx[i], (count & (EEPROM_PAGE_SIZE-1)) + i);
          break;
        }
      }
      hs_printf("hz=%dkHz ", hz);
      if (i >= count)
        hs_printf("PASS\n");
      else {
        for (i=0; i<8; i++)
          hs_printf("%02x ", rx[i]);
        hs_printf("\n");
      }
      hs_i2c_uninit(handle);
    }
    hs_free(rx);
    return;

  fail_out:
    hs_printf("FAIL\n");
    hs_free(rx);
    return;
  }

 if (strcmp(argv[0], "slave") == 0) {

    if(!i2c_test_init(&i2c_slave_cfg))
      return;
    
    if (strcmp(argv[1], "read") == 0) {
      status = i2cSlaveReceiveTimeout(&I2CD0, rx, 32, tmo*100000);
      if (status != MSG_OK)   
      	chprintf(chp, "i2c slave read failed.");    
    } else if (strcmp(argv[1], "write") == 0) {
      status =  i2cSlaveTransmitTimeout(&I2CD0, tx, 32, tmo*10000);
      if (status != MSG_OK) 
        chprintf(chp, "i2c slave write failed.");       
    }
    
    i2c_test_uninit();
    return;
  } else if (strcmp(argv[0], "pwm") == 0) {
    chprintf(chp, "I2C eeprom pwm auto wr test :\r\n");
    res = i2c_rw_test(I2C_PWM_OP, chp);
  } else if (strcmp(argv[0], "dma") == 0) {
    chprintf(chp, "I2C eeprom dma wr test :\r\n");
    res = i2c_rw_test(I2C_DMA_OP, chp);      
  } else if (strcmp(argv[0], "pio") == 0) {
    chprintf(chp, "I2C eeprom pio wr test :\r\n");
    res = i2c_rw_test(I2C_PIO_OP, chp);      
  } else {
    chprintf(chp, "I2C eeprom pwm auto wr test :\r\n");
    res = i2c_rw_test(I2C_PWM_OP, chp);
    if (!res)
       goto end;
    chprintf(chp, "I2C eeprom dma wr test :\r\n");
    res = i2c_rw_test(I2C_DMA_OP, chp);
    if (!res)
       goto end;
    chprintf(chp, "I2C eeprom pio wr test :\r\n");
    res = i2c_rw_test(I2C_PIO_OP, chp);
    if (!res)
       goto end;
  }
  
end:
   if (res)
     chprintf(chp, "PASS\r\n");
   else
     chprintf(chp, "FAIL\r\n");

    return;
    
#endif /* HAL_USE_I2C */
}

void cmd_hs6760(BaseSequentialStream *chp, int argc, char *argv[]) {
#if HAL_USE_I2C && HS_USE_HS6760
  argc--;

  if (strncmp(argv[0], "init", strlen("init")) == 0) {
    m_hs6760_handle = hs6760_fm_open(XTAL_24M);
    return;
  }

  if (strncmp(argv[0], "freq", strlen("freq")) == 0) {
    int freq = 1043;
    if (argc >= 1)
      freq = atoi(argv[1]);
    hs6760_fm_set_freq(freq);
    return;
  }

  if (strncmp(argv[0], "reg", strlen("reg")) == 0) {
    uint8_t pos = 0, val;
    if (0 == m_hs6760_handle) {
      chprintf(chp, "hs6760 init\r\n");
      return;
    }
    if (argc >= 1) {
      pos = strtoul(argv[1], NULL, 16);
      if (argc >= 2) {
        val = strtoul(argv[2], NULL, 16);
        hs_i2c_write(m_hs6760_handle, pos, &val, 1);
        chprintf(chp, "write 0x%02x @0x%02x\r\n", val, pos);
      }
      else {
        hs_i2c_read(m_hs6760_handle, pos, &val, 1);
        chprintf(chp, "read 0x%02x @0x%02x\r\n", val, pos);
      }
    }
    else {
      chprintf(chp, "dump all:\r\n");
      for (pos = 0; pos <= 0x11; pos++) {
        hs_i2c_read(m_hs6760_handle, pos, &val, 1);
        chprintf(chp, "0x%02x ", val);
      }
      chprintf(chp, "\r\n");
    }
    return;
  }

  return;
#else
  (void)chp;
  (void)argc;
  (void)argv;
#endif /* HAL_USE_I2C && HS_USE_HS6760 */
}
