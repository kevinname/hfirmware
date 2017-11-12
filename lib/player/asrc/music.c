#include "lib.h"
#include "mp3common.h"


#if HS_USE_MP3 && HS_USE_PLAYER

#define MUSIC_POS_BASE            0x80
#define MUSIC_POS_SD              (MUSIC_POS_BASE + FATFS_MEMDEV_SD)
#define MUSIC_POS_UDISK           (MUSIC_POS_BASE + FATFS_MEMDEV_UDISK)

#define AVERAGE_FILENAME_LENGTH   40
#define MAX_PLAYLIST_SIZE         800


static hs_musicinfo_t *g_pstMusic;
static uint16_t g_u16LastPos;
void _music_thread(void* arg);

uint8_t hs_music_getDefaultEqNum(void);
const hs_codec_eqpara_t * hs_music_getDefaultEqData(uint8_t u8Idx);
void hs_adec_ffRew(hs_adec_t *pstAdec, uint32_t u32Frames);
void hs_adec_ffRewExit(hs_adec_t *pstAdec);
static void _music_loadDec(void const *arg);

static int _music_alphaCmp(const void* p1, const void* p2)
{
  int pos1 = ((hs_fileinfo_t *)p1)->indices;
  int pos2 = ((hs_fileinfo_t *)p2)->indices;

  return wstrcmp((const TCHAR *)&g_pstMusic->buffer[pos1],
                (const TCHAR *)&g_pstMusic->buffer[pos2]);
}

static int _music_alphaCmpInvt(const void* p1, const void* p2)
{
  int pos1 = ((hs_fileinfo_t *)p1)->indices;
  int pos2 = ((hs_fileinfo_t *)p2)->indices;

  return wstrcmp((const TCHAR *)&g_pstMusic->buffer[pos2],
                (const TCHAR *)&g_pstMusic->buffer[pos1]);
}

static int _music_dateCmp(const void* p1, const void* p2)
{
  return ((hs_fileinfo_t *)p1)->time - ((hs_fileinfo_t *)p2)->time;
}

static int _music_dateCmpInvt(const void* p1, const void* p2)
{
  return ((hs_fileinfo_t *)p2)->time - ((hs_fileinfo_t *)p1)->time;
}

static int _music_searchByName(hs_musicinfo_t *pstMusic, TCHAR *filename, uint8_t cnt)
{
  int i = 0, i_f = 0;
  int pos;

  while(i < pstMusic->amount){
    pos = pstMusic->pstFileInfo[i].indices;
    if(wstrcmp((const TCHAR *)filename, (const TCHAR *)&pstMusic->buffer[pos]) == 0)
    {
      i_f ++;
      if(i_f == cnt)
        return i;
    }

    i++;
  }

  return -1;
}

static int16_t _music_searchUdisk(hs_musicinfo_t *pstMusic)
{
  hs_musicpara_t *pstCfg = &pstMusic->stCfg;
  int16_t i = 1, s16MusicIdx = 0;

  if(wstrlen((TCHAR *)pstCfg->udisk_name) == 0)
      s16MusicIdx = pstMusic->sdNum;
  else
  {
    do
    {
      s16MusicIdx = _music_searchByName(pstMusic, (TCHAR *)pstCfg->udisk_name, i);
      i++;

      if(s16MusicIdx >= pstMusic->amount)
      {
        s16MusicIdx = pstMusic->sdNum;
        break;
      }

      if((s16MusicIdx < pstMusic->amount) && (s16MusicIdx >= pstMusic->sdNum))
        break;
      
    }while(s16MusicIdx > 0);
  }

  return s16MusicIdx;
}

static uint16_t _music_getCurrIdx(hs_musicinfo_t *pstMusic)
{
  hs_musicpara_t *pstCfg = &pstMusic->stCfg;
  int16_t i = 1, s16MusicIdx = 0;

  if(g_u16LastPos == 0)
  {
    if(pstMusic->sdNum > 0)
      s16MusicIdx = _music_searchByName(pstMusic, (TCHAR *)pstCfg->sd_name, 1);
    else
      s16MusicIdx = _music_searchByName(pstMusic, (TCHAR *)pstCfg->udisk_name, 1);
  }
  else if(g_u16LastPos == MUSIC_POS_SD)
  {
    if(pstMusic->sdNum <= 0)
      return 0;

    s16MusicIdx = _music_searchByName(pstMusic, (TCHAR *)pstCfg->sd_name, i);
    
    if(s16MusicIdx < 0)
    {
      pstCfg->sd_offset = 0;
      pstCfg->sd_t      = 0;
    }
  }
  else
  {
    if(pstMusic->udiskNum <= 0)
      return 0;

    s16MusicIdx = _music_searchUdisk(pstMusic);

    if(s16MusicIdx < 0)
    {
      pstCfg->udisk_offset = 0;
      pstCfg->udisk_t      = 0;
      s16MusicIdx = pstMusic->sdNum;
    }
  }
  
  s16MusicIdx = s16MusicIdx > 0 ? s16MusicIdx : 0;
  return s16MusicIdx;
}

void _music_saveContext(hs_musicinfo_t *pstMusic)
{
  uint32_t *pu32Frames, *pu32Tim, u32Frame, u32Tmp;

  if(pstMusic->saved)
    return ;

  u32Frame = hs_adec_context(pstMusic->pstAdec);
  if(u32Frame > 0)
  {
    pu32Frames = pstMusic->current < pstMusic->sdNum ?
                 &pstMusic->stCfg.sd_offset :
                 &pstMusic->stCfg.udisk_offset;

    pu32Tim    = pstMusic->current < pstMusic->sdNum ?
                 &pstMusic->stCfg.sd_t :
                 &pstMusic->stCfg.udisk_t;
                 
    pstMusic->stCfg.musicPos = pstMusic->current < pstMusic->sdNum ? 0 : 1;
    *pu32Frames = u32Frame;
    u32Tmp = hs_music_getTime();
    if(u32Tmp > 0)
      *pu32Tim = u32Tmp;

    hs_cfg_setDataByIndex(HS_CFG_MISC_MP3, (uint8_t *)&pstMusic->stCfg, sizeof(hs_musicpara_t), 0);
    pstMusic->saved = 1;
  }
}

