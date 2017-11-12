/*
    drvhal - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/drvhal/infrared.h
 * @brief   pwm file.
 * @details 
 *
 * @addtogroup  lib drvhal
 * @details 
 * @{
 */

#ifndef __INFRARED_H__
#define __INFRARED_H__


typedef struct
{
  uint8_t     u8PinIdx;
  uint8_t     u8IcuIdx;
  uint8_t     u8ChnIdx;
  uint8_t     u8FunIdx;
}hs_infrared_para_t;

typedef struct
{
  uint8_t     u8Module;
  uint16_t    u16Msg;
  uint32_t    u32MsgPara;

  uint32_t    u32Dec;
}hs_infrared_dec_t;

void hs_infrared_serStart(uint16_t u16Idx, void *parg);

#endif

