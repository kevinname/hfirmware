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
#include <stdlib.h> //strtol
#include <wchar.h>
#include "ch.h"
#include "lib.h"
#include "lib_mem.h"

#if HAL_USE_AUDIO

#define TEST_THREAD_STACK_SIZE 2048
static volatile int isStopLoop;
static volatile int isStopRec;

typedef struct{
  BaseSequentialStream *stream;
  TCHAR *filename;
  int sample_rate;
  int i2s_mode;
}audio_thead_param;

#if HS_CODEC_USE_INSIDE
static hs_audio_config_t g_recI2sCfg =
{
  I2S_SAMPLE_44P1K,
  I2S_BITWIDTH_16BIT,
  I2S_BITWIDTH_32BIT,
  I2S_WORKMODE_MASTER,
  I2S_PCMMODE_STEREO,

  I2S_REC_BLOCK_SIZE,
  I2S_REC_BLOCK_NUM,
};
#endif

static hs_audio_config_t g_plyI2sCfg =
{
  I2S_SAMPLE_44P1K,
  I2S_BITWIDTH_16BIT,
  I2S_BITWIDTH_32BIT,
  I2S_WORKMODE_MASTER,
  I2S_PCMMODE_STEREO,

  I2S_PLY_BLOCK_SIZE,
  I2S_PLY_BLOCK_NUM,
};

static hs_audio_config_t g_plyI2sCfg_24B =
{
  I2S_SAMPLE_48K,
  I2S_BITWIDTH_24BIT,
  I2S_BITWIDTH_32BIT,
  I2S_WORKMODE_MASTER,
  I2S_PCMMODE_STEREO,

  I2S_PLY_BLOCK_SIZE,
  I2S_PLY_BLOCK_NUM,
};

static hs_audio_config_t g_recordI2sCfg_24B =
{
  I2S_SAMPLE_8K,
  I2S_BITWIDTH_24BIT,
  I2S_BITWIDTH_32BIT,
  I2S_WORKMODE_MASTER,
  I2S_PCMMODE_STEREO,

  I2S_REC_BLOCK_SIZE,
  I2S_REC_BLOCK_NUM,
};

static void audioRecThread(void * arg)
{
  audio_thead_param *param = (audio_thead_param *)arg;
  BaseSequentialStream *chp = param->stream;
  TCHAR *filename = param->filename;
  FIL *fp;
  wave_header header;

  uint8_t *pData;
  uint32_t size;
  systime_t timeout = S2ST(1);
  char tmp[64];
  wcstombs(tmp, filename, sizeof(tmp) - 1);
  tmp[sizeof(tmp) - 1] = 0;

  if ((fp = (FIL *) hs_malloc(sizeof(FIL), __MT_DMA)) == NULL) {
    chprintf(chp, "hs_malloc failed!\r\n");
    return;
  }

  FRESULT err;
  if((err = f_open(fp, filename, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK){
    chprintf(chp, "open %s failed(error code %d)!\r\n", tmp, err);
    goto exit;
  }

  memcpy(header.riff_sig, "RIFF", 4);
  memcpy(header.wave_sig, "WAVE", 4);
  memcpy(header.format_sig, "fmt ", 4);
  memcpy(header.data_sig, "data", 4);

  header.format_chunk_size = 0x00000010;
  header.format_tag = 0x01;

  if(param->i2s_mode == I2S_PCMMODE_MONO)
    header.channels = 1;
  else
    header.channels = 2;

  header.sample_rate = param->sample_rate;
  header.bits_per_sample = 16;
  header.bytes_per_sec = header.sample_rate * header.bits_per_sample * header.channels / 8;
  header.block_align = header.channels * header.bits_per_sample / 8;

  size = sizeof(header);
  if(f_write(fp, &header, size, (UINT *)&size) != FR_OK){
    chprintf(chp, "f_write %s failed!\r\n", tmp);
    goto exit;
  }

  int length = 0;
  while(!isStopRec){
    size = 120 * 4; //MMCSD_BLOCK_SIZE ;
    if((size = audioRecGetDataBuffer(&pData, size, timeout)) == 0){
      chprintf(chp, "audioRecGetDataBuffer failed!\r\n");
      break;
    }

    if(f_write(fp, pData, size, (UINT *)&size) != FR_OK){
      chprintf(chp, "f_write %s failed!\r\n", tmp);
      break ;
    }

    audioRecGetDataDone(pData, size);
    length += size;
  }

  f_lseek(fp, 0);
  header.waveform_chunk_size = length + 0x2c - 8;
  header.data_size = length;

  size = sizeof(header);
  if(f_write(fp, &header, size, (UINT *)&size) != FR_OK){
    chprintf(chp, "f_write %s failed!\r\n", tmp);
  }

exit:
  audioRecordStop();
  f_sync(fp);
  f_close(fp);
  hs_free(fp);
  return;
}

static void audioPlyThread(void * arg)
{
  audio_thead_param *param = (audio_thead_param *)arg;
  BaseSequentialStream *chp = param->stream;
  TCHAR *filename = param->filename;

  FIL *fp;
  uint8_t *pData;
  uint32_t size;
  wave_header header;

  systime_t timeout = S2ST(-1);
  char tmp[64];
  wcstombs(tmp, filename, sizeof(tmp) - 1);
  tmp[sizeof(tmp) - 1] = 0;

  if ((fp = (FIL *) hs_malloc(sizeof(FIL), __MT_DMA)) == NULL) {
    chprintf(chp, "hs_malloc failed!\r\n");
    return;
  }

  FRESULT err;
  if((err = f_open(fp, filename, FA_READ)) != FR_OK){
    chprintf(chp, "open %s failed(error code %d)!\r\n", tmp, err);
    goto exit;
  }

  size = sizeof(header);
  if(f_read(fp, &header, size, (UINT *)&size) != FR_OK){
    chprintf(chp, " read header%s failed!\r\n", tmp);
    goto exit;
  }

  int data_length = fp->fsize - sizeof(wave_header);
  while(data_length > 0){
    size = 2400;
    if((size = audioPlyGetDataBuffer(&pData, size, timeout)) == 0){
      chprintf(chp, "audioPlyGetDataBuffer failed!\r\n");
      break;
    }

    if(f_read(fp, pData, size, (UINT *)&size) != FR_OK){
      chprintf(chp, " read data %s failed!\r\n", tmp);
      break ;
    }

    audioPlySendDataDone(pData, size);
    data_length -= size;
  }

exit:
  audioPlayStop();
  f_close(fp);
  hs_free(fp);
  return;
}

void audio_play_sim()
{
  unsigned char simData[] = { 0xA4, 0xD2, 0xa4, 0xD2, 0xDA, 0xBF, 0xDA, 0xBF, 0xa4, 0xd2, 0xa4, 0xd2,
    0x00, 0x00, 0x00, 0x00,  0x5c, 0x2d, 0x5c, 0x2d, 0x27, 0x40, 0x27, 0x40, 0x5d, 0x2d, 0x5d, 0x2d,
    0x00, 0x00, 0x00, 0x00
  };

  uint8_t *pSrc, *pDst;
  uint32_t size;
  systime_t timeout = S2ST(1);

  pSrc = simData;
  int i = 0;
  while(i++ < 100000){
    pSrc = simData;
    size = MMCSD_BLOCK_SIZE * 4;
    if((size = audioPlyGetDataBuffer(&pDst, size, timeout)) == 0){

      break;
    }

    int j = 0;
    while((j + sizeof(simData)) < size){
      memcpy(pDst + j, pSrc, sizeof(simData));
      j += sizeof(simData);
    }

    audioPlySendDataDone(pDst, size);
  }

  chprintf((BaseSequentialStream *)&SD1, "player stop\r\n");
}

#if HS_CODEC_USE_INSIDE

void audio_sim_test(BaseSequentialStream *chp)
{
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);

  audioSetRecordSample(I2S_SAMPLE_8K);
  audioSetPlaySample(I2S_SAMPLE_8K);
  audioPlaySetVolume(0);

  audio_play_sim();

  chprintf(chp, " finish\r\n");
}

static void d2a_8k_audio(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_8k_audio_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_8k_audio.wav");
  rec_param.sample_rate = I2S_SAMPLE_8K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:test.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_8K);
  audioSetPlaySample(I2S_SAMPLE_8K);

  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);

  audioPlaySetVolume(0);
  //chThdSleepMilliseconds(10000);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void d2a_16k_audio(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_16k_audio_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_16k_audio.wav");
  rec_param.sample_rate = I2S_SAMPLE_44P1K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:test.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_16K);
  audioSetPlaySample(I2S_SAMPLE_16K);

  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);

  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void d2a_32k_audio(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_32k_audio_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_32k_audio.wav");
  rec_param.sample_rate = I2S_SAMPLE_44P1K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:test.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_32K);
  audioSetPlaySample(I2S_SAMPLE_32K);

  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void d2a_8k_audio_short(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_8k_audio_short_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_8k_audio_short.wav");
  rec_param.sample_rate = I2S_SAMPLE_44P1K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:test.wav");

  isStopRec = 0;
  audioSetShortFir(1);
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_8K);
  audioSetPlaySample(I2S_SAMPLE_8K);

  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);
  audioSetShortFir(0);

  chprintf(chp, " finish\r\n");
}

