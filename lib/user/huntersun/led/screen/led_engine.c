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
  uint32_t            u32TimOut;
  uint32_t            u32DigVal;
  uint8_t             u8Queue[LED_QUEUE_SIZE * 4];

  uint8_t             u8BTCon;
  uint32_t            u32Cnt;
}hs_ledengine_t;

static hs_ledengine_t g_stledInfo;

void _led_doDigitEvent(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;
  uint32_t u32Dig = (uint32_t)parg;

  g_stledInfo.u32DigVal = g_stledInfo.u32DigVal * 10 + u32Dig;

  hs_led_disp(LED_DISP_DIGIT);
}

void _led_doBlueStatus(uint16_t u16Msg, void *parg)
{
  (void)parg;

  if((u16Msg == HS_CFG_EVENT_BT_DISCONNECTED) || (u16Msg == HS_CFG_EVENT_BT_AUDIO_STOP))
    g_stledInfo.u8BTCon = 0;

  if(u16Msg == HS_CFG_EVENT_BT_CONNECTED)
    g_stledInfo.u8BTCon = 1;
}

void _led_dispDigit(void)
{
  uint32_t u32Val = g_stledInfo.u32DigVal;

  u32Val %= 10000;
  hs_led_setDig(0, (u32Val / 1000));
  
  u32Val %= 1000;
  hs_led_setDig(1, (u32Val / 100));
  
  u32Val %= 100;  
  hs_led_setDig(2, u32Val / 10);
  hs_led_setDig(3, u32Val % 10);

  g_stledInfo.u32TimOut = 2000;
}

void _led_dispIdle(void)
{
  hs_led_setDig(0, 10);
  hs_led_setDig(1, 10);

  hs_led_setFunc(LED_FUNC_COLON, LED_OFF);

  hs_led_setDig(2, 10);
  hs_led_setDig(3, 10);
}

void _led_dispHi(void)
{
  hs_led_setDig(0, 18);
  hs_led_setDig(1, 16);
  
  hs_led_setDig(2, 1);
  hs_led_setDig(3, 18);

  hs_led_setFunc(LED_FUNC_PLAY, LED_OFF);  
  hs_led_setFunc(LED_FUNC_COLON, LED_OFF);
  hs_led_setFunc(LED_FUNC_MP3, LED_OFF);
  hs_led_setFunc(LED_FUNC_FM,  LED_OFF);
  hs_led_setFunc(LED_FUNC_SD,  LED_OFF);
  hs_led_setFunc(LED_FUNC_USB, LED_OFF);
  hs_led_setFunc(LED_FUNC_PAUSE,  LED_OFF);
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

  g_stledInfo.u32TimOut = 2000;
}

void _led_dispBlue(void)
{
  hs_led_setDig(0, 11);
  hs_led_setDig(1, 12);

  hs_led_setFunc(LED_FUNC_COLON, LED_OFF);

  hs_led_setDig(2, 13);
  hs_led_setDig(3, 14);

  if(g_stledInfo.u8BTCon == 0)
    hs_led_setFuncReverse(LED_FUNC_PLAY);
  else
    hs_led_setFunc(LED_FUNC_PLAY, LED_ON);

  hs_led_setFunc(LED_FUNC_MP3, LED_OFF);
  hs_led_setFunc(LED_FUNC_FM,  LED_OFF);

  hs_led_setFunc(LED_FUNC_SD,  LED_OFF);
  hs_led_setFunc(LED_FUNC_USB, LED_OFF);

  hs_led_setFunc(LED_FUNC_PAUSE,  LED_OFF);
}

void _led_dispPh(void)
{
  hs_led_setDig(0, 18);
  hs_led_setDig(1, 15);

  hs_led_setFunc(LED_FUNC_COLON, LED_OFF);

  hs_led_setDig(2, 16);
  hs_led_setDig(3, 18);

  hs_led_setFunc(LED_FUNC_MP3, LED_OFF);
  hs_led_setFunc(LED_FUNC_FM,  LED_OFF);

  hs_led_setFunc(LED_FUNC_SD,  LED_OFF);
  hs_led_setFunc(LED_FUNC_USB, LED_OFF);

  hs_led_setFunc(LED_FUNC_PLAY,   LED_OFF);
  hs_led_setFunc(LED_FUNC_PAUSE,  LED_OFF);
}

