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

#include "../config.h"
#include "../config.c"

typedef struct hsCfg_sys_reg_s  hs_cfg_sys_reg_t;
struct hsCfg_sys_reg_s
{
  hs_cfg_mess_oper_t  stMessOper;
  hs_cfg_sys_reg_t    *pstNextOper;
};

typedef struct
{
  hs_cfg_mod_t        eMod;
  hs_cfg_mess_type_t  eType;
  uint16_t            u16Msg;
  void               *parg;
}hs_cfg_sysmsg_t;

static osMessageQId g_pstMess;
static hs_cfg_sys_reg_t *g_pstEventOper;
static osMutexId g_pstMsgMutex;
static uint32_t g_u32FastEvent;

extern hs_cfg_res_t hs_cfg_sysLedSendEvent(hs_cfg_mess_type_t m_type, uint16_t message);
extern void hs_cfg_sysBnInit(void);
extern void hs_cfg_sysLedInit(void);

static hs_cfg_sys_reg_t *_cfg_sysChkMsg(uint16_t message, hs_evtFunction_t fnOper)
{
  hs_cfg_sys_reg_t *pstEvent = g_pstEventOper;

  while(pstEvent != NULL)
  {
    if((pstEvent->stMessOper.u16MessType == message) 
      && (pstEvent->stMessOper.pfnOper == fnOper))
    {      
      return pstEvent;
    }
    else
    {    
      pstEvent = pstEvent->pstNextOper;
    }
  }

  return NULL;
}


static hs_cfg_sys_reg_t * _cfg_sysNewEvent(uint16_t message, hs_evtFunction_t fnOper)
{
  hs_cfg_sys_reg_t * pstTemp;

  pstTemp = (hs_cfg_sys_reg_t *)hs_malloc(sizeof(hs_cfg_sys_reg_t), __MT_Z_GENERAL);
  if(pstTemp == NULL)
  {
    return NULL;
  }

  pstTemp->stMessOper.u16MessType = message;
  pstTemp->stMessOper.pfnOper = fnOper;
  pstTemp->pstNextOper = NULL;

  return pstTemp;
}

static void _cfg_sysDoEvent(uint16_t message, void *parg)
{
  hs_cfg_sys_reg_t * pstTemp = g_pstEventOper;

  oshalMutexWait(g_pstMsgMutex, -1);
  while(pstTemp != NULL)
  {
    if((pstTemp->stMessOper.u16MessType == message)
      && (pstTemp->stMessOper.pfnOper != NULL))
    {
      oshalMutexRelease(g_pstMsgMutex);
      pstTemp->stMessOper.pfnOper(message, parg);
      oshalMutexWait(g_pstMsgMutex, -1);
    }

    pstTemp = pstTemp->pstNextOper;
  }
  oshalMutexRelease(g_pstMsgMutex);
}

static hs_cfg_res_t _cfg_sysDoMessage(hs_cfg_sysmsg_t *pstMsg)
{
  hs_cfg_sysmsg_t stMsg;
  hs_cfg_mess_oper_t *pstMessage = NULL;

  if(!pstMsg)
    return HS_CFG_ERR_NO_DATA;
  
  memcpy(&stMsg, pstMsg, sizeof(stMsg));
  hs_free(pstMsg);

  if((stMsg.u16Msg & FAST_EVENT_MASK) && (g_u32FastEvent > 0))
  {
    stMsg.u16Msg &= ~FAST_EVENT_MASK;
    g_u32FastEvent -= 1;
  }

  if ((stMsg.u16Msg == HS_CFG_EVENT_NONE) || (stMsg.eMod >= HS_CFG_MODULE_NUM))
    return HS_CFG_ERR_NO_DATA;

  if(stMsg.eType == HS_CFG_SYS_EVENT)
  {
    pstMessage = (hs_cfg_mess_oper_t *)g_pstSysEventOper[stMsg.eMod];
  }

  if(stMsg.eType == HS_CFG_SYS_STATUS)
  {
    pstMessage = (hs_cfg_mess_oper_t *)g_pstSysStatusOper[stMsg.eMod];
  }

  while(pstMessage)
  {    
    if(pstMessage->u16MessType == 0)
    {
      pstMessage = NULL;
      break;
    }
    
    if(pstMessage->u16MessType == stMsg.u16Msg)
    {
      break;
    }

    pstMessage++;
  }

  if((pstMessage != NULL) && (pstMessage->pfnOper != NULL))
  {
    pstMessage->pfnOper(stMsg.u16Msg, stMsg.parg);
  }

  if(stMsg.eType == HS_CFG_SYS_EVENT)
    _cfg_sysDoEvent(stMsg.u16Msg, stMsg.parg);

  if(g_u32FastEvent == 0)
    hs_cfg_sysLedSendEvent(stMsg.eType, stMsg.u16Msg);

  #if HS_USE_TONE
  if(g_u32FastEvent == 0)
    hs_cfg_toneDoEvent(stMsg.eType, stMsg.u16Msg, 1);
  #endif
  
  return HS_CFG_OK;
}

static msg_t _cfg_sysServiceThread(void *arg)
{
  osMessageQId pstMess = (osMessageQId)arg;
  hs_cfg_sysmsg_t *pstMsg; 
  osEvent res;

  chRegSetThreadName("sysService");
  while(1)
  {
    res = oshalMessageGet(pstMess, -1);
    if(res.status != osEventMessage)
      continue;

    boardKickWatchDog();
    pstMsg = (hs_cfg_sysmsg_t *)(res.value.v);
    _cfg_sysDoMessage(pstMsg);
  }

  return 0;
}

