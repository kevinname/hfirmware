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
#include <stdlib.h> //atoi
#include "lib.h"


/*===========================================================================*/
/* SF driver test code: write & verify SPI NOR Flash                         */
/*===========================================================================*/

#if HAL_USE_SF
#define BUF_SIZE 1024

/* SPI NOR Flash */
static SFConfig sfcfg = {
    NULL,
    WIDTH_8BIT,
    SELECT_CS0,
    CLKMODE_0,
    LOW_ACTIVE,
    400000,
};
static const uint32_t sf_clocks[] = {200*1000, 400*1000, 2000000, 4000000, 8000000, 16*1000000, 24*1000000, 48*1000000};
uint8_t *rxbuf, *txbuf;

void cmd_sf(BaseSequentialStream *chp, int argc, char *argv[]) {

  systime_t tmo = MS2ST(10000);
  uint32_t offset = 0x0000; //offset address in SF
  size_t count = 8;
  uint32_t ii, jj;
  bool_t pass = TRUE;

  /* set pad5 pad6 pad7 pad8 sf function */
  //palSetPadMode(IOPORT0, 5, PAL_MODE_OUTPUT_PULLUP| PAL_MODE_ALTERNATE(0) | PAL_MODE_DRIVE_CAP(3));
  //palSetPadMode(IOPORT0, 6, PAL_MODE_INPUT_PULLUP | PAL_MODE_ALTERNATE(0) | PAL_MODE_DRIVE_CAP(3));
  //palSetPadMode(IOPORT0, 7, PAL_MODE_OUTPUT_PULLUP| PAL_MODE_ALTERNATE(0) | PAL_MODE_DRIVE_CAP(3));
  //palSetPadMode(IOPORT0, 8, PAL_MODE_OUTPUT_PULLUP| PAL_MODE_ALTERNATE(0) | PAL_MODE_DRIVE_CAP(3));

  rxbuf = hs_malloc(BUF_SIZE, __MT_DMA);
  txbuf = hs_malloc(BUF_SIZE, __MT_DMA);
  memset(rxbuf, 0x00, count);

  sfStop(&SFD);  
  if (argc >= 2) {
    sfcfg.speed = atoi(argv[0]);
    offset = strtol(argv[1], NULL, 16);
    count = atoi(argv[2]);
    if (count > BUF_SIZE)
      count = BUF_SIZE;
    
    if (0 == count)
      return;

    sfStart(&SFD, &sfcfg);
    sfReleasePD(&SFD, tmo);
    sfProbe(&SFD, tmo);
    chprintf(chp, "MCLK@%dHz: name=%s mid=0x%08lx page_size=%d sector=%d*%d block=%d*%d\r\n",
                    SFD.mclk, SFD.sfinfo->name, SFD.mid, SFD.sfinfo->page_size,
                    SFD.sfinfo->sector_size, SFD.sfinfo->sector_num,
                    SFD.sfinfo->block_size, SFD.sfinfo->block_num);

    sfAcquireBus(&SFD);
    sfErase(&SFD, 0, /*len*/0, TIME_INFINITE);
    sfRead(&SFD, offset, rxbuf, count, tmo);
    sfReleaseBus(&SFD);
    
    chprintf(chp, "dump SPI NOR flash @0x%04x in %dbytes:\r\n", offset, count);
    for (jj = 0; jj < count; jj++)
    {
      if(((jj % 16) == 0) && (jj != 0))
        chprintf(chp, "\r\n");
      
      chprintf(chp, "0x%02x ", rxbuf[jj]);
    }
    chprintf(chp, "\r\n");
    
    for (jj = 0; jj < count; jj++)
    {
      rxbuf[jj] = jj;
    }
    
    sfAcquireBus(&SFD);
    sfWrite(&SFD, offset, rxbuf, count, tmo);    
    memset(rxbuf, 0, count);
    sfRead(&SFD, offset, rxbuf, count, tmo);
    sfReleaseBus(&SFD);
    
    chprintf(chp, "dump SPI NOR flash @0x%04x in %dbytes:\r\n", offset, count);
    for (jj = 0; jj < count; jj++)
    {
      if(((jj % 16) == 0) && (jj != 0))
        chprintf(chp, "\r\n");
      
      chprintf(chp, "0x%02x ", rxbuf[jj]);
    }
    chprintf(chp, "\r\n");
    
    sfDeepPD(&SFD, tmo);
    //sfStop(&SFD);

    hs_free(rxbuf);
    hs_free(txbuf);
    rxbuf = txbuf = NULL;

    return;
  }

  chprintf(chp, "Usage: sf [speed offset count]\r\n");
  chprintf(chp, "           speed in Hz\r\n"); 
  chprintf(chp, "           count: 0 to erase chip\r\n");

  for (ii = 0; ii < sizeof(sf_clocks)/sizeof(uint32_t); ii++) {
    /* adjust SF Master clock to validate change sf source clk */
	uint32_t src_clk = ii<=3 ? 48000000 : 96000000;
	cpm_set_clock(HS_SF_CLK, src_clk);
    sfcfg.speed = sf_clocks[ii];
    sfStart(&SFD, &sfcfg);

    sfProbe(&SFD, tmo);
    chprintf(chp, "MCLK@%dkHz: MID=0x%08lx\r\n", SFD.mclk/1000, SFD.mid);
    if (((SFD.mid & 0xffffff) == 0xffffff) || ((SFD.mid & 0xffffff) == 0)) {
      pass = FALSE;
      goto sf_test_end;
    }

    /* get random count */
    chSysLock();
    count = rand() & (BUF_SIZE - 1);
    chSysUnlock();
    count &= ~(SFD.sfinfo->page_size - 1);
    if (0 == count)
      count = SFD.sfinfo->page_size;
    offset = count;

    /* fill random pattern */
    for (jj=0; jj < count; jj++)
      txbuf[jj] = rand();
    memset(rxbuf, 0x00, count);

    sfAcquireBus(&SFD);
    /* erase then verify */
    sfErase(&SFD, offset, count, TIME_INFINITE);
    sfRead(&SFD, offset, rxbuf, count, TIME_INFINITE);
    for (jj=0; jj<count; jj++) {
      if (0xFF != rxbuf[jj]) {
	chprintf(chp, "%d: 0x%02x\r\n", jj, rxbuf[jj]);
	pass = FALSE;
	goto sf_test_end;
      }
    }

    /* write then verify */
    sfWrite(&SFD, offset, txbuf, count, TIME_INFINITE);
#ifndef RUN_RTL_IN_SERVER
    chThdSleepMilliseconds(100);
#endif
    sfRead(&SFD, offset, rxbuf, count, TIME_INFINITE);
    for (jj=0; jj<count; jj++) {
      if (txbuf[jj] != rxbuf[jj]) {
	chprintf(chp, "0x%08lx: 0x%02x -> 0x%02x\r\n",
                offset+jj, txbuf[jj], rxbuf[jj]);
	pass = FALSE;
	goto sf_test_end;
      }
    }

    /* chip erase */
    //sfErase(&SFD, 0, /*len*/0, TIME_INFINITE);

  sf_test_end:
    sfReleaseBus(&SFD);
    sfStop(&SFD);
  }

  sfStart(&SFD, &sfcfg);
  sfProbe(&SFD, TIME_INFINITE);
  
  if (pass)
    chprintf(chp, "PASS\r\n");
  else
    chprintf(chp, "FAIL\r\n");

  hs_free(rxbuf);
  hs_free(txbuf);
  rxbuf = txbuf = NULL;
}

