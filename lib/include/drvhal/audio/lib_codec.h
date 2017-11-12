/*
    drvhal - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/drvhal/codec.h
 * @brief   codec include file.
 * @details 
 *
 * @addtogroup  lib drvhal
 * @details 
 * @{
 */
#ifndef __LIB_CODEC_H__
#define __LIB_CODEC_H__

#include "lib.h"


#ifdef __cplusplus
extern "C" {
#endif

#if HAL_USE_CODEC

void hs_codec_scan(void);
void hs_codec_setEq(const hs_codec_eqpara_t *pstEq);
void hs_codec_setPointEq(uint8_t pointIdx, uint32_t freq, int32_t gain, uint32_t bandWidth);
__USED bool hs_codec_getPlugin(void);

#endif

#ifdef __cplusplus
}
#endif


#endif
 /** @} */