void _led_dispRec(void)
{
  hs_led_setDig(0, 18);
  hs_led_setDig(1, 19);

  hs_led_setFunc(LED_FUNC_COLON, LED_OFF);

  hs_led_setDig(2, 14);
  hs_led_setDig(3, 20);

  hs_led_setFunc(LED_FUNC_MP3, LED_OFF);
  hs_led_setFunc(LED_FUNC_FM,  LED_OFF);

  hs_led_setFunc(LED_FUNC_SD,  LED_OFF);
  hs_led_setFunc(LED_FUNC_USB, LED_OFF);

  hs_led_setFunc(LED_FUNC_PLAY,   LED_OFF);
  hs_led_setFunc(LED_FUNC_PAUSE,  LED_OFF);

  g_stledInfo.u32TimOut = 2000;
}

void _led_dispMusic(void)
{
  uint32_t u32Time;
  uint8_t  u8Min, u8Ms;
  uint32_t u32Num = g_stledInfo.u32TimOut >= 1000 ? 1 : (1000 / g_stledInfo.u32TimOut);
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

  if((g_stledInfo.u32Cnt % u32Num) == 0)
    hs_led_setFuncReverse(LED_FUNC_COLON);

  if(MUSIC_DEVICE_SD  == hs_music_getCurDev())
  {
    hs_led_setFunc(LED_FUNC_SD,  LED_ON);
    hs_led_setFunc(LED_FUNC_USB, LED_OFF);
  }
  else if(MUSIC_DEVICE_UDISK  == hs_music_getCurDev())
  {
    hs_led_setFunc(LED_FUNC_SD,  LED_OFF);
    hs_led_setFunc(LED_FUNC_USB, LED_ON);
  }
  else
  {
    hs_led_setFunc(LED_FUNC_SD,  LED_OFF);
    hs_led_setFunc(LED_FUNC_USB, LED_OFF);
  }

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

  g_stledInfo.u32TimOut = 2000;
}

void _led_dispRecTime(void)
{
  uint32_t u32Time;
  uint8_t  u8Min, u8Ms;
  hs_recwr_dev_t eStatus;

  u32Time = hs_recwr_getTime();

  u8Min = u32Time / 60;
  u8Ms  = u32Time % 60;

  hs_led_setDig(0, u8Min / 10);
  hs_led_setDig(1, u8Min % 10);
  
  hs_led_setDig(2, u8Ms / 10);
  hs_led_setDig(3, u8Ms % 10);

  hs_led_setFunc(LED_FUNC_MP3,   LED_OFF);
  hs_led_setFunc(LED_FUNC_FM,    LED_OFF);
  hs_led_setFunc(LED_FUNC_COLON, LED_ON);

  eStatus = hs_recwr_getPos();
  if(eStatus == RECWR_DEVICE_NO)
  {
    hs_led_setFunc(LED_FUNC_SD,  LED_OFF);
    hs_led_setFunc(LED_FUNC_USB, LED_OFF);
  }
  else if(eStatus == RECWR_DEVICE_SD)
  {
    hs_led_setFuncReverse(LED_FUNC_SD);
    hs_led_setFunc(LED_FUNC_USB, LED_OFF);
  }
  else
  {
    hs_led_setFunc(LED_FUNC_SD,  LED_OFF);
    hs_led_setFuncReverse(LED_FUNC_USB);
  }
}


void _led_dispFmChn(uint8_t u8Chn)
{
  hs_led_setDig(0, 18);
  hs_led_setDig(1, 15);

  hs_led_setFunc(LED_FUNC_COLON, LED_OFF);

  hs_led_setDig(2, u8Chn / 10);
  hs_led_setDig(3, u8Chn % 10);

  g_stledInfo.u32TimOut = 2000;
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

  if((FM_STATUS_PLAYING == hs_fm_getStatus()) 
    || (FM_STATUS_SCANNING == hs_fm_getStatus()) )
    hs_led_setFunc(LED_FUNC_PAUSE,  LED_OFF);
  else
    hs_led_setFunc(LED_FUNC_PAUSE,  LED_ON);

  hs_led_setFunc(LED_FUNC_SD,  LED_OFF);
  hs_led_setFunc(LED_FUNC_USB, LED_OFF);
}

