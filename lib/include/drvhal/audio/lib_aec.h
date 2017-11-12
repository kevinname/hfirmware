/*
    drvhal - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/drvhal/audio/lib_aec.h
 * @brief   codec include file.
 * @details 
 *
 * @addtogroup  lib drvhal
 * @details 
 * @{
 */
#ifndef __LIB_AEC_H__
#define __LIB_AEC_H__

#include "lib.h"

#if HAL_USE_AUDIO

#ifdef __cplusplus
extern "C" {
#endif

void  hs_aec_init(void);
void  hs_aec_uninit(void);
short hs_aec_process(short xin, short din);

#ifdef __cplusplus
}
#endif

#endif

#endif
 /** @} */

