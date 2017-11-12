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

/**
 * @file    hs66xx/sf_lld.c
 * @brief   SPI flash interface Driver subsystem low level driver source template.
 *
 * @addtogroup SF
 * @{
 */

#include "ch.h"
#include "hal.h"

#if HAL_USE_SF || defined(__DOXYGEN__)



/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/



/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   SF driver identifier.
 */
#if HS_SF_USE_SF0 && !defined(__DOXYGEN__)
SFDriver SFD;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

static const SFlashInfo_t g_SFInfo[] = {

  {"General flash",   0xFFFFFF, 256, 4*1024,   0, 64*1024,  0},
  {"EN25F40",         0x1c3113, 256, 4*1024, 128, 64*1024,  8},
  {"EN25T80",         0x1c5114, 256, 4*1024, 256, 64*1024, 16},
};

static const uint32_t  g_SFCnt = sizeof(g_SFInfo)/sizeof(SFlashInfo_t);

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Handling of stalled SF transactions.
 *
 * @param[in] sfp      pointer to the @p SFDriver object
 *
 * @notapi
 */
#ifndef RUN_IN_FLASH
static void sf_safety_timeout(void *p) {
  SFDriver *sfp = (SFDriver *)p;

  chSysLockFromISR();
  if (sfp->thread) {
    thread_t *tp = sfp->thread;
    sfp->thread = NULL;
    tp->p_u.rdymsg = MSG_TIMEOUT;
    chSchReadyI(tp);
  }
  chSysUnlockFromISR();
}
#endif

#ifdef RUN_IN_FLASH

#if HS_USE_LEDDISP
void hs_led_frameDisp(void) __ONCHIP_CODE__;
#endif

static void sf_wait(SFDriver *sfp) __ONCHIP_CODE__;
static void sf_wait(SFDriver *sfp)
{
  HS_SF_Type *sfrp = sfp->sf;
  uint32_t t = 0;

  sfrp->RAW_INTR_STATUS = sfrp->RAW_INTR_STATUS;  
  
  while(((sfrp->RAW_INTR_STATUS & 1) == 0) && (t++ < 0x50000000)) ;
  
  sfrp->RAW_INTR_STATUS = sfrp->RAW_INTR_STATUS;
  sfrp->CONFIGURATION_1 = sfp->fmode;
}
#endif

static uint32_t sf_read_mid(SFDriver *sfp) __ONCHIP_CODE__;
static uint32_t sf_read_mid(SFDriver *sfp) {
  HS_SF_Type *sfrp = sfp->sf;
  const SFConfig *sfcfgp = sfp->config;
  uint32_t mid, cmd_r = 0; 
  #ifndef RUN_IN_FLASH
  virtual_timer_t vt={0};
  
  /* Global timeout for the whole operation.*/
  if (sfp->timeout != TIME_INFINITE)
    chVTSetI(&vt, sfp->timeout, sf_safety_timeout, (void *)sfp);
  #endif

  sfrp->CONFIGURATION_1 = 0;

  cmd_r |= SF_CMD_WIDTH(32);
  cmd_r |= (sfcfgp->cs & 3) << 4;
  cmd_r |= SF_CMD_READ;
  cmd_r |= SF_CMD_KEEPCS;

  sfrp->COMMAND_DATA0_REG = SF_CMD_RDID;
  sfrp->COMMAND = cmd_r;

  #ifdef RUN_IN_FLASH
  uint32_t t=0x10000000;

  sfrp->RAW_INTR_STATUS = sfrp->RAW_INTR_STATUS;
  while(((sfrp->RAW_INTR_STATUS & 1) == 0) && (t--)) ;
  
  sfrp->RAW_INTR_STATUS = sfrp->RAW_INTR_STATUS;
  sfrp->CONFIGURATION_1 = sfp->fmode;
  //sf_wait(sfp);
  #else
  /* Waits for the operation completion or a timeout.*/
  sfp->thread = currp;
  chSchGoSleepS(CH_STATE_SUSPENDED);
  if ((sfp->timeout != TIME_INFINITE) && chVTIsArmedI(&vt))
    chVTResetI(&vt);
  sfrp->CONFIGURATION_1 = sfp->fmode;
  #endif

  mid = sfrp->READ0_REG & 0xffffff;

  return mid;   
}

