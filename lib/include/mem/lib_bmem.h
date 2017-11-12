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
#ifndef __LIB_BMEM_H__
#define __LIB_BMEM_H__

#ifdef __cplusplus
extern "C" {
#endif

void  lib_bmem_init(void);
void *lib_bmem_alloc(uint16_t u16Size);
void  lib_bmem_free(void *p);
uint32_t lib_bmem_getInfo(uint32_t *pu32Size);

#ifdef __cplusplus
}
#endif


#endif
 /** @} */