int _music_chkFileExist(hs_musicinfo_t *pstMusic)
{
  if(FR_OK != f_lseek(pstMusic->pstFid, 0))
    return -1;

  return 0;
}

static int _music_listSort(hs_musicinfo_t *pstMusic, int type)
{
  if(type == SORT_ALPHA)
    qsort(pstMusic->pstFileInfo, pstMusic->amount, 
                   sizeof(hs_fileinfo_t), _music_alphaCmp);
  else if(type == SORT_ALPHA_REVERSED)
    qsort(pstMusic->pstFileInfo, pstMusic->amount, 
                  sizeof(hs_fileinfo_t), _music_alphaCmpInvt);
  else if(type == SORT_DATE)
    qsort(pstMusic->pstFileInfo, pstMusic->amount, 
                  sizeof(hs_fileinfo_t), _music_dateCmp);
  else if(type == SORT_DATE_REVERSED)
     qsort(pstMusic->pstFileInfo, pstMusic->amount, 
                  sizeof(hs_fileinfo_t), _music_dateCmpInvt);
  
  return 0;
}

static int _music_listAdd(hs_musicinfo_t *pstMusic, 
                          const TCHAR *filename, 
                          int strLength, uint32_t date)
{
  int len = strLength * sizeof(TCHAR);

  if((len + sizeof(TCHAR) >= pstMusic->buffer_size - pstMusic->buffer_end_pos * sizeof(TCHAR)) 
    || (pstMusic->amount >= pstMusic->max_playlist_size))
  {
    return -1;
  }

  pstMusic->pstFileInfo[pstMusic->amount].indices = pstMusic->buffer_end_pos;

  memcpy(&pstMusic->buffer[pstMusic->buffer_end_pos], filename, len);
  pstMusic->buffer_end_pos += strLength;

  pstMusic->pstFileInfo[pstMusic->amount].time = date;
  pstMusic->amount++;
  pstMusic->buffer[pstMusic->buffer_end_pos++] = _T('\0');

  return 0;
}

static uint16_t _music_scan(hs_musicinfo_t *pstMusic, TCHAR* path, int depth)
{
  FILINFO filInfo = {0};
  DIR d;
  TCHAR *p;
  uint16_t num = 0;

  TCHAR *tmp = (TCHAR *)hs_malloc(sizeof(TCHAR) * (_MAX_LFN + 1), __MT_Z_GENERAL);;   
  filInfo.lfname = (TCHAR *)hs_malloc(sizeof(TCHAR) * (_MAX_LFN + 1), __MT_Z_GENERAL);
  filInfo.lfsize = sizeof(TCHAR) * (_MAX_LFN + 1);

  if(FR_OK != f_opendir(&d, path))
  {
    hs_free(tmp);
    hs_free(filInfo.lfname);
    return num;
  }

  while(FR_OK == f_readdir(&d, &filInfo))
  {
    if(filInfo.fname[0] == 0)
      break;

    p = (filInfo.lfname[0] != 0)? filInfo.lfname : filInfo.fname;

    if(wstrcmp((const TCHAR *)p, _T(".")) == 0)
      continue;

    if(wstrcmp((const TCHAR *)p, _T("..")) == 0)  //skip parent directory(HS6600B1)
      continue;

    if(wstrstr(p, _T(".mp3")) != 0 || wstrstr(p, _T(".wav")) != 0 
        #if PLAYER_INC_WMA
        || (wstrstr(p, _T(".wma")) != 0 && (pstMusic->stCfg.featureEnables & 0x2))
        #endif
        )
     {
      memcpy(tmp, path, wstrlen(path) * sizeof(TCHAR));
      memcpy(tmp + wstrlen(path), _T("/"), sizeof(TCHAR));
      memcpy(tmp + wstrlen(path) + 1, p, wstrlen(p) * sizeof(TCHAR));
      tmp[wstrlen(path) + 1 + wstrlen(p)] = 0;
      if(_music_listAdd(pstMusic, tmp, wstrlen(path) + 1 + wstrlen(p),    //tmp has ':', so we can't use wcslen here
                        (filInfo.fdate << 16) + filInfo.ftime) != 0)
      {
        break;
      }

      num += 1;
      continue;
    }

    if((filInfo.fattrib && AM_DIR) && (depth < pstMusic->stCfg.dirLvl))
    {
      memcpy(tmp, path, wstrlen((const TCHAR *)path) * sizeof(TCHAR));
      memcpy(tmp + wstrlen((const TCHAR *)path), _T("/"), sizeof(TCHAR));
      memcpy(tmp + wstrlen((const TCHAR *)path) + 1, p, wstrlen((const TCHAR *)p) * sizeof(TCHAR));
      tmp[wstrlen(path) + 1 + wstrlen(p)] = 0;
      num += _music_scan(pstMusic, tmp, depth + 1);
    }
  }

  hs_free(tmp);
  hs_free(filInfo.lfname);
  return num;
}


TCHAR * _music_getCurr(hs_musicinfo_t *pstMusic)
{
  int index;

  if((!pstMusic) || (pstMusic->amount <= 0))
    return NULL;

  index = pstMusic->pstFileInfo[pstMusic->current].indices;
  return &pstMusic->buffer[index];
}

