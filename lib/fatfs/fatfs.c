#include "lib.h"

#if HAL_USE_FATFS

typedef struct
{
  FATFS      *pstFs;
  uint8_t     u8IsMount;
}hs_fatfs_info_t;

static hs_fatfs_info_t g_stFsInfo[FATFS_MEMDEV_NUM];
static const TCHAR  *g_tcRoot[FATFS_MEMDEV_NUM] =
{
  _T("0:"),
  _T("1:"),
};

int hs_sd_timTaining(void);

void hs_fatfs_mount(uint16_t msg, void *parg)
{
  (void)msg;
  hs_fatfs_memdev_t memDev = (hs_fatfs_memdev_t)parg;

  if ((memDev >= FATFS_MEMDEV_NUM) || (g_stFsInfo[memDev].u8IsMount != 0))
    return ;

  if(!g_stFsInfo[memDev].pstFs)
    return ;

  if(FR_OK != f_mount(g_stFsInfo[memDev].pstFs, g_tcRoot[memDev], 1))
  {
    f_mount(NULL, g_tcRoot[memDev], 0);
    return ;
  }

  g_stFsInfo[memDev].u8IsMount = 1;
  hs_cfg_sysReqPeripArg(HS_CFG_EVENT_MEMDEV_IN, parg); 
  return ;
}

void hs_fatfs_unmount(uint16_t msg, void *parg)
{
  (void)msg;
  hs_fatfs_memdev_t memDev = (hs_fatfs_memdev_t)parg;

  if((memDev >= FATFS_MEMDEV_NUM) || (g_stFsInfo[memDev].u8IsMount == 0))
    return ;  

  hs_cfg_sysReqPeripArg(HS_CFG_EVENT_MEMDEV_OUT, parg);   
  f_mount(NULL, g_tcRoot[memDev], 0);
  g_stFsInfo[memDev].u8IsMount = 0;
}

#endif

bool hs_fatfs_isMount(hs_fatfs_memdev_t memDev)
{
  #if HAL_USE_FATFS
  return (bool)(g_stFsInfo[memDev].u8IsMount);
  #else
  (void)memDev;
  return FALSE;
  #endif
}

void hs_fatfs_init(void)
{
  g_stFsInfo[FATFS_MEMDEV_SD].u8IsMount = 0;
  g_stFsInfo[FATFS_MEMDEV_SD].pstFs = (FATFS *)hs_malloc(sizeof(FATFS), __MT_DMA);

  g_stFsInfo[FATFS_MEMDEV_UDISK].u8IsMount = 0;
  g_stFsInfo[FATFS_MEMDEV_UDISK].pstFs = (FATFS *)hs_malloc(sizeof(FATFS), __MT_DMA);
}


