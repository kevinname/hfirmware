/*
    bootloader - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    config/cfg_sys.c
 * @brief   config system manage file.
 * @details 
 *
 * @addtogroup  config
 * @details 
 * @{
 */

#include <string.h>
#include "lib.h"

/*===========================================================================*/
/* local definitions.                                                        */
/*===========================================================================*/
typedef struct
{
  hs_cfg_led_action_t   stLedAction;
  uint8_t               u8LedIndex;
  uint8_t               u8Cnt;  

  osTimerId             pstLedTimer;
}hs_cfg_Led_ioinfo_t;

static uint32_t g_u32LedEnableMap;
static uint32_t g_u32LedOverMap;
/*===========================================================================*/
/* local functions.                                                          */
/*===========================================================================*/
static void _cfg_sysLedGpioOut(uint8_t u8LedIndex, uint8_t val)
{
  ioportid_t pGpio;

  if(u8LedIndex < PAL_IOPORTS_WIDTH)
  {
    pGpio = IOPORT0;
  }
  else
  {
    u8LedIndex = u8LedIndex - PAL_IOPORTS_WIDTH;
    pGpio = IOPORT1;
  }

  palWritePad(pGpio, u8LedIndex, val);
}

static void _cfg_sysLedSwAction(void *arg)
{
  hs_cfg_Led_ioinfo_t  *pstLedIoInfo = (hs_cfg_Led_ioinfo_t *)arg;
  uint16_t u16Ms;
  uint32_t u32Mask; 
  uint8_t u8LedIdx = pstLedIoInfo->u8LedIndex;

  if(pstLedIoInfo == NULL)
    return ;

  pstLedIoInfo->u8Cnt ++; 
  if(pstLedIoInfo->u8Cnt % 2)
  {
    _cfg_sysLedGpioOut(u8LedIdx, ~(pstLedIoInfo->stLedAction.u8LedOnLevel));
    u16Ms = pstLedIoInfo->stLedAction.u16PeroidTime - pstLedIoInfo->stLedAction.u16DutyTime;
  }
  else
  {     
    u32Mask = 1u << u8LedIdx;

    if((g_u32LedEnableMap & u32Mask) == 0)
    {   
      g_u32LedOverMap |= u32Mask;        
      goto __Led_timer;
    }
    
    if(pstLedIoInfo->stLedAction.u8RepeatCnt != 0)
    {
      if(pstLedIoInfo->stLedAction.u8RepeatCnt <= (pstLedIoInfo->u8Cnt / 2))
      {
        g_u32LedOverMap |= u32Mask;
        goto __Led_timer;
      }
    }

    _cfg_sysLedGpioOut(u8LedIdx, pstLedIoInfo->stLedAction.u8LedOnLevel);
    u16Ms = pstLedIoInfo->stLedAction.u16DutyTime;
  }

  oshalTimerStart(pstLedIoInfo->pstLedTimer, u16Ms);
  return ;
  
__Led_timer:
  oshalTimerDelete(pstLedIoInfo->pstLedTimer);
  hs_free(pstLedIoInfo);
}

static void _cfg_sysLedAction(uint8_t u8LedIdx, hs_cfg_led_action_t *pstLedAction)
{
  hs_cfg_Led_ioinfo_t  *pstLedIoInfo; 
  osTimerDef_t stTmDef;
  uint32_t u32Mask = 1u << u8LedIdx;

  if((g_u32LedOverMap & u32Mask) == 0)
  {
    g_u32LedEnableMap &= ~u32Mask;   

    while((g_u32LedOverMap & u32Mask) == 0)
    {
      msleep(10);
    }
  }
  
  if(pstLedAction->u16DutyTime == 0)
  {
    _cfg_sysLedGpioOut(u8LedIdx, pstLedAction->u8LedOnLevel);
    return ;
  }

  g_u32LedOverMap &= ~u32Mask;

  pstLedIoInfo = (hs_cfg_Led_ioinfo_t *)hs_malloc(sizeof(hs_cfg_Led_ioinfo_t), __MT_Z_GENERAL);
  __cfg_chkPtrNoRet(pstLedIoInfo);

  pstLedIoInfo->u8LedIndex = u8LedIdx;
  pstLedIoInfo->u8Cnt = 0;
  memcpy(&pstLedIoInfo->stLedAction, pstLedAction, sizeof(hs_cfg_led_action_t));

  stTmDef.ptimer = (os_ptimer)_cfg_sysLedSwAction;
  pstLedIoInfo->pstLedTimer = oshalTimerCreate(&stTmDef, osTimerOnce, (void *)pstLedIoInfo);
  if(NULL == pstLedIoInfo->pstLedTimer)
  {
    hs_free(pstLedIoInfo);
    return ;
  }

  g_u32LedEnableMap |= u32Mask;
  _cfg_sysLedGpioOut(u8LedIdx, pstLedAction->u8LedOnLevel);
  oshalTimerStart(pstLedIoInfo->pstLedTimer, pstLedAction->u16DutyTime);
}


/*===========================================================================*/
/* main and output functions.                                                */
/*===========================================================================*/
void hs_cfg_sysLedInit(void)
{
  g_u32LedEnableMap = 0;
  g_u32LedOverMap = 0xffffffff;
}


hs_cfg_res_t hs_cfg_sysLedSendEvent(hs_cfg_mess_type_t m_type, uint16_t message)
{
  struct hs_cfg_event_led stEventLed;
  hs_cfg_sys_info_t stSysInfo;
  hs_cfg_led_action_t stLedAction;
  hs_cfg_res_t enRes;
  uint16_t i, u16Offset;

  if(message == 0)
  {
    return HS_CFG_OK;
  }
  
  enRes = hs_cfg_getDataByIndex(HS_CFG_SYS_INFO, (uint8_t *)&stSysInfo, sizeof(hs_cfg_sys_info_t));
  if(enRes != HS_CFG_OK)
  {
    return enRes;
  }

  for(i=0; i<stSysInfo.u8EvtBindLedCnt; i++)
  {
    u16Offset = i * sizeof(struct hs_cfg_event_led);    
    enRes = hs_cfg_getPartDataByIndex(HS_CFG_SYS_EVENT_BIND_LED, (uint8_t *)&stEventLed, 
                                      sizeof(struct hs_cfg_event_led), u16Offset);
    if(enRes != HS_CFG_OK)
    {
      continue;
    }

    if((stEventLed.u16Message == message) && (stEventLed.u8MessType == (uint8_t)m_type))
    {
      if(stEventLed.u8LedActionIdx >= stSysInfo.u8LedActionCnt)
      {
        continue;
      }

      u16Offset = stEventLed.u8LedActionIdx * sizeof(hs_cfg_led_action_t);    
      enRes = hs_cfg_getPartDataByIndex(HS_CFG_SYS_LED_ACTION, (uint8_t *)&stLedAction, 
                                                sizeof(hs_cfg_led_action_t), u16Offset);
      if(enRes != HS_CFG_OK)
      {
        continue;
      }

      if(stEventLed.u8LedIndex >= 32)
      {
        continue;
      }

      _cfg_sysLedAction(stEventLed.u8LedIndex, &stLedAction);
    }
  }  

  return HS_CFG_OK;
}



/** @} */
