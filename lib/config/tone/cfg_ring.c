/*
    bootloader - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    config/tone/cfg_tone.c
 * @brief   tone manage file.
 * @details 
 *
 * @addtogroup  config
 * @details 
 * @{
 */
#include "lib.h"
#include "string.h"
#include "cfg_audio.h"
#include "cfg_sbc.h"
#include "cfg_ring.h"
#include "cfg_tone.h"

#if HS_USE_TONE

#if HS_USE_BT
extern uint8_t* hsc_HFP_GetCurCallNum(void);
extern uint8_t* hsc_HFP_GetLastCallNum(void);
extern uint16_t hsc_HFP_GetState(void);
extern uint8_t hsc_HFp_GetRingType(void);
extern uint8_t hs_cfg_toneGetIntervalInbandRing(void);

#define BT_HFP_CALL_STATUS_INCOMING    0x0010
#endif

extern const hs_cfg_sound_info_t *hs_cfg_toneGetPhoneNumberInfo(uint16_t u16Idx);
extern hs_cfg_res_t hs_cfg_toneGetPhoneNumberData(uint16_t u16Idx,uint8_t* pu8SbcPtr, uint16_t u16FrmLen,uint32_t u32Offset);

static uint8_t g_u8PhoneCalled = CFG_RING_NOT_CALLED;
static uint8_t g_u8NameRecord;
static thread_t *g_pstRecThrdHand;

static int32_t _cfg_ringFindTone(uint8_t *pu8PhoneNumber, 
                                hs_cfg_ring_info_t *pstRingInfo, 
                                hs_cfg_ring_header_t *pstHeader)
{
  uint16_t i;
  int32_t s32BaseAddr = 0;
  hs_cfg_res_t enRes;

  if (NULL == pu8PhoneNumber)
    return -1;

  for(i=0; i<pstRingInfo->u16SaveCnt; i++)
  {
    enRes = hs_cfg_getPartDataByIndex(HS_CFG_RING_SPACE_BASE, (uint8_t *)pstHeader, sizeof(hs_cfg_ring_header_t), s32BaseAddr);
    if(enRes != HS_CFG_OK)
    {
      return -1;
    }
    
    if(strcmp((char *)pstHeader->u8TelPhone, (char *)pu8PhoneNumber) == 0)
    {
      return s32BaseAddr;
    }

    s32BaseAddr += pstRingInfo->u16OneSize;
  }

  return -1;
}

static void _cfg_ringTonePlay(uint32_t u32SampleRate, int32_t s32ToneAddr, uint32_t u32ValidLen)
{
  uint8_t *pu8SbcPtr, *pu8PcmPtr, u8Interrupt;
  int32_t s32Len;
  uint16_t u16FrmLen, u16PcmLen, u16Tmp;
  uint8_t hdr[2];
  hs_sbc_handle_t Handler;
  hs_cfg_res_t enRes;
  
  s32ToneAddr += sizeof(hs_cfg_ring_header_t);
  u16FrmLen = CFG_RING_SBC_FRAMELEN;
  u16PcmLen = u16FrmLen * SBC_COMPRESS_PROPORTION;
  pu8SbcPtr = (uint8_t *)hs_malloc(u16FrmLen, __MT_GENERAL);
  pu8PcmPtr = (uint8_t *)hs_malloc(u16PcmLen, __MT_GENERAL);

  hs_cfg_toneAudioStart(u32SampleRate, TONE_AUDIO_MODE_STEREO, 60);  
  hs_sbc_build_hdr(hdr, u32SampleRate, CFG_RING_SBC_BLOCKS,
                   CFG_RING_SBC_CHNNUM == 2 ? SBC_MODE_STEREO : SBC_MODE_MONO,
                   SBC_AM_LOUDNESS, CFG_RING_SBC_SUBBANDS, CFG_RING_SBC_BITPOOL);
  Handler = hs_sbc_open(hdr);

  s32Len = u32ValidLen;
  u8Interrupt = 0;
  do
  {
    enRes = hs_cfg_getPartDataByIndex(HS_CFG_RING_SPACE_BASE, pu8SbcPtr, u16FrmLen, s32ToneAddr);
    if(enRes != HS_CFG_OK)
    {
      cfg_dbg("Read sbc data error!");        
      break ;
    }

    u16Tmp = hs_sbc_decode(Handler, pu8SbcPtr, u16FrmLen, pu8PcmPtr, &u16PcmLen);
    if(u16Tmp != u16FrmLen)
    {
      cfg_dbg("Sbc frame length calculate error!");
      break;
    }

    enRes = hs_cfg_toneTxStream(pu8PcmPtr, u16PcmLen);
    if(enRes != HS_CFG_OK)
    {
      cfg_dbg("tone play stopped!");    
      u8Interrupt = 1;
      break ;
    }
      
    s32ToneAddr += u16FrmLen;
    s32Len -= u16FrmLen;
  }while(s32Len > 0);

  cfg_dbg("play over! The last length: 0x%x, total length: 0x%x", s32Len, u32ValidLen);

  hs_cfg_toneAudioStop(u8Interrupt);
  hs_sbc_close(Handler);

  hs_free(pu8PcmPtr);
  hs_free(pu8SbcPtr); 
}

