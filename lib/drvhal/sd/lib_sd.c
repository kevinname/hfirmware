/*
    drvhal - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/drvhal/lib_sd.c
 * @brief   sd file.
 * @details 
 *
 * @addtogroup  lib drvhal
 * @details 
 * @{
 */

#include "lib.h"

#if HAL_USE_SDC

#define __sd_setTiming(pstTiming)    cpm_set_sd_dev_clock(pstTiming)

static sd_dev_clk_t g_stSdTiming =
{
  SD_CLK_PHASE_0,
  -1,
  -1
};

static uint8_t g_u8SdScanCnt = 0;

int _sd_detectChk(void)
{
  ioportid_t pstPort;
  int s32Res = -1;
  const hs_drvhal_cfg_t *pstDrvCfg = hs_boardGetDrvCfg();
  uint32_t u32Pin = pstDrvCfg->u8SdDetPin;

  if(!chMtxTryLock(&SDCD0.mutex))
    return -1;
  
  if(u32Pin >= PB0)
  {
    pstPort = IOPORT1;
    u32Pin -= PB0;
  }
  else
  {
    pstPort = IOPORT0;
  }

  palSetPadMode(pstPort, u32Pin, 
                PAL_MODE_INPUT_PULLUP | PAL_MODE_ALTERNATE(PAD_FUNC_GPIO)|PAL_MODE_DRIVE_CAP(pstDrvCfg->u8SdDrvCap));
  
  s32Res = pstDrvCfg->u8SdDetLvl == palReadPad(pstPort, u32Pin) ? 1 : 0;

  
  palSetPadMode(IOPORT0, PA10, 
                  PAL_MODE_INPUT_PULLUP | PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB)|PAL_MODE_DRIVE_CAP(pstDrvCfg->u8SdDrvCap));

  chMtxUnlock(&SDCD0.mutex);
  return s32Res;
}

int hs_sd_timTaining(void)
{
  sd_dev_clk_t *pstTiming = &g_stSdTiming;
  
  for(; pstTiming->mclk_phase <= SD_CLK_PHASE_180; pstTiming->mclk_phase++)
  {
    for(; pstTiming->smp_half_cycles < 8; pstTiming->smp_half_cycles ++)
    {
      for(; pstTiming->drv_half_cycles < 8; pstTiming->drv_half_cycles ++)
      {
        __sd_setTiming(pstTiming);
        if(HAL_SUCCESS == sdcConnect(&SDCD0))
        {
          pstTiming->drv_half_cycles ++;
          return 0;
        }

        msleep(1);
      }

      pstTiming->drv_half_cycles = -1;
    }

    pstTiming->smp_half_cycles = -1;
  }

  return -1;
}

static uint32_t g_cnt = 0;
static uint8_t g_u8FirstPos = 0xff;
void hs_sd_scanCard(void)
{
  int s32Detect;
  
  if(g_stSdTiming.mclk_phase > SD_CLK_PHASE_180)
  {
    g_stSdTiming.mclk_phase = SD_CLK_PHASE_0;
    g_stSdTiming.drv_half_cycles = -1;
    g_stSdTiming.smp_half_cycles = -1;

    g_u8SdScanCnt += 1;
  }

  if(g_u8SdScanCnt >= 3)
    return ;

  s32Detect = _sd_detectChk();
  if(s32Detect < 0)
    return ;

  if(g_u8FirstPos == 0xff)
    g_u8FirstPos = s32Detect;
    
  if(s32Detect > 0)
  {
    g_cnt += 1;
    if(g_cnt < 10)
      return ;
    
    if(hs_fatfs_isMount(FATFS_MEMDEV_SD))
      return ;  
    
    if (0 != hs_sd_timTaining())
      return ;

    if(g_u8FirstPos == 0)
      hs_cfg_sysSendMsg(HS_CFG_MODULE_PERIP, HS_CFG_SYS_STATUS, HS_CFG_STATUS_MEMDEV_IN | FAST_EVENT_MASK, (void *)FATFS_MEMDEV_SD);
    else
      hs_cfg_sysSendMsg(HS_CFG_MODULE_PERIP, HS_CFG_SYS_STATUS, HS_CFG_STATUS_MEMDEV_IN, (void *)FATFS_MEMDEV_SD);
  }
  else
  {
    g_cnt = 0;
    if(!hs_fatfs_isMount(FATFS_MEMDEV_SD))
      return ;

    hs_cfg_sysSendMsg(HS_CFG_MODULE_PERIP, HS_CFG_SYS_STATUS, HS_CFG_STATUS_MEMDEV_OUT, (void *)FATFS_MEMDEV_SD);
    msleep(1);
    sdcDisconnect(&SDCD0);
    
    g_stSdTiming.mclk_phase = SD_CLK_PHASE_0;
    g_stSdTiming.drv_half_cycles = g_stSdTiming.smp_half_cycles = 0xff;
    g_u8SdScanCnt = 0;
  }
}

#endif

/** @} */
