/*
    application - Copyright (C) 2012~2017 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/user/.../menu.c
 * @brief   user function
 * @details 
 *
 * @addtogroup  lib user's function
 * @details 
 * @{
 */
#include "lib.h"

static hs_usermode_t g_eMenuMode;

void _ledBT_start(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;
  (void)parg;  
  
  hs_led_disp(LED_DISP_MODE_CHANGE);
  hs_cfg_sysCancelListenMsg(HS_CFG_EVENT_BT_PAIRABLE, _ledBT_start);
}

void _ledPh_start(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;
  (void)parg;
  
  hs_led_disp(LED_DISP_MODE_CHANGE);
  hs_cfg_sysCancelListenMsg(HS_CFG_EVENT_BT_HID_PARIABLE, _ledPh_start);
}

void _ledFm_start(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;
  (void)parg;
  
  hs_led_disp(LED_DISP_MODE_CHANGE);
  hs_cfg_sysCancelListenMsg(HS_CFG_EVENT_PLAYER_FMCREATE, _ledFm_start);
}

void _ledAux_start(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;
  (void)parg;
  
  hs_led_disp(LED_DISP_MODE_CHANGE);
  hs_cfg_sysCancelListenMsg(HS_CFG_EVENT_PLAYER_AUXCREATE, _ledAux_start);
}

void _menu_sdmemIn(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;
  uint32_t type = (uint32_t)parg;

  if(type == FATFS_MEMDEV_SD)
    hs_cfg_playerReq(HS_CFG_EVENT_PLAYER_USBAUDCREATE);
}

void _menu_modeStart(hs_usermode_t eMode)
{
  switch(eMode)
  {
    /*
     * Bluetooh Audio Mode, no player.
     */
    case USER_MODE_BT:  
      hs_led_disp(LED_DISP_MODE_IDLE);
      hs_cfg_sysListenMsg(HS_CFG_EVENT_BT_PAIRABLE,  _ledBT_start);
      /* 
       * before starting bluetooth, pls destroy player first 
       */
      hs_cfg_playerReq(HS_CFG_EVENT_PLAYER_DESTROY);
      hs_cfg_systemReq(HS_CFG_EVENT_BT_AUDIO_START);
      hs_printf("Bluetooth starting...\r\n");
      break;

    /*
     * Music Mode, Bluetooth running in background.
     */     
    case USER_MODE_MUSIC1:    
      hs_led_disp(LED_DISP_MODE_IDLE);
      /* 
       * before starting music, pls stop bluetooth first 
       */ 
      hs_cfg_systemReq(HS_CFG_EVENT_BT_AUDIO_STOP);
      
      /* 
       * listen whether some memory device insert.
       * if some memory device inserted, music restarting.
       */
      if(hs_fatfs_isMount(FATFS_MEMDEV_SD) || hs_fatfs_isMount(FATFS_MEMDEV_UDISK))
      {
        hs_cfg_playerReq(HS_CFG_EVENT_PLAYER_MP3CREATE);
        hs_printf("Music of player starting...\r\n");
        break;
      }
      /* fallthrough */
      g_eMenuMode = USER_MODE_AUX;

    /*
     * AUX Mode, Bluetooth running in background. 
     */
    case USER_MODE_AUX:
      hs_led_disp(LED_DISP_MODE_IDLE);
      hs_cfg_sysListenMsg(HS_CFG_EVENT_PLAYER_AUXCREATE, _ledAux_start);
      /* 
       * aux start.
       */
      hs_cfg_playerReq(HS_CFG_EVENT_PLAYER_DESTROY);
      hs_cfg_playerReq(HS_CFG_EVENT_PLAYER_AUXCREATE);
      hs_printf("AUX of player starting...\r\n");
      break;

    default:
      break;      
  }
}

void _menu_modeChange(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;
  (void)parg;
  
  g_eMenuMode += 1;
  g_eMenuMode %= USER_MODE_NUM;
  _menu_modeStart(g_eMenuMode);
}

void _menu_memIn(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;
  (void)parg;

  hs_cfg_systemReq(HS_CFG_EVENT_BT_AUDIO_STOP);

  g_eMenuMode = USER_MODE_MUSIC1;
  _menu_modeStart(g_eMenuMode);
}

void _menu_memOut(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;
  (void)parg;

  if((g_eMenuMode == USER_MODE_MUSIC1) || (g_eMenuMode == USER_MODE_MUSIC1))
  {
    if(hs_fatfs_isMount(FATFS_MEMDEV_SD) || hs_fatfs_isMount(FATFS_MEMDEV_UDISK))
      return ;
    
    g_eMenuMode = USER_MODE_AUX;
    _menu_modeStart(g_eMenuMode);
  }
}

void hs_menu_init(hs_usermode_t eDefaultMode)
{
  g_eMenuMode = eDefaultMode;
  _menu_modeStart(eDefaultMode);
  hs_menu_ir_init();

  hs_cfg_sysListenMsg(USER_MSG_MODE_CHANGED,  _menu_modeChange);

  msleep(8000);
  hs_cfg_sysListenMsg(HS_CFG_EVENT_MEMDEV_IN,  _menu_memIn);  
  hs_cfg_sysListenMsg(HS_CFG_EVENT_MEMDEV_OUT, _menu_memOut); 
}

hs_usermode_t hs_menu_getMode(void)
{
  return g_eMenuMode;
}


/** @} */
