/*
    bootloader - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/mem/lib_bmem.h
 * @brief   bus memory manager.
 * @details 
 *
 * @addtogroup  config
 * @details 
 * @{
 */
#ifndef __CMSIS_BMEM_H__
#define __CMSIS_BMEM_H__

#include "string.h"
#include "stdint.h"

#define BMEM_BASE           0x10000000
#define BMEM_MASK           0xf0000000


#ifdef __cplusplus
extern "C" {
#endif

void *osBmemAllocS(uint16_t u16Size)  __attribute__((used));
void osBmemFreeS(void *p)  __attribute__((used));
void *osBmemAlloc(uint16_t u16Size)  __attribute__((used));
void osBmemFree(void *p)  __attribute__((used));
uint32_t osBmemGetInfo(uint32_t *pu32Size)  __attribute__((used));

#ifdef __cplusplus
}
#endif


#endif
 /** @} */
