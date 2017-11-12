/*
    application - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    app/led_engine.c
 * @brief   led application.
 * @details 
 *
 * @addtogroup  app
 * @details 
 * @{
 */
#include "led_engine.h"

#if HS_USE_LEDDISP

#define LED_QUEUE_SIZE    4

typedef struct
{
  osMessageQId        pstMsgId;
  osThreadId          pstThd;
  uint8_t             u8Queue[LED_QUEUE_SIZE * 4];
}hs_ledengine_t;

static hs_ledengine_t g_stledInfo;

#if HS_USE_BT
extern uint8_t* hsc_HFP_GetCurCallNum(void);
#endif

void _led_memIn(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;
  uint32_t type = (uint32_t)parg;

  if(type == FATFS_MEMDEV_SD)
    hs_led_setFunc(LED_FUNC_SD, LED_ON);
  else
    hs_led_setFunc(LED_FUNC_USB, LED_ON);
}

void _led_memOut(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;
  uint32_t type = (uint32_t)parg;

  if(type == FATFS_MEMDEV_SD)
    hs_led_setFunc(LED_FUNC_SD, LED_OFF);
  else
    hs_led_setFunc(LED_FUNC_USB, LED_OFF);
}

void _led_dispIdle(void)
{
  hs_led_setDig(0, 10);
  hs_led_setDig(1, 10);

  hs_led_setFunc(LED_FUNC_COLON, LED_OFF);

  hs_led_setDig(2, 10);
  hs_led_setDig(3, 10);
}

void _led_dispVolume(hs_avol_dev_t eDev)
{
  uint16_t vol;

  vol = hs_audio_volGetLvl(eDev);
  
  hs_led_setDig(0, 18);
  hs_led_setDig(1, 13);

  hs_led_setFunc(LED_FUNC_COLON, LED_OFF);

  hs_led_setDig(2, vol / 10);
  hs_led_setDig(3, vol % 10);
}

void _led_dispBlue(void)
{
  hs_led_setDig(0, 11);
  hs_led_setDig(1, 12);

  hs_led_setFunc(LED_FUNC_COLON, LED_OFF);

  hs_led_setDig(2, 13);
  hs_led_setDig(3, 14);

  hs_led_setFunc(LED_FUNC_MP3, LED_OFF);
  hs_led_setFunc(LED_FUNC_FM,  LED_OFF);
}

void _led_dispPh(void)
{
  hs_led_setDig(0, 10);
  hs_led_setDig(1, 15);

  hs_led_setFunc(LED_FUNC_COLON, LED_OFF);

  hs_led_setDig(2, 16);
  hs_led_setDig(3, 10);

  hs_led_setFunc(LED_FUNC_MP3, LED_OFF);
  hs_led_setFunc(LED_FUNC_FM,  LED_OFF);
}

void _led_dispMusic(void)
{
  uint32_t u32Time;
  uint8_t  u8Min, u8Ms;
  hs_musicstatus_t eStatus;

  u32Time = hs_music_getTime();

  u8Min = u32Time / 60;
  u8Ms  = u32Time % 60;

  hs_led_setDig(0, u8Min / 10);
  hs_led_setDig(1, u8Min % 10);
  
  hs_led_setDig(2, u8Ms / 10);
  hs_led_setDig(3, u8Ms % 10);

  hs_led_setFunc(LED_FUNC_MP3,   LED_ON);
  hs_led_setFunc(LED_FUNC_FM,    LED_OFF);
  hs_led_setFuncReverse(LED_FUNC_COLON);

  eStatus = hs_music_getStatus();
  if(eStatus == MUSIC_STATUS_PLAYING)
  {
    hs_led_setFunc(LED_FUNC_PLAY,   LED_ON);
    hs_led_setFunc(LED_FUNC_PAUSE,  LED_OFF);
  }
  else if(eStatus == MUSIC_STATUS_TERMINATE)
  {
    hs_led_setFunc(LED_FUNC_PLAY,   LED_OFF);
    hs_led_setFunc(LED_FUNC_PAUSE,  LED_OFF);
  }
  else
  {
    hs_led_setFunc(LED_FUNC_PLAY,   LED_OFF);
    hs_led_setFunc(LED_FUNC_PAUSE,  LED_ON);
  }
}

