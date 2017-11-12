/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio
                 Copyright (C) 2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn

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
 * @file    hs66xx/codec_lld.c
 * @brief   audio codec Driver subsystem low level driver source template.
 *
 * @addtogroup CODEC
 * @{
 */

#include "ch.h"
#include "hal.h"

#if HAL_USE_CODEC || defined(__DOXYGEN__)


/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/
#define HS_CODEC_IRQ_W1C TRUE

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
* @brief   CODEC driver identifier.
*/
#if HS_CODEC_USE_DRV && !defined(__DOXYGEN__)
CODECDriver CODECD;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

#ifdef HS6601_FPGA
//extern uint8_t low_power_enable;
static int codec_sel = AUDIO_EXTERN_CODEC;
#else
static int codec_sel = AUDIO_INSIDE_CODEC; 
#endif

#if HS_CODEC_USE_WM8753

#define WM8753_DEV_I2C            (&I2CD0)
#define WM8753_I2C_ADDR           0x1a
#define WM8753_I2C_WRCMD_LEN      2
static uint32_t g_bclk;
#endif

const hs_codec_boardinfo_t *hs_boardGetCodecInfo(void);

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
static void usleep(int us)
{
  //delay_4cycles(us * (cpm_get_clock(HS_CPU_CLK) / 1000 / 1000 / 4));
  cpm_delay_us(us);
}

#if HS_CODEC_USE_INSIDE
void _codec_config_audio_pa(CODECDriver *codecp, uint8_t enable)
{
  const hs_codec_boardinfo_t *pstInfo = codecp->pboardinfo;

  if(!pstInfo)
    return ;
  
  if (pstInfo->pa_gpio < PIN_NUM) {
    uint8_t gpio = codecp->pboardinfo->pa_gpio & 0x7f;
    ioportid_t port;

    if (gpio < 16) {
      port = IOPORT0;
    } else {
      port = IOPORT1;
      gpio -= 16;
    }

    palSetPadMode(port, gpio,  PAL_MODE_OUTPUT|PAL_MODE_ALTERNATE(PAD_FUNC_GPIO)|PAL_MODE_DRIVE_CAP(3));
    
    if((enable) && (pstInfo->pa_lvl))
      palSetPad(port, gpio);
    else
      palClearPad(port, gpio);

    if(enable)
    {
      if(pstInfo->pa_lvl)
        palSetPad(port, gpio);
      else
        palClearPad(port, gpio);        
    }
    else
    {
      if(!pstInfo->pa_lvl)
        palSetPad(port, gpio);
      else
        palClearPad(port, gpio);    
    }
  }
}


static inline void _codec_analog_audioPowerOn(void)
{
  __codec_clr_bit(HS_ANA->AU_ANA_CFG[0], 17);
  usleep(2);
}

static inline void _codec_analog_audioPowerOff(void)
{
  __codec_set_bit(HS_ANA->AU_ANA_CFG[0], 17);
  usleep(2);
}

static inline void _codec_analog_refPowerOn(void)
{
  __codec_clr_bit(HS_ANA->AU_ANA_CFG[0], 14);
  pmu_ana_set(28, 28, 1);
  usleep(2);
  pmu_ana_set(28, 28, 0);
  
  pmu_ana_set(27, 27, 1);
  usleep(2);
  pmu_ana_set(27, 27, 0);
}

static inline void _codec_analog_refPowerOff(void)
{
  __codec_set_bit(HS_ANA->AU_ANA_CFG[0], 14);
  usleep(2);
}

static inline void _codec_analog_biasgenPowerOn(void)
{
  __codec_clr_bit(HS_ANA->AU_ANA_CFG[0], 19);
  usleep(2);
}

static inline void _codec_analog_biasgenPowerOff(void)
{
  __codec_set_bit(HS_ANA->AU_ANA_CFG[0], 19);
  usleep(2);
}

static inline void _codec_analog_clkPowerOn(void)
{
  __codec_clr_bit(HS_ANA->AU_ANA_CFG[0], 18);
  usleep(2);
}

static inline void _codec_analog_clkPowerOff(void)
{
  __codec_set_bit(HS_ANA->AU_ANA_CFG[0], 18);
  usleep(2);
}

static inline void _codec_analog_vddPowerOn(void)
{
  __codec_clr_bit(HS_ANA->AU_ANA_CFG[0], 6);
  __codec_clr_bit(HS_ANA->AU_ANA_CFG[0], 15);
  usleep(2);
}

static inline void _codec_analog_vddPowerOff(void)
{
  __codec_set_bit(HS_ANA->AU_ANA_CFG[0], 15);
  __codec_set_bit(HS_ANA->AU_ANA_CFG[0], 6);
  usleep(2);
}

static inline void _codec_analog_tiaPowerOn(void)
{
  __codec_set_bitsval(HS_ANA->AU_ANA_CFG[0], 2, 3, 0);
  usleep(2);
}

static inline void _codec_analog_tiaPowerOff(void)
{
  __codec_set_bitsval(HS_ANA->AU_ANA_CFG[0], 2, 3, 3);
  usleep(2);
}

static inline void _codec_analog_dacPowerOn(void)
{
  __codec_set_bitsval(HS_ANA->AU_ANA_CFG[0], 10, 11, 0);
  usleep(2);
}

static inline void _codec_analog_dacPowerOff(void)
{
  __codec_set_bitsval(HS_ANA->AU_ANA_CFG[0], 10, 11, 3);
  usleep(2);
}

static inline void _codec_analog_drvPowerOn(void)
{
  __codec_set_bitsval(HS_ANA->AU_ANA_CFG[0], 7, 9, 0);
  usleep(2);
  pmu_ana_set(318, 319, 0);
}

static inline void _codec_analog_drvPowerOff(void)
{
  __codec_set_bitsval(HS_ANA->AU_ANA_CFG[0], 7, 9, 7);
  usleep(2);
  pmu_ana_set(318, 319, 3);
}

static inline void _codec_analog_adcPowerOn(void)
{
  __codec_set_bitsval(HS_ANA->AU_ANA_CFG[0], 12, 13, 0);
  pmu_ana_set(7, 7, 1);
  pmu_ana_set(2, 3, 3);
  usleep(1);
  pmu_ana_set(7, 8, 0);
}

static inline void _codec_analog_adcPowerOff(void)
{
  __codec_set_bitsval(HS_ANA->AU_ANA_CFG[0], 12, 13, 3);
  usleep(2);
}

static inline void _codec_analog_pgaPowerOn(uint32_t isLinein, uint32_t gain)
{
  __codec_set_bitsval(HS_ANA->AU_ANA_CFG[0], 4, 5, 0);
  
  usleep(2);
  codec_analog_set_pga_gain(gain);
  
  pmu_ana_set(17, 17, 0); 
  pmu_ana_set(18, 18, (isLinein & 1));  
}

static inline void _codec_analog_pgaPowerOff(void)
{
  __codec_set_bitsval(HS_ANA->AU_ANA_CFG[0], 4, 5, 3);
  usleep(2);
}

static int _codec_analog_micbiasCalibration(void)
{
  uint32_t tune = 0;
  
  __codec_set_bitsval(HS_ANA->AU_ANA_CFG[0], 0, 1, 0);
  usleep(2);
  
  pmu_ana_set(37, 40, tune++);
  usleep(10);
  
  while( (HS_ANA->AU_ANA_CFG[0] & 0x80000000) == 0)
  {
  	pmu_ana_set(37, 40, tune++);
    usleep(10);
    
    if(tune > 15)
    {
      __codec_set_bit(HS_ANA->AU_ANA_CFG[0], 0);
      return -1;
    }
  }
  
  __codec_set_bit(HS_ANA->AU_ANA_CFG[0], 0);
  return 0;
}

static void _codec_analog_commonPowerOn(void)
{
  _codec_analog_audioPowerOn();
  _codec_analog_refPowerOn();
  _codec_analog_biasgenPowerOn();
  _codec_analog_vddPowerOn();
}

static void _codec_analog_playPowerOn(uint32_t isSingle)
{
  (void)isSingle;
  
  _codec_analog_vddPowerOn();
  _codec_analog_clkPowerOn();
  _codec_analog_tiaPowerOn();  
  _codec_analog_dacPowerOn();
  
  pmu_ana_set(21, 22, 2);
  pmu_ana_set(34, 34, 1);
  pmu_ana_set(318, 319, 0);  
  pmu_ana_set(37, 40, 8);
}

static void _codec_analog_playPowerOff(void)
{
  usleep(2);
  _codec_analog_dacPowerOff(); 
  
  usleep(2);
  _codec_analog_tiaPowerOff();
  _codec_analog_clkPowerOff();
  _codec_analog_vddPowerOff();
}

static void _codec_analog_recPowerOn(const hs_codec_boardinfo_t *pboardinfo)
{
  _codec_analog_vddPowerOn();
  _codec_analog_clkPowerOn();
  
  _codec_analog_pgaPowerOn(0, pboardinfo->pga_gain_mic);
  usleep(10);
  
  _codec_analog_adcPowerOn();
  usleep(10);  
}

static void _codec_analog_recPowerOff(void)
{
  _codec_analog_adcPowerOff();
  
  usleep(10);
  _codec_analog_pgaPowerOff(); 
  
  //usleep(10);
  //_codec_analog_clkPowerOff();
  //_codec_analog_vddPowerOff();
}

static void _codec_analog_commonPowerOff(void)
{
  _codec_analog_playPowerOff();
  _codec_analog_recPowerOff(); 

  _codec_analog_vddPowerOff();
  
  _codec_analog_biasgenPowerOff();
  _codec_analog_refPowerOff();

  _codec_analog_audioPowerOff();
}

static void _codec_enableDrc(CODECDriver *codecp)
{
  (void)codecp;
  #if 0
  HS_CODEC_Type *codec_r = codecp->codec;
  
  __codec_set_bitsval(codec_r->DAC_DRC_CTRL_3, 2,  6,  7);      //-1 db   LT
  __codec_set_bitsval(codec_r->DAC_DRC_CTRL_3, 8,  14, 12);     //-7 db   CT
  __codec_set_bitsval(codec_r->DAC_DRC_CTRL_3, 16, 22, 0x7f);     //-36 db  ET
  __codec_set_bitsval(codec_r->DAC_DRC_CTRL_3, 24, 26, 7);      //-42 db  NT

  __codec_set_bitsval(codec_r->DAC_DRC_CTRL_4, 0, 2, 4);	      //cs
  __codec_set_bitsval(codec_r->DAC_DRC_CTRL_4, 3, 5, 5);	      //es

  __codec_set_bitsval(codec_r->DAC_DRC_CTRL_3, 0, 1, 3);
  #endif
}

static void _codec_set_deci_filt(CODECDriver *codecp, uint32_t sample_rate)
{
  if(sample_rate == I2S_SAMPLE_44P1K || sample_rate == I2S_SAMPLE_22K ||
     sample_rate == I2S_SAMPLE_11K){
       codecp->codec->ADC_DECI_FILT_11 = 0xd9f753aa;
       codecp->codec->ADC_DECI_FILT_12 = 0xe6fd133a;
       codecp->codec->ADC_DECI_FILT_13 = 0x133a;

       codecp->codec->ADC_DECI_FILT_21 = 0xce6b55b0;
       codecp->codec->ADC_DECI_FILT_22 = 0xdae922e1;
       codecp->codec->ADC_DECI_FILT_23 = 0x22e1;

       codecp->codec->ADC_DECI_FILT_31 = 0xe44c51f0;
       codecp->codec->ADC_DECI_FILT_32 = 0xee4f0e3c;
       codecp->codec->ADC_DECI_FILT_33 = 0x0e3c;

       codecp->codec->ADC_DECI_FILT_41 = 0xc6955755;

       codecp->codec->ADC_DECI_FILT_42 = 0xf648133a;
       codecp->codec->ADC_DECI_FILT_43 = 0x133a;

       codecp->codec->ADC_DECI_FILT_51 = 0xc1dc58f8;

       codecp->codec->ADC_DECI_FILT_52 = 0x11e10e51;
       codecp->codec->ADC_DECI_FILT_53 = 0x0e51;
     }
  else{
    codecp->codec->ADC_DECI_FILT_11 = 0xcbed6256;
    codecp->codec->ADC_DECI_FILT_12 = 0xe7b7117f;
    codecp->codec->ADC_DECI_FILT_13 = 0x117f;

    codecp->codec->ADC_DECI_FILT_21 = 0xd59c5ea3;
    codecp->codec->ADC_DECI_FILT_22 = 0xcbe12186;
    codecp->codec->ADC_DECI_FILT_23 = 0x2186;

    codecp->codec->ADC_DECI_FILT_31 = 0xc56b6500;
    codecp->codec->ADC_DECI_FILT_32 = 0xf3d60cb5;
    codecp->codec->ADC_DECI_FILT_33 = 0x0cb5;

    codecp->codec->ADC_DECI_FILT_41 = 0xde5f5b59;

    codecp->codec->ADC_DECI_FILT_42 = 0xe567118f;
    codecp->codec->ADC_DECI_FILT_43 = 0x118f;

    codecp->codec->ADC_DECI_FILT_51 = 0xc1866709;

    codecp->codec->ADC_DECI_FILT_52 = 0x0b1e0cb5;
    codecp->codec->ADC_DECI_FILT_53 = 0x0cb5;
  }
}

void codec_lld_micbias_calibration(void)
{
  _codec_analog_micbiasCalibration();
}


#endif  //HS_CODEC_USE_INSIDE

static uint32_t _codec_get_bclk(uint32_t sample, uint32_t width, uint32_t *pbclk)
{
  uint32_t bclk, res = 12000000, bclk_flg = I2S_BCLK_12MHZ;

  #if HS_AUDIO_HWREPAIR_WSCLK
  *pbclk = bclk_flg;
  return res;
  #endif
  
  bclk = sample * 2 * (width + 1);
  if((sample == I2S_SAMPLE_44P1K)
     || (sample == I2S_SAMPLE_22K)
       || (sample == I2S_SAMPLE_11K))
  {
    bclk = 3000000;
  }

#if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC){
    if(g_bclk != 0)
      bclk = g_bclk;
  }
#endif

  if(bclk <= 600000)
  {
    bclk_flg = I2S_BCLK_0P6MHZ;
    res = 600000;
  }

  if(bclk > 600000)
  {
    bclk_flg = I2S_BCLK_1P2MHZ;
    res = 1200000;
  }

  if(bclk > 1200000)
  {
    bclk_flg = I2S_BCLK_2P4MHZ;
    res = 2400000;
  }

  if(bclk > 2400000)
  {
    bclk_flg = I2S_BCLK_3MHZ;
    res = 3000000;
  }

  if(bclk > 3000000)
  {
    bclk_flg = I2S_BCLK_4P8MHZ;
    res = 4800000;
  }

  if(bclk > 4800000)
  {
    bclk_flg = I2S_BCLK_6MHZ;
    res = 6000000;
  }

  if(bclk > 6000000)
  {
    bclk_flg = I2S_BCLK_12MHZ;
    res = 12000000;
  }

  *pbclk = bclk_flg;

#if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC){
    if(g_bclk == 0)
      g_bclk = res;
  }
