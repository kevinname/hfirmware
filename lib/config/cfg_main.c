/*
    bootloader - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    config/cfg_main.c
 * @brief   config main file.
 * @details 
 *
 * @addtogroup  config
 * @details 
 * @{
 */
#include "lib.h"
#include "string.h"

#if HS_USE_CONF

#define __CFG_FETCH_ONCE        SIZE_1K

typedef struct
{
  uint32_t u32Tag;
  uint32_t u32Len;
}hs_cfg_header_t;

hs_cfg_para_t g_stPara;
extern char __rw_lma_end[];

#define __cfg_getBase()       (((uint32_t)__rw_lma_end + SIZE_4K - 1) & (~(SIZE_4K - 1)))

hs_cfg_res_t _cfg_getOffsetByIndex(hs_cfg_index_t index, uint32_t *pu32Addr, uint8_t *pu8Attr)
{
  hs_cfg_res_t enRes;
  uint8_t u8Buf[CFG_INDEX_LENGTH];
  uint32_t u32BodyAddr, u32IdxAddr;

  if((g_stPara.bValid == FALSE) && (index != HS_CFG_BAW_AREAD))
  {
    return HS_CFG_ERR_NO_DATA;
  }

  u32IdxAddr = index * CFG_INDEX_LENGTH + __cfg_getBase();

  if(index != HS_CFG_BAW_AREAD)
    enRes = hs_cfg_read(u32IdxAddr, u8Buf, CFG_INDEX_LENGTH);
  else
    enRes = hs_cfg_halRead(u32IdxAddr, u8Buf, CFG_INDEX_LENGTH);
  if(HS_CFG_OK != enRes)
  {
    return HS_CFG_ERR_NO_DATA;
  }

  u32BodyAddr = hs_cfg_halByteToWord(u8Buf, CFG_INDEX_LENGTH);
  if((u32BodyAddr & CFG_DATA_VALID_MASK) == CFG_NO_VALID_DATA)
  {
    return HS_CFG_ERR_NO_DATA;
  }

  if(pu8Attr != NULL)
  {
    *pu8Attr = u32BodyAddr & CFG_INDEX_ATTR_MASK;
  }

  if(pu32Addr != NULL)
  {
    *pu32Addr = ((u32BodyAddr & (~CFG_INDEX_ATTR_MASK)) >> CFG_INDEX_TO_ADDR_MV) + __cfg_getBase();
    
    if(index > HS_CFG_BAW_AREAD)
    {
      *pu32Addr += g_stPara.u32BodyOffset;
    }
  }

  return HS_CFG_OK;
}

hs_cfg_res_t _cfg_getInfoByLocal(hs_cfg_info_t *pstInfo)
{
  hs_cfg_res_t enRes;
  uint8_t u8Buf[CFG_INDEX_LENGTH];
  uint32_t u32Addr, u32Chksum, u32Tmp; 

  enRes = hs_cfg_read(__cfg_getBase(), u8Buf, CFG_INDEX_LENGTH);
  if(HS_CFG_OK != enRes)
  {
    return enRes;
  }

  u32Addr = hs_cfg_halByteToWord(u8Buf, CFG_INDEX_LENGTH);
  if((u32Addr & CFG_DATA_VALID_MASK) == CFG_NO_VALID_DATA)
  {
    return HS_CFG_ERR_NO_DATA;
  }

  u32Addr = (u32Addr & 0xfffff0) >> CFG_INDEX_TO_ADDR_MV;
  u32Addr += __cfg_getBase();
  enRes = hs_cfg_read(u32Addr, (uint8_t *)pstInfo, sizeof(hs_cfg_info_t));
  if(HS_CFG_OK != enRes)
  {
    return enRes;
  }

  u32Tmp = pstInfo->u32Chksum;
  pstInfo->u32Chksum = 0;
  u32Chksum = hs_cfg_halCalChecksum((uint8_t *)pstInfo, sizeof(hs_cfg_info_t));
  if(u32Chksum != u32Tmp)
  {
    return HS_CFG_ERR_NO_DATA; 
  }

  if ((pstInfo->u32InfoFlg1 != CFG_DATA_FLAG_1) || (pstInfo->u32InfoFlg2 != CFG_DATA_FLAG_2))
  {
    return HS_CFG_ERR_NO_DATA;
  }
  
  return HS_CFG_OK;
}

