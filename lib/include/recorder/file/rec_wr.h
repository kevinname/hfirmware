/*
    recorder - Copyright (C) 2012~2017 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    recorder/file/rec_wr.h
 * @brief   recorder file.
 * @details 
 *
 * @addtogroup  recorder
 * @details 
 * @{
 */


#ifndef __AUDIO_RECWR_H__
#define __AUDIO_RECWR_H__

#include "lib.h"


typedef enum
{
  RECWR_STATUS_TERMINATE      = 0,
  RECWR_STATUS_START          ,
  RECWR_STATUS_STOP           ,
}hs_recwr_status_t;

typedef enum
{
  RECWR_DEVICE_NO             = 0,
  RECWR_DEVICE_SD             ,
  RECWR_DEVICE_UDISK          ,
}hs_recwr_dev_t;


typedef struct
{
  hs_pstFile_t          pstFile;
  hs_ai_t              *pstAi;
  
  osThreadId            pstThd;
  hs_recwr_status_t     eStatus;
  uint32_t              u32Frame;
  uint8_t               u8Pos;
}hs_recwr_t;

hs_recwr_t *hs_recwr_start(hs_rec_cfg_t *pstRecCfg, hs_ai_t *pstAi);
void hs_recwr_stop(hs_recwr_t *pstRecWr);
__USED uint32_t hs_recwr_getTime(void);
__USED hs_recwr_dev_t hs_recwr_getPos(void);


#endif
