#include <string.h>
#include <stdlib.h> 
#include "lib.h"

void cmd_pwmSet(BaseSequentialStream *chp, int argc, char *argv[]) {

  //uint32_t period, hWidth;
  
  (void)argv;
  if(argc != 2) {
    chprintf(chp, "Usage: pwmset period active_width\r\n");
    return ;
  }

  //period = atoll(argv[0]);
  //hWidth = atoll(argv[1]);

  palSetPadMode(IOPORT0, 15, PAL_MODE_OUTPUT | PAL_MODE_ALTERNATE(PAD_FUNC_GPIO) | PAL_MODE_DRIVE_CAP(3));
  //hs_pwm_set(2, 1, period, hWidth);
}

void cmd_pwmExit(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argv;
  if(argc != 0) {
    chprintf(chp, "Usage: pwmexit\r\n");
    return ;
  }

  //hs_pwm_close(2);
}

/* 
 * eq of codec 
 */
hs_codec_eqpara_t *g_pstEq;

void cmd_eqSetFreq(BaseSequentialStream *chp, int argc, char *argv[]) {

  #if HAL_USE_AUDIO
  uint32_t i, flag = 1;

  if(argc != EQ_BAND_NUM) {
    chprintf(chp, "Usage: eqfreq band_freq1 ... band_freq7 \r\n");
    return;
  }

  if(!g_pstEq) {
    flag = 0;
    g_pstEq = hs_malloc(sizeof(hs_codec_eqpara_t), __MT_Z_GENERAL);
  }

  for(i=0; i<EQ_BAND_NUM; i++)
    g_pstEq->u32Freq[i] = atoll(argv[i]);

  if(flag)
    hs_codec_setEq(g_pstEq);
  #else
  (void)chp;
  (void)argc;
  (void)argv;
  #endif
}


void cmd_eqSetGain(BaseSequentialStream *chp, int argc, char *argv[]) {

  #if HAL_USE_AUDIO
  uint32_t i, flag = 1;

  if(argc != EQ_BAND_NUM) {
    chprintf(chp, "Usage: eqgain gain1 ... gain7 \r\n");
    return;
  }

  if(!g_pstEq) {
    flag = 0;
    g_pstEq = hs_malloc(sizeof(hs_codec_eqpara_t), __MT_Z_GENERAL);
  }

  for(i=0; i<EQ_BAND_NUM; i++)
    g_pstEq->s32Gain[i] = atoll(argv[i]);
  
  if(flag)
    hs_codec_setEq(g_pstEq);
  #else
  (void)chp;
  (void)argc;
  (void)argv;
  #endif
}

void cmd_eqSetPoint(BaseSequentialStream *chp, int argc, char *argv[]) {

  #if HAL_USE_AUDIO
  uint32_t pointIdx, freq, gain, bandWidth;

  if(argc != 4) {
    chprintf(chp, "Usage: eqpoint pointIdx freq gain bandWidth \r\n");
    return;
  }

  pointIdx  = atoll(argv[0]);
  freq      = atoll(argv[1]);
  gain      = atoll(argv[2]);
  bandWidth = atoll(argv[3]);
  
  hs_codec_setPointEq(pointIdx, freq, gain, bandWidth);
  #else
  (void)chp;
  (void)argc;
  (void)argv;
  #endif
}


