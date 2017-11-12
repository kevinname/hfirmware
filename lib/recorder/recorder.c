/*
    recorder - Copyright (C) 2012~2017 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    recorder/recorder.c
 * @brief   recorder file.
 * @details 
 *
 * @addtogroup  recorder
 * @details 
 * @{
 */


#include "lib.h"

typedef enum
{
  REC_STATUS_TERIMINATE     = 0,
  REC_STATUS_WORKING        ,
  REC_STATUS_STOPPED        ,
}hs_rec_status_t;

typedef struct
{
  hs_rec_status_t   eStatus;

  hs_recwr_t       *pstRecWr;
  hs_ai_t          *pstAi;

  hs_rec_cfg_t      stRecCfg;
}hs_recorder_t;

static hs_recorder_t *g_pstRecorder;

hs_recorder_t *_recorder_create(void)
{
  hs_recorder_t *pstRec;
  hs_rec_cfg_t  *pstRecCfg;

  pstRec = (hs_recorder_t *)hs_malloc(sizeof(hs_recorder_t), __MT_Z_GENERAL);
  if(!pstRec)
    return NULL;

  pstRecCfg = &pstRec->stRecCfg;
  if(HS_CFG_OK != hs_cfg_getDataByIndex(HS_CFG_RECORDER_INFO, (uint8_t *)pstRecCfg, sizeof(hs_rec_cfg_t)))
  {
    pstRecCfg->u32Sample = 16000;
    pstRecCfg->u32MaxNum = 10000;
    pstRecCfg->s16Gain   = 24;
    pstRecCfg->u16Mode   = 1;
    
    memcpy(pstRecCfg->u8DirName, "RECORDER", 8);
    memcpy(pstRecCfg->u8FilePrefix, "REC", 3);
  }

  pstRecCfg->u32MaxNum = pstRecCfg->u32MaxNum > 10000 ? 10000 : pstRecCfg->u32MaxNum;
  pstRec->eStatus = REC_STATUS_STOPPED;
  return pstRec;
}

void _recorder_destroy(hs_recorder_t *pstRec)
{
  if(!pstRec)
    return ;

  hs_recwr_stop(pstRec->pstRecWr);
  hs_ai_destroy(pstRec->pstAi);
  hs_free(pstRec); 
}

void hs_recorder_switch(uint16_t u16Idx, void *parg)
{
  (void)u16Idx;
  (void)parg;
  hs_recorder_t *pstRec;
  
  if(g_pstRecorder)
  {
    _recorder_destroy(g_pstRecorder);
    g_pstRecorder = NULL;
    return ;
  }

  if(!hs_fatfs_isMount(FATFS_MEMDEV_SD) && !hs_fatfs_isMount(FATFS_MEMDEV_UDISK))
    return ;
  
  pstRec = _recorder_create();
  if(!pstRec) 
    return ;

  pstRec->pstAi = hs_ai_create(&pstRec->stRecCfg);
  if(!pstRec->pstAi)
  {
    hs_free(pstRec);
    return ;
  }

  pstRec->pstRecWr = hs_recwr_start(&pstRec->stRecCfg, pstRec->pstAi);
  if(!pstRec->pstRecWr)
  {
    hs_ai_destroy(pstRec->pstAi);
    hs_free(pstRec);
    return ;
  }

  g_pstRecorder = pstRec;
}

bool hs_recorder_isWorking(void)
{
  return (g_pstRecorder != NULL);
}



