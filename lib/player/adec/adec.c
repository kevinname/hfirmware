/*
    audio player - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/audio/dec/dec.c
 * @brief   source file.
 * @details 
 *
 * @addtogroup  decoder
 * @details 
 * @{
 */

#include "lib.h"
#include "mp3common.h"


#if HS_USE_MP3

static const uint8_t g_u8BitWidth[] = 
{
  0, 12, 16, 20, 24, 32, 64, 128, 192
};

static const hs_pfnAdecCreate_t g_pfnAdecCreate[] =
{
  (hs_pfnAdecCreate_t)hs_adec_wavCreate,
  (hs_pfnAdecCreate_t)hs_adec_mp3Create,
  #if PLAYER_INC_WMA
  (hs_pfnAdecCreate_t)hs_adec_wmaCreate,
  #endif
};

static int _adec_fetchData(hs_adec_t *pstAdec, uint8_t *u8Buf, uint32_t u32Len)
{
  int s32ReadLen;
  
  if((!pstAdec) || (!pstAdec->pstSrcFile) || (!u8Buf) || (u32Len == 0))
    return 0;

  oshalSemaphoreWait(pstAdec->pstSemId, -1);
  if(FR_OK != f_read(pstAdec->pstSrcFile, u8Buf, u32Len, (UINT *) &s32ReadLen))
  {
    oshalSemaphoreRelease(pstAdec->pstSemId);
    return 0;
  }

  oshalSemaphoreRelease(pstAdec->pstSemId);
  return s32ReadLen;
}

hs_adec_t *hs_adec_creat(hs_adectype_t eType, FIL *pFile, hs_ao_t *pstAo, uint32_t u32Frames)
{
  hs_adec_t *pstAdec;
  osSemaphoreDef_t semdef;  
  
  if(eType >= ADEC_TYPE_NUM)
    return NULL;

  pstAdec = (*g_pfnAdecCreate[eType])();
  if(!pstAdec)
    return NULL;

  pstAdec->pstSemId = oshalSemaphoreCreate(&semdef, 1);
  if(!pstAdec->pstSemId)
  {
    hs_adec_destroy(pstAdec);
    return NULL;
  }

  pstAdec->eAdecType  = eType;
  pstAdec->pstSrcFile = pFile; 
  pstAdec->pstAo      = pstAo;
  pstAdec->pu8Width   = g_u8BitWidth;
  pstAdec->u32Frames  = (u32Frames >= 0x80000000) ? 0 : u32Frames;

  pstAdec->pfnFetchData  = _adec_fetchData;
  pstAdec->pfnCarryAway  = hs_ao_fetchData;  

  return pstAdec;
}

void hs_adec_destroy(hs_adec_t *pstAdec)
{
  if((!pstAdec) || (!pstAdec->pfnDestroy))
    return ;
  
  pstAdec->pfnDestroy(pstAdec);
}

void hs_adec_Reinit(hs_adec_t *pstAdec, FIL *pFile)
{
  if((!pstAdec) || (!pstAdec->pfnReinit))
    return ;

  pstAdec->pfnReinit(pstAdec);
  pstAdec->pstSrcFile = pFile;
  pstAdec->u32Frames  = 0;
}

int hs_adec_run(hs_adec_t *pstAdec)
{
  if((!pstAdec) || (!pstAdec->pfnRun))
    return ADEC_ERR;
  
  return pstAdec->pfnRun(pstAdec);
}

int hs_adec_skip(hs_adec_t *pstAdec)
{
  if((!pstAdec) || (!pstAdec->pfnSkip))
    return ADEC_ERR;
  
  return pstAdec->pfnSkip(pstAdec);
}

int hs_adec_context(hs_adec_t *pstAdec)
{
  if((!pstAdec) || (!pstAdec->pfnSkip))
    return ADEC_ERR;
  
  return pstAdec->u32Frames;
}

void hs_adec_ffRew(hs_adec_t *pstAdec, uint32_t u32Frames)
{
  hs_adecmp3_t* decoder = (hs_adecmp3_t *)pstAdec->decoder;
  hs_adecwav_t * pstAdecWav = (hs_adecwav_t *)pstAdec->decoder;
  MP3DecInfo *pstDecInfo = (MP3DecInfo *)decoder->decoder;
  
  if((!pstAdec) || (!pstAdec->pstSrcFile))
    return ;

  oshalSemaphoreWait(pstAdec->pstSemId, -1);

  if(u32Frames > f_size(pstAdec->pstSrcFile))
  {
    oshalSemaphoreRelease(pstAdec->pstSemId);
    return ;
  }
  
  if(FR_OK != f_lseek(pstAdec->pstSrcFile, u32Frames))
  {
    oshalSemaphoreRelease(pstAdec->pstSemId);
    return ;
  }

  pstAdec->u32Frames = u32Frames; 

  if(pstAdec->eAdecType == ADEC_TYPE_MP3)
  {
    decoder->frames = u32Frames;
    decoder->ms = u32Frames / (pstDecInfo->bitrate / 8) * 1000;
  }
  else
  {
    pstAdecWav->frames = u32Frames;
  }

  oshalSemaphoreRelease(pstAdec->pstSemId);
}

void hs_adec_ffRewExit(hs_adec_t *pstAdec)
{
  if(!pstAdec)
    return ;
}


#endif



