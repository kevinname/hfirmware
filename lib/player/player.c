#include "lib.h"

#if HS_USE_PLAYER

typedef void (*hs_pfnPlayerOper_t)(void *arg);
typedef void (*hs_pfnPlayerFunc_t)(void *arg, uint32_t u32Fun);

typedef struct __playerInfoS
{
  #if HS_USE_MP3
  hs_musicinfo_t     *pstMusic;
  #endif

  #if HAL_USE_USB_AUDIO
  hs_usbaudio_t      *pstUsbaudio;
  #endif

  #if HS_USE_FM
  hs_fm_t            *pstFm;
  #endif

  #if HS_USE_AUX
  hs_linein_t        *pstLinein;
  #endif
  
  hs_ao_t            *pstAo;
  
  hs_playermode_t     eWorkMode;   
  osSemaphoreId       pstSemId; 
  uint16_t            mute;
  void               *uarg;

  hs_pfnPlayerOper_t  pfnStart;
  hs_pfnPlayerOper_t  pfnStop;
  hs_pfnPlayerOper_t  pfnMute;
  hs_pfnPlayerOper_t  pfnNext;
  hs_pfnPlayerOper_t  pfnPrev;
  hs_pfnPlayerOper_t  pfnVolumeInc;
  hs_pfnPlayerOper_t  pfnVolumeDec;
  hs_pfnPlayerOper_t  pfnVolumeIncBig;
  hs_pfnPlayerOper_t  pfnVolumeDecBig;
  hs_pfnPlayerOper_t  pfnFreqInc;
  hs_pfnPlayerOper_t  pfnFreqDec;  

  hs_pfnPlayerFunc_t  pfnSetFunc;
}hs_player_t;


static hs_player_t *g_pstPlayer;
static void _player_destroy(hs_player_t *pstPlayer)
{
  if(!pstPlayer)
    return ;   
  
  if(pstPlayer->pstSemId)
  {
    oshalSemaphoreDelete(pstPlayer->pstSemId);
    pstPlayer->pstSemId = NULL;
  }  
  
  hs_free(pstPlayer);
}

hs_player_t *_player_create(void)
{
  hs_player_t *pstPlayer;  

  pstPlayer = (hs_player_t *)hs_malloc(sizeof(hs_player_t), __MT_Z_GENERAL);
  if(!pstPlayer)
    return NULL;
  
  pstPlayer->eWorkMode = PLAYER_WORKMODE_UNKNOWN;
  pstPlayer->pstSemId = oshalSemaphoreCreate(NULL, 1);
  if(!pstPlayer->pstSemId)
    goto __player_create_err;

  return pstPlayer;

__player_create_err:
  _player_destroy(pstPlayer);
  return NULL;
}

#if HS_USE_MP3
static void _player_initMusicOper(hs_player_t *pstPlayer)
{
  if(!pstPlayer)
    return ;

  pstPlayer->pfnStart     = (hs_pfnPlayerOper_t)hs_music_start;
  pstPlayer->pfnStop      = (hs_pfnPlayerOper_t)hs_music_stop;
  pstPlayer->pfnNext      = (hs_pfnPlayerOper_t)hs_music_next;
  pstPlayer->pfnPrev      = (hs_pfnPlayerOper_t)hs_music_prev;
  pstPlayer->pfnVolumeInc = (hs_pfnPlayerOper_t)hs_music_volumeInc;
  pstPlayer->pfnVolumeDec = (hs_pfnPlayerOper_t)hs_music_volumeDec;
  
  pstPlayer->pfnVolumeIncBig = (hs_pfnPlayerOper_t)hs_music_volumeIncBig;
  pstPlayer->pfnVolumeDecBig = (hs_pfnPlayerOper_t)hs_music_volumeDecBig;

  pstPlayer->pfnSetFunc   = (hs_pfnPlayerFunc_t)hs_music_funcSet;

  pstPlayer->uarg         = pstPlayer->pstMusic;
}
#endif

