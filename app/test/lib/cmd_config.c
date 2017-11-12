#include <string.h>
#include <stdlib.h> 
#include "lib.h"


void cmd_cfgread(BaseSequentialStream *chp, int argc, char *argv[]) {

  uint8_t *ptr, index;
  uint32_t i, fos, len;
  hs_cfg_res_t res;
  
  if(argc != 2) {
    chprintf(chp, "Usage: cfgread index length\r\n");
    return;
  }

  index = strtol(argv[0], NULL, 16);
  len   = strtol(argv[1], NULL, 16);

  if(len == 0)
    return ;
  
  ptr = hs_malloc(len, __MT_Z_GENERAL);
  if(ptr == NULL)
  {    
    chprintf(chp, "Have not enough memory!\r\n");
    return;
  }
  
  res = hs_cfg_getOffsetByIndex(index, &fos);
  if(res != HS_CFG_OK)
  {
    chprintf(chp, "Read config offset error, code: 0x%x\r\n", res);
    hs_free(ptr);
    return ;
  }

  res = hs_cfg_getDataByIndex(index, ptr, len);
  if(res == HS_CFG_OK)
  {
    for(i=0; i<len; i++)
    {
      if((i%16) == 0)
        chprintf(chp, "\r\n%08X:\t", fos+i+0x80000000);

      chprintf(chp, "%02X ", ptr[i]);
    }
  }
  else
    chprintf(chp, "Read config data error, code: 0x%x\r\n", res);

  hs_free(ptr);
  chprintf(chp, "\r\n\r\n");
}

void cmd_cfgwrite(BaseSequentialStream *chp, int argc, char *argv[]) {

  uint8_t index;
  uint32_t *u32ptr, i, mlen, fos, len, pattern = 0x12345678;
  hs_cfg_res_t res;
  
  if((argc != 2) && (argc != 3)) {
    chprintf(chp, "Usage: cfgwrite index length [pattern]\r\n");
    return;
  }

  index = strtol(argv[0], NULL, 16);
  len   = strtol(argv[1], NULL, 16);
  if(argc == 3)
    pattern = strtol(argv[2], NULL, 16);

  if(len == 0)
    return ;

  mlen = (len + 3) & ~3u;
  u32ptr = (uint32_t *)hs_malloc(mlen, __MT_Z_GENERAL);
  if(u32ptr == NULL)
  {    
    chprintf(chp, "Have not enough memory!\r\n");
    return;
  }

  for(i=0; i<mlen/4; i++)
    u32ptr[i] = pattern;
  
  res = hs_cfg_getOffsetByIndex(index, &fos);
  if(res != HS_CFG_OK)
  {
    chprintf(chp, "get config data offset error, code: 0x%x\r\n", res);
    hs_free(u32ptr);
    return ;
  }

  chprintf(chp, "config data[%d] offset:0x%08X !\r\n", index, fos+0x80000000);
  res = hs_cfg_setDataByIndex(index, (uint8_t *)u32ptr, len, 0);
  if(res == HS_CFG_OK)
  {
    chprintf(chp, "Write config data successful!\r\n");
  }
  else
  {
    chprintf(chp, "Write config data error, code: 0x%x\r\n", res);
  }

  hs_free(u32ptr);
  chprintf(chp, "\r\n");
}

void cmd_cfgflush(BaseSequentialStream *chp, int argc, char *argv[]) {

  hs_cfg_flushtype_t ft = FLUSH_TYPE_ONEBLOCK;
  hs_cfg_res_t res;
  
  if(argc > 2) {
    chprintf(chp, "Usage: cfgflush [type]\r\n\t0-one block, 1-all\r\n");
    return;
  }

  if(argc == 1)
    ft = atoi(argv[0]); 
  
  res = hs_cfg_flush(ft);
  if(res == HS_CFG_OK)
  {
    chprintf(chp, "flush flash data successful!\r\n");
  }
  else
  {
    chprintf(chp, "flush flash data error!\r\n");
  }
}

