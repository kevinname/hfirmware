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

void *lib_bmem_alloc(uint16_t u16Size)
{
  return osBmemAlloc(u16Size);
}

void lib_bmem_free(void *p)
{
  osBmemFree(p);
}

uint32_t lib_bmem_getInfo(uint32_t *pu32Size)
{
  return osBmemGetInfo(pu32Size);
}

void lib_bmem_init(void)
{
  ;
}

/** @} */
