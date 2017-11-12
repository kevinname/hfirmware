#include "lib.h"
#include "bt_config.h"
#include "bt_os.h"
#include "autoconf.h"

#define BT_HOST_VAR_STATUS_NULL     0x00
#define BT_HOST_VAR_STATUS_STARTING 0x01
#define BT_HOST_VAR_STATUS_START    0x02
#define BT_HOST_VAR_STATUS_STOPING  0x03

typedef struct
{
  osThreadId  pstHostThd;

  #ifndef CONFIG_USE_OS_TIMER
  osThreadId  pstTmrThd;
  #endif

  #ifdef CONFIG_HFP
  osThreadId  pstHfpThd;
  #endif

  #ifdef CONFIG_A2DP
  osThreadId   pstA2dpThd;
  osMessageQId pstA2dpMsgId;
  #endif
  uint32_t    u32Status;
}__bthostVar_t;

static __bthostVar_t *g_pstHostVar;

extern void BtInit(void);
extern void BtDone(void);
extern void App_FsmInit(uint8_t mode);
extern void App_FsmDone(void);
extern void App_FsmWaitReconnect(void);
extern void InitTransportLayer(void);
extern void FreeTransportLayer(void);
extern void ScheduleLoop(void);
extern void APP_AudioScheduleLoop(void);
extern void APP_AudioThreadExit(void);
extern void BtTimerInit(void);
extern void BtTimerExit(void);
extern void BtTimerTask(void);
extern void App_AudioStop(void);
extern uint8_t hsc_GetBtMode(void);

#ifdef CONFIG_A2DP
extern void hsc_A2dpScheduleLoop(osMessageQId pstA2dpMsgId);
extern void hsc_A2dpThreadExit(void);

#define APP_BT_A2DP_QUEUE_SIZE   8
static uint32_t app_thread_a2dp_queue[APP_BT_A2DP_QUEUE_SIZE * 4];
#endif

#ifdef CONFIG_SPP
extern void App_SPP_Stop(void);
#endif
#ifdef CONFIG_BLE
extern void App_BLE_Stop(void);
extern void App_BTLED_Uninit(void);
#endif
extern void App_SetBtState(uint16_t state);
#ifdef CONFIG_HID
extern void App_HID_Stop(void);
#endif
#ifdef CONFIG_DEBUG
extern void DBG_Init(void);
extern void DBG_UnInit(void);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	This is the function to write debug string to the APP defined output
	device.
---------------------------------------------------------------------------*/
static void App_DbgPrintFunc(UINT32 length, UINT8 *data)
{
  PRINT_TIMESTAMP();
  
  while (length--) 
  {
    if ('\n' == *data)
      hs_printf('\r');
    
    hs_printf("%c", *data);
    data++;
  }
}
#endif

#ifdef CONFIG_MEM_USER_FUNC
void os_alert(void)
{
  HS_UART1->THR = 'O';
  __disable_irq();
  while(1);
}
#endif


static void _bthost_thread(void *arg)
{
  (void)arg;
  chRegSetThreadName("bthost");

  ScheduleLoop();
}

#ifndef CONFIG_USE_OS_TIMER
static void _bthost_tmrThread(void *arg)
{
  (void)arg;

  chRegSetThreadName("bttmr");
  BtTimerTask();
}
#endif

#ifdef CONFIG_HFP
static void _bthost_hfpThread(void *arg)
{
  (void)arg;
  
  chRegSetThreadName("hfp");
  APP_AudioScheduleLoop();
}
#endif 

#ifdef CONFIG_A2DP
static void _bthost_a2dpThread(void *arg)
{
  __bthostVar_t *pHostVar = (__bthostVar_t *)arg;
  
  chRegSetThreadName("a2dp");
  hsc_A2dpScheduleLoop(pHostVar->pstA2dpMsgId);
}
#endif 

