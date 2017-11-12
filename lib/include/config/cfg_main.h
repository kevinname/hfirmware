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
#ifndef __CFG_MAIN_H__
#define __CFG_MAIN_H__

#include "./base/cfg_hal.h"
#include "./config.h"

/*===========================================================================*/
/* macros default.                                                           */
/*===========================================================================*/
#define CFG_DATA_FLAG_1             0x3CC3A55A
#define CFG_DATA_FLAG_2             0x48533636

#define CFG_INDEX_LENGTH            0x3

#define CFG_INDEX_ATTR_WIDTH        4
#define CFG_INDEX_ATTR_MASK         ((1 << CFG_INDEX_ATTR_WIDTH) - 1)

#define CFG_INDEX_ADDR_WIDTH        20
#define CFG_DATA_ADDR_WIDTH         22
#define CFG_INDEX_TO_ADDR_MV        (CFG_INDEX_ATTR_WIDTH - (CFG_DATA_ADDR_WIDTH - CFG_INDEX_ADDR_WIDTH))


#define CFG_DATA_VALID_MASK         BIT0
#define CFG_DATA_WR_MASK            BIT1

#define CFG_ADDR_AGLIN_LEN          0x4

/*===========================================================================*/
/* data structure and data type.                                             */
/*===========================================================================*/
enum
{
  CFG_HAVE_VALID_DATA     = 0,
  CFG_NO_VALID_DATA       ,
};

enum
{
  CFG_DATA_READ_ONLY     = 0,
  CFG_DATA_READ_WRITE    ,
};

/**
 * @brief config data manage info.
 */
typedef struct 
{
  uint32_t      u32InfoFlg1;          /*!< config data valid flag             */
  uint32_t      u32InfoFlg2;          /*!< config data valid flag             */
  
  uint32_t      u32Version;           /*!< config data version                */
  uint32_t      u32Date;              /*!< the date of config data generated  */

  uint32_t      u32Local;             /*!< the local of config data           */
  uint32_t      u32IndexCnt;          /*!< config data count                  */
  uint32_t      u32BodyAddr;          /*!< the address of config data body    */

  uint32_t      u32BackupLen;         /*!< for backup write                   */

  uint32_t      u32Chksum;  
} hs_cfg_info_t;

typedef struct
{
  uint16_t  bValid;

  uint16_t  u16CfgLocal;  
  uint16_t  u16IndexCnt;  
  uint32_t  u32BodyOffset;

  uint16_t  u16BackupLen;   
}hs_cfg_para_t;

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/




#ifdef __cplusplus
extern "C" {
#endif

void hs_cfg_Init(void);

/*
 * @brief               get offset address of a config data
 *                      
 * @param[in] index     config data index
 * @param[out] pu32Addr config data address
 *                      .
 * @return              0-ok, other-some error happened
 */
hs_cfg_res_t hs_cfg_getOffsetByIndex(hs_cfg_index_t index, uint32_t *pu32Addr) __attribute__((used));

/*
 * @brief               get total config data by index
 *                      
 * @param[in] index     config data index
 * @param[out] buf      data buffer for saving config data
 * @param[in] length    config data length
 *                      .
 * @return              0-ok, other-some error happened
 */
hs_cfg_res_t hs_cfg_getDataByIndex(hs_cfg_index_t index, uint8_t *buf, uint16_t length) __attribute__((used));

/*
 * @brief               get a part of config data by index and offset
 *                      
 * @param[in] index     config data index
 * @param[out] buf      data buffer for saving config data
 * @param[in] length    the length of this reading operating
 * @param[in] offset    the offset in this config data
 *                      .
 * @return              0-ok, other-some error happened
 */
hs_cfg_res_t hs_cfg_getPartDataByIndex(hs_cfg_index_t index, uint8_t *buf, uint16_t length, uint32_t offset) __attribute__((used));

/*
 * @brief               get a part of config data by index and offset
 *                      
 * @param[in] index     config data index
 * @param[out] buf      data buffer for saving config data
 * @param[in] length    the length of this writing operating
 * @param[in] offset    the offset in this config data
 *                      .
 * @return              0-ok, other-some error happened
 */
hs_cfg_res_t hs_cfg_setDataByIndex(hs_cfg_index_t index, uint8_t *buf, uint16_t length, uint32_t offset) __attribute__((used));

#ifdef __cplusplus
}
#endif


#endif
 /** @} */
