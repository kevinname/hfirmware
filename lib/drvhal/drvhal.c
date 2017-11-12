/*
    drv-hal - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    drv/drvhal.c
 * @brief   drvhal file.
 * @details 
 *
 * @addtogroup  drvhal
 * @details 
 * @{
 */
#include "lib.h"

#if HS_PRODUCT_TYPE != HS_PRODUCT_HEADPHONE

typedef struct
{
  osTimerId pstTimer;
  
  uint32_t  u32Cnt;
  uint32_t  u32NextPoint;

  const hs_drvhal_cfg_t *pstDrvCfg;
}hs_drvhal_t;

static hs_drvhal_t g_stDrvhalInfo;

void hs_usb_scanPlug(void);
void hs_pmu_scanPowerPin(void);

void _drvhal_scan(void const *arg)
{
  hs_drvhal_t *pstDrvhalInfo = (hs_drvhal_t *)arg;
  
  #if HAL_USE_ADC
  hs_adc_scan();
  #endif

  hs_pmu_scanPowerPin();
  hs_pmu_cpuStatistic();

  #if HAL_USE_CODEC
  hs_codec_scan();
  #endif

  if(pstDrvhalInfo->pstDrvCfg->u8SdEnable)
    hs_sd_scanCard();

  if(pstDrvhalInfo->pstDrvCfg->u8UsbEnable)
  {
    hs_usb_scanPlug();

    #if HAL_USE_USB
    hs_usb_scanDisk();
    #endif  
  }

  pstDrvhalInfo->u32Cnt += 1;
  if((pstDrvhalInfo->u32Cnt % 20) == 0)
    hs_cfg_systemReq(HS_CFG_EVENT_NONE);

  if((pstDrvhalInfo->u32NextPoint != 0) 
    && (pstDrvhalInfo->u32NextPoint == pstDrvhalInfo->u32Cnt))
  {
    uint32_t u32PdType = pstDrvhalInfo->pstDrvCfg->u8PdType;
    
    hs_cfg_systemReq(HS_CFG_EVENT_BT_AUDIO_STOP);
    hs_cfg_systemReq(HS_CFG_EVENT_PRE_POWEROFF);
    hs_cfg_systemReqArg(HS_CFG_EVENT_PMU_POWEROFF, (void *)u32PdType);
  }
}
#endif

void _drvhal_autoPowerCharging(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;
  (void)parg;
  
  g_stDrvhalInfo.u32NextPoint = 0;
  hs_cfg_sysCancelListenMsg(HS_CFG_EVENT_BATTERY_CHARGING,  _drvhal_autoPowerCharging);  
}

void _drvhal_autoPowerOff(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;
  (void)parg;
  
  g_stDrvhalInfo.u32NextPoint = g_stDrvhalInfo.u32Cnt + g_stDrvhalInfo.pstDrvCfg->u16PdDelay * 10;
  hs_cfg_sysListenMsg(HS_CFG_EVENT_BATTERY_CHARGING,  _drvhal_autoPowerCharging);  
}

int hs_drvhal_init(void)
{
  #if HS_PRODUCT_TYPE != HS_PRODUCT_HEADPHONE
  osTimerDef_t stTmDef; 

  #if HAL_USE_ADC
  hs_adc_init();
  #endif

  hs_audio_init();
  if(!g_stDrvhalInfo.pstTimer)
  {
    stTmDef.ptimer = _drvhal_scan;
    g_stDrvhalInfo.pstTimer = oshalTimerCreate(&stTmDef, osTimerPeriodic, (void *)&g_stDrvhalInfo);
    if(!g_stDrvhalInfo.pstTimer)
      return -1;

    oshalTimerStart(g_stDrvhalInfo.pstTimer, 100);
  }
  
  #endif

  g_stDrvhalInfo.u32Cnt = 0;
  g_stDrvhalInfo.u32NextPoint = 0;
  g_stDrvhalInfo.pstDrvCfg = hs_boardGetDrvCfg();
  hs_cfg_sysListenMsg(HS_CFG_EVENT_BATTERY_NEAREMPTY,  _drvhal_autoPowerOff);  
  return 0;
}

void hs_drvhal_uninit(void)
{
  #if HS_PRODUCT_TYPE != HS_PRODUCT_HEADPHONE
  hs_sd_discon();
  hs_adc_close();

  oshalTimerDelete(g_stDrvhalInfo.pstTimer);
  g_stDrvhalInfo.pstTimer = NULL;

  #if HAL_USE_FATFS
  hs_fatfs_unmount(HS_CFG_EVENT_MEMDEV_OUT, (void *)FATFS_MEMDEV_SD);
  hs_fatfs_unmount(HS_CFG_EVENT_MEMDEV_OUT, (void *)FATFS_MEMDEV_UDISK);
  #endif

  hs_cfg_flush(FLUSH_TYPE_ONEBLOCK);
  #endif
}

void boardInit(void) 
{
  cpm_reset_system();
  cpm_init_clock();
  
  halInit();
  oshalKernelInitialize();  

  hs_cfg_Init();
  hs_fatfs_init();
  hs_cfg_systemReq(HS_CFG_EVENT_PMU_POWERON);
}

