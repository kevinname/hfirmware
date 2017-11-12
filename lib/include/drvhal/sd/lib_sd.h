/*
    drvhal - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/drvhal/lib_sd.h
 * @brief   sd include file.
 * @details 
 *
 * @addtogroup  lib drvhal
 * @details 
 * @{
 */
#ifndef __LIB_SD_H__
#define __LIB_SD_H__

#include "lib.h"

#if HAL_USE_SDC

#ifdef __cplusplus
extern "C" {
#endif

void hs_sd_scanCard(void);

#ifdef __cplusplus
}
#endif

static inline void hs_sd_discon(void)
{
  sdcDisconnect(&SDCD0);
}
#else

#define hs_sd_scanCard()  
#define hs_sd_discon()    

#endif

#endif
 /** @} */
