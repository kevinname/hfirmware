/*
    bootloader - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    config/cfg_cachemem.c
 * @brief   config hal file.
 * @details 
 *
 * @addtogroup  config
 * @details 
 * @{
 */
#include "lib.h"
#include "string.h"

#define __cache_chkAddr(paddr, plen, vaddr)   \
  ((vaddr >= paddr) && (vaddr <= (paddr + plen)))
#define __cache_chkCacheAddr(pstMem, vaddr)   \
  __cache_chkAddr((pstMem)->u32PhyAddr, CFG_CACHEBK_SIZE, vaddr)
#define __cache_chkLen(paddr, plen, vaddr, vlen)    \
  ((vaddr + vlen) <= (paddr + plen))
#define __cache_chkCacheLen(pstMem, vaddr, vlen)     \
  __cache_chkLen((pstMem)->u32PhyAddr, CFG_CACHEBK_SIZE, vaddr, vlen)
#define __cache_getLen(paddr, plen, vaddr)    \
  (paddr + plen - vaddr)
#define __cache_getHitLen(pstMem, vaddr)     \
  __cache_getLen((pstMem)->u32PhyAddr, CFG_CACHEBK_SIZE, vaddr)
#define __cache_chkAddrInBlock(addr, len)   \
  (((addr & CFG_CACHEBK_MASK) + CFG_CACHEBK_SIZE) < (addr + len))
#define __cache_getLenInBlock(addr, len)   \
  (((addr & CFG_CACHEBK_MASK) + CFG_CACHEBK_SIZE) - addr)
#define __cache_chkResAndJump(res, pos)  \
  do { if(res != HS_CFG_OK) {res = HS_CFG_ERROR; goto pos;}}while(0)

static hs_cfg_pcachemem_t g_pstCacheMem;
static osSemaphoreId g_pstSemId;

hs_cfg_res_t _cfg_cachemem_restore(void)
{
  hs_cfg_bkw_t stBkwInfo;
  uint8_t *pu8BackPtr;
  uint32_t u32BWAddr, u32Tmp;
  hs_cfg_res_t enRes;  

  enRes = hs_cfg_getOffsetByIndex(HS_CFG_BAW_AREAD, &u32BWAddr);
  __cfg_chkResRet(enRes);

  enRes = hs_cfg_halRead(u32BWAddr, (uint8_t *)&stBkwInfo, sizeof(hs_cfg_bkw_t));
  __cfg_chkResRet(enRes);

  u32Tmp = stBkwInfo.u32BkwChkSum;
  stBkwInfo.u32BkwChkSum = 0;
  stBkwInfo.u32BkwChkSum = hs_cfg_halCalChecksum((uint8_t *)&stBkwInfo, sizeof(hs_cfg_bkw_t));
  if( (u32Tmp != stBkwInfo.u32BkwChkSum) || (stBkwInfo.u32BkwInfoFlg != CFG_BKW_DATA_FLAG))
    return HS_CFG_OK;
  
  pu8BackPtr = (uint8_t *)hs_malloc(stBkwInfo.u32BkwDataLen, __MT_GENERAL);
  __cfg_chkPtrRet(pu8BackPtr);

  enRes = hs_cfg_halRead(u32BWAddr + CFG_BKW_BLOCK_SIZE, pu8BackPtr, stBkwInfo.u32BkwDataLen);
  __cache_chkResAndJump(enRes, __cfg_wrrestore_exit);

  enRes = hs_cfg_halErase(stBkwInfo.u32BkwDataAddr, stBkwInfo.u32BkwDataLen);
  __cache_chkResAndJump(enRes, __cfg_wrrestore_exit);

  enRes = hs_cfg_halWrite(stBkwInfo.u32BkwDataAddr, pu8BackPtr, stBkwInfo.u32BkwDataLen);
  __cache_chkResAndJump(enRes, __cfg_wrrestore_exit);

  enRes = hs_cfg_halErase(u32BWAddr, sizeof(hs_cfg_bkw_t));
  __cache_chkResAndJump(enRes, __cfg_wrrestore_exit);

  memset(pu8BackPtr, 0, stBkwInfo.u32BkwDataLen);
  enRes = hs_cfg_halWrite(u32BWAddr, pu8BackPtr, stBkwInfo.u32BkwDataLen);
  
__cfg_wrrestore_exit:
  hs_free(pu8BackPtr);
  return enRes;
}