#endif  

  return res;
}

static void _codec_set_samplerate(CODECDriver *codecp, hs_audio_config_t *cfgp,
hs_audio_streamdir_t dir)
{
  uint32_t sample_rate = cfgp->sample_rate;
  uint32_t ws_width = 32;
  uint32_t bclk, bclk_reg, ws_right, ws_eq;
#if HS_CODEC_USE_INSIDE
  uint32_t rate_cfg=0x21, dmic_cfg=0;
#endif

  switch(cfgp->ws_width)
  {
  case I2S_BITWIDTH_16BIT:
    ws_width = 16;
    break;
  case I2S_BITWIDTH_24BIT:
    ws_width = 24;
    break;
  case I2S_BITWIDTH_32BIT:
    ws_width = 32;
    break;
  case I2S_BITWIDTH_64BIT:
    ws_width = 64;
    break;
  case I2S_BITWIDTH_128BIT:
    ws_width = 128;
    break;
  case I2S_BITWIDTH_192BIT:
    ws_width = 192;
    break;
  default:
    ws_width = 32;
    break;
  }

#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    switch(cfgp->sample_rate)
    {
    case I2S_SAMPLE_8K:
      rate_cfg = 0x44;
      dmic_cfg = 1;
      break;
    case I2S_SAMPLE_11K:
      rate_cfg = 0x24;
      dmic_cfg = 0;
      break;
    case I2S_SAMPLE_12K:
      rate_cfg = 0x14;
      dmic_cfg = 1;
      break;
    case I2S_SAMPLE_16K:
      rate_cfg = 0x42;
      dmic_cfg = 1;
      break;
    case I2S_SAMPLE_22K:
      rate_cfg = 0x22;
      dmic_cfg = 0;
      break;
    case I2S_SAMPLE_24K:
      rate_cfg = 0x12;
      dmic_cfg = 1;
      break;
    case I2S_SAMPLE_32K:
      rate_cfg = 0x41;
      dmic_cfg = 1;
      break;
    case I2S_SAMPLE_44P1K:
      rate_cfg = 0x21;
      dmic_cfg = 0;
      break;
    case I2S_SAMPLE_48K:
      rate_cfg = 0x11;
      dmic_cfg = 1;
      break;
    default:
      rate_cfg = 0x11;
      dmic_cfg = 1;
      break;
    };
  }
#endif

  bclk = _codec_get_bclk(sample_rate, ws_width, &bclk_reg);
  ws_width = bclk / sample_rate;
  ws_right =  ws_width / 2;
  ws_eq = 0;
  if(ws_width % 2)
  {
    ws_eq = 1;
    ws_right += 1;
  }

#if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC){
    if(dir == AUDIO_STREAM_PLAYBACK)
    {
      __codec_set_bitsval(codecp->codec->CLK_CTRL_2, 23, 23, ws_eq);
      __codec_set_bitsval(codecp->codec->CLK_CTRL_2, 24, 31, ws_right);
      __codec_set_bitsval(codecp->codec->CLK_CTRL_2, 11, 11, ws_eq);
      __codec_set_bitsval(codecp->codec->CLK_CTRL_2, 12, 19, ws_right);
    }
    else
    {
      __codec_set_bitsval(codecp->codec->CLK_CTRL_2, 11, 11, ws_eq);
      __codec_set_bitsval(codecp->codec->CLK_CTRL_2, 12, 19, ws_right);
    }

    __codec_set_bitsval(codecp->codec->CLK_CTRL_2, 20, 22, bclk_reg);
    __codec_set_bitsval(codecp->codec->CLK_CTRL_2, 8, 10, bclk_reg);
  }
#endif

#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    if(dir == AUDIO_STREAM_PLAYBACK)
    {
      __codec_set_bitsval(codecp->codec->CLK_CTRL_2, 20, 22, bclk_reg);
      __codec_set_bitsval(codecp->codec->CLK_CTRL_2, 23, 23, ws_eq);
      __codec_set_bitsval(codecp->codec->CLK_CTRL_2, 24, 31, ws_right);
      __codec_set_bitsval(codecp->codec->DAC_CTRL, 0, 7, rate_cfg);

      __codec_set_bitval(HS_PSO->CODEC_CFG, 9, 0);
    }
    else
    {
      __codec_set_bitsval(codecp->codec->CLK_CTRL_2, 8, 10, bclk_reg);
      __codec_set_bitsval(codecp->codec->CLK_CTRL_2, 11, 11, ws_eq);
      __codec_set_bitsval(codecp->codec->CLK_CTRL_2, 12, 19, ws_right);
      __codec_set_bitsval(codecp->codec->ADC_CTRL, 8, 14, rate_cfg);
      __codec_set_bitsval(codecp->codec->CLK_CTRL_2, 7, 7, dmic_cfg);
    }
  }
