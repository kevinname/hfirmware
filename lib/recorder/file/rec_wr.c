/*
    recorder - Copyright (C) 2012~2017 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    recorder/file/rec_wr.c
 * @brief   recorder file.
 * @details 
 *
 * @addtogroup  recorder
 * @details 
 * @{
 */


#include "lib.h"
#include "rec_wr.h"


#define REC_ROOT0             ((TCHAR *)_T("0://"))
#define REC_ROOT1             ((TCHAR *)_T("1://"))
#define _REC_LEN_ONECE        512

typedef struct
{
  char  riff_sig[4];              // 'RIFF'
  long  waveform_chunk_size;      // 8
  char  wave_sig[4];              // 'WAVE'
  char  format_sig[4];            // 'fmt ' (notice space after)
  long  format_chunk_size;        // 16;
  short format_tag;               // WAVE_FORMAT_PCM
  short channels;                 // # of channels
  long  sample_rate;              // sampling rate
  long  bytes_per_sec;            // bytes per second
  short block_align;              // sample block alignment
  short bits_per_sample;          // bits per second
  char  data_sig[4];              // 'data'
  long  data_size;                // size of waveform data
}hs_wav_h_t;

static hs_recwr_t *g_pstRecWr;
static hs_musicpara_t *g_pstMusicRec;

uint32_t _recwr_createDir(hs_rec_cfg_t *pstRecCfg, TCHAR **pptPath)
{
  TCHAR *ptPath, *ptTmp;
  uint32_t i, len;
  hs_musicpara_t *pstMusic = g_pstMusicRec;
  const void *rootDir;

  len = REC_DIRNAME_LEN + REC_FILEFIX_LEN + 6;
  ptPath = (TCHAR *)hs_malloc(sizeof(TCHAR) * len, __MT_Z_GENERAL);
  if(!ptPath)
  {
    *pptPath = NULL;
    return 0;
  }

  rootDir = pstMusic->musicPos == 0 ? (const void *)REC_ROOT0 : (const void *)REC_ROOT1;
  memcpy(ptPath, (const void *)rootDir, wstrlen(rootDir) * sizeof(TCHAR));
  ptTmp = ptPath + wstrlen(rootDir);
  
  for(i=0; i<strlen((char *)pstRecCfg->u8DirName); i++)
  {
    *ptTmp = pstRecCfg->u8DirName[i];
    ptTmp += 1;
  }

  if(FR_OK != f_mkdir(ptPath)) 
    hs_printf("dir already exist!\r\n");

  *ptTmp = _T('/');
  ptTmp += 1;

  *pptPath = ptPath;
  return (uint32_t)(ptTmp - ptPath);
}

uint32_t _recwr_getPath(hs_rec_cfg_t *pstRecCfg, TCHAR *ptPath, uint32_t u32Idx)
{
  uint32_t i, u32Offset = 0;

  for(i=0; i<strlen((char *)pstRecCfg->u8FilePrefix); i++)
    ptPath[u32Offset++] = pstRecCfg->u8FilePrefix[i];

  u32Idx %= 10000;
  ptPath[u32Offset++] = u32Idx / 1000 + _T('0');

  u32Idx %= 1000;
  ptPath[u32Offset++] = u32Idx / 100 + _T('0');

  u32Idx %= 100;
  ptPath[u32Offset++] = u32Idx / 10 + _T('0');
  ptPath[u32Offset++] = u32Idx % 10 + _T('0');

  ptPath[u32Offset++] = _T('.');
  ptPath[u32Offset++] = _T('w');
  ptPath[u32Offset++] = _T('a');
  ptPath[u32Offset++] = _T('v');
  
  return u32Offset;
}

void _recwr_wavHeaderInit(hs_wav_h_t *pstHeader, hs_ai_t *pstAi)
{
  memcpy(pstHeader->riff_sig,   "RIFF", 4);
  memcpy(pstHeader->wave_sig,   "WAVE", 4);
  memcpy(pstHeader->format_sig, "fmt ", 4);
  memcpy(pstHeader->data_sig,   "data", 4);

  pstHeader->format_chunk_size = 0x00000010;
  pstHeader->format_tag = 0x01;

  if(pstAi->stI2sCfg.i2s_mode == I2S_PCMMODE_MONO)
    pstHeader->channels = 1;
  else
    pstHeader->channels = 2;

  pstHeader->sample_rate = pstAi->stI2sCfg.sample_rate;
  pstHeader->bits_per_sample = 16;
  pstHeader->bytes_per_sec = pstHeader->sample_rate * pstHeader->bits_per_sample * pstHeader->channels / 8;
  pstHeader->block_align = pstHeader->channels * pstHeader->bits_per_sample / 8;
}

