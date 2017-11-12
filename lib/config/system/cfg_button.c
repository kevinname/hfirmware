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

#include "lib.h"
#include "string.h"


/*===========================================================================*/
/* local definitions.                                                        */
/*===========================================================================*/

#define CFG_BNACTION_QUEUE_SIZE         12

typedef struct
{
  uint16_t u16TimerCnt;
  uint16_t u16InvalidCnt;
  uint16_t u16LastRepeatPtr;
  uint8_t  u8MultiClickCnt;
  uint8_t  u8ValidCnt;

  uint32_t u32PressMap;
  uint8_t  u8PressNum; 
  uint8_t  u8Sample;
  hs_cfg_bn_type_t bnType; 

  uint8_t  u8Pd;
  uint8_t  u8VLPressSend;
  uint8_t  u8LPressSend;
}hs_cfg_bninfo_t;

typedef struct
{
  uint16_t bnType; 
  uint32_t u32PressMap;
}hs_bnevent_t;

static hs_cfg_bn_attr_t g_stBnAttr = 
{
  /* no any key is pre-defined */
  0x00000000, 60, 800, 1500, 5000, 800, 0, 10, 3
};

static hs_cfg_bninfo_t  g_stBnInfo;
static osMessageQId     g_pstBnMess;
static osTimerId        g_pstBnTimer;
static hs_bnevent_t     g_stQueue[CFG_BNACTION_QUEUE_SIZE];

void hs_cfg_sysBnPressed(ioportid_t port, uint8_t pad);
void hs_cfg_setFastEvent(uint32_t u32Cnt);

/*===========================================================================*/
/* local functions.                                                          */
/*===========================================================================*/
static uint32_t _cfg_sysBnGetGpioVal(uint32_t u32BitMap)
{
  uint32_t u32Val;
  
  u32Val = palReadPort(IOPORT1) & 0xffff;
  u32Val = (u32Val << 16) | (palReadPort(IOPORT0) & 0xffff);

  return (u32Val & u32BitMap);
}

static msg_t _cfg_sysBnSendEventThread(void *arg)
{
  hs_cfg_sys_info_t stSysInfo;
  hs_cfg_bn_event_t stBnEvent;
  hs_bnevent_t *pstBnEvent;
  uint16_t i;
  uint32_t u32BitMap;
  hs_cfg_bn_type_t bnType;
  osMessageQId pstMessId = (osMessageQId)arg;
  osEvent res;

  chRegSetThreadName("ButtonService");
  while(1)
  {
    res = oshalMessageGet(pstMessId, -1);
    if(res.status != osEventMessage)
      continue;

    pstBnEvent = (hs_bnevent_t *)res.value.v;
    if(!pstBnEvent)
      continue;

    bnType    = pstBnEvent->bnType;
    u32BitMap = pstBnEvent->u32PressMap;
    hs_free(pstBnEvent);

    hs_printf("type:%d, map:0x%x\r\n", bnType, u32BitMap);
    
    if(HS_CFG_OK != hs_cfg_getDataByIndex(HS_CFG_SYS_INFO, 
                                          (uint8_t *)&stSysInfo, 
                                          sizeof(hs_cfg_sys_info_t)))
    {      
      continue;
    }

    for(i=0; i<stSysInfo.u8BnBindEvtCnt; i++)
    {
      if(HS_CFG_OK != hs_cfg_getPartDataByIndex(HS_CFG_SYS_BUTTON_BIND_EVENT, 
                                               (uint8_t *)&stBnEvent, 
                                               sizeof(hs_cfg_bn_event_t), 
                                               i * sizeof(hs_cfg_bn_event_t)))
      {
        continue;
      }

      if((stBnEvent.u32BnMask == u32BitMap) && (stBnEvent.u8BnType == (uint8_t)bnType))
      {
        hs_cfg_sysSendMsg((hs_cfg_mod_t)stBnEvent.u8EventMod, HS_CFG_SYS_EVENT, 
                          (hs_cfg_event_type_t)(stBnEvent.u16Event | FAST_EVENT_MASK), (void *)stBnEvent.u32Arg);
      }
    }
  }

  return 0;
}


static void _cfg_sysBnSendEvent(uint32_t u32BnMap, hs_cfg_bn_type_t bnType)
{
  hs_bnevent_t *pstBnEvent;  
  
  pstBnEvent = hs_malloc(sizeof(hs_bnevent_t), __MT_Z_GENERAL);
  if(!pstBnEvent)
    return ;

  pstBnEvent->u32PressMap = u32BnMap;
  pstBnEvent->bnType = bnType;
  oshalMessagePut(g_pstBnMess, (uint32_t)pstBnEvent, 0);

  if(BN_TYPE_PRESS_RELEASE == bnType)
    hs_cfg_setFastEvent(0);
}

