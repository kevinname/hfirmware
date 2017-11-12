/*
    application - Copyright (C) 2012~2016 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    unitTest/mem_test.c
 * @brief   memory management test in unit test.
 * @details 
 *
 * @addtogroup  unit test
 * @details 
 * @{
 */
#include "unitTest.h"

#ifdef HS_UNIT_TEST

void _unittest_mem(void)
{
  uint32_t frag_cnt, tmp_cnt, free_size, temp_size;
  uint8_t *dptr1, *dptr2, *dptr3, *dptr4;

  /*
   * General memory manage test starting 
   */
  frag_cnt = hs_memInfo(&free_size, __MT_GENERAL);
  hs_printf("1. General memory frag_cnt:%d, free size:%d. \r\n", frag_cnt, free_size);

  /* 1. parameter check start */
  dptr1 = hs_malloc(free_size, __MT_GENERAL);
  __assert(dptr1 == NULL);

  dptr1 = hs_malloc((free_size + 0x10), __MT_GENERAL);
  __assert(dptr1 == NULL);

  dptr1 = hs_malloc((free_size / 4), 0x8000);
  __assert(dptr1 == NULL);
  /* parameter check end */
  
  /* 2. memory size check start */
  dptr1 = hs_malloc(free_size / 3, __MT_GENERAL);
  __assert(dptr1 != NULL);

  dptr2 = hs_malloc(free_size / 3, __MT_GENERAL);
  __assert(dptr2 != NULL);

  dptr3 = hs_malloc(free_size / 3, __MT_GENERAL);
  __assert(dptr3 == NULL);

  dptr3 = hs_malloc(free_size / 3 - 40, __MT_GENERAL);
  __assert(dptr3 != NULL);

  hs_free(dptr2);
  hs_free(dptr1);
  hs_free(dptr3);  

  tmp_cnt = hs_memInfo(&temp_size, __MT_GENERAL);
  hs_printf("2. General memory frag_cnt:%d, free size:%d. \r\n", tmp_cnt, temp_size);
  msleep(100);  
  __assert((tmp_cnt - frag_cnt) <= 1);
  __assert((free_size - temp_size) <= 8);
  /* memory size check end */

  /* 3. generating fragment more and more */
  dptr1 = hs_malloc(free_size / 10, __MT_GENERAL);
  __assert(dptr1 != NULL);

  dptr2 = hs_malloc(free_size / 10, __MT_GENERAL);
  __assert(dptr1 != NULL);

  dptr3 = hs_malloc(free_size / 15, __MT_GENERAL);
  __assert(dptr3 != NULL);

  dptr4 = hs_malloc(free_size / 15, __MT_GENERAL);
  __assert(dptr3 != NULL);

  hs_free(dptr2);
  dptr2 = hs_malloc(free_size / 8, __MT_GENERAL);
  __assert(dptr1 != NULL);

  hs_free(dptr1);  
  dptr1 = hs_malloc(free_size / 8, __MT_GENERAL);
  __assert(dptr1 != NULL);

  hs_free(dptr4);  
  dptr4 = hs_malloc(free_size / 8, __MT_GENERAL);
  __assert(dptr4 != NULL);

  hs_free(dptr3);
  dptr3 = hs_malloc(free_size / 8, __MT_GENERAL);
  __assert(dptr3 != NULL);

  hs_free(dptr1);  
  dptr1 = hs_malloc(free_size / 5, __MT_GENERAL);
  __assert(dptr1 != NULL);

  hs_free(dptr3);
  dptr3 = hs_malloc(free_size / 5, __MT_GENERAL);
  __assert(dptr3 != NULL);

  hs_free(dptr4);  
  dptr4 = hs_malloc(free_size / 5, __MT_GENERAL);
  __assert(dptr4 != NULL);

  hs_free(dptr2);
  dptr2 = hs_malloc(free_size / 5, __MT_GENERAL);
  __assert(dptr1 != NULL);

  tmp_cnt = hs_memInfo(&temp_size, __MT_GENERAL);
  hs_printf("3. General memory frag_cnt:%d, free size:%d. \r\n", tmp_cnt, temp_size);
  msleep(100);

  hs_free(dptr2);
  hs_free(dptr1);
  hs_free(dptr3);
  hs_free(dptr4);  

  tmp_cnt = hs_memInfo(&temp_size, __MT_GENERAL);
  __assert((tmp_cnt - frag_cnt) <= 1);
  __assert((free_size - temp_size) <= 8);

  hs_printf("4. General memory frag_cnt:%d, free size:%d. \r\n\r\nGeneral Memory Test Pass!\r\n\r\n", tmp_cnt, temp_size);
  msleep(100);

  /*
   * dma memory manage test starting 
   */
  frag_cnt = hs_memInfo(&free_size, __MT_DMA);
  hs_printf("1. DMA memory frag_cnt:%d, free size:%d. \r\n", frag_cnt, free_size);

  free_size -= 128 * (frag_cnt - 1);

  /* 1. parameter check start */
  dptr1 = hs_malloc(free_size, __MT_DMA);
  __assert(dptr1 == NULL);

  dptr1 = hs_malloc((free_size + 0x10), __MT_DMA);
  __assert(dptr1 == NULL);

  dptr1 = hs_malloc((free_size / 4), 0x8000);
  __assert(dptr1 == NULL);
  /* parameter check end */
  
  /* 2. memory size check start */
  dptr1 = hs_malloc(free_size / 3, __MT_DMA);
  __assert(dptr1 != NULL);

  dptr2 = hs_malloc(free_size / 3, __MT_DMA);
  __assert(dptr2 != NULL);

  dptr3 = hs_malloc(free_size / 3, __MT_DMA);
  __assert(dptr3 == NULL);

  tmp_cnt = hs_memInfo(&temp_size, __MT_DMA);
  hs_printf("2.1 DMA memory frag_cnt:%d, free size:%d. \r\n", tmp_cnt, temp_size);
  msleep(100);

  dptr3 = hs_malloc(free_size / 3 - 128 * (frag_cnt + 1) - 8, __MT_DMA);
  __assert(dptr3 != NULL);

  hs_free(dptr2);
  hs_free(dptr1);
  hs_free(dptr3);  

  tmp_cnt = hs_memInfo(&temp_size, __MT_DMA);
  hs_printf("2.2 DMA memory frag_cnt:%d, free size:%d. \r\n", tmp_cnt, temp_size);
  msleep(100);
  
  temp_size -= 128 * (frag_cnt - 1);
  __assert((tmp_cnt - frag_cnt) <= 1);
  __assert((temp_size - free_size) <= 8);
  /* memory size check end */

  /* 3. generating fragment more and more */
  dptr1 = hs_malloc(free_size / 10, __MT_DMA);
  __assert(dptr1 != NULL);

  dptr2 = hs_malloc(free_size / 10, __MT_DMA);
  __assert(dptr1 != NULL);

  dptr3 = hs_malloc(free_size / 15, __MT_DMA);
  __assert(dptr3 != NULL);

  dptr4 = hs_malloc(free_size / 15, __MT_DMA);
  __assert(dptr3 != NULL);

  hs_free(dptr2);
  dptr2 = hs_malloc(free_size / 8, __MT_DMA);
  __assert(dptr1 != NULL);

  hs_free(dptr1);  
  dptr1 = hs_malloc(free_size / 8, __MT_DMA);
  __assert(dptr1 != NULL);

  hs_free(dptr4);  
  dptr4 = hs_malloc(free_size / 8, __MT_DMA);
  __assert(dptr4 != NULL);

  hs_free(dptr3);
  dptr3 = hs_malloc(free_size / 8, __MT_DMA);
  __assert(dptr3 != NULL);

  hs_free(dptr1);  
  dptr1 = hs_malloc(free_size / 6, __MT_DMA);
  __assert(dptr1 != NULL);

  hs_free(dptr3);
  dptr3 = hs_malloc(free_size / 6, __MT_DMA);
  __assert(dptr3 != NULL);

  hs_free(dptr4);  
  dptr4 = hs_malloc(free_size / 6, __MT_DMA);
  __assert(dptr4 != NULL);

  hs_free(dptr2);
  dptr2 = hs_malloc(free_size / 5, __MT_DMA);
  __assert(dptr1 != NULL);

  tmp_cnt = hs_memInfo(&temp_size, __MT_DMA);
  hs_printf("3. DMA memory frag_cnt:%d, free size:%d. \r\n", tmp_cnt, temp_size);
  msleep(100);
  
  hs_free(dptr2);
  hs_free(dptr1);
  hs_free(dptr3);
  hs_free(dptr4);  

  tmp_cnt = hs_memInfo(&temp_size, __MT_DMA);
  temp_size -= 128 * (frag_cnt - 1);
  __assert((tmp_cnt - frag_cnt) <= 1);
  __assert((temp_size - free_size) <= 8);

  hs_printf("4. DMA memory frag_cnt:%d, free size:%d. \r\n\r\nDMA Memory Test Pass!\r\n\r\n", tmp_cnt, temp_size);
}

void hs_unittest_mem(void)
{
  hs_printf("\r\n");
  hs_printf("*********************************************************\r\n");
  hs_printf("*                 Memory Manage Unit Test               *\r\n");
  hs_printf("*********************************************************\r\n");
  hs_printf("\r\n");

  _unittest_mem();
}

#endif

/** @} */
