/*
    bootloader - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    config/cfg_cachemem.h
 * @brief   config include file.
 * @details 
 *
 * @addtogroup  config
 * @details 
 * @{
 */
#ifndef __CFG_CACHEMEM_H__
#define __CFG_CACHEMEM_H__

#define CFG_BKW_DATA_FLAG             0x4746435A
#define CFG_BKW_BLOCK_SIZE            SIZE_4K
#define CFG_BKW_BLOCK_ALIGN_MASK      (~(CFG_BKW_BLOCK_SIZE - 1))

#define CFG_CACHEBK_SIZE              CFG_BKW_BLOCK_SIZE
#define CFG_CACHEBK_MASK              CFG_BKW_BLOCK_ALIGN_MASK

typedef enum
{
  FLUSH_TYPE_ONEBLOCK       = 0,
  FLUSH_TYPE_ALL
}hs_cfg_flushtype_t;

typedef struct _cfg_cachemem  * hs_cfg_pcachemem_t;
struct _cfg_cachemem
{
  hs_cfg_pcachemem_t pstNext;
  
  uint32_t  u32PhyAddr;
  uint8_t   u8VtCache[CFG_CACHEBK_SIZE];  
};

typedef struct 
{
  uint32_t      u32BkwInfoFlg;       /*!< backup data valid flag             */
  uint32_t      u32BkwDataAddr;      /*!< Backup data address                */
  uint32_t      u32BkwDataLen;       /*!< Backup data length                 */
  uint32_t      u32BkwChkSum;        /*!< Backup info checksum               */
}hs_cfg_bkw_t;

#ifdef __cplusplus
extern "C" {
#endif

void hs_cfg_memInit(void);
hs_cfg_res_t hs_cfg_flush(hs_cfg_flushtype_t type);
hs_cfg_res_t hs_cfg_write(uint32_t offset, uint8_t *pbuf, uint32_t len);
hs_cfg_res_t hs_cfg_writeS(uint32_t offset, uint8_t *pbuf, uint32_t len);
hs_cfg_res_t hs_cfg_read(uint32_t offset, uint8_t *pbuf, uint32_t len);
hs_cfg_res_t hs_cfg_readS(uint32_t offset, uint8_t *pbuf, uint32_t len);
hs_cfg_res_t hs_cfg_restore(void);

void hs_cfg_lock(void);
void hs_cfg_unlock(void);

#ifdef __cplusplus
}
#endif


#endif
 /** @} */
