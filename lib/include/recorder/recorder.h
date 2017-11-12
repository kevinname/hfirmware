/*
    recorder - Copyright (C) 2012~2017 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    recorder/recorder.h
 * @brief   recorder file.
 * @details 
 *
 * @addtogroup  recorder
 * @details 
 * @{
 */


#ifndef __AUDIO_RECORDER_H__
#define __AUDIO_RECORDER_H__

#define REC_DIRNAME_LEN         32
#define REC_FILEFIX_LEN         16


enum
{
  RECORDER_ERR    = -1,
  RECORDER_OK     = 0,
};

typedef struct
{
  uint8_t     u8DirName[REC_DIRNAME_LEN];
  uint8_t     u8FilePrefix[REC_FILEFIX_LEN];

  uint32_t    u32Sample;
  uint32_t    u32MaxNum;

  int16_t     s16Gain;
  uint16_t    u16Mode;    /* 0-stereo, 1-mono */
}hs_rec_cfg_t;

typedef FIL *  hs_pstFile_t;

#include "ai.h"
#include "rec_wr.h"

void hs_recorder_switch(uint16_t u16Idx, void *parg);
bool hs_recorder_isWorking(void);


#endif