hs_cfg_res_t _cfg_chkData(hs_cfg_header_t *pstHeader, uint32_t u32BodyAddr)
{
  hs_cfg_res_t enRes;
  uint8_t *pu8Buf;
  uint32_t i, u32TotalLen, u32TmpLen, u32Sum = 0;

  if(!pstHeader)
    return HS_CFG_ERROR;

  u32TotalLen = pstHeader->u32Len;
  u32TmpLen = u32TotalLen > __CFG_FETCH_ONCE ? __CFG_FETCH_ONCE : u32TotalLen;
  pu8Buf = (uint8_t *)hs_malloc(u32TmpLen, __MT_Z_GENERAL);
  if(!pu8Buf)
    return HS_CFG_ERROR;

  while(u32TotalLen > 0)
  {
    u32TmpLen = u32TotalLen > __CFG_FETCH_ONCE ? __CFG_FETCH_ONCE : u32TotalLen;
    enRes = hs_cfg_readS(u32BodyAddr, pu8Buf, u32TmpLen);
    if(HS_CFG_OK != enRes)
    {
      hs_free(pu8Buf);
      return HS_CFG_ERROR;
    }

    for(i=0; i<u32TmpLen; i++)
      u32Sum += pu8Buf[i];

    u32TotalLen -= u32TmpLen;
    u32BodyAddr += u32TmpLen;
  }

  hs_free(pu8Buf);
  if(pstHeader->u32Tag != u32Sum)
    return HS_CFG_ERROR;

  return HS_CFG_OK;
}

hs_cfg_res_t _cfg_setChkData(hs_cfg_header_t *pstHeader, uint32_t u32BodyAddr)
{
  hs_cfg_res_t enRes;
  uint8_t *pu8Buf;
  uint32_t i, u32TotalLen, u32TmpLen, u32Sum = 0;

  if(!pstHeader)
    return HS_CFG_ERROR;

  u32TotalLen = pstHeader->u32Len;
  u32TmpLen = u32TotalLen > __CFG_FETCH_ONCE ? __CFG_FETCH_ONCE : u32TotalLen;
  pu8Buf = (uint8_t *)hs_malloc(u32TmpLen, __MT_Z_GENERAL);
  if(!pu8Buf)
    return HS_CFG_ERROR;

  while(u32TotalLen > 0)
  {
    u32TmpLen = u32TotalLen > __CFG_FETCH_ONCE ? __CFG_FETCH_ONCE : u32TotalLen;
    enRes = hs_cfg_readS(u32BodyAddr, pu8Buf, u32TmpLen);
    if(HS_CFG_OK != enRes)
    {
      hs_free(pu8Buf);
      return HS_CFG_ERROR;
    }

    for(i=0; i<u32TmpLen; i++)
      u32Sum += pu8Buf[i];

    u32TotalLen -= u32TmpLen;
    u32BodyAddr += u32TmpLen;
  }

  hs_free(pu8Buf);
  pstHeader->u32Tag = u32Sum;
  return HS_CFG_OK;
}

void _cfg_fillPara(hs_cfg_info_t *pstInfo)
{
  g_stPara.u16CfgLocal = pstInfo->u32Local;
  g_stPara.u16IndexCnt = pstInfo->u32IndexCnt;
  g_stPara.u32BodyOffset = pstInfo->u32BodyAddr;
  g_stPara.u16BackupLen = pstInfo->u32BackupLen;

  g_stPara.bValid = TRUE;
}

void _cfg_getInfo(void)
{
  hs_cfg_info_t stInfo;
  hs_cfg_res_t enRes;

  enRes = _cfg_getInfoByLocal(&stInfo);
  if(HS_CFG_OK == enRes)
  {
    _cfg_fillPara(&stInfo);
    return ;
  }

  memset(&g_stPara, 0, sizeof(hs_cfg_para_t));
  g_stPara.bValid = FALSE;
}
#endif

/*===========================================================================*/
/* output functions.                                                         */
/*===========================================================================*/
void hs_cfg_Init(void)
{ 
  #if HS_USE_CONF
  hs_cfg_memInit();
  
  _cfg_getInfo();
  
  hs_cfg_restore();  
  hs_cfg_sysInit();
  #endif
}

/*
 * @brief               get offset address of a config data
 *                      
 * @param[in] index     config data index
 * @param[out] pu32Addr config data address
 *                      .
 * @return              0-ok, other-some error happened
 */
hs_cfg_res_t hs_cfg_getOffsetByIndex(hs_cfg_index_t index, uint32_t *pu32Addr)
{
  #if HS_USE_CONF
  hs_cfg_res_t enRes;

  enRes = _cfg_getOffsetByIndex(index, pu32Addr, NULL);
  *pu32Addr += sizeof(hs_cfg_header_t);
  return enRes;
  #else
  (void)index;
  (void)pu32Addr;
  return HS_CFG_ERROR;
  #endif
}

/*
 * @brief               get total config data by index
 *                      
 * @param[in] index     config data index
 * @param[out] buf      data buffer for saving config data
 * @param[in] length    config data length
 *                      .
 * @return              0-ok, other-some error happened
 */