static void d2a_16k_audio_short(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_16k_audio_short_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_16k_audio_short.wav");
  rec_param.sample_rate = I2S_SAMPLE_44P1K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:test.wav");

  isStopRec = 0;
  audioSetShortFir(1);
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_16K);
  audioSetPlaySample(I2S_SAMPLE_16K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);
  audioSetShortFir(0);

  chprintf(chp, " finish\r\n");
}

static void d2a_32k_audio_short(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_32k_short_audio_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_32k_audio_short.wav");
  rec_param.sample_rate = I2S_SAMPLE_44P1K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:test.wav");

  isStopRec = 0;
  audioSetShortFir(1);
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_32K);
  audioSetPlaySample(I2S_SAMPLE_32K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);
  audioSetShortFir(0);

  chprintf(chp, " finish\r\n");
}

static void d2a_12k_audio(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_12k_audio_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_12k_audio.wav");
  rec_param.sample_rate = I2S_SAMPLE_44P1K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:test.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_12K);
  audioSetPlaySample(I2S_SAMPLE_12K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void d2a_24k_audio(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_24k_audio_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_24k_audio.wav");
  rec_param.sample_rate = I2S_SAMPLE_44P1K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:test.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_24K);
  audioSetPlaySample(I2S_SAMPLE_24K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void d2a_48k_audio(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_48k_audio_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_48k_audio.wav");
  rec_param.sample_rate = I2S_SAMPLE_44P1K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:test.wav");
  play_param.sample_rate = I2S_SAMPLE_44P1K;

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_48K);
  audioSetPlaySample(I2S_SAMPLE_48K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void d2a_11k_audio(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_11k_audio_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_11k_audio.wav");
  rec_param.sample_rate = I2S_SAMPLE_44P1K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:test.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_11K);
  audioSetPlaySample(I2S_SAMPLE_11K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void d2a_22k_audio(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_22k_audio_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_22k_audio.wav");
  rec_param.sample_rate = I2S_SAMPLE_44P1K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:test.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_22K);
  audioSetPlaySample(I2S_SAMPLE_22K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void d2a_44k_audio(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_44k_audio_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_44k_audio.wav");
  rec_param.sample_rate = I2S_SAMPLE_44P1K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:test.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_44P1K);
  audioSetPlaySample(I2S_SAMPLE_44P1K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void d2a_96k_audio(BaseSequentialStream *chp)
{
  audio_thead_param play_param;

  chprintf(chp, "d2a_44k_audio_test ...");

  play_param.stream = chp;
  play_param.filename = _T("0:test.wav");

  isStopRec = 0;
  audioSetPlaySource(AUDIO_PLAY_RAM);

  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetPlaySample(I2S_SAMPLE_96K);

  audioPlyThread(&play_param);

  chprintf(chp, " finish\r\n");
}

static void d2a_8k_snr(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_8k_snr_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_8k_snr.wav");
  rec_param.sample_rate = I2S_SAMPLE_8K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:8k_snr.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_8K);
  audioSetPlaySample(I2S_SAMPLE_8K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void d2a_16k_snr(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_16k_snr_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_16k_snr.wav");
  rec_param.sample_rate = I2S_SAMPLE_16K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:16k_snr.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_16K);
  audioSetPlaySample(I2S_SAMPLE_16K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void d2a_32k_snr(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_32k_snr_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_32k_snr.wav");
  rec_param.sample_rate = I2S_SAMPLE_32K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:32k_snr.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_32K);
  audioSetPlaySample(I2S_SAMPLE_32K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void d2a_8k_snr_short(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_8k_snr_short_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_8k_snr_short.wav");
  rec_param.sample_rate = I2S_SAMPLE_8K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:8k_snr.wav");

  isStopRec = 0;
  audioSetShortFir(1);
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_8K);
  audioSetPlaySample(I2S_SAMPLE_8K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);
  audioSetShortFir(0);

  chprintf(chp, " finish\r\n");
}

static void d2a_16k_snr_short(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_16k_snr_short_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_16k_snr_short.wav");
  rec_param.sample_rate = I2S_SAMPLE_16K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:16k_snr.wav");

  isStopRec = 0;
  audioSetShortFir(1);
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_16K);
  audioSetPlaySample(I2S_SAMPLE_16K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);
  audioSetShortFir(0);

  chprintf(chp, " finish\r\n");
}

static void d2a_32k_snr_short(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_32k_snr_short_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_32k_snr_short.wav");
  rec_param.sample_rate = I2S_SAMPLE_32K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:32k_snr.wav");

  isStopRec = 0;
  audioSetShortFir(1);
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_32K);
  audioSetPlaySample(I2S_SAMPLE_32K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);
  audioSetShortFir(0);

  chprintf(chp, " finish\r\n");
}

static void d2a_12k_snr(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_12k_snr_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_12k_snr.wav");
  rec_param.sample_rate = I2S_SAMPLE_12K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:12k_snr.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_12K);
  audioSetPlaySample(I2S_SAMPLE_12K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void d2a_24k_snr(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_24k_snr_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_24k_snr.wav");
  rec_param.sample_rate = I2S_SAMPLE_24K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:24k_snr.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_24K);
  audioSetPlaySample(I2S_SAMPLE_24K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void d2a_48k_snr(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_48k_snr_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_48k_snr.wav");
  rec_param.sample_rate = I2S_SAMPLE_48K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:48k_snr.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_48K);
  audioSetPlaySample(I2S_SAMPLE_48K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void d2a_11k_snr(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_11k_snr_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_11k_snr.wav");
  rec_param.sample_rate = I2S_SAMPLE_11K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:11k_snr.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_11K);
  audioSetPlaySample(I2S_SAMPLE_11K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void d2a_22k_snr(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_22k_snr_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_22k_snr.wav");
  rec_param.sample_rate = I2S_SAMPLE_22K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:22k_snr.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_22K);
  audioSetPlaySample(I2S_SAMPLE_22K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void d2a_44k_snr(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "d2a_44k_snr_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_44k_snr.wav");
  rec_param.sample_rate = I2S_SAMPLE_44P1K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:44k_snr.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_44P1K);
  audioSetPlaySample(I2S_SAMPLE_44P1K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void dmic_8k_audio(BaseSequentialStream *chp)
{
  audio_thead_param param;
  thread_t *tp;

  chprintf(chp, "dmic_8k_audio_test ...");

  param.stream = chp;
  param.filename = _T("0:dmic_8k_audio.wav");
  param.sample_rate = I2S_SAMPLE_8K;

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_DMIC);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioSetRecordSample(I2S_SAMPLE_8K);

  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &param);
  chThdSleepMilliseconds(10000);

  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void dmic_16k_audio(BaseSequentialStream *chp)
{
  audio_thead_param param;
  thread_t *tp;

  chprintf(chp, "dmic_16k_audio_test ...");

  param.stream = chp;
  param.filename = _T("0:dmic_16k_audio.wav");
  param.sample_rate = I2S_SAMPLE_16K;

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_DMIC);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioSetRecordSample(I2S_SAMPLE_16K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &param);
  chThdSleepMilliseconds(10000);

  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void dmic_32k_audio(BaseSequentialStream *chp)
{
  audio_thead_param param;
  thread_t *tp;

  chprintf(chp, "dmic_32k_audio_test ...");

  param.stream = chp;
  param.filename = _T("0:dmic_32k_audio.wav");
  param.sample_rate = I2S_SAMPLE_32K;

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_DMIC);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioSetRecordSample(I2S_SAMPLE_32K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &param);

  chThdSleepMilliseconds(10000);

  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void dmic_12k_audio(BaseSequentialStream *chp)
{
  audio_thead_param param;
  thread_t *tp;

  chprintf(chp, "dmic_12k_audio_test ...");

  param.stream = chp;
  param.filename = _T("0:dmic_12k_audio.wav");
  param.sample_rate = I2S_SAMPLE_12K;

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_DMIC);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioSetRecordSample(I2S_SAMPLE_12K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &param);
  chThdSleepMilliseconds(10000);

  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void dmic_24k_audio(BaseSequentialStream *chp)
{
  audio_thead_param param;
  thread_t *tp;

  chprintf(chp, "dmic_24k_audio_test ...");

  param.stream = chp;
  param.filename = _T("0:dmic_24k_audio.wav");
  param.sample_rate = I2S_SAMPLE_24K;

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_DMIC);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioSetRecordSample(I2S_SAMPLE_24K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &param);
  chThdSleepMilliseconds(10000);

  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void dmic_48k_audio(BaseSequentialStream *chp)
{
  audio_thead_param param;
  thread_t *tp;

  chprintf(chp, "dmic_48k_audio_test ...");

  param.stream = chp;
  param.filename = _T("0:dmic_48k_audio.wav");
  param.sample_rate = I2S_SAMPLE_48K;

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_DMIC);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioSetRecordSample(I2S_SAMPLE_48K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &param);
  chThdSleepMilliseconds(10000);

  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void dmic_11k_audio(BaseSequentialStream *chp)
{
  audio_thead_param param;
  thread_t *tp;

  chprintf(chp, "dmic_11k_audio_test ...");

  param.stream = chp;
  param.filename = _T("0:dmic_11k_audio.wav");
  param.sample_rate = I2S_SAMPLE_11K;

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_DMIC);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioSetRecordSample(I2S_SAMPLE_11K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &param);
  chThdSleepMilliseconds(10000);

  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void dmic_22k_audio(BaseSequentialStream *chp)
{
  audio_thead_param param;
  thread_t *tp;

  chprintf(chp, "dmic_22k_audio_test ...");

  param.stream = chp;
  param.filename = _T("0:dmic_22k_audio.wav");
  param.sample_rate = I2S_SAMPLE_22K;

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_DMIC);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioSetRecordSample(I2S_SAMPLE_22K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &param);
  chThdSleepMilliseconds(10000);

  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void dmic_44k_audio(BaseSequentialStream *chp)
{
  audio_thead_param param;
  thread_t *tp;

  chprintf(chp, "dmic_44k_audio_test ...");

  param.stream = chp;
  param.filename = _T("0:dmic_44k_audio.wav");
  param.sample_rate = I2S_SAMPLE_44P1K;

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_DMIC);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioSetRecordSample(I2S_SAMPLE_44P1K);
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &param);
  chThdSleepMilliseconds(1000);

  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

static void d2a_dac_mute_unmute(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp_rec, *tp_ply;

  chprintf(chp, "d2a_dac_mute_unmute_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_dac_mute_unmute.wav");
  rec_param.sample_rate = I2S_SAMPLE_8K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:test.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_8K);
  audioSetPlaySample(I2S_SAMPLE_8K);
  tp_rec = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  tp_ply = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioPlyThread, &play_param);

  chThdSleepMilliseconds(4000);
  audioPlayMute();
  chThdSleepMilliseconds(4000);
  audioPlayUnmute();
  chThdSleepMilliseconds(4000);
  chThdWait(tp_ply);
  isStopRec = 1;
  chThdWait(tp_rec);

  chprintf(chp, " finish\r\n");
}

static void d2a_adc_mute_unmute(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp_rec, *tp_ply;

  chprintf(chp, "d2a_adc_mute_unmute_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_adc_mute_unmute.wav");
  rec_param.sample_rate = I2S_SAMPLE_8K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:test.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_8K);
  audioSetPlaySample(I2S_SAMPLE_8K);
  tp_rec = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  tp_ply = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioPlyThread, &play_param);

  chThdSleepMilliseconds(4000);
  audioRecordMute();
  chThdSleepMilliseconds(4000);
  audioRecordUnmute();
  chThdSleepMilliseconds(4000);
  chThdWait(tp_ply);
  isStopRec = 1;
  chThdWait(tp_rec);

  chprintf(chp, " finish\r\n");
}

static void d2a_dac_vol_ctrl(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp_rec, *tp_ply;
  int count;

  chprintf(chp, "d2a_dac_vol_ctrl_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_dac_vol_ctrl.wav");
  rec_param.sample_rate = I2S_SAMPLE_8K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:test.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_8K);
  audioSetPlaySample(I2S_SAMPLE_8K);
  tp_rec = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  tp_ply = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioPlyThread, &play_param);
  count = 0;
  int vol = 0;
  while(count++ < 12){
    chThdSleepMilliseconds(1000);

    if(count < 4)
      vol += 2;
    else
      vol -= 2;

    audioPlaySetVolume(vol);
  }

  isStopRec = 1;
  chThdWait(tp_rec);
  chThdWait(tp_ply);

  chprintf(chp, " finish\r\n");
}

static void d2a_adc_vol_ctrl(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp_rec, *tp_ply;
  int count;

  chprintf(chp, "d2a_adc_vol_ctrl_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_adc_vol_ctrl.wav");
  rec_param.sample_rate = I2S_SAMPLE_8K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:test.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_8K);
  audioSetPlaySample(I2S_SAMPLE_8K);
  tp_rec = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  tp_ply = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioPlyThread, &play_param);
  int vol = 0;
  count = 0;
  while(count++ < 12){
    chThdSleepMilliseconds(1000);

    if(count < 4)
      vol += 2;
    else
      vol -= 2;

    audioRecordSetVolume(vol);
  }

  isStopRec = 1;
  chThdWait(tp_rec);
  chThdWait(tp_ply);

  chprintf(chp, " finish\r\n");
}


static void d2a_adc_drc_limiter(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp_rec;

  chprintf(chp, "d2a_adc_drc_limiter_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_adc_drc_limiter.wav");
  rec_param.sample_rate = I2S_SAMPLE_8K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:limiter.wav");

  isStopRec = 0;
  audioSetAdcDrcMode(1);
  audioSetAdcDrcLimiter(-15);
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_8K);
  audioSetPlaySample(I2S_SAMPLE_8K);
  tp_rec = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);

  isStopRec = 1;
  chThdWait(tp_rec);
  audioSetAdcDrcMode(0);

  chprintf(chp, " finish\r\n");
}

