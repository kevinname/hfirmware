/*
 ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio
 Copyright (C) 2014 HunterSun Technologies
 wei.lu@huntersun.com.cn

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#include <string.h>
#include "lib.h"

#define BUFFER_SIZE 2048
#define DMA_RUN_TWO_PATT
#define dma_dbg(chp, fmt,args...)	\
do\
{\
  chSysUnlock();\
  chprintf(chp, fmt, ##args);\
  chSysLock();\
}while(0)

void cmd_dma(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void) argc;
  (void) argv;

  /* MAX_BLK_SIZE is 0x0A, i.e. 4095 bytes */
  uint8_t *patterns1 = 0, *patterns2 = 0, *buf1 = 0, *buf2 = 0;

  patterns1 = hs_malloc(BUFFER_SIZE, __MT_DMA);
  patterns2 = hs_malloc(BUFFER_SIZE, __MT_DMA);
  buf1 = hs_malloc(BUFFER_SIZE, __MT_DMA);
  buf2 = hs_malloc(BUFFER_SIZE, __MT_DMA);

#if defined(HS_DMA_REQUIRED)
  uint32_t i;
  hs_dma_stream_t *dmastp0, *dmastp1;

  chSysLock();

  dma_dbg(chp, "\r\nDma test start! \r\n");

  /* Allocating two DMA streams for memory copy operations.*/
#ifdef DMA_RUN_TWO_PATT
  dmastp0 = dmaStreamAllocate(0, NULL, NULL);
#endif
  dmastp1 = dmaStreamAllocate(0, NULL, NULL);
#ifdef DMA_RUN_TWO_PATT
  if ((NULL == dmastp0) || (NULL == dmastp1))
  {
    dma_dbg(chp, "Allocate dma channel error!\r\n");
    goto dma_test_exit;
  }
#endif
  for (i = 0; i < BUFFER_SIZE; i++)
    patterns1[i] = (uint8_t)i;
  for (i = 0; i < BUFFER_SIZE; i++)
    patterns2[i] = (uint8_t)(i ^ 0xAA);
#ifdef DMA_RUN_TWO_PATT
  /* Copy pattern 1.*/
  dmaStartMemCopy(dmastp0, patterns1, buf1, BUFFER_SIZE);
#endif
  dmaStartMemCopy(dmastp1, patterns1, buf2, BUFFER_SIZE);
#ifdef DMA_RUN_TWO_PATT
  dmaWaitCompletion(dmastp0);
#endif
  dmaWaitCompletion(dmastp1);
#ifdef DMA_RUN_TWO_PATT
  if (memcmp(patterns1, buf1, BUFFER_SIZE))
  {
    dma_dbg(chp, "Dma1:0 run finished, but check result error!\r\n");
    goto dma_test_exit;
  }
#endif  
  if (memcmp(patterns1, buf2, BUFFER_SIZE))
  {
    dma_dbg(chp, "Dma2:0 run finished, but check result error!\r\n");
    goto dma_test_exit;
  }
#ifdef DMA_RUN_TWO_PATT
  /* Copy pattern 2.*/
  dmaStartMemCopy(dmastp0, patterns2, buf1, BUFFER_SIZE);
#endif
  dmaStartMemCopy(dmastp1, patterns2, buf2, BUFFER_SIZE);
#ifdef DMA_RUN_TWO_PATT
  dmaWaitCompletion(dmastp0);
#endif
  dmaWaitCompletion(dmastp1);
#ifdef DMA_RUN_TWO_PATT
  if (memcmp(patterns2, buf1, BUFFER_SIZE))
  {
    dma_dbg(chp, "Dma1:1 run finished, but check result error!\r\n");
    goto dma_test_exit;
  }
#endif
  if (memcmp(patterns2, buf2, BUFFER_SIZE))
  {
    dma_dbg(chp, "Dma1:1 run finished, but check result error!\r\n");
    goto dma_test_exit;
  }

  dma_dbg(chp, "Dma test PASS!\r\n");

  dma_test_exit:
#ifdef DMA_RUN_TWO_PATT
  dmaStreamRelease(dmastp0);
#endif
  dmaStreamRelease(dmastp1);
  chSysUnlock();
#ifndef RUN_RTL_IN_SERVER
  chThdSleepMilliseconds(100);
#endif
#endif

  if(patterns1 != NULL){
    hs_free(patterns1);
  }

  if(patterns2 != NULL){
    hs_free(patterns2);
  }

  if(buf1 != NULL){
    hs_free(buf1);
  }

  if(buf2 != NULL){
    hs_free(buf2);
  }
}
