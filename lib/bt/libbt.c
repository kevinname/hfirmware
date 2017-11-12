#include "lib.h"
#include "bt_os.h"

#if HS_USE_BT == HS_BT_DATA_TRANS
#include "btstack_uapi.h"
extern void bt_hc_start(void);
extern void bt_hc_stop(void);
extern void bt_host_start(void);
extern void bt_host_stop(void);
extern void App_CFG_GetBtHcRf(uint8_t mode);
extern void App_CFG_GetBtHost(void* arg);
extern void bt_host_control_init(void);
extern void bt_host_control_uninit(void);

int8_t bluetooth_init(void)
{
    bt_host_sys_config_t *p_host_cfg;
    
    if (cpm_get_clock(HS_BTBB_CLK) != 24000000)
      return -1;

    if(0 != hs_mem_loadCode(__MCT_BTSTACK))
      return -1;

    if (HS_PSO->BTPHY_CFG & CPM_BUS_GATE) {
      cpmEnableBTPHY();
      /* guarantee LDO is stable */
      msleep(1);
    }
    
    App_CFG_GetBtHcRf(0);
    bt_host_get_sys_config(&p_host_cfg);
    App_CFG_GetBtHost((void*)p_host_cfg);
    bt_host_control_init();
    
    NVIC_SetPriority(IRQ_BTBB, HS_BT_IRQ_PRIORITY);
    NVIC_EnableIRQ(IRQ_BTBB);
    return 0;
}
  
void hs_bt_start(uint16_t u16Idx, void *parg)
{
  (void)u16Idx;
  (void)parg;
  
  if (bluetooth_init() != 0)
  {
    return;
  }
  hs_printf("bt start...\r\n");
  bt_hc_start();
  hs_printf("hc ok...\r\n");
  bt_host_start();
  hs_printf("bt start.\r\n");
}
  
void hs_bt_stop(uint16_t u16Idx, void *parg)
{
  (void)u16Idx;
  (void)parg;
  
  NVIC_DisableIRQ(IRQ_BTBB);
  bt_host_control_uninit();
  bt_host_stop();
  bt_hc_stop();
  hs_printf("bt stop.\r\n");
}

#endif

#if HS_USE_BT == HS_BT_AUDIO

uint8_t HCI_Generic_LC_Monitor(void);
void SYSpwr_Halt_System(void);


void hs_bt_start(uint16_t u16Idx, void *parg)
{
  (void)u16Idx;
  (void)parg;

  hs_bthc_start(BT_HOST_VAR_MODE_AUDIO);  
  hs_bthost_start(BT_HOST_VAR_MODE_AUDIO);
}

void hs_bt_stop(uint16_t u16Idx, void *parg)
{
  (void)u16Idx;
  (void)parg;
  
  hs_bthc_stop(); 
  hs_bthost_stop(BT_HOST_VAR_MODE_AUDIO); 
}

void hs_bt_hid_start(uint16_t u16Idx, void *parg)
{
  (void)u16Idx;
  (void)parg;

  hs_bthc_start(BT_HOST_VAR_MODE_HID);  
  hs_bthost_start(BT_HOST_VAR_MODE_HID);
}

void hs_bt_hid_stop(uint16_t u16Idx, void *parg)
{
  (void)u16Idx;
  (void)parg;
  
  hs_bthc_stop(); 
  hs_bthost_stop(BT_HOST_VAR_MODE_HID); 
}

void hs_bt_chkLowpower(void)
{
  if (HCI_Generic_LC_Monitor())
  {
    SYSpwr_Halt_System();
  }
}
#endif

#if HS_USE_BT == HS_BT_DONGLE
extern void bt_usb_start(uint8_t dev);
extern void bt_usb_stop(void);

void hs_bt_start(uint16_t u16Idx, void *parg)
{
  (void)u16Idx;

  hs_bthc_start((uint8_t)((uint32_t)parg));
  bt_usb_start(0);
}

void hs_bt_stop(uint16_t u16Idx, void *parg)
{
  (void)u16Idx;
  (void)parg;

  bt_usb_stop();
  hs_bthc_stop();
}
#endif