static void d2a_adc_drc_agc(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp_rec;

  chprintf(chp, "d2a_adc_drc_agc_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_adc_drc_agc.wav");
  rec_param.sample_rate = I2S_SAMPLE_8K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:limiter.wav");

  isStopRec = 0;
  audioSetAdcDrcMode(1);
  audioSetAdcDrcAgc(-15);
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_8K);
  audioSetPlaySample(I2S_SAMPLE_8K);
  tp_rec = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);

  isStopRec = 1;
  chThdWait(tp_rec);
  audioSetAdcDrcMode(0);

  chprintf(chp, " finish\r\n");
}

static void d2a_dac_drc_limiter(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp_rec;

  chprintf(chp, "d2a_dac_drc_limiter_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_dac_drc_limiter.wav");
  rec_param.sample_rate = I2S_SAMPLE_8K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:limiter.wav");

  isStopRec = 0;
  audioSetDacDrcMode(1);
  audioSetDacDrcLimiter(-15);
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_8K);
  audioSetPlaySample(I2S_SAMPLE_8K);
  tp_rec = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);

  isStopRec = 1;
  chThdWait(tp_rec);
  audioSetDacDrcMode(0);

  chprintf(chp, " finish\r\n");
}

static void d2a_dac_drc_agc(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp_rec;

  chprintf(chp, "d2a_dac_drc_agc_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_dac_drc_agc.wav");
  rec_param.sample_rate = I2S_SAMPLE_8K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:limiter.wav");

  isStopRec = 0;
  audioSetDacDrcMode(1);
  audioSetDacDrcAgc(-15);
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_8K);
  audioSetPlaySample(I2S_SAMPLE_8K);
  tp_rec = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);

  isStopRec = 1;
  chThdWait(tp_rec);
  audioSetDacDrcMode(0);

  chprintf(chp, " finish\r\n");
}

