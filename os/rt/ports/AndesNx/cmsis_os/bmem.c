/*
    bootloader - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/mem/lib_bmem.c
 * @brief   bus memory manager.
 * @details 
 *
 * @addtogroup  lib
 * @details 
 * @{
 */

#include "bmem.h"
#include "ch.h"

#define BMEM_SIZE           (32*1024)
#define BMEM_UNIT           128
#define BMEM_MAPUNIT        32
#define BMEM_BITSIZE        (BMEM_SIZE / BMEM_UNIT)
#define BMEM_MAPSIZE        (BMEM_BITSIZE / BMEM_MAPUNIT)

typedef struct
{
  uint32_t u32Addr;
  uint32_t u32Cnt;
}hs_bmem_t;

static uint32_t g_u32BMemMap[BMEM_MAPSIZE];

static void *_bmem_alloc(uint16_t u16Size)
{
  hs_bmem_t *pstInfo;
  uint16_t u16Cnt, u16Os, u16Bit;
  uint16_t u16Start, u16Idle;
  uint32_t i;
  
  if(u16Size >= BMEM_SIZE)
    return NULL;

  u16Start = 0xffff;
  u16Idle = 0;
  u16Cnt = (sizeof(hs_bmem_t) + u16Size + BMEM_UNIT - 1) / BMEM_UNIT;
  for(i=0; i<BMEM_BITSIZE; i++)
  {
    u16Os  = i/BMEM_MAPUNIT;
    u16Bit = i%BMEM_MAPUNIT;

    if((g_u32BMemMap[u16Os] & (1u << u16Bit)) == 0)
    {
      if(u16Start == 0xffff) 
      {
          u16Start = i;
          u16Idle = 0;
      }

      u16Idle ++;
    }
    else
    {
      u16Start = 0xffff;
    }

    if(u16Idle >= u16Cnt)
      break;
  }

  if((u16Start == 0xffff) || (u16Idle < u16Cnt))
    return NULL;

  for(i=u16Start; i<(u16Start+u16Cnt); i++)
  {
    u16Os  = i/BMEM_MAPUNIT;
    u16Bit = i%BMEM_MAPUNIT;

    g_u32BMemMap[u16Os] |= 1u << u16Bit;
  }

  pstInfo = (hs_bmem_t *)(BMEM_BASE + u16Start * BMEM_UNIT);
  pstInfo->u32Addr = (uint32_t)pstInfo;
  pstInfo->u32Cnt = u16Cnt;
  pstInfo++;

  return (void *)pstInfo;
}

void _bmem_free(void *p)
{
  hs_bmem_t *pstInfo = (hs_bmem_t *)p;
  uint16_t i, u16Start;

  pstInfo --;
  if(pstInfo->u32Addr != (uint32_t)pstInfo)
    return ;

  u16Start = (pstInfo->u32Addr - BMEM_BASE) / BMEM_UNIT;
  for(i=u16Start; i<(u16Start+pstInfo->u32Cnt); i++)
  {
    g_u32BMemMap[i/BMEM_MAPUNIT] &= ~(1u << (i%BMEM_MAPUNIT));
  }
}

void *osBmemAllocS(uint16_t u16Size)
{
  return _bmem_alloc(u16Size);
}

void osBmemFreeS(void *p)
{
  _bmem_free(p);
}

void *osBmemAlloc(uint16_t u16Size)
{
  syssts_t sts;
  void *p;

  sts = chSysGetStatusAndLockX();
  p = _bmem_alloc(u16Size);
  chSysRestoreStatusX(sts);

  return p;
}

void osBmemFree(void *p)
{
  syssts_t sts;

  if(!p)
    return ;

  sts = chSysGetStatusAndLockX();
  _bmem_free(p);
  chSysRestoreStatusX(sts);
}

uint32_t osBmemGetInfo(uint32_t *pu32Size)
{
  uint32_t u32Cnt = 0, u32Idle = 0;
  uint16_t i, u16Os, u16Bit, u16F = 0;

  for(i=0; i<BMEM_BITSIZE; i++)
  {
    u16Os  = i/BMEM_MAPUNIT;
    u16Bit = i%BMEM_MAPUNIT;

    if((g_u32BMemMap[u16Os] & (1u << u16Bit)) == 0)
    {
      if(u16F)
        u32Cnt++;
      
      u16F = 0;
      u32Idle++;
    }
    else
    {
      u16F = 1;
    }
  }

  *pu32Size = u32Idle * BMEM_UNIT;

  return u32Cnt;
}

/** @} */