void cmd_erase(BaseSequentialStream *chp, int argc, char *argv[]) {

  uint32_t offset, length;

  if (argc != 2) {
    chprintf(chp, "Usage: sferase offset length\r\n");
    return;
  }

  offset = strtol(argv[0], NULL, 16);
  length  = strtol(argv[1], NULL, 16);

  sfAcquireBus(&SFD);
  sfReleasePD(&SFD, TIME_INFINITE);
  sfErase(&SFD, offset, length, TIME_INFINITE);
  sfReleaseBus(&SFD);

  chprintf(chp, "flash [0x%08X, 0x%08X] erase over!\r\n", offset, (offset+length));
}

void cmd_SetMode(BaseSequentialStream *chp, int argc, char *argv[]) {

  sfqe_t mode;
  uint32_t status;

  if (argc != 1) {
    chprintf(chp, "Usage: sfmode mode\r\n\tmode: 0-single mode 1-quad mode");
    return;
  }

  mode = atoi(argv[0]) ? QUAD_ENABLE : QUAD_DISABLE;

  sfAcquireBus(&SFD);
  sfQuadEn(&SFD, mode, TIME_INFINITE);
  status = sfReadStatus(&SFD, TIME_INFINITE);
  sfReleaseBus(&SFD);

  chprintf(chp, "flash status:0x%04X, ", status);  
  if(((status >> 9) & 1) == mode )
    chprintf(chp, "Set mode success!\r\n");  
  else
    chprintf(chp, "Set mode failed!\r\n");  
  
}

void cmd_ReadStatus(BaseSequentialStream *chp, int argc, char *argv[]) {

  uint32_t status;

  (void)argv;
  if (argc != 0) {
    chprintf(chp, "Usage: sfrds\r\n");
    return;
  }

  sfAcquireBus(&SFD);
  status = sfReadStatus(&SFD, TIME_INFINITE);
  sfReleaseBus(&SFD);

  chprintf(chp, "flash status:0x%04X. \r\n", status);  
}