TCHAR * _music_getNext(hs_musicinfo_t *pstMusic)
{
  int index;

  if(0 == pstMusic->amount)
    return NULL;

  switch(pstMusic->stCfg.playMode)
  {
  case REPEAT_OFF:
    pstMusic->current += 1;
    if(pstMusic->current >= pstMusic->maxIdx)
    {
      pstMusic->current = pstMusic->minIdx;
      return 0;
    }
    
    break;

  case REPEAT_ALL:
    pstMusic->current += 1;
    if(pstMusic->current >= pstMusic->maxIdx)
      pstMusic->current = pstMusic->minIdx;
    
    break;

  case REPEAT_ONE:
    break;

  case REPEAT_SHUFFLE:
    pstMusic->current = pstMusic->minIdx + rand() % (pstMusic->maxIdx - pstMusic->minIdx);
    break;

  default:
    break;
  }

  index = pstMusic->pstFileInfo[pstMusic->current].indices;
  return &pstMusic->buffer[index];
}

TCHAR *_music_getPrev(hs_musicinfo_t *pstMusic)
{
  int index;

  if(0 == pstMusic->amount)
    return NULL;

  switch(pstMusic->stCfg.playMode)
  {
  case REPEAT_OFF:
    pstMusic->current -= 1;
    if(pstMusic->current < pstMusic->minIdx)
    {
      pstMusic->current = pstMusic->minIdx;
      return 0;
    }
    break;

  case REPEAT_ALL:
    pstMusic->current -= 1;
    if(pstMusic->current < pstMusic->minIdx)
      pstMusic->current = pstMusic->maxIdx - 1;
    break;

  case REPEAT_ONE:
    break;

  case REPEAT_SHUFFLE:
    pstMusic->current = pstMusic->minIdx + rand() % (pstMusic->maxIdx - pstMusic->minIdx);
    break;

  default:
    break;
  }

  index = pstMusic->pstFileInfo[pstMusic->current].indices;
  return &pstMusic->buffer[index];
}

static void _music_initCfg(hs_musicpara_t *pstPyCfg)
{
  if(HS_CFG_OK != hs_cfg_getDataByIndex(HS_CFG_MISC_MP3, (uint8_t *)pstPyCfg, sizeof(hs_musicpara_t)))
  {
    pstPyCfg->volume         = -20;
    pstPyCfg->volume_step    = 1;
    pstPyCfg->mode_map       = 0xffff;
  }

  if(pstPyCfg->mode_map == 0)
    pstPyCfg->mode_map = 0xffff;
}

static int _music_initServ(hs_musicinfo_t *pstMusic)
{
  osMessageQDef_t *pstMsgDef;
  osThreadDef_t stThdDef;
  osTimerDef_t stTmDef;

  pstMusic->saved = 0;
  pstMusic->pstFid = (FIL *)hs_malloc(sizeof(FIL), __MT_DMA);
  if(!pstMusic->pstFid)
    return PLAYER_ERR;  

  pstMsgDef = &pstMusic->stMsgDef;
  pstMsgDef->queue_sz = PLAYER_MSG_SIZE * sizeof(int);
  pstMsgDef->item_sz  = sizeof(int);
  pstMsgDef->items    = hs_malloc(pstMsgDef->queue_sz, __MT_Z_GENERAL);
  pstMusic->pstMsgId = oshalMessageCreate(pstMsgDef, NULL);
  if(!pstMusic->pstMsgId)
    return PLAYER_ERR;  

  stThdDef.pthread   = (os_pthread)_music_thread;
  stThdDef.stacksize = 2048;
  stThdDef.tpriority = osPriorityNormal;     
  pstMusic->pstThd  = oshalThreadCreate(&stThdDef, pstMusic); 
  if(!pstMusic->pstThd)
    return PLAYER_ERR;  

  stTmDef.ptimer = _music_loadDec;
  pstMusic->pstTimer = oshalTimerCreate(&stTmDef, osTimerOnce, (void *)pstMusic);
  if(!pstMusic->pstTimer)
    return PLAYER_ERR;
  
  pstMusic->defaultEqNum = hs_music_getDefaultEqNum();
  return PLAYER_OK;
}

static int _music_load(hs_musicinfo_t *pstMusic, TCHAR *filename)
{
  hs_adectype_t eType;
  uint32_t *pu32Offset;
  
  if(pstMusic->pstAdec)
  {
    hs_ao_stop(pstMusic->pstAo);
    pstMusic->eStatus = MUSIC_STATUS_PAUSE;
    msleep(50);
    f_close(pstMusic->pstFid);
  }

  if(f_open(pstMusic->pstFid, filename, FA_READ) != FR_OK)
  {
    msleep(10);
    return PLAYER_ERR;
  }

  if(wstrstr(filename, _T(".mp3")) != 0)
    eType = ADEC_TYPE_MP3;
  else if(wstrstr(filename, _T(".wav")) != 0)
    eType = ADEC_TYPE_WAV;    
  else if(wstrstr(filename, _T(".wma")) != 0)
    eType = ADEC_TYPE_WMA; 
  else
    eType = ADEC_TYPE_UNKNOWN;  

  pu32Offset = pstMusic->current < pstMusic->sdNum ?
               &pstMusic->stCfg.sd_offset :
               &pstMusic->stCfg.udisk_offset;
                 
  if(0)//(pstMusic->pstAdec != NULL && eType == pstMusic->pstAdec->eAdecType)
  {
    *pu32Offset = 0;
    hs_adec_Reinit(pstMusic->pstAdec, pstMusic->pstFid);    
  }
  else
  {    
    if(pstMusic->pstAdec != NULL)
      hs_adec_destroy(pstMusic->pstAdec);
    
    pstMusic->pstAdec = hs_adec_creat(eType, pstMusic->pstFid, pstMusic->pstAo, *pu32Offset);
    if(!pstMusic->pstAdec)
    {
      f_close(pstMusic->pstFid);
      return PLAYER_ERR;
    }

    *pu32Offset = 0;
  }
  
  if(hs_adec_skip(pstMusic->pstAdec) != PLAYER_OK)   //skip nonsupport file, play next file
    return PLAYER_ERR;

  uint8_t *pu8Ptr;
  pu8Ptr = pstMusic->current >= pstMusic->sdNum ? 
            pstMusic->stCfg.udisk_name :
            pstMusic->stCfg.sd_name;    
  memcpy(pu8Ptr, filename, wstrlen(filename) * sizeof(TCHAR));
    
  hs_cfg_setDataByIndex(HS_CFG_MISC_MP3, (uint8_t *)&pstMusic->stCfg, sizeof(hs_musicpara_t), 0);
  hs_cfg_flush(FLUSH_TYPE_ALL);

  return PLAYER_OK;
}