static uint32_t _cfg_sysBnGetPressed(uint32_t *pu32PressMap)
{
  uint32_t i, u32GpioVal, u32BitMap;
  uint32_t u32Cnt = 0, u32AdcKey, u32PressMap = 0;

  if((g_stBnAttr.u32BitMap & 0x80000000) == 0)
  {
    if(g_stBnAttr.u32BitMap & 1)
    {
      if( pmu_get_powerPinStatus() == g_stBnAttr.u8PressLevel)
      {
        u32PressMap = 1;
        u32Cnt += 1;
      }
    }

    u32BitMap = g_stBnAttr.u32BitMap; 
    u32GpioVal = _cfg_sysBnGetGpioVal(u32BitMap);

    for(i=1; i<PIN_NUM; i++)
    {
      u32BitMap >>= 1;
      u32GpioVal >>= 1;
      
      if((u32BitMap & 1) && ((u32GpioVal & 1) == g_stBnAttr.u8PressLevel))
      {
        u32PressMap |= 1 << i;
        u32Cnt ++;
      }
    }
  }

  u32AdcKey = hs_adc_getKeyMap() & g_stBnAttr.u32BitMap;
  if(u32AdcKey)
  {
    u32PressMap |= u32AdcKey;
    u32Cnt ++;
  }

  *pu32PressMap = u32PressMap;
  return u32Cnt;
}

static void _cfg_sysBnTimer(void const *arg)
{
  hs_cfg_bninfo_t *pstBnInfo = (hs_cfg_bninfo_t *)arg;
  uint16_t u16PressNum;
  uint32_t u32PressMap;
  uint16_t u16Time;
    
  pstBnInfo->u16TimerCnt ++;

  u16PressNum = _cfg_sysBnGetPressed(&u32PressMap);  
  if(u16PressNum > 0)
  {
    pstBnInfo->u8ValidCnt ++; 
    pstBnInfo->u16InvalidCnt = 0;

    if(pstBnInfo->u8MultiClickCnt == 1)
    {
      pstBnInfo->u8MultiClickCnt |= 0x80;
    }

    if((pstBnInfo->u8Sample) && !(g_stBnAttr.u32BitMap & 0xff000000))
    {
      pstBnInfo->u32PressMap |= u32PressMap;
      if(u16PressNum > pstBnInfo->u8PressNum)
      {
        pstBnInfo->u8PressNum = u16PressNum;
      }
    }

    if((pstBnInfo->u8MultiClickCnt == 0) && (pstBnInfo->u8Sample))
    {
      /* identify repeat event */
      u16Time = pstBnInfo->u16TimerCnt * g_stBnAttr.u8DebounceTime;
      if((u16Time - pstBnInfo->u16LastRepeatPtr) > g_stBnAttr.u16RepeatTime)
      {
        pstBnInfo->u16LastRepeatPtr = u16Time;
        _cfg_sysBnSendEvent(pstBnInfo->u32PressMap, BN_TYPE_REPEAT);
      }

      if ((pstBnInfo->u8VLPressSend == 0) && (u16Time >= g_stBnAttr.u16VeryLongPressTime))
      {
        pstBnInfo->u8VLPressSend = 1;
        _cfg_sysBnSendEvent(pstBnInfo->u32PressMap, BN_TYPE_VERYLONG_PRESS);        
      }

      if ((pstBnInfo->u8LPressSend == 0) && (u16Time >= g_stBnAttr.u16LongPressTime))
      {
        pstBnInfo->u8LPressSend = 1;
        _cfg_sysBnSendEvent(pstBnInfo->u32PressMap, BN_TYPE_LONG_PRESS);        
      }
    }

    if ((pstBnInfo->u16TimerCnt > g_stBnAttr.u8DebounceNumber)
       && (pstBnInfo->u8PressNum == 0))
    {
      if(pstBnInfo->u8ValidCnt > (pstBnInfo->u16TimerCnt * 7 / 10))
      {
        pstBnInfo->u8Sample = 1;
        pstBnInfo->u8PressNum = u16PressNum;

        pstBnInfo->u32PressMap = u32PressMap;
        _cfg_sysBnSendEvent(pstBnInfo->u32PressMap, BN_TYPE_PRESS_DOWN);
      }
      else
      {
        /* a invalid press action */
        memset(pstBnInfo, 0, sizeof(hs_cfg_bninfo_t)); 
        goto __bn_timer_over;
      }
    } 

  }
  else
  {
    pstBnInfo->u16InvalidCnt ++;

    if((pstBnInfo->u8PressNum > 0) && (pstBnInfo->u16InvalidCnt >= g_stBnAttr.u8DebounceNumber))
    {
      /**
       * a press action over 
       */
      if(pstBnInfo->u8PressNum != 1)
      {
        /* combination key pressed */
        _cfg_sysBnSendEvent(pstBnInfo->u32PressMap, BN_TYPE_PRESS_RELEASE);
        _cfg_sysBnSendEvent(pstBnInfo->u32PressMap, BN_TYPE_COMBINATION_KEY);
        goto __bn_timer_over;
      }      
      
      u16Time = pstBnInfo->u16TimerCnt * g_stBnAttr.u8DebounceTime;
      if(u16Time >= g_stBnAttr.u16VeryLongPressTime)
      {
        _cfg_sysBnSendEvent(pstBnInfo->u32PressMap, BN_TYPE_PRESS_RELEASE);
        goto __bn_timer_over;
      }

      if(u16Time >= g_stBnAttr.u16LongPressTime)
      {
        _cfg_sysBnSendEvent(pstBnInfo->u32PressMap, BN_TYPE_PRESS_RELEASE);
        goto __bn_timer_over;
      }

      if((pstBnInfo->u8MultiClickCnt & 0x81) == 0x81)
      {
        _cfg_sysBnSendEvent(pstBnInfo->u32PressMap, BN_TYPE_PRESS_RELEASE);
        _cfg_sysBnSendEvent(pstBnInfo->u32PressMap, BN_TYPE_DOUBLE_CLICK);        
        goto __bn_timer_over;
      }

      u16Time = pstBnInfo->u16InvalidCnt * g_stBnAttr.u8DebounceTime;
      if(u16Time > g_stBnAttr.u16SingleRepeatInterval)
      {
        _cfg_sysBnSendEvent(pstBnInfo->u32PressMap, BN_TYPE_PRESS_RELEASE);
        _cfg_sysBnSendEvent(pstBnInfo->u32PressMap, BN_TYPE_SINGLE_CLICK);        
        goto __bn_timer_over;
      }

      if(u16Time > g_stBnAttr.u16DoubleClickInterval)
      {
        pstBnInfo->u8MultiClickCnt = 1;
      }      
    }    
    
    if ((pstBnInfo->u16TimerCnt > g_stBnAttr.u8DebounceNumber)
       && (pstBnInfo->u8PressNum == 0))
    {
      if(pstBnInfo->u8ValidCnt < (pstBnInfo->u16TimerCnt * 8 / 10))      
      {
        /* a invalid press action */
        memset(pstBnInfo, 0, sizeof(hs_cfg_bninfo_t)); 
        goto __bn_timer_over;
      }
    }
  }

  return ;

__bn_timer_over:
  pstBnInfo->u8Pd = 0;
  oshalTimerStop(g_pstBnTimer);
  return ;
}