#define T_WRITE_SIZE   0x4000
void cmd_sfwrite(BaseSequentialStream *chp, int argc, char *argv[]) {

  uint32_t i, offset, wdata;
  uint8_t *pu8Ptr;
  uint32_t *pu32Ptr;

  if ((argc != 1) && (argc != 2)) {
    chprintf(chp, "Usage: sfwr offset [pattern]\r\n");
    return;
  }

  pu8Ptr = (uint8_t *)hs_malloc(T_WRITE_SIZE, __MT_DMA);
  if(pu8Ptr == NULL) {
    chprintf(chp, "Alloc memory error!\r\n");
    return ;
  }

  offset = strtol(argv[0], NULL, 16);  
  if(argc == 1) {
    for(i=0; i<T_WRITE_SIZE; i++)
      pu8Ptr[i] = i;
  }
  else {    
    wdata  = strtol(argv[1], NULL, 16);
    pu32Ptr = (uint32_t *)pu8Ptr;
    for(i=0; i<T_WRITE_SIZE/4; i++)
      pu32Ptr[i] = wdata;
  }

  i=0;
  do{
  sfAcquireBus(&SFD);  
  sfWrite(&SFD, offset, pu8Ptr, T_WRITE_SIZE, TIME_INFINITE);
  sfReleaseBus(&SFD);
  }while(i<1000);

  hs_free((void *)pu8Ptr);
  chprintf(chp, "flash write over!\r\n");
}

void cmd_sfRead(BaseSequentialStream *chp, int argc, char *argv[]) {

  uint32_t i, offset, len;
  uint8_t *pu8Ptr;

  if (argc != 2) {
    chprintf(chp, "Usage: sfrd offset len\r\n");
    return;
  }

  offset = strtol(argv[0], NULL, 16); 
  len    = strtol(argv[1], NULL, 16);

  pu8Ptr = (uint8_t *)hs_malloc(len, __MT_DMA);
  if(pu8Ptr == NULL) {
    chprintf(chp, "Alloc memory error!\r\n");
    return ;
  }

  sfAcquireBus(&SFD);  
  sfRead(&SFD, offset, pu8Ptr, len, TIME_INFINITE);
  sfReleaseBus(&SFD);

  for(i=0; i<len; i++)
  {
    if((i%16) == 0)
      chprintf(chp, "\r\n%08X:\t", offset+i);

    chprintf(chp, "%02X ", pu8Ptr[i]);
  }

  chprintf(chp, "\r\n\r\n");

  hs_free((void *)pu8Ptr);
  chprintf(chp, "flash read over!\r\n");
}


void cmd_sfClk(BaseSequentialStream *chp, int argc, char *argv[]) {

  uint32_t clock;

  if (argc != 1) {
    chprintf(chp, "Usage: sfclk clock\r\n");
    return;
  }

  clock = atoll(argv[0]); 
  sfcfg.speed = clock;

  sfStop(&SFD);
  
  sfStart(&SFD, &sfcfg);
  sfProbe(&SFD, TIME_INFINITE);

  chprintf(chp, "flash id: 0x%06X\r\n", SFD.mid);

}

void cmd_sfTestWrite(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  uint8_t *pu8Ptr;
  uint32_t i, offset = 0x80000;
  
  HS_SYS->DEBUG_MON_ID = 0x60;
  
  //palSetPadMode(IOPORT0, 10 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3)); // 0
  //palSetPadMode(IOPORT0, 12 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3)); // 1
  palSetPadMode(IOPORT0, 14 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3)); // 2
  //palSetPadMode(IOPORT1, 0 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3));  // 3
  //palSetPadMode(IOPORT1, 1 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3));  // 4
  //palSetPadMode(IOPORT1, 3 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3));  // 6
  //palSetPadMode(IOPORT1, 4 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3));  // 7
  //palSetPadMode(IOPORT0, 2 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3));  // 8
  palSetPadMode(IOPORT0, 7 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3));  // 9
  //palSetPadMode(IOPORT0, 9 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3));  // 10
  //palSetPadMode(IOPORT0, 11 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3)); // 11

  palSetPadMode(IOPORT1, 2 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3)); // 14
  palSetPadMode(IOPORT1, 6 , PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_DEBUG)|PAL_MODE_DRIVE_CAP(3)); // 15

  pu8Ptr = (uint8_t *)hs_malloc(T_WRITE_SIZE, __MT_DMA);
  if(pu8Ptr == NULL) {
    chprintf(chp, "Alloc memory error!\r\n");
    return ;
  }

  for(i=0; i<T_WRITE_SIZE; i++)
    pu8Ptr[i] = i;

  i = 0;
  while(1) 
  {
    sfWrite(&SFD, offset, pu8Ptr, T_WRITE_SIZE, TIME_INFINITE);
    
    offset += T_WRITE_SIZE;
    chprintf(chp, ".");

    i += 1;
    if((i%16) == 0)
      chprintf(chp, "\r\n");
  };
}

#endif /* HAL_USE_SF */
