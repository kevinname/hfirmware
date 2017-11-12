#include "lib.h"
#include "bt_config.h"
#include "bthc_uapi.h"
#include "bt_os.h"

#define BTHC_TMOUT        (1 * CH_CFG_ST_FREQUENCY / 1000)

extern void FreeTransportLayer(void);

enum
{
  BTHC_STATUS_STOPPED     = 0,
  BTHC_STATUS_RUNNING     ,
};

typedef struct
{
  osThreadId  pstThd;
  uint32_t    u32Status;
}__bthcVar_t;

static __bthcVar_t *g_pstHcVar;

extern int BTms_OS_Event_Dispatcher(void);
extern void BTms_OS_Event_Exit(void);
extern void SYSirq_Baseband_IRQ_Handler(void);
extern void vhci_h4_hc2host_packet(uint8_t *data, uint32_t pdu_len, uint8_t *header, uint8_t head_len);

OSAL_IRQ_HANDLER(BT_IRQHandler)
{
  OSAL_IRQ_PROLOGUE();

  //chSysLockFromISR();
  SYSirq_Baseband_IRQ_Handler();
  //chSysUnlockFromISR();
  
  OSAL_IRQ_EPILOGUE();
}

static __ONCHIP_VHCI__ void _bthc_setSignalI(uint32_t mask)
{
  if(g_pstHcVar)
    oshalSignalSet(g_pstHcVar->pstThd, mask);
}

static __ONCHIP_VHCI__ uint32_t _bthc_waitSignal(uint32_t mask, uint16_t timeout)
{
  osEvent evt;

  mask &= 0xffff; 
  if (mask == 0xffff)
    mask = 0;
    
  evt = oshalSignalWait(mask, timeout);
  if (evt.status == osEventSignal)
    return evt.value.signals;

  return 0;
}


static void _bthc_thread(void *arg)
{
  (void)arg;
  
  chRegSetThreadName("bthc");

#if HS_USE_BT != HS_BT_DONGLE
  /* uint8_t *data, uint32_t pdu_len, uint8_t *header, uint8_t head_len */
  HCI_Generic_Register_Tx_Callback(vhci_h4_hc2host_packet);
#endif

  oshalSignalClear(curthread(), -1);
  HCI_Generic_Register_HC_Event_Handlers_Ex(_bthc_setSignalI, _bthc_waitSignal, BTHC_TMOUT);

  while (1) 
  {
    #if HAL_USE_WATCHDOG
    wdt_keepalive();
    #endif
    
    //(*BTms_Callback)();
    int ret = BTms_OS_Event_Dispatcher();
    if (ret < 0)
      break;
  }
}

void * hs_bthc_malloc(uint32_t size)
{
  void *ret = NULL;

  ret = hs_malloc(size, __MT_GENERAL);

  if (ret == NULL) {__disable_irq(); while(1);}
  
  return ret;
}

void hs_bthc_free(void *p)
{
  hs_free(p);
}

int hs_bthc_start(uint8_t mode)
{
  osThreadDef_t thdDef;

  if(g_pstHcVar)
    return -1;
  
  if (cpm_get_clock(HS_BTBB_CLK) != 24000000)
    return -1;

  if(0 != hs_mem_loadCode(__MCT_BTSTACK))
    return -1;

  g_pstHcVar = (__bthcVar_t *)hs_malloc(sizeof(__bthcVar_t), __MT_Z_GENERAL);
  if(!g_pstHcVar)
    return -1;

  if (HS_PSO->BTPHY_CFG & CPM_BUS_GATE) {
    cpmEnableBTPHY();
    /* guarantee LDO is stable */
    msleep(1);
  }
  
#if HS_USE_BT == HS_BT_DONGLE
  (void)mode;
  HCI_Generic_Config(NULL, NULL);
#else
  /* it will call HCI_Generic_Config() */
  App_CFG_GetBtHcRf(mode);
#endif
  
  /* The numbers passed in become are obsoleted. 
     It will allocate data buffers which are specified in g_sys_config.
     The last two arguments are borrowed to pass into malloc/free hooks. */
  HCI_Generic_Initialise(12,              /* num_in_acl_packets */
                         8,               /* num_out_acl_packets */
                         20,              /* num_in_sco_packets */
                         20,              /* num_out_sco_packets */
                         hs_bthc_malloc,  /* flash_config_addr */
                         hs_bthc_free     /* hw_config_info */);

  NVIC_SetPriority(IRQ_BTBB, HS_BT_IRQ_PRIORITY);
  NVIC_EnableIRQ(IRQ_BTBB);

  thdDef.pthread   = (os_pthread)_bthc_thread;
  thdDef.stacksize = TASK_STK_SIZE_BT_HC;
  thdDef.tpriority = TASK_PRIO_BT_HC;
  g_pstHcVar->pstThd = oshalThreadCreate(&thdDef, g_pstHcVar); 
  if(!g_pstHcVar->pstThd)
  {
    hs_free(g_pstHcVar);
    g_pstHcVar = NULL;
    return -1;
  }

  return 0;
}

extern void LM_Shutdown(void);

void hs_bthc_stop(void)
{
  if(!g_pstHcVar)
    return ;
#if 0
#ifdef CONFIG_A2DP
  if (hsc_A2DP_GetState() == APP_A2DP_STATE_START) {
      hsc_A2DP_Stop();
      msleep(500);
  }
#endif
#endif

  LM_Shutdown();

  FreeTransportLayer();
#if HS_USE_BT != HS_BT_DONGLE
  HCI_Generic_Register_Tx_Callback(NULL);
#endif

  NVIC_DisableIRQ(IRQ_BTBB);
  BTms_OS_Event_Exit();
  oshalThreadTerminate(g_pstHcVar->pstThd);
  HCI_Generic_Shutdown();
  cpmDisableBTPHY();

  hs_free(g_pstHcVar);
  g_pstHcVar = NULL;
}

//used to low power(sniff mode)
extern hs_cfg_res_t hs_cfg_sysSendMsg(hs_cfg_mod_t m_mod, hs_cfg_mess_type_t m_type, uint16_t message, void *parg);
__attribute__((used)) void hs_bthc_send_event(bt_low_power_t event) 
{
    chSysUnlockFromISR();
    hs_cfg_sysSendMsg(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_LOW_POWER, (void*)event);
    chSysLockFromISR();
}


