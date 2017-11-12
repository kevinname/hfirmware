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
 * @file    hs66xx/codec_lld.h
 * @brief   audio interface Driver subsystem low level driver header template.
 *
 * @addtogroup codec
 * @{
 */

#ifndef _CODEC_LLD_H_
#define _CODEC_LLD_H_

#if HAL_USE_CODEC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/
#define EQ_BAND_NUM     7


/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

enum
{
  I2S_BCLK_3MHZ     = 0,
  I2S_BCLK_6MHZ     ,
  I2S_BCLK_12MHZ    ,
  I2S_BCLK_0P6MHZ   ,
  I2S_BCLK_1P2MHZ   ,
  I2S_BCLK_2P4MHZ   ,
  I2S_BCLK_4P8MHZ   ,
};

#define CLOCK_DIV_EN     (1<<0)

/**
 * @brief   CODEC interrupt priority level setting.
 */
#if !defined(HS_CODEC_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define HS_CODEC_IRQ_PRIORITY    3
#endif

#if HS_CODEC_USE_WM8753
/* DAC control */
#define WM8753_DACMUTE      1

/* digital hi-fi audio interface format */
#define WM8753_WL_32        3
#define WM8753_WL_24        2
#define WM8753_WL_20        1
#define WM8753_WL_16        0

#define WM8753_FT_DSP       (3 << 0)
#define WM8753_FT_I2S       (2 << 0)
#define WM8753_FT_LEFT      (1 << 0)
#define WM8753_FT_RIGHT     (0 << 0)

/* power management 1*/
#define VMIDSEL_DISABLED    0
#define VMIDSEL_50K         1
#define VMIDSEL_500K        2
#define VMIDSEL_5K          3

/* power management 2 */
#define MICAMP1EN           1
#define MICAMP2EN           1
#define ALCMIX              1
#define PGAL                1
#define PGAR                1
#define ADCL                1
#define ADCR                1
#define RXMIX               1
#define LINEMIX             1

/* power management 3*/
#define LOUT1               1
#define ROUT1               1

/* power management 4 */
#define RIGHTMIX            1
#define LEFTMIX             1

#define VREF                1
#define MICB                1
#define VDAC                1
#define DACL                1
#define DACR                1
#define DIGENB              0

/* clock inputs */
#define WM8753_MCLK         0
#define WM8753_PCMCLK       1

/* clock divider id's */
#define WM8753_PCMDIV       0
#define WM8753_BCLKDIV      1
#define WM8753_VXCLKDIV     2

/* left mixer control 1 */
#define LD2LO               1
#define LM2LO               1

/* left output mixer control 1 */
#define RD2RO               1
#define RM2RO               1

/* PCM clock dividers */
#define WM8753_PCM_DIV_1    (0 << 6)
#define WM8753_PCM_DIV_3    (2 << 6)
#define WM8753_PCM_DIV_5_5  (3 << 6)
#define WM8753_PCM_DIV_2    (4 << 6)
#define WM8753_PCM_DIV_4    (5 << 6)
#define WM8753_PCM_DIV_6    (6 << 6)
#define WM8753_PCM_DIV_8    (7 << 6)

/* BCLK clock dividers */
#define WM8753_BCLK_DIV_1   (0 << 3)
#define WM8753_BCLK_DIV_2   (1 << 3)
#define WM8753_BCLK_DIV_4   (2 << 3)
#define WM8753_BCLK_DIV_8   (3 << 3)
#define WM8753_BCLK_DIV_16  (4 << 3)

/* VXCLK clock dividers */
#define WM8753_VXCLK_DIV_1  (0 << 6)
#define WM8753_VXCLK_DIV_2  (1 << 6)
#define WM8753_VXCLK_DIV_4  (2 << 6)
#define WM8753_VXCLK_DIV_8  (3 << 6)
#define WM8753_VXCLK_DIV_16 (4 << 6)

#define WM8753_DAI_HIFI     0
#define WM8753_DAI_VOICE    1

#define WM8753_MAC1ALC      1

/* SAMPCTRL values for the supported samplerates (24MHz MCLK/USB): */
#define WM8753_USB24_8000HZ   0x60
#define WM8753_USB24_8021HZ   0x17
#define WM8753_USB24_11025HZ  0x19
#define WM8753_USB24_12000HZ  0x08
#define WM8753_USB24_16000HZ  0x0A
#define WM8753_USB24_22058HZ  0x1B
#define WM8753_USB24_24000HZ  0x1C
#define WM8753_USB24_32000HZ  0x0C
#define WM8753_USB24_44118HZ  0x11
#define WM8753_USB24_48000HZ  0x00
#define WM8753_USB24_88235HZ  0x1F
#define WM8753_USB24_96000HZ  0x0E
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/
#if HS_CODEC_USE_WM8753
#define WM8753_DAC        0x01
#define WM8753_ADC        0x02
#define WM8753_PCM        0x03
#define WM8753_HIFI       0x04
#define WM8753_IOCTL      0x05
#define WM8753_SRATE1     0x06
#define WM8753_SRATE2     0x07
#define WM8753_LDAC       0x08
#define WM8753_RDAC       0x09
#define WM8753_BASS       0x0a
#define WM8753_TREBLE     0x0b
#define WM8753_ALC1       0x0c
#define WM8753_ALC2       0x0d
#define WM8753_ALC3       0x0e
#define WM8753_NGATE      0x0f
#define WM8753_LADC       0x10
#define WM8753_RADC       0x11
#define WM8753_ADCTL1     0x12
#define WM8753_3D         0x13
#define WM8753_PWR1       0x14
#define WM8753_PWR2       0x15
#define WM8753_PWR3       0x16
#define WM8753_PWR4       0x17
#define WM8753_ID         0x18
#define WM8753_INTPOL     0x19
#define WM8753_INTEN      0x1a
#define WM8753_GPIO1      0x1b
#define WM8753_GPIO2      0x1c
#define WM8753_RESET      0x1f
#define WM8753_RECMIX1    0x20
#define WM8753_RECMIX2    0x21
#define WM8753_LOUTM1     0x22
#define WM8753_LOUTM2     0x23
#define WM8753_ROUTM1     0x24
#define WM8753_ROUTM2     0x25
#define WM8753_MOUTM1     0x26
#define WM8753_MOUTM2     0x27
#define WM8753_LOUT1V     0x28
#define WM8753_ROUT1V     0x29
#define WM8753_LOUT2V     0x2a
#define WM8753_ROUT2V     0x2b
#define WM8753_MOUTV      0x2c
#define WM8753_OUTCTL     0x2d
#define WM8753_ADCIN      0x2e
#define WM8753_INCTL1     0x2f
#define WM8753_INCTL2     0x30
#define WM8753_LINVOL     0x31
#define WM8753_RINVOL     0x32
#define WM8753_MICBIAS    0x33
#define WM8753_CLOCK      0x34
#define WM8753_PLL1CTL1   0x35
#define WM8753_PLL1CTL2   0x36
#define WM8753_PLL1CTL3   0x37
#define WM8753_PLL1CTL4   0x38
#define WM8753_PLL2CTL1   0x39
#define WM8753_PLL2CTL2   0x3a
#define WM8753_PLL2CTL3   0x3b
#define WM8753_PLL2CTL4   0x3c
#define WM8753_BIASCTL    0x3d
#define WM8753_ADCTL2     0x3f

#define WM8753_PLL1       0
#define WM8753_PLL2       1

struct reg_default {
	unsigned int reg;
	unsigned int def;
};
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/
typedef enum
{
  AUDIO_RECORD_SET_VOLUME         = 0,
  AUDIO_RECORD_SET_VOLUME_LEFT    ,
  AUDIO_RECORD_SET_VOLUME_RIGHT   ,
  AUDIO_RECORD_GET_VOLUME_MAX     ,
  AUDIO_RECORD_GET_VOLUME_MIN     ,
  AUDIO_RECORD_MUTE               ,
  AUDIO_RECORD_UNMUTE             ,
  AUDIO_RECORD_SET_DRC            ,
  AUDIO_RECORD_SET_IIR            ,
  AUDIO_RECORD_SET_DECI           ,
  AUDIO_RECORD_SIDE_CTRL          ,
  AUDIO_RECORD_SET_DCOFFSET       ,
  AUDIO_RECORD_SET_MIXED          ,
  AUDIO_RECORD_SET_SAMPLE         ,

  AUDIO_PLAY_SET_VOLUME           = 0x40,
  AUDIO_PLAY_SET_VOLUME_LEFT      ,
  AUDIO_PLAY_SET_VOLUME_RIGHT     ,
  AUDIO_PLAY_GET_VOLUME_MAX       ,
  AUDIO_PLAY_GET_VOLUME_MIN       ,
  AUDIO_PLAY_MUTE                 ,
  AUDIO_PLAY_UNMUTE               ,
  AUDIO_PLAY_SET_EQ               ,
  AUDIO_PLAY_SET_DRC              ,
  AUDIO_PLAY_MOD_CTRL             ,
  AUDIO_PLAY_SET_MIXED            ,
	AUDIO_PLAY_SET_SAMPLE           ,

  AUDIO_SET_INPUT_SOURCE          = 0x60,
  AUDIO_SET_CODEC_SEL             ,
  AUDIO_SET_RECORD_SOURCE         ,
  AUDIO_SET_PLAY_SOURCE           ,
  AUDIO_SET_SHORT_FIR             ,
  AUDIO_SET_ADC_DRC_MODE          ,
  AUDIO_SET_ADC_DRC_LIMITER       ,
  AUDIO_SET_ADC_DRC_AGC           ,
  AUDIO_SET_DAC_DRC_MODE          ,
  AUDIO_SET_DAC_DRC_LIMITER       ,
  AUDIO_SET_DAC_DRC_AGC           ,
  AUDIO_SET_DAC_MIX               ,
  AUDIO_SET_ADC_MIX               ,
  AUDIO_SET_DAC_MODE              ,
  AUDIO_INVERT_I2S_OUTPUT         ,
  AUDIO_INVERT_I2S_INPUT          ,

  AUDIO_SET_EQ                    ,
  AUDIO_SET_BAND1_GAIN            ,
  AUDIO_SET_BAND1_COEFF           ,
  AUDIO_SET_BAND2_GAIN            ,
  AUDIO_SET_BAND2_COEFF           ,
  AUDIO_SET_BAND3_GAIN            ,
  AUDIO_SET_BAND3_COEFF           ,
  AUDIO_SET_BAND4_GAIN            ,
  AUDIO_SET_BAND4_COEFF           ,
  AUDIO_SET_BAND5_GAIN            ,
  AUDIO_SET_BAND5_COEFF           ,
  AUDIO_SET_BAND6_GAIN            ,
  AUDIO_SET_BAND6_COEFF           ,
  AUDIO_SET_BAND7_GAIN            ,
  AUDIO_SET_BAND7_COEFF           ,
  AUDIO_SET_TEST_MODE             ,
  AUDIO_SET_I2S_CONN_CTRL         ,
  AUDIO_GET_DAC_RMS               ,
  AUDIO_AEC_DELAY_MEASURE_INIT    ,
  AUDIO_GET_AEC_DELAY             ,
  AUDIO_SET_DAC_PEAK              ,
}hs_audio_ctrltype_t;

typedef enum
{
  CODEC_STATE_IDLE        = 0,
  CODEC_STATE_STOP        ,
  CODEC_STATE_WORKING     ,
}hs_codec_state_t;

typedef enum
{
  AUDIO_INSIDE_CODEC      = 0,
  AUDIO_EXTERN_CODEC      ,         //wm8753
}hs_audio_codec;

typedef enum
{
  AUDIO_RECORD_LINEIN     = 0,      //line in
  AUDIO_RECORD_DMIC       ,         //digital mic
  AUDIO_RECORD_MIC        ,         //analog mic
  AUDIO_RECORD_FM         ,         //fm radio
}hs_audio_record_source;

typedef enum
{
  AUDIO_PLAY_RAM          = 0,      //data from memory
  AUDIO_PLAY_PCM          ,         //ceva pcm
  AUDIO_PLAY_LINEIN       ,         //route line-in to line-out directly
  AUDIO_PLAY_FM           ,         //fm radio
  AUDIO_PLAY_MIC          ,
}hs_audio_play_source;

#define DB2VOL_DAC(db)    (uint8_t)((db) < -47 ? 0 : (db) * 2 + 95)
#define DB2VOL_DAC_WM(db) (uint8_t)((db) < -73 ? 0 : (db) * 1 + 0x079)

#define DB2VOL_ADC(db)    (uint8_t)((db) < -50 ? 0 : (db) * 2 + 100)
#define DB2VOL_ADC_WM(db) (uint8_t)((db) < -97 ? 0 : (db) * 2 + 0x0c3)


typedef struct
{
  uint32_t    u32Freq[EQ_BAND_NUM];
  int32_t     s32Gain[EQ_BAND_NUM];
}hs_codec_eqpara_t;

typedef struct
{
  uint8_t     pa_gpio;
  uint8_t     pa_lvl;
  
  int8_t      drv_gain;
  uint8_t     drv_single;
  uint8_t     mic_sel;            // mic select: b'00-LR b'01-RL b'10-LL b'11-RR
  uint8_t     pga_gain_aux;       // line-in
  uint8_t     pga_gain_mic;       // mic-in
  uint8_t     sel_pkg;            // package 0-dual path, 1-single path

  uint8_t     dac_mix_en;         
  uint8_t     adc_mix_en; 
  
  int         dac_max_gain;       // <=  26db
  int         dac_min_gain;       // >= -47db

  int         adc_max_gain;       // <=  24db
  int         adc_min_gain;       // >= -47db

  int         hfp_dacmax_gain;
  int         adc_default_gain;   // for hfp

  const hs_codec_eqpara_t *pstDefaultEq;
}hs_codec_boardinfo_t;


/**
 * @brief   Structure representing an I2S driver.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
typedef struct
{
  HS_CODEC_Type          *codec;

  hs_audio_config_t      *rx_cfg;
  hs_audio_config_t      *tx_cfg;

  hs_codec_state_t       rx_state;
  hs_codec_state_t       tx_state;

  const hs_codec_boardinfo_t *pboardinfo;

  uint32_t                mclk;
  uint8_t                 micbias_tune;
  uint8_t                 rc_tune;
  int32_t                 db;
  uint32_t                cnt;
}CODECDriver;

typedef void (*hs_fnCtrlMethod)(CODECDriver *, void *);

typedef struct
{
  hs_audio_ctrltype_t   type;
  hs_fnCtrlMethod       fn_ctrl_method;
}hs_codec_ctrl_t;


/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

#define RC_CALIBRATE_NUMBER   5
#define PGA_GAIN_MAXIDX       7

#define __codec_clr_bit(val, bit)            (val) &= ~(1u<<bit)
#define __codec_set_bit(val, bit)            (val) |= (1u<<bit)

#define __codec_set_bitval(val, bit, bitval)        \
do{                                                 \
  uint32_t mask;                                    \
  mask = 1u<<(bit);                                 \
  (val) = ((val)&~mask) | (((bitval)<<(bit))&mask); \
}while(0)

#define __codec_set_bitsval(val, s, e, bitval)      \
do{                                                 \
  uint32_t mask;                                    \
  mask = ((1u<<((e)-(s)+1)) - 1) << (s);            \
  (val) = ((val)&~mask) | (((bitval)<<(s))&mask);   \
}while(0)

#define __codec_adc_enable(codec)             __codec_set_bit((codec)->ADC_CTRL, 0)
#define __codec_adc_disable(codec)            __codec_clr_bit((codec)->ADC_CTRL, 0)
#define __codec_adc_sw_reset(codec)           __codec_set_bit((codec)->ADC_CTRL, 1)
#define __codec_adc_dmic_enable(codec)        __codec_set_bit((codec)->ADC_CTRL, 2)
#define __codec_adc_dmic_disable(codec)       __codec_clr_bit((codec)->ADC_CTRL, 2)
#define __codec_adc_dc_enable(codec)          __codec_set_bit((codec)->ADC_CTRL, 3)
#define __codec_adc_dc_disable(codec)         __codec_clr_bit((codec)->ADC_CTRL, 3)

#define __codec_adc_set_mute(codec, val)      __codec_set_bitval((codec)->ADC_VOL_CTRL, 0, (val))
#define __codec_adc_set_unmute(codec, val)    __codec_set_bitval((codec)->ADC_VOL_CTRL, 1, (val))
#define __codec_adc_set_bypass(codec, val)    __codec_set_bitval((codec)->ADC_VOL_CTRL, 2, (val))
#define __codec_adc_mixed_enable(codec, val)  __codec_set_bitval((codec)->ADC_VOL_CTRL, 3, (val))
#define __codec_adc_set_leftvol(codec, val)  __codec_set_bitsval((codec)->ADC_VOL_CTRL, 8, 15, (val))
#define __codec_adc_set_rightvol(codec, val) __codec_set_bitsval((codec)->ADC_VOL_CTRL, 16, 23, (val))
#define __codec_adc_set_volupdate(codec, val) __codec_set_bitval((codec)->ADC_VOL_CTRL, 24, (val))

#define __codec_dac_short_fir_enable(codec)   __codec_set_bit((codec)->DAC_CTRL, 11)
#define __codec_dac_short_fir_disable(codec)  __codec_clr_bit((codec)->DAC_CTRL, 11)
#define __codec_dac_sw_reset(codec)           __codec_set_bit((codec)->DAC_CTRL, 9)
#define __codec_dac_enable(codec)             __codec_set_bit((codec)->DAC_CTRL, 8)
#define __codec_dac_disable(codec)            __codec_clr_bit((codec)->DAC_CTRL, 8)
#define __codec_dac_set_fir(codec, val)       __codec_set_bitval((codec)->DAC_CTRL, 10, (val))

#define __codec_dac_set_mute(codec, val)      __codec_set_bitval((codec)->DAC_VOL_CTRL, 0, (val))
#define __codec_dac_set_unmute(codec, val)    __codec_set_bitval((codec)->DAC_VOL_CTRL, 1, (val))
#define __codec_dac_set_bypass(codec, val)    __codec_set_bitval((codec)->DAC_VOL_CTRL, 2, (val))
#define __codec_dac_mixed_enable(codec, val)  __codec_set_bitval((codec)->DAC_VOL_CTRL, 3, (val))
#define __codec_dac_set_leftvol(codec, val)  __codec_set_bitsval((codec)->DAC_VOL_CTRL, 8, 15, (val))
#define __codec_dac_set_rightvol(codec, val) __codec_set_bitsval((codec)->DAC_VOL_CTRL, 16, 23, (val))
#define __codec_dac_set_volupdate(codec, val) __codec_set_bitval((codec)->DAC_VOL_CTRL, 24, (val))

#define __codec_test_set_mode(codec, val)     (codec)->TEST_MODE = (val)

//#define __codec_powerdown_micbias(codec)      __codec_set_bitsval((codec)->ANA_CTRL_1, 3, 3, 1);


/*
 * output operation
 */
#define _lld_enableRxI2s()               codec_setI2sRxClk(&CODECD, 0x01)
#define _lld_disableRxI2s()              codec_setI2sRxClk(&CODECD, 0x00)

#define _lld_enableTxI2s()               codec_setI2sTxClk(&CODECD, 0x01)
#define _lld_disableTxI2s()              codec_setI2sTxClk(&CODECD, 0x00)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if HS_CODEC_USE_DRV && !defined(__DOXYGEN__)
extern CODECDriver CODECD;
#endif


#ifdef __cplusplus
extern "C" {
#endif

void codec_lld_init(void);
void codec_lld_start(CODECDriver *codecp);
void codec_lld_stop(CODECDriver *codecp);

void codec_lld_record_start(CODECDriver *codecp, hs_audio_config_t *cfgp);
void codec_lld_record_stop(CODECDriver *codecp);

void codec_lld_play_start(CODECDriver *codecp, hs_audio_config_t *cfgp);
void codec_lld_play_stop(CODECDriver *codecp);

void codec_lld_ctrl(CODECDriver *codecp, hs_audio_ctrltype_t ctype, void *arg);

void codec_setI2sRxClk(CODECDriver *codecp, uint8_t val);
void codec_setI2sTxClk(CODECDriver *codecp, uint8_t val);

void codec_lld_rxAdc_calibration();
void codec_lld_auAdc_calibration();
void codec_lld_txDac_calibration();
void codec_lld_rxFilter_calibration();
void codec_lld_rxTia_calibration();
void codec_lld_micbias_calibration();

uint32_t codec_lld_getStatus(CODECDriver *codecp);
int32_t codec_analog_set_pga_gain(int32_t pga_idx);
void codec_lld_setBTCfg(void);
void codec_lld_setMixer(uint8_t vol);
int codec_lld_isMixerEn(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_CODEC */

#endif /* _CODEC_LLD_H_ */

/** @} */
