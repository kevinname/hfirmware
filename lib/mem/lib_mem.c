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

#include "lib.h"
#include "lib_bmem.h"

typedef struct
{
  uint32_t u32VtAddr;
  uint32_t u32Size;
  uint32_t u32LdAddr;
  uint32_t reserve;
}__rcodeType_t;

extern char _ovly_table[];

void *hs_malloc(uint32_t u32Size, memtype_t type)
{
  void *p = NULL;

  if(__MT_ChkType(type))
    return NULL;

  if(type & __MT_GENERAL)
    p = chHeapAlloc(NULL, u32Size);

  if(type & __MT_DMA)
    p = lib_bmem_alloc(u32Size);

  if((type & __MT_ZERO) && p)
    memset(p, 0, u32Size);

  return p;
}

void hs_free(void *p)
{
  memtype_t type;

  if(p == NULL)
    return ;
  
  type = __MT_GetType(p);
  
  if(type & __MT_GENERAL)
    chHeapFree(p);

  if(type & __MT_DMA)
    lib_bmem_free(p);
}

uint32_t hs_memInfo(uint32_t *pu32Size, memtype_t type)
{
  if(pu32Size == NULL)
    return 0;
  
  if(__MT_ChkType(type))
    return 0;

  if(type & __MT_GENERAL)
  {
    uint32_t u32Frag;

    u32Frag = chHeapStatus(NULL, pu32Size);
    *pu32Size += chCoreGetStatusX();
    return u32Frag;
  }

  if(type & __MT_DMA)
    return lib_bmem_getInfo(pu32Size);

  return 0;
}

int hs_mem_loadCode(int type)
{
  #if defined(RUN_IN_FLASH) && HS_PRODUCT_TYPE == HS_PRODUCT_SOUNDBOX
  __rcodeType_t *pstRamCode = (__rcodeType_t *)_ovly_table;

  if(type >= __MCT_NUM)
    return -1;

  memcpy((void *)pstRamCode[type].u32VtAddr, (void *)pstRamCode[type].u32LdAddr, pstRamCode[type].u32Size);
  #else
  (void)type;
  #endif
  
  return 0;
}

void lib_mem_init(void)
{
  lib_bmem_init();
}


/** @} */
