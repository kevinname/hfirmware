/*
    audio plaer - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/audio/dec/mp3.h
 * @brief   include file.
 * @details 
 *
 * @addtogroup  decoder
 * @details 
 * @{
 */
#ifndef __LIB_ADEC_MP3_H__
#define __LIB_ADEC_MP3_H__

#if HS_USE_MP3

#include "libmp3/pub/mp3dec.h"
#include "adec.h"

typedef struct
{
  hs_adec_t       stAdec;

  short outBuf[MAX_NCHAN * MAX_NGRAN * MAX_NSAMP];

  /* mp3 read session */
  uint8_t  *read_buffer, *read_ptr;
  int32_t read_offset;
  uint32_t bytes_left, bytes_left_before_decoding;
  uint32_t frames, ms;

  /* mp3 information */
  HMP3Decoder decoder;
  MP3FrameInfo frame_info;
}hs_adecmp3_t;

hs_adec_t* hs_adec_mp3Create(void);

#endif

#endif