static void _music_loadDec(void const *arg)
{
  hs_musicinfo_t *pstMusic = (hs_musicinfo_t *)arg;
  TCHAR *filename = (TCHAR *)pstMusic->pTimerArg;

  if(0 == _music_load(pstMusic, filename))
  {    
    if(pstMusic->eStatus != MUSIC_STATUS_PLAYING)
    {
      hs_ao_start(pstMusic->pstAo);      
      pstMusic->eStatus = MUSIC_STATUS_PLAYING;
      oshalSignalSet(pstMusic->pstThd, PLAYER_SIGNAL_WAKEUP); 
    }
  }
  else
  {
    hs_music_stop(pstMusic);
    hs_music_next(pstMusic);
  }
}

static inline void _music_msgHandlerLoad(hs_musicinfo_t *pstMusic, TCHAR *p)
{
  hs_music_stop(pstMusic);
  
  if(pstMusic->current < pstMusic->sdNum)
  {
    pstMusic->stCfg.sd_offset    = 0;
    pstMusic->stCfg.sd_t         = 0;
    memset(pstMusic->stCfg.sd_name, 0, NAME_MAX_NUM);
  }
  else
  {
    pstMusic->stCfg.udisk_offset = 0;
    pstMusic->stCfg.udisk_t      = 0;
    memset(pstMusic->stCfg.udisk_name, 0, NAME_MAX_NUM);
  }

  #if HS_USE_LEDDISP
  hs_led_disp(LED_DISP_MUSICIDX);
  #endif

  if(pstMusic->pstTimer)
  {
    oshalTimerStop(pstMusic->pstTimer);

    pstMusic->pTimerArg = p;
    oshalTimerStart(pstMusic->pstTimer, 1000);
  }
}

static int _music_msgHandler(hs_musicinfo_t *pstMusic, hs_musicmsg_t msg)
{
  int ret = 0;
  TCHAR *p;

  switch(msg)
  {
  case PLAY_PAUSE:
    hs_ao_stop(pstMusic->pstAo);
    pstMusic->eStatus = MUSIC_STATUS_PAUSE;

    oshalSignalClear(curthread(), PLAYER_SIGNAL_WAIT);
    oshalSignalWait(PLAYER_SIGNAL_WAIT, -1);
    break;
  case PLAY_START:
    if(pstMusic->eStatus != MUSIC_STATUS_PAUSE)
    {
      p = _music_getCurr(pstMusic);
      if(!p)
      {
        hs_music_stop(pstMusic);
        hs_cfg_sysSendMessage(HS_CFG_MODULE_SYS, HS_CFG_SYS_STATUS, HS_CFG_STATUS_MP3_NOT_FOUND);
      }
      else
      {
        if(_music_load(pstMusic, p) != 0)
        {
          hs_music_stop(pstMusic);
          hs_music_next(pstMusic);
        }
      }
    }

    if(pstMusic->eStatus == MUSIC_STATUS_PLAYING)
    {
      hs_ao_stop(pstMusic->pstAo);
      pstMusic->eStatus = MUSIC_STATUS_STOPED;
    }

    hs_ao_start(pstMusic->pstAo);
    pstMusic->eStatus = MUSIC_STATUS_PLAYING;
    break;
  case PLAY_NEXT:
    p = _music_getNext(pstMusic);
    _music_msgHandlerLoad(pstMusic, p);
    break;
  case PLAY_PREV:
    p = _music_getPrev(pstMusic);
    _music_msgHandlerLoad(pstMusic, p);
    break;
  case PLAY_STOP:
    hs_ao_stop(pstMusic->pstAo);
    pstMusic->eStatus = MUSIC_STATUS_STOPED;
    break;
  default:
    break;
  }

  return ret;
}

