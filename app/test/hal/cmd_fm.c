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
#include <stdlib.h>
#include "board.h"
#include "ch.h"
#include "hal.h"
#include "lib.h"
#include "chprintf.h"

/*===========================================================================*/
/* radio driver test code                            */
/*===========================================================================*/

#if HAL_USE_FM

#define __fm_set_bitval(val, bit, bitval)        \
do{                                                 \
  uint32_t mask;                                    \
  mask = 1u<<(bit);                                 \
  (val) = ((val)&~mask) | (((bitval)<<(bit))&mask); \
}while(0)

#define __fm_set_bitsval(val, s, e, bitval)      \
do{                                                 \
  HS_BTPHY->SPI_APB_SWITCH = 0;                     \
  uint32_t mask;                                    \
  mask = ((1u<<((e)-(s)+1)) - 1) << (s);            \
  (val) = ((val)&~mask) | (((bitval)<<(s))&mask);   \
  HS_BTPHY->SPI_APB_SWITCH = 1;                     \
}while(0)

extern struct FMDriver FMD0;
static hs_audio_config_t g_fmI2sCfg ={
  I2S_SAMPLE_32K,
  I2S_BITWIDTH_16BIT,
  I2S_BITWIDTH_32BIT,
  I2S_WORKMODE_MASTER,
  I2S_PCMMODE_STEREO,

  I2S_PLY_BLOCK_SIZE,
  6,
};

typedef struct{
  BaseSequentialStream *stream;
  TCHAR *filename;
  int sample_rate;
  int i2s_mode;
}audio_thead_param;

static volatile int isStopRec;

