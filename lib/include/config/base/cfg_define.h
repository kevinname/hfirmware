/*
    bootloader - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    config/cfg_define.h
 * @brief   config define file.
 * @details 
 *
 * @addtogroup  config
 * @details 
 * @{
 */

#ifndef __CONFIG_DEFINE_H__
#define __CONFIG_DEFINE_H__

#define SIZE_1K                  1024
#define SIZE_4K                  (4 * 1024)
#define SIZE_8K                  (8 * 1024)
#define SIZE_16K                 (16 * 1024)
#define SIZE_64K                 (64 * 1024)
#define SIZE_512K                (512 * 1024)

#define SIZE_1M                  (1024 * 1024)
#define SIZE_4M                  (4 * 1024 * 1024)
#define SIZE_8M                  (8 * 1024 * 1024)
#define SIZE_16M                 (16 * 1024 * 1024)

#define BIT0    0x00000001u
#define BIT1    0x00000002u
#define BIT2    0x00000004u
#define BIT3    0x00000008u
#define BIT4    0x00000010u
#define BIT5    0x00000020u
#define BIT6    0x00000040u
#define BIT7    0x00000080u
#define BIT8    0x00000100u
#define BIT9    0x00000200u
#define BIT10   0x00000400u
#define BIT11   0x00000800u
#define BIT12   0x00001000u
#define BIT13   0x00002000u
#define BIT14   0x00004000u
#define BIT15   0x00008000u
#define BIT16   0x00010000u
#define BIT17   0x00020000u
#define BIT18   0x00040000u
#define BIT19   0x00080000u
#define BIT20   0x00100000u
#define BIT21   0x00200000u
#define BIT22   0x00400000u
#define BIT23   0x00800000u
#define BIT24   0x01000000u
#define BIT25   0x02000000u
#define BIT26   0x04000000u
#define BIT27   0x08000000u
#define BIT28   0x10000000u
#define BIT29   0x20000000u
#define BIT30   0x40000000u
#define BIT31   0x80000000u


/**
 * @brief config funtion execute result.
 */
typedef enum 
{
  HS_CFG_OK                     = 0,                /*!< result ok                          */

  HS_CFG_ERR_NO_DATA            = 0x10,             /*!< no config data                     */
  HS_CFG_ERR_DATA_READ_ONLY     ,                   /*!< config data can't be written       */

  HS_CFG_ERR_DATA_ERASE         ,                   /*!< config data erase error            */
  HS_CFG_ERR_DATA_WRITE         ,                   /*!< config data write error            */

  HS_CFG_ERR_DATA_RESTORE       ,                   /*!< config data restore error          */

  HS_CFG_ERR_PARAMTER           ,                   /*!< config data paramter error         */
  HS_CFG_ERR_REGISTER           ,                   /*!< register event error               */

  HS_CFG_ERROR                  = 0x7F,             /*!< some error have happened           */
} hs_cfg_res_t;

#if CH_DBG_ENABLE_PRINTF
#include <stdio.h>
#define cfg_print(fmt,args...)  hs_printf(fmt, ##args)
#define cfg_dbg(...)      \
  do \
  {\
    cfg_print("[Func]:%s [Line]:%d [Info]: \r\n\t", __FUNCTION__, __LINE__);\
    cfg_print(__VA_ARGS__);\
    cfg_print("\r\n\r\n");\
  }while(0)
#else
#define cfg_dbg(...)
#endif

#endif 
/** @} */
