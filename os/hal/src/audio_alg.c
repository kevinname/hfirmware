/*
    ChibiOS/RT - Copyright (C) 2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    audio_alg.c
 * @brief   audio algorithm of audio system .
 *
 * @addtogroup audio
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "string.h"
#include "chprintf.h"

#if HAL_USE_AUDIO || defined(__DOXYGEN__)

#define ALG_FUN_AEC_EN      (1 << 0)
#define ALG_FUN_ANS_EN      (1 << 1)

#define ALG_AEC_BUF_LEN     (1024 * 12)
#define ALG_ANS_UNIT_LEN    120

typedef struct
{
  uint32_t        u32RestLen;
  
  uint8_t         u8TxBuf[ALG_AEC_BUF_LEN];
  uint32_t        u32TxWptr;
  uint32_t        u32TxRptr;

  pfnAlgAec_t     pfnAecCal;
  pfnAlgInit_t    pfnAecInit;
  pfnAlgUninit_t  pfnAecUninit;
}hs_alg_aec_t;

typedef struct
{
  uint32_t        u32UnitLen;
  
  pfnAlgAns_t     pfnAnsCal;
  pfnAlgInit_t    pfnAnsInit;
  pfnAlgUninit_t  pfnAnsUninit;

  uint8_t         u8RestBuf[ALG_ANS_UNIT_LEN];
  uint32_t        u32RestLen;
}hs_alg_ans_t;

typedef struct
{
  uint32_t      u32FunMap;
  
  hs_alg_aec_t *pstAec;
  hs_alg_ans_t *pstAns;
}hs_alg_t;

static hs_alg_t g_stAlg;

#define __malloc(size)      chHeapAlloc(NULL, size)
#define __mfree(ptr)        chHeapFree((void *)ptr)

uint32_t _audio_frame2byte(hs_audio_stream_t *stmp, uint32_t frame);
int32_t _audio_record_avail(hs_audio_stream_t *stmp);
int32_t _audio_play_avail(hs_audio_stream_t *stmp);

void hs_audio_algInit(void)
{
  memset(&g_stAlg, 0, sizeof(hs_alg_t));
}

int hs_audio_algAecStart(hs_audio_t *pstAudio, pfnAlgInit_t pfnAecInit, 
                    pfnAlgAec_t pfnAecCal, pfnAlgUninit_t pfnAecUninit)
{
  hs_alg_aec_t *pstAec;
  
  if ((ALG_FUN_AEC_EN & g_stAlg.u32FunMap) || (!pfnAecCal))
    return -1;

  if ((pstAudio->ply.state < AUDIO_PREPARE) || (pstAudio->rec.state < AUDIO_RUNING))
    return -2;

  if(g_stAlg.pstAec)
  {
    __mfree(g_stAlg.pstAec);
    g_stAlg.pstAec = NULL;
  }

  pstAec = (hs_alg_aec_t *)__malloc(sizeof(hs_alg_aec_t));
  if(!pstAec)
    return -3;

  memset(pstAec, 0, sizeof(hs_alg_aec_t));
  pstAec->pfnAecInit   = pfnAecInit;
  pstAec->pfnAecCal    = pfnAecCal;
  pstAec->pfnAecUninit = pfnAecUninit;

  if(pfnAecInit)
    pfnAecInit();

  chSysLock();
  if(pstAudio->ply.state == AUDIO_PREPARE)
    pstAec->u32RestLen = _audio_frame2byte(&pstAudio->ply, pstAudio->ply.start_threshold);
  else
  {
    audio_update_hw_ptr(&pstAudio->ply, 0);
    pstAec->u32RestLen = _audio_frame2byte(&pstAudio->ply, _audio_play_avail(&pstAudio->ply));
  }

  audio_update_hw_ptr(&pstAudio->rec, 0);
  pstAec->u32RestLen += _audio_frame2byte(&pstAudio->rec, _audio_record_avail(&pstAudio->rec));
  pstAec->u32RestLen += 5 * 2;

  g_stAlg.pstAec     = pstAec;
  g_stAlg.u32FunMap |= ALG_FUN_AEC_EN;
  
  chSysUnlock();
  hs_printf("aec start!\r\n");
  return 0;
}

void hs_audio_algAecStop(void)
{
  if (!(ALG_FUN_AEC_EN & g_stAlg.u32FunMap))
    return ;

  g_stAlg.u32FunMap &= ~ALG_FUN_AEC_EN;

  if(g_stAlg.pstAec)
  {
    if(g_stAlg.pstAec->pfnAecUninit)
      g_stAlg.pstAec->pfnAecUninit();
    
    __mfree(g_stAlg.pstAec);
    g_stAlg.pstAec = NULL;
  }  
}

int hs_audio_algAecSaveTxData(uint8_t *pbuf, uint32_t len)
{
  uint8_t *u8Ptr;
  uint32_t tmpLen;
  
  if((!g_stAlg.pstAec) || ((g_stAlg.u32FunMap & ALG_FUN_AEC_EN) == 0))
    return -1;

  u8Ptr = g_stAlg.pstAec->u8TxBuf + g_stAlg.pstAec->u32TxWptr;
  tmpLen = (g_stAlg.pstAec->u32TxWptr + len) > ALG_AEC_BUF_LEN ? 
                   ALG_AEC_BUF_LEN - g_stAlg.pstAec->u32TxWptr : len;

  g_stAlg.pstAec->u32TxWptr = (g_stAlg.pstAec->u32TxWptr + len) % ALG_AEC_BUF_LEN;
  
  memcpy(u8Ptr, pbuf, tmpLen);
  len -= tmpLen;
  if(len)
    memcpy(g_stAlg.pstAec->u8TxBuf, pbuf + tmpLen, len);
  
  return 0;
}

int hs_audio_algAecDoData(uint8_t *pbuf, uint32_t len)
{
  short *s16TxPtr, *s16RxPtr;
  uint32_t i;
  
  if(((g_stAlg.u32FunMap & ALG_FUN_AEC_EN) == 0) 
    || (!pbuf) || (!g_stAlg.pstAec) || (!g_stAlg.pstAec->pfnAecCal))
    return -1;

  if(g_stAlg.pstAec->u32RestLen > 0)
  {
    if(g_stAlg.pstAec->u32RestLen < len)
    {
      len  -= g_stAlg.pstAec->u32RestLen;
      pbuf += g_stAlg.pstAec->u32RestLen;
      g_stAlg.pstAec->u32RestLen = 0;
    }
    else
    {
      g_stAlg.pstAec->u32RestLen -= len;
      return 0;
    }
  }
  
  len /= 2;
  s16RxPtr = (short *)pbuf;

  for(i=0; i<len; i++)
  {
    s16TxPtr = (short *)&g_stAlg.pstAec->u8TxBuf[g_stAlg.pstAec->u32TxRptr];
    s16RxPtr[i] = g_stAlg.pstAec->pfnAecCal(s16TxPtr[0], s16RxPtr[i]);

    g_stAlg.pstAec->u32TxRptr += 2;
    g_stAlg.pstAec->u32TxRptr %= ALG_AEC_BUF_LEN;
  }

  return 0;
}


int hs_audio_algAnsStart(hs_audio_t *pstAudio, pfnAlgInit_t pfnAnsInit, 
                    pfnAlgAns_t pfnAnsCal, pfnAlgUninit_t  pfnAnsUninit)
{
  hs_alg_ans_t *pstAns;
  
  if ((ALG_FUN_ANS_EN & g_stAlg.u32FunMap) || (!pfnAnsCal))
    return -1;

  if(pstAudio->rec.state < AUDIO_RUNING)
    return -2;

  if(g_stAlg.pstAns)
  {
    __mfree(g_stAlg.pstAns);
    g_stAlg.pstAns = NULL;
  }

  pstAns = (hs_alg_ans_t *)__malloc(sizeof(hs_alg_ans_t));
  if(!pstAns)
    return -3;

  pstAns->pfnAnsInit   = pfnAnsInit;
  pstAns->pfnAnsCal    = pfnAnsCal;
  pstAns->pfnAnsUninit = pfnAnsUninit;

  if(pfnAnsInit)
    pfnAnsInit();

  g_stAlg.pstAns     = pstAns;
  g_stAlg.u32FunMap |= ALG_FUN_ANS_EN;
  hs_printf("ans start!\r\n");
  
  return 0;
}

void hs_audio_algAnsStop(void)
{
  if (!(ALG_FUN_ANS_EN & g_stAlg.u32FunMap))
    return ;

  g_stAlg.u32FunMap &= ~ALG_FUN_ANS_EN;

  if(g_stAlg.pstAns)
  {
    if(g_stAlg.pstAns->pfnAnsUninit)
      g_stAlg.pstAns->pfnAnsUninit();
    
    __mfree(g_stAlg.pstAns);
    g_stAlg.pstAns = NULL;
  }  
}

uint32_t hs_audio_algAnsDoData(uint8_t *pbuf, uint32_t len)
{
  hs_alg_ans_t *pstAns = g_stAlg.pstAns;
  uint32_t tlen = len;
  
  if(((g_stAlg.u32FunMap & ALG_FUN_ANS_EN) == 0) 
    || (!pbuf) || (!pstAns) || (!pstAns->pfnAnsCal))
    return len;

  if(len < ALG_ANS_UNIT_LEN)
    return 0;

  do
  {    
    pstAns->pfnAnsCal((short *)pbuf, (short *)pstAns->u8RestBuf);
    memcpy(pbuf, pstAns->u8RestBuf, ALG_ANS_UNIT_LEN);

    len  -= ALG_ANS_UNIT_LEN;
    pbuf += ALG_ANS_UNIT_LEN;
  }while(len >= ALG_ANS_UNIT_LEN);

  if(len != 0)
    while(1);

  tlen -= len;

  return tlen;
}

#endif

/** @} */