void hs_cfg_sysInit(void)
{
  osMessageQDef_t mess;
  osThreadDef_t thdDef;

  g_u32FastEvent = 0;
  g_pstEventOper = NULL;
  mess.item_sz = sizeof(long);
  mess.queue_sz = MESSAGE_QUEUE_CNT * sizeof(long);
  mess.items = hs_malloc(mess.queue_sz, __MT_Z_GENERAL);
  __cfg_chkPtrNoRet(mess.items);

  g_pstMess = oshalMessageCreate(&mess, NULL);
  __cfg_chkPtrNoRet(g_pstMess);

  g_pstMsgMutex = oshalMutexCreate(NULL);
  __cfg_chkPtrNoRet(g_pstMsgMutex);

  thdDef.pthread   = (os_pthread)_cfg_sysServiceThread;
  thdDef.stacksize = 1024 * 2;
  thdDef.tpriority = osPriorityAboveNormal;
  oshalThreadCreate(&thdDef, g_pstMess);    
  
  hs_cfg_sysBnInit();
  hs_cfg_sysLedInit();
}

__USED uint32_t hs_cfg_getFastEvent(void)
{
  return g_u32FastEvent;
}

__USED void hs_cfg_setFastEvent(uint32_t u32Cnt)
{
  g_u32FastEvent = u32Cnt;
}

__USED void hs_cfg_sysClearMsg(void)
{
  oshalMessageReset(g_pstMess);
}

/*
 * @brief               send message to system manage when entering some status 
 *                      or some event generated
 *                      
 * @param[in] m_mod     message module
 * @param[in] m_type    message type
 * @param[in] message   message content
 *                      .
 * @return              0-ok, other-some error happened
 */
hs_cfg_res_t hs_cfg_sysSendMsg(hs_cfg_mod_t m_mod, hs_cfg_mess_type_t m_type, uint16_t message, void *parg)
{
  hs_cfg_sysmsg_t *pstMsg;
  osStatus sts;

  if(!g_pstMess)
    return HS_CFG_ERROR;

  pstMsg = (hs_cfg_sysmsg_t *)hs_malloc(sizeof(hs_cfg_sysmsg_t), __MT_Z_GENERAL);
  if(!pstMsg)
    return HS_CFG_ERROR;

  if(message & FAST_EVENT_MASK)
    g_u32FastEvent += 1;

  pstMsg->eMod   = m_mod;
  pstMsg->eType  = m_type;
  pstMsg->u16Msg = message;  
  pstMsg->parg   = parg;
  nds32_dcache_clean();
  
  sts = oshalMessagePut(g_pstMess, (uint32_t)pstMsg, 0);
  if(sts != osOK) 
  {
    hs_free(pstMsg);
    return HS_CFG_ERROR;
  }

  return HS_CFG_OK;
}

hs_cfg_res_t hs_cfg_sysSendMessage(hs_cfg_mod_t m_mod, hs_cfg_mess_type_t m_type, uint16_t message)
{
  return hs_cfg_sysSendMsg(m_mod, m_type, message, NULL);
}

hs_cfg_res_t hs_cfg_sysListenMsg(uint16_t message, hs_evtFunction_t fnOper)
{
  hs_cfg_sys_reg_t *pstEvent = g_pstEventOper;
  hs_cfg_sys_reg_t *pstTemp;

  if(fnOper == NULL)
  {
    return HS_CFG_ERR_REGISTER;
  }

  oshalMutexWait(g_pstMsgMutex, -1);
  if(_cfg_sysChkMsg(message, fnOper))
  {
    oshalMutexRelease(g_pstMsgMutex);
    return HS_CFG_OK;
  }

  pstTemp = _cfg_sysNewEvent(message, fnOper);
  if(pstTemp == NULL)
  {
    oshalMutexRelease(g_pstMsgMutex);
    return HS_CFG_ERR_REGISTER;
  }

  if(g_pstEventOper == NULL)
  {
    g_pstEventOper = pstTemp;
    oshalMutexRelease(g_pstMsgMutex);
    return HS_CFG_OK;
  }

  while(pstEvent->pstNextOper !=NULL)
  {
    pstEvent = pstEvent->pstNextOper;
  }

  pstEvent->pstNextOper = pstTemp;

  oshalMutexRelease(g_pstMsgMutex);
  return HS_CFG_OK;
}

void hs_cfg_sysCancelListenMsg(uint16_t message, hs_evtFunction_t fnOper)
{
  hs_cfg_sys_reg_t *pstEvent = g_pstEventOper;
  hs_cfg_sys_reg_t *pstLast = NULL;

  if(fnOper == NULL)
    return ;

  oshalMutexWait(g_pstMsgMutex, -1);
  while(pstEvent != NULL)
  {
    if((pstEvent->stMessOper.u16MessType == message) 
      && (pstEvent->stMessOper.pfnOper == fnOper))
    {
      if(!pstLast)
        g_pstEventOper = pstEvent->pstNextOper;
      else
        pstLast->pstNextOper = pstEvent->pstNextOper;

      hs_free(pstEvent);
      oshalMutexRelease(g_pstMsgMutex);
      return ;
    }
    else
    {
      pstLast = pstEvent;
      pstEvent = pstEvent->pstNextOper;
    }
  }

  oshalMutexRelease(g_pstMsgMutex);
}


/** @} */
