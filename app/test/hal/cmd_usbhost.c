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
#include "ch.h"
#include "hal.h"
#include "ff.h"
#include "lib.h"

#if HAL_USE_USB_HOST_STORAGE
#define USB_DRIVER_NUMBER _T("1:")
#define USBH_BLOCK_SIZE             512
#define USBH_BURST_SIZE             8 /* how many sectors reads at once */
#define USBH_LARGE_FILE_RW_TEST     FALSE
#define BLOCK_RW_NUM                4//1000
#endif

void cmd_usbhost(BaseSequentialStream *chp, int argc, char *argv[])
{
#if HAL_USE_USB_HOST_STORAGE
  (void)argc;
  (void)argv;
  USBHostDriver *usbhp = &USBHOST0;
  FRESULT err;
  uint32_t clusters;
  FATFS *fsp;
  uint32_t bytes_written;
  uint32_t bytes_read;
  FILINFO filinfo;
  uint8_t teststring[] = {"This is test file\r\n"};
  uint8_t buf[sizeof(teststring)];
  uint16_t i=0;
  systime_t t1;
  systime_t t2;
  systime_t t3;
  systime_t t4;
  uint32_t speed1 = 0;
  uint32_t speed2 = 0;
  uint8_t *outbuf = 0;
  uint8_t *inbuf = 0;
  FATFS *usbh_fs = 0;
  FIL *fileObject = 0;

  chprintf(chp, "Trying to find USB host connecting storage dev...\n\r");

  palSetPadMode(IOPORT0, 14, PAL_MODE_INPUT|PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB)|PAL_MODE_DRIVE_CAP(3));   //dp
  palSetPadMode(IOPORT1, 0, PAL_MODE_INPUT_PULLDOWN|PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB)|PAL_MODE_DRIVE_CAP(3));    //dm
  palSetPadMode(IOPORT1, 1, PAL_MODE_INPUT|PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB)|PAL_MODE_DRIVE_CAP(3));    //cid
  //palSetPadMode(IOPORT1, 7, PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB)|PAL_MODE_DRIVE_CAP(3));   //vbus_en

  usbSetPowerSessMode(&USBD1, USB_POWER_CONTROL_BY_REG);

  if((usbh_fs = (FATFS *)hs_malloc(sizeof(FATFS), __MT_DMA)) == 0) {
    chprintf(chp, "hs_malloc failed\r\n");
    goto cmd_usbhost_fail;
  }

  if((fileObject = (FIL *)hs_malloc(sizeof(FIL), __MT_DMA)) == 0) {
    chprintf(chp, "hs_malloc failed\r\n");
    goto cmd_usbhost_fail;
  }

  if((outbuf = hs_malloc(USBH_BLOCK_SIZE * USBH_BURST_SIZE * sizeof(uint8_t), __MT_DMA)) == 0) {
    chprintf(chp, "hs_malloc failed\r\n");
    goto cmd_usbhost_fail;
  }

  if((inbuf = hs_malloc(USBH_BLOCK_SIZE * USBH_BURST_SIZE * sizeof(uint8_t), __MT_DMA)) == 0) {
    chprintf(chp, "hs_malloc failed\r\n");
    goto cmd_usbhost_fail;
  }

  usb_h_start(usbhp);

  while(usbhp->host_state!= USB_H_READY)
  {
    chThdSleepMilliseconds(1000);
    if (usbhp->host_state == USB_H_UNKNOWNDEV) {
      chprintf(chp, "Fail\r\n");
      goto cmd_usbhost_fail;
    }
  }
  chprintf(chp, "OK\r\n");

  if(!usb_h_readdisk(usbhp, 0, 1, inbuf))
  {
    chprintf(chp, "Block read failed\r\n");
    goto cmd_usbhost_fail;
  }

  chprintf(chp, "block size = %04x, num of block = %d\r\n", usbhp->blkinfo.blk_size, usbhp->blkinfo.blk_cnt);

  //raw data rw speed test
  chprintf(chp, "Block RW speed test:\r\n");
  t1 = chSysGetRealtimeCounterX();
  for(i=0;i<BLOCK_RW_NUM;i++)
  {
    if(!usb_h_readdisk(usbhp, USBH_BURST_SIZE*i, USBH_BURST_SIZE, inbuf))
    {
      chprintf(chp, " Failed\r\n");
      break;
    }
  }
  t2 = chSysGetRealtimeCounterX();
  if(t1 != t2){
    speed1 = i*USBH_BURST_SIZE*USBH_BLOCK_SIZE*CH_CFG_ST_FREQUENCY/(t2-t1);
  }
  else{
    speed1 = 0;
  }
  chprintf(chp, "Block Read speed = %d Byte/s\r\n", speed1);
