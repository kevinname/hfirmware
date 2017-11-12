/*
    bootloader - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    config/tone/cfg_sbc.c
 * @brief   sbc encode and decode.
 * @details
 *
 * @addtogroup  config
 * @details
 * @{
 */
#include <stdio.h>
#include <string.h>

#include "lib.h"
#include "cfg_sbc.h"

/*
 * @brief  This function generates SBC parameters from the information of Tone or Ring.
 * @note   This header will be verified when decode SBC frame.
 * @note   This header cannot support mSBC because mSBC's blocks is 15.
 *
 * SBC Header Format:
 *     2b       2b     2b     1b   1b
 *  +-------+-------+-------+----+----+
 *  | rate  | blocks| mode  | am | sub| Byte 0
 *  +-------+-------+-------+----+----+
 *   sampling_frequency      allocation_method
 *    00 16khz                0 LOUDNESS
 *    01 32khz                1 SNR
 *    10 44.1khz
 *    11 48khz
 *           blocks               subbands
 *            00 4                 0 4
 *            01 8                 1 8
 *            10 12
 *            11 16
 *                   channel_mode
 *                    00 MONO         1
 *                    01 DUAL_CHANNEL 2
 *                    10 STEREO       2
 *                    11 JOINT_STEREO 2
 *
 *   +---------------------------------+
 *   |             bitpool             | Byte 1
 *   +---------------------------------+
 */
void hs_sbc_build_hdr(uint8_t *hdr,
                      uint32_t bitrate, uint8_t blocks, uint8_t chnmode, uint8_t subbands, uint8_t am,
                      uint8_t bitpool)
{
  if (NULL == hdr)
    return;

  hdr[0] = 0;
  switch (bitrate)
  {
    case 16000:
      hdr[0] |= SBC_FREQ_16000 << 6;
      break;
    case 32000:
      hdr[0] |= SBC_FREQ_32000 << 6;
      break;
    case 44100:
      hdr[0] |= SBC_FREQ_44100 << 6;
      break;
    case 48000:
      hdr[0] |= SBC_FREQ_48000 << 6;
      break;
  }

  hdr[0] |= (blocks / 4 - 1) << 4;
  hdr[0] |= chnmode << 2;
  hdr[0] |= am << 1;
  hdr[0] |= (subbands / 4 - 1) << 0;
  hdr[1] = bitpool;
}

/** @} */