#endif
}

#if HS_CODEC_USE_WM8753

/*
* wm8753 register cache
* We can't read the WM8753 register space when we
* are using 2 wire for device control, so we cache them instead.
*/
static struct reg_default wm8753_regs[] = {
	{ 0x00, 0x0000 },
	{ 0x01, 0x0008 },
	{ 0x02, 0x0000 },
	{ 0x03, 0x000a },
	{ 0x04, 0x000a },
	{ 0x05, 0x0033 },
	{ 0x06, 0x0000 },
	{ 0x07, 0x0007 },
	{ 0x08, 0x00ff },
	{ 0x09, 0x00ff },
	{ 0x0a, 0x000f },
	{ 0x0b, 0x000f },
	{ 0x0c, 0x007b },
	{ 0x0d, 0x0000 },
	{ 0x0e, 0x0032 },
	{ 0x0f, 0x0000 },
	{ 0x10, 0x00c3 },
	{ 0x11, 0x00c3 },
	{ 0x12, 0x00c0 },
	{ 0x13, 0x0000 },
	{ 0x14, 0x0000 },
	{ 0x15, 0x0000 },
	{ 0x16, 0x0000 },
	{ 0x17, 0x0000 },
	{ 0x18, 0x0000 },
	{ 0x19, 0x0000 },
	{ 0x1a, 0x0000 },
	{ 0x1b, 0x0000 },
	{ 0x1c, 0x0000 },
	{ 0x1d, 0x0000 },
	{ 0x1e, 0x0000 },
	{ 0x1f, 0x0000 },
	{ 0x20, 0x0055 },
	{ 0x21, 0x0005 },
	{ 0x22, 0x0050 },
	{ 0x23, 0x0055 },
	{ 0x24, 0x0050 },
	{ 0x25, 0x0055 },
	{ 0x26, 0x0050 },
	{ 0x27, 0x0055 },
	{ 0x28, 0x0079 },
	{ 0x29, 0x0079 },
	{ 0x2a, 0x0079 },
	{ 0x2b, 0x0079 },
	{ 0x2c, 0x0079 },
	{ 0x2d, 0x0000 },
	{ 0x2e, 0x0005 },//adc input mode line
	{ 0x2f, 0x0000 },
	{ 0x30, 0x0000 },
	{ 0x31, 0x0097 },
	{ 0x32, 0x0097 },
	{ 0x33, 0x0000 },
	{ 0x34, 0x0004 },
	{ 0x35, 0x0000 },
	{ 0x36, 0x0083 },
	{ 0x37, 0x0024 },
	{ 0x38, 0x01ba },
	{ 0x39, 0x0000 },
	{ 0x3a, 0x0083 },
	{ 0x3b, 0x0024 },
	{ 0x3c, 0x01ba },
	{ 0x3d, 0x0000 },
	{ 0x3e, 0x0000 },
	{ 0x3f, 0x0000 },
};

void _codec_wm8753_write(uint8_t reg_addr, uint16_t val)
{
  uint8_t txBuf[WM8753_I2C_WRCMD_LEN];
#if HAL_USE_I2C
  systime_t u32TimeOut = MS2ST(40);
#endif
  msg_t s32Status = MSG_OK;

  txBuf[0] = reg_addr<<1 | ((val>>8)&1);
  txBuf[1] = val;

#if HAL_USE_I2C
  /* locked in the upper layer, in audio.c */
  chSysUnlock();
  i2cAcquireBus(WM8753_DEV_I2C);
  /*
  s32Status = i2cMasterTransmitTimeout(WM8753_DEV_I2C, WM8753_I2C_ADDR, txBuf,
      WM8753_I2C_WRCMD_LEN, NULL, 0, u32TimeOut);
      */
  s32Status = i2cMasterWriteMemTimeout(WM8753_DEV_I2C, WM8753_I2C_ADDR, txBuf[0],
    1, &txBuf[1], 1, u32TimeOut);
  i2cReleaseBus(WM8753_DEV_I2C);
  chSysLock();
#endif

  if (s32Status != MSG_OK)
  {
    audio_dbg("[wm8753 write]addr:0x%x value0x%x write error\r\n", reg_addr, val);
  }
}

void _codec_wm8753_set(uint8_t reg_addr, uint8_t start, uint8_t end, uint16_t val)
{
  __codec_set_bitsval(wm8753_regs[reg_addr].def, start, end, val);
  _codec_wm8753_write(reg_addr, wm8753_regs[reg_addr].def);
}

void _codec_wm8753_set_sample(uint32_t sample_rate)
{
  uint32_t rate_cfg = 0;

  switch(sample_rate)
  {
  case I2S_SAMPLE_8K:
    rate_cfg = 0x6;
    break;

  case I2S_SAMPLE_16K:
    rate_cfg = 0xa;
    break;

  case I2S_SAMPLE_32K:
    rate_cfg = 0xc;
    break;

  case I2S_SAMPLE_44P1K:
    rate_cfg = 0x11;
    break;

  case I2S_SAMPLE_48K:
    rate_cfg = 0x00;
    break;

  default:
    rate_cfg = 0x00;
    break;
  };

  _codec_wm8753_set(WM8753_SRATE1, 0, 0, 0x01);     //USB mode
  _codec_wm8753_set(WM8753_SRATE1, 1, 5, rate_cfg); //sample rate
}
#endif //HS_CODEC_USE_WM8753

/*===========================================================================*/
/* codec control functions.                                                  */
/*===========================================================================*/
void _codec_set_record_mute(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    __codec_adc_set_unmute(cddp->codec, 0);
    __codec_adc_set_mute(cddp->codec, 1);
  }
#endif

#if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC){
  }
#endif
}

void _codec_set_record_unmute(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    __codec_adc_set_unmute(cddp->codec, 1);
    __codec_adc_set_mute(cddp->codec, 0);
  }
#endif

#if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC){
  }
#endif
}

static void codec_lld_start_dac_drv(CODECDriver *codecp)
{
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    _codec_config_audio_pa(codecp, 0);
    __codec_dac_set_bypass(codecp->codec, 0);
    __codec_dac_set_unmute(codecp->codec, 0);
    __codec_dac_set_mute(codecp->codec, 1);

    __codec_dac_set_fir(codecp->codec, 1);

    __codec_dac_sw_reset(codecp->codec);
    __codec_dac_enable(codecp->codec);
    
    usleep(10);                                                 //wait 10us
    __codec_dac_set_fir(codecp->codec, 0);

    _codec_analog_playPowerOn(codecp->pboardinfo->drv_single);

    __codec_dac_set_unmute(codecp->codec, 1);
    __codec_dac_set_mute(codecp->codec, 0);
    //usleep(1000);
    //_codec_config_audio_pa(codecp, 1);
  }
#endif

#if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC){
    _codec_wm8753_set(WM8753_DAC, 3, 3, WM8753_DACMUTE);
    _codec_wm8753_set(WM8753_PWR1, 2, 3, (DACL << 1) | DACR);

    _codec_wm8753_set(WM8753_PWR3, 7, 8, (LOUT1 << 1) | ROUT1);
    _codec_wm8753_set(WM8753_PWR4, 0, 1, (LEFTMIX << 1)| RIGHTMIX);

    _codec_wm8753_set(WM8753_LOUTM1, 8, 8, LD2LO);
    _codec_wm8753_set(WM8753_ROUTM1, 8, 8, RD2RO);

    _codec_wm8753_set(WM8753_LOUTM1, 4, 6, 0);  //left mixer volume +6dB
    _codec_wm8753_set(WM8753_ROUTM1, 4, 6, 0);  //right mixer volume +6dB

    if(codecp->tx_cfg->sample_width == I2S_BITWIDTH_24BIT){
      _codec_wm8753_set(WM8753_HIFI, 0, 3, (WM8753_WL_24 << 2) | WM8753_FT_I2S);
    }
    else{
      _codec_wm8753_set(WM8753_HIFI, 0, 3, (WM8753_WL_16 << 2) | WM8753_FT_I2S);
    }

    _codec_wm8753_set(WM8753_DAC, 3, 3, ~WM8753_DACMUTE);

  }
#endif

  __codec_set_bitsval(codecp->codec->CLK_CTRL_1, 3, 3, 0x1);
}

static void codec_lld_stop_dac_drv(CODECDriver *codecp)
{
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    _codec_config_audio_pa(codecp, 0);    
    usleep(1000);    
    __codec_dac_set_unmute(codecp->codec, 0);
    __codec_dac_set_mute(codecp->codec, 1);
    
    _codec_analog_playPowerOff();  

    __codec_dac_set_fir(codecp->codec, 1);
    __codec_dac_disable(codecp->codec);
    __codec_dac_sw_reset(codecp->codec);
  }
#endif

#if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC){
    _codec_wm8753_set(WM8753_DAC, 3, 3, WM8753_DACMUTE);
    _codec_wm8753_set(WM8753_PWR3, 0, 8, 0);
    _codec_wm8753_set(WM8753_PWR4, 0, 8, 0);

    if(CODECD.rx_state == CODEC_STATE_STOP)
      g_bclk = 0;
  }
#endif
  __codec_set_bitsval(codecp->codec->CLK_CTRL_1, 3, 3, 0x0);
}

void _codec_set_record_volume(CODECDriver *cddp, void *arg)
{
  int32_t db = *(int32_t *)arg;
  uint32_t vol;

#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    vol = DB2VOL_ADC(db);
    if (vol > 149)
      vol = 149;

    __codec_adc_set_unmute(cddp->codec, 0);
    __codec_adc_set_mute(cddp->codec, 0);

    __codec_adc_set_volupdate(cddp->codec, 0);
    __codec_adc_set_leftvol(cddp->codec, vol);
    __codec_adc_set_rightvol(cddp->codec, vol);
    __codec_adc_set_volupdate(cddp->codec, 1);
  }