//goto cmd_usbhost_fail;
  t3 = chSysGetRealtimeCounterX();
  for(i=0;i<BLOCK_RW_NUM;i++)
  {
    if(!usb_h_readdisk(usbhp, USBH_BURST_SIZE*i, USBH_BURST_SIZE, inbuf))
    {
      chprintf(chp, " Failed\r\n");
      break;
    }

    if(!usb_h_writedisk(usbhp, USBH_BURST_SIZE*i, USBH_BURST_SIZE, inbuf))
    {
      chprintf(chp, " Failed\r\n");
      break;
    }

    //chThdSleepMilliseconds(10);
  }
  t4 = chSysGetRealtimeCounterX();
  if(t2 + t3 != t1 + t4){
    speed2 = i*USBH_BURST_SIZE*USBH_BLOCK_SIZE*CH_CFG_ST_FREQUENCY/(t4+t1-t2-t3);
  }
  else{
    speed2 = 0;
  }
  chprintf(chp, "Block write speed = %d Byte/s\r\n", speed2);

  /**
   * Now perform some FS tests.
   */

  chprintf(chp, "Working area for filesystem... ");
  chThdSleepMilliseconds(100);

  err = f_mount(usbh_fs, USB_DRIVER_NUMBER, 0);
  f_chdrive(USB_DRIVER_NUMBER);
  if (err != FR_OK) {
    chprintf(chp, " Failed\r\n");
    goto cmd_usbhost_fail;
  }
  else {
    chprintf(chp, "OK\r\n");
  }

  chprintf(chp, "Mount filesystem... ");
  chThdSleepMilliseconds(100);
  err = f_getfree((const TCHAR *)_T("/"), ((DWORD *)&clusters), &fsp);
  if (err != FR_OK) {
    chprintf(chp, " Failed\r\n");
    goto cmd_usbhost_fail;
  }
  chprintf(chp, "OK\r\n");
  chprintf(chp,
      "FS: %lu free clusters, %lu sectors per cluster, %lu bytes free\r\n",
      clusters, (uint32_t)usbh_fs->csize,
      clusters * (uint32_t)usbh_fs->csize * USBH_BLOCK_SIZE);

  chprintf(chp, "Create file \"chtest.txt\"... ");
  chThdSleepMilliseconds(100);
  err = f_open(fileObject, (const TCHAR *)_T("1:chtest.txt"), FA_WRITE | FA_OPEN_ALWAYS);
  if (err != FR_OK) {
    chprintf(chp, " Failed\r\n");
    goto cmd_usbhost_fail;
  }
  chprintf(chp, "OK\r\n");
  chprintf(chp, "Write some data in it... ");
  chThdSleepMilliseconds(100);
  err = f_write(fileObject, teststring, sizeof(teststring), (void *)&bytes_written);
  if (err != FR_OK) {
    chprintf(chp, " Failed\r\n");
    f_close(fileObject);
    goto cmd_usbhost_fail;
  }
  else {
    chprintf(chp, "OK\r\n");
  }

  chprintf(chp, "Close file \"chtest.txt\"... ");
  err = f_close(fileObject);
  if (err != FR_OK) {
    chprintf(chp, " Failed\r\n");
    goto cmd_usbhost_fail;
  }
  else {
    chprintf(chp, "OK\r\n");
  }

  chprintf(chp, "Check file size \"1:chtest.txt\"... ");
  memset(&filinfo, 0, sizeof(FILINFO));
  err = f_stat((const TCHAR *)_T("1:chtest.txt"), &filinfo);
  chThdSleepMilliseconds(100);
  if (err != FR_OK) {
    chprintf(chp, " Failed\r\n");
    goto cmd_usbhost_fail;
  }
  else {
    if (filinfo.fsize == sizeof(teststring))
    chprintf(chp, "OK\r\n");
    else {
      chprintf(chp, " Failed\r\n");
      goto cmd_usbhost_fail;
    }
  }

  chprintf(chp, "Check file content \"1:chtest.txt\"... ");
  err = f_open(fileObject, (const TCHAR *)_T("1:chtest.txt"), FA_READ | FA_OPEN_EXISTING);
  chThdSleepMilliseconds(100);
  if (err != FR_OK) {
    chprintf(chp, " Failed\r\n");
    goto cmd_usbhost_fail;
  }

  err = f_read(fileObject, buf, sizeof(teststring), (void *)&bytes_read);
  if (err != FR_OK) {
    chprintf(chp, " Failed\r\n");
    f_close(fileObject);
    goto cmd_usbhost_fail;
  }
  else {
    if (memcmp(teststring, buf, sizeof(teststring)) != 0) {
      chprintf(chp, " Failed\r\n");
      goto cmd_usbhost_fail;
    }
    else {
      chprintf(chp, "OK\r\n");
    }
  }
  err = f_close(fileObject);
  if (err != FR_OK) {
    chprintf(chp, " Failed\r\n");
    goto cmd_usbhost_fail;
  }

  //file rw speed test
  chprintf(chp, "file RW speed test:\r\n");
  err = f_open(fileObject, (const TCHAR *)_T("1:rwspeed.txt"), FA_WRITE | FA_OPEN_ALWAYS);
  if (err != FR_OK)
  {
    chprintf(chp, " Failed\r\n");
    goto cmd_usbhost_fail;
  }
  memset(inbuf, 0xAA, USBH_BLOCK_SIZE * USBH_BURST_SIZE);
  t1 = chSysGetRealtimeCounterX();
  for(i=0;i<BLOCK_RW_NUM;i++)
  {
    err = f_write(fileObject, inbuf, sizeof(inbuf), (void *)&bytes_written);
    if (err != FR_OK) {
      chprintf(chp, "Failed\r\n");
      break;
    }
  }
  t2 = chSysGetRealtimeCounterX();
  if(t2 != t1){
    speed1 = i*USBH_BURST_SIZE*USBH_BLOCK_SIZE*CH_CFG_ST_FREQUENCY/(t2-t1);
  }
  else{
    speed1 = 0;
  }
  chprintf(chp, "file write speed = %d Byte/s\r\n", speed1);
  err = f_close(fileObject);
  if (err != FR_OK) {
    chprintf(chp, " Failed\r\n");
    goto cmd_usbhost_fail;
  }

  err = f_open(fileObject, (const TCHAR *)_T("1:rwspeed.txt"), FA_READ | FA_OPEN_EXISTING);
  chThdSleepMilliseconds(100);
  if (err != FR_OK) {
    chprintf(chp, " Failed\r\n");
    goto cmd_usbhost_fail;
  }

  t3 = chSysGetRealtimeCounterX();
  for(i=0;i<BLOCK_RW_NUM;i++)
  {
    err = f_read(fileObject, outbuf, sizeof(outbuf), (void *)&bytes_read);
    if (err != FR_OK) {
      chprintf(chp, " Failed\r\n");
      break;
    }
  }
  t4 = chSysGetRealtimeCounterX();
  if(t4 != t3){
    speed2 = i*USBH_BURST_SIZE*USBH_BLOCK_SIZE*CH_CFG_ST_FREQUENCY/(t4-t3);
  }
  else{
    speed2 = 0;
  }
  chprintf(chp, "file read speed = %d Byte/s\r\n", speed2);
  err = f_close(fileObject);
  if (err != FR_OK) {
    chprintf(chp, " Failed\r\n");
    goto cmd_usbhost_fail;
  }

  if(USBH_LARGE_FILE_RW_TEST)
  {
    /* when len=40000, the file size is 156MB */
    uint32_t len = 400; //clusters * (uint32_t)SDC_FS.csize / 4 ;
    uint32_t count = 400;//clusters * (uint32_t)SDC_FS.csize / 4 ;

    chprintf(chp, "large file read & write test...\r\n");
    memset(inbuf, 0xAA, USBH_BLOCK_SIZE * USBH_BURST_SIZE);
    chprintf(chp, "Create file \"large_test.txt\"... ");
    chThdSleepMilliseconds(100);
    err = f_open(fileObject, (const TCHAR *)_T("1:large_test.txt"), FA_WRITE | FA_OPEN_ALWAYS);
    if (err != FR_OK) {
      chprintf(chp, " Failed\r\n");
      goto cmd_usbhost_fail;
    }
    chprintf(chp, "OK\r\n");
    chprintf(chp, "Write data in ... ");
    do {
      chThdSleepMilliseconds(100);
      err = f_write(fileObject, inbuf, sizeof(inbuf), (void *)&bytes_written);
      if (err != FR_OK) {
        chprintf(chp, "Failed\r\n");
        f_close(fileObject);
        goto cmd_usbhost_fail;
      }
      chprintf(chp, "W");
    }while(len--);
    chprintf(chp, "\r\nOK\r\n");

    chprintf(chp, "Close file \"1:large_test.txt\"... ");
    err = f_close(fileObject);
    if (err != FR_OK) {
      chprintf(chp, " Failed\r\n");
      goto cmd_usbhost_fail;
    }
    else {
      chprintf(chp, "OK\r\n");
    }

    chprintf(chp, "Read data Out ... ");
    err = f_open(fileObject, (const TCHAR *)_T("1:large_test.txt"), FA_READ | FA_OPEN_EXISTING);
    chThdSleepMilliseconds(100);
    if (err != FR_OK) {
      chprintf(chp, " Failed\r\n");
      goto cmd_usbhost_fail;
    }
    do {
      memset(outbuf, 0, USBH_BLOCK_SIZE * USBH_BURST_SIZE * sizeof(uint8_t));
      err = f_read(fileObject, outbuf, sizeof(outbuf), (void *)&bytes_read);
      if (err != FR_OK) {
        chprintf(chp, " Failed\r\n");
        f_close(fileObject);
        goto cmd_usbhost_fail;
      }
      else {
        if (memcmp(inbuf, outbuf, USBH_BLOCK_SIZE * USBH_BURST_SIZE * sizeof(uint8_t)) != 0) {
          chprintf(chp, " Failed\r\n");
          f_close(fileObject);
          goto cmd_usbhost_fail;
        }
      }
      count--;
      chprintf(chp, "R");
    }while(count);
    chprintf(chp, "\r\nOK\r\n");

    chprintf(chp, "Close file \"large_test.txt\"... ");
    err = f_close(fileObject);
    if (err != FR_OK) {
      chprintf(chp, " Failed\r\n");
      goto cmd_usbhost_fail;
    }
    else {
      chprintf(chp, "OK\r\n");
    }
  }

  chprintf(chp, "Umount filesystem... ");
  f_mount(NULL, USB_DRIVER_NUMBER, 0);
  chprintf(chp, "OK\r\n");

  chprintf(chp, "disconnect USB host...");
  chThdSleepMilliseconds(100);


  chprintf(chp, " OK\r\n");
  chprintf(chp, "------------------------------------------------------\r\n");
  chprintf(chp, "All tests passed successfully.\r\n");

  cmd_usbhost_fail:
  usb_h_stop(usbhp);
  if(usbh_fs != NULL) {
    hs_free(usbh_fs);
  }

  if(outbuf != NULL) {
    hs_free(outbuf);
  }

  if(inbuf != NULL) {
    hs_free(inbuf);
  }

  if(fileObject != NULL){
    hs_free(fileObject);
  }
#else
  (void)chp;
  (void)argc;
  (void)argv;
#endif
}


