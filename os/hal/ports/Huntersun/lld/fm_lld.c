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

/**
 * @file    hs66xx/fm_lld.c
 * @brief   RADIO Driver subsystem low level driver source.
 *
 * @addtogroup FM
 * @{
 */

#include "ch.h"
#include "hal.h"
#include <string.h>

#if HAL_USE_FM || defined(__DOXYGEN__)

#ifdef HS6601_FPGA
#if HS_FM_USE_MAX2829
#include "rf_lld.h"
#endif
#endif


/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   FM driver identifier.
 */
FMDriver FMD0;


/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level RADIO driver initialization.
 *
 * @notapi
 */
void fm_lld_init(void) {
  fmObjectInit(&FMD0);
  FMD0.thread = NULL;
  FMD0.freq_max = 108000; //kHz
  FMD0.freq_min =  87400; //kHz
  FMD0.step = 100;

  FMD0.th.chhc_th = 15;
  FMD0.th.lrhc_th = 12;
  FMD0.th.rssi_th = 8; //10;
  FMD0.th.rssi_th_stereo = 22; //lets phy switch stereo or mono automatically; if 100, auto mono
  FMD0.th.snr_th = 4; //10;
  FMD0.th.snr_th_stereo  = -128;//10; //static [-128,-128] on 2017.02.21
}


/**
 * @brief   Configures and activates the FM peripheral.
 *
 * @param[in] fmp      pointer to the @p FMDriver object
 *
 * @notapi
 */