static uint32_t sf_read_status_0(SFDriver *sfp) __ONCHIP_CODE__;
static uint32_t sf_read_status_0(SFDriver *sfp) {
  HS_SF_Type *sfrp = sfp->sf;
  const SFConfig *sfcfgp = sfp->config;
  uint32_t status, cmd_r = 0;
  #ifndef RUN_IN_FLASH
  virtual_timer_t vt={0};
  
  /* Global timeout for the whole operation.*/
  if (sfp->timeout != TIME_INFINITE)
    chVTSetI(&vt, sfp->timeout, sf_safety_timeout, (void *)sfp);
  #endif

  sfrp->CONFIGURATION_1 = 0;
  
  cmd_r |= SF_CMD_WIDTH(16);
  cmd_r |= (sfcfgp->cs & 3) << 4;
  cmd_r |= SF_CMD_READ;
  cmd_r |= SF_CMD_KEEPCS;

  sfrp->COMMAND_DATA0_REG = SF_CMD_RDSTAS;
  sfrp->COMMAND = cmd_r;

  #ifdef RUN_IN_FLASH
  sf_wait(sfp);
  #else
  /* Waits for the operation completion or a timeout.*/
  sfp->thread = currp;
  chSchGoSleepS(CH_STATE_SUSPENDED);
  if ((sfp->timeout != TIME_INFINITE) && chVTIsArmedI(&vt))
    chVTResetI(&vt);
  sfrp->CONFIGURATION_1 = sfp->fmode;
  #endif
  status = sfrp->READ0_REG & 0xff;

  return status;   
}

static uint32_t sf_read_status_1(SFDriver *sfp) __ONCHIP_CODE__;
static uint32_t sf_read_status_1(SFDriver *sfp) {
  HS_SF_Type *sfrp = sfp->sf;
  const SFConfig *sfcfgp = sfp->config;
  uint32_t status, cmd_r = 0;
  #ifndef RUN_IN_FLASH
  virtual_timer_t vt={0};
  
  /* Global timeout for the whole operation.*/
  if (sfp->timeout != TIME_INFINITE)
    chVTSetI(&vt, sfp->timeout, sf_safety_timeout, (void *)sfp);
  #endif
  sfrp->CONFIGURATION_1 = 0;

  cmd_r |= SF_CMD_WIDTH(16);
  cmd_r |= (sfcfgp->cs & 3) << 4;
  cmd_r |= SF_CMD_READ;
  cmd_r |= SF_CMD_KEEPCS;

  sfrp->COMMAND_DATA0_REG = SF_CMD_RDSTAS1;
  sfrp->COMMAND = cmd_r;

  #ifdef RUN_IN_FLASH
  sf_wait(sfp);
  #else
  /* Waits for the operation completion or a timeout.*/
  sfp->thread = currp;
  chSchGoSleepS(CH_STATE_SUSPENDED);
  if ((sfp->timeout != TIME_INFINITE) && chVTIsArmedI(&vt))
    chVTResetI(&vt);
  sfrp->CONFIGURATION_1 = sfp->fmode;
  #endif

  status = sfrp->READ0_REG & 0xff;

  return status;   
}

uint32_t sf_read_status(SFDriver *sfp) __ONCHIP_CODE__;
uint32_t sf_read_status(SFDriver *sfp){
  uint32_t status;

  status  = sf_read_status_1(sfp) << 8;
  status = sf_read_status_0(sfp) + status;

  return status;
}
  

static void sf_write_status(SFDriver *sfp, uint32_t status) __ONCHIP_CODE__;
static void sf_write_status(SFDriver *sfp, uint32_t status) {
  HS_SF_Type *sfrp = sfp->sf;
  const SFConfig *sfcfgp = sfp->config;
  uint32_t cmd_r = 0, cmd_data = 0;
  #ifndef RUN_IN_FLASH
  virtual_timer_t vt={0};
  
  /* Global timeout for the whole operation.*/
  if (sfp->timeout != TIME_INFINITE)
    chVTSetI(&vt, sfp->timeout, sf_safety_timeout, (void *)sfp);
  #endif
  sfrp->CONFIGURATION_1 = 0;

  cmd_r |= SF_CMD_WIDTH(24);
  cmd_r |= (sfcfgp->cs & 3) << 4;
  cmd_r |= SF_CMD_WRITE;
  cmd_r |= SF_CMD_KEEPCS;

  cmd_data = SF_CMD_WRSTAS;
  cmd_data |= (status & 0xff) << 16;
  cmd_data |= ((status >> 8) & 0xff) << 8;

  sfrp->COMMAND_DATA0_REG = cmd_data;
  sfrp->COMMAND = cmd_r;    

  #ifdef RUN_IN_FLASH
  sf_wait(sfp);
  #else
  /* Waits for the operation completion or a timeout.*/
  sfp->thread = currp;
  chSchGoSleepS(CH_STATE_SUSPENDED);
  if ((sfp->timeout != TIME_INFINITE) && chVTIsArmedI(&vt))
    chVTResetI(&vt);
  sfrp->CONFIGURATION_1 = sfp->fmode;
  #endif

  while(1) {
    status = sf_read_status(sfp);
    if((status & 1) == 0) {
      break;
    }
  }   
}

