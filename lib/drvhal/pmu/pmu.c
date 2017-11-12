/*
    drvhal - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/drvhal/pmu/pmu.c
 * @brief   pad config file.
 * @details 
 *
 * @addtogroup  drvhal
 * @details 
 * @{
 */

#include "lib.h"
#include "context.h"

typedef struct
{
  uint8_t  u8Cnt;
  uint8_t  u8ScanEnable;
}hs_pmupara_t;

typedef struct
{
  uint32_t          u32Cnt;
  uint32_t          u32IdleCnt;
  hs_pmu_status_t   eStatus;
  uint32_t          u32DPDisable;

  uint8_t           u8FlushDis;
  uint8_t           u8InPd;
}hs_pmu_manage_t;

static hs_pmupara_t g_stPmuPara;
static hs_pmu_manage_t g_stPmuManage;

void hs_cfg_sysBnPressed(ioportid_t port, uint8_t pad);
void cpu_enter_sleepNoWakeup(void);

void _pmu_powerByButton(hs_pmupara_t *pstPmuPara, uint8_t u8Status)
{
  if(!pstPmuPara->u8ScanEnable)
  {
    if(u8Status)
      pstPmuPara->u8Cnt += 1;
    else
      pstPmuPara->u8Cnt  = 0;
      
    if(pstPmuPara->u8Cnt > 10)
    {
      pstPmuPara->u8Cnt = 0;
      pstPmuPara->u8ScanEnable = 1;
    }
  }
  else
  {    
    if(u8Status)
    {
      if(pstPmuPara->u8Cnt > 30)
      {
        pstPmuPara->u8ScanEnable = 0;
        hs_cfg_sysSendMessage(HS_CFG_MODULE_SYS, HS_CFG_SYS_STATUS, HS_CFG_STATUS_POWER_OFF);
      }
      
      pstPmuPara->u8Cnt = 0;
      return ;
    }
    else
    {
      pstPmuPara->u8Cnt += 1;    

      if(pstPmuPara->u8Cnt == 30)
        hs_cfg_systemReq(HS_CFG_EVENT_PRE_POWEROFF);
    }
  }
}

void hs_pmu_chipPrePowerOff(uint16_t msg, void *parg)
{
  (void)msg;
  (void)parg;

  g_stPmuManage.u8InPd = 1;
  g_stPmuManage.eStatus = PMU_STATUS_SLEEPING;

  #if HS_USE_LEDDISP  
  hs_led_frameDisp();
  #endif
  
  hs_cfg_flush(FLUSH_TYPE_ALL);
  hs_cfg_playerReq(HS_CFG_EVENT_PLAYER_DESTROY);
  hs_cfg_systemReq(HS_CFG_EVENT_BT_AUDIO_STOP);
}

void hs_pmu_chipPowerOff(uint16_t msg, void *parg)
{
  (void)msg;
  uint32_t u32SwPower = (uint32_t)parg;

  if(u32SwPower == 1)
  {
    cpu_enter_sleepNoWakeup();
  }
  else
  {
    cpm_switch_to_xtal();
    pmu_chip_poweroff();
  }
}

void hs_pmu_chipDeepSleep(uint16_t msg, void *parg)
{
  (void)msg;

  g_stPmuManage.eStatus = PMU_STATUS_SLEEPING;

  #if HS_USE_LEDDISP  
  hs_led_frameDisp();
  #endif

  msleep(3000);  
  cpu_enter_sleep((int)parg);
}

void hs_pmu_scanPowerPin(void)
{
  hs_pmupara_t *pstPmuPara = &g_stPmuPara;
  uint8_t u8Status = pmu_get_powerPinStatus();
  const hs_drvhal_cfg_t *pstDrvCfg = hs_boardGetDrvCfg();

  if(HS_POWER_MODE_BUTTON == pstDrvCfg->u8PowerOnMode)
  {
    if((u8Status == 0) && pstPmuPara->u8ScanEnable)
      hs_cfg_sysBnPressed(0, 0);
    
    _pmu_powerByButton(pstPmuPara, u8Status);
  }
  else
  {
    if(!u8Status)
      pstPmuPara->u8Cnt = 0;
    else
      pstPmuPara->u8Cnt += 1;

    if(pstPmuPara->u8Cnt == 10)
    {
      hs_cfg_playerReq(HS_CFG_EVENT_PLAYER_DESTROY);
      hs_cfg_systemReq(HS_CFG_EVENT_BT_AUDIO_STOP);
      
      hs_cfg_systemReq(HS_CFG_EVENT_PRE_POWEROFF);
      hs_cfg_systemReq(HS_CFG_EVENT_PMU_POWEROFF);
    }
  }
}

void hs_pmu_cpuStatistic(void)
{
  hs_pmu_manage_t *pstPmuM = &g_stPmuManage;
  thread_t *pstTp;
  float fTotal = 0, fIdle = 0, fTimer = 0;
  hs_pmu_status_t eStatus = pstPmuM->eStatus;
  uint32_t u32BTOnline = 0;

  pstPmuM->u32Cnt += 1;
  if((pstPmuM->u32Cnt % 10) != 0)
    return ;

  pstTp = chRegFirstThread();
  do 
  {
    fTotal += pstTp->p_time;
    if(0 == strcmp(pstTp->p_name, "idle"))
      fIdle += pstTp->p_time;

    if(0 == strcmp(pstTp->p_name, "TimerService"))
      fTimer += pstTp->p_time;

    if(0 == strcmp(pstTp->p_name, "bthc"))
      u32BTOnline = 1;

    pstTp->p_time = 0;
    pstTp = chRegNextThread(pstTp);
  } while (pstTp != NULL);

  fIdle = fIdle / fTotal * 100;

  if((fIdle > 99.0) && (pstPmuM->u8FlushDis == 0))
    hs_cfg_flush(FLUSH_TYPE_ONEBLOCK);

  if(g_stPmuManage.u8InPd == 1)
    return ;

  if((fIdle > 99.9) && (u32BTOnline == 0) 
          && (pstPmuM->u32DPDisable == 0))
  {
    pstPmuM->u32IdleCnt += 1;
  }
  else
  {
    pstPmuM->u32IdleCnt = 0;
    eStatus = PMU_STATUS_WORKING;
  }
  
  if(pstPmuM->u32IdleCnt >= 300)
    eStatus = PMU_STATUS_SLEEPING;

  if(eStatus != pstPmuM->eStatus)
  {
    pstPmuM->eStatus = eStatus;
    hs_printf("System status:%d\r\n", eStatus);
  }
  
  //if(pstPmuM->eStatus == PMU_STATUS_SLEEPING)
  //  cpu_enter_sleep(0); 
}

__ONCHIP_CODE__ bool hs_pmu_isSleeping(void)
{
  return (g_stPmuManage.eStatus == PMU_STATUS_SLEEPING);
}

void hs_pmu_enSleep(uint8_t enble)
{
  g_stPmuManage.u32DPDisable = enble == 1 ? 0 : 1;
}

void hs_pmu_flushDisable(uint8_t u8Dis)
{
  g_stPmuManage.u8FlushDis = u8Dis;
}


/** @} */