void _music_thread(void* arg)
{
  hs_musicinfo_t *pstMusic = (hs_musicinfo_t *)arg;
  osEvent eEvt;
  hs_musicmsg_t eMsg;

  chRegSetThreadName("mp3Service");
  while(1)
  {
    eEvt = oshalMessageGet(pstMusic->pstMsgId, 0);
    eMsg = (hs_musicmsg_t)eEvt.value.v;
    if((pstMusic->eStatus == MUSIC_STATUS_TERMINATE) 
      || (eMsg == PLAY_OVER))
    {
      _music_saveContext(pstMusic);
      break;
    }
    
    switch(eEvt.status)
    {
    case osEventTimeout:    
      if(pstMusic->eStatus == MUSIC_STATUS_STOPED)
      {
        oshalSignalClear(curthread(), PLAYER_SIGNAL_WAIT);
        oshalSignalWait(PLAYER_SIGNAL_WAIT, -1);
        break;
      }
      
      if(hs_adec_run(pstMusic->pstAdec) == -1)
      {
        if(_music_chkFileExist(pstMusic) == 0)
        {
          _music_msgHandler(pstMusic, PLAY_NEXT);
        }
        else
        {
          hs_printf("file system wrong\r\n");

          pstMusic->eStatus = MUSIC_STATUS_STOPED;          
          msleep(10);
        }
      }

      break;
    case osEventMessage:
      _music_msgHandler(pstMusic, eMsg);
      break;
    default:
      break;
    }
  }
  
  hs_printf("status: %d, msg: 0x%x\r\n", pstMusic->eStatus, eMsg);
  hs_adec_destroy(pstMusic->pstAdec);
}

void _music_eqChange(hs_musicinfo_t *pstMusic)
{
  if((!pstMusic) || (!pstMusic->pstMsgId))
    return ;

  pstMusic->stCfg.eqIdx += 1;
  pstMusic->stCfg.eqIdx %= pstMusic->defaultEqNum;  
  hs_codec_setEq(hs_music_getDefaultEqData(pstMusic->stCfg.eqIdx));

  hs_cfg_setDataByIndex(HS_CFG_MISC_MP3, (uint8_t *)&pstMusic->stCfg.eqIdx, 1, 5);
}

void _music_modeChange(hs_musicinfo_t *pstMusic)
{
  uint8_t i;
  
  if((!pstMusic) || (!pstMusic->pstMsgId))
    return ;

  for(i=0; i<=REPEAT_MODE_NUM; i++)
  {
    pstMusic->stCfg.playMode += 1;
    pstMusic->stCfg.playMode %= REPEAT_MODE_NUM;  

    if((1u << pstMusic->stCfg.playMode) & pstMusic->stCfg.mode_map)
      break;
  }

  hs_cfg_setDataByIndex(HS_CFG_MISC_MP3, (uint8_t *)&pstMusic->stCfg.playMode, 1, 1);
}

void _music_memIn(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;

  g_u16LastPos = (((uint32_t)parg) & 1) + MUSIC_POS_BASE;
}

void _music_memOut(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;
  (void)parg;
  hs_musicinfo_t *pstMusic = g_pstMusic;
  hs_musicpara_t *pstCfg = &pstMusic->stCfg;

  if(!pstMusic)
    return ;

  _music_saveContext(pstMusic);
  if((!hs_fatfs_isMount(FATFS_MEMDEV_SD)) 
    && (!hs_fatfs_isMount(FATFS_MEMDEV_UDISK)))
  {
    audioPlayMute();
    g_u16LastPos = 0;
    return ;
  }

  if(hs_fatfs_isMount(FATFS_MEMDEV_SD))
  {
    pstMusic->minIdx = 0;
    pstMusic->maxIdx = pstMusic->sdNum;

    hs_printf("current:%d, sdnum: %d \r\n", pstMusic->current, pstMusic->sdNum);

    if(pstMusic->current >= pstMusic->sdNum)
    {
      hs_cfg_playerReq(HS_CFG_EVENT_PLAYER_MUSIC_SD);
      pstMusic->current = _music_searchByName(pstMusic, (TCHAR *)pstCfg->sd_name, 1);
      if(pstMusic->current < 0)
        pstMusic->current = 0;

      audioPlayMute();
      pstMusic->eStatus = MUSIC_STATUS_STOPED;
      oshalMessagePut(pstMusic->pstMsgId, PLAY_START, -1); 
      if(pstMusic->eStatus != MUSIC_STATUS_PLAYING)
        oshalSignalSet(pstMusic->pstThd, PLAYER_SIGNAL_WAKEUP);  
    }

    pstMusic->udiskNum = 0;
  }
  else
  {
    pstMusic->minIdx = pstMusic->sdNum;
    pstMusic->maxIdx = pstMusic->amount;

    if(pstMusic->current < pstMusic->sdNum)
    {
      hs_cfg_playerReq(HS_CFG_EVENT_PLAYER_MUSIC_UDISK);
      pstMusic->current = _music_searchUdisk(pstMusic);
      if(pstMusic->current < 0)
        pstMusic->current = 0;

      audioPlayMute();
      pstMusic->eStatus = MUSIC_STATUS_STOPED;
      oshalMessagePut(pstMusic->pstMsgId, PLAY_START, -1); 
      if(pstMusic->eStatus != MUSIC_STATUS_PLAYING)
        oshalSignalSet(pstMusic->pstThd, PLAYER_SIGNAL_WAKEUP);  
    }

    pstMusic->sdNum = 0;
  }  
}

int _music_dataChange(hs_musicinfo_t *pstMusic, int32_t s32Step)
{
  uint32_t u32Sample, u32ChnNum, u32ByteRate = 0;
  hs_adecmp3_t* decoder;
  MP3DecInfo *pstDecInfo;
  int32_t s32Frames;
  
  if((!pstMusic->pstAdec) || (!pstMusic->pstAo))
    return -1;

  u32Sample = pstMusic->pstAo->stI2sCfg.sample_rate;
  u32ChnNum = pstMusic->pstAo->stI2sCfg.i2s_mode == I2S_PCMMODE_STEREO ? 2 : 1;

  if(pstMusic->pstAdec->eAdecType == ADEC_TYPE_MP3)
  {
    decoder = (hs_adecmp3_t *)pstMusic->pstAdec->decoder;
    pstDecInfo = (MP3DecInfo *)decoder->decoder;
    
    u32ByteRate = pstDecInfo->bitrate / 8;    
  }
  else if(pstMusic->pstAdec->eAdecType == ADEC_TYPE_WAV)
  {
    u32ByteRate = 2 * u32ChnNum * u32Sample;
  }
  else
  {
    u32ByteRate = 0;
  }

  if(u32ByteRate == 0)
    return -1;

  s32Frames = hs_adec_context(pstMusic->pstAdec) + s32Step * u32ByteRate;
  s32Frames = s32Frames < 0 ? 0 : s32Frames;
  hs_adec_ffRew(pstMusic->pstAdec, s32Frames);

  #if HS_USE_LEDDISP
  hs_led_disp(LED_DISP_MODE_CHANGE);
  #endif
  
  return 0;
}