void _led_dispAux(void)
{
  hs_led_setDig(0, 18);
  hs_led_setDig(1, 17);

  hs_led_setDig(2, 13);
  hs_led_setDig(3, 16);

  hs_led_setFunc(LED_FUNC_COLON, LED_OFF);
  hs_led_setFunc(LED_FUNC_MP3, LED_OFF);
  hs_led_setFunc(LED_FUNC_FM,  LED_OFF);

  hs_led_setFunc(LED_FUNC_SD,  LED_OFF);
  hs_led_setFunc(LED_FUNC_USB, LED_OFF);

  hs_led_setFunc(LED_FUNC_PLAY,   LED_OFF);

  if(hs_linein_isStarting())
    hs_led_setFunc(LED_FUNC_PAUSE,  LED_OFF);
  else
    hs_led_setFunc(LED_FUNC_PAUSE,  LED_ON);
}

void _led_engineDispMode()
{
  hs_usermode_t eMode;
  uint32_t u32Freq;

  g_stledInfo.u32Cnt += 1;
  eMode = hs_menu_getMode();
  switch(eMode)
  {
    case USER_MODE_BT:
      _led_dispBlue();
      break;

    case USER_MODE_BT_HID:
      _led_dispPh();
      break;

    case USER_MODE_REC:
      _led_dispRecTime();
      break;

    case USER_MODE_MUSIC1:
    case USER_MODE_MUSIC2:
      _led_dispMusic();
      break;

    case USER_MODE_FM:
      u32Freq = hs_fm_getFreq();
      if (u32Freq != 0) 
	  {
        u32Freq >>= 8;
        _led_dispFm(u32Freq);
      }
      break;

    case USER_MODE_AUX:
      _led_dispAux();
      break;

    default:
      _led_dispIdle();
      break;
  }
}

void _led_engineDispInfo(hs_ledengine_t *pstLedInfo, hs_led_dispType_t eDispType)
{
  (void )pstLedInfo;
  uint16_t u16tmp = (uint16_t)eDispType;
  hs_avol_dev_t eDev;
  uint32_t u32Freq;
  
  switch(eDispType & LED_DISP_MASK)
  {
    case LED_DISP_MODE_CHANGE:
      _led_engineDispMode();
      break;

    case LED_DISP_REC:
      _led_dispRec();
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

    case LED_DISP_DIGIT:
      _led_dispDigit();
      break;

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

  _led_dispHi();
  msleep(3000);
  
  while(1)
  {
    eEvt = oshalMessageGet(pstLedInfo->pstMsgId, pstLedInfo->u32TimOut);
    eMsg = (hs_led_dispType_t)eEvt.value.v;
    
    switch(eEvt.status)
    {
    case osEventTimeout:    
      _led_engineDispInfo(pstLedInfo, LED_DISP_MODE_CHANGE);
      pstLedInfo->u32TimOut = 100;

      if(pstLedInfo->u32DigVal != 0)
      {
        hs_printf("Get Value from key: %d\r\n", pstLedInfo->u32DigVal);
        hs_menu_doKey(pstLedInfo->u32DigVal);
        pstLedInfo->u32DigVal = 0;
      }
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

  g_stledInfo.u32TimOut = 1000;
  g_stledInfo.u32DigVal = 0;
  g_stledInfo.u8BTCon   = 0;
  
  stMsgDef.queue_sz = LED_QUEUE_SIZE * sizeof(int);
  stMsgDef.item_sz  = sizeof(int);
  stMsgDef.items    = g_stledInfo.u8Queue;
  g_stledInfo.pstMsgId = oshalMessageCreate(&stMsgDef, NULL);

  stThdDef.pthread   = (os_pthread)_led_engineThread;
  stThdDef.stacksize = 512;
  stThdDef.tpriority = osPriorityRealtime;     
  g_stledInfo.pstThd  = oshalThreadCreate(&stThdDef, &g_stledInfo); 

  hs_cfg_sysListenMsg(HS_CFG_EVENT_INPUT_DIG,  _led_doDigitEvent);  

  hs_cfg_sysListenMsg(HS_CFG_EVENT_BT_CONNECTED,  _led_doBlueStatus); 
  hs_cfg_sysListenMsg(HS_CFG_EVENT_BT_DISCONNECTED,  _led_doBlueStatus); 
  hs_cfg_sysListenMsg(HS_CFG_EVENT_BT_AUDIO_STOP,  _led_doBlueStatus); 
}

void hs_led_disp(hs_led_dispType_t eMsg)
{
  if(g_stledInfo.pstMsgId)
    oshalMessagePut(g_stledInfo.pstMsgId, eMsg, 0);
}

#endif

/** @} */