static void d2a_dac_mix(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp_rec;

  chprintf(chp, "d2a_dac_mix_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_dac_mix.wav");
  rec_param.sample_rate = I2S_SAMPLE_8K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:mix.wav");

  isStopRec = 0;
  audioSetDacMix(1);
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_8K);
  audioSetPlaySample(I2S_SAMPLE_8K);
  tp_rec = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);

  isStopRec = 1;
  chThdWait(tp_rec);
  audioSetDacMix(0);

  chprintf(chp, " finish\r\n");
}

static void d2a_adc_mix(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp_rec;

  chprintf(chp, "d2a_adc_mix_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_adc_mix.wav");
  rec_param.sample_rate = I2S_SAMPLE_8K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:mix.wav");

  isStopRec = 0;
  audioSetAdcMix(1);
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_8K);
  audioSetPlaySample(I2S_SAMPLE_8K);
  tp_rec = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);

  isStopRec = 1;
  chThdWait(tp_rec);
  audioSetAdcMix(0);

  chprintf(chp, " finish\r\n");
}

static void d2a_adc_dither_s_r(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp_rec;

  chprintf(chp, "d2a_adc_dither_s_r_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_adc_dither_s_r.wav");
  rec_param.sample_rate = I2S_SAMPLE_44P1K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:dither.wav");

  isStopRec = 0;
  audioSetDacMode(0x0e);
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_44P1K);
  audioSetPlaySample(I2S_SAMPLE_44P1K);
  tp_rec = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);

  isStopRec = 1;
  chThdWait(tp_rec);
  audioSetDacMode(0x00);

  chprintf(chp, " finish\r\n");
}

