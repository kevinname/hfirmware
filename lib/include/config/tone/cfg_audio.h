/*
    bootloader - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    config/tone/cfg_audio.h
 * @brief   audio include file.
 * @details 
 *
 * @addtogroup  config
 * @details 
 * @{proportion
 */

#ifndef __CFG_AUDIO_H__
#define __CFG_AUDIO_H__

#define TONE_AUDIO_MODE_MONO      I2S_PCMMODE_MONO
#define TONE_AUDIO_MODE_STEREO    I2S_PCMMODE_STEREO

enum
{
  CFG_AUDIO_STOP    = 0,
  CFG_AUDIO_RUNNING ,
};

void hs_cfg_toneAudioStart(uint32_t u32Rate, uint8_t u8Mode, int16_t s16ToneVolume);
hs_cfg_res_t hs_cfg_toneTxStream(uint8_t *u8Buf, uint32_t u32Len);
void hs_cfg_toneAudioStop(uint8_t u8Interrupt);

void hs_cfg_toneRecStart(uint32_t u32Rate, uint8_t u8Mode, int16_t s16ToneVolume);
hs_cfg_res_t hs_cfg_toneRxStream(uint8_t *u8Buf, uint32_t u32Len);
void hs_cfg_toneRecStop(uint8_t u8Interrupt);

#endif
/** @} */
