/*
    application - Copyright (C) 2012~2017 HunterSun Technologies
                 wei.lu@huntersun.com.cn
 */

/**
 * @file    lib/user/.../menu_ir.c
 * @brief   user function for infrared remote control
 * @details 
 *
 * @addtogroup  lib user's function
 * @details 
 * @{
 */
#include "lib.h"

typedef enum
{
  MENU_IR_DISP_FREQ  = 0,   
  MENU_IR_EDIT_FREQ  ,
  MENU_IR_EDIT_MUSIC ,
  MENU_IR_EDIT_EQ    ,
  
  MENU_IR_NUM
} hs_menu_ir_t;

typedef struct hs_fmcfg_s
{
  int8_t volume;
  int8_t volume_step;
  int8_t th[6];
  int frequency;
  int freq_step;
  int freq_max;
  int freq_min;
} hs_fmcfg_t;

static int m_fm_freq = 981;
static hs_menu_ir_t m_menu_ir = MENU_IR_DISP_FREQ;

static void _led_dispFreq(int freq)
{
  uint32_t u32Dig, u32Freq = freq;

  if (freq > 1999)
    return;

  u32Dig = (u32Freq / 1000) == 1 ? 1 : CHAR_off;
  hs_led_setDig(0, u32Dig);

  u32Freq = u32Freq % 1000;
  u32Dig = u32Freq / 100;
  if ((u32Dig == 0) && (freq < 1000))
    hs_led_setDig(1, CHAR_sign);
  else
    hs_led_setDig(1, u32Dig);
  
  u32Freq %= 100;  
  u32Dig = u32Freq / 10;
  if ((u32Dig == 0) && (freq < 100))
    hs_led_setDig(2, CHAR_sign);
  else
    hs_led_setDig(2, u32Dig);

  hs_led_setDig(3, u32Freq % 10);

  if (freq < 10)
    hs_led_setFunc(LED_FUNC_DOT, LED_OFF);
  else
    hs_led_setFunc(LED_FUNC_DOT, LED_ON);
}

static void _led_dispNum(int digit)
{
  uint8_t old2, old3;
  old2 = hs_led_getDig(2);
  old3 = hs_led_getDig(3);
  hs_led_setDig(3, digit);
  hs_led_setDig(2, old3);
  hs_led_setDig(1, old2);
}

static void _ir_event_handler(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;
  uint32_t key = (uint32_t)parg;
  static uint16_t fm_digits = 0;
  static uint8_t cur_digit_idx = 0;

  if (16/*eq*/ == key) {
    hs_cfg_sysSendMsg(HS_CFG_MODULE_PLAYER, HS_CFG_SYS_EVENT,
                      HS_CFG_EVENT_PLAYER_FUNCSET, (void *)MUSIC_FUN_EQ);
    return;
  }

  switch (m_menu_ir) {
  case MENU_IR_DISP_FREQ:
    if (19/*ch+*/ == key) {
      if (m_fm_freq < 1999)
        m_fm_freq++;
    }
    else if (20/*ch-*/ == key) {
      if (m_fm_freq > 0)
        m_fm_freq--;
    }
    else if (key <= 9) {
      if (USER_MODE_MUSIC1 == hs_menu_getMode()) {
        m_menu_ir = MENU_IR_EDIT_MUSIC;
        cur_digit_idx = 3;
        hs_led_setBlank();
        _led_dispNum(key);
        return;
      }
      else
        m_menu_ir = MENU_IR_EDIT_FREQ;
      fm_digits = key;
      _led_dispFreq(fm_digits);
    }
    break;

  case MENU_IR_EDIT_FREQ:
    if ((19/*ch+*/ == key) || (20/*ch-*/ == key)) {
      m_menu_ir = MENU_IR_DISP_FREQ;
      m_fm_freq = fm_digits;
    }
    else if (key <= 9) {
      fm_digits = fm_digits * 10 + key;
      if (fm_digits > 1999)
        fm_digits = key;
      _led_dispFreq(fm_digits);
    }
    break;

  case MENU_IR_EDIT_MUSIC:
    if (key <= 9) {
      cur_digit_idx--;
      _led_dispNum(key);
      if (cur_digit_idx == 1) {
        /* hs_music_setIdx(cur_digits) */
        hs_cfg_sysSendMsg(HS_CFG_MODULE_PLAYER, HS_CFG_SYS_EVENT,
                          HS_CFG_EVENT_PLAYER_NEXT, NULL);
        osDelay(1000);
        fm_digits = 0;
        m_menu_ir = MENU_IR_DISP_FREQ;
        _led_dispFreq(m_fm_freq);
        return;
      }
    }
    break;

  default:
    break;
  }

  if (MENU_IR_DISP_FREQ == m_menu_ir) {
    _led_dispFreq(m_fm_freq);
    #if HS_USE_HS6760
    hs6760_fm_set_freq(m_fm_freq);
    #endif
    hs_cfg_setDataByIndex(HS_CFG_MISC_FM, (uint8_t *)&m_fm_freq, 4, 8);
  }
}

static void _led_incoming_call(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;
  (void)parg;
  hs_led_disp(LED_DISP_PHONE_NUM);
}

static void _led_call_ended(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;
  (void)parg;
  hs_led_disp(LED_DISP_PHONE_NUM);
  osDelay(1000);
  _led_dispFreq(m_fm_freq);
}

void hs_menu_ir_init(void)
{
  hs_cfg_res_t ret;
  ret = hs_cfg_getPartDataByIndex(HS_CFG_MISC_FM, (uint8_t *)&m_fm_freq, sizeof(m_fm_freq), 8);
  /* back-compitable with soundbox FM receiver's cfg data */
  if ((HS_CFG_OK == ret) && (m_fm_freq > 1999))
    m_fm_freq /= 100;

  _led_dispFreq(m_fm_freq);
  #if HS_USE_HS6760
  hs6760_fm_set_freq(m_fm_freq);
  hs6760_fm_set_mode(NORMAL_MODE);
  #endif
  hs_cfg_sysListenMsg(HS_CFG_EVENT_IR_INFO, _ir_event_handler);

  hs_cfg_sysListenMsg(HS_CFG_EVENT_BT_INCOMING_CALL, _led_incoming_call);
  hs_cfg_sysListenMsg(HS_CFG_EVENT_BT_CALL_ENDED,    _led_call_ended);
  hs_cfg_sysListenMsg(HS_CFG_EVENT_BT_CALL_REJECTED, _led_call_ended);
}

/** @} */