static void _cfg_ringCallTone(hs_cfg_ring_info_t *pstRingInfo)
{
  hs_cfg_ring_header_t stHeader;
  uint8_t *pu8Name = NULL;
  int32_t s32ToneAddr;

  if(pstRingInfo->u16SaveCnt == 0)
  {
    cfg_dbg("no ring tone!");
    return ;
  }

#if HS_USE_BT
  pu8Name = (uint8_t *)hsc_HFP_GetCurCallNum();
#endif
  s32ToneAddr = _cfg_ringFindTone(pu8Name, pstRingInfo, &stHeader);
  if(s32ToneAddr < 0)
  {
    cfg_dbg("have not ring tone for telephone:%s!", pu8Name);
    return ;
  }

  _cfg_ringTonePlay(pstRingInfo->u32SampleRate, s32ToneAddr, stHeader.u32ValidLen);
}

static THD_FUNCTION(_cfg_ringRecThread, arg)
{
  hs_cfg_ring_info_t *pstRingInfo = (hs_cfg_ring_info_t *)arg;
  hs_cfg_ring_header_t stHeader;
  int32_t s32ToneAddr, s32CfgAddr, u32TotalLen;
  uint8_t *pu8Name = NULL, *pu8SbcPtr, *pu8TmpPtr, *pu8PcmPtr;
  uint16_t u16SbcFrmLen, u16PcmFrmLen;
  uint16_t u16Tmp, u16SbcLen, u16RingNum;
  uint8_t hdr[2];
  hs_sbc_handle_t Handler;
  hs_cfg_res_t enRes;

  chRegSetThreadName("ring");
#if HS_USE_BT
  pu8Name = (uint8_t *)hsc_HFP_GetLastCallNum();
#endif
  s32CfgAddr = _cfg_ringFindTone(pu8Name, pstRingInfo, &stHeader);
  if(s32CfgAddr < 0)
  {
    /* a new ring */
    u16RingNum = pstRingInfo->u32SpaceSize / pstRingInfo->u16OneSize;
    if(pstRingInfo->u16SaveCnt >= u16RingNum)
    {
      cfg_dbg("Ring space is full!");
      goto recThreadExit;
    }

    s32CfgAddr = pstRingInfo->u16SaveCnt * pstRingInfo->u16OneSize;
    pstRingInfo->u16SaveCnt ++;
  }

  memcpy(stHeader.u8TelPhone, pu8Name, CFG_RING_CALLPHONE_LEN);  

  cfg_dbg("Record name start!");

  u32TotalLen = 0;
  s32ToneAddr = s32CfgAddr;
  u16SbcFrmLen = CFG_RING_SBC_FRAMELEN;
  u16PcmFrmLen = CFG_RING_PCM_FRAMELEN;
  pu8SbcPtr = (uint8_t *)hs_malloc(pstRingInfo->u16OneSize, __MT_GENERAL);
  pu8PcmPtr = (uint8_t *)hs_malloc(u16PcmFrmLen, __MT_GENERAL);
  pu8TmpPtr = pu8SbcPtr;

  hs_sbc_build_hdr(hdr, pstRingInfo->u32SampleRate, CFG_RING_SBC_BLOCKS,
                   CFG_RING_SBC_CHNNUM == 2 ? SBC_MODE_STEREO : SBC_MODE_MONO,
                   SBC_AM_LOUDNESS, CFG_RING_SBC_SUBBANDS, CFG_RING_SBC_BITPOOL);
  Handler = hs_sbc_open(hdr);
  hs_cfg_toneRecStart(pstRingInfo->u32SampleRate, TONE_AUDIO_MODE_STEREO,
                      audioRecordGetVolumeMax(AUDIO_REC_SRC_RING));

  do
  {
    enRes = hs_cfg_toneRxStream(pu8PcmPtr, u16PcmFrmLen);
    if(enRes != HS_CFG_OK)
    {
      g_u8NameRecord = CFG_RING_NAME_RECORD_STOP;
      hs_cfg_toneRecStop(1);
      hs_sbc_close(Handler);

      hs_free(pu8SbcPtr);
      hs_free(pu8PcmPtr); 

      cfg_dbg("tone record stopped!");  

      goto recThreadExit;
    }

    u16Tmp = hs_sbc_encode(Handler, pu8PcmPtr, u16PcmFrmLen, pu8TmpPtr, u16SbcFrmLen, &u16SbcLen);
    if(u16Tmp != u16PcmFrmLen)
    {
      cfg_dbg("Pcm frame length calculate error!");
    }    

    pu8TmpPtr += u16SbcLen;
    u32TotalLen += u16SbcLen;

    if(u32TotalLen > (pstRingInfo->u16OneSize - u16SbcFrmLen))
    {
      break;
    }

    if(g_u8NameRecord == CFG_RING_NAME_RECORD_STOP)
    {
      break;
    }
  }while(1);

  cfg_dbg("Record name over! its length: 0x%x", u32TotalLen);

  hs_cfg_toneRecStop(0);
  hs_sbc_close(Handler);
  
  stHeader.u32ValidLen = u32TotalLen;
  hs_cfg_setDataByIndex(HS_CFG_RING_SPACE_BASE, (uint8_t *)&stHeader, sizeof(hs_cfg_ring_header_t), s32ToneAddr);

  pu8TmpPtr = pu8SbcPtr;
  do
  {
    if(u32TotalLen > CFG_RING_WRITE_BLOCKSIZE)
    {
      u16Tmp = CFG_RING_WRITE_BLOCKSIZE;
    }
    else
    {
      u16Tmp = u32TotalLen;
    }
    
    hs_cfg_setDataByIndex(HS_CFG_RING_SPACE_BASE, pu8TmpPtr, u16Tmp, s32ToneAddr + sizeof(hs_cfg_ring_header_t));

    u32TotalLen -= u16Tmp;
    s32ToneAddr += u16Tmp;
    pu8TmpPtr += u16Tmp;
  }while(u32TotalLen > 0);  
  
  hs_cfg_setDataByIndex(HS_CFG_RING_INFO, (uint8_t *)pstRingInfo, sizeof(hs_cfg_ring_info_t), 0);  

  g_u8NameRecord = CFG_RING_NAME_RECORD_STOP;  

  hs_free(pu8SbcPtr);
  hs_free(pu8PcmPtr); 

  _cfg_ringTonePlay(pstRingInfo->u32SampleRate, s32CfgAddr, stHeader.u32ValidLen);

  recThreadExit:
  hs_free(pstRingInfo);
  chThdExit(0);
}