#if HAL_USE_FATFS
static THD_FUNCTION(fmRecThread, arg)
{
  audio_thead_param *param = (audio_thead_param *)arg;
  BaseSequentialStream *chp = param->stream;
  TCHAR *filename = param->filename;
  FIL *fp = NULL;
  wave_header header;

  uint8_t *pData;
  uint32_t size;
  systime_t timeout = S2ST(1);

  char tmp[64];
  wcstombs(tmp, filename, sizeof(tmp) - 1);
  tmp[sizeof(tmp) - 1] = 0;

  if(!hs_fatfs_isMount(FATFS_MEMDEV_SD)) {
    chprintf(chp, "fatfs is not mount!\r\n\r\n");
    return ;
  }

  if ((fp = (FIL *) hs_malloc(sizeof(FIL), __MT_DMA)) == NULL) {
    chprintf(chp, "hs_malloc failed!\r\n");
    goto exit ;
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
    goto exit ;
  }

  int length = 0;
  while(!isStopRec){
    size = MMCSD_BLOCK_SIZE * 4;
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
  if(fp != NULL){
    f_close(fp);
    hs_free(fp);
  }
}

void fm_record(BaseSequentialStream *chp)
{
  audio_thead_param rec_param;
  thread_t *tp_rec;

  chprintf(chp, "fm record ...");
  rec_param.stream = chp;
  rec_param.filename = (TCHAR *)_T("0:fm-rec.wav");
  rec_param.sample_rate = I2S_SAMPLE_32K;
  rec_param.i2s_mode = I2S_PCMMODE_STEREO;

  isStopRec = 0;
  audioSetRecordSource(AUDIO_RECORD_FM);
  //audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioRecordStart(&g_fmI2sCfg, NULL);
  tp_rec = chThdCreateFromHeap(NULL, 2048, NORMALPRIO, fmRecThread, &rec_param);
  chThdSleepMilliseconds(1000 * 20);
  isStopRec = 1;
  chThdWait(tp_rec);
  chprintf(chp, " over\r\n");
}
#endif

void _fm_help(BaseSequentialStream *chp)
{
  chprintf(chp, "Usage: fm action [params]\r\n");
  chprintf(chp, "          init   \r\n");
  chprintf(chp, "          freq   [.1xmhz to write]\r\n");
  chprintf(chp, "          rec    \r\n");
  chprintf(chp, "          rssi   [count=10]\r\n");
  chprintf(chp, "          vol    dB\r\n");
  chprintf(chp, "          th     [rssi_th=8] [snr_th=4] [stereo_rssi_th=22] [stereo_snr_th=-128] [chhc_th=15] [lrhc_th=12]\r\n");
  chprintf(chp, "          next   \r\n");
  chprintf(chp, "          prev   \r\n");
  chprintf(chp, "          scan   [min=875] [max=1080] [step=1]\r\n");
  chprintf(chp, "          swap   \r\n");
  chprintf(chp, "          agc    [fil_gain.tia_gain.gm_gain.lna_gm.lna_ro(hex) | agc_en=fffff]\r\n");
  chprintf(chp, "          test   [freq_fm=1 | freq_rf=2 | freq_sys=4 | ldo_lobuf=11 | ldo_vco=14 | in_xtal=20 | in_pll=21 ]\r\n");
  chprintf(chp, "          filter [lpf=0 | pos=1 | neg=2]\r\n");
  chprintf(chp, "          check  \r\n");
  chprintf(chp, "          rc     [rxadc.auadc.txdac.rxfil.rxtia(hex) | cali=fffff]\r\n");
  chprintf(chp, "          ldo    [lobuf.xtal.btvco(hex) | cali=fff]\r\n");
  chprintf(chp, "          monkey [count=32] [delay=500ms]\r\n");
}

void cmd_fm(BaseSequentialStream *chp, int argc, char *argv[]) {
  char *operate= "";
  int error = 0, value = 1;

  if(argc >= 1)
    operate = argv[0];
  argc--;
  if (argc >= 1) {
    value = atoi(argv[1]);
  }

  if(strncmp(operate, "init", strlen("init")) == 0)
  {
    static FMConfig config = {
      103900, //current
      108000, //max in china
       87500, //min in china
      100,
      {
        15, 12,  //chhc, lrhc
        8, 22,   //rssi
        4, -128, //snr
      }
    };

    #ifdef HS6601_FPGA
    MAX2829_init();
    audioSetCodecSel(AUDIO_EXTERN_CODEC);
    #else
    audioSetCodecSel(AUDIO_INSIDE_CODEC);
    #endif
    audioStart();
    audioPlayStart(&g_fmI2sCfg, 0, NULL);

    audioSetPlaySource(AUDIO_PLAY_FM);
    audioPlaySetVolume(0);

    fmStart(&FMD0, &config);
    //fmSetFrequency(&FMD0, 97.4 * 1000);
  }
  else if(strncmp(operate, "freq", strlen("freq")) == 0)
  {
    if (argc >= 1) {
      int dmhz = atoi(argv[1]);
      fmSetFrequency(&FMD0, dmhz * 100);
    }
    else {
      int khz = fmGetFrequency(&FMD0);
      chprintf(chp, "FM %d.%dMHz: IF_REG=0x%x\r\n", khz/1000, khz%1000, HS_BTPHY->IF_REG);
    }
  }
  else if(strncmp(operate, "rec", strlen("rec")) == 0)
  {
#if HAL_USE_FATFS
    fm_record(chp);
#endif
  }
  else if(strncmp(operate, "rssi", strlen("rssi")) == 0)
  {
    int i = 0;
    int count = 10;

    if (argc >= 1) {
      count = atoi(argv[1]);
    }

    while(i++ < count) {
      chprintf(chp, "reg:0x%x rssi:%ddB snr:%ddB\r\n", HS_BTPHY->FM_RSSI,
               fmGetRssi(&FMD0), fmGetSnr(&FMD0));
      chThdSleepMilliseconds(1000);
    }
  }
  else if(strncmp(operate, "vol", strlen("vol")) == 0)
  {
    audioPlaySetVolume(value);
  }
  else if(strncmp(operate, "th", strlen("th")) == 0)
  {
    if (argc >= 1)
      FMD0.th.rssi_th = atoi(argv[1]);
    if (argc >= 2)
      FMD0.th.snr_th = atoi(argv[2]);
    if (argc >= 3)
      FMD0.th.rssi_th_stereo = atoi(argv[3]);
    if (argc >= 4)
      FMD0.th.snr_th_stereo = atoi(argv[4]);
    if (argc >= 5)
      FMD0.th.chhc_th = atoi(argv[5]);
    if (argc >= 6)
      FMD0.th.lrhc_th = atoi(argv[6]);
    chprintf(chp, "threshold rssi=%d snr=%d rssi_stereo=%d snr_stereo=%d chhc=%d lrhc=%d\r\n",
             FMD0.th.rssi_th, FMD0.th.snr_th, FMD0.th.rssi_th_stereo, FMD0.th.snr_th_stereo, FMD0.th.chhc_th, FMD0.th.lrhc_th);
  }
  else if(strncmp(operate, "next", strlen("next")) == 0)
  {
    audioPlayMute();
    fmScanNext(&FMD0);
    audioPlayUnmute();
    chprintf(chp, "freq=%d rssi=%ddB snr=%ddB\r\n", fmGetFrequency(&FMD0), fmGetRssi(&FMD0), fmGetSnr(&FMD0));
  }
  else if(strncmp(operate, "prev", strlen("prev")) == 0)
  {
    audioPlayMute();
    fmScanPrev(&FMD0);
    audioPlayUnmute();
    chprintf(chp, "freq=%d rssi=%ddB snr=%ddB\r\n", fmGetFrequency(&FMD0), fmGetRssi(&FMD0), fmGetSnr(&FMD0));
  }
  else if(strncmp(operate, "scan", strlen("scan")) == 0)
  {
    int khz, delay = 200;
    if (argc >= 1)
      FMD0.freq_min = atoi(argv[1]) * 100;
    if (argc >= 2)
      FMD0.freq_max = atoi(argv[2]) * 100;
    if (argc >= 3)
      FMD0.step = atoi(argv[3]) * 100;
    if (argc >= 4)
      delay = atoi(argv[4]);
    for (khz = FMD0.freq_min; khz < FMD0.freq_max; khz += FMD0.step) {
      int i, sum_rssi = 0, sum_snr = 0;
      audioPlayMute();
      fmSetFrequency(&FMD0, khz);
      audioPlayUnmute();
      chprintf(chp, "\t%d", khz/100);
      for (i = 0; i < delay; i++) {
        osDelay(1);
        sum_rssi += fmGetRssi(&FMD0);
        sum_snr  += fmGetSnr(&FMD0);
      }
      chprintf(chp, "\t%d\t%d\t%d\t%d\r\n", sum_rssi/delay, sum_snr/delay, fmGetRssi(&FMD0), fmGetSnr(&FMD0));
      osDelay(1000);
    }
  }
  else if(strncmp(operate, "swap", strlen("swap")) == 0)
  {
    HS_BTPHY->IQ_IN_SWAP = 1 - HS_BTPHY->IQ_IN_SWAP;
    chprintf(chp, "IQ_IN_SWAP=%d\r\n", HS_BTPHY->IQ_IN_SWAP);
  }
  else if(strncmp(operate, "agc", strlen("agc")) == 0)
  {
    if (argc >= 1) {
      /* write */
      uint32_t gain = strtol(argv[1], NULL, 16);
      if (gain == 0xFFFFF) {
        HS_ANA->FM_AGC_CFG[0] |= (1 << 15) | (1 << 7); //gm_gain, tia_gain; lna_gm, lna_ro from fsm
        HS_ANA->RX_AGC_CFG[4] |= (1 << 16);            //fil_gain from fsm
        chprintf(chp, "FM AGC enabled: RX_AGC_CFG4=0x%08lx, FM_AGC_CFG0=0x%08lx\r\n", HS_ANA->RX_AGC_CFG[4], HS_ANA->FM_AGC_CFG[0]);
      }
      else {
        uint8_t fil_gain, tia_gain, gm_gain, lna_gm, lna_ro;
        fil_gain = (gain >> 16) & 0x0f;
        tia_gain = (gain >> 12) & 0x0f;
        gm_gain  = (gain >>  8) & 0x0f;
        lna_gm   = (gain >>  4) & 0x0f;
        lna_ro   = (gain >>  0) & 0x0f;
        HS_ANA->RX_AGC_CFG[4] = (HS_ANA->RX_AGC_CFG[4] & 0xFFFEFFFC) | (0 << 16) | (fil_gain << 0); //fil_gain[1:0]
        HS_ANA->FM_AGC_CFG[0] = (HS_ANA->FM_AGC_CFG[0] & 0xFFFF0000) | (0 << 15) | (0 << 7) |
          (gm_gain << (8+3)) | (tia_gain << 8) | (lna_gm << (0+3)) | (lna_ro << 0);                 //gm_gain[2:0], tia_gain[2:0], lna_gm[2:0], lna_ro[2:0]
        chprintf(chp, "FM gain regs set: RX_AGC_CFG4=0x%08lx, FM_AGC_CFG0=0x%08lx\r\n", HS_ANA->RX_AGC_CFG[4], HS_ANA->FM_AGC_CFG[0]);
      }
    }
    else {
      /* read */
      uint32_t rxagc, fmagc;
      HS_ANA->DBG_IDX = 2;
      rxagc = HS_ANA->DBG_RDATA;
      HS_ANA->DBG_IDX = 3;
      fmagc = HS_ANA->DBG_RDATA;
      chprintf(chp, "FM gain = %x.%x?.%x.%x.%x\r\n",
               (rxagc >> 16) & 0x3, (fmagc >> 12) & 0x0f, (fmagc >> 8) & 0x0f, (fmagc >> 4) & 0x0f, (fmagc >> 0) & 0x0f);
    }
  }
  else if(strncmp(operate, "test", strlen("test")) == 0)
  {
    int khz = fmGetFrequency(&FMD0);
    uint32_t start, end;

    value = strtol(argv[1], NULL, 16);

    if (value == 0x20) {
      cpm_switch_to_xtal();
      HS_PMU->ANA_CON |= (1 << 24); //sys_pll_gt_48m
      start = __nds32__mfsr(NDS32_SR_PFMC0);
      osDelay(1000);
      end = __nds32__mfsr(NDS32_SR_PFMC0);
      chprintf(chp, "in XTAL: 1s cycles %10d, ANA_CON=0x%08lx\r\n", end-start, HS_PMU->ANA_CON);
      return;
    }
    if (value == 0x21) {
      HS_PMU->ANA_CON &= ~(1 << 24);
      cpm_switch_to_pll();
      start = __nds32__mfsr(NDS32_SR_PFMC0);
      osDelay(1000);
      end = __nds32__mfsr(NDS32_SR_PFMC0);
      chprintf(chp, "in PLL : 1s cycles %10d, ANA_CON=0x%08lx\r\n", end-start, HS_PMU->ANA_CON);
      return;
    }

    if (value == 1)
      HS_ANA->FM_CFG[0] |= (1 << 8); //fmtest_en
    else if (value == 2)
      HS_ANA->REGS.RF_PLL_TEST = 0;  //RF PLL's feedback output @16MHz
    else if (value == 4)
      HS_ANA->REGS.SYS_PLL_TEST_EN = 1; //System PLL's feedback output @16MHz
    if (value <= 4) {
      __hal_set_bitsval(HS_ANA->COMMON_CFG[1], 20, 22, value); //tst_digi_ctrl
      if (value == 1) {
        khz -= 125;
        chprintf(chp, "FM freq test output %d.%dMHz at PC1: ", khz/4000, (khz - khz/4000*4000)/4);
      }
      else {
        chprintf(chp, "PLL feedback test output 16MHz at PC1: ");
      }
      chprintf(chp, " COMMON_CFG1=0x%08lx FM_CFG0=0x%08lx\r\n", HS_ANA->COMMON_CFG[1], HS_ANA->FM_CFG[0]);
    }

    if (value >= 0x10) {
      uint8_t ldo_out[8] = {0, 13, 13, 13, 16, 15, 13, 0};

      value = value & 0x0f;
      HS_ANA->REGS.RF_PLL_LDO_TEST = 1;
      HS_ANA->REGS.RF_PLL_LDO_SEL = value;

      chprintf(chp, "RF PLL LDO test output at PC0: %d.%dV\r\n", ldo_out[value]/10, ldo_out[value]%10);
    }

  }
  else if(strncmp(operate, "filter", strlen("filter")) == 0) {
    /* rxtia
       [25]fre_sel    b'1=pos b'0=neg
       [23]lp_cp_sel  b'1=complex filter; b'0=LPF for calibration
       [22]modsel_tia b'1=BT b'0=FM */
    /* rxfil
       [13:12]swap_fil b'00=complex filter pos, b'01=neg, b'1x=LPF for DCOC calibration
       [0]modsel_fil   b'1=BT b'0=FM */
    if (value == 0) {
      HS_ANA->COMMON_CFG[0] &= ~(1 << 23);
      HS_ANA->RX_FIL_CFG &= ~(3 << 12); HS_ANA->RX_FIL_CFG |= (3 << 12);
      chprintf(chp, "Low Pass Filter: COMMON_CFG0=0x%08lx RX_FIL_CFG=0x%08lx\r\n", HS_ANA->COMMON_CFG[0], HS_ANA->RX_FIL_CFG);
    }
    else if (value == 1) {
      HS_ANA->COMMON_CFG[0] |= (1 << 23); HS_ANA->COMMON_CFG[0] |= (1 << 25);
      HS_ANA->RX_FIL_CFG &= ~(3 << 12); HS_ANA->RX_FIL_CFG |= (0 << 12);
      chprintf(chp, "complex filter's pos: COMMON_CFG0=0x%08lx RX_FIL_CFG=0x%08lx\r\n", HS_ANA->COMMON_CFG[0], HS_ANA->RX_FIL_CFG);
    } else if (value == 2) {
      HS_ANA->COMMON_CFG[0] |= (1 << 23); HS_ANA->COMMON_CFG[0] &= ~(1 << 25);
      HS_ANA->RX_FIL_CFG &= ~(3 << 12); HS_ANA->RX_FIL_CFG |= (1 << 12);
      chprintf(chp, "complex filter's neg: COMMON_CFG0=0x%08lx RX_FIL_CFG=0x%08lx\r\n", HS_ANA->COMMON_CFG[0], HS_ANA->RX_FIL_CFG);
    }
  }
  else if(strncmp(operate, "check", strlen("check")) == 0)
  {
    chprintf(chp, "RF current source  is ");
    if ((HS_ANA->PD_CFG[0] & (1 << 21)) == 0)
      chprintf(chp, "on ");
    else
      chprintf(chp, "off");
    chprintf(chp, "  [21]pd_rfcstgm\r\n");

    chprintf(chp, "FM LNA             is ");
    if ((HS_ANA->PD_CFG[0] & (1 << 17)) == 0)
      chprintf(chp, "on ");
    else
      chprintf(chp, "off");
    chprintf(chp, "  [17]pd_fm_lna\r\n");

    chprintf(chp, "FM GM              is ");
    if ((HS_ANA->PD_CFG[0] & (1 << 18)) == 0)
      chprintf(chp, "on ");
    else
      chprintf(chp, "off");
    chprintf(chp, "  [18]pd_fm_gm\r\n");

    chprintf(chp, "FM Mixer           is ");
    if ((HS_ANA->PD_CFG[0] & (1 << 19)) == 0)
      chprintf(chp, "on ");
    else
      chprintf(chp, "off");
    chprintf(chp, "  [18]pd_fm_mixer\r\n");

    chprintf(chp, "RX TIA             is ");
    if ((HS_ANA->PD_CFG[2] & ((1 << 26) | (1 << 10))) == 0)
      chprintf(chp, "on ");
    else if (HS_ANA->PD_CFG[2] & (1 << 26))
      chprintf(chp, "fsm");
    else
      chprintf(chp, "off");
    chprintf(chp, "  [26][10]pd_rxtia\r\n");

    chprintf(chp, "RX Filter          is ");
    if ((HS_ANA->PD_CFG[2] & ((1 << 28) | (1 << 12))) == 0)
      chprintf(chp, "on ");
    else if (HS_ANA->PD_CFG[2] & (1 << 28))
      chprintf(chp, "fsm");
    else
      chprintf(chp, "off");
    chprintf(chp, "  [28][12]pd_rxfil\r\n");

    chprintf(chp, "RX ADC             is ");
    if ((HS_ANA->PD_CFG[2] & ((1 << 22) | (1 << 21) | (1 << 20) | (1 << 19) | (1 << 18) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6))) == 0)
      chprintf(chp, "on ");
    else if (HS_ANA->PD_CFG[2] & ((1 << 22) | (1 << 21) | (1 << 20) | (1 << 19) | (1 << 18)))
      chprintf(chp, "fsm");
    else
      chprintf(chp, "off");
    chprintf(chp, "  pd_rxadc_xxx\r\n");

    chprintf(chp, "RX LNA             is ");
    if (((HS_ANA->PD_CFG[2] & (1 << 24)) == 0) && (HS_ANA->PD_CFG[2] & (1 << 8)))
      chprintf(chp, "off");
    else if (HS_ANA->PD_CFG[2] & (1 << 24))
      chprintf(chp, "fsm");
    else
      chprintf(chp, "on ");
    chprintf(chp, "  [24][8]pd_rxlna\r\n");

    chprintf(chp, "RX GM              is ");
    if (((HS_ANA->PD_CFG[2] & (1 << 25)) == 0) && (HS_ANA->PD_CFG[2] & (1 << 9)))
      chprintf(chp, "off");
    else if (HS_ANA->PD_CFG[2] & (1 << 25))
      chprintf(chp, "fsm");
    else
      chprintf(chp, "on ");
    chprintf(chp, "  [25][9]pd_rxgm\r\n");

    chprintf(chp, "RX Mixer           is ");
    if (((HS_ANA->PD_CFG[2] & (1 << 27)) == 0) && (HS_ANA->PD_CFG[2] & (1 << 11)))
      chprintf(chp, "off");
    else if (HS_ANA->PD_CFG[2] & (1 << 27))
      chprintf(chp, "fsm");
    else
      chprintf(chp, "on ");
    chprintf(chp, "  [27][11]pd_rxmix\r\n");

    chprintf(chp, "PD_CFG0=0x%08lx PD_CFG2=0x%08lx\r\n", HS_ANA->PD_CFG[0], HS_ANA->PD_CFG[2]);
  }
  else if(strncmp(operate, "rc", strlen("rc")) == 0)
  {
    /* RC calibration for rxadc, txdac, auadc, rxfil, rxtia */
    if (argc >= 1) {
      uint32_t rctune = strtol(argv[1], NULL, 16);
      if (rctune == 0xFFFFF) {
        pmu_cali_rc();
        chprintf(chp, "RC calibration done: ");
      }
      else {
        __hal_set_bitsval(HS_ANA->COMMON_CFG[0], 4, 7,   (rctune >> 16) & 0xf);
        HS_ANA->REGS.RC_TUNE =                           (rctune >> 12) & 0xf;
        HS_ANA->REGS.TXDAC_BW_CAL =                      (rctune >>  8) & 0xf;
        __hal_set_bitsval(HS_ANA->RX_FIL_CFG, 4, 6,      (rctune >>  4) & 0xf);
        __hal_set_bitsval(HS_ANA->COMMON_CFG[1], 16, 18, (rctune >>  0) & 0xf);
        chprintf(chp, "RC tune set: ");
      }
    }
    chprintf(chp, "rc_cnt/2 = (t1+t2)/2 = %d\r\n", (HS_ANA->RC_CALIB_CNS >> 16) / 2);
    chprintf(chp, "rctune = rxadc.auadc.txdac.rxfil.rxtia=%d.%d.%d.%d.%d\r\n",
             (HS_ANA->COMMON_CFG[0] >> 4) & 0xf, HS_ANA->REGS.RC_TUNE, HS_ANA->REGS.TXDAC_BW_CAL,
             (HS_ANA->RX_FIL_CFG    >> 4) & 0x7,
             (HS_ANA->COMMON_CFG[1] >> 16) & 0x7);
  }
  else if(strncmp(operate, "ldo", strlen("ldo")) == 0)
  {
    uint32_t ctrl;

    /* low noise LDO calibration for lobuf, xtal, btvco */
    if (argc >= 1) {
      /* write */
      ctrl = strtol(argv[1], NULL, 16);
      if (ctrl == 0xFFF) {
        pmu_cali_ldo();
        HS_ANA->DBG_IDX = 7;
        ctrl = HS_ANA->DBG_RDATA;
        chprintf(chp, "LDO calibration done: en?.lobuf.xtal.btvco=%x?.%x.%x.%x\r\n",
                 (ctrl >> 8) & 0x7,
                 (ctrl >> 4) & 0x3,
                 (ctrl >> 2) & 0x3,
                 (ctrl >> 0) & 0x3);
      } else {
        HS_ANA->LDO_CALIB_CNS &= ~(1 << 0); //ld_calib_en: 0=reg
        HS_ANA->PD_CFG[1] &= ~((1 << 28) | (1 << 22) | (1 << 12) | (1 << 6));
        /* reg<268:267> */
        HS_ANA->REGS.CON_LDO_VCO = ctrl & 0x3;
        /* reg<257:256> */
        HS_ANA->REGS.CON_LDO_XTAL = (ctrl >> 4) & 0x3;
        /* reg<45:44> */
        HS_ANA->REGS.LDO_LOBUF_CTRL = (ctrl >> 8) & 0x3;
        chprintf(chp, "LDO regs set: lobuf.xtal.btvco=%x.%x.%x\r\n",
                 HS_ANA->REGS.LDO_LOBUF_CTRL, HS_ANA->REGS.CON_LDO_XTAL, HS_ANA->REGS.CON_LDO_VCO);
      }
    }
    else {
      /* read */
      if (HS_ANA->LDO_CALIB_CNS & (1 << 0)) {
        /* ld_calib_en: 1=fsm */
        HS_ANA->DBG_IDX = 7;
        ctrl = HS_ANA->DBG_RDATA;
        chprintf(chp, "LDO fsm get: en?.lobuf.xtal.btvco=%x?.%x.%x.%x\r\n",
                 (ctrl >> 8) & 0x7,
                 (ctrl >> 4) & 0x3,
                 (ctrl >> 2) & 0x3,
                 (ctrl >> 0) & 0x3);
      }
      else {
        /* ld_calib_en: 0=reg */
        chprintf(chp, "LDO regs get: lobuf.xtal.btvco=%x.%x.%x\r\n",
                 HS_ANA->REGS.LDO_LOBUF_CTRL, HS_ANA->REGS.CON_LDO_XTAL, HS_ANA->REGS.CON_LDO_VCO);
      }
    }
  }
  else if(strncmp(operate, "vb_mix", strlen("vb_mix")) == 0)
  {
    /* reg<98:88>vb_mix[2:0] */
    HS_ANA->REGS.VB_MIX = value;
  }
  else if(strncmp(operate, "fm_pdet", strlen("fm_pdet")) == 0)
  {
    /* reg<106:103>fm_pdet[3:0] */
    HS_ANA->REGS.FM_PDET = value;
  }
  else if(strncmp(operate, "fm_pdet_space", strlen("fm_pdet_space")) == 0)
  {
    /* reg<108:107>fm_pdet_space[1:0] */
    HS_ANA->REGS.FM_PDET_SPACE = value;
  }
  else if(strncmp(operate, "monkey", strlen("monkey")) == 0) {
    uint16_t msg, i, count = 32, delay = 500;
    if (argc >= 1) count = atoi(argv[1]);
    if (argc >= 2) delay = atoi(argv[2]);
    for (i = 0; i < count; i++) {
      msg = HS_CFG_EVENT_PLAYER_START + rand() % (HS_CFG_EVENT_PLAYER_FUNCSET - HS_CFG_EVENT_PLAYER_START);
      if(HS_CFG_OK != hs_cfg_sysSendMsg(5, HS_CFG_SYS_EVENT, msg, 0))
        chprintf(chp, "Send message of event error!\r\n");
      osDelay(delay);

      if (i % 10 == 1) {
        int ii;
        for (ii=0; ii<6; ii++) {
          hs_cfg_sysSendMsg(1, HS_CFG_SYS_EVENT, 0x101, 0);
          osDelay(delay);
        }
      }
    }
  }
  else
  {
    error = 1;
  }

  if (error)
    _fm_help(chp);
}

#endif
