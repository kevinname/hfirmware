/*
    audio plaer - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/audio/dec/wav.h
 * @brief   include file.
 * @details 
 *
 * @addtogroup  decoder
 * @details 
 * @{
 */
#ifndef __LIB_ADEC_WAV_H__
#define __LIB_ADEC_WAV_H__

#include "adec.h"

#define WAVE_FORMAT_PCM             0x0001 /* PCM */
#define WAV_READ_BUFFER_SIZE        (4 * 1024)

typedef struct{
  char riff_sig[4];             // 'RIFF'
  long waveform_chunk_size;     // 8
  char wave_sig[4];             // 'WAVE'
  char format_sig[4];           // 'fmt ' (notice space after)
  long format_chunk_size;       // 16;
  short format_tag;             // WAVE_FORMAT_PCM
  short channels;               // # of channels
  long sample_rate;             // sampling rate
  long bytes_per_sec;           // bytes per second
  short block_align;            // sample block alignment
  short bits_per_sample;        // bits per second
  char data_sig[4];             // 'data'
  long data_size;               // size of waveform data
}wave_header;

#if HS_USE_MP3

typedef struct
{
  hs_adec_t       stAdec;

  /* wav read session */
  uint8_t *read_buffer, *read_ptr;
  int32_t  read_offset;
  uint32_t bytes_left, bytes_left_before_decoding;
  uint32_t frames;
}hs_adecwav_t;

hs_adec_t* hs_adec_wavCreate(void);

#endif

#endif