static void sf_write_enable(SFDriver *sfp) __ONCHIP_CODE__;
static void sf_write_enable(SFDriver *sfp) {
  HS_SF_Type *sfrp = sfp->sf;
  const SFConfig *sfcfgp = sfp->config;
  uint32_t cmd_r = 0;
  #ifndef RUN_IN_FLASH
  virtual_timer_t vt={0};
  
  /* Global timeout for the whole operation.*/
  if (sfp->timeout != TIME_INFINITE)
    chVTSetI(&vt, sfp->timeout, sf_safety_timeout, (void *)sfp);
  #endif
  sfrp->CONFIGURATION_1 = 0;

  cmd_r |= SF_CMD_WIDTH(8);
  cmd_r |= (sfcfgp->cs & 3) << 4;
  cmd_r |= SF_CMD_WRITE;
  cmd_r |= SF_CMD_KEEPCS;

  sfrp->COMMAND_DATA0_REG = SF_CMD_WREN;
  sfrp->COMMAND = cmd_r;

  #ifdef RUN_IN_FLASH
  sf_wait(sfp);
  #else
  /* Waits for the operation completion or a timeout.*/
  sfp->thread = currp;
  chSchGoSleepS(CH_STATE_SUSPENDED);
  if ((sfp->timeout != TIME_INFINITE) && chVTIsArmedI(&vt))
    chVTResetI(&vt);
  sfrp->CONFIGURATION_1 = sfp->fmode;
  #endif
}

#if 0
static void sf_write_hpm(SFDriver *sfp) __ONCHIP_CODE__;
static void sf_write_hpm(SFDriver *sfp) {
  HS_SF_Type *sfrp = sfp->sf;
  const SFConfig *sfcfgp = sfp->config;
  uint32_t cmd_r = 0;
  virtual_timer_t vt={0};

  /* Global timeout for the whole operation.*/
  if (sfp->timeout != TIME_INFINITE)
    chVTSetI(&vt, sfp->timeout, sf_safety_timeout, (void *)sfp);

  cmd_r |= SF_CMD_WIDTH(32);
  cmd_r |= (sfcfgp->cs & 3) << 4;
  cmd_r |= SF_CMD_WRITE;
  cmd_r |= SF_CMD_KEEPCS;

  sfrp->COMMAND_DATA0_REG = SF_CMD_HPM;
  sfrp->COMMAND = cmd_r;

  /* wait for finished */
  sfp->thread = currp;
  chSchGoSleepS(CH_STATE_SUSPENDED);
  if ((sfp->timeout != TIME_INFINITE) && chVTIsArmedI(&vt))
    chVTResetI(&vt);
}
#endif

static void sf_wrStart(SFDriver *sfp) __ONCHIP_CODE__;
static void sf_wrStart(SFDriver *sfp){
  uint32_t status;
  status = sf_read_status(sfp);

  sf_write_enable(sfp);
  sf_write_status(sfp, (status&0x783));  
}

static void sf_setflash_wren(SFDriver *sfp) __ONCHIP_CODE__;
static void sf_setflash_wren(SFDriver *sfp) {
  uint32_t status;

  //status = sf_read_status(sfp);

  //sf_write_enable(sfp);
  //sf_write_status(sfp, (status&0xff03)|0x02);

  while(1) {
    sf_write_enable(sfp);

    status = sf_read_status(sfp);
    if((status & 0x3) == 2) {
      break;
    }
    
    if((status & 0x1c) != 0) {      
      sf_write_status(sfp, (status&0xff03) | 0x03);
      sf_write_enable(sfp);
    }
  }
}

static void sf_chip_erase(SFDriver *sfp) __ONCHIP_CODE__;
static void sf_chip_erase(SFDriver *sfp) {
  HS_SF_Type *sfrp = sfp->sf;
  const SFConfig *sfcfgp = sfp->config;
  uint32_t status, cmd_r = 0;
  #ifndef RUN_IN_FLASH
  virtual_timer_t vt={0};

  sf_setflash_wren(sfp);
  /* Global timeout for the whole operation.*/
  if (sfp->timeout != TIME_INFINITE)
    chVTSetI(&vt, sfp->timeout, sf_safety_timeout, (void *)sfp);
  #else
  sf_setflash_wren(sfp);
  #endif
  sfrp->CONFIGURATION_1 = 0;
  
  cmd_r |= SF_CMD_WIDTH(8);
  cmd_r |= (sfcfgp->cs & 3) << 4;
  cmd_r |= SF_CMD_WRITE;
  cmd_r |= SF_CMD_KEEPCS;

  sfrp->COMMAND_DATA0_REG = SF_CMD_CE;
  sfrp->COMMAND = cmd_r;

  #ifdef RUN_IN_FLASH
  sf_wait(sfp);
  #else
  /* Waits for the operation completion or a timeout.*/
  sfp->thread = currp;
  chSchGoSleepS(CH_STATE_SUSPENDED);
  if ((sfp->timeout != TIME_INFINITE) && chVTIsArmedI(&vt))
    chVTResetI(&vt);
  sfrp->CONFIGURATION_1 = sfp->fmode;
  #endif

  while(1) {
    status = sf_read_status_0(sfp);
    if((status & 1) == 0) {
      break;
    }
  }     
}