/**
 * set gpio input and enble interrupt
 */
static void _cfg_sysBnGpioInit(uint32_t u32BitMap, uint8_t level)
{
  uint32_t i;

  if((u32BitMap & 0x80000000) != 0)
    return ;

  for(i=0; i<32; i++)
  {	  
    if(u32BitMap & 1)
      palRegEvent(i, (hs_gpio_inter_t)level, (ioevent_t)hs_cfg_sysBnPressed);

    u32BitMap >>= 1;
  }
}

/*===========================================================================*/
/* main and output functions.                                                */
/*===========================================================================*/
void hs_cfg_sysBnInit(void)
{
  osMessageQDef_t mess;
  osThreadDef_t thdDef;
  osTimerDef_t stTmDef;
  
  hs_cfg_getDataByIndex(HS_CFG_SYS_BUTTON_IDENTIFY, (uint8_t *)&g_stBnAttr, sizeof(hs_cfg_bn_attr_t));
  if(g_stBnAttr.u8DebounceTime == 0)
    g_stBnAttr.u8DebounceTime = 10; 
  
  mess.item_sz = sizeof(int);
  mess.queue_sz = CFG_BNACTION_QUEUE_SIZE * sizeof(hs_bnevent_t);
  mess.items = (void *)g_stQueue;
  __cfg_chkPtrNoRet(mess.items);
  g_pstBnMess = oshalMessageCreate(&mess, NULL);
  __cfg_chkPtrNoRet(g_pstBnMess);

  
  thdDef.pthread   = (os_pthread)_cfg_sysBnSendEventThread;
  thdDef.stacksize = 512;
  thdDef.tpriority = osPriorityRealtime;
  oshalThreadCreate(&thdDef, g_pstBnMess);

  stTmDef.ptimer = (os_ptimer)_cfg_sysBnTimer;
  g_pstBnTimer = oshalTimerCreate(&stTmDef, osTimerPeriodic, (void *)&g_stBnInfo);
  __cfg_chkPtrNoRet(g_pstBnTimer);
  
  _cfg_sysBnGpioInit(g_stBnAttr.u32BitMap, g_stBnAttr.u8PressLevel);
}

void hs_cfg_sysBnPressed(ioportid_t port, uint8_t pad)
{
  (void)port;
  (void)pad;
  if(g_stBnInfo.u8Pd)
    return ;

  oshalTimerStop(g_pstBnTimer);
  
  memset(&g_stBnInfo, 0, sizeof(hs_cfg_bninfo_t));  
  g_stBnInfo.u16LastRepeatPtr = g_stBnAttr.u16LongPressTime;
  g_stBnInfo.u8Pd = 1;

  oshalTimerStart(g_pstBnTimer, g_stBnAttr.u8DebounceTime);
}




/** @} */
