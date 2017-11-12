/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio
                 Copyright (C) 2014 HunterSun Technologies
                 hongwei.li@huntersun.com.cn

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
#include <stdlib.h> //rand()
#include "lib.h"

#if HAL_USE_SDC

#define SDC_LARGE_FILE_RW_TEST     FALSE
#define SDC_DATA_DESTRUCTIVE_TEST  TRUE//FALSE
#define SDC_BURST_SIZE      11 /* how many sectors reads at once */

/**
 * @brief   Parody of UNIX badblocks program.
 *
 * @param[in] start       first block to check
 * @param[in] end         last block to check
 * @param[in] blockatonce number of blocks to check at once
 * @param[in] pattern     check pattern
 *
 * @return              The operation status.
 * @retval SDC_SUCCESS  operation succeeded, the requested blocks have been
 *                      read.
 * @retval SDC_FAILED   operation failed, the state of the buffer is uncertain.
 */
bool_t badblocks(uint8_t *inbuf, uint8_t *outbuf, uint32_t start, uint32_t end, uint32_t blockatonce, uint8_t pattern){
  uint32_t position = 0;
  uint32_t i = 0;

  osalDbgCheck(blockatonce <= SDC_BURST_SIZE);

  /* fill control buffer */
  for (i=0; i < MMCSD_BLOCK_SIZE * blockatonce; i++)
    outbuf[i] = pattern;

  /* fill SD card with pattern. */
  position = start;
  while (position < end){
    if (sdcWrite(&SDCD0, position, outbuf, blockatonce))
      goto ERROR;
    position += blockatonce;
  }

  /* read and compare. */
  position = start;
  while (position < end){
    if (sdcRead(&SDCD0, position, inbuf, blockatonce))
      goto ERROR;
    if (memcmp(inbuf, outbuf, blockatonce * MMCSD_BLOCK_SIZE) != 0)
      goto ERROR;
    position += blockatonce;
  }
  return FALSE;

ERROR:
  return TRUE;
}

/**
 *
 */
void fillbuffer(uint8_t pattern, uint8_t *b){
  uint32_t i = 0;
  for (i=0; i < MMCSD_BLOCK_SIZE * SDC_BURST_SIZE; i++)
    b[i] = pattern + rand();
}

/**
 *
 */
void fillbuffers(uint8_t *inbuf, uint8_t *outbuf, uint8_t pattern){
  fillbuffer(pattern, inbuf);
  fillbuffer(pattern, outbuf);
}

/**
 *
 */
void cmd_sd(BaseSequentialStream *chp, int argc, char *argv[]){
  (void)argc;
  (void)argv;
  static const char *mode[] = {"SDV11", "SDV20", "MMC", NULL};
  FATFS *fatfs = NULL;
  systime_t start, end;
  uint32_t n, startblk;

  uint8_t *outbuf = 0, *inbuf = 0;

  chprintf(chp, "Trying to connect SD... ");

  outbuf = hs_malloc(MMCSD_BLOCK_SIZE * SDC_BURST_SIZE + 1, __MT_DMA);
  inbuf = hs_malloc(MMCSD_BLOCK_SIZE * SDC_BURST_SIZE + 1, __MT_DMA);

  palSetPadMode(IOPORT0, 6, PAL_MODE_INPUT_PULLUP | PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB) | PAL_MODE_DRIVE_CAP(3));
  palSetPadMode(IOPORT0, 8, PAL_MODE_INPUT_PULLUP | PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB) | PAL_MODE_DRIVE_CAP(3));
  palSetPadMode(IOPORT0, 10, PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB) | PAL_MODE_DRIVE_CAP(3));
  //palSetPadMode(IOPORT0, 11, PAL_MODE_INPUT_PULLUP | PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB) | PAL_MODE_DRIVE_CAP(3));
  //palSetPadMode(IOPORT0, 12, PAL_MODE_INPUT_PULLUP | PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB) | PAL_MODE_DRIVE_CAP(3));
  //palSetPadMode(IOPORT0, 13, PAL_MODE_INPUT_PULLUP | PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB) | PAL_MODE_DRIVE_CAP(3));

#ifndef RUN_RTL_IN_SERVER
  chThdSleepMilliseconds(100);