static void sf_sector_erase(SFDriver *sfp, uint32_t offset) __ONCHIP_CODE__;
static void sf_sector_erase(SFDriver *sfp, uint32_t offset) {
  HS_SF_Type *sfrp = sfp->sf;
  const SFConfig *sfcfgp = sfp->config;
  uint32_t status, cnt, cmd_r = 0, cmd_d = 0;
  #ifndef RUN_IN_FLASH
  virtual_timer_t vt={0};

  sf_setflash_wren(sfp);
  /* Global timeout for the whole operation.*/
  if (sfp->timeout != TIME_INFINITE)
    chVTSetI(&vt, sfp->timeout, sf_safety_timeout, (void *)sfp);
  #else
  sf_setflash_wren(sfp);
  #endif  

  sfrp->CONFIGURATION_1 = 0;

  cmd_r |= SF_CMD_WIDTH(32);
  cmd_r |= (sfcfgp->cs & 3) << 4;
  cmd_r |= SF_CMD_WRITE;
  cmd_r |= SF_CMD_KEEPCS;

  cmd_d |= SF_CMD_SE;
  cmd_d |= offset & 0xffffff;

  sfrp->COMMAND_DATA0_REG = cmd_d;
  sfrp->COMMAND = cmd_r;

  #ifdef RUN_IN_FLASH
  sf_wait(sfp);
  #else
  /* Waits for the operation completion or a timeout.*/
  sfp->thread = currp;
  chSchGoSleepS(CH_STATE_SUSPENDED);
  if ((sfp->timeout != TIME_INFINITE) && chVTIsArmedI(&vt))
    chVTResetI(&vt);
  sfrp->CONFIGURATION_1 = sfp->fmode;
  #endif

  cnt = 0;
  while(1) {
    status = sf_read_status_0(sfp);
    if((status & 1) == 0) {

      #if HS_USE_LEDDISP  
      hs_led_frameDisp();
      #endif
      break;
    }

    #if HS_USE_LEDDISP  
    cnt += 1;
    if((cnt % 15) == 0)
      hs_led_frameDisp();
    #endif
  }     
}

static void sf_enter_deepPD(SFDriver *sfp) __ONCHIP_CODE__;
static void sf_enter_deepPD(SFDriver *sfp) {
  HS_SF_Type *sfrp = sfp->sf;
  const SFConfig *sfcfgp = sfp->config;
  uint32_t cmd_r = 0;
  #ifndef RUN_IN_FLASH
  virtual_timer_t vt={0};
  
  /* Global timeout for the whole operation.*/
  if (sfp->timeout != TIME_INFINITE)
    chVTSetI(&vt, sfp->timeout, sf_safety_timeout, (void *)sfp);
  #endif
  sfrp->CONFIGURATION_1 = 0;
  
  cmd_r |= SF_CMD_WIDTH(8);
  cmd_r |= (sfcfgp->cs & 3) << 4;
  cmd_r |= SF_CMD_WRITE;
  cmd_r |= SF_CMD_KEEPCS;

  sfrp->COMMAND_DATA0_REG = SF_CMD_DEEPPD;
  sfrp->COMMAND = cmd_r;

  #ifdef RUN_IN_FLASH
  sf_wait(sfp);
  #else
  /* Waits for the operation completion or a timeout.*/
  sfp->thread = currp;
  chSchGoSleepS(CH_STATE_SUSPENDED);
  if ((sfp->timeout != TIME_INFINITE) && chVTIsArmedI(&vt))
    chVTResetI(&vt);
  sfrp->CONFIGURATION_1 = sfp->fmode;
  #endif
}