hs_cfg_res_t hs_cfg_getDataByIndex(hs_cfg_index_t index, uint8_t *buf, uint16_t length)
{
  #if HS_USE_CONF
  hs_cfg_res_t enRes;
  uint32_t u32BodyAddr;
  hs_cfg_header_t stHeader;

  enRes = _cfg_getOffsetByIndex(index, &u32BodyAddr, NULL);
  if(enRes != HS_CFG_OK)
    return enRes;

  hs_cfg_lock();
  enRes = hs_cfg_readS(u32BodyAddr, (uint8_t *)&stHeader, sizeof(hs_cfg_header_t));
  if(HS_CFG_OK != enRes)
  {
    hs_cfg_unlock();
    return enRes;
  }

  u32BodyAddr += sizeof(hs_cfg_header_t);  
  enRes = _cfg_chkData(&stHeader, u32BodyAddr);  
  if(HS_CFG_OK != enRes)
  {
    hs_cfg_unlock();
    return enRes;
  }

  enRes =  hs_cfg_readS(u32BodyAddr, buf, length);
  hs_cfg_unlock();
  return enRes;
  #else
  (void)index;
  (void)buf;
  (void)length;
  return HS_CFG_ERROR;
  #endif
}

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
hs_cfg_res_t hs_cfg_getPartDataByIndex(hs_cfg_index_t index, uint8_t *buf, uint16_t length, uint32_t offset)
{
  #if HS_USE_CONF
  hs_cfg_res_t enRes;
  uint32_t u32BodyAddr;
  hs_cfg_header_t stHeader;

  enRes = _cfg_getOffsetByIndex(index, &u32BodyAddr, NULL);
  if(enRes != HS_CFG_OK)
    return enRes;

  if(index == HS_CFG_SOUND_INFO)
  {
    u32BodyAddr += sizeof(hs_cfg_header_t);
    u32BodyAddr += offset;
    return hs_cfg_read(u32BodyAddr, buf, length);
  }

  hs_cfg_lock();
  enRes = hs_cfg_readS(u32BodyAddr, (uint8_t *)&stHeader, sizeof(hs_cfg_header_t));
  if(HS_CFG_OK != enRes)
  {
    hs_cfg_unlock();
    return enRes;
  }

  u32BodyAddr += sizeof(hs_cfg_header_t);
  enRes = _cfg_chkData(&stHeader, u32BodyAddr);
  if(HS_CFG_OK != enRes)
  {
    hs_cfg_unlock();
    return enRes;
  }

  u32BodyAddr += offset;
  enRes =  hs_cfg_readS(u32BodyAddr, buf, length);
  hs_cfg_unlock();
  return enRes;
  #else
  (void)index;
  (void)buf;
  (void)length;
  (void)offset;
  return HS_CFG_ERROR;
  #endif
}

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
hs_cfg_res_t hs_cfg_setDataByIndex(hs_cfg_index_t index, uint8_t *buf, uint16_t length, uint32_t offset)
{
  #if HS_USE_CONF
  hs_cfg_res_t enRes;
  hs_cfg_header_t stHeader;
  uint32_t u32BodyAddr, u32DataAddr; 
  uint8_t u8Attr;

  enRes = _cfg_getOffsetByIndex(index, &u32BodyAddr, &u8Attr);
  if(enRes != HS_CFG_OK)
    return enRes;

  if ((u8Attr & CFG_DATA_WR_MASK) == CFG_DATA_READ_ONLY)
    return HS_CFG_ERR_DATA_READ_ONLY;

  enRes = hs_cfg_read(u32BodyAddr, (uint8_t *)&stHeader, sizeof(hs_cfg_header_t));
  if(HS_CFG_OK != enRes)
    return enRes;

  if(stHeader.u32Len < (offset + length))
    return HS_CFG_ERR_PARAMTER;

  hs_cfg_lock();
  u32DataAddr = u32BodyAddr + sizeof(hs_cfg_header_t);
  offset += u32DataAddr;
  enRes = hs_cfg_writeS(offset, buf, length);
  if(HS_CFG_OK != enRes)
  {
    hs_cfg_unlock();
    return enRes;
  }

  enRes = _cfg_setChkData(&stHeader, u32DataAddr);
  if(HS_CFG_OK != enRes)
  {
    hs_cfg_unlock();
    return enRes;
  }

  enRes = hs_cfg_writeS(u32BodyAddr, (uint8_t *)&stHeader, sizeof(hs_cfg_header_t));
  hs_cfg_unlock();
  return enRes;
  #else
  (void)index;
  (void)buf;
  (void)length;
  (void)offset;
  return HS_CFG_ERROR;
  #endif
}

/** @} */