static hs_cfg_res_t _cfg_cachemem_blockwr(hs_cfg_pcachemem_t pstMBlk)
{
  hs_cfg_bkw_t stBkwInfo;
  uint32_t u32BWAddr;
  hs_cfg_res_t enRes;

  stBkwInfo.u32BkwDataAddr = pstMBlk->u32PhyAddr;  
  stBkwInfo.u32BkwInfoFlg = CFG_BKW_DATA_FLAG;  
  stBkwInfo.u32BkwDataLen = CFG_BKW_BLOCK_SIZE;  
  stBkwInfo.u32BkwChkSum = 0;
  stBkwInfo.u32BkwChkSum = hs_cfg_halCalChecksum((uint8_t *)&stBkwInfo, sizeof(hs_cfg_bkw_t));

  enRes = hs_cfg_getOffsetByIndex(HS_CFG_BAW_AREAD, &u32BWAddr);
  __cache_chkResAndJump(enRes, __cachemem_bwr_exit);

  enRes = hs_cfg_halErase(u32BWAddr, CFG_BKW_BLOCK_SIZE * 2);
  __cache_chkResAndJump(enRes, __cachemem_bwr_exit);

  enRes = hs_cfg_halWrite(u32BWAddr + CFG_BKW_BLOCK_SIZE, pstMBlk->u8VtCache, CFG_BKW_BLOCK_SIZE);
  __cache_chkResAndJump(enRes, __cachemem_bwr_exit);

  enRes = hs_cfg_halWrite(u32BWAddr, (uint8_t *)&stBkwInfo, sizeof(hs_cfg_bkw_t));
  __cache_chkResAndJump(enRes, __cachemem_bwr_exit);

  enRes = hs_cfg_halErase(pstMBlk->u32PhyAddr, CFG_BKW_BLOCK_SIZE);
  __cache_chkResAndJump(enRes, __cachemem_bwr_exit);

  enRes = hs_cfg_halWrite(pstMBlk->u32PhyAddr, pstMBlk->u8VtCache, CFG_BKW_BLOCK_SIZE);
  __cache_chkResAndJump(enRes, __cachemem_bwr_exit);

  memset(&stBkwInfo, 0, sizeof(hs_cfg_bkw_t));
  enRes = hs_cfg_halWrite(u32BWAddr, (uint8_t *)&stBkwInfo, sizeof(hs_cfg_bkw_t));

__cachemem_bwr_exit:
  return enRes;
}

static uint32_t _cfg_cachemem_searchhit(uint8_t **p, hs_cfg_pcachemem_t pstMem, uint32_t vaddr, uint32_t vlen)
{
  uint32_t u32Os, u32hlen = 0;
  
  if(pstMem == NULL)
    return 0;

  if(__cache_chkCacheAddr(pstMem, vaddr))
  {
    u32Os = vaddr - pstMem->u32PhyAddr;
    *p = pstMem->u8VtCache + u32Os;
    if(__cache_chkCacheLen(pstMem, vaddr, vlen))
      u32hlen = vlen;
    else
      u32hlen = __cache_getHitLen(pstMem, vaddr);
  }

  return u32hlen;
}

uint32_t _cfg_cachemem_write(uint32_t offset, uint8_t *pbuf, uint32_t len)
{
  hs_cfg_pcachemem_t *pstMem = &g_pstCacheMem;
  uint32_t u32hlen, u32TLen = len;
  uint8_t *pu8Ptr;

  do
  {
    pstMem = &g_pstCacheMem;

    while((*pstMem != NULL) && (len > 0))
    {
      u32hlen = _cfg_cachemem_searchhit(&pu8Ptr, *pstMem, offset, len);
      pstMem = &(*pstMem)->pstNext;
      if(0 == u32hlen)
        continue;

      memcpy(pu8Ptr, pbuf, u32hlen);
      offset += u32hlen;
      pbuf   += u32hlen;
      len    -= u32hlen;
    };

    if((*pstMem == NULL) && (len > 0))
    {
      *pstMem = hs_malloc(sizeof(struct _cfg_cachemem), __MT_Z_GENERAL);
      if(*pstMem == NULL)
        break;

      (*pstMem)->u32PhyAddr = offset & CFG_CACHEBK_MASK;
      hs_cfg_halRead((*pstMem)->u32PhyAddr, (*pstMem)->u8VtCache, CFG_CACHEBK_SIZE);

      if(len > 0)
      {
        u32hlen = __cache_chkAddrInBlock(offset, len) ? 
                    __cache_getLenInBlock(offset, len) : len;
        
        pu8Ptr = (*pstMem)->u8VtCache + (offset - (*pstMem)->u32PhyAddr);
        memcpy(pu8Ptr, pbuf, u32hlen);

        offset += u32hlen;
        pbuf   += u32hlen;
        len    -= u32hlen;
      }
    }    
  }while(len > 0);

  return (u32TLen - len);
}