#endif

#if HS_CODEC_USE_WM8753
  (void)cddp;
  if(codec_sel == AUDIO_EXTERN_CODEC){
    vol = DB2VOL_ADC_WM(db);
    _codec_wm8753_set(WM8753_LADC, 0, 8, vol | 0x100);
    _codec_wm8753_set(WM8753_RADC, 0, 8, vol | 0x100);
  }
#endif
}

void _codec_get_record_volume_max(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  int *max = (int *)arg;

#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    if(cddp->pboardinfo->adc_max_gain > 24)
      *max = 24;
    else
      *max = cddp->pboardinfo->adc_max_gain;
  }
#endif

#if HS_CODEC_USE_WM8753
  (void)cddp;
  if(codec_sel == AUDIO_EXTERN_CODEC){
    *max = 30; /*0xff=+30dB*/
  }
#endif
}

void _codec_get_record_volume_min(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  int *min = (int *)arg;

#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    if(cddp->pboardinfo->adc_min_gain < -47)
      *min = -47;
    else
      *min = cddp->pboardinfo->adc_min_gain;
  }
#endif

#if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC){
    *min = -97 /*0x01=-97dB*/;
  }
#endif
}

void _codec_set_play_mute(CODECDriver *cddp, void *arg)
{
  (void)arg;
  (void)cddp;
  
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){

    _codec_config_audio_pa(cddp, 0);
    __codec_dac_set_unmute(cddp->codec, 0);
    __codec_dac_set_mute(cddp->codec, 1);
  }
#endif

#if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC){
    _codec_wm8753_set(WM8753_DAC, 3, 3, WM8753_DACMUTE);
  }
#endif
}

void _codec_set_play_unmute(CODECDriver *cddp, void *arg)
{
  (void)arg;
  (void)cddp;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    
    __codec_dac_set_unmute(cddp->codec, 1);
    __codec_dac_set_mute(cddp->codec, 0); 
    _codec_config_audio_pa(cddp, 1);
  }
#endif

#if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC){
    _codec_wm8753_set(WM8753_DAC, 3, 3, ~WM8753_DACMUTE);
  }
#endif
}

void _codec_set_play_volume(CODECDriver *cddp, void *arg)
{
  int32_t db = *(int32_t *)arg;
  uint32_t vol;

#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    
    vol = DB2VOL_DAC(db);
    if (vol > 149)
      vol = 149;

    __codec_dac_set_unmute(cddp->codec, 0);
    __codec_dac_set_mute(cddp->codec, 0);

    __codec_dac_set_volupdate(cddp->codec, 0);
    __codec_dac_set_leftvol(cddp->codec, vol);
    __codec_dac_set_rightvol(cddp->codec, vol);
    __codec_dac_set_volupdate(cddp->codec, 1);
  }
#endif

#if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC){
    (void)cddp;
    vol = DB2VOL_DAC_WM(db);
    _codec_wm8753_set(WM8753_LOUT1V, 0, 8, vol | 0x100);
    _codec_wm8753_set(WM8753_ROUT1V, 0, 8, vol | 0x100);
  }
#endif
}

void _codec_get_play_volume_max(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  int *max = (int *)arg;
  hs_audio_ply_src_t src = *max;

#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){

    if(cddp->pboardinfo->dac_max_gain > 26)
      *max = 26;
    else
      *max = cddp->pboardinfo->dac_max_gain;

    if(src == AUDIO_PLY_SRC_HFP)
      *max = cddp->pboardinfo->hfp_dacmax_gain;
  }
#endif

#if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC){
    (void)cddp;
    *max = 6; /*0xff=6dB*/
  }
#endif
}

void _codec_get_play_volume_min(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  int *min = (int *)arg;

#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    if(cddp->pboardinfo->dac_min_gain < -47)
      *min = -47;
    else
      *min = cddp->pboardinfo->dac_min_gain;
  }
#endif

#if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC){
    *min = -73; /*0x30=-73dB*/
  }
#endif
}

void _codec_set_codec_sel(CODECDriver *cddp, void *arg)
{
  (void)arg;
  (void)cddp;
  codec_sel = *(uint32_t *)arg;
}

void _codec_set_record_source(CODECDriver *cddp, void *arg)
{
  uint32_t source = *(uint32_t *)arg;
  HS_CODEC_Type *codec_r = cddp->codec;

  //__codec_set_bitsval(codec_r->IF_CTRL, 5, 5, 0x01);
  __codec_set_bitsval(codec_r->IF_CTRL, 4, 4, 0x00);
#if HS_CODEC_USE_WM8753
    if(codec_sel == AUDIO_EXTERN_CODEC){
      __codec_set_bitsval(codec_r->IF_CTRL, 6, 6, 0x01);
    }
#endif

  switch(source)
  {
  case AUDIO_RECORD_LINEIN:
#if HS_CODEC_USE_WM8753
    if(codec_sel == AUDIO_EXTERN_CODEC){
      _codec_wm8753_set(WM8753_ADCIN, 0, 3, 0x05);
      break;
    }
#endif
#if HS_CODEC_USE_INSIDE
    if(codec_sel == AUDIO_INSIDE_CODEC){
      __codec_set_bitsval(cddp->codec->IF_CTRL, 2, 3, 0);
      _codec_analog_pgaPowerOn(1, cddp->pboardinfo->pga_gain_aux);
      break;
    }
#endif
    break;

  case AUDIO_RECORD_DMIC:
#if HS_CODEC_USE_INSIDE
    if(codec_sel == AUDIO_INSIDE_CODEC){
      __codec_adc_dmic_enable(codec_r);
      break;
    }
#endif
#if HS_CODEC_USE_WM8753
    if(codec_sel == AUDIO_EXTERN_CODEC){
      break;
    }
#endif
    break;

  case AUDIO_RECORD_MIC:
#if HS_CODEC_USE_INSIDE
    if(codec_sel == AUDIO_INSIDE_CODEC){
      _codec_analog_pgaPowerOn(0, cddp->pboardinfo->pga_gain_mic);
      break;
    }
#endif
#if HS_CODEC_USE_WM8753
    if(codec_sel == AUDIO_EXTERN_CODEC){
      _codec_wm8753_set(WM8753_PWR1, 5, 5, MICB);
      _codec_wm8753_set(WM8753_PWR2, 8, 8, MICAMP1EN);
      _codec_wm8753_set(WM8753_PWR2, 4, 6, (ALCMIX << 2) | (PGAL << 1) | PGAR);

      _codec_wm8753_set(WM8753_ADCIN, 0, 3, 0);
      _codec_wm8753_set(WM8753_INCTL2, 1, 1, WM8753_MAC1ALC);
      _codec_wm8753_set(WM8753_LINVOL, 0, 8, 0x119);        //0x139);        //pga gain = 28.5dB
      _codec_wm8753_set(WM8753_RINVOL, 0, 8, 0x119);        //pga gain = 28.5dB

      //_codec_wm8753_set(WM8753_INCTL1, 0, 8, 0x60);      	//mic boost = 30dB
      //_codec_wm8753_set(WM8753_INCTL1, 5, 6, 3);
      _codec_wm8753_set(WM8753_ALC1, 0, 8, 0x1cb);//0x1fb);        //alc off
      //_codec_wm8753_set(WM8753_ALC3, 0, 7, 0x0);
      //_codec_wm8753_set(WM8753_ALC2, 4, 7, 6);

      //_codec_wm8753_set(WM8753_ADC, 0, 0, 0x1);

      _codec_wm8753_set(WM8753_MICBIAS, 0, 0, 1);           // Mic Bias Current Comparator Circuit enable
      break;
    }
#endif
    break;

  case AUDIO_RECORD_FM:
  #if HS_CODEC_USE_INSIDE
    if(codec_sel == AUDIO_INSIDE_CODEC){
      __codec_set_bitsval(codec_r->DAC_MOD_CTRL, 10, 10, 0x01);
      break;
    }
#endif

    break;

  default:
    break;
  }
}