void _recwr_saveCurrPath(TCHAR *ptPath)
{ 
  //hs_musicpara_t *pstMusic = g_pstMusicRec;
  uint8_t *pu8Ptr;  

  if(g_pstMusicRec->musicPos == 0)
  {
    pu8Ptr = g_pstMusicRec->sd_name;
    g_pstMusicRec->sd_offset = 0;
  }
  else
  {
    pu8Ptr = g_pstMusicRec->udisk_name;
    g_pstMusicRec->udisk_offset = 0;
  }
  memcpy(pu8Ptr, ptPath, wstrlen(ptPath) * sizeof(TCHAR));
      
  hs_cfg_setDataByIndex(HS_CFG_MISC_MP3, (uint8_t *)g_pstMusicRec, sizeof(hs_musicpara_t), 0);
} 

hs_pstFile_t _recwr_createFile(hs_rec_cfg_t *pstRecCfg)
{
  hs_pstFile_t pstFile;
  FILINFO stfile;
  TCHAR *ptPath, *ptTmp;
  uint32_t i, u32Num;

  pstFile = (hs_pstFile_t)hs_malloc(sizeof(FIL), __MT_Z_DMA);
  if(!pstFile)
    return NULL;

  u32Num = _recwr_createDir(pstRecCfg, &ptPath);
  if(!ptPath)
  {
    hs_free(pstFile);
    return NULL;
  }

  for(i=0; i<pstRecCfg->u32MaxNum; i++)
  {
    ptTmp = ptPath + u32Num;
    _recwr_getPath(pstRecCfg, ptTmp, i);

    if(FR_OK != f_stat(ptPath, &stfile))
      break;
  }

  if(FR_OK != f_open(pstFile, ptPath, FA_CREATE_ALWAYS | FA_WRITE))
  {
    hs_printf("create %s failed!\r\n", ptPath);
    hs_free(pstFile);
    hs_free(ptPath);
    return NULL;
  }

  _recwr_saveCurrPath(ptPath);
  hs_free(ptPath);
  return pstFile;
}

void _recwr_closeFile(hs_pstFile_t pstFile)
{
  if(!pstFile)
    return ;

  f_sync(pstFile);
  f_close(pstFile);

  hs_free(pstFile);
}

void _recwr_thread(void* arg)
{
  hs_recwr_t *pstRecWr = (hs_recwr_t *)arg;
  hs_wav_h_t stHeader;
  uint32_t u32Size;
  uint8_t *pu8Ptr;

  chRegSetThreadName("recService");
  hs_printf("recording......\r\n");

  pstRecWr->u32Frame = 0;
  pu8Ptr = (uint8_t *)hs_malloc(_REC_LEN_ONECE, __MT_Z_DMA);
  if(!pu8Ptr)
    return ;

  _recwr_wavHeaderInit(&stHeader, pstRecWr->pstAi);
  u32Size = sizeof(hs_wav_h_t);
  if(FR_OK != f_write(pstRecWr->pstFile, &stHeader, u32Size, (UINT *)&u32Size)){
    hs_printf("recorder write file failed!\r\n");
    return ;
  }

  while(1)
  {
    u32Size = hs_ai_fetchData(pstRecWr->pstAi, pu8Ptr, _REC_LEN_ONECE);
    if(FR_OK != f_write(pstRecWr->pstFile, pu8Ptr, u32Size, (UINT *)&u32Size))
      break ;

    pstRecWr->u32Frame += u32Size;
    if(pstRecWr->eStatus != RECWR_STATUS_START)
      break;
  }

  f_lseek(pstRecWr->pstFile, 0);
  stHeader.waveform_chunk_size = pstRecWr->u32Frame + 0x2c - 8;
  stHeader.data_size = pstRecWr->u32Frame;

  u32Size = sizeof(hs_wav_h_t);
  f_write(pstRecWr->pstFile, &stHeader, u32Size, (UINT *)&u32Size);
  f_sync(pstRecWr->pstFile);

  hs_free(pu8Ptr);
  hs_printf("recorder over!\r\n");
}

