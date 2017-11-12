#ifndef __AUDIO_PLAYER_H__
#define __AUDIO_PLAYER_H__

#if HS_USE_PLAYER

#include "afm.h"
#include "linein.h"
#include "usbaudio.h"
#include "music.h"
#include "ao.h"

enum
{
  PLAYER_ERR    = -1,
  PLAYER_OK     = 0,
};

typedef enum 
{
  PLAYER_WORKMODE_FM        ,
  PLAYER_WORKMODE_MP3       ,
  PLAYER_WORKMODE_LINEIN    ,
  PLAYER_WORKMODE_USBAUDIO  ,
  PLAYER_WORKMODE_UNKNOWN   ,
}hs_playermode_t;
  
void hs_player_start(uint16_t u16Idx, void *parg);
void hs_player_stop(uint16_t u16Idx, void *parg);
void hs_player_mute(uint16_t u16Idx, void *parg);
void hs_player_next(uint16_t u16Idx, void *parg);
void hs_player_prev(uint16_t u16Idx, void *parg);
void hs_player_volumeInc(uint16_t u16Idx, void *parg);
void hs_player_volumeDec(uint16_t u16Idx, void *parg);
void hs_player_volumeIncBig(uint16_t u16Idx, void *parg);
void hs_player_volumeDecBig(uint16_t u16Idx, void *parg);
void hs_player_freqInc(uint16_t u16Idx, void *parg);
void hs_player_freqDec(uint16_t u16Idx, void *parg);

void hs_player_funcSet(uint16_t u16Idx, void *parg);

#if HS_USE_MP3
void hs_player_createMusic(uint16_t u16Idx, void *parg);
#endif

#if HS_USE_FM
void hs_player_createFm(uint16_t u16Idx, void *parg);
#endif

#if HS_USE_AUX
void hs_player_createAux(uint16_t u16Idx, void *parg);
#endif

#if HAL_USE_USB_AUDIO
void hs_player_createUsbaudio(uint16_t u16Idx, void *parg);
#endif

void hs_player_destroy(uint16_t u16Idx, void *parg);

#endif

#endif