void fm_lld_start(FMDriver *fmp) {
  /* if escape from BT mode, BTPHY must be reset, otherwise FM's rssi will be +40dBm? */
  cpmResetBTPHY();
  cpm_delay_us(1);
  cpmEnableFM();

  HS_ANA->REGS.RXADC_SEL_INPUT = 0; /* always 0 for hs6601; select RXADC input for hs66000: BT RX; 1=BT TX cali or FM */
  HS_ANA->REGS.RF_PLL_MODE = 0;     /* RF PLL mode: 1=BT; 0=FM */

  /* by zhangzd on 2016/09/29 */
  HS_ANA->REGS.FM_PDET = 4;       //6 -> 4
  HS_ANA->REGS.FM_PDET_SPACE = 3; //2 -> 3
  HS_ANA->FM_AGC_CFG[1] = 0x00201140; //dpd_thes_l:0x5a -> 0x20; kdp_thes_h:0xb5 -> 0x40

  /* fil_gain by fm agc: 6dBm -> 0dBm */
  HS_ANA->FILT_GAINC_LUT_REG  = 0x00000000; //filt_gainc_lut: 0x012->0x000, referenced by dcoc calibration & fm agc

  /* turn off parts of BT RX manually because workaround BT's RXENA fsm will turn on these */
  /* [27][11]pd_rxmix: 1 */
  /* [25][9]pd_rxgm:   1 */
  /* [24][8]pd_rxlna:  1 */
  /* [23][7]pd_rxagc:  1 */
  HS_ANA->PD_CFG[2] &= ~((1 << 27) | (1 << 25) | (1 << 24) | (1 << 23));
  HS_ANA->PD_CFG[2] |=  ((1 << 11) | (1 <<  9) | (1 <<  8) | (1 << 7));
  /* turn off BT's rx agc of digital */
  /* rx_gain: [15:14]lna_gain [13]shrt_lna [12]sw_lna [11]en_att [10:9]att_ctrl [8:6]tca_ctrl [5]en_gm [4:2]tia_ctrl(by fm agc) [1:0]filt_gain(0 for fm) */
  HS_ANA->RX_AGC_CFG[4] = 0xA2600000; //[16]rx_gain_flag ...: 0=reg

  /* power on LDOs of RF PLL: [0]pd_bt_ldodiv, [1]pd_bt_ldommd, [2]pd_bt_synth_cp, [3]pd_bt_synth_ldocp, [4]pd_bt_synth_vc_det, [5]pd_bt_ldopfd, [6]pd_ldo_v1p3_lobuf, [12]pd_bt_ldovco */
  HS_ANA->PD_CFG[1] &= ~((1 << 28) | (1 << 22) | (1 << 21) | (1 << 20) | (1 << 19) | (1 << 18) | (1 << 17) | (1 << 16)); //xxx_flag
  HS_ANA->PD_CFG[1] &= ~((1 << 12) | (1 <<  6) | (1 <<  5) | (1 <<  4) | (1 <<  3) | (1 <<  2) | (1 <<  1) | (1 <<  0)); //xxx_reg
  /* pd_bt_vco_pkdetect: fsm */

  /* VCO amplitude calibration */
  //HS_ANA->SDM_CFG |= (1 << 12);   //bt_synth_dith_en_sdm
  HS_ANA->PEAKDET_CFG |= 0x10; //[4]vco_amp_cal_start: w1
  chThdSleep(MS2ST(1));
  osalDbgAssert((HS_ANA->PEAKDET_CFG & (1 << 4))== 0, "vco amplititude calibration");

#if 0 //this step is not required by FM, because it is used to setup ctune/ftune table for BT
  /* AFC calibration */
  HS_ANA->VCO_AFC_CFG[0] |= 0x100; //[8]rf_pll_afc_start: w1
  chThdSleep(MS2ST(1));
  osalDbgAssert((HS_ANA->VCO_AFC_CFG[0] & (1 << 8)) == 0, "afc calibration");
#endif

  /* [12]bt_synth_dith_en_sdm: 1 */
  osalDbgAssert(HS_ANA->SDM_CFG & (1 << 12), "dither is not enabled");

  /* [15]pd_bt_mmd_fm:    0
     ?[16]pd_fm_synth_ldo: 0, removed in hs6601 */
  HS_ANA->PD_CFG[0] &= ~((1 << 16) | (1 << 15));
  /* [12]rf_pll_mode_fm=mode_fm_sdm: b'0=BT b'1=FM */
  HS_ANA->VCO_AFC_CFG[0] |= (1 << 12);

  /* [4]rf_pll_track_mn=1 [3:0]rf_pll_track_reg=bt_vco_track=b'100 by yanguang */
  HS_ANA->VTRACK_CFG = (1 << 4) | (0x4 << 0);

#if 1
  //fm_lld_set_freq(fmp, 103900/*khz*/);
  fm_lld_set_freq(fmp, fmp->config->freq_current);
#else
  /* [15:12]fm_divnum[3:0]=0~15
     [31:0]fm_freq=bt_synth_freq_dem[31:0]: 12-bit=int(mHz) 20-bit=fra
      FM(mHz)= ((fm_freq>>20) + (fm_freq&0xfffff)/(1024*1024)) / (2 * (fm_divnum+8))
      FM(kHz)= ((fm_freq>>20)*1000 + (fm_freq&0xfffff)*1000/(1024*1024)) / (2 * (fm_divnum+8))

     103.9mHz= 103900kHz= 2493600kHz / 24 = ((2493*1000)+(600))kHz / 24
                                                                     fm_divnum=24/2 - 8=4
                                            fm_freq=(2493<<20) + 600*1024*1024/1000

     103.9mHz-125kHz= (103900-125)kHz= 2490600kHz / 24 = ((2490*1000)+(600))kHz / 24
                                                                     fm_divnum=24/2 - 8=4
                                            fm_freq=(2490<<20) + 600*1024*1024/1000
  */
  HS_ANA->FM_CFG[0] = (HS_ANA->FM_CFG[0] & ~(0xf << 12)) | (4/*fm_divnum*/ << 12);
  HS_ANA->FM_CFG[1] = (2490<<20) | 0x99999;
  HS_ANA->VCO_AFC_CFG[0] |= 0x100; //[8]rf_pll_afc_start: w1
  chThdSleep(MS2ST(1));
  osalDbgAssert((HS_ANA->VCO_AFC_CFG[0] & (1 << 8)) == 0, "fm pll lock");
#endif
  /* workaround: keep BT's RXENA high to un-reset SDM */
  HS_BTPHY->SPI_APB_SWITCH = 0;
  HS_BTPHY->ANALOGUE[0x30] = (1<<7) + (1<<4); //[7]rx_start: w1 trigger to start rx fsm

  /* [17]pd_fm_lna:   0
     [18]pd_fm_gm:    0
     [19]pd_fm_mixer: 0 */
  HS_ANA->PD_CFG[0] &= ~((1 << 19) | (1 << 18) | (1 << 17));
  chThdSleep(MS2ST(1));
  /* [12]pd_rxadc_dacbias: 0 */
  HS_ANA->PD_CFG[0] &= ~((1 << 12));

  /* [28][12]pd_rxfil:        0, pd_fil is wrong name */
  /* [26][10]pd_rxtia:        0 */
  /* [22][6]pd_rxadc_ovdect:  0 */
  /* [21][5]pd_rxadc_q:       0 */
  /* [20][4]pd_rxadc_i:       0 */
  /* [19][3]pd_rxadc_dem:     0 */
  /* [18][2]pd_rxadc_biasgen: 0 */
  HS_ANA->PD_CFG[2] &= ~((1 << 28) | (1 << 26) | (1 << 22) | (1 << 21) | (1 << 20) | (1 << 19) | (1 << 18));
  HS_ANA->PD_CFG[2] &= ~((1 << 12) | (1 << 10) | (1 <<  6) | (1 <<  5) | (1 <<  4) | (1 <<  3) | (1 <<  2));

  /* [0]sel_mod_dcoc  b'1=BT b'0=FM */
  HS_ANA->ADCOC_CNS &= ~(1 << 0);
  /* [21]pd_rfcstgm [20]pd_dcoc by xuebaiqing; pd_dcoc=1 for fm by wangjianting */
  HS_ANA->PD_CFG[0] &= ~((1 << 21) | (1 << 20));
  HS_ANA->PD_CFG[0] |= (1 << 20);
  /* rxtia
     [25]fre_sel    b'1=pos b'0=neg
     [23]lp_cp_sel  b'1=complex filter; b'0=LPF for calibration
     [22]modsel_tia b'1=BT b'0=FM */
  /* rxfil
     [13:12]swap_fil b'00=complex filter pos, b'01=neg, b'1x=LPF for DCOC calibration
     [0]modsel_fil  b'1=BT b'0=FM */
#if 1 //lpf by zhangzd on 2016/09/29
  HS_ANA->COMMON_CFG[0] = (0 << 30) | (0 << 29) | (0 << 25) | (0 << 23) | (0 << 22) | (2 << 14) | (1 << 8) | (8 << 4); //lpf in rxtia
  HS_ANA->RX_FIL_CFG = (3 << 12) | (0 << 8) | (3 << 4) | (0 << 0); //lpf in rxfil
#else
  //HS_ANA->COMMON_CFG[0] = (0 << 30) | (0 << 29) | (1 << 25) | (1 << 23) | (0 << 22) | (2 << 14) | (1 << 8) | (8 << 4); //pos
  //HS_ANA->RX_FIL_CFG = (0 << 12) | (0 << 8) | (3 << 4) | (0 << 0); //pos
  HS_ANA->COMMON_CFG[0] = (0 << 30) | (0 << 29) | (0 << 25) | (1 << 23) | (0 << 22) | (2 << 14) | (1 << 8) | (8 << 4); //neg
  HS_ANA->RX_FIL_CFG = (1 << 12) | (0 << 8) | (3 << 4) | (0 << 0); //neg
#endif

  /* agc: fm_lna_gm[2:0] fm_lna_ro[2:0] fm_gm_gain[2:0] */

  HS_BTPHY->IF_REG = 0x40;     //fm's if is 125khz
  HS_BTPHY->EN_DC_REMOVAL = 1; //enable dc removal
  HS_BTPHY->DCNOTCH_K_SEL = 9; //2^-11 bandwidth? for fm
  HS_BTPHY->RMV2_DEEM = (1u << 28) | (1u << 24) | (3u << 20) | (4u << 16) | (1u << 12) | (1u << 8) | (1u << 4) | (3u << 0); //m_pll_kn=1, fm_dc_rmv2_k=3 on 2017.02.21

  /* fm stereo new alg by zhangzd on 2016.12.09 */
  HS_BTPHY->FM_MODE = MODE_FM | 0;//MODE_MONO; //fm mode & stereo
  //HS_BTPHY->TX_RX_EN |= 0x02;            //enable bt rx data to modem on hs6600?
  HS_BTPHY->FM_RSSI = ((HS_BTPHY->FM_RSSI) & ~0x03) | 0x03;  // fm_rssi_k_sel: 2^-11
#if 1
  /* disable hi-cut for true SNR by zhangzd on 2016.09.30, but it will lost 3+dB */
  HS_BTPHY->FM_CHHC_FILT &= ~((1 << 20) | (1 << 16)); //fm_hc_mode=0: reg set; fm_chhc_filt_en=0 in default
  HS_BTPHY->FM_LRHC_FILT &= ~(1 << 16);               //fm_lrhc_filt_en=0 in default
#else
  /* enable hi-cut manually outside to run SNR benchmark */
  HS_BTPHY->FM_CHHC_FILT = (1 << 20) | (0 << 16) | (16 << 8) | (14 << 0); //fm_hc_mode=1: auto, it will enable hi-cut mode automatically if SNR is worse
  //HS_BTPHY->FM_CHHC_FILT = (1 << 20) | (1 << 16) | (16 << 8) | (14 << 0); //fm_chhc_filt_en=1
  //HS_BTPHY->FM_LRHC_FILT =             (1 << 16) | (16 << 8) | (14 << 0); //fm_lrhc_filt_en=1
#endif
  /* fm stereo by zhangzd on 2016.09.30 */
  //HS_BTPHY->FM_STEREO = (12 << 24) | (22 << 16) | ((uint8_t)-80 << 8) | ((uint8_t)-70 << 0);
  /* fm stereo new alg by zhangzd on 2016.12.09 */
  HS_BTPHY->FM_STEREO = ((uint8_t)(fmp->th.snr_th_stereo)/*snr_thl*/ << 24) | ((uint8_t)(fmp->th.snr_th_stereo+0)/*snr_thh*/ << 16) | ((uint8_t)-80 << 8) | ((uint8_t)-70 << 0);
  HS_BTPHY->IQ_IN_SWAP = 1;                //swap is required by FM; and BT swap it in analog circuit
  chThdSleep(MS2ST(1));

  fmp->freq_current = fmp->config->freq_current;
  fmp->freq_min = fmp->config->freq_min;
  fmp->freq_max = fmp->config->freq_max;
  fmp->step = fmp->config->step;
  fmp->th = fmp->config->th;

#ifdef HS6601_FPGA
#if  HS_FM_USE_MAX2829
  MAX2829_RxMode_init();
  //MAX2829_init();
#endif
#endif
}

