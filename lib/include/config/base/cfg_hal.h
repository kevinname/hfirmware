/*
    bootloader - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    config/cfg_main.h
 * @brief   config include file.
 * @details 
 *
 * @addtogroup  config
 * @details 
 * @{
 */
#ifndef __CFG_HAL_H__
#define __CFG_HAL_H__

#include "stdint.h"

/*===========================================================================*/
/* macros default.                                                           */
/*===========================================================================*/
#define DEVICE_SF             (&SFD)
#define FLH_MEM_BASE          0x80000000
/*===========================================================================*/
/* data structure and data type.                                             */
/*===========================================================================*/



/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#define __cfg_byte2word(lbyte, hbyte, word)    \
{                                               \
  word = hbyte & 0xff;                          \
  word = (word << 8) + lbyte;                   \
}

#define __cfg_byte2dword(buf, word)            \
do                                              \
{                                               \
  word = buf[3];                                \
  word = (word << 8) + buf[2];                  \
  word = (word << 8) + buf[1];                  \
  word = (word << 8) + buf[0];                  \
}while(0)


#define __cfg_chkPtrNoRet(ptr)      do {if(ptr == NULL) return ;}while(0)
#define __cfg_chkPtrRet(ptr)        do {if(ptr == NULL) return HS_CFG_ERROR;}while(0)
#define __cfg_chkResRet(res)        do {if(res != HS_CFG_OK) return HS_CFG_ERROR;}while(0)

#ifdef __cplusplus
extern "C" {
#endif

hs_cfg_res_t hs_cfg_halRead(uint32_t offset, uint8_t *pbuf, uint32_t len);
hs_cfg_res_t hs_cfg_halErase(uint32_t offset, uint32_t len);
hs_cfg_res_t hs_cfg_halWrite(uint32_t offset, uint8_t *pbuf, uint32_t len);

uint32_t hs_cfg_halCalChecksum(uint8_t *pu8Buf, uint32_t len);
uint32_t hs_cfg_halByteToWord(uint8_t *pu8Buf, uint32_t len);

#ifdef __cplusplus
}
#endif


#endif
 /** @} */
