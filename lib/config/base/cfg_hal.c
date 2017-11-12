/*
    bootloader - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    config/cfg_hal.c
 * @brief   config hal file.
 * @details 
 *
 * @addtogroup  config
 * @details 
 * @{
 */
#include "lib.h"
#include "string.h"

#define FLH_RDWR_LEN    256

void nds32_dcache_invalidate_range(unsigned long start, unsigned long end);
/*===========================================================================*/
/* main and output functions.                                                */
/*===========================================================================*/
hs_cfg_res_t hs_cfg_halErase(uint32_t offset, uint32_t len)
{
  hs_cfg_res_t res;

  offset &= 0xfffffff;
    
  sfAcquireBus(DEVICE_SF);
  res = sfErase(DEVICE_SF, offset, len, TIME_INFINITE);
  sfReleaseBus(DEVICE_SF);

  nds32_dcache_invalidate_range(FLH_MEM_BASE + offset, FLH_MEM_BASE + offset + len);
  
  return res;
}

hs_cfg_res_t hs_cfg_halRead(uint32_t offset, uint8_t *pbuf, uint32_t len)
{  
  uint8_t *pu8Ptr;

  offset &= 0xfffffff;
  pu8Ptr = (uint8_t *)(FLH_MEM_BASE | offset);
  memcpy(pbuf, pu8Ptr, len);

  return HS_CFG_OK;
}

hs_cfg_res_t hs_cfg_halWrite(uint32_t offset, uint8_t *pbuf, uint32_t len)
{
  hs_cfg_res_t enRes = HS_CFG_OK;
  uint32_t u32Tmp, u32Start, u32End;
  uint8_t *pu8Ptr;

  offset &= 0xfffffff;
  pu8Ptr = hs_malloc(FLH_RDWR_LEN, __MT_DMA);
  if(pu8Ptr == NULL)
    return HS_CFG_ERROR;

  u32Start = FLH_MEM_BASE + offset;
  u32End   = u32Start + len;
  while(len > 0)
  { 
    u32Tmp = len > FLH_RDWR_LEN ? FLH_RDWR_LEN : len;
    
    memcpy(pu8Ptr, pbuf, u32Tmp);
    sfAcquireBus(DEVICE_SF);
    enRes = sfWrite(DEVICE_SF, offset, pu8Ptr, u32Tmp, TIME_INFINITE);
    sfReleaseBus(DEVICE_SF);
    
    if(enRes != HS_CFG_OK)
    {
      enRes = HS_CFG_ERROR;
      goto sfwr_exit;
    }

    pbuf   += u32Tmp;
    offset += u32Tmp;
    len    -= u32Tmp;
  }  

sfwr_exit:
  hs_free(pu8Ptr);
  nds32_dcache_invalidate_range(u32Start, u32End);  
  return enRes;
}

uint32_t hs_cfg_halCalChecksum(uint8_t *pu8Buf, uint32_t len)
{
  uint32_t i,u32ChkSum = 0;

	for(i=0; i<len; i++)
    u32ChkSum += pu8Buf[i];

	return u32ChkSum;
}

uint32_t hs_cfg_halByteToWord(uint8_t *pu8Buf, uint32_t len)
{
  uint32_t i, u32Tmp, u32Val = 0;

  if(len > 4)
    return 0;

  for(i=0; i<len; i++)
  {
    u32Tmp = pu8Buf[i];
    u32Val |= u32Tmp << (i * 8);
  }

	return u32Val;
}

/** @} */
