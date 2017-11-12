/*
    drvhal - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/drvhal/pad.h
 * @brief   config pad include file.
 * @details 
 *
 * @addtogroup  drvhal
 * @details 
 * @{
 */
#ifndef __LIB_PADCFG_H__
#define __LIB_PADCFG_H__

enum
{
  CFG_PAD_DIR_INPUT       = 0,
  CFG_PAD_DIR_OUTPUT      ,
};

enum
{
  CFG_PAD_PULL_NO         = 0,
  CFG_PAD_PULL_UP         ,
  CFG_PAD_PULL_DOWN       ,
};

typedef struct
{
  uint16_t  u16PadIdx;
  uint8_t   u8PadMode;
  uint8_t   u8PadDir;
  uint8_t   u8PadPull;
  uint8_t   u8PadDrvCap;
}hs_padinfo_t;


#ifdef __cplusplus
extern "C" {
#endif

int hs_pad_init(void) __attribute__((used));
void hs_pad_config(const hs_padinfo_t *pstPadInfo) __attribute__((used));

#ifdef __cplusplus
}
#endif


#endif
 /** @} */