hs_musicinfo_t *hs_music_create(hs_ao_t *pstAo)
{
  hs_musicinfo_t *pstMusic;

  if(g_pstMusic)
    hs_music_destroy(g_pstMusic);

  if((!hs_fatfs_isMount(FATFS_MEMDEV_SD)) 
    && (!hs_fatfs_isMount(FATFS_MEMDEV_UDISK)))
    return NULL;
  
  pstMusic = (hs_musicinfo_t *)hs_malloc(sizeof(hs_musicinfo_t), __MT_Z_GENERAL);
  if(!pstMusic)
    return NULL;

  pstMusic->pstAo       = pstAo;
  pstMusic->pstFileInfo = (hs_fileinfo_t *)hs_malloc(sizeof(hs_fileinfo_t) * MAX_PLAYLIST_SIZE, __MT_Z_GENERAL);
  if(!pstMusic->pstFileInfo)
  {
    hs_free(pstMusic);
    return NULL;
  }

  _music_initCfg(&pstMusic->stCfg);  
  if(PLAYER_OK != _music_initServ(pstMusic))
  {
    hs_music_destroy(pstMusic);
    return NULL;
  }

  pstMusic->eStatus     = MUSIC_STATUS_STOPED;
  pstMusic->buffer_size = AVERAGE_FILENAME_LENGTH * MAX_PLAYLIST_SIZE;
  pstMusic->buffer = (TCHAR *)hs_malloc(pstMusic->buffer_size, __MT_GENERAL);
  if(!pstMusic->buffer)
  {
    hs_music_destroy(pstMusic);
    return NULL;
  }

  pstMusic->max_playlist_size = MAX_PLAYLIST_SIZE;   
  g_pstMusic = pstMusic;
  
  pstMusic->sdNum    =_music_scan(pstMusic, MUSIC_DIR0, 0);
  pstMusic->udiskNum =_music_scan(pstMusic, MUSIC_DIR1, 0);
  _music_listSort(pstMusic, pstMusic->stCfg.sortType);

  if((g_u16LastPos != MUSIC_POS_SD) && (g_u16LastPos != MUSIC_POS_UDISK))
    g_u16LastPos = pstMusic->stCfg.musicPos == 0 ? MUSIC_POS_SD : MUSIC_POS_UDISK;
  pstMusic->current = _music_getCurrIdx(pstMusic);

  if((pstMusic->current >= pstMusic->sdNum) && (pstMusic->udiskNum > 0))
  {
    pstMusic->minIdx = pstMusic->sdNum;
    pstMusic->maxIdx = pstMusic->amount;

    hs_cfg_playerReq(HS_CFG_EVENT_PLAYER_MUSIC_UDISK);
  }
  
  if((pstMusic->current < pstMusic->sdNum) && (pstMusic->sdNum > 0))
  {
    pstMusic->minIdx = 0;
    pstMusic->maxIdx = pstMusic->sdNum;

    hs_cfg_playerReq(HS_CFG_EVENT_PLAYER_MUSIC_SD);
  }  

  hs_cfg_sysListenMsg(HS_CFG_EVENT_MEMDEV_IN,  _music_memIn);  
  hs_cfg_sysListenMsg(HS_CFG_EVENT_MEMDEV_OUT, _music_memOut);  

  if((pstMusic->minIdx == 0) && (pstMusic->maxIdx == 0))
  {
    hs_music_destroy(pstMusic);
    return NULL;
  }
  
  return pstMusic;
}

void hs_music_destroy(hs_musicinfo_t *pstMusic)
{
  if(!pstMusic)
    return ;

  hs_cfg_sysCancelListenMsg(HS_CFG_EVENT_MEMDEV_OUT,  _music_memOut);    
  pstMusic->eStatus = MUSIC_STATUS_TERMINATE;
  if(pstMusic->pstMsgId)
  {
    while(pstMusic->eStatus == MUSIC_STATUS_PLAYING)
      msleep(1);

    msleep(10);
    oshalMessagePut(pstMusic->pstMsgId, PLAY_OVER, -1);
    oshalSignalSet(pstMusic->pstThd, PLAYER_SIGNAL_WAKEUP);
  }

  if(pstMusic->pstTimer)
  {
    oshalTimerDelete(pstMusic->pstTimer);
  }
  
  if(pstMusic->pstThd)
  {
    oshalThreadTerminate(pstMusic->pstThd);
    pstMusic->pstThd = NULL;
  }

  if(pstMusic->pstMsgId)
  {    
    oshalMessageFree(pstMusic->pstMsgId);

    hs_free(pstMusic->stMsgDef.items);    
    pstMusic->pstMsgId = NULL;
  }  

  g_pstMusic = NULL;
  if(pstMusic->pstFid)
    hs_free(pstMusic->pstFid);
  
  if(pstMusic->pstFileInfo)
    hs_free(pstMusic->pstFileInfo);

  if(pstMusic->buffer)
    hs_free(pstMusic->buffer);

  hs_adec_destroy(pstMusic->pstAdec);
  hs_free(pstMusic);
}


