#ifndef __LIB_FATFS_H__
#define __LIB_FATFS_H__

#include "ff.h"

typedef enum
{
  FATFS_MEMDEV_SD       = 0,
  FATFS_MEMDEV_UDISK    ,

  FATFS_MEMDEV_NUM      
}hs_fatfs_memdev_t;

#if HAL_USE_FATFS
void hs_fatfs_mount(uint16_t msg, void *parg);
void hs_fatfs_unmount(uint16_t msg, void *parg);
#endif

bool hs_fatfs_isMount(hs_fatfs_memdev_t memDev) ;
void hs_fatfs_init(void);
#endif