static void sf_release_deepPD(SFDriver *sfp) __ONCHIP_CODE__;
static void sf_release_deepPD(SFDriver *sfp) {
  HS_SF_Type *sfrp = sfp->sf;
  const SFConfig *sfcfgp = sfp->config;
  uint32_t cmd_r = 0;
  #ifndef RUN_IN_FLASH
  virtual_timer_t vt={0};
  
  /* Global timeout for the whole operation.*/
  if (sfp->timeout != TIME_INFINITE)
    chVTSetI(&vt, sfp->timeout, sf_safety_timeout, (void *)sfp);
  #endif

  sfrp->CONFIGURATION_1 = 0;
  
  cmd_r |= SF_CMD_WIDTH(8);
  cmd_r |= (sfcfgp->cs & 3) << 4;
  cmd_r |= SF_CMD_WRITE;
  cmd_r |= SF_CMD_KEEPCS;

  sfrp->COMMAND_DATA0_REG = SF_CMD_RELEASEPD;
  sfrp->COMMAND = cmd_r;

  #ifdef RUN_IN_FLASH
  sf_wait(sfp);
  #else
  /* Waits for the operation completion or a timeout.*/
  sfp->thread = currp;
  chSchGoSleepS(CH_STATE_SUSPENDED);
  if ((sfp->timeout != TIME_INFINITE) && chVTIsArmedI(&vt))
    chVTResetI(&vt);
  sfrp->CONFIGURATION_1 = sfp->fmode;
  #endif
}


static void sf_read(SFDriver *sfp, uint32_t offset, void *rdbuf, size_t len) __ONCHIP_CODE__;
static void sf_read(SFDriver *sfp, uint32_t offset, void *rdbuf, size_t len) {
  HS_SF_Type *sfrp = sfp->sf;
  const SFConfig *sfcfgp = sfp->config;
  uint32_t cmd_r = 0, cmd_d = 0;
  #ifndef RUN_IN_FLASH
  virtual_timer_t vt={0};
  
  /* Global timeout for the whole operation.*/
  if (sfp->timeout != TIME_INFINITE)
    chVTSetI(&vt, sfp->timeout, sf_safety_timeout, (void *)sfp);
  #else
  //uint32_t t = 0x10000;
  //while(t--) __nds32__nop();
  #endif

  sfrp->CONFIGURATION_1 = 0;

  cmd_r |= SF_DATA_CNT(len);
  cmd_r |= SF_CMD_WIDTH(40);
  cmd_r |= (sfcfgp->cs & 3) << 4;
  cmd_r |= SF_CMD_READ;
  cmd_r |= SF_CMD_KEEPCS;

  cmd_d |= SF_CMD_FTRD;
  cmd_d |= offset & 0xffffff;

  sfrp->COMMAND_DATA0_REG = cmd_d;
  sfrp->COMMAND_DATA1_REG = 0;
  sfrp->ADDRESS_REG = (uint32_t)rdbuf;  
  sfrp->COMMAND = cmd_r;

  #ifdef RUN_IN_FLASH
  sf_wait(sfp);
  #else
  /* Waits for the operation completion or a timeout.*/
  sfp->thread = currp;
  chSchGoSleepS(CH_STATE_SUSPENDED);
  if ((sfp->timeout != TIME_INFINITE) && chVTIsArmedI(&vt))
    chVTResetI(&vt);
  sfrp->CONFIGURATION_1 = sfp->fmode;
  #endif
}

#if 0
static void sf_qread(SFDriver *sfp, uint32_t offset, void *rdbuf, size_t len) {
  HS_SF_Type *sfrp = sfp->sf;
  const SFConfig *sfcfgp = sfp->config;
  uint32_t cmd_r = 0, cmd_d = 0;
  virtual_timer_t vt={0};

  /* Global timeout for the whole operation.*/
  if (sfp->timeout != TIME_INFINITE)
    chVTSetI(&vt, sfp->timeout, sf_safety_timeout, (void *)sfp);

  cmd_r |= SF_DATA_CNT(len);
  cmd_r |= SF_CMD_WIDTH(40);
  cmd_r |= (sfcfgp->cs & 3) << 4;
  cmd_r |= SF_CMD_READ;
  cmd_r |= SF_CMD_KEEPCS;

  cmd_d |= SF_CMD_FTQRD;
  cmd_d |= offset & 0xffffff;

  sfrp->COMMAND_DATA0_REG = cmd_d;
  sfrp->COMMAND_DATA1_REG = 0;
  sfrp->ADDRESS_REG = (uint32_t)rdbuf;  
  sfrp->COMMAND = cmd_r;

  /* wait for finished */
  sfp->thread = currp;
  chSchGoSleepS(CH_STATE_SUSPENDED);
  if ((sfp->timeout != TIME_INFINITE) && chVTIsArmedI(&vt))
    chVTResetI(&vt);
}
#endif