static void d2a_adc_dither_s_t(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp_rec;

  chprintf(chp, "d2a_adc_dither_s_t_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_adc_dither_s_t.wav");
  rec_param.sample_rate = I2S_SAMPLE_44P1K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:dither.wav");

  isStopRec = 0;
  audioSetDacMode(0x06);
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_44P1K);
  audioSetPlaySample(I2S_SAMPLE_44P1K);
  tp_rec = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);

  isStopRec = 1;
  chThdWait(tp_rec);
  audioSetDacMode(0x00);

  chprintf(chp, " finish\r\n");
}

static void d2a_adc_dither_d_r(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp_rec;

  chprintf(chp, "d2a_adc_dither_d_r_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_adc_dither_d_r.wav");
  rec_param.sample_rate = I2S_SAMPLE_44P1K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:dither.wav");

  isStopRec = 0;
  audioSetDacMode(0x0a);
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_44P1K);
  audioSetPlaySample(I2S_SAMPLE_44P1K);
  tp_rec = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);

  isStopRec = 1;
  chThdWait(tp_rec);
  audioSetDacMode(0x00);

  chprintf(chp, " finish\r\n");
}

static void d2a_adc_dither_d_t(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp_rec;

  chprintf(chp, "d2a_adc_dither_d_t_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_adc_dither_d_t.wav");
  rec_param.sample_rate = I2S_SAMPLE_44P1K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:dither.wav");

  isStopRec = 0;
  audioSetDacMode(0x03);
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_44P1K);
  audioSetPlaySample(I2S_SAMPLE_44P1K);
  tp_rec = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);

  isStopRec = 1;
  chThdWait(tp_rec);
  audioSetDacMode(0x00);

  chprintf(chp, " finish\r\n");
}

static void d2a_left_right_swap(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp_rec;

  chprintf(chp, "d2a_left_right_swap_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_left_right_swap.wav");
  rec_param.sample_rate = I2S_SAMPLE_44P1K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:swap.wav");

  isStopRec = 0;
  audioInvertI2sInput(TRACK_RL);
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_44P1K);
  audioSetPlaySample(I2S_SAMPLE_44P1K);
  tp_rec = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);

  isStopRec = 1;
  chThdWait(tp_rec);
  audioInvertI2sInput(TRACK_LR);

  chprintf(chp, " finish\r\n");
}

static void i2s_mono_mode(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp_rec;

  chprintf(chp, "i2s_mono_mode_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:i2s_mono_mode.wav");
  rec_param.sample_rate = I2S_SAMPLE_44P1K;
  rec_param.i2s_mode = I2S_PCMMODE_MONO;

  play_param.stream = chp;
  play_param.filename = _T("0:mono.wav");

  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_8K);
  audioSetPlaySample(I2S_SAMPLE_8K);

  isStopRec = 0;
  audioSetRecMode(1);
  audioSetPlyMode(1);

  tp_rec = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);

  isStopRec = 1;
  chThdWait(tp_rec);
  audioSetRecMode(0);
  audioSetPlyMode(0);

  chprintf(chp, " finish\r\n");
}

static void d2a_dac_eq(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp_rec;

  chprintf(chp, "d2a_dac_eq_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:d2a_dac_eq_6.wav");
  rec_param.sample_rate = I2S_SAMPLE_44P1K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:eq.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioSetRecordSample(I2S_SAMPLE_32K);
  audioSetPlaySample(I2S_SAMPLE_32K);

  audioSetEqEnable(1);
  audioSetBand6Gain(12);
  audioSetBand6Coeff(0x85a8c467);
  tp_rec = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);
  audioPlyThread(&play_param);

  isStopRec = 1;
  chThdWait(tp_rec);

  audioSetBand6Gain(0);
  audioSetBand6Coeff(0);
  audioSetEqEnable(0);

  chprintf(chp, " finish\r\n");
}

#endif //end of HS_CODEC_USE_INSIDE