void _led_dispMusicIdx(void)
{
  uint8_t u8Idx = hs_music_getIdx();
  
  hs_led_setDig(0, 18);
  hs_led_setDig(1, 15);
  
  hs_led_setDig(2, u8Idx / 10);
  hs_led_setDig(3, u8Idx % 10);

  hs_led_setFunc(LED_FUNC_MP3,   LED_ON);
  hs_led_setFunc(LED_FUNC_FM,    LED_OFF);
  hs_led_setFunc(LED_FUNC_COLON, LED_OFF);
  hs_led_setFunc(LED_FUNC_PLAY,   LED_ON);
}

void _led_dispFmChn(uint8_t u8Chn)
{
  hs_led_setDig(0, 18);
  hs_led_setDig(1, 15);

  hs_led_setFunc(LED_FUNC_COLON, LED_OFF);

  hs_led_setDig(2, u8Chn / 10);
  hs_led_setDig(3, u8Chn % 10);
}


void _led_dispFm(uint32_t u32Freq)
{
  uint32_t u32Dig;

  u32Freq = u32Freq / 100;
  u32Dig = (u32Freq / 1000) == 0 ? 18 : (u32Freq / 1000);  
  hs_led_setDig(0, u32Dig);

  u32Freq = u32Freq % 1000;
  u32Dig = u32Freq / 100;
  hs_led_setDig(1, u32Dig);
  
  u32Freq %= 100;  
  hs_led_setDig(2, u32Freq / 10);
  hs_led_setDig(3, u32Freq % 10);

  hs_led_setFunc(LED_FUNC_MP3, LED_OFF);
  hs_led_setFunc(LED_FUNC_FM,  LED_ON);
  hs_led_setFunc(LED_FUNC_COLON, LED_OFF);

  hs_led_setFunc(LED_FUNC_PLAY,   LED_OFF);
  hs_led_setFunc(LED_FUNC_PAUSE,  LED_OFF);
}

void _led_dispAux(void)
{
  hs_led_setDig(0, 17);
  hs_led_setDig(1, 13);

  hs_led_setDig(2, 16);
  hs_led_setDig(3, 18);

  hs_led_setFunc(LED_FUNC_COLON, LED_OFF);
  hs_led_setFunc(LED_FUNC_MP3, LED_OFF);
  hs_led_setFunc(LED_FUNC_FM,  LED_OFF);
}

static void _led_dispPhoneNum(hs_ledengine_t *pstLedInfo)
{
  uint8_t *pu8PhoneNum = NULL;
  uint8_t dig, idx, led_idx = 1;
  osEvent eEvt;

#if HS_USE_BT
  if (NULL == pu8PhoneNum)
    pu8PhoneNum = (uint8_t *)hsc_HFP_GetCurCallNum();
#endif
  if (NULL == pu8PhoneNum)
    return;

  eEvt.status = osEventTimeout;
  hs_led_setBlank();
  while (osEventTimeout == eEvt.status) {
    for (idx = 0; idx < 32/*CFG_RING_CALLPHONE_LEN*/; idx++) {
      dig = *(pu8PhoneNum+idx);
      if ('\0' == dig)
        break;
      if ((0 == idx) && ('1' == dig))
        hs_led_setDig(0, dig-'0');
      else
        hs_led_setDig(led_idx++, dig-'0');
      if (4 == led_idx) {
        eEvt = oshalMessageGet(pstLedInfo->pstMsgId, 1000);
        hs_led_setBlank();
        led_idx = 1;
        if (osEventTimeout != eEvt.status)
          break;
      }
    }
  }
}