static void sf_page_write(SFDriver *sfp, uint32_t offset, void *wrbuf, size_t len) __ONCHIP_CODE__;
static void sf_page_write(SFDriver *sfp, uint32_t offset, void *wrbuf, size_t len) {
  HS_SF_Type *sfrp = sfp->sf;
  const SFConfig *sfcfgp = sfp->config;
  const SFlashInfo_t *sfinfop = sfp->sfinfo;
  uint32_t status, cmd_r = 0, cmd_d = 0;
  #ifndef RUN_IN_FLASH
  virtual_timer_t vt={0}; 
  #endif

  if(len > sfinfop->page_size) {
    return ;
  }

  sf_setflash_wren(sfp);  

  #ifndef RUN_IN_FLASH
 /* Global timeout for the whole operation.*/
  if (sfp->timeout != TIME_INFINITE)
    chVTSetI(&vt, sfp->timeout, sf_safety_timeout, (void *)sfp);
  #endif
  sfrp->CONFIGURATION_1 = 0;
  
  cmd_r |= SF_DATA_CNT(len);
  cmd_r |= SF_CMD_WIDTH(32);
  cmd_r |= (sfcfgp->cs & 3) << 4;
  cmd_r |= SF_CMD_WRITE;
  cmd_r |= SF_CMD_KEEPCS;

  cmd_d |= SF_CMD_PP;
  cmd_d |= offset & 0xffffff;

  sfrp->COMMAND_DATA0_REG = cmd_d;
  sfrp->ADDRESS_REG = (uint32_t)wrbuf;  
  sfrp->COMMAND = cmd_r;

  #ifdef RUN_IN_FLASH
  sf_wait(sfp);
  #else
  /* Waits for the operation completion or a timeout.*/
  sfp->thread = currp;
  chSchGoSleepS(CH_STATE_SUSPENDED);
  if ((sfp->timeout != TIME_INFINITE) && chVTIsArmedI(&vt))
    chVTResetI(&vt);
  sfrp->CONFIGURATION_1 = sfp->fmode;
  #endif  
  
  while(1) {
    status = sf_read_status(sfp);
    if((status & 1) == 0) {
      break;
    }
  }   
}




/**
 * @brief   Wakes up the waiting thread.
 *
 * @param[in] sfp      pointer to the @p SFDriver object
 * @param[in] msg       wakeup message
 *
 * @notapi
 */
#define wakeup_isr(sfp, msg) {                                             \
  chSysLockFromISR();                                                       \
  if ((sfp)->thread != NULL) {                                             \
    thread_t *tp = (sfp)->thread;                                            \
    (sfp)->thread = NULL;                                                  \
    tp->p_u.rdymsg = (msg);                                                 \
    chSchReadyI(tp);                                                        \
  }                                                                         \
  chSysUnlockFromISR();                                                     \
}

/**
 * @brief   Common IRQ handler.
 *
 * @param[in] sdp       communication channel associated to the UART
 */