void cmd_codec(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  /* sd */
  palSetPadMode(IOPORT0, 6, PAL_MODE_INPUT_PULLUP | PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB) | PAL_MODE_DRIVE_CAP(3));
  palSetPadMode(IOPORT0, 8, PAL_MODE_INPUT_PULLUP | PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB) | PAL_MODE_DRIVE_CAP(3));
  palSetPadMode(IOPORT0, 10, PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB) | PAL_MODE_DRIVE_CAP(3));
  palSetPadMode(IOPORT0, 11, PAL_MODE_INPUT_PULLUP | PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB) | PAL_MODE_DRIVE_CAP(3));
  palSetPadMode(IOPORT0, 12, PAL_MODE_INPUT_PULLUP | PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB) | PAL_MODE_DRIVE_CAP(3));
  palSetPadMode(IOPORT0, 13, PAL_MODE_INPUT_PULLUP | PAL_MODE_ALTERNATE(PAD_FUNC_SD_USB) | PAL_MODE_DRIVE_CAP(3));

  if(!hs_fatfs_isMount(FATFS_MEMDEV_SD)) {
    chprintf(chp, "fatfs is not mount!\r\n\r\n");
    return ;
  }

  do{
#if HS_CODEC_USE_INSIDE
    chprintf(chp, "start test\r\n");
    audioSetCodecSel(AUDIO_INSIDE_CODEC);
    audioStart();
    audioSetTestMode(0x01);
    //d2a_8k_audio(chp);
    /*
    dmic_8k_audio(chp);
    dmic_16k_audio(chp);
    dmic_32k_audio(chp);
    dmic_12k_audio(chp);
    dmic_24k_audio(chp);
    */
    dmic_48k_audio(chp);
    /*
    dmic_11k_audio(chp);
    dmic_22k_audio(chp);
    dmic_44k_audio(chp);
    */
    break ;

    d2a_adc_dither_s_r(chp);
    d2a_adc_dither_s_t(chp);
    d2a_adc_dither_d_r(chp);
    d2a_adc_dither_d_t(chp);

    d2a_left_right_swap(chp);
    i2s_mono_mode(chp);

    dmic_8k_audio(chp);
    dmic_16k_audio(chp);
    dmic_32k_audio(chp);
    dmic_12k_audio(chp);
    dmic_24k_audio(chp);
    dmic_48k_audio(chp);
    dmic_11k_audio(chp);
    dmic_22k_audio(chp);
    dmic_44k_audio(chp);

    d2a_8k_audio(chp);
    d2a_16k_audio(chp);
    d2a_32k_audio(chp);
    d2a_8k_audio_short(chp);
    d2a_16k_audio_short(chp);
    d2a_32k_audio_short(chp);
    d2a_12k_audio(chp);
    d2a_24k_audio(chp);
    d2a_48k_audio(chp);
    d2a_11k_audio(chp);
    d2a_22k_audio(chp);
    d2a_44k_audio(chp);

    d2a_8k_snr(chp);
    d2a_16k_snr(chp);
    d2a_32k_snr(chp);
    d2a_8k_snr_short(chp);
    d2a_16k_snr_short(chp);
    d2a_32k_snr_short(chp);
    d2a_12k_snr(chp);
    d2a_24k_snr(chp);
    d2a_48k_snr(chp);
    d2a_11k_snr(chp);
    d2a_22k_snr(chp);
    d2a_44k_snr(chp);

    d2a_dac_mute_unmute(chp);
    d2a_adc_mute_unmute(chp);
    d2a_dac_vol_ctrl(chp);
    d2a_adc_vol_ctrl(chp);

    d2a_adc_drc_limiter(chp);
    d2a_dac_drc_limiter(chp);
    d2a_adc_drc_agc(chp);
    d2a_dac_drc_agc(chp);


    d2a_dac_eq(chp);
    d2a_dac_mix(chp);
    d2a_adc_mix(chp);

    d2a_adc_dither_s_r(chp);
    d2a_adc_dither_s_t(chp);
    d2a_adc_dither_d_r(chp);
    d2a_adc_dither_d_t(chp);

    d2a_left_right_swap(chp);

    i2s_mono_mode(chp);

    d2a_96k_audio(chp);
#endif
  }while(0);
}

void i2s_test_24B(BaseSequentialStream *chp)
{
  audio_thead_param rec_param, play_param;
  thread_t *tp;

  chprintf(chp, "i2s_test_24B_test ...");

  rec_param.stream = chp;
  rec_param.filename = _T("0:i2s_test_24B.wav");
  rec_param.sample_rate = I2S_SAMPLE_8K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  play_param.stream = chp;
  play_param.filename = _T("0:sin_48k_24b.wav");

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);
  audioRecordStart(&g_recordI2sCfg_24B, NULL);
  audioPlayStart(&g_plyI2sCfg_24B, I2S_PLY_BUFFER_SIZE / 8, NULL);

  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
    audioRecThread, &rec_param);

  audioPlaySetVolume(0);
  audioPlyThread(&play_param);
  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, " finish\r\n");
}

void cmd_i2s(BaseSequentialStream *chp, int argc, char *argv[])
{
  char *operate = 0;

  if(argc < 1){
    return ;
  }

  operate = argv[0];

#if 0
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
#endif


  if(!hs_fatfs_isMount(FATFS_MEMDEV_SD)) {
    chprintf(chp, "fatfs is not mount!\r\n\r\n");
    return ;
  }

  audioSetCodecSel(AUDIO_INSIDE_CODEC);
  //audioSetCodecSel(AUDIO_EXTERN_CODEC);
  audioStart();

  audio_thead_param rec_param, play_param;
  thread_t *tp;
  char *tmp = "test.wav";

  TCHAR *filename = 0;
  if(argc >= 2){
    tmp = argv[1];
    filename = (TCHAR *)hs_malloc((strlen(tmp) + 1) * sizeof(TCHAR), __MT_GENERAL);
    mbstowcs(filename, tmp, (strlen(tmp) + 1));
  }

  if(strcmp("play", operate) == 0){
    audioPlaySetVolume(-10);

    play_param.stream = chp;
    play_param.filename = filename;

    isStopRec = 0;

    audioSetPlaySource(AUDIO_PLAY_RAM);

    audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
    audioSetPlaySample(I2S_SAMPLE_44P1K);

    audioPlyThread(&play_param);
  }
  else if(strcmp("record", operate) == 0){
    uint16_t cycle = 0x7FFF;
    uint8_t add_reduce = 0;
    if(argc >= 3){
      cycle = strtol(argv[2], NULL, 10);
    }

    if(argc >= 4){
      add_reduce = strtol(argv[3], NULL, 10);
    }

    rec_param.stream = chp;
    rec_param.filename = filename;
    rec_param.sample_rate = I2S_SAMPLE_44P1K;
    rec_param.i2s_mode = I2S_PCMMODE_STEREO;
    HS_CODEC->DAC_CTRL |= (1 << 12);
    HS_PSO->CODEC_CFG |= ((1 << 9) | (add_reduce << 8) | (cycle << 16));
    audioSetRecordSource(AUDIO_RECORD_LINEIN);
    audioRecordStart(&g_recI2sCfg, NULL);
    audioSetRecordSample(I2S_SAMPLE_44P1K);
    isStopRec = 0;
    //audioSetTestMode(0x01);
    tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO,
      audioRecThread, &rec_param);
    chThdSleepMilliseconds(10000);

    isStopRec = 1;
    chThdWait(tp);
  }
  else if(strcmp("buffer_level", operate) == 0){
    int count = 10;
    if(argc >= 2){
      count = strtol(argv[1], NULL, 10);
    }

    int i, level;
    for(i = 0; i < count; i++){
      level = audioGetBufferLevel();
      chprintf(chp, "buffer level %d\r\n", level);
      chThdSleepMilliseconds(1000);
    }
  }
  else{
    chprintf(chp, "unknown operate\r\n");
  }

  if(filename != NULL){
    hs_free(filename);
  }
}

