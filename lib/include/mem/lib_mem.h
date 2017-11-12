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
#ifndef __LIB_MEM_H__
#define __LIB_MEM_H__

#define __MT_GENERAL        0x0001
#define __MT_DMA            0x0002
#define __MT_ZERO           0x0004

#define __MT_Z_GENERAL      (__MT_GENERAL | __MT_ZERO)
#define __MT_Z_DMA          (__MT_DMA     | __MT_ZERO)

#define __MT_T_MASK         (__MT_DMA | __MT_GENERAL)
#define __MT_ChkType(mt)    ((__MT_T_MASK & (mt)) == __MT_T_MASK)
#define __MT_GetType(ad)    (memtype_t)(((uint32_t)(ad) & BMEM_MASK) == BMEM_BASE ? __MT_DMA : __MT_GENERAL)


#define __MCT_MP3           0
#define __MCT_BTSTACK       1
#define __MCT_NUM           2

typedef uint32_t memtype_t;

#ifdef __cplusplus
extern "C" {
#endif

uint32_t hs_memInfo(uint32_t *pu32Size, memtype_t type);
void *hs_malloc(uint32_t u32Size, memtype_t type);
void hs_free(void *p);
void lib_mem_init(void);

int hs_mem_loadCode(int type);

#ifdef __cplusplus
}
#endif


#endif
 /** @} */