/**
 * @brief   Deactivates the  FM peripheral.
 *
 * @param[in] fmp      pointer to the @p FMDriver object
 *
 * @notapi
 */
void fm_lld_stop(FMDriver *fmp) {
  (void)fmp;

  /* canncel BT's RXENA */
  HS_BTPHY->ANALOGUE[0x30] = (1<<6) + (1<<4); //[6]rx_end: w1 trigger to end rx fsm when rxen is spi mode
                                              //[4]rxen_sel: 0=gio; 1=spi

  HS_BTPHY->IQ_IN_SWAP = 0;
  HS_ANA->REGS.RXADC_SEL_INPUT = 0;
  HS_ANA->REGS.RF_PLL_MODE = 1;

  HS_BTPHY->FM_MODE = 0;     //common phy mode for bt
  HS_BTPHY->IF_REG = 0x300;    //bt's if is 750khz
  HS_BTPHY->DCNOTCH_K_SEL = 3; //2^-3 bandwidth? for bt
  //HS_BTPHY->TX_RX_EN &= ~0x02;

  /* [0]modsel_fil  b'1=BT b'0=FM */
  HS_ANA->RX_FIL_CFG |= (1 << 0);
  /* [22]modsel_tia b'1=BT b'0=FM */
  HS_ANA->COMMON_CFG[0] |= (1 << 22);
  /* [0]sel_mod_dcoc  b'1=BT b'0=FM */
  HS_ANA->ADCOC_CNS |= (1 << 0);

  /* [4]rf_pll_track_mn=0 for BT */
  HS_ANA->VTRACK_CFG = (0 << 4) | (0x4 << 0);

  /* [12]rf_pll_mode_fm=mode_fm_sdm: b'0=BT b'1=FM */
  HS_ANA->VCO_AFC_CFG[0] &= ~(1 << 12);

  /* post */
  HS_ANA->PD_CFG[0] = 0xFFCFEFFF; //[12]pd_rxadc_dacbias [21]pd_rfcstgm [20]pd_dcoc
  HS_ANA->PD_CFG[1] = 0xFFFFFFEF; //[4]pd_bt_synth_vc_det=0 [20]pd_bt_synth_vc_det_flag(removed)
  HS_ANA->PD_CFG[2] = 0xFFFFFFFF;
  HS_ANA->RX_AGC_CFG[4] = 0xA2610000; //[16]rx_gain_flag ...: 1=fsm to enable AGC

  cpmDisableFM();
}