void _codec_set_play_source(CODECDriver *cddp, void *arg)
{
  uint32_t source = *(uint32_t *)arg;
  HS_CODEC_Type *codec_r = cddp->codec;

  switch(source)
  {
  case AUDIO_PLAY_RAM:
    __codec_set_bitsval(codec_r->IF_CTRL, 4, 4, 0x00);
    //__codec_set_bitsval(codec_r->IF_CTRL, 6, 6, 0x01);
#if HS_CODEC_USE_WM8753
    if(codec_sel == AUDIO_EXTERN_CODEC){
      __codec_set_bitsval(codec_r->IF_CTRL, 5, 5, 0x01);

      _codec_wm8753_set(WM8753_BASS, 7, 7, 1);
      _codec_wm8753_set(WM8753_BASS, 4, 6, 0);
      _codec_wm8753_set(WM8753_BASS, 0, 3, 1);

      //_codec_wm8753_set(WM8753_TREBLE, 6, 6, 1);
      //_codec_wm8753_set(WM8753_TREBLE, 0, 3, 0xb);
    }
#endif

#if HS_CODEC_USE_INSIDE
    if(codec_sel == AUDIO_INSIDE_CODEC){
      __codec_set_bitsval(codec_r->DAC_MOD_CTRL, 9, 9, 0x00);  //fm_in_en = 0,disable codec play fm data(HS6600B1)
      pmu_ana_set(1, 1, 0);    //drv input = dac output
      break;
    }
#endif

    break;

  case AUDIO_PLAY_PCM:
    __codec_set_bitsval(codec_r->IF_CTRL, 8, 8, 0x00);
    __codec_set_bitsval(codec_r->IF_CTRL, 6, 6, 0x01);
    __codec_set_bitsval(codec_r->IF_CTRL, 4, 4, 0x01);
    break;

  case AUDIO_PLAY_LINEIN:
#if HS_CODEC_USE_INSIDE
    if(codec_sel == AUDIO_INSIDE_CODEC){
      _codec_config_audio_pa(cddp, 0);
      _codec_analog_pgaPowerOn(1, cddp->pboardinfo->pga_gain_aux);      
      pmu_ana_set(1, 1, 1);
      chThdSleepMilliseconds(300);
      _codec_config_audio_pa(cddp, 1);
      break;
    }
#endif
#if HS_CODEC_USE_WM8753
    if(codec_sel == AUDIO_EXTERN_CODEC){
      _codec_wm8753_set(WM8753_PWR3, 7, 8, (LOUT1 << 1) | ROUT1);
      _codec_wm8753_set(WM8753_PWR4, 0, 1, (LEFTMIX << 1)| RIGHTMIX);

      _codec_wm8753_set(WM8753_LOUTM1, 7, 7, LM2LO);
      _codec_wm8753_set(WM8753_ROUTM1, 7, 7, RM2RO);
      break;
    }
#endif
    break;

  case AUDIO_PLAY_FM:
    __codec_set_bitsval(codec_r->IF_CTRL, 4, 4, 0x00);
    __codec_set_bitsval(codec_r->IF_CTRL, 6, 6, 0x01);

#if HS_CODEC_USE_INSIDE
    if(codec_sel == AUDIO_INSIDE_CODEC){
      __codec_set_bitsval(codec_r->DAC_MOD_CTRL, 9, 9, 0x01);
      pmu_ana_set(1, 1, 0);
    }
#endif


#if HS_CODEC_USE_WM8753
    if(codec_sel == AUDIO_EXTERN_CODEC){
      __codec_set_bitsval(codec_r->IF_CTRL, 5, 5, 0x01);
      __codec_set_bitsval(codec_r->DAC_MOD_CTRL, 9, 9, 0x00);
      __codec_set_bitsval(codec_r->DAC_MOD_CTRL, 10, 10, 0x01);
    }
#endif

    __codec_set_bitsval(codec_r->CLK_CTRL_1, 0, 3, 0xf);

    __codec_set_bitsval(codec_r->ADC_CTRL, 1, 1, 0x01);
    __codec_set_bitsval(codec_r->ADC_CTRL, 14, 14, 0x01);
    __codec_set_bitsval(codec_r->ADC_CTRL, 10, 10, 0x00);
    __codec_set_bitsval(codec_r->ADC_CTRL, 8, 8, 0x01);
    __codec_set_bitsval(codec_r->ADC_CTRL, 0, 0, 0x01);

    __codec_set_bitsval(codec_r->DAC_CTRL, 6, 6, 0x01);
    __codec_set_bitsval(codec_r->DAC_CTRL, 0, 0, 0x01);
    __codec_set_bitsval(codec_r->DAC_CTRL, 2, 2, 0x00);
    __codec_set_bitsval(codec_r->DAC_CTRL, 8, 8, 0x01);
    chThdSleep(MS2ST(10));

    break;

  case AUDIO_PLAY_MIC:
#if HS_CODEC_USE_WM8753
    _codec_wm8753_set(WM8753_PWR1, 5, 5, MICB);
    _codec_wm8753_set(WM8753_PWR2, 8, 8, MICAMP1EN);

    _codec_wm8753_set(WM8753_INCTL1, 0, 8, 0x60);      //mic boost = 30dB
    _codec_wm8753_set(WM8753_INCTL2, 4, 5, 0x01);

    _codec_wm8753_set(WM8753_PWR1, 2, 3, (DACL << 1) | DACR);

    _codec_wm8753_set(WM8753_PWR3, 7, 8, (LOUT1 << 1) | ROUT1);
    _codec_wm8753_set(WM8753_PWR4, 0, 1, (LEFTMIX << 1)| RIGHTMIX);


    _codec_wm8753_set(WM8753_LOUTM1, 4, 6, 0);  //left mixer volume +6dB
    _codec_wm8753_set(WM8753_ROUTM1, 4, 6, 0);  //right mixer volume +6dB

    _codec_wm8753_set(WM8753_LOUTM2, 7, 7, 1);
    _codec_wm8753_set(WM8753_LOUTM2, 4, 6, 1);
    _codec_wm8753_set(WM8753_ROUTM2, 7, 7, 1);
    _codec_wm8753_set(WM8753_ROUTM2, 4, 6, 1);
#endif
    break;
  default:
    break;
  }
}

void _codec_set_record_sample(CODECDriver *cddp, void *arg)
{
  uint32_t sample = *(uint32_t *)arg;

  cddp->rx_cfg->sample_rate = (hs_i2s_sample_t)sample;
  _codec_set_samplerate(cddp, cddp->rx_cfg, AUDIO_STREAM_RECORD);

#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    _codec_set_deci_filt(cddp, sample);
  }
#endif

#if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC){
    _codec_wm8753_set_sample(sample);
  }
#endif
}

void _codec_set_play_sample(CODECDriver *cddp, void *arg)
{
  uint32_t sample = *(uint32_t *)arg;

  cddp->tx_cfg->sample_rate = (hs_i2s_sample_t)sample;
  _codec_set_samplerate(cddp, cddp->tx_cfg, AUDIO_STREAM_PLAYBACK);

#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    _codec_set_deci_filt(cddp, sample);
  }
#endif

#if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC){
    _codec_wm8753_set_sample(sample);
  }
#endif
}

void _codec_set_short_fir(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint32_t enable = *(uint32_t *)arg;
    HS_CODEC_Type *codec_r = cddp->codec;

    if(enable)
      __codec_dac_short_fir_enable(codec_r);
    else
      __codec_dac_short_fir_disable(codec_r);
  }
#endif

#if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC){
  }
#endif
}

void _codec_set_adc_drc_mode(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    //uint32_t mode = *(uint32_t *)arg;
    //HS_CODEC_Type *codec_r = cddp->codec;
    /* disable or enable if cfg permits */
    //if ((mode == 0) || (mode == (cddp->cfg_vol.adc_drc[2] & 0x3)))
    //  __codec_set_bitsval(codec_r->ADC_DRC_CTRL_3, 0, 1, mode);
  }
#endif
}

void _codec_set_adc_drc_limiter(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint32_t limiter = *(uint32_t *)arg;
    HS_CODEC_Type *codec_r = cddp->codec;
    uint32_t ct;

    ct = (uint32_t)((-6 + 1) * 2);
    __codec_set_bitsval(codec_r->ADC_DRC_CTRL_3, 8, 14, -ct);	//ct
    __codec_set_bitsval(codec_r->ADC_DRC_CTRL_3, 16, 22, -ct);	//et
    __codec_set_bitsval(codec_r->ADC_DRC_CTRL_3, 2, 6, -(limiter + 1));	//lt
    __codec_set_bitsval(codec_r->ADC_DRC_CTRL_4, 0, 2, 4);	//cs
    __codec_set_bitsval(codec_r->ADC_DRC_CTRL_4, 3, 5, 5);	//es

    __codec_set_bitsval(codec_r->ADC_DRC_CTRL_5, 16, 16, 0);

    /*sample rate = 441000***/
    codec_r->ADC_DRC_CTRL_1 = 0x04270c75;	//at0,at1
    codec_r->ADC_DRC_CTRL_2 = 0x0046ff2f;	//rt0,rt1
  }
#endif
}

void _codec_set_adc_drc_agc(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    HS_CODEC_Type *codec_r = cddp->codec;
    uint32_t ct;

    ct = (uint32_t)((-6 + 1) * 2);
    __codec_set_bitsval(codec_r->ADC_DRC_CTRL_3, 8, 14, -ct);	//ct
    __codec_set_bitsval(codec_r->ADC_DRC_CTRL_3, 16, 22, -ct);	//et
    __codec_set_bitsval(codec_r->ADC_DRC_CTRL_3, 2, 6, 0);	//lt
    __codec_set_bitsval(codec_r->ADC_DRC_CTRL_4, 0, 2, 5);	//cs
    __codec_set_bitsval(codec_r->ADC_DRC_CTRL_4, 3, 5, 0);	//es

    __codec_set_bitsval(codec_r->ADC_DRC_CTRL_4, 9, 9, 1);	//es_inv

    __codec_set_bitsval(codec_r->ADC_DRC_CTRL_5, 16, 16, 0);

    /*sample rate = 441000***/
    codec_r->ADC_DRC_CTRL_1 = 0x04270c75;	//at0,at1
    codec_r->ADC_DRC_CTRL_2 = 0x0046ff2f;	//rt0,rt1
  }
#endif
}

void _codec_set_dac_drc_mode(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    //uint32_t mode = *(uint32_t *)arg;
    //HS_CODEC_Type *codec_r = cddp->codec;
    /* disable or enable if cfg permits */
    //if ((mode == 0) || (mode == (cddp->cfg_vol.dac_drc[2] & 0x3)))
    //  __codec_set_bitsval(codec_r->DAC_DRC_CTRL_3, 0, 1, mode);
  }
#endif
}

void _codec_set_dac_drc_limiter(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint32_t limiter = *(uint32_t *)arg;
    HS_CODEC_Type *codec_r = cddp->codec;
    uint32_t ct;

    ct = (uint32_t)((-6 + 1) * 2);
    __codec_set_bitsval(codec_r->DAC_DRC_CTRL_3, 8, 14, -ct);	//ct
    __codec_set_bitsval(codec_r->DAC_DRC_CTRL_3, 16, 22, -ct);	//et
    __codec_set_bitsval(codec_r->DAC_DRC_CTRL_3, 2, 6, -(limiter + 1));	//lt
    __codec_set_bitsval(codec_r->DAC_DRC_CTRL_4, 0, 2, 4);	//cs
    __codec_set_bitsval(codec_r->DAC_DRC_CTRL_4, 3, 5, 5);	//es

    /*sample rate = 441000***/
    codec_r->DAC_DRC_CTRL_1 = 0x04270c75;	//at0,at1
    codec_r->DAC_DRC_CTRL_2 = 0x0046ff2f;	//rt0,rt1
  }
