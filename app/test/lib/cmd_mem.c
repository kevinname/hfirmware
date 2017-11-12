#include <string.h>
#include <stdlib.h> 
#include "lib.h"

void cmd_memAlloc(BaseSequentialStream *chp, int argc, char *argv[]) {

  uint32_t size, t;
  memtype_t type;
  uint8_t *pu8Ptr;

  if(argc != 2) {
    chprintf(chp, "Usage: malloc type size\r\n\t0-general memory 1-dma memory\r\n");
    return;
  }

  t = strtol(argv[0], NULL, 16); 
  size = strtol(argv[1], NULL, 16); 
  if(size == 0) {
    chprintf(chp, "Paramter Error!\r\nUsage: malloc type size\r\n\t0-general memory 1-dma memory\r\n");
    return;
  }

  type = t == 0 ? __MT_GENERAL : __MT_DMA;
  pu8Ptr = (uint8_t *)hs_malloc(size, type);  
  if(pu8Ptr == NULL) {
    chprintf(chp, "Can not alloc so much size(0x%x bytes) memory!\r\n", size);
    return;
  }

  chprintf(chp, "Alloc memory successful! memory address: 0x%08X\r\n", (uint32_t)pu8Ptr);
}

void cmd_memFree(BaseSequentialStream *chp, int argc, char *argv[]) {

  uint32_t addr;
  void *pPtr;

  if(argc != 1) {
    chprintf(chp, "Usage: mfree address\r\n");
    return;
  }

  addr = strtoul(argv[0], NULL, 16); 
  pPtr = (uint8_t *)addr;
  if(pPtr == NULL) {
    chprintf(chp, "Paramter Error!\r\nUsage: mfree address\r\n");
    return;
  }

  hs_free(pPtr);  
  chprintf(chp, "free memory over! memory address: 0x%08X\r\n", (uint32_t)pPtr);
}