void hs_music_start(hs_musicinfo_t *pstMusic)
{
  if((!pstMusic) || (!pstMusic->pstMsgId))
    return ;

  if(pstMusic->eStatus == MUSIC_STATUS_PLAYING)
  {    
    oshalMessagePut(pstMusic->pstMsgId, PLAY_PAUSE, -1);
    return ;
  }

  oshalMessagePut(pstMusic->pstMsgId, PLAY_START, -1); 
  if(pstMusic->eStatus != MUSIC_STATUS_PLAYING)
    oshalSignalSet(pstMusic->pstThd, PLAYER_SIGNAL_WAKEUP);  
}

void hs_music_stop(hs_musicinfo_t *pstMusic)
{
  if((!pstMusic) || (!pstMusic->pstMsgId))
    return ;  

  oshalMessagePut(pstMusic->pstMsgId, PLAY_STOP, -1);
}

void hs_music_next(hs_musicinfo_t *pstMusic)
{
  if((!pstMusic) || (!pstMusic->pstMsgId))
    return ;
  
  oshalMessagePut(pstMusic->pstMsgId, PLAY_NEXT, -1);  
  if(pstMusic->eStatus != MUSIC_STATUS_PLAYING)
    oshalSignalSet(pstMusic->pstThd, PLAYER_SIGNAL_WAKEUP);
}

void hs_music_prev(hs_musicinfo_t *pstMusic)
{
  if((!pstMusic) || (!pstMusic->pstMsgId))
    return ;  

  oshalMessagePut(pstMusic->pstMsgId, PLAY_PREV, -1);
  if(pstMusic->eStatus != MUSIC_STATUS_PLAYING)
    oshalSignalSet(pstMusic->pstThd, PLAYER_SIGNAL_WAKEUP);
}

void hs_music_volumeInc(hs_musicinfo_t *pstMusic)
{
  hs_musicpara_t *pstCfg = &pstMusic->stCfg;

  if((!pstMusic) || (!pstMusic->pstMsgId))
    return ;

  pstCfg->volume = hs_audio_volGet(AVOL_DEV_NOR) + pstCfg->volume_step;
  hs_ao_setVol(pstMusic->pstAo, pstCfg->volume);
}

void hs_music_volumeDec(hs_musicinfo_t *pstMusic)
{  
  hs_musicpara_t *pstCfg = &pstMusic->stCfg;

  if((!pstMusic) || (!pstMusic->pstMsgId))
    return ;
  
  pstCfg->volume = hs_audio_volGet(AVOL_DEV_NOR) - pstCfg->volume_step;
  hs_ao_setVol(pstMusic->pstAo, pstCfg->volume);
}

void hs_music_funcSet(hs_musicinfo_t *pstMusic, uint32_t u32Fun)
{  
  hs_musicpara_t *pstCfg = &pstMusic->stCfg;
  
  if((!pstMusic) || (!pstMusic->pstMsgId) || (u32Fun >= MUSIC_FUN_NUM))
    return ;

  if(u32Fun == MUSIC_FUN_PLAYMODE)
    _music_modeChange(pstMusic);
  else if(u32Fun == MUSIC_FUN_EQ)
    _music_eqChange(pstMusic);
  else if(u32Fun == MUSIC_FF)
    _music_dataChange(pstMusic, pstCfg->ff_rew_step);
  else if(u32Fun == MUSIC_REW)
    _music_dataChange(pstMusic, 0 - pstCfg->ff_rew_step);
  else if(u32Fun == MUSIC_FFREW_STOP)
    hs_adec_ffRewExit(pstMusic->pstAdec);
  else
    return ;
}

int hs_music_changeDev(uint8_t u8Msg)
{
  hs_musicinfo_t *pstMusic = g_pstMusic;
  hs_musicpara_t *pstCfg = &pstMusic->stCfg;
  
  if(!pstMusic)
    return -1;

  _music_saveContext(pstMusic);
  g_u16LastPos = 0;
  if(pstMusic->current >= pstMusic->sdNum)
  {
    if(pstMusic->sdNum == 0)
      return -1;

    pstMusic->minIdx = 0;
    pstMusic->maxIdx = pstMusic->sdNum;

    if(u8Msg)
      hs_cfg_playerReq(HS_CFG_EVENT_PLAYER_MUSIC_SD);

    pstMusic->current = _music_searchByName(pstMusic, (TCHAR *)pstCfg->sd_name, 1);
    if(pstMusic->current < 0)
      pstMusic->current = 0;
  }
  else
  {
    if(pstMusic->udiskNum == 0)
      return -1;

    pstMusic->minIdx = pstMusic->sdNum;
    pstMusic->maxIdx = pstMusic->amount;

    if(u8Msg)
      hs_cfg_playerReq(HS_CFG_EVENT_PLAYER_MUSIC_UDISK);

    pstMusic->current = _music_searchUdisk(pstMusic);
    if(pstMusic->current < 0)
      pstMusic->current = 0;
  }

  pstMusic->eStatus = MUSIC_STATUS_STOPED;
  oshalMessagePut(pstMusic->pstMsgId, PLAY_START, -1); 
  oshalSignalSet(pstMusic->pstThd, PLAYER_SIGNAL_WAKEUP);  

  return 0;
}

