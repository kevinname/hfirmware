/*
    audio plaer - Copyright (C) 2012~2016 HunterSun Technologies
                  pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/audio/dec/dec.h
 * @brief   include file.
 * @details 
 *
 * @addtogroup  decoder
 * @details 
 * @{
 */
#ifndef __LIB_AUDIO_DEC_H__
#define __LIB_AUDIO_DEC_H__

#include "ao.h"
#include "ff.h"

#if HS_USE_MP3

typedef enum
{
  ADEC_TYPE_WAV     = 0,
  ADEC_TYPE_MP3     ,
  ADEC_TYPE_WMA     ,

  ADEC_TYPE_NUM     ,
  ADEC_TYPE_UNKNOWN ,
}hs_adectype_t;

enum
{
  ADEC_ERR          = -1,
  ADEC_OK           = 0,
};

typedef void *(*hs_pfnAdecCreate_t)(void);

typedef struct __adecInfoS  hs_adec_t;

struct __adecInfoS
{
  char    flag[24];  

  void   *decoder;  
  void  (*pfnReinit)(hs_adec_t *);
  int   (*pfnSkip)(hs_adec_t *);
  int   (*pfnRun)(hs_adec_t *);
  void  (*pfnDestroy)(hs_adec_t *);

  int   (*pfnFetchData)(hs_adec_t *, uint8_t *, uint32_t);
  int   (*pfnCarryAway)(uint8_t *, uint32_t);

  hs_adectype_t       eAdecType;
  FIL                *pstSrcFile;
  hs_ao_t            *pstAo;
  const uint8_t      *pu8Width;
  uint8_t             u8ffrewS;
  uint32_t            u32Frames;
  osSemaphoreId       pstSemId;  
};


hs_adec_t *hs_adec_creat(hs_adectype_t eType, FIL *pFile, hs_ao_t *pstAo, uint32_t u32Frames);
void hs_adec_destroy(hs_adec_t *pstAdec);
void hs_adec_Reinit(hs_adec_t *pstAdec, FIL *pFile);
int  hs_adec_run(hs_adec_t *pstAdec);
int  hs_adec_skip(hs_adec_t *pstAdec);
int  hs_adec_context(hs_adec_t *pstAdec);


#endif

#endif