void _led_engineDispMode()
{
#if 0//xcx
  hs_usermode_t eMode;
  uint32_t u32Freq;
  
  eMode = hs_menu_getMode();
  switch(eMode)
  {
    case USER_MODE_BT:
      _led_dispBlue();
      break;

    case USER_MODE_BT_HID:
      _led_dispPh();
      break;

    case USER_MODE_MUSIC1:
    case USER_MODE_MUSIC2:
      _led_dispMusic();
      break;

    case USER_MODE_FM:
      u32Freq = hs_fm_getFreq();
      u32Freq >>= 8;
      _led_dispFm(u32Freq);
      break;

    case USER_MODE_AUX:
      _led_dispAux();
      break;

    default:
      _led_dispIdle();
      break;
  }
#endif
}

void _led_engineDispInfo(hs_ledengine_t *pstLedInfo, hs_led_dispType_t eDispType)
{
  (void )pstLedInfo;
  uint16_t u16tmp = (uint16_t)eDispType;
  hs_avol_dev_t eDev;
  uint32_t u32Freq;
  
  switch(eDispType & LED_DISP_MASK)
  {
#if 1//xcx
    case LED_DISP_PHONE_NUM:
      _led_dispPhoneNum(pstLedInfo);
      break;
#else
    case LED_DISP_MODE_CHANGE:
      _led_engineDispMode();
      break;

    case LED_DISP_MODE_IDLE:
      _led_dispIdle();
      break;

    case LED_DISP_VOLUME:      
      eDev = (hs_avol_dev_t)((u16tmp >> 12u) & 0xf);
      _led_dispVolume(eDev);
      break;

    case LED_DISP_FREQ:
      u32Freq = hs_fm_getFreq();
      if((u32Freq & 0xff) == 0xff)
      {
        u32Freq >>= 8;
        _led_dispFm(u32Freq);
      }
      else
      {
        u32Freq &= 0xff;
        _led_dispFmChn(u32Freq);
      }
      break;

    case LED_DISP_MUSICIDX:
      _led_dispMusicIdx();
      break;
#endif

    default:
      break;
  }
}

void _led_engineThread(void* arg)
{
  hs_ledengine_t *pstLedInfo = (hs_ledengine_t *)arg;
  osEvent eEvt;
  hs_led_dispType_t eMsg;

  chRegSetThreadName("ledsEngine");
  while(1)
  {
    eEvt = oshalMessageGet(pstLedInfo->pstMsgId, 1000);
    eMsg = (hs_led_dispType_t)eEvt.value.v;
    
    switch(eEvt.status)
    {
    case osEventTimeout:    
      _led_engineDispInfo(pstLedInfo, LED_DISP_MODE_CHANGE);
      break;
    
    case osEventMessage:
      _led_engineDispInfo(pstLedInfo, eMsg);
      break;
    default:
      break;
    }
  }
}

void hs_led_init(void)
{
  osMessageQDef_t stMsgDef;
  osThreadDef_t stThdDef;
  
  hs_led_initDisp(100);

  hs_cfg_sysListenMsg(HS_CFG_EVENT_MEMDEV_IN,  _led_memIn);
  hs_cfg_sysListenMsg(HS_CFG_EVENT_MEMDEV_OUT, _led_memOut); 

  stMsgDef.queue_sz = LED_QUEUE_SIZE * sizeof(int);
  stMsgDef.item_sz  = sizeof(int);
  stMsgDef.items    = g_stledInfo.u8Queue;
  g_stledInfo.pstMsgId = oshalMessageCreate(&stMsgDef, NULL);

  stThdDef.pthread   = (os_pthread)_led_engineThread;
  stThdDef.stacksize = 512;
  stThdDef.tpriority = osPriorityNormal;     
  g_stledInfo.pstThd  = oshalThreadCreate(&stThdDef, &g_stledInfo); 
}

void hs_led_disp(hs_led_dispType_t eMsg)
{
  if(g_stledInfo.pstMsgId)
    oshalMessagePut(g_stledInfo.pstMsgId, eMsg, 0);
}

#endif

/** @} */