static void serve_interrupt(SFDriver *sfp) {
  HS_SF_Type *sfrp = sfp->sf;

  sfrp->RAW_INTR_STATUS = sfrp->INTR_STATUS;
  sfrp->COMMAND = 0;

  wakeup_isr(sfp, MSG_OK);
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/
/**
 * @brief   SF interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SF_IRQHandler) {

  CH_IRQ_PROLOGUE();

  serve_interrupt(&SFD);

  CH_IRQ_EPILOGUE();
}
/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level SF driver initialization.
 *
 * @notapi
 */
void sf_lld_init(void) {

#if HS_SF_USE_SF0
  /* Driver initialization.*/
  sfObjectInit(&SFD);

  SFD.sf = HS_SF;
  SFD.timeout = TIME_INFINITE; 
  SFD.sfinfo  = &g_SFInfo[0];
  HS_SF->INTR_MASK = 0;

  sf_lld_start(&SFD);
#endif /* HS_SF_USE_SF0 */
}

/**
 * @brief   Configures and activates the SF peripheral.
 *
 * @param[in] sfp      pointer to the @p SFDriver object
 *
 * @notapi
 */
int sf_lld_start(SFDriver *sfp) 
{
  const SFConfig *sfcp;
  HS_SF_Type *sfrp;
  uint32_t config;

  sfcp = sfp->config;
  sfrp = sfp->sf;
  config = 0;

  if (sfcp == NULL) {
    return -1;
  }  

  if (sfp->state == SF_STOP) {
    uint32_t divider;

    #ifndef RUN_IN_FLASH
    cpmEnableSF();
    //cpmResetSF();   
    nvicEnableVector(IRQ_SFLASH, ANDES_PRIORITY_MASK(HS_SF_IRQ_PRIORITY));    
    #endif

    if(sfcp->speed == 64000000)
      cpm_set_clock(HS_SF_CLK, 64000000);  
    else
      cpm_set_clock(HS_SF_CLK, 96000000);  
    
    SFD.clock = cpm_get_clock(HS_SF_CLK);  
    sf_inner_select(SF_LOC_INNER);

    config |= (sfcp->data_width & 3) << 16;
    config |= (sfcp->clk_mode & 3) << 8;
    /* limitation: divider must be even; minimum divider is 2 */
    divider = DIV_ROUND_UP(sfp->clock, sfcp->speed) / 2 * 2;
    if (divider < 2)
      divider = 2;
    if (divider > 0xff)
      divider = 0xff;
    /* the wanted speed vs. the true master clock */
    sfp->mclk = sfp->clock / divider;
    config |= divider;

    if(sfcp->speed == 48000000)
      config |= (1u << 12);
    else if((sfcp->speed == 96000000) || (sfcp->speed == 64000000))
    {
      config |= (4u << 12);
      config |= (1u << 10);
    }

    #if 1
    sfrp->CONFIGURATION_0 = config;
    sfrp->CS_CONFIGURATION_0 &= 0xfffffffe;
    sfrp->CS_CONFIGURATION_0 |= sfcp->cs_pol & 0x01;

    sfp->fmode = sfrp->CONFIGURATION_1;
    #else
    if (sfcp->cs == SELECT_CS0) {
      sfrp->CONFIGURATION_0 = config;

      sfrp->CS_CONFIGURATION_0 &= 0xfffffffe;
      sfrp->CS_CONFIGURATION_0 |= sfcp->cs_pol & 0x01;
    }
    else if (sfcp->cs == SELECT_CS1) {
      sfrp->CONFIGURATION_1 = config;

      sfrp->CS_CONFIGURATION_1 &= 0xfffffffe;
      sfrp->CS_CONFIGURATION_1 |= sfcp->cs_pol & 0x01;
    }
    else if (sfcp->cs == SELECT_CS2) {
      sfrp->CONFIGURATION_2 = config;

      sfrp->CS_CONFIGURATION_1 &= 0xfffffffe;
      sfrp->CS_CONFIGURATION_1 |= sfcp->cs_pol & 0x01;
    }
    else if (sfcp->cs == SELECT_CS3) {
      sfrp->CONFIGURATION_3 = config;

      sfrp->CS_CONFIGURATION_1 &= 0xfffffffe;
      sfrp->CS_CONFIGURATION_1 |= sfcp->cs_pol & 0x01;
    }
    #endif

    #ifndef RUN_IN_FLASH
    sfrp->INTR_MASK = SF_INTR_UNMASK;
    #else
    sfrp->INTR_MASK = 0;
    #endif
    sfrp->RAW_INTR_STATUS = SF_INTR_CLRSTS;
  } 

  return 0;
}

/**
 * @brief   Deactivates the SF peripheral.
 *
 * @param[in] sfp      pointer to the @p SFDriver object
 *
 * @notapi
 */
int sf_lld_stop(SFDriver *sfp) {

  if (sfp->state != SF_STOP) {
    nvicDisableVector(IRQ_SFLASH);
    sfp->sf->INTR_MASK = 0;
  }

  return 0;
}

/**
 * @brief   probe spi flash.
 *
 * @param[in] sfp      pointer to the @p SFDriver object
 *
 * @notapi
 */
int sf_lld_probe(SFDriver *sfp) {
  uint32_t mid, i;
  
  mid = sf_read_mid(sfp);
  sfp->mid = mid;

  for(i = 1; i < g_SFCnt; i++) {
    if (mid == g_SFInfo[i].mid) {
      sfp->sfinfo = &g_SFInfo[i];
      return 0;
    }
  }

  if(((mid & 0xffffff) == 0xffffff) || ((mid & 0xffffff) == 0)) {
    sfp->sfinfo = NULL;
    return -1;
  }

  sfp->sfinfo = &g_SFInfo[0];
  return 1;
}

/**
 * @brief   erase spi flash.
 *
 * @param[in] sfp      pointer to the @p SFDriver object
 * @param[in] offset   flash base address erased
 * @param[in] len      erase length
 *
 * @notapi
 */
int sf_lld_erase(SFDriver *sfp, uint32_t offset, size_t len) {
  const SFlashInfo_t *sfinfop = sfp->sfinfo;
  uint32_t i, h_len, t_len, setor_num = 0;

  if(sfp->sfinfo == NULL) {
    return -1;
  }

  //cpmEnableSF();
  sf_wrStart(sfp);
  
  if(len == 0) {
    sf_chip_erase(sfp);

    cpmDisableSF();
    return 0;
  }

  if(len < sfinfop->sector_size) {
    setor_num = 1;
  }
  else {  
    h_len = offset & (sfinfop->sector_size - 1);

    if(h_len) {
      setor_num ++;
      h_len = sfinfop->sector_size - h_len;
    }
    else {
      h_len = 0;
    }
    
    t_len = (offset + len) & (sfinfop->sector_size - 1);

    if(t_len) {
      setor_num ++;
    }  

    setor_num += (len - h_len - t_len) / sfinfop->sector_size;
  }

  for(i = 0; i < setor_num; i++) {
    sf_sector_erase(sfp, offset);
    offset += sfinfop->sector_size;
  }

  //cpmDisableSF();
  return 0;
}

/**
 * @brief   read spi flash.
 *
 * @param[in] sfp      pointer to the @p SFDriver object
 * @param[in] offset   flash base address erased
 * @param[in] rdbuf    data buffer, must alignment with 4bytes
 * @param[in] len      read length
 *
 * @notapi
 */
int sf_lld_read(SFDriver *sfp, uint32_t offset, void *rdbuf, size_t len) {
  char *bufp;
  uint32_t i, cnt, transLen;

  if(sfp->sfinfo == NULL) {
    return -1;
  }

  //cpmEnableSF();
#if defined(__nds32__)
  nds32_dcache_flush();
#endif

  bufp = rdbuf;

  if(len % SF_READ_PAGE)
    cnt = len / SF_READ_PAGE + 1;
  else
    cnt = len / SF_READ_PAGE;

  for(i = 0; i < cnt; i++) {
    if(len > SF_READ_PAGE)
      transLen = SF_READ_PAGE;
    else
      transLen = len;

    sf_read(sfp, offset, bufp, transLen);

    offset += transLen;
    bufp += transLen;
    len -= transLen;
  }

  //cpmDisableSF();
  return 0;
}

/**
 * @brief   write spi flash.
 *
 * @param[in] sfp      pointer to the @p SFDriver object
 * @param[in] offset   flash base address erased
 * @param[in] wrbuf    data buffer, must alignment with 4bytes
 * @param[in] len      read length
 *
 * @notapi
 */
int sf_lld_write(SFDriver *sfp, uint32_t offset, void *wrbuf, size_t len) {  
  char *bufp;
  const SFlashInfo_t *sfinfop = sfp->sfinfo;
  uint32_t i, cnt, transLen;

  if(sfp->sfinfo == NULL) {
    return -1;
  }

  //cpmEnableSF();
#if defined(__nds32__)
  nds32_dcache_flush();
#endif

  bufp = wrbuf;

  if(offset % sfinfop->page_size) {
    transLen = sfinfop->page_size - (offset % sfinfop->page_size);
    
    transLen = len > transLen ? transLen : len;

    sf_page_write(sfp, offset, bufp, transLen);

    offset += transLen;
    bufp += transLen;
    len -= transLen;  
    
    if(len == 0)
      goto write_exit;
  }

  if(len % sfinfop->page_size)
    cnt = len / sfinfop->page_size + 1;
  else
    cnt = len / sfinfop->page_size;

  for(i = 1; i <= cnt; i++) {
    if(len > sfinfop->page_size)
      transLen = sfinfop->page_size;
    else
      transLen = len;

    sf_page_write(sfp, offset, bufp, transLen);

    //if((i%14) == 0)
    {
      #if HS_USE_LEDDISP  
      hs_led_frameDisp();
      #endif
    }

    offset += transLen;
    bufp += transLen;
    len -= transLen;
  }

write_exit:
  //cpmDisableSF();
  return 0;
}

void sf_lld_QuadEn(SFDriver *sfp, sfqe_t qe){

  uint32_t status;  

  sf_write_enable(sfp);

  status = sf_read_status(sfp);  
  if(QUAD_DISABLE == qe) {
    sf_write_status(sfp, (status&0xf9ff));
  }
  else {
    sf_write_status(sfp, (status & 0xf9ff) | 0x200);
  }
}

int sf_lld_deepPd(SFDriver *sfp) {

  if(sfp->sfinfo == NULL) {
    return -1;
  }

  //cpmEnableSF();
  
  sf_enter_deepPD(sfp);

  //cpmDisableSF();
  return 0;
}

int sf_lld_releasePd(SFDriver *sfp) {

  if(sfp->sfinfo == NULL) {
    return -1;
  }

  //cpmEnableSF();
  
  sf_release_deepPD(sfp);

  //cpmDisableSF();
  return 0;
}

#endif /* HAL_USE_SF */

/** @} */