#endif
}

void _codec_set_dac_drc_agc(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    HS_CODEC_Type *codec_r = cddp->codec;
    uint32_t ct;

    ct = (uint32_t)((-6 + 1) * 2);
    __codec_set_bitsval(codec_r->DAC_DRC_CTRL_3, 8, 14, -ct);	//ct
    __codec_set_bitsval(codec_r->DAC_DRC_CTRL_3, 16, 22, -ct);	//et
    __codec_set_bitsval(codec_r->DAC_DRC_CTRL_3, 2, 6, 0);	//lt
    __codec_set_bitsval(codec_r->DAC_DRC_CTRL_4, 0, 2, 5);	//cs
    __codec_set_bitsval(codec_r->DAC_DRC_CTRL_4, 3, 5, 0);	//es

    __codec_set_bitsval(codec_r->DAC_DRC_CTRL_4, 9, 9, 1);	//es_inv
    __codec_set_bitsval(codec_r->DAC_DRC_CTRL_5, 16, 16, 0);

    /*sample rate = 441000***/
    codec_r->DAC_DRC_CTRL_1 = 0x04270c75;	//at0,at1
    codec_r->DAC_DRC_CTRL_2 = 0x0046ff2f;	//rt0,rt1
  }
#endif
}

void _codec_set_adc_mix(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint32_t enable = *(uint32_t *)arg;

    HS_CODEC_Type *codec_r = cddp->codec;
    __codec_set_bitsval(codec_r->ADC_VOL_CTRL, 3, 3, enable);
  }
#endif
}

void _codec_set_dac_mix(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint32_t enable = *(uint32_t *)arg;

    HS_CODEC_Type *codec_r = cddp->codec;
    __codec_set_bitsval(codec_r->DAC_VOL_CTRL, 3, 3, enable);
  }
#endif
}

void _codec_set_dac_mode(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint32_t mode = *(uint32_t *)arg;

    HS_CODEC_Type *codec_r = cddp->codec;
    codec_r->DAC_MOD_CTRL = mode;
  }
#endif
}

/**
* @brief   i2s input track sel
*
* @param[in] codecp      pointer to the @p CODEDriver object
* @param[in] *arg        00-LR 01-RL 02-LL 03-RR
*
* @notapi
*/
void _codec_invert_i2s_input(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    //uint32_t sel = *(uint32_t *)arg;

    HS_CODEC_Type *codec_r = cddp->codec;
    __codec_set_bitsval(codec_r->IF_CTRL, 2, 3, cddp->pboardinfo->mic_sel);
  }
#endif
}

/**
* @brief   i2s output track sel
*
* @param[in] codecp      pointer to the @p CODEDriver object
* @param[in] *arg        00-LR 01-RL 02-LL 03-RR
*
* @notapi
*/
void _codec_invert_i2s_output(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  uint32_t sel = *(uint32_t *)arg;

#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    HS_CODEC_Type *codec_r = cddp->codec;
    /*
     * dynamic settings before play mono or stereo
     * LR: output I2S's L&R at PCB's L&R, used for stereo
     * RL: output I2S's L&R at PCB's R&L, unused
     * LL: output I2S's L&L at PCB's L&R, used for mono
     * RR: output I2S's R&R at PCB's L&R, unused
     */
    if(sel == 0)
    {
      __codec_set_bitsval(codec_r->IF_CTRL, 0, 1, 0);
    }
    else if(sel == 1)
    {
      __codec_set_bitsval(codec_r->IF_CTRL, 0, 1, 3);
    }
    else if(sel == 2)
    {
      __codec_set_bitsval(codec_r->IF_CTRL, 0, 1, 2);
    }
    else
    {
      __codec_set_bitsval(codec_r->IF_CTRL, 0, 1, 1);
    }
  }
#endif

#if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC){
    if(sel == 0)
    {
      _codec_wm8753_set(WM8753_DAC, 4, 5, 0);
    }
    else if(sel == 2 || sel == 3)
    {
      _codec_wm8753_set(WM8753_DAC, 4, 5, 3);
    }
  }
#endif
}

void _codec_set_eq(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint32_t enable = *(uint32_t *)arg;

    HS_CODEC_Type *codec_r = cddp->codec;
    uint32_t bypass = enable == 0? 1:0;
    __codec_set_bitsval(codec_r->DAC_EQ_CTRL_1, 24, 24, bypass);
  }
#endif
}

void _codec_set_band1_gain(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint8_t gain = *(int8_t *)arg + 12;
    HS_CODEC_Type *codec_r = cddp->codec;
    __codec_set_bitsval(codec_r->DAC_EQ_CTRL_1, 16, 20, gain);
  }
#endif
}

void _codec_set_band1_coeff(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint16_t coeff = *(uint16_t *)arg;
    HS_CODEC_Type *codec_r = cddp->codec;

    __codec_set_bitsval(codec_r->DAC_EQ_CTRL_1, 0, 15, coeff);
  }
#endif
}

void _codec_set_band2_gain(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint8_t gain = *(int8_t *)arg + 12;
    HS_CODEC_Type *codec_r = cddp->codec;
    __codec_set_bitsval(codec_r->DAC_EQ_CTRL_8, 0, 4, gain);
  }
#endif
}

void _codec_set_band2_coeff(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint32_t coeff = *(uint32_t *)arg;
    HS_CODEC_Type *codec_r = cddp->codec;

    __codec_set_bitsval(codec_r->DAC_EQ_CTRL_2, 0, 15, coeff >> 16);
    __codec_set_bitsval(codec_r->DAC_EQ_CTRL_2, 16, 31, coeff & 0xFFFF);
  }
#endif
}

void _codec_set_band3_gain(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint8_t gain = *(int8_t *)arg + 12;
    HS_CODEC_Type *codec_r = cddp->codec;
    __codec_set_bitsval(codec_r->DAC_EQ_CTRL_8, 8, 12, gain);
  }
#endif
}

void _codec_set_band3_coeff(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint32_t coeff = *(uint32_t *)arg;
    HS_CODEC_Type *codec_r = cddp->codec;

    __codec_set_bitsval(codec_r->DAC_EQ_CTRL_3, 0, 15, coeff >> 16);
    __codec_set_bitsval(codec_r->DAC_EQ_CTRL_3, 16, 31, coeff & 0xFFFF);
  }
#endif
}

void _codec_set_band4_gain(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint8_t gain = *(int8_t *)arg + 12;
    HS_CODEC_Type *codec_r = cddp->codec;
    __codec_set_bitsval(codec_r->DAC_EQ_CTRL_8, 16, 20, gain);
  }
#endif
}

void _codec_set_band4_coeff(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint32_t coeff = *(uint32_t *)arg;
    HS_CODEC_Type *codec_r = cddp->codec;

    __codec_set_bitsval(codec_r->DAC_EQ_CTRL_4, 0, 15, coeff >> 16);
    __codec_set_bitsval(codec_r->DAC_EQ_CTRL_4, 16, 31, coeff & 0xFFFF);
  }
#endif
}

void _codec_set_band5_gain(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint8_t gain = *(int8_t *)arg + 12;
    HS_CODEC_Type *codec_r = cddp->codec;
    __codec_set_bitsval(codec_r->DAC_EQ_CTRL_8, 24, 28, gain);
  }
#endif
}

void _codec_set_band5_coeff(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint32_t coeff = *(uint32_t *)arg;
    HS_CODEC_Type *codec_r = cddp->codec;
    __codec_set_bitsval(codec_r->DAC_EQ_CTRL_5, 0, 15, coeff >> 16);
    __codec_set_bitsval(codec_r->DAC_EQ_CTRL_5, 16, 31, coeff & 0xFFFF);
  }
#endif
}

void _codec_set_band6_gain(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint8_t gain = *(int8_t *)arg + 12;
    HS_CODEC_Type *codec_r = cddp->codec;
    __codec_set_bitsval(codec_r->DAC_EQ_CTRL_7, 16, 20, gain);
  }
#endif
}

void _codec_set_band6_coeff(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint32_t coeff = *(uint32_t *)arg;
    HS_CODEC_Type *codec_r = cddp->codec;

    __codec_set_bitsval(codec_r->DAC_EQ_CTRL_6, 0, 15, coeff >> 16);
    __codec_set_bitsval(codec_r->DAC_EQ_CTRL_6, 16, 31, coeff & 0xFFFF);
  }
#endif
}

void _codec_set_band7_gain(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint8_t gain = *(int8_t *)arg + 12;
    HS_CODEC_Type *codec_r = cddp->codec;
    __codec_set_bitsval(codec_r->DAC_EQ_CTRL_7, 24, 28, gain);
  }
#endif
}

void _codec_set_band7_coeff(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint32_t coeff = *(uint16_t *)arg;
    HS_CODEC_Type *codec_r = cddp->codec;

    __codec_set_bitsval(codec_r->DAC_EQ_CTRL_7, 0, 15, coeff);
  }
#endif
}

void _codec_set_test_mode(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint8_t mode = *(uint8_t *)arg;
    HS_CODEC_Type *codec_r = cddp->codec;

    __codec_set_bitsval(codec_r->TEST_MODE, 0, 4, mode);
  }
#endif
}

void _codec_set_i2s_conn_ctrl(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    uint8_t enable = *(uint8_t *)arg;
    HS_CODEC_Type *codec_r = cddp->codec;

    __codec_set_bitsval(codec_r->IF_CTRL, 7, 7, enable);
  }
#endif
}

void _codec_get_rms(CODECDriver *cddp, void *arg)
{
  (void)cddp;
  (void)arg;
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    int *rms = (int *)arg;
    HS_CODEC_Type *codec_r = cddp->codec;    

    *rms = (((codec_r->DAC_PEAK_READ >> 16) & 0xffff) + (codec_r->DAC_PEAK_READ & 0xffff)) / 2;
    //*rms = codec_r->DAC_PEAK_READ & 0xffff;
  }
