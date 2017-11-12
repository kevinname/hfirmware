#include "oshal.h"
#include "lib_mem.h"
#include "config.h"

/********************************************************************************************
 * virt-timer
 ********************************************************************************************/
static mailbox_t g_stTmMail;
static uint32_t  g_stTmQueue[5];

static void _oshal_timerCb(void const *arg) 
{
  osTimerId pstTimerId = (osTimerId)arg;

  chSysLockFromISR();
  chMBPostI(&g_stTmMail, (msg_t)arg);  
  if (pstTimerId->type == osTimerPeriodic) 
  {    
    chVTDoSetI(&pstTimerId->vt, MS2ST(pstTimerId->millisec),
                             (vtfunc_t)_oshal_timerCb, pstTimerId);
  }
  chSysUnlockFromISR();
}

osTimerId oshalTimerCreate(const osTimerDef_t *timer_def, os_timer_type type, void *argument)
{
  osTimerId pstTimerId;

  pstTimerId = (osTimerId)hs_malloc(sizeof(struct os_timer_cb), __MT_Z_GENERAL);
  if(pstTimerId == NULL)
    return NULL;
  
  pstTimerId->argument = argument;
  pstTimerId->type = type;
  pstTimerId->ptimer = timer_def->ptimer;
  chVTObjectInit(&pstTimerId->vt);

  return pstTimerId;
}

osStatus  oshalTimerStart(osTimerId timer_id, uint32_t millisec)
{
  syssts_t sts;
  
  if (millisec == 0)
    return osErrorValue;

  if(timer_id == NULL)
    return osErrorValue;

  timer_id->millisec = millisec;
  
  sts = chSysGetStatusAndLockX();
  chVTSetI(&timer_id->vt, MS2ST(millisec), (vtfunc_t)_oshal_timerCb, timer_id);
  chSysRestoreStatusX(sts);

  return osOK;
}

osStatus  oshalTimerStop(osTimerId timer_id)
{
  syssts_t sts;

  if(timer_id == NULL)
    return osErrorValue;

  sts = chSysGetStatusAndLockX();
  chVTResetI(&timer_id->vt);
  chSysRestoreStatusX(sts);

  return osOK;
}

osStatus  oshalTimerDelete(osTimerId timer_id)
{
  if(timer_id == NULL)
    return osErrorValue;
  
  oshalTimerStop(timer_id);
  hs_free((void *)timer_id);

  return osOK;
}

static void _oshal_timerServeThd(void *argument)
{
  mailbox_t *pstMail = (mailbox_t *)argument;
  osTimerId  pstTimerId;
  msg_t rdy, msg;

  chRegSetThreadName("TimerService");
  while(1)
  {
    rdy = chMBFetch(pstMail, &msg, TIME_INFINITE);
    if(MSG_OK == rdy)
    {
      pstTimerId = (osTimerId)msg;
      if((pstTimerId != NULL) && (pstTimerId->ptimer != NULL))
      {
        pstTimerId->ptimer(pstTimerId->argument);
      }
    }
  }
}
  
static void oshalTimerInit(void)
{
  osThreadDef_t stTimerThd;

  chMBObjectInit(&g_stTmMail, (msg_t *)g_stTmQueue, 5);
  
  stTimerThd.pthread = (os_pthread)_oshal_timerServeThd;
  stTimerThd.stacksize = 512;
  stTimerThd.tpriority = 63;

  oshalThreadCreate(&stTimerThd, &g_stTmMail);
  
}

osMessageQId oshalMessageCreate(osMessageQDef_t *queue_def, osThreadId thread_id)
{
  queue_def->mailbox = hs_malloc(sizeof(mailbox_t), __MT_Z_GENERAL);
  if(queue_def->mailbox == NULL)
    return NULL;
  
  osMessageCreate(queue_def, thread_id);

  return (osMessageQId)(queue_def->mailbox);
}

osStatus oshalMessageFree(osMessageQId queue_id)
{
  hs_free((void *)queue_id);

  return osOK;
}

void oshalMessageReset(osMessageQId queue_id)
{
  if(!queue_id)
    return ;
  
  if (port_is_isr_context()) 
  {
    chSysLockFromISR();
    chMBResetI((mailbox_t *)queue_id);
    chSysUnlockFromISR();
  }
  else
  {
    chMBReset((mailbox_t *)queue_id);
  }
}

osStatus oshalKernelInitialize(void)
{
  osKernelInitialize();
  oshalTimerInit();  
  return osOK;
}