#endif
  if (!sdcConnect(&SDCD0)) {

    chprintf(chp, "OK\r\n\r\nCard Info\r\n");
    chprintf(chp, "CSD      : %08X %8X %08X %08X \r\n",
             SDCD0.csd[3], SDCD0.csd[2], SDCD0.csd[1], SDCD0.csd[0]);
    chprintf(chp, "CID      : %08X %8X %08X %08X \r\n",
             SDCD0.cid[3], SDCD0.cid[2], SDCD0.cid[1], SDCD0.cid[0]);
    chprintf(chp, "Mode     : %s\r\n", mode[SDCD0.cardmode & SDC_MODE_CARDTYPE_MASK]);
    chprintf(chp, "Capacity : %DMB\r\n", SDCD0.capacity / 2048);

    /* The test is performed in the middle of the flash area.*/
    startblk = (SDCD0.capacity / MMCSD_BLOCK_SIZE) / 2;

    /* Single block read performance, aligned.*/
    chprintf(chp, "Single block aligned read performance:           ");
    start = chVTGetSystemTime();
    end = start + MS2ST(1000);
    n = 0;

    do {
      if (blkRead(&SDCD0, startblk, inbuf, 1)) {
        chprintf(chp, "failed\r\n");
        goto exittest;
      }
      n++;
    } while (chVTIsSystemTimeWithin(start, end));
    chprintf(chp, "%D blocks/S, %D bytes/S\r\n", n, n * MMCSD_BLOCK_SIZE);

    /* Multiple sequential blocks read performance, aligned.*/
    chprintf(chp, "16 sequential blocks aligned read performance:   ");
    start = chVTGetSystemTime();
    end = start + MS2ST(1000);
    n = 0;
    do {
      if (blkRead(&SDCD0, startblk, inbuf, SDC_BURST_SIZE)) {
        chprintf(chp, "failed\r\n");
        goto exittest;
      }
      n += SDC_BURST_SIZE;
    } while (chVTIsSystemTimeWithin(start, end));
    chprintf(chp, "%D blocks/S, %D bytes/S\r\n", n, n * MMCSD_BLOCK_SIZE);

    /* Single block read performance, unaligned.*/
    chprintf(chp, "Single block unaligned read performance:         ");
    start = chVTGetSystemTime();
    end = start + MS2ST(1000);
    n = 0;
    do {
      if (blkRead(&SDCD0, startblk, inbuf + 1, 1)) {
        chprintf(chp, "failed\r\n");
        goto exittest;
      }
      n++;
    } while (chVTIsSystemTimeWithin(start, end));
    chprintf(chp, "%D blocks/S, %D bytes/S\r\n", n, n * MMCSD_BLOCK_SIZE);

    /* Multiple sequential blocks read performance, unaligned.*/
    chprintf(chp, "16 sequential blocks unaligned read performance: ");
    start = chVTGetSystemTime();
    end = start + MS2ST(1000);
    n = 0;
    do {
      if (blkRead(&SDCD0, startblk, inbuf + 1, SDC_BURST_SIZE)) {
        chprintf(chp, "failed\r\n");
        goto exittest;
      }
      n += SDC_BURST_SIZE;
    } while (chVTIsSystemTimeWithin(start, end));
    chprintf(chp, "%D blocks/S, %D bytes/S\r\n", n, n * MMCSD_BLOCK_SIZE);

#if SDC_DATA_DESTRUCTIVE_TEST

    chprintf(chp, "Single aligned write...");
#ifndef RUN_RTL_IN_SERVER
    chThdSleepMilliseconds(100);
#endif
    fillbuffer(0xAA, inbuf);

    if (sdcWrite(&SDCD0, 0, inbuf, 1)){
      chprintf(chp, " Failed\r\n");
      return;
    }
#ifndef RUN_RTL_IN_SERVER
    chThdSleepMilliseconds(100);
#endif
    fillbuffer(0, outbuf);
    if (sdcRead(&SDCD0, 0, outbuf, 1)){
      chprintf(chp, " Failed\r\n");
      return;
    }

    if (memcmp(inbuf, outbuf, MMCSD_BLOCK_SIZE) != 0){
      chprintf(chp, " Failed\r\n");
      return;
    }
    chprintf(chp, " OK\r\n");
    chprintf(chp, "Single unaligned write...");
#ifndef RUN_RTL_IN_SERVER
    chThdSleepMilliseconds(100);
#endif
    fillbuffer(0xFF, inbuf);
    if (sdcWrite(&SDCD0, 12, inbuf+1, 1)){
      chprintf(chp, " Failed\r\n");
      return;
    }
    fillbuffer(0, outbuf);
    if (sdcRead(&SDCD0, 12, outbuf+1, 1)){
      chprintf(chp, " Failed\r\n");
      return;
    }
    if (memcmp(inbuf+1, outbuf+1, MMCSD_BLOCK_SIZE) != 0){
      chprintf(chp, " Failed\r\n");
      return;
    }
    chprintf(chp, " OK\r\n");

    chprintf(chp, "Running badblocks at 0x1000 offset...");
#ifndef RUN_RTL_IN_SERVER
    chThdSleepMilliseconds(100);
#endif
    if(badblocks(inbuf, outbuf, 0x1000, 0x1100, SDC_BURST_SIZE, 0xAA)){
      chprintf(chp, " Failed\r\n");
      return;
    }
    chprintf(chp, " OK\r\n");

#endif /* !SDC_DATA_DESTRUCTIVE_TEST */


    /**
     * Now perform some FS tests.
     */

    if(fatfs == NULL){
      fatfs = (FATFS *)hs_malloc(sizeof(FATFS), __MT_DMA);
    }
    FRESULT err;
    uint32_t clusters;
    FATFS *fsp;
    FIL *fileObject = 0;
    uint32_t bytes_written;
    uint32_t bytes_read;
    FILINFO filinfo;
    uint8_t teststring[] = {"This is test file\r\n"};

    fileObject = (FIL *)hs_malloc(sizeof(FIL), __MT_DMA);

    chprintf(chp, "Register working area for filesystem... ");
#ifndef RUN_RTL_IN_SERVER
    chThdSleepMilliseconds(100);
#endif
    /* Do not mount (delayed mount) */
    err = f_mount(fatfs, _T("0:"), 0);
    if (err != FR_OK){
#ifndef RUN_RTL_IN_SERVER
      chprintf(chp, " Failed\r\n");
#else
      rtl_print("SD Failed!!!\r\n");
#endif
      return;
    }
    else{
      chprintf(chp, "OK\r\n");
    }

#if SDC_DATA_DESTRUCTIVE_TEST
    chprintf(chp, "Formatting... ");
#ifndef RUN_RTL_IN_SERVER
    chThdSleepMilliseconds(100);
#endif
    err = f_mkfs(_T("0:"),0,0);
    if (err != FR_OK){
      chprintf(chp, " Failed\r\n");
      return;
    }
    else{
      chprintf(chp, "OK\r\n");
    }
#endif /* SDC_DATA_DESTRUCTIVE_TEST */


    chprintf(chp, "Mount filesystem... ");
#ifndef RUN_RTL_IN_SERVER
    chThdSleepMilliseconds(100);
#endif
    err = f_getfree((const TCHAR *)_T("/"), ((DWORD *)&clusters), &fsp);
    if (err != FR_OK) {
      chprintf(chp, " Failed\r\n");
      return;
    }
    chprintf(chp, "OK\r\n");
    chprintf(chp,
             "FS: %lu free clusters, %lu sectors per cluster, %lu bytes free\r\n",
             clusters, (uint32_t)fatfs->csize,
             clusters * (uint32_t)fatfs->csize * (uint32_t)MMCSD_BLOCK_SIZE);

    chprintf(chp, "Create file \"chtest.txt\"... ");
#ifndef RUN_RTL_IN_SERVER
    chThdSleepMilliseconds(100);
#endif
    err = f_open(fileObject, (const TCHAR *)_T("0:chtest.txt"), FA_WRITE | FA_OPEN_ALWAYS);
    if (err != FR_OK) {
      chprintf(chp, " Failed\r\n");
      return;
    }
    chprintf(chp, "OK\r\n");
    chprintf(chp, "Write some data in it... ");
#ifndef RUN_RTL_IN_SERVER
    chThdSleepMilliseconds(100);
#endif
    err = f_write(fileObject, teststring, sizeof(teststring), (void *)&bytes_written);
    if (err != FR_OK) {
      chprintf(chp, " Failed\r\n");
      return;
    }
    else{
      chprintf(chp, "OK\r\n");
    }

    chprintf(chp, "Close file \"chtest.txt\"... ");
    err = f_close(fileObject);
    if (err != FR_OK) {
      chprintf(chp, " Failed\r\n");
      return;
    }
    else{
      chprintf(chp, "OK\r\n");
    }

    chprintf(chp, "Check file size \"chtest.txt\"... ");
    memset(&filinfo, 0, sizeof(FILINFO));
    err = f_stat((const TCHAR *)_T("0:chtest.txt"), &filinfo);
#ifndef RUN_RTL_IN_SERVER
    chThdSleepMilliseconds(100);
#endif
    if (err != FR_OK) {
      chprintf(chp, " Failed\r\n");
      return;
    }
    else{
      if (filinfo.fsize == sizeof(teststring))
        chprintf(chp, "OK\r\n");
      else{
      chprintf(chp, " Failed\r\n");
        return;
      }
    }


    chprintf(chp, "Check file content \"chtest.txt\"... ");
    err = f_open(fileObject, (const TCHAR *)_T("0:chtest.txt"), FA_READ | FA_OPEN_EXISTING);
#ifndef RUN_RTL_IN_SERVER
    chThdSleepMilliseconds(100);
#endif
    if (err != FR_OK) {
      chprintf(chp, " Failed\r\n");
      return;
    }
    uint8_t buf[sizeof(teststring)];
    err = f_read(fileObject, buf, sizeof(teststring), (void *)&bytes_read);
    if (err != FR_OK) {
      chprintf(chp, " Failed\r\n");
      return;
    }
    else{
      if (memcmp(teststring, buf, sizeof(teststring)) != 0){
      chprintf(chp, " Failed\r\n");
      return;
      }
      else{
        chprintf(chp, "OK\r\n");
      }
    }

#if SDC_LARGE_FILE_RW_TEST
    /* when len=40000, the file size is 156MB */
    uint32_t len =  40000;   //clusters * (uint32_t)SDC_FS.csize / 4 ;
    uint32_t count = 40000;  //clusters * (uint32_t)SDC_FS.csize / 4 ;

    chprintf(chp, "large file read & write test...\r\n");
    fillbuffer(0xAA, inbuf);
    chprintf(chp, "Create file \"large_test.txt\"... ");
#ifndef RUN_RTL_IN_SERVER
    chThdSleepMilliseconds(100);
#endif
    err = f_open(&fileObject, "0:large_test.txt", FA_WRITE | FA_CREATE_NEW);
    if (err != FR_OK) {
      chprintf(chp, " Failed\r\n");
      return;
    }
    chprintf(chp, "OK\r\n");
    chprintf(chp, "Write data in ... ");
    do {
#ifndef RUN_RTL_IN_SERVER
      chThdSleepMilliseconds(100);
#endif
      err = f_write(&fileObject, inbuf, sizeof(inbuf), (void *)&bytes_written);
      if (err != FR_OK) {
      chprintf(chp, " Failed\r\n");
        return;
      }
     chprintf(chp, "W");
    } while(len--);
    chprintf(chp, "OK\r\n");

    chprintf(chp, "Close file \"large_test.txt\"... ");
    err = f_close(&fileObject);
    if (err != FR_OK) {
      chprintf(chp, " Failed\r\n");
      return;
    }
    else{
      chprintf(chp, "OK\r\n");
    }

    chprintf(chp, "Read data Out ... ");
    err = f_open(&fileObject, "0:large_test.txt", FA_READ | FA_OPEN_EXISTING);
#ifndef RUN_RTL_IN_SERVER
    chThdSleepMilliseconds(100);
#endif
    if (err != FR_OK) {
      chprintf(chp, " Failed\r\n");
      return;
    }
    do {
      memset(outbuf, 0, sizeof(outbuf));
      err = f_read(&fileObject, outbuf, sizeof(outbuf), (void *)&bytes_read);
      if (err != FR_OK) {
      chprintf(chp, " Failed\r\n");
        return;
      }
      else{
        if (memcmp(inbuf, outbuf, sizeof(outbuf)) != 0){
      chprintf(chp, " Failed\r\n");
        return;
        }
      }
      count--;
      chprintf(chp, "R");
    } while(count);
    chprintf(chp, "OK\r\n");

    chprintf(chp, "Close file \"large_test.txt\"... ");
    err = f_close(&fileObject);
    if (err != FR_OK) {
      chprintf(chp, " Failed\r\n");
      return;
    }
    else{
      chprintf(chp, "OK\r\n");
    }

#endif
    if(fileObject != NULL){
      hs_free(fileObject);
    }

    chprintf(chp, "Umount filesystem... ");
    f_mount(NULL, _T("/"), 1);
    chprintf(chp, "OK\r\n");

    chprintf(chp, "Disconnecting from SD...");
#ifndef RUN_RTL_IN_SERVER
    chThdSleepMilliseconds(100);
#endif
    if (sdcDisconnect(&SDCD0)){
      chprintf(chp, " Failed\r\n");
      return;
    }
    chprintf(chp, " OK\r\n");
    chprintf(chp, "------------------------------------------------------\r\n");

    chprintf(chp, "All tests passed successfully.\r\n");
#ifndef RUN_RTL_IN_SERVER
    chThdSleepMilliseconds(100);
#endif
  }
  else{
      chprintf(chp, " Failed\r\n");
#ifndef RUN_RTL_IN_SERVER
    chThdSleepMilliseconds(100);
#endif
  }

  /* Card disconnect and command end.*/
exittest:
  sdcDisconnect(&SDCD0);

  if (inbuf != NULL) {
    hs_free(inbuf);
  }

  if (outbuf != NULL) {
    hs_free(outbuf);
  }
}

#endif /* HAL_USE_SDC */