#endif
}

void _codec_aec_delay_measure_init(CODECDriver *codecp, void *arg)
{
  int threshold = *(int *)arg;
  __codec_set_bitsval(codecp->codec->AEC_DELAY_CFG, 0, 15, threshold);

#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC)
  {
    // disable gain gradually increased from 0 and gain gradually decreased to 0
    __codec_set_bitsval(codecp->codec->ADC_VOL_CTRL, 0, 2, 0x04);
    __codec_set_bitsval(codecp->codec->DAC_VOL_CTRL, 0, 2, 0x04);
  }
#endif

#if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC){
    __codec_set_bitsval(codecp->codec->TEST_MODE, 0, 4, 0x10);

    codec_sel = AUDIO_INSIDE_CODEC;
    _codec_set_samplerate(codecp, codecp->tx_cfg, AUDIO_STREAM_PLAYBACK);
    codec_sel = AUDIO_EXTERN_CODEC;
    __codec_dac_set_fir(codecp->codec, 1);
    __codec_dac_set_unmute(codecp->codec, 1);
    __codec_dac_set_mute(codecp->codec, 0);

    __codec_dac_sw_reset(codecp->codec);
    __codec_dac_enable(codecp->codec);

    //chThdSleepS(US2ST(10));
    usleep(10);                                                 //wait 10us
    __codec_dac_set_fir(codecp->codec, 0);
    __codec_set_bitsval(codecp->codec->CLK_CTRL_1, 3, 3, 0x1);

    codec_sel = AUDIO_INSIDE_CODEC;
    _codec_set_samplerate(codecp, codecp->rx_cfg, AUDIO_STREAM_RECORD);
    codec_sel = AUDIO_EXTERN_CODEC;
    __codec_adc_set_unmute(codecp->codec, 1);
    __codec_adc_set_mute(codecp->codec, 0);
    __codec_adc_sw_reset(codecp->codec);
    __codec_adc_enable(codecp->codec);
    __codec_set_bitsval(codecp->codec->CLK_CTRL_1, 2, 2, 0x1);
  }
#endif
}

void _codec_aec_get_delay(CODECDriver *cddp, void *arg)
{
  int *pDelay = (int *)arg;
  HS_CODEC_Type *codec_r = cddp->codec;

 *pDelay = codec_r->AEC_DELAY_CFG >> 16;
}

void _codec_setDacPeakDetection(CODECDriver *codecp, void *arg)
{
  (void)codecp;
  (void)arg;
  #if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC)
  {
    uint8_t enable = *((uint8_t *)arg);
    __codec_set_bitsval(codecp->codec->DAC_MOD_CTRL, 8, 8, enable);
    if(enable != 0){
      __codec_set_bitsval(codecp->codec->DAC_DRC_CTRL_3, 0, 1, 1);
      __codec_set_bitsval(codecp->codec->DAC_DRC_CTRL_3, 28, 31, 0);
      __codec_set_bitsval(codecp->codec->DAC_DRC_CTRL_4, 16, 31, (uint32_t)0xB7B7);
    }
  }
  #endif
}

static hs_codec_ctrl_t g_ctrl_method[] =
{
  {AUDIO_RECORD_MUTE,             _codec_set_record_mute},
  {AUDIO_RECORD_UNMUTE,           _codec_set_record_unmute},
  {AUDIO_RECORD_SET_VOLUME,       _codec_set_record_volume},
  {AUDIO_RECORD_GET_VOLUME_MAX,   _codec_get_record_volume_max},
  {AUDIO_RECORD_GET_VOLUME_MIN,   _codec_get_record_volume_min},
  {AUDIO_RECORD_SET_SAMPLE, 		  _codec_set_record_sample},
  {AUDIO_PLAY_MUTE,               _codec_set_play_mute},
  {AUDIO_PLAY_UNMUTE,             _codec_set_play_unmute},
  {AUDIO_PLAY_SET_VOLUME,         _codec_set_play_volume},
  {AUDIO_PLAY_GET_VOLUME_MAX,     _codec_get_play_volume_max},
  {AUDIO_PLAY_GET_VOLUME_MIN,     _codec_get_play_volume_min},
  {AUDIO_PLAY_SET_SAMPLE, 		    _codec_set_play_sample},
  //{AUDIO_SET_INPUT_SOURCE, 		    _codec_set_input_source},
  {AUDIO_SET_CODEC_SEL,           _codec_set_codec_sel},
  {AUDIO_SET_RECORD_SOURCE,       _codec_set_record_source},
  {AUDIO_SET_PLAY_SOURCE,         _codec_set_play_source},
  {AUDIO_SET_SHORT_FIR,						_codec_set_short_fir},
  {AUDIO_SET_ADC_DRC_MODE,				_codec_set_adc_drc_mode},
  {AUDIO_SET_ADC_DRC_LIMITER,			_codec_set_adc_drc_limiter},
  {AUDIO_SET_ADC_DRC_AGC,					_codec_set_adc_drc_agc},
  {AUDIO_SET_DAC_DRC_MODE,				_codec_set_dac_drc_mode},
  {AUDIO_SET_DAC_DRC_LIMITER,			_codec_set_dac_drc_limiter},
  {AUDIO_SET_DAC_DRC_AGC,					_codec_set_dac_drc_agc},
  {AUDIO_SET_ADC_MIX,							_codec_set_adc_mix},
  {AUDIO_SET_DAC_MIX,							_codec_set_dac_mix},
  {AUDIO_SET_DAC_MODE,						_codec_set_dac_mode},
  {AUDIO_INVERT_I2S_INPUT,				_codec_invert_i2s_input},
  {AUDIO_INVERT_I2S_OUTPUT,				_codec_invert_i2s_output},
  {AUDIO_SET_EQ,				          _codec_set_eq},
  {AUDIO_SET_BAND1_GAIN,				  _codec_set_band1_gain},
  {AUDIO_SET_BAND1_COEFF,				  _codec_set_band1_coeff},
  {AUDIO_SET_BAND2_GAIN,				  _codec_set_band2_gain},
  {AUDIO_SET_BAND2_COEFF,				  _codec_set_band2_coeff},
  {AUDIO_SET_BAND3_GAIN,				  _codec_set_band3_gain},
  {AUDIO_SET_BAND3_COEFF,				  _codec_set_band3_coeff},
  {AUDIO_SET_BAND4_GAIN,				  _codec_set_band4_gain},
  {AUDIO_SET_BAND4_COEFF,				  _codec_set_band4_coeff},
  {AUDIO_SET_BAND5_GAIN,				  _codec_set_band5_gain},
  {AUDIO_SET_BAND5_COEFF,				  _codec_set_band5_coeff},
  {AUDIO_SET_BAND6_GAIN,				  _codec_set_band6_gain},
  {AUDIO_SET_BAND6_COEFF,				  _codec_set_band6_coeff},
  {AUDIO_SET_BAND7_GAIN,				  _codec_set_band7_gain},
  {AUDIO_SET_BAND7_COEFF,				  _codec_set_band7_coeff},
  {AUDIO_SET_TEST_MODE,           _codec_set_test_mode},
  {AUDIO_SET_I2S_CONN_CTRL,       _codec_set_i2s_conn_ctrl},
  {AUDIO_GET_DAC_RMS,             _codec_get_rms},
  {AUDIO_AEC_DELAY_MEASURE_INIT,  _codec_aec_delay_measure_init},
  {AUDIO_GET_AEC_DELAY,           _codec_aec_get_delay},
  {AUDIO_SET_DAC_PEAK,            _codec_setDacPeakDetection},
};

static uint32_t g_method_cnt = sizeof(g_ctrl_method) / sizeof(hs_codec_ctrl_t);


/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
* @brief   Low level CODEC driver initialization.
*
* @notapi
*/
void codec_lld_init(void)
{
#if HS_CODEC_USE_DRV

  CODECD.codec = HS_CODEC;
  CODECD.rx_state = CODEC_STATE_IDLE;
  CODECD.tx_state = CODEC_STATE_IDLE;

  CODECD.pboardinfo = hs_boardGetCodecInfo();

#endif /* HS_CODEC_USE_DRV */
}

/**
* @brief   Configures and activates the codec peripheral.
*
* @param[in] codecp      pointer to the @p CODECDriver object
*
* @notapi
*/
void codec_lld_start(CODECDriver *codecp)
{
  HS_CODEC_Type *codec_r = codecp->codec;

  cpmResetCODEC();
  cpmEnableCODEC();
  CODECD.mclk = cpm_get_clock(HS_CODEC_MCLK);

  __codec_set_bitsval(codec_r->CLK_CTRL_1, 0, 1, 0x3);
  __codec_set_bitsval(codec_r->CLK_CTRL_2, 0, 3, 0xf);

#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){  

    _codec_config_audio_pa(codecp, 0);

    #if 1
    _codec_analog_commonPowerOn();
    codec_lld_micbias_calibration();
    #else
    __codec_dac_set_bypass(codecp->codec, 0);
    __codec_dac_set_fir(codecp->codec, 1);

    __codec_dac_sw_reset(codecp->codec);
    __codec_dac_enable(codecp->codec);
    
    usleep(10);                                                 //wait 10us
    __codec_dac_set_fir(codecp->codec, 0);

    _codec_analog_playPowerOn(codecp->pboardinfo->drv_single);
    
    codec_lld_micbias_calibration();
    #endif

    _codec_analog_drvPowerOn();
    __codec_dac_enable(codecp->codec);
    if(codecp->pboardinfo->drv_single)
      pmu_ana_set(299, 302, 0xd);     
    else
      pmu_ana_set(299, 302, 0); 
    
    __codec_set_bitsval(codec_r->DAC_VOL_CTRL, 4, 7, 0);
    __codec_set_bitsval(codec_r->ADC_VOL_CTRL, 4, 7, 0);   

    pmu_ana_set(303, 303, codecp->pboardinfo->sel_pkg);
  }