#if HS_USE_FM
static void _player_initFmOper(hs_player_t *pstPlayer)
{
  if(!pstPlayer)
    return ;

  pstPlayer->pfnStart     = (hs_pfnPlayerOper_t)hs_fm_start;
  pstPlayer->pfnStop      = (hs_pfnPlayerOper_t)hs_fm_stop;
  pstPlayer->pfnNext      = (hs_pfnPlayerOper_t)hs_fm_next;
  pstPlayer->pfnPrev      = (hs_pfnPlayerOper_t)hs_fm_prev;
  pstPlayer->pfnVolumeInc = (hs_pfnPlayerOper_t)hs_fm_volumeInc;
  pstPlayer->pfnVolumeDec = (hs_pfnPlayerOper_t)hs_fm_volumeDec;
  pstPlayer->pfnFreqInc   = (hs_pfnPlayerOper_t)hs_fm_freqInc;
  pstPlayer->pfnFreqDec   = (hs_pfnPlayerOper_t)hs_fm_freqDec;

  pstPlayer->pfnSetFunc   = (hs_pfnPlayerFunc_t)hs_fm_funcSet;

  pstPlayer->uarg         = pstPlayer->pstFm;
}
#endif

#if HS_USE_AUX
static void _player_initLineinOper(hs_player_t *pstPlayer)
{
  if(!pstPlayer)
    return ;

  pstPlayer->pfnStart     = (hs_pfnPlayerOper_t)hs_linein_start;
  pstPlayer->pfnStop      = (hs_pfnPlayerOper_t)hs_linein_stop;
  pstPlayer->pfnVolumeInc = (hs_pfnPlayerOper_t)hs_linein_volumeInc;
  pstPlayer->pfnVolumeDec = (hs_pfnPlayerOper_t)hs_linein_volumeDec;

  pstPlayer->uarg         = pstPlayer->pstLinein;
}
#endif

#if HAL_USE_USB_AUDIO
static void _player_initUsbaudioOper(hs_player_t *pstPlayer)
{
  if(!pstPlayer)
    return ;

  pstPlayer->pfnVolumeInc = (hs_pfnPlayerOper_t)hs_usbaudio_volumeInc;
  pstPlayer->pfnVolumeDec = (hs_pfnPlayerOper_t)hs_usbaudio_volumeDec;

  pstPlayer->uarg         = pstPlayer->pstUsbaudio;
}
#endif

void hs_player_start(uint16_t u16Idx, void *parg)
{
  (void)u16Idx;
  (void)parg;
  
  if((!g_pstPlayer) || (!g_pstPlayer->pstSemId))
    return ;
  
  oshalSemaphoreWait(g_pstPlayer->pstSemId, -1); 
  
  if(g_pstPlayer->pfnStart)
    g_pstPlayer->pfnStart(g_pstPlayer->uarg);
  
  oshalSemaphoreRelease(g_pstPlayer->pstSemId);
}

void hs_player_stop(uint16_t u16Idx, void *parg)
{
  (void)u16Idx;
  (void)parg;
  
  if((!g_pstPlayer) || (!g_pstPlayer->pstSemId))
    return ;
  
  oshalSemaphoreWait(g_pstPlayer->pstSemId, -1);

  if(g_pstPlayer->pfnStop)
    g_pstPlayer->pfnStop(g_pstPlayer->uarg);

  oshalSemaphoreRelease(g_pstPlayer->pstSemId);
}

void hs_player_next(uint16_t u16Idx, void *parg)
{
  (void)u16Idx;
  (void)parg;
  
  if((!g_pstPlayer) || (!g_pstPlayer->pstSemId))
    return ;
  
  oshalSemaphoreWait(g_pstPlayer->pstSemId, -1);

  if(g_pstPlayer->pfnNext)
    g_pstPlayer->pfnNext(g_pstPlayer->uarg);
  
  oshalSemaphoreRelease(g_pstPlayer->pstSemId);
}

void hs_player_prev(uint16_t u16Idx, void *parg)
{
  (void)u16Idx;
  (void)parg;
  
  if((!g_pstPlayer) || (!g_pstPlayer->pstSemId))
    return ;
  
  oshalSemaphoreWait(g_pstPlayer->pstSemId, -1);

  if(g_pstPlayer->pfnPrev)
    g_pstPlayer->pfnPrev(g_pstPlayer->uarg);
  
  oshalSemaphoreRelease(g_pstPlayer->pstSemId);
}

void hs_player_volumeInc(uint16_t u16Idx, void *parg)
{
  (void)u16Idx;
  (void)parg;
  
  if((!g_pstPlayer) || (!g_pstPlayer->pstSemId))
    return ;
  
  oshalSemaphoreWait(g_pstPlayer->pstSemId, -1);

  if(g_pstPlayer->pfnVolumeInc)
  {
    g_pstPlayer->mute = 0;
    g_pstPlayer->pfnVolumeInc(g_pstPlayer->uarg);
  }

  oshalSemaphoreRelease(g_pstPlayer->pstSemId);
}

