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
#include "cfg_audio.h"
#include "cfg_sbc.h"
#include "cfg_tone.h"

#if HS_USE_TONE

static uint8_t g_u8ToneInt;

const hs_cfg_sound_info_t *hs_cfg_toneGetDefaultInfo(uint16_t u16Idx);
hs_cfg_res_t hs_cfg_toneGetDefaultData(uint16_t u16Idx, uint8_t *pu8SbcPtr, uint16_t u16FrmLen, uint32_t u32Offset);
uint32_t hs_cfg_getFastEvent(void);

static uint32_t _cfg_soundGetOffset(hs_cfg_tone_info_t *pstToneInfo, uint16_t u16Idx)
{
  uint32_t u32Tmp, u32Addr = 0;

  u32Tmp = sizeof(hs_cfg_sound_info_t) + SIZE_4K;  
  if(u16Idx < pstToneInfo->u8Sound4kNum)
  {
    u32Addr += u32Tmp * u16Idx;
    return u32Addr;
  }

  u32Addr = u32Tmp * pstToneInfo->u8Sound4kNum;

  u32Tmp = sizeof(hs_cfg_sound_info_t) + SIZE_8K; 
  u16Idx -= pstToneInfo->u8Sound4kNum;
  if(u16Idx < pstToneInfo->u8Sound8kNum)
  {    
    u32Addr += u32Tmp * u16Idx;
    return u32Addr;
  }

  u32Addr += u32Tmp * pstToneInfo->u8Sound8kNum;

  u32Tmp = sizeof(hs_cfg_sound_info_t) + SIZE_16K; 
  u16Idx -= pstToneInfo->u8Sound8kNum;
  if(u16Idx < pstToneInfo->u8Sound16kNum)
  {    
    u32Addr += u32Tmp * u16Idx;
    return u32Addr;
  }

  return CFG_TONE_ERROR_ADDR;
}

static void _cfg_soundPlay(const hs_cfg_sound_info_t *pstSndInfo, uint32_t u32SndAddr, uint16_t u16Idx)
{
  uint16_t u16Subbands, u16ChnNum, u16Bitpool, u16Join, u16Block;
  uint16_t u16FrmLen, u16PcmLen, u16Tmp;
  int16_t i, s16Len;
  hs_cfg_res_t enRes;
  uint8_t *pu8SbcPtr, *pu8PcmPtr, u8AudioMode, u8Interrupt;
  uint8_t hdr[2];
  hs_sbc_handle_t Handler;
  uint32_t u32TmpAddr;

  if(!pstSndInfo)
    return ;

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
  u32SndAddr += sizeof(hs_cfg_sound_info_t);
  pu8SbcPtr = (uint8_t *)hs_malloc(u16FrmLen, __MT_GENERAL);
  pu8PcmPtr = (uint8_t *)hs_malloc(u16PcmLen, __MT_GENERAL);
  u8Interrupt = 0;

  hs_cfg_toneAudioStart(pstSndInfo->u32BitRate, u8AudioMode, pstSndInfo->s16ToneVolume);  
  hs_sbc_build_hdr(hdr, pstSndInfo->u32BitRate, pstSndInfo->u8Blocks, pstSndInfo->u8ChnMode, SBC_AM_LOUDNESS, pstSndInfo->u8SubBands, pstSndInfo->u8BitPool);
  Handler = hs_sbc_open(hdr);
  if(!Handler)
  {
    hs_cfg_toneAudioStop(u8Interrupt);
    hs_free(pu8PcmPtr);
    hs_free(pu8SbcPtr);
    return ;
  }

  for(i=0; i<pstSndInfo->u16LoopNum; i++)
  {
    u32TmpAddr = (u16Idx & CFG_TONE_DEFAULT_MASK) ? 0 : u32SndAddr;
    s16Len = pstSndInfo->u16DataLen;
    do
    {
      if(u16Idx & CFG_TONE_DEFAULT_MASK)
      {
        enRes = hs_cfg_toneGetDefaultData(u16Idx, pu8SbcPtr, u16FrmLen, u32TmpAddr);
      }
      else
      {
        enRes = hs_cfg_getPartDataByIndex(HS_CFG_SOUND_INFO, pu8SbcPtr, u16FrmLen, u32TmpAddr);        
      }

      if(enRes != HS_CFG_OK)
      {
        cfg_dbg("Read sbc data error!");        
        break ;
      }

      boardKickWatchDog();
      if((hs_cfg_getFastEvent() != 0) && (g_u8ToneInt == 1))
        break ;

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
        break ;
      }
      
      u32TmpAddr += u16FrmLen;
      s16Len -= u16FrmLen;
    }while(s16Len > 0);

    msleep((pstSndInfo->u16Delay * 1000));
  }

  hs_cfg_toneAudioStop(u8Interrupt);
  hs_sbc_close(Handler);

  hs_free(pu8PcmPtr);
  hs_free(pu8SbcPtr);
}