void hs_cfg_ringPlay(uint8_t u8Idx, void *parg)
{
  (void)u8Idx;
  (void)parg;
  
  hs_cfg_res_t enRes;
  hs_cfg_ring_info_t stRingInfo;

  enRes = hs_cfg_getDataByIndex(HS_CFG_RING_INFO, (uint8_t *)&stRingInfo, sizeof(hs_cfg_ring_info_t));
  if(enRes != HS_CFG_OK)
  {
    return ;
  }

  if(stRingInfo.u16OneSize <= CFG_RING_CALLPHONE_LEN)
  {
    return ;
  }

  if((hs_cfg_ring_type_t)stRingInfo.u16RingType == CFG_RING_TYPE_NAME)
  {
    _cfg_ringCallTone(&stRingInfo);
  }

  g_u8PhoneCalled = CFG_RING_HAVE_CALLED;
  return ;
}

void hs_cfg_ringRecordStart(uint8_t u8Idx, void *parg)
{
  (void)u8Idx;
  (void)parg;
  
  hs_cfg_res_t enRes;
  hs_cfg_ring_info_t *pstRingInfo;
  
  if(g_u8PhoneCalled != CFG_RING_HAVE_CALLED)
  {
    //return ;
  }

  pstRingInfo = hs_malloc(sizeof(hs_cfg_ring_info_t), __MT_GENERAL);
  if(pstRingInfo == NULL)
  {
    cfg_dbg("have not enough buffer!");
    return ;
  }
  
  enRes = hs_cfg_getDataByIndex(HS_CFG_RING_INFO, (uint8_t *)pstRingInfo, sizeof(hs_cfg_ring_info_t));
  if(enRes != HS_CFG_OK)
  {
    hs_free(pstRingInfo);
    return ;
  }

  if((hs_cfg_ring_type_t)pstRingInfo->u16RingType == CFG_RING_TYPE_NAME)
  {
    g_u8NameRecord = CFG_RING_NAME_RECORD_START;
    g_pstRecThrdHand = chThdCreateFromHeap(NULL, 4096, CFG_RING_REC_THREAD_PRIO, _cfg_ringRecThread, pstRingInfo);
  }

  g_u8PhoneCalled = CFG_RING_NOT_CALLED;
  return ;
}

