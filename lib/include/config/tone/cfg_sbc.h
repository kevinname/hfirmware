/*
    bootloader - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    config/tone/cfg_sbc.h
 * @brief   sbc encode and decode include file.
 * @details 
 *
 * @addtogroup  config
 * @details 
 * @{proportion
 */

#ifndef __CFG_SBC_H__
#define __CFG_SBC_H__

#include "sbc_bluez.h"

#define SBC_COMPRESS_PROPORTION     16

typedef sbc_t*                      hs_sbc_handle_t;

#define hs_sbc_open(hdr)            sbc_bluez_open(hdr)

#define hs_sbc_decode(handle, pInBuf, inSize, pOutBuf, pPcmLen)   \
  sbc_bluez_decode(handle, pInBuf, inSize, pOutBuf, (inSize) * SBC_COMPRESS_PROPORTION, pPcmLen)

#define hs_sbc_encode(handle, pInBuf, inSize, pOutBuf, OutSize, pPcmLen) \
  sbc_bluez_encode(handle, pInBuf, inSize, pOutBuf, OutSize, pPcmLen)
  
#define hs_sbc_close(handle)        sbc_bluez_close(handle)

void hs_sbc_build_hdr(uint8_t *hdr,
                      uint32_t bitrate, uint8_t blocks, uint8_t chnmode, uint8_t subbands, uint8_t alloc,
                      uint8_t bitpool);

#endif
/** @} */