static void _cfg_soundDoEvent(hs_cfg_tone_info_t *pstToneInfo, hs_cfg_mess_type_t m_type, uint16_t message)
{
  hs_cfg_sound_event_t stSndEvent;
  hs_cfg_sound_info_t  stSndInfo;
  hs_cfg_res_t enRes;
  uint32_t u32SndAddr;
  uint16_t i, u16Offset, u16Idx;

  for(i=0; i<pstToneInfo->u8SndBindEvent; i++)
  {
    u16Offset = i * sizeof(hs_cfg_sound_event_t);
    enRes = hs_cfg_getPartDataByIndex(HS_CFG_EVENT_BIND_SOUND, (uint8_t *)&stSndEvent, sizeof(hs_cfg_sound_event_t), u16Offset);
    if(enRes != HS_CFG_OK)
    {
      return ;
    }

    if((stSndEvent.u16Message == message) && (stSndEvent.u8MessType == m_type))
    {
      break;
    }
  }

  if(i >= pstToneInfo->u8SndBindEvent)
  {
    cfg_dbg("have not found the sound tone! type:0x%x, message:0x%x.", m_type, message);
    return ;
  }

  if(stSndEvent.u16ToneIndex & CFG_TONE_DEFAULT_MASK)
  {
    u16Idx = stSndEvent.u16ToneIndex & ~CFG_TONE_DEFAULT_MASK;    
    _cfg_soundPlay(hs_cfg_toneGetDefaultInfo(u16Idx), 0, stSndEvent.u16ToneIndex);
  }
  else
  {
    u32SndAddr = _cfg_soundGetOffset(pstToneInfo, stSndEvent.u16ToneIndex);
    if(u32SndAddr == CFG_TONE_ERROR_ADDR)
    {
      cfg_dbg("get the sound offset error!");
      return ;
    }

    enRes = hs_cfg_getPartDataByIndex(HS_CFG_SOUND_INFO, (uint8_t *)&stSndInfo, sizeof(hs_cfg_sound_info_t), u32SndAddr);
    if(enRes != HS_CFG_OK)
    {
      cfg_dbg("get the sound info error!");
      return ;
    }

    _cfg_soundPlay(&stSndInfo, u32SndAddr, stSndEvent.u16ToneIndex);
  }
}

hs_cfg_res_t hs_cfg_toneDoEvent(hs_cfg_mess_type_t m_type, uint16_t message, uint8_t inte)
{
  hs_cfg_res_t enRes;
  hs_cfg_tone_info_t stToneInfo;

  if(message == 0)
  {
    return HS_CFG_OK;
  }

  enRes = hs_cfg_getDataByIndex(HS_CFG_TONE_INFO, (uint8_t *)&stToneInfo, sizeof(hs_cfg_tone_info_t));
  if(enRes != HS_CFG_OK)
  {
    return enRes;
  }

  if(stToneInfo.u8ToneEnable)
  {
    g_u8ToneInt = inte;
    _cfg_soundDoEvent(&stToneInfo, m_type, message);
  }
  
  return HS_CFG_OK;
}

#endif /* HS_USE_TONE */

/** @} */