__USED uint32_t hs_music_getTime(void)
{
  hs_musicinfo_t *pstMusic = g_pstMusic;
  uint32_t u32Tim, u32Sample, u32ChnNum, u32BitRate = 0;
  hs_adecmp3_t* decoder;
  MP3DecInfo *pstDecInfo;
  
  if(!pstMusic)
    return 0;

  if((!pstMusic->pstAdec) || (!pstMusic->pstAo))
    return 0;

  u32Sample = pstMusic->pstAo->stI2sCfg.sample_rate;
  u32ChnNum = pstMusic->pstAo->stI2sCfg.i2s_mode == I2S_PCMMODE_STEREO ? 2 : 1;

  if(pstMusic->pstAdec->eAdecType == ADEC_TYPE_MP3)
  {
    decoder = (hs_adecmp3_t *)pstMusic->pstAdec->decoder;
    if(!decoder)
      return 0;
    
    pstDecInfo = (MP3DecInfo *)decoder->decoder;
    if(!pstDecInfo)
      return 0;
    
    u32BitRate = pstDecInfo->bitrate;
    if(u32BitRate == 0)
      return 0;

    u32Tim = pstMusic->current < pstMusic->sdNum ? pstMusic->stCfg.sd_t : pstMusic->stCfg.udisk_t;
    u32Tim += decoder->ms / 1000;
  }
  else if(pstMusic->pstAdec->eAdecType == ADEC_TYPE_WAV)
    u32Tim = pstMusic->pstAdec->u32Frames / 2 / u32ChnNum / u32Sample;
  else
    u32Tim = 0;

  return u32Tim;
}

__USED uint32_t hs_music_getIdx(void)
{
  hs_musicinfo_t *pstMusic = g_pstMusic;
  uint32_t u32Idx;
  
  if(!pstMusic)
    return 0;

  u32Idx = pstMusic->current;
  if(pstMusic->current >= pstMusic->sdNum)
    u32Idx = (pstMusic->current - pstMusic->minIdx);

  u32Idx += 1;
  return u32Idx;
}

__USED void hs_music_setIdx(uint32_t u32Idx)
{
  hs_musicinfo_t *pstMusic = g_pstMusic;
  
  if((!pstMusic) || (u32Idx == 0))
    return ;

  if(pstMusic->current >= pstMusic->sdNum)
  {
    u32Idx = pstMusic->minIdx + u32Idx - 1;

    if(u32Idx >= (uint32_t)pstMusic->amount)
      return ;
  }
  else
  {
    u32Idx = u32Idx - 1;
    
    if(u32Idx >= pstMusic->sdNum)
      return ;
  }

  audioPlayMute();
  oshalMessagePut(pstMusic->pstMsgId, PLAY_STOP, -1);
  pstMusic->current = u32Idx;
  oshalMessagePut(pstMusic->pstMsgId, PLAY_START, -1); 
  if(pstMusic->eStatus != MUSIC_STATUS_PLAYING)
    oshalSignalSet(pstMusic->pstThd, PLAYER_SIGNAL_WAKEUP);  
}

__USED hs_musicstatus_t hs_music_getStatus(void)
{
  hs_musicinfo_t *pstMusic = g_pstMusic;

  if(!pstMusic)
    return MUSIC_STATUS_TERMINATE;

  return pstMusic->eStatus;
}

__USED hs_musicdevs_t hs_music_getCurDev(void)
{
  hs_musicinfo_t *pstMusic = g_pstMusic;

  if(!pstMusic)
    return MUSIC_DEVICE_NULL;

  if((pstMusic->current >= pstMusic->sdNum) && (pstMusic->udiskNum > 0))
    return MUSIC_DEVICE_UDISK;
  else if ((pstMusic->current < pstMusic->sdNum) && (pstMusic->sdNum > 0))
    return MUSIC_DEVICE_SD;
  else
    return MUSIC_DEVICE_NULL;

  return MUSIC_DEVICE_NULL;
}

__USED int hs_music_getEqIdx(void)
{
  hs_musicinfo_t *pstMusic = g_pstMusic;
  hs_musicpara_t *pstCfg = &pstMusic->stCfg;

  if(!pstMusic)
    return MUSIC_DEVICE_NULL;

  return pstCfg->eqIdx;
}

__USED int hs_music_getModeIdx(void)
{
  hs_musicinfo_t *pstMusic = g_pstMusic;
  hs_musicpara_t *pstCfg = &pstMusic->stCfg;

  if(!pstMusic)
    return MUSIC_DEVICE_NULL;

  return pstCfg->playMode;
}

__USED hs_musicinfo_t * hs_music_getHandle(void)
{
  return g_pstMusic;
}

void hs_music_volumeIncBig(hs_musicinfo_t *pstMusic)
{
  hs_musicpara_t *pstCfg = &pstMusic->stCfg;
  int max;

  if((!pstMusic) || (!pstMusic->pstMsgId))
    return ;

  pstCfg->volume += pstCfg->volume_step * 10;
  max = audioPlayGetVolumeMax(AUDIO_PLY_SRC_MP3);
  if(pstCfg->volume > max)
    pstCfg->volume = max;
  
  hs_cfg_setDataByIndex(HS_CFG_MISC_MP3, (uint8_t *)&pstMusic->stCfg.volume, 1, 2);
  hs_ao_setVol(pstMusic->pstAo, pstCfg->volume);
}

void hs_music_volumeDecBig(hs_musicinfo_t *pstMusic)
{  
  hs_musicpara_t *pstCfg = &pstMusic->stCfg;
  int16_t min;

  if((!pstMusic) || (!pstMusic->pstMsgId))
    return ;
  
  pstCfg->volume -= pstCfg->volume_step * 10;
  min = audioPlayGetVolumeMin(AUDIO_PLY_SRC_MP3);
  if(pstCfg->volume < min)
    pstCfg->volume = min;

  hs_cfg_setDataByIndex(HS_CFG_MISC_MP3, (uint8_t *)&pstMusic->stCfg.volume, 1, 2);
  hs_ao_setVol(pstMusic->pstAo, pstCfg->volume);
}

#endif