hs_cfg_res_t _cfg_cachemem_cleanunit(void)
{
  hs_cfg_pcachemem_t pstMem = g_pstCacheMem;
  hs_cfg_res_t enRes;

  if(pstMem == NULL)
    return HS_CFG_OK;  

  hs_printf("Flash cache:0x%X, PA:0x%x\r\n", (uint32_t)pstMem, pstMem->u32PhyAddr);
  enRes = _cfg_cachemem_blockwr(pstMem);
  if(enRes == HS_CFG_OK)
  {
    hs_free(pstMem);
    g_pstCacheMem = pstMem->pstNext;     
  }
  hs_printf("flush a block\r\n");

  return enRes;
}

hs_cfg_res_t _cfg_cachemem_cleanall(void)
{
  hs_cfg_res_t enRes = HS_CFG_OK;

  while(g_pstCacheMem != NULL)
  {
    enRes = _cfg_cachemem_cleanunit();
    __cfg_chkResRet(enRes);
  }

  return enRes;
}

hs_cfg_res_t hs_cfg_readS(uint32_t offset, uint8_t *pbuf, uint32_t len)
{
  hs_cfg_pcachemem_t pstMem = g_pstCacheMem;
  uint32_t u32hlen;
  uint8_t *pu8Ptr;
  
  if(pstMem == NULL)
    return hs_cfg_halRead(offset, pbuf, len);

  do
  {
    pstMem = g_pstCacheMem;

    while((pstMem != NULL) && (len > 0))
    {
      u32hlen = _cfg_cachemem_searchhit(&pu8Ptr, pstMem, offset, len);
      pstMem = pstMem->pstNext; 
      if(0 == u32hlen)
        continue;

      memcpy(pbuf, pu8Ptr, u32hlen);
      offset += u32hlen;
      pbuf   += u32hlen;
      len    -= u32hlen;    
    }; 

    if(len > 0)
    {
      u32hlen = __cache_chkAddrInBlock(offset, len) ?
                  __cache_getLenInBlock(offset, len) : len;
      
      hs_cfg_halRead(offset, pbuf, u32hlen);
      offset += u32hlen;
      pbuf   += u32hlen;
      len    -= u32hlen;
    }
  }while(len > 0);
  
  return HS_CFG_OK;  
}

hs_cfg_res_t hs_cfg_restore(void)
{
  hs_cfg_res_t enRes;
  
  oshalSemaphoreWait(g_pstSemId, -1);
  enRes = _cfg_cachemem_restore();
  oshalSemaphoreRelease(g_pstSemId);

  return enRes;
}

hs_cfg_res_t hs_cfg_read(uint32_t offset, uint8_t *pbuf, uint32_t len)
{
  hs_cfg_res_t enRes;
  
  oshalSemaphoreWait(g_pstSemId, -1);
  enRes = hs_cfg_readS(offset, pbuf, len);
  oshalSemaphoreRelease(g_pstSemId);

  return enRes;
}

hs_cfg_res_t hs_cfg_write(uint32_t offset, uint8_t *pbuf, uint32_t len)
{
  uint32_t u32Len;
  hs_cfg_res_t enRes = HS_CFG_OK;
  
  oshalSemaphoreWait(g_pstSemId, -1);
  u32Len = _cfg_cachemem_write(offset, pbuf, len);
  oshalSemaphoreRelease(g_pstSemId);

  if(u32Len != len)
    enRes = HS_CFG_ERROR;

  return enRes;
}

hs_cfg_res_t hs_cfg_writeS(uint32_t offset, uint8_t *pbuf, uint32_t len)
{
  uint32_t u32Len;
  hs_cfg_res_t enRes = HS_CFG_OK;
  
  u32Len = _cfg_cachemem_write(offset, pbuf, len);
  if(u32Len != len)
    enRes = HS_CFG_ERROR;

  return enRes;
}

hs_cfg_res_t hs_cfg_flush(hs_cfg_flushtype_t type)
{
  hs_cfg_res_t enRes;
  
  oshalSemaphoreWait(g_pstSemId, -1);
  
  enRes = type == FLUSH_TYPE_ALL ?
            _cfg_cachemem_cleanall() :
            _cfg_cachemem_cleanunit();  

  if(enRes != HS_CFG_OK)
    _cfg_cachemem_restore();
  
  oshalSemaphoreRelease(g_pstSemId);

  return enRes;
}

void hs_cfg_lock(void)
{
  oshalSemaphoreWait(g_pstSemId, -1);
}

void hs_cfg_unlock(void)
{
  oshalSemaphoreRelease(g_pstSemId);
}


void hs_cfg_memInit(void)
{
  osSemaphoreDef_t semdef;

  g_pstSemId = oshalSemaphoreCreate(&semdef, 1);
}

/** @} */