void hs_player_volumeDec(uint16_t u16Idx, void *parg)
{  
  (void)u16Idx;
  (void)parg;
  
  if((!g_pstPlayer) || (!g_pstPlayer->pstSemId))
    return ;
  
  oshalSemaphoreWait(g_pstPlayer->pstSemId, -1);

  if(g_pstPlayer->pfnVolumeDec)
  {
    g_pstPlayer->mute = 0;
    g_pstPlayer->pfnVolumeDec(g_pstPlayer->uarg);
  }

  oshalSemaphoreRelease(g_pstPlayer->pstSemId);
}

void hs_player_volumeIncBig(uint16_t u16Idx, void *parg)
{
  (void)u16Idx;
  (void)parg;
  
  if((!g_pstPlayer) || (!g_pstPlayer->pstSemId))
    return ;
  
  oshalSemaphoreWait(g_pstPlayer->pstSemId, -1);

  if(g_pstPlayer->pfnVolumeInc)
  {
    g_pstPlayer->mute = 0;
    g_pstPlayer->pfnVolumeIncBig(g_pstPlayer->uarg);
  }

  oshalSemaphoreRelease(g_pstPlayer->pstSemId);
}

void hs_player_volumeDecBig(uint16_t u16Idx, void *parg)
{  
  (void)u16Idx;
  (void)parg;
  
  if((!g_pstPlayer) || (!g_pstPlayer->pstSemId))
    return ;
  
  oshalSemaphoreWait(g_pstPlayer->pstSemId, -1);

  if(g_pstPlayer->pfnVolumeDec)
  {
    g_pstPlayer->mute = 0;
    g_pstPlayer->pfnVolumeDecBig(g_pstPlayer->uarg);
  }

  oshalSemaphoreRelease(g_pstPlayer->pstSemId);
}

void hs_player_freqInc(uint16_t u16Idx, void *parg)
{
  (void)u16Idx;
  (void)parg;
  
  if((!g_pstPlayer) || (!g_pstPlayer->pstSemId))
    return ;
  
  oshalSemaphoreWait(g_pstPlayer->pstSemId, -1);

  if(g_pstPlayer->pfnFreqInc)
    g_pstPlayer->pfnFreqInc(g_pstPlayer->uarg);

  oshalSemaphoreRelease(g_pstPlayer->pstSemId);
}

void hs_player_freqDec(uint16_t u16Idx, void *parg)
{  
  (void)u16Idx;
  (void)parg;
  
  if((!g_pstPlayer) || (!g_pstPlayer->pstSemId))
    return ;
  
  oshalSemaphoreWait(g_pstPlayer->pstSemId, -1);

  if(g_pstPlayer->pfnFreqDec)
    g_pstPlayer->pfnFreqDec(g_pstPlayer->uarg);

  oshalSemaphoreRelease(g_pstPlayer->pstSemId);
}

void hs_player_funcSet(uint16_t u16Idx, void *parg)
{  
  (void)u16Idx;
  
  if((!g_pstPlayer) || (!g_pstPlayer->pstSemId))
    return ;
  
  oshalSemaphoreWait(g_pstPlayer->pstSemId, -1);

  if(g_pstPlayer->pfnSetFunc)
    g_pstPlayer->pfnSetFunc(g_pstPlayer->uarg, (uint32_t)parg);

  oshalSemaphoreRelease(g_pstPlayer->pstSemId);
}

void hs_player_mute(uint16_t u16Idx, void *parg)
{  
  (void)u16Idx;
  (void)parg;
  
  if((!g_pstPlayer) || (!g_pstPlayer->pstSemId))
    return ;
  
  oshalSemaphoreWait(g_pstPlayer->pstSemId, -1);

  g_pstPlayer->mute = g_pstPlayer->mute == 1 ? 0 : 1;  
  if(g_pstPlayer->mute)
    audioPlayMute();
  else
    audioPlayUnmute();

  oshalSemaphoreRelease(g_pstPlayer->pstSemId);
}

#if HS_USE_MP3
void hs_player_createMusic(uint16_t u16Idx, void *parg)
{
  if(g_pstPlayer)
    hs_player_destroy(u16Idx, parg);
  
  g_pstPlayer = _player_create();
  if(!g_pstPlayer) 
    return ;

  g_pstPlayer->eWorkMode = PLAYER_WORKMODE_MP3; 
  g_pstPlayer->pstAo     = hs_ao_create(g_pstPlayer->eWorkMode);
  if(!g_pstPlayer->pstAo)
  {
    hs_player_destroy(u16Idx, parg);
    return ;
  }
  
  g_pstPlayer->pstMusic  = hs_music_create(g_pstPlayer->pstAo);
  if(!g_pstPlayer->pstMusic)
  {
    hs_player_destroy(u16Idx, parg);
    return ;
  }

  _player_initMusicOper(g_pstPlayer);
  hs_player_start(u16Idx, parg);
}
#endif