/**
 * @brief   fm pll afc calibration
 *
 * @param[in] fmp      pointer to the @p FMDriver object
 *
 * @notapi
 */
void fm_lld_afc_calibration(FMDriver *fmp) {
  (void)fmp;
  //osalDbgAssert((HS_ANA->VCO_AFC_CFG[0] & (1 << 8)) == 0, "afc calibration");
  HS_ANA->VCO_AFC_CFG[0] |= (1 << 8);   //rf_pll_afc_start
  chThdSleep(MS2ST(1)); //500us
}

/**
 * @brief   set FM frequency. refer to FM_PLL IOÎÄµµ.docx
 *
 * @param[in] fmp      pointer to the @p FMDriver object
 * @param[in] frequency   fm frequency
 ¡Á
 * @notapi
 */
int fm_lld_set_freq(FMDriver *fmp, int frequency){
  int div = 0, mhz, fra;

  /* workaround: [8]bt_synth_con_mmd_mn=1 [7:0]bt_synth_con_mmd=0x00 to turn on mmd during BT's RXENA is high, before change the new frequency */
  __hal_set_bitsval(HS_ANA->SDM_CFG, 0, 8, 0x100);
  cpm_delay_us(1);

  if(frequency > fmp->freq_max){
    fmp->freq_current = fmp->freq_max;
  }
  else if(frequency < fmp->freq_min){
    fmp->freq_current = fmp->freq_min;
  }
  else{
    fmp->freq_current = frequency;
  }

  fmp->freq_current -= 125; /* fm's if */
  for(div = (0+8); div <= (15+8); div++){
    int pllout_khz = fmp->freq_current * 2 * div;
    if((pllout_khz >= BT_MHZ_MIN*1000) && (pllout_khz <= BT_MHZ_MAX*1000)){
      break;
    }
  }

  osalDbgAssert(div < 24, "fm lld set frequency");

  HS_ANA->FM_CFG[0] = (HS_ANA->FM_CFG[0] & 0xFFFF0FFF) | ((div-8) << 12); //fm_divnum
  mhz = fmp->freq_current * 2 * div / 1000;
  fra = fmp->freq_current * 2 * div - mhz * 1000;
  HS_ANA->FM_CFG[1] = (mhz << 20) | (fra*1024*1024/1000);
  cpm_delay_us(1);

  fm_lld_afc_calibration(fmp);

  /* workaround: [8]bt_synth_con_mmd_mn=0 to turn off mmd, FM PLL will become lock */
  __hal_set_bitsval(HS_ANA->SDM_CFG, 0, 8, 0x000);
  //wait 100us, but skip

  fmp->freq_current += 125; /* fm's if */
  return fmp->freq_current;
}