void hs_cfg_ringRecordStop(uint8_t u8Idx, void *parg)
{
  (void)u8Idx;
  (void)parg;
  
  g_u8NameRecord = CFG_RING_NAME_RECORD_STOP;

  while(g_pstRecThrdHand)
  {
    if(chThdTerminatedX(g_pstRecThrdHand)) 
    {
      chThdRelease(g_pstRecThrdHand);    
      g_pstRecThrdHand = NULL; 
    }

    msleep(1);
  }
  
  cfg_dbg("record process over!");
}

//-----------------------------Incoming Call Feature------------------------------
#define PHONE_INCOMING_CALL_EVENT 1
static thread_t *g_pstIncomingCallThrdHand=NULL;
static uint8_t _sco_link_exist=0x00;
  
static void _SetScoLinkExitFlag(uint16_t u16Msg, void *parg)
{
  (void) u16Msg;
  _sco_link_exist = (uint32_t)parg;
}

static void _cfg_ringIncomingCallPlay(void)
{
  uint16_t u16Subbands, u16ChnNum, u16Bitpool, u16Join, u16Block;
  uint16_t u16FrmLen, u16PcmLen, u16Tmp;
  int16_t s16Len;
  hs_cfg_res_t enRes;
  uint8_t *pu8SbcPtr, *pu8PcmPtr, u8AudioMode, u8Interrupt=0x00;
  uint8_t hdr[2];
  hs_sbc_handle_t Handler;
  uint32_t u32TmpAddr;
  
  uint32_t j;
  const hs_cfg_sound_info_t *pstSndInfo;
  uint16_t u16PhoneNumIdx;
  uint8_t *pu8PhoneNumber;
  
#if HS_USE_BT
  pu8PhoneNumber = (uint8_t *)hsc_HFP_GetCurCallNum();
  if(!pu8PhoneNumber) 
  {
    return;
  }
#else
  return ;
#endif
  //TODO, check phone number length.

  pstSndInfo = hs_cfg_toneGetPhoneNumberInfo(0);    //SBC code of the phone number are are same 
  u16Subbands = pstSndInfo->u8SubBands;
  u16Bitpool = pstSndInfo->u8BitPool;
  u16Block = pstSndInfo->u8Blocks;
  u16ChnNum = 2;
  u16Join = 0;

  u8AudioMode = TONE_AUDIO_MODE_STEREO;
  if(pstSndInfo->u8ChnMode == CFG_SBC_CHNMODE_MONO)
  {
    u16ChnNum = 1;
    u8AudioMode = TONE_AUDIO_MODE_MONO;
  }

  if(pstSndInfo->u8ChnMode == CFG_SBC_CHNMODE_JOINTSTEREO)
  {
    u16Join = 1;
  }

  if(pstSndInfo->u8ChnMode >= CFG_SBC_CHNMODE_STEREO)
  {
    u16Tmp = u16Join * u16Subbands + u16Block * u16Bitpool;
    u16Tmp = (u16Tmp + 7) / 8;
  }
  else
  {
    u16Tmp = u16Block * u16ChnNum * u16Bitpool;
    u16Tmp = (u16Tmp + 7) / 8;
  }

  u16FrmLen = 4 + (4 * u16Subbands * u16ChnNum) / 8 + u16Tmp;
  u16PcmLen = u16FrmLen * SBC_COMPRESS_PROPORTION;

  hs_cfg_sysListenMsg(HS_CFG_EVENT_BT_SCO_STATUS, _SetScoLinkExitFlag);
  _sco_link_exist = 1;
  

  pu8SbcPtr = (uint8_t *)hs_malloc(u16FrmLen, __MT_GENERAL);
  if(pu8SbcPtr == NULL)return;
  pu8PcmPtr = (uint8_t *)hs_malloc(u16PcmLen, __MT_GENERAL);
  if(pu8PcmPtr == NULL) 
  {
    hs_free(pu8SbcPtr);
    return;
  }
  
  u8Interrupt = 0; 
  hs_cfg_toneAudioStart(pstSndInfo->u32BitRate, u8AudioMode, pstSndInfo->s16ToneVolume);  
  hs_sbc_build_hdr(hdr, pstSndInfo->u32BitRate, pstSndInfo->u8Blocks, pstSndInfo->u8ChnMode, SBC_AM_LOUDNESS, pstSndInfo->u8SubBands, pstSndInfo->u8BitPool);
  Handler = hs_sbc_open(hdr);
  if(Handler==NULL) 
  {
    hs_printf("SBC handler NULL!\r\n");
    goto playexit;
  }

    
  while (1)    //loop play 
  {
    for(j=0x00;j<(strlen((const char*)pu8PhoneNumber)+2);j++)     //traversal all phone number
    {
      boardKickWatchDog();
      
#if HS_USE_BT
  	  if((hsc_HFP_GetState() != BT_HFP_CALL_STATUS_INCOMING) || (_sco_link_exist==0))
      {
        goto playexit;
      }
#endif
      if(j<strlen((const char*)pu8PhoneNumber))
      {
        if((pu8PhoneNumber[j]<'0') || (pu8PhoneNumber[j]>'9'))
        {
          cfg_dbg("phone number error, not in the scope [0-9]!");
          continue;    //play next number.
        }
        u16PhoneNumIdx = pu8PhoneNumber[j]-'0';
      }
      else
      {
        u16PhoneNumIdx = 10;    //ring
      }
      
      u32TmpAddr = 0x00;
      s16Len = hs_cfg_toneGetPhoneNumberInfo(u16PhoneNumIdx)->u16DataLen;
      do
      {
        enRes = hs_cfg_toneGetPhoneNumberData(u16PhoneNumIdx, pu8SbcPtr, u16FrmLen, u32TmpAddr);
        if(enRes != HS_CFG_OK)
        {
          cfg_dbg("Read sbc data error!");  
          //break;
          goto playexit;
        }

        /* send the data to decode and then playing */
        u16Tmp = hs_sbc_decode(Handler, pu8SbcPtr, u16FrmLen, pu8PcmPtr, &u16PcmLen);
        if(u16Tmp != u16FrmLen)
        {
          cfg_dbg("Sbc frame length calculate error!");
        }
        enRes = hs_cfg_toneTxStream(pu8PcmPtr, u16PcmLen);
        if(enRes != HS_CFG_OK)
        {
          cfg_dbg("tone play stopped!");
          u8Interrupt = 1;
          //break ;
          goto playexit;
        }
        u32TmpAddr += u16FrmLen;
        s16Len -= u16FrmLen;
      }while(s16Len > 0);
      msleep((pstSndInfo->u16Delay * 1000));
    }
    
#if HS_USE_BT
    if(hsc_HFP_GetState()==BT_HFP_CALL_STATUS_INCOMING)
    {
      if(hsc_HFp_GetRingType())
      {    
        msleep(hs_cfg_toneGetIntervalInbandRing()*1000);    //sleep 5 s.
      }
    }
    else
#endif
    {
      break;
    }

  }

playexit:
  hs_cfg_toneAudioStop(u8Interrupt);
  hs_sbc_close(Handler);
  hs_free(pu8SbcPtr);
  hs_free(pu8PcmPtr);
    
  hs_cfg_sysCancelListenMsg(HS_CFG_EVENT_BT_SCO_STATUS, _SetScoLinkExitFlag);
}


