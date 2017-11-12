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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "lib.h"

#if HS_USE_MP3

typedef struct{
  BaseSequentialStream *stream;
  char *filename;
  int sample_rate;
  int i2s_mode;

  FATFS *fatfs;
}audio_thead_param;

static THD_FUNCTION(Mp3TestThread, arg)
{
  hs_adectype_t type = ADEC_TYPE_UNKNOWN;
  hs_adec_t *pstAdec;
  hs_ao_t   *pstAo;
  FIL *fp;
  
  chRegSetThreadName("Mp3Test");

  audio_thead_param *param = (audio_thead_param *)arg;
  audioSetCodecSel(AUDIO_EXTERN_CODEC);
  audioStart();

  chprintf(param->stream, "start...\r\n");

  TCHAR *filename = (TCHAR *)hs_malloc((strlen(param->filename) + 1) * sizeof(TCHAR), __MT_GENERAL);
  mbstowcs(filename, param->filename, (strlen(param->filename) + 1));

  if(strstr(param->filename, ".mp3") != NULL){
    type = ADEC_TYPE_MP3;
  }
  else if(strstr(param->filename, ".wav") != NULL){
    type = ADEC_TYPE_WAV;
  }
  else if(strstr(param->filename, ".wma") != NULL){
    type = ADEC_TYPE_WMA;
  }
  else{
    chprintf(param->stream, "unknown format\r\n");
  }  

  if ((fp = (FIL *) hs_malloc(sizeof(FIL), __MT_DMA)) == NULL) {
    chprintf(param->stream, "hs_malloc failed!\r\n");
    return;
  }

  if (f_open(fp, filename, FA_READ) != FR_OK) {
    chprintf(param->stream, "open file failed!\r\n");
    hs_free(fp);
    return;
  }

  pstAo   = hs_ao_create(PLAYER_WORKMODE_MP3);
  hs_ao_start(pstAo);
  pstAdec = hs_adec_creat(type, fp, pstAo, 0);

  hs_adec_skip(pstAdec); 
  while(-1 != hs_adec_run(pstAdec));

  hs_adec_destroy(pstAdec);
  hs_ao_destroy(pstAo);

  chprintf(param->stream, "over\r\n");
}

#endif

void cmd_avcodec(BaseSequentialStream *chp, int argc, char *argv[])
{
  #if HS_USE_MP3
  if(argc < 1){
    chprintf(chp, "input file(*.mp3;*.wav;*.wma) is null\r\n");
    return ;
  }

  // i2c
  palSetPadMode(IOPORT0, 5, PAL_MODE_OUTPUT| PAL_MODE_ALTERNATE(PAD_FUNC_I2C_SCK)|PAL_MODE_DRIVE_CAP(3));
  palSetPadMode(IOPORT0, 9, PAL_MODE_INPUT | PAL_MODE_ALTERNATE(PAD_FUNC_I2C_SDA)|PAL_MODE_DRIVE_CAP(3));

  //i2s
  palSetPadMode(IOPORT1, 2, PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_I2S_WS_RX)|PAL_MODE_DRIVE_CAP(3));
  palSetPadMode(IOPORT1, 3, PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_I2S_SDI)|PAL_MODE_DRIVE_CAP(3));
  palSetPadMode(IOPORT1, 4, PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_I2S_SDO)|PAL_MODE_DRIVE_CAP(3));
  palSetPadMode(IOPORT1, 5, PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_I2S_WS_TX)|PAL_MODE_DRIVE_CAP(3));
  palSetPadMode(IOPORT1, 6, PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_I2S_SCK_TX)|PAL_MODE_DRIVE_CAP(3));

  /* sd */
  palSetPadMode(IOPORT0, 6, PAL_MODE_INPUT_PULLUP | PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB) | PAL_MODE_DRIVE_CAP(3));
  palSetPadMode(IOPORT0, 8, PAL_MODE_INPUT_PULLUP | PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB) | PAL_MODE_DRIVE_CAP(3));
  palSetPadMode(IOPORT0, 10, PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB) | PAL_MODE_DRIVE_CAP(3));
  palSetPadMode(IOPORT0, 11, PAL_MODE_INPUT_PULLUP | PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB) | PAL_MODE_DRIVE_CAP(3));
  palSetPadMode(IOPORT0, 12, PAL_MODE_INPUT_PULLUP | PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB) | PAL_MODE_DRIVE_CAP(3));
  palSetPadMode(IOPORT0, 13, PAL_MODE_INPUT_PULLUP | PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB) | PAL_MODE_DRIVE_CAP(3));


  chThdSleepMilliseconds(100);

  if(!hs_fatfs_isMount(FATFS_MEMDEV_SD)) {
    chprintf(chp, "fatfs is not mount!\r\n\r\n");
    return ;
  }

  static audio_thead_param play_param;
  play_param.stream = chp;
  play_param.filename = argv[0];
  chThdCreateFromHeap(NULL, 4096, NORMALPRIO + 1, Mp3TestThread, &play_param);
  #else
  (void)chp;
  (void)argc;
  (void)argv;
  #endif
}