/**
 * @brief   get FM frequency.
 *
 * @param[in] fmp      pointer to the @p FMDriver object
 *
 * @notapi
 */
int fm_lld_get_freq(FMDriver *fmp){
  (void)fmp;
  return fmp->freq_current;
}

/**
 * @brief   get FM signal strength.
 *
 * @param[in] fmp      pointer to the @p FMDriver object
 *
 * @notapi
 */
int8_t fm_lld_get_rssi(FMDriver *fmp)
{
  (void)fmp;
  return (int8_t)((HS_BTPHY->FM_RSSI >> 12) & 0xff);
}

/**
 * @brief   get FM signal noise ratio
 *
 * @param[in] fmp      pointer to the @p FMDriver object
 *
 * @notapi
 */
int8_t fm_lld_get_snr(FMDriver *fmp)
{
  (void)fmp;
  return (int8_t)((HS_BTPHY->FM_RSSI >> 4) & 0xff);
}

static int _fm_lld_get_rssi(FMDriver *fmp)
{
  (void)fmp;
  int i, sum = 0;

  for (i=0; i<5; i++) {
    sum += (int8_t)((HS_BTPHY->FM_RSSI >> 12) & 0xff);
    chThdSleep(MS2ST(1));
  }
  return (sum / 5);
}