int hs_bthost_start(uint8_t mode)
{
  osThreadDef_t thdDef;

  if (g_pstHostVar)
    return 0;

  g_pstHostVar = (__bthostVar_t *)hs_malloc(sizeof(__bthostVar_t), __MT_Z_GENERAL);
  if(!g_pstHostVar)
    return -1;
  //hs_printf("bt starting...\r\n");
  g_pstHostVar->u32Status = BT_HOST_VAR_STATUS_STARTING;
  App_CFG_Init(mode);

  /* Register debug output if supported */
#ifdef CONFIG_DEBUG
  SetDbgPrint((DbgPrintFunc *)App_DbgPrintFunc);
  DBG_Init();
#endif
#ifdef CONFIG_MEM_USER_FUNC
  RegisterUserFunc((uint8_t *)os_malloc, (uint8_t *)os_free, (uint8_t *)os_alert);
#endif

  /* IVT stack init */
  BtInit();
  /* APP init */
  App_FsmInit(mode);
  /* Initialize transport layer */
  InitTransportLayer();
#ifndef CONFIG_USE_OS_TIMER
  /* timers depends on static memory allocator, and fsm depends on timers */
  BtTimerInit();
#endif

  thdDef.pthread   = (os_pthread)_bthost_thread;
  thdDef.stacksize = TASK_STK_SIZE_BT_HOST;
  thdDef.tpriority = TASK_PRIO_BT_HOST;
  g_pstHostVar->pstHostThd = oshalThreadCreate(&thdDef, g_pstHostVar); 

#ifndef CONFIG_USE_OS_TIMER
  thdDef.pthread   = (os_pthread)_bthost_tmrThread;
  thdDef.stacksize = TASK_STK_SIZE_BT_TIMER;
  thdDef.tpriority = TASK_PRIO_BT_TIMER;
  g_pstHostVar->pstTmrThd = oshalThreadCreate(&thdDef, g_pstHostVar); 
#endif

  #ifdef CONFIG_HFP
  thdDef.pthread   = (os_pthread)_bthost_hfpThread;
  thdDef.stacksize = TASK_STK_SIZE_BT_HFP;
  thdDef.tpriority = TASK_PRIO_BT_HFP;
  g_pstHostVar->pstHfpThd = oshalThreadCreate(&thdDef, g_pstHostVar); 
  #endif

  #ifdef CONFIG_A2DP
  {
    osMessageQDef_t stMsgDef;
    stMsgDef.queue_sz = APP_BT_A2DP_QUEUE_SIZE * sizeof(int);
    stMsgDef.item_sz  = sizeof(int);
    stMsgDef.items    = app_thread_a2dp_queue;
    g_pstHostVar->pstA2dpMsgId = oshalMessageCreate(&stMsgDef, NULL);
    
    thdDef.pthread   = (os_pthread)_bthost_a2dpThread;
    thdDef.stacksize = TASK_STK_SIZE_BT_A2DP;
    thdDef.tpriority = TASK_PRIO_BT_A2DP;
    g_pstHostVar->pstA2dpThd = oshalThreadCreate(&thdDef, g_pstHostVar); 
  }
  #endif
  
  msleep(1000);
  g_pstHostVar->u32Status = BT_HOST_VAR_STATUS_START;
  //hs_printf("bt start ok...\r\n");
  return 0;
}

/*
 * stop host stack threads
 */
void hs_bthost_stop(uint8_t mode)
{
  (void)mode;
  if(!g_pstHostVar)
    return ;  

  if (g_pstHostVar->u32Status == BT_HOST_VAR_STATUS_STOPING)
    return ;
    
  // wait status to BT_HOST_VAR_STATUS_START
  while (g_pstHostVar->u32Status == BT_HOST_VAR_STATUS_STARTING)
  {
    msleep(100);
  } 

  hs_printf("bt stoping...\r\n");
  g_pstHostVar->u32Status = BT_HOST_VAR_STATUS_STOPING;
  App_SetBtState(0x0001); //APP_BT_STATE_NULL
  App_FsmWaitReconnect();
  //hs_printf("bt stoping step 1\r\n");

  #ifdef CONFIG_HFP
  /* exit task of bt hfp audio record */
  if (g_pstHostVar->pstHfpThd != NULL)
  {
    APP_AudioThreadExit();
    osThreadTerminate(g_pstHostVar->pstHfpThd);
  }
  #endif

#ifndef CONFIG_USE_OS_TIMER
  /* exit task of bt timer */
  BtTimerExit();
  osThreadTerminate(g_pstHostVar->pstTmrThd);
#endif

  //hs_printf("bt stoping step 2\r\n");

  /* exit task of bt host stack */
  BtDone();
  osThreadTerminate(g_pstHostVar->pstHostThd);

#ifdef CONFIG_BLE
  App_BTLED_Uninit();
  App_BLE_Stop();
#endif
  //hs_printf("bt stoping step 3\r\n");
  if (hsc_GetBtMode() == BT_HOST_VAR_MODE_AUDIO)
  {
    App_AudioStop();

    #ifdef CONFIG_SPP
    App_SPP_Stop();
    #endif
  }

  #ifdef CONFIG_A2DP
  if (g_pstHostVar->pstA2dpThd != NULL)
  {
    hsc_A2dpThreadExit();
    osThreadTerminate(g_pstHostVar->pstA2dpThd);

    oshalMessageFree(g_pstHostVar->pstA2dpMsgId);
  }
  #endif
  if (hsc_GetBtMode() == BT_HOST_VAR_MODE_HID)
  {
#ifdef CONFIG_HID
      App_HID_Stop();
#endif
  }
  App_FsmDone();
  
#ifdef CONFIG_DEBUG
  DBG_UnInit();
#endif

  hs_cfg_flush(FLUSH_TYPE_ALL);
  
  hs_free(g_pstHostVar);
  g_pstHostVar = NULL;
  App_SetBtState(0x0001); //APP_BT_STATE_NULL

  hs_printf("bt stop ok!\r\n");
}