#if HS_USE_FM
void hs_player_createFm(uint16_t u16Idx, void *parg)
{
  if(g_pstPlayer)
    hs_player_destroy(u16Idx, parg);
  
  g_pstPlayer = _player_create();
  if(!g_pstPlayer) 
    return ;

  g_pstPlayer->eWorkMode = PLAYER_WORKMODE_FM; 
  g_pstPlayer->pstAo     = hs_ao_create(g_pstPlayer->eWorkMode);
  if(!g_pstPlayer->pstAo)
  {
    hs_player_destroy(u16Idx, parg);
    return ;
  }
  
  g_pstPlayer->pstFm  = hs_fm_create(g_pstPlayer->pstAo);
  if(!g_pstPlayer->pstFm)
  {
    hs_player_destroy(u16Idx, parg);
    return ;
  }
  
  _player_initFmOper(g_pstPlayer);
  hs_player_start(u16Idx, parg);
}
#endif

#if HS_USE_AUX
void hs_player_createAux(uint16_t u16Idx, void *parg)
{
  if(g_pstPlayer)
    hs_player_destroy(u16Idx, parg);
  
  g_pstPlayer = _player_create();
  if(!g_pstPlayer) 
    return ;

  g_pstPlayer->eWorkMode = PLAYER_WORKMODE_LINEIN; 
  g_pstPlayer->pstAo     = hs_ao_create(g_pstPlayer->eWorkMode);
  if(!g_pstPlayer->pstAo)
  {
    hs_player_destroy(u16Idx, parg);
    return ;
  }
  
  g_pstPlayer->pstLinein = hs_linein_create(g_pstPlayer->pstAo);
  if(!g_pstPlayer->pstLinein)
  {
    hs_player_destroy(u16Idx, parg);
    return ;
  }
  
  _player_initLineinOper(g_pstPlayer);
  hs_linein_start(g_pstPlayer->pstLinein);
}
#endif

#if HAL_USE_USB_AUDIO
void hs_player_createUsbaudio(uint16_t u16Idx, void *parg)
{
  if(g_pstPlayer)
    hs_player_destroy(u16Idx, parg);
  
  g_pstPlayer = _player_create();
  if(!g_pstPlayer) 
    return ;

  g_pstPlayer->eWorkMode = PLAYER_WORKMODE_USBAUDIO;   
  g_pstPlayer->pstUsbaudio = hs_usbaudio_create();
  if(!g_pstPlayer->pstUsbaudio)
  {
    hs_player_destroy(u16Idx, parg);
    return ;
  }
  
  _player_initUsbaudioOper(g_pstPlayer);
}
#endif

void hs_player_destroy(uint16_t u16Idx, void *parg)
{
  if(!g_pstPlayer)
    return ;

  audioPlayMute();
  hs_player_stop(u16Idx, parg);
  if(g_pstPlayer->pstSemId)
    oshalSemaphoreWait(g_pstPlayer->pstSemId, -1);  

  #if HS_USE_MP3
  hs_music_destroy(g_pstPlayer->pstMusic);
  g_pstPlayer->pstMusic = NULL;  
  #endif

  #if HS_USE_FM
  hs_fm_destroy(g_pstPlayer->pstFm);
  g_pstPlayer->pstFm = NULL;
  #endif

  #if HS_USE_AUX
  hs_linein_destroy(g_pstPlayer->pstLinein);
  g_pstPlayer->pstLinein = NULL;
  #endif

  #if HAL_USE_USB_AUDIO
  hs_usbaudio_destroy(g_pstPlayer->pstUsbaudio);
  g_pstPlayer->pstUsbaudio = NULL;
  #endif
  
  hs_ao_destroy(g_pstPlayer->pstAo);
  g_pstPlayer->pstAo = NULL;  

  if(g_pstPlayer->pstSemId)
    oshalSemaphoreRelease(g_pstPlayer->pstSemId);

  _player_destroy(g_pstPlayer);  
  g_pstPlayer = NULL;

  hs_cfg_flush(FLUSH_TYPE_ALL);
}

#endif