#endif

#if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC){

    _codec_wm8753_set(WM8753_RESET, 0, 8, 0);
    _codec_wm8753_set(WM8753_PWR1, 6, 8, (VMIDSEL_50K << 1) | VREF);
  }
#endif

  __codec_set_bitsval(codec_r->IF_CTRL, 7, 7, codec_sel);

  CODECD.rx_state = CODEC_STATE_STOP;
  CODECD.tx_state = CODEC_STATE_STOP;
}

/**
* @brief   Deactivates the I2S peripheral.
*
* @param[in] i2sp      pointer to the @p I2SDriver object
*
* @notapi
*/
void codec_lld_stop(CODECDriver *codecp)
{
#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    codecp->codec->IF_CTRL = 0x00;

    _codec_config_audio_pa(codecp, 0);
    _codec_analog_drvPowerOff();
    _codec_analog_commonPowerOff();
  }
#endif

#if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC){
    g_bclk = 0;
    _codec_wm8753_set(WM8753_PWR1, 0, 8, 0x00);
    _codec_wm8753_set(WM8753_PWR2, 0, 8, 0x00);
    _codec_wm8753_set(WM8753_PWR3, 0, 8, 0x00);
    _codec_wm8753_set(WM8753_PWR4, 0, 8, 0x00);
  }
#endif

  __codec_set_bitsval(codecp->codec->CLK_CTRL_2, 0, 3, 0);
  __codec_set_bitsval(codecp->codec->CLK_CTRL_1, 0, 1, 0);
  nvicDisableVector(IRQ_CODEC);
  cpmDisableCODEC();

  CODECD.rx_state = CODEC_STATE_STOP;
  CODECD.tx_state = CODEC_STATE_STOP;

}

void codec_lld_record_start(CODECDriver *codecp, hs_audio_config_t *cfgp)
{
  int db;
  
  /* config i2s clock */
  _codec_set_samplerate(codecp, cfgp, AUDIO_STREAM_RECORD);
  codecp->rx_cfg = cfgp;

#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    __codec_set_bitsval(codecp->codec->ADC_CTRL, 4, 6, 3);
    __codec_set_bitsval(codecp->codec->ADC_CTRL, 3, 3, 1);
    __codec_set_bitsval(codecp->codec->IF_CTRL, 2, 3, codecp->pboardinfo->mic_sel);

    db = codecp->pboardinfo->adc_default_gain;
    _codec_set_record_volume(codecp, (void *)&db);
    
    //mute
    __codec_adc_set_bypass(codecp->codec, 0);
    __codec_adc_set_unmute(codecp->codec, 0);
    __codec_adc_set_mute(codecp->codec, 1);

    _codec_analog_recPowerOn(codecp->pboardinfo);

    __codec_adc_set_unmute(codecp->codec, 1);
    __codec_adc_set_mute(codecp->codec, 0);

    __codec_adc_sw_reset(codecp->codec);
    __codec_adc_enable(codecp->codec);

    __codec_set_bitsval(codecp->codec->ADC_VOL_CTRL, 3, 3, codecp->pboardinfo->adc_mix_en);
  }
#endif

#if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC){
    _codec_wm8753_set_sample(cfgp->sample_rate);
    _codec_wm8753_set(WM8753_PWR2, 2, 3, (ADCL << 1) | ADCR);
    _codec_wm8753_set(WM8753_IOCTL, 0, 8, 0x2c);
    _codec_wm8753_set(WM8753_SRATE2, 0, 8, 0x05);


    if(codecp->tx_cfg->sample_width == I2S_BITWIDTH_24BIT){
      _codec_wm8753_set(WM8753_PCM,  0, 3, (WM8753_WL_24 << 2) | WM8753_FT_I2S | (1 << 4));
      _codec_wm8753_set(WM8753_HIFI, 0, 3, (WM8753_WL_24 << 2) | WM8753_FT_I2S);
    }
    else{
      _codec_wm8753_set(WM8753_PCM,  0, 3, (WM8753_WL_16 << 2) | WM8753_FT_I2S | (1 << 4));
      _codec_wm8753_set(WM8753_HIFI, 0, 3, (WM8753_WL_16 << 2) | WM8753_FT_I2S);
    }
  }
#endif

  __codec_set_bitsval(codecp->codec->CLK_CTRL_1, 2, 2, 0x1);
  CODECD.rx_state = CODEC_STATE_WORKING;
}

void codec_lld_record_stop(CODECDriver *codecp)
{
  CODECD.rx_state = CODEC_STATE_STOP;
  codecp->rx_cfg = NULL;

#if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC){
    HS_CODEC_Type *pCod = codecp->codec;

    __codec_adc_set_unmute(pCod, 0);
    __codec_adc_set_mute(pCod, 1);

    _codec_analog_recPowerOff();

    __codec_adc_disable(pCod);
    __codec_adc_sw_reset(pCod);

  }
#endif

#if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC){
    _codec_wm8753_set(WM8753_PWR2, 0, 8, 0);

    if(CODECD.tx_state == CODEC_STATE_STOP)
      g_bclk = 0;
  }
#endif
    __codec_set_bitsval(codecp->codec->CLK_CTRL_1, 2, 2, 0x0);
}

void codec_lld_play_start(CODECDriver *codecp, hs_audio_config_t *cfgp)
{
  /* config i2s clock */
  _codec_set_samplerate(codecp, cfgp, AUDIO_STREAM_PLAYBACK);
  codecp->tx_cfg = cfgp;
  __codec_set_bitsval(codecp->codec->TEST_MODE, 0, 4, 0);
  __codec_set_bitval(codecp->codec->ADC_SIDE_CTRL, 16, 0);
  __codec_set_bitsval(codecp->codec->DAC_MOD_CTRL, 6, 7, 3);
  pmu_ana_set(36, 36, codecp->pboardinfo->drv_gain);
  _codec_enableDrc(codecp);
  
  codec_lld_start_dac_drv(codecp);

  __codec_set_bitsval(codecp->codec->DAC_VOL_CTRL, 3, 3, codecp->pboardinfo->dac_mix_en);
  CODECD.tx_state = CODEC_STATE_WORKING;
}

void codec_lld_play_stop(CODECDriver *codecp)
{
  CODECD.tx_state = CODEC_STATE_STOP;
  codecp->tx_cfg  = NULL;
  __codec_set_bitsval(codecp->codec->DAC_MOD_CTRL, 6, 7, 0);
  __codec_set_bitval(codecp->codec->ADC_SIDE_CTRL, 16, 0);
  
  codec_lld_stop_dac_drv(codecp);
}

void codec_lld_ctrl(CODECDriver *codecp, hs_audio_ctrltype_t ctype, void *arg)
{
  uint32_t i;

  for(i = 0; i < g_method_cnt; i++)
  {
    if ((g_ctrl_method[i].type == ctype)
        && (g_ctrl_method[i].fn_ctrl_method != NULL))
    {
      g_ctrl_method[i].fn_ctrl_method(codecp, arg);
      return ;
    }
  }
}

void codec_setI2sRxClk(CODECDriver *codecp, uint8_t val)
{
  #if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC)
  {
    __codec_set_bitsval(codecp->codec->IF_CTRL, 6, 6, val);
  }
  #endif

  #if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC)
  {
    __codec_set_bitsval(codecp->codec->IF_CTRL, 6, 6, val);
  }
  #endif
}

void codec_setI2sTxClk(CODECDriver *codecp, uint8_t val)
{
  #if HS_CODEC_USE_INSIDE
  if(codec_sel == AUDIO_INSIDE_CODEC)
  {
    __codec_set_bitsval(codecp->codec->IF_CTRL, 5, 5, val);
  }
  #endif

  #if HS_CODEC_USE_WM8753
  if(codec_sel == AUDIO_EXTERN_CODEC)
  {
    __codec_set_bitsval(codecp->codec->IF_CTRL, 5, 5, val);
  }
  #endif
}

/*
 * set PGA gain in value
 * @pga_index: 000=-6dB;001=-3dB; 010=0dB;011=3dB;100=6dB;101=12dB;110=18dB;111=24dB
 * @return:    the gain in db
 */
int32_t codec_analog_set_pga_gain(int32_t pga_idx)
{
  int gain[8] = {-6, -3, 0, 3, 6, 12, 18, 24};

  pga_idx = pga_idx > PGA_GAIN_MAXIDX ? PGA_GAIN_MAXIDX : pga_idx;
  __codec_set_bitsval(HS_ANA->COMMON_PACK[0], 10, 12, pga_idx);
  
  return gain[pga_idx];
}

uint32_t codec_lld_getStatus(CODECDriver *codecp)
{
  HS_CODEC_Type *codec_r = codecp->codec;
  uint32_t status = codec_r->INT_STATUS;

  codec_r->INT_STATUS = status;
  return status;
}

void codec_lld_setBTCfg(void)
{
  //_codec_config_audio_pa(&CODECD, 0);
  //pmu_ana_set(21, 22, 1);
  //pmu_ana_set(19, 20, 0);
  //pmu_ana_set(36, 36, 1);
  //pmu_ana_set(37, 40, 6);

  //usleep(500);
}

void codec_lld_setMixer(uint8_t vol)
{
  HS_CODEC_Type *codec_r = CODECD.codec;
  uint32_t val = 0x10000;

  val |= vol << 8;
  val |= vol;

  codec_r->ADC_SIDE_CTRL = val;
}

int codec_lld_isMixerEn(void)
{
  HS_CODEC_Type *codec_r = CODECD.codec;
  int val = codec_r->ADC_SIDE_CTRL;

  val >>= 16;
  val &= 0x1;
  
  return val;
}


#endif /* HAL_USE_CODEC */

/** @} */