#if HS_CODEC_USE_INSIDE

#if 0
static void audio_full_loop(BaseSequentialStream *chp)
{
  chprintf(chp, "audio_full_loop ...");

  audioSetCodecSel(AUDIO_INSIDE_CODEC);
  audioStart();
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);

  audioSetTestMode(0x08);

  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);
  audioRecordStart(&g_recI2sCfg, NULL);

  chprintf(chp, " finish\r\n");
}
#endif

void cmd_audio_analog(BaseSequentialStream *chp, int argc, char *argv[])
{
  //char *operate = 0;
  //char *type = 0;
#if 1
	(void)chp;
	(void)argc;
	(void)argv;
#else
  if(argc >= 1){
    operate = argv[0];
  }

  if(argc >= 2){
    type = argv[1];
  }


  if(memcmp(operate, "test", strlen(operate)) == 0){
    if(memcmp(type, "pgaloop", strlen(type)) == 0){
      cpmEnableCODEC();
      HS_CODEC->ANA_CTRL_1 = (1 << 19);
      HS_CODEC->RSVD_1 |= (1 << 6);   //pgainput = linein
      //HS_CODEC->RSVD_1 &= ~(1 << 6);  //pgainput = mic

      HS_BTPHY->SPI_APB_SWITCH = 0;
      HS_BTPHY->ANALOGUE[0x62] |=  0x08;
      HS_CODEC->RSVD_1 |= (1 << 0);
      HS_BTPHY->SPI_APB_SWITCH = 1;
    }
    else if(memcmp(type, "addaloop", strlen(type)) == 0){
      cpmEnableCODEC();
      HS_CODEC->ANA_CTRL_1 = (1 << 19);
      HS_CODEC->RSVD_1 |= (1 << 6);

      HS_BTPHY->SPI_APB_SWITCH = 0;

      HS_BTPHY->ANALOGUE[0x62] |=  0x02;
      HS_BTPHY->ANALOGUE[0x62] |=  0x10;
      HS_BTPHY->SPI_APB_SWITCH = 1;
    }
    else if(memcmp(type, "phymode", strlen(type)) == 0){
      cpmEnableCODEC();
      HS_CODEC->ANA_CTRL_1 = (1 << 19);

      HS_CODEC->RSVD_1 |= (1 << 6);
      HS_CODEC->RSVD_1 |= (1 << 5);

      HS_BTPHY->SPI_APB_SWITCH = 0;
      HS_BTPHY->ANALOGUE[0x62] |=  0x02;
      HS_BTPHY->ANALOGUE[0x62] |=  0x10;

      HS_BTPHY->SPI_APB_SWITCH = 1;
    }
    else if(memcmp(type, "fullloop", strlen(type)) == 0){
      audio_full_loop(chp);
    }
    else{
      chprintf(chp, "unknow type\r\n");
    }
  }
  else if(memcmp(operate, "set", strlen(operate)) == 0){
    int value = 0;
    if(argc >= 2)
      value = strtol(argv[2], NULL, 10);

    if(memcmp(type, "drvgain", strlen(type)) == 0){
      value &= 0x01;
      HS_BTPHY->SPI_APB_SWITCH = 0;
      HS_BTPHY->ANALOGUE[0x62] = (HS_BTPHY->ANALOGUE[0x62] & 0xFFEF) | (value << 4);
      HS_BTPHY->SPI_APB_SWITCH = 1;
    }
    else if(memcmp(type, "pgagain", strlen(type)) == 0){
      value &= 0x07;

      HS_BTPHY->SPI_APB_SWITCH = 0;
      HS_BTPHY->ANALOGUE[0x60] =  (HS_BTPHY->ANALOGUE[0x60] & 0xFE3FF) | (value << 10);
      HS_BTPHY->SPI_APB_SWITCH = 1;
    }
    else if(memcmp(type, "adcdem", strlen(type)) == 0){
      value &= 0x01;

      HS_BTPHY->SPI_APB_SWITCH = 0;
      HS_CODEC->RSVD_1 = (HS_CODEC->RSVD_1 & 0xFFFB) | ((value << 2));
      HS_BTPHY->SPI_APB_SWITCH = 1;
    }
    else if(memcmp(type, "dacdem", strlen(type)) == 0){
      value &= 0x01;

      HS_BTPHY->SPI_APB_SWITCH = 0;
      HS_BTPHY->ANALOGUE[0x62] =  (HS_BTPHY->ANALOGUE[0x62] & 0xFFFB) | (value << 2);
      HS_BTPHY->SPI_APB_SWITCH = 1;
    }
    else if(memcmp(type, "refbyp", strlen(type)) == 0){
      value &= 0x01;
      HS_CODEC->RSVD_1 = (HS_CODEC->RSVD_1 & 0xFF7F) | ((value << 7));
    }
     else if(memcmp(type, "reffast", strlen(type)) == 0){
      value &= 0x01;
      HS_CODEC->RSVD_1 = (HS_CODEC->RSVD_1 & 0xFEFF) | ((value << 8));
    }
    else if(memcmp(type, "dither", strlen(type)) == 0){
      value &= 0x01;
      HS_CODEC->RSVD_1 = (HS_CODEC->RSVD_1 & 0xFFFD) | ((value << 1));
    }
    else if(memcmp(type, "rctune", strlen(type)) == 0){
      HS_BTPHY->SPI_APB_SWITCH = 0;
      __codec_set_bitsval(HS_BTPHY->ANALOGUE[0x60], 13, 15, value & 0x07);   //reg[16:13]
      __codec_set_bitsval(HS_BTPHY->ANALOGUE[0x61], 0, 0, value >> 3);
      HS_BTPHY->SPI_APB_SWITCH = 1;
    }
    else if(memcmp(type, "ldoau", strlen(type)) == 0){
      value &= 0x03;
      HS_BTPHY->SPI_APB_SWITCH = 0;
      HS_BTPHY->ANALOGUE[0x61] =  (HS_BTPHY->ANALOGUE[0x61] & 0xFFE7) | (value << 3);
      HS_BTPHY->SPI_APB_SWITCH = 1;
    }
    else if(memcmp(type, "biassel", strlen(type)) == 0){
      value &= 0x03;
      HS_BTPHY->SPI_APB_SWITCH = 0;
      HS_BTPHY->ANALOGUE[0x61] =  (HS_BTPHY->ANALOGUE[0x61] & 0x9FFF) | (value << 13);
      HS_BTPHY->SPI_APB_SWITCH = 1;
    }
    else{
       chprintf(chp, "unknow type\r\n");
    }
  }
  else{
    chprintf(chp, "unknow test\r\n");
  }
#endif
}
#endif