hs_recwr_t *hs_recwr_start(hs_rec_cfg_t *pstRecCfg, hs_ai_t *pstAi)
{
  hs_recwr_t *pstRecWr;
  osThreadDef_t stThdDef;  

  pstRecWr = (hs_recwr_t *)hs_malloc(sizeof(hs_recwr_t), __MT_Z_GENERAL);
  if(!pstRecWr)
    return NULL;

  g_pstMusicRec = (hs_musicpara_t *)hs_malloc(sizeof(hs_musicpara_t), __MT_Z_GENERAL);
  if(!g_pstMusicRec)
  {
    hs_free(pstRecWr);
    return NULL;
  }
  
  if(HS_CFG_OK != hs_cfg_getDataByIndex(HS_CFG_MISC_MP3, (uint8_t *)g_pstMusicRec, sizeof(hs_musicpara_t)))
    g_pstMusicRec->musicPos = hs_fatfs_isMount(FATFS_MEMDEV_SD) ? 0 : 1;
  else
    g_pstMusicRec->musicPos = (hs_fatfs_isMount(FATFS_MEMDEV_SD) 
                                 && g_pstMusicRec->musicPos == 0) ? 0 : 1;

  pstRecWr->u8Pos = g_pstMusicRec->musicPos;
  pstRecWr->pstFile = _recwr_createFile(pstRecCfg);
  if(!pstRecWr->pstFile)
  {
    hs_free(pstRecWr);
    hs_free(g_pstMusicRec);
    g_pstMusicRec = NULL;
    return NULL;
  }

  pstRecWr->pstAi   = pstAi;
  pstRecWr->eStatus = RECWR_STATUS_START;
  
  stThdDef.pthread   = (os_pthread)_recwr_thread;
  stThdDef.stacksize = 1024 * 4;
  stThdDef.tpriority = osPriorityNormal;     
  pstRecWr->pstThd  = oshalThreadCreate(&stThdDef, pstRecWr); 
  if(!pstRecWr->pstThd)
  {
    pstRecWr->eStatus = RECWR_STATUS_TERMINATE;
    _recwr_closeFile(pstRecWr->pstFile);
    hs_free(pstRecWr);

    hs_free(g_pstMusicRec);
    g_pstMusicRec = NULL;
    return NULL;  
  }

  hs_free(g_pstMusicRec);
  g_pstMusicRec = NULL;

  g_pstRecWr = pstRecWr;
  return pstRecWr;
}

void hs_recwr_stop(hs_recwr_t *pstRecWr)
{
  if(!pstRecWr)
    return ;

  g_pstRecWr = NULL;
  pstRecWr->eStatus = RECWR_STATUS_TERMINATE;
  if(pstRecWr->pstThd)
    oshalThreadTerminate(pstRecWr->pstThd);

  _recwr_closeFile(pstRecWr->pstFile);
  hs_free(pstRecWr->pstFile);
  hs_free(g_pstMusicRec);
  g_pstMusicRec = NULL;
}

__USED uint32_t hs_recwr_getTime(void)
{
  hs_recwr_t *pstRecWr = g_pstRecWr;
  uint32_t u32Sample, u32ChnNum;

  if((!pstRecWr) || (pstRecWr->eStatus != RECWR_STATUS_START))
    return 0;

  u32Sample = pstRecWr->pstAi->stI2sCfg.sample_rate;
  u32ChnNum = pstRecWr->pstAi->stI2sCfg.i2s_mode == I2S_PCMMODE_STEREO ? 2 : 1;

  return (pstRecWr->u32Frame / 2 / u32ChnNum / u32Sample);
}

__USED hs_recwr_dev_t hs_recwr_getPos(void)
{
  hs_recwr_t *pstRecWr = g_pstRecWr;

  if(!pstRecWr)
    return RECWR_DEVICE_NO;

  return (pstRecWr->u8Pos == 0 ? RECWR_DEVICE_SD : RECWR_DEVICE_UDISK);
}



