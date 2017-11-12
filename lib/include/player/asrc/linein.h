#ifndef __PLAYER_LINEIN_H__
#define __PLAYER_LINEIN_H__

#include "ao.h"

#if HS_USE_AUX

typedef struct _CfgLineinParm 
{
  int8_t volume;
  int8_t volume_step;

  int8_t reserve[2];
}hs_lineinpara_t;

typedef struct _linein_player
{
  hs_lineinpara_t     stPara;
  hs_ao_t            *pstAo;
  
  osThreadId          pstThd;
  uint32_t            u32Start;
}hs_linein_t;

void hs_linein_start(hs_linein_t *pstLinein);
void hs_linein_stop(hs_linein_t *pstLinein);
void hs_linein_volumeInc(hs_linein_t *pstLinein);
void hs_linein_volumeDec(hs_linein_t *pstLinein);

hs_linein_t *hs_linein_create(hs_ao_t *pstAo);
void hs_linein_destroy(hs_linein_t *pstLinein);
uint8_t hs_linein_isStarting(void);

#endif
  
#endif