void cmd_audioRecord(BaseSequentialStream *chp, int argc, char *argv[])
{
  audio_thead_param param;
  uint32_t src, samp;
  thread_t *tp;

  if (argc != 2) {
    chprintf(chp, "Usage: audrec source sample\r\n\tsource:\r\n\t  0:linein 1:dmic 2: mic 3: fm\r\n");
    return;
  }

  if(!hs_fatfs_isMount(FATFS_MEMDEV_SD)) {
    chprintf(chp, "fatfs is not mount!\r\n\r\n");
    return ;
  }

  src  = atoll(argv[0]);
  samp = atoll(argv[1]);

  param.stream = chp;
  param.filename = _T("0:rec_audio.wav");
  param.sample_rate = samp;
  param.i2s_mode = I2S_PCMMODE_STEREO;

  isStopRec = 0;
  
  g_recI2sCfg.sample_rate = samp;  
  audioRecordStart(&g_recI2sCfg, NULL);
  audioSetRecordSource(src);

  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO, audioRecThread, &param);
  chThdSleepMilliseconds(10000);

  isStopRec = 1;
  chThdWait(tp);

  chprintf(chp, "sample: %d recorder finish!\r\n", samp);
}


static void audioRecPlyThread(void * arg)
{
  audio_thead_param *param = (audio_thead_param *)arg;
  BaseSequentialStream *chp = param->stream;
  uint32_t size, rec_len, ply_len, cpy_len;
  systime_t timeout = S2ST(-1);
  uint8_t *rec_ptr, *ply_ptr;

  size = 512;
  while(1){    
    
    if((rec_len = audioRecGetDataBuffer(&rec_ptr, size, timeout)) == 0){
      chprintf(chp, "audioRecGetDataBuffer failed!\r\n");
      continue;
    }

    if((ply_len = audioPlyGetDataBuffer(&ply_ptr, size, timeout)) == 0){
      chprintf(chp, "audioPlyGetDataBuffer failed!\r\n");
      continue;
    }

    cpy_len = rec_len > ply_len ? ply_len : rec_len;
    memcpy(ply_ptr, rec_ptr, cpy_len);

    audioRecGetDataDone(rec_ptr, cpy_len);
    audioPlySendDataDone(ply_ptr, cpy_len);

    if(isStopLoop)
      break;
  }
}

thread_t *tp;
void cmd_audioTestS(BaseSequentialStream *chp, int argc, char *argv[])
{
  audio_thead_param param;
  uint32_t mode, samp;

  if (argc != 2) {
    chprintf(chp, "Usage: audts mode sample\r\n");
    return;
  }

  mode = atoll(argv[0]);
  samp = atoll(argv[1]);

  param.stream = chp;
  
  g_recI2sCfg.sample_rate = samp;
  g_plyI2sCfg.sample_rate = samp;

  //g_recI2sCfg.i2s_mode = I2S_PCMMODE_MONO;
  //g_plyI2sCfg.i2s_mode = I2S_PCMMODE_MONO;

  //g_recI2sCfg.sample_width = I2S_BITWIDTH_24BIT;
  //g_plyI2sCfg.sample_width = I2S_BITWIDTH_24BIT;
  
  audioRecordStart(&g_recI2sCfg, NULL);
  audioPlayStart(&g_plyI2sCfg, I2S_PLY_BUFFER_SIZE / 8, NULL);

  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioSetPlaySource(AUDIO_PLAY_RAM);

  audioSetTestMode(mode);

  isStopLoop = 0;
  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO, audioRecPlyThread, &param);  
}

void cmd_audioTestE(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)chp;
  (void)argc;
  (void)argv;
  
  isStopLoop = 1;
  isStopRec = 1;

  if(tp)
    chThdWait(tp);

  audioPlayStop();
  audioRecordStop();

  chprintf(chp, "Finished!\r\n\r\n");
}

void cmd_audioAlgRecord(BaseSequentialStream *chp, int argc, char *argv[])
{
  audio_thead_param *param;
  uint32_t src, alg;
  //thread_t *tp;

  if (argc != 2) {
    chprintf(chp, "Usage: audalg source alg\r\n\tsource:\r\n\t  0:linein 1:dmic 2: mic 3: fm\r\n\talg:\r\n\t  0-no alg 1-aec 2-ans 3-aec+ans\r\n\r\n");
    return;
  }

  if(!hs_fatfs_isMount(FATFS_MEMDEV_SD)) {
    chprintf(chp, "fatfs is not mount!\r\n\r\n");
    return ;
  }

  param = (audio_thead_param *)hs_malloc(sizeof(audio_thead_param), __MT_Z_GENERAL);
  if(!param) {
    chprintf(chp, "mem alloc error!\r\n\r\n");
    return ;
  }
  

  src  = atoll(argv[0]);
  alg  = atoll(argv[1]);

  param->stream = chp;
  param->filename = _T("0:alg_recdata.wav");
  param->sample_rate = I2S_SAMPLE_8K;
  param->i2s_mode = I2S_PCMMODE_MONO;

  isStopRec = 0;
  
  g_recI2sCfg.sample_rate = I2S_SAMPLE_8K;  
  g_recI2sCfg.i2s_mode    = I2S_PCMMODE_MONO;
  audioRecordStart(&g_recI2sCfg, NULL);
  audioSetRecordSource(src);
  audioInvertI2sInput(TRACK_RR);
  //audioRecordSetVolume(10);

  tp = chThdCreateFromHeap(NULL, TEST_THREAD_STACK_SIZE, NORMALPRIO, audioRecThread, param);

  if(alg & 1)
    hs_audio_enAec();

  if(alg & 2)
    hs_audio_enAns();

  chprintf(chp, "Recording......\r\n");
  
  //chThdSleepMilliseconds(30000);

  //chprintf(chp, "Waiting......\r\n");
  
  //isStopRec = 1;
  //chThdWait(tp);

  //chprintf(chp, "Record finish!\r\n\r\n");
}

#endif  //end of HAL_USE_AUDIO