static int _fm_lld_get_snr(FMDriver *fmp)
{
  (void)fmp;
  int i, max = -1;

  for (i=0; i<5; i++) {
    int8_t snr = (int8_t)((HS_BTPHY->FM_RSSI >> 4) & 0xff);
    if (snr > max)
      max = snr;
    chThdSleep(MS2ST(1));
  }
  return max;
}

static int _fm_lld_scan(FMDriver *fmp, int step)
{
  int index, freq_end, round;
  int snr[4] = {0}, rssi[4] = {0}, frequency[4] = {0}, fc_seeked = 0;
  int rssi_noise;

  round = 0;
  index = 0;
  /* return the current point if found nothing */
  fc_seeked = fmp->freq_current;
  /* start point */
  frequency[3] = fmp->freq_current + step;
  /* end point */
  freq_end = fmp->freq_current;
  if(freq_end < fmp->freq_min){
    freq_end = fmp->freq_max;
  }

  if(freq_end > fmp->freq_max){
    freq_end = fmp->freq_min;
  }

  //HS_BTPHY->FM_CHHC_FILT = 0x10100a;
  //chThdSleep(MS2ST(1));
  //HS_BTPHY->FM_LRHC_FILT = 0x001008;
  //HS_BTPHY->FM_RSSI = 0x02;

  while(1){
    if(frequency[3] < fmp->freq_min){
      frequency[3] = fmp->freq_max;
    }

    if(frequency[3] > fmp->freq_max){
      frequency[3] = fmp->freq_min;
    }

    fm_lld_set_freq(fmp, frequency[3]);
    if(frequency[3] == freq_end){
      round++;

      /* scan one round for next/prev in order */
      if(round >= 1){
        /* return the start point if found nothing */
        //fc_seeked = fmp->freq_current;
        break;
      }
    }

    chThdSleep(MS2ST(30));
    rssi[3] = _fm_lld_get_rssi(fmp);
    snr[3] = _fm_lld_get_snr(fmp);
    if (index<3)
    {
      index++;
    }

    if ((index == 3) && 
        (rssi[2] >= (rssi[0] + fmp->th.rssi_th)) &&
        (rssi[3] <= rssi[2]) &&
        (rssi[1] <= rssi[2]) &&
        (snr[2]  >= fmp->th.snr_th) &&
        /* unreliable at 96.0MHz due to interference by digital source */
        (frequency[2] != 96000)) {
      fc_seeked = frequency[2];

#if 1//fm stereo new alg by zhangzd on 2016.12.09
      rssi_noise = rssi[0];
      HS_BTPHY->FM_STEREO = (HS_BTPHY->FM_STEREO & 0xFFFF0000) | ((uint8_t)(rssi_noise+fmp->th.rssi_th_stereo)/*rssi_thl*/ << 8) | ((uint8_t)(rssi_noise+fmp->th.rssi_th_stereo+10)/*rssi_thh*/ << 0);
#else
      if ((rssi[2] - rssi[0]) >= fmp->th.stereo_th)
        HS_BTPHY->FM_MODE = MODE_FM |         0; //stereo
      else
        HS_BTPHY->FM_MODE = MODE_FM | MODE_MONO; //mono
#endif

      if ((rssi[2] - rssi[0]) >= fmp->th.chhc_th) {
        HS_BTPHY->FM_CHHC_FILT &= ~(1 << 16); //fm_chhc_filt_en=0
      } else {
        HS_BTPHY->FM_CHHC_FILT |= (1 << 16); //fm_chhc_filt_en=1
      }
      if ((rssi[2] - rssi[0]) >= fmp->th.lrhc_th) {
        HS_BTPHY->FM_LRHC_FILT &= ~(1 << 16); //fm_lrhc_filt_en=0
      } else {
        HS_BTPHY->FM_LRHC_FILT |= (1 << 16); //fm_lrhc_filt_en=1
      }
      break;
    }
    else if ((index == 3) &&
             (rssi[1] >= (rssi[3] + fmp->th.rssi_th)) &&
             (rssi[0] <= rssi[1]) &&
             (rssi[3] <= rssi[2]) &&
             (snr[1]  >= fmp->th.snr_th) &&
             /* unreliable at 96.0MHz due to interference by digital source */            
             (frequency[1] != 96000)) {
      fc_seeked = frequency[1];

#if 1
      rssi_noise = rssi[3];
      HS_BTPHY->FM_STEREO = (HS_BTPHY->FM_STEREO & 0xFFFF0000) | ((uint8_t)(rssi_noise+fmp->th.rssi_th_stereo)/*rssi_thl*/ << 8) | ((uint8_t)(rssi_noise+fmp->th.rssi_th_stereo+10)/*rssi_thh*/ << 0);
#else
      if ((rssi[1] - rssi[3]) >= fmp->stereo_th)
        HS_BTPHY->FM_MODE = MODE_FM |         0; //stereo
      else
        HS_BTPHY->FM_MODE = MODE_FM | MODE_MONO; //mono
#endif

      if ((rssi[1] - rssi[3]) >= fmp->th.chhc_th) {
        HS_BTPHY->FM_CHHC_FILT &= ~(1 << 16); //fm_chhc_filt_en=0
      } else {
        HS_BTPHY->FM_CHHC_FILT |= (1 << 16); //fm_chhc_filt_en=1
      }
      if ((rssi[1] - rssi[3]) >= fmp->th.lrhc_th) {
        HS_BTPHY->FM_LRHC_FILT &= ~(1 << 16); //fm_lrhc_filt_en=0
      } else {
        HS_BTPHY->FM_LRHC_FILT |= (1 << 16); //fm_lrhc_filt_en=1
      }
      break;
    }
    else{
      rssi[0] = rssi[1];
      rssi[1] = rssi[2];
      rssi[2] = rssi[3];
      snr[0] = snr[1];
      snr[1] = snr[2];
      snr[2] = snr[3];
      frequency[0] = frequency[1];
      frequency[1] = frequency[2];
      frequency[2] = frequency[3];
    }

    frequency[3] = frequency[2] + step;
  }

  fm_lld_set_freq(fmp, fc_seeked);

  return fc_seeked;
}


