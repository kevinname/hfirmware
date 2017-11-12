#ifndef __PLAYER_FM_H__
#define __PLAYER_FM_H__

#if HS_USE_FM

#include "ao.h"
#include "hal.h"

enum
{
  FM_FUNC_SEARCH      = 0,
  FM_FUNC_INVALIDATE     ,
  FM_FUNC_ABORT_SEARCH   ,
  FM_FUNC_SCAN_SEMI_NEXT ,
  FM_FUNC_SCAN_SEMI_PREV ,

  FM_FUNC_NUM
};

typedef enum
{
  FM_STATUS_SCANNING  = 0,
  FM_STATUS_PLAYING   ,
  FM_STATUS_PAUSE     ,
  FM_STATUS_TERMINATE ,
}hs_fmstatus_t;

typedef struct _CfgFmParm 
{
  int8_t volume;
  int8_t volume_step;
  fm_th_t th;
  int frequency;
  int freq_step;
  int freq_max;
  int freq_min;
  uint8_t chan_vip;
  uint8_t scan_found_delay;
  uint8_t scan_chns_max;
  uint8_t scan_chns_avail;
}hs_fmpara_t;

typedef struct _fm_player
{
  hs_fmpara_t         stPara;
  FMConfig            fmconfig;  
  hs_ao_t            *pstAo;

  osThreadId          FmScanThreadId;
  volatile uint8_t    start_flag;
  uint8_t             paused;
  uint8_t             scan_flag;
  volatile uint8_t    abort_scan;
  int                 freq_vip;
  int                *fm_tbl_freq;
  int                *fm_tbl_ctx;
  uint8_t             chan_idx;
  uint8_t             scan_1by1;
}hs_fm_t;

hs_fm_t *hs_fm_create(hs_ao_t *pstAo);
void hs_fm_destroy(hs_fm_t *pstFm);
void hs_fm_start(hs_fm_t *pstFm);
void hs_fm_stop(hs_fm_t *pstFm);
void hs_fm_next(hs_fm_t *pstFm);
void hs_fm_prev(hs_fm_t *pstFm);
void hs_fm_volumeInc(hs_fm_t *pstFm);
void hs_fm_volumeDec(hs_fm_t *pstFm);
void hs_fm_freqInc(hs_fm_t *pstFm);
void hs_fm_freqDec(hs_fm_t *pstFm);
void hs_fm_funcSet(hs_fm_t *pstFm, uint32_t u32Func);
__USED uint32_t hs_fm_getFreq(void);
__USED bool hs_fm_isScan(void);
__USED hs_fmstatus_t hs_fm_getStatus(void);
__USED void hs_fm_setIdx(uint32_t u32Idx);
__USED hs_fm_t *hs_fm_getHandle(void);

#endif

#endif