static THD_FUNCTION(_cfg_ringIncomingCallThread, arg)
{
  (void)arg;
  chRegSetThreadName("incomingcall");
  
    #if HAL_USE_WATCHDOG
    wdt_keepalive();
    #endif
    //if(g_pstIncomingCallThrdHand!=NULL)
    {
      oshalSignalWait(PHONE_INCOMING_CALL_EVENT, 0);    //wait PHONE_INCOMING_CALL_EVENT event, wait infinite
      _cfg_ringIncomingCallPlay();
      oshalSignalClear(g_pstIncomingCallThrdHand,PHONE_INCOMING_CALL_EVENT);
    }

    //TODO   kill the thread.
    chThdExit(0);
  
}


#define CFG_RING_INCOMING_CALL_THREAD_PRIO  (NORMALPRIO + 10)

void hs_cfg_ringIncomingCallStart(uint16_t u8Idx, void *parg)
{
  (void)u8Idx;
  (void)parg; 
  g_pstIncomingCallThrdHand = chThdCreateFromHeap(NULL, 1024, CFG_RING_INCOMING_CALL_THREAD_PRIO, _cfg_ringIncomingCallThread, NULL);
  oshalSignalSet(g_pstIncomingCallThrdHand, PHONE_INCOMING_CALL_EVENT);    //ringIncomingCall.
}

void hs_cfg_ringIncomingCallStop(uint16_t u8Idx, void *parg)
{
  (void)u8Idx;
  (void)parg; 

  while(g_pstIncomingCallThrdHand)
  {
    #if HAL_USE_WATCHDOG
    wdt_keepalive();
    #endif
    if(chThdTerminatedX(g_pstIncomingCallThrdHand)) 
    {
      chThdRelease(g_pstIncomingCallThrdHand);    
      g_pstIncomingCallThrdHand = NULL; 
    }
    msleep(1);
  }
}




#endif /* HS_USE_TONE */

/** @} */