int fm_lld_scan_next(FMDriver *fmp)
{
  (void)fmp;
  return _fm_lld_scan(fmp, fmp->step);
}

int fm_lld_scan_perv(FMDriver *fmp)
{
  (void)fmp;
  return _fm_lld_scan(fmp, -fmp->step);;
}

/**
 * @brief   get FM phy threshold after scan a channel
 *
 * @param[in] fmp      pointer to the @p FMDriver object
 * @return             [17]lrhc_filt_en [16]chhc_filt_en [15:8]stereo_rssi_thl [7:0]stereo_rssi_thh
 *
 * @notapi
 */
uint32_t fm_lld_get_hwctx(FMDriver *fmp)
{
  (void)fmp;
  uint32_t th = HS_BTPHY->FM_STEREO & 0x0000FFFF;
  th |= HS_BTPHY->FM_CHHC_FILT & (1u << 16);
  th |= (HS_BTPHY->FM_LRHC_FILT & (1u << 16)) << 1;
  return th;
}

/**
 * @brief   set FM phy threshold after set a channel
 *
 * @param[in] fmp      pointer to the @p FMDriver object
 *
 * @notapi
 */
void fm_lld_set_hwctx(FMDriver *fmp, uint32_t th)
{
  (void)fmp;

  if (0 == th)
    return;
  HS_BTPHY->FM_STEREO = (HS_BTPHY->FM_STEREO & 0xFFFF0000) | (th & 0x0000FFFF);
  if (th & (1u << 16))
    HS_BTPHY->FM_CHHC_FILT |= (1u << 16);
  else
    HS_BTPHY->FM_CHHC_FILT &= ~(1u << 16);
  if (th & (1u << 17))
    HS_BTPHY->FM_LRHC_FILT |= (1u << 16);
  else
    HS_BTPHY->FM_LRHC_FILT &= ~(1u << 16);
}

#endif /* HAL_USE_FM */

/** @} */
