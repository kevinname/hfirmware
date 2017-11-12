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
 * @file    templates/adc_lld.h
 * @brief   ADC Driver subsystem low level driver header template.
 *
 * @addtogroup ADC
 * @{
 */

#ifndef _ADC_LLD_H_
#define _ADC_LLD_H_

#if HAL_USE_ADC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/
#include "dma_lld.h"
/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   ADC1 driver enable switch.
 * @details If set to @p TRUE the support for ADC1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(PLATFORM_ADC_USE_ADC0) || defined(__DOXYGEN__)
#define PLATFORM_ADC_USE_ADC0             TRUE
#endif
/** @} */

#if !defined(HS_ADC_DMA_PRIORITY) || defined(__DOXYGEN__)
#define HS_ADC_DMA_PRIORITY               0
#endif

#if !defined(HS_ADC_ADC0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define HS_ADC_ADC0_IRQ_PRIORITY          3
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/
#define ADC_ALL_DONE_MASK                 (1u << 9)
#define ADC_OVERFLOW_MASK                 (1u << 12)
/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/
typedef uint8_t adc_timer_chn_t;
/**
 * @brief   Type of a structure representing an ADC driver.
 */
typedef struct ADCDriver ADCDriver;

typedef enum {
  ADC_CHANNEL_CHIP_TEMPERATURE   = 0,
  ADC_CHANNEL_CHIP_BATTERY       ,
  ADC_CHANNEL_CHIP_VCM           ,
  ADC_CHANNEL_EXTERN_PIN0        ,
  ADC_CHANNEL_EXTERN_PIN1        ,
  ADC_CHANNEL_EXTERN_PIN2        ,
  ADC_CHANNEL_EXTERN_PIN3        ,
  ADC_CHANNEL_EXTERN_PIN4        ,
  ADC_CHANNEL_PEAK_DETECTOR      ,
  
  ADC_CHANNEL_NUM
} adc_channel_t;

typedef enum {
  ADC_TEST_CHOPPER_FREQ_500KHZ      = 0,
  ADC_TEST_CHOPPER_FREQ_250KHZ      ,
  ADC_TEST_CHOPPER_FREQ_125KHZ      ,
  ADC_TEST_CHOPPER_FREQ_62P5KHZ     ,
} adc_test_chopper_freq_t;

typedef enum {
  ADC_TEST_GAIN_AMP_MULTI_0P5             = 0,
  ADC_TEST_GAIN_AMP_MULTI_1               ,
  ADC_TEST_GAIN_AMP_MULTI_2               ,
  ADC_TEST_GAIN_AMP_MULTI_4               ,
} adc_test_gain_amp_t;

typedef enum {
  ADC_TEST_SEL_AMPHV_MULTI_1             = 0,
  ADC_TEST_SEL_AMPHV_MULTI_2             ,
  ADC_TEST_SEL_AMPHV_MULTI_4             ,
  ADC_TEST_SEL_AMPHV_MULTI_SATURATION    ,
} adc_test_sel_amphv_t;

typedef enum {
  ADC_TEST_CTRL_LDO_2P3V              = 0,
  ADC_TEST_CTRL_LDO_2P4V              ,
  ADC_TEST_CTRL_LDO_2P5V              ,
  ADC_TEST_CTRL_LDO_2P6V              ,
} adc_test_ctrl_ldo_t;

typedef enum {
  ADC_CONT_MODE_ONLY_ONCE               = 0,
  ADC_CONT_MODE_MULTI_NUM               ,
} adc_cont_mode_t;

typedef enum {
  ADC_TRIGER_MODE_SOFTWARE              = 0,
  ADC_TRIGER_MODE_HARDWARE              ,
} adc_triger_mode_t;

typedef enum {
  ADC_DATA_ALIGN_RIGHT                  = 0,
  ADC_DATA_ALIGN_LEFT                   ,
} adc_data_align_t;

typedef enum {
  ADC_CHN_ORDER_SMALL_FIRST             = 0,
  ADC_CHN_ORDER_BIG_FIRST               ,
} adc_chn_order_t;

typedef enum {
  ADC_HDTRIGER_SOURCE_TM0_0             = 0,
  ADC_HDTRIGER_SOURCE_TM0_1             ,
  ADC_HDTRIGER_SOURCE_TM0_2             ,
  ADC_HDTRIGER_SOURCE_TM0_3             ,
  ADC_HDTRIGER_SOURCE_TM1_0             ,
  ADC_HDTRIGER_SOURCE_TM1_1             ,
  ADC_HDTRIGER_SOURCE_TM1_2             ,
  ADC_HDTRIGER_SOURCE_TM1_3             ,
  ADC_HDTRIGER_SOURCE_TM2_0             ,
  ADC_HDTRIGER_SOURCE_TM2_1             ,
  ADC_HDTRIGER_SOURCE_TM2_2             ,
  ADC_HDTRIGER_SOURCE_TM2_3             ,
  ADC_HDTRIGER_SOURCE_EXT               ,
} adc_hdtriger_source_t;

typedef enum {
  ADC_TRIGER_EDGE_POS                 = 0,
  ADC_TRIGER_EDGE_NEG                 ,
  ADC_TRIGER_EDGE_DUAL                ,
  ADC_TRIGER_EDGE_NOUSED              ,
} adc_triger_edge_t;

typedef enum {
  ADC_TEST_MODE_ADC                   = 0,
  ADC_TEST_MODE_REG                   ,
} adc_test_mode_t;

typedef enum {
  ADC_DISC_MODE_CONT                  = 0,
  ADC_DISC_MODE_DISCONT               ,
} adc_disc_mode_t;

typedef enum {
  ADC_INPUT_SRC_FILTER                = 0,
  ADC_INPUT_SRC_ANALOG                ,
} adc_input_src_t;


typedef struct
{
  uint8_t  t1;
  uint8_t  t2;
  uint8_t  t3;
  uint8_t  t4;
  uint16_t t5;
  uint8_t  t6;

  uint8_t  cic_cycle;
  uint8_t  smp_cycle;
  uint8_t  rst_cycle;
}adc_timing_t;

typedef struct
{
  uint8_t en_r2r;
  uint8_t en_chop;
  uint8_t gtune;
  uint8_t ldoctrl;
  uint8_t sel_fchop;
  uint8_t sel_inp;
  uint8_t sel_vcm;
  uint8_t en_count_sar;
  uint8_t en_dem_sar;
  uint8_t sar_buf;
  uint8_t en_sar_ckdelay;
}adc_chn_para_t;

typedef struct
{
  adc_cont_mode_t        cont;
  adc_data_align_t       align;
  adc_triger_mode_t      hw_en;
  adc_chn_order_t        scandir;
  adc_hdtriger_source_t  timer_sel;
  adc_triger_edge_t      start_edge;
  adc_disc_mode_t        disen;

  adc_input_src_t        sarq_bypass;
  uint8_t                swap_enable;
  adc_test_mode_t        test_mode;
}adc_attr_t;


/**
 * @brief   Structure representing an ADC driver.
 */
struct ADCDriver {
  adcstate_t                state;
  HS_ADC_Type               *adc;

  uint16_t                  channel_mask;
  uint16_t                  channel_num;
  //const adc_timing_t        *timing;
  //const adc_attr_t          *attribute;
  //const adc_chn_para_t      *pPara[ADC_CHANNEL_NUM];          /* can be set to 'NULL', otherwise self-default */

  int32_t                   samples;
  float                     adjust_ratio;

  hs_dma_stream_t           *pdma;
  thread_t                  *thread[ADC_CHANNEL_NUM];
  thread_t                  *dmathrd;
#if ADC_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
#if CH_CFG_USE_MUTEXES || defined(__DOXYGEN__)
  mutex_t                   mutex;
#elif CH_CFG_USE_SEMAPHORES
  semaphore_t               semaphore;
#endif
#endif /* ADC_USE_MUTUAL_EXCLUSION */
  /* End of the mandatory fields.*/
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

#define __adc_set_bitval(val, bit, bitval)          \
do{                                                 \
  uint32_t mask;                                    \
  mask = 1u<<(bit);                                 \
  (val) = ((val)&~mask) | (((bitval)<<(bit))&mask); \
}while(0)

#define __adc_set_bitsval(val, s, e, bitval)        \
do{                                                 \
  uint32_t mask;                                    \
  mask = ((1u<<((e)-(s)+1)) - 1) << (s);            \
  (val) = ((val)&~mask) | (((bitval)<<(s))&mask);   \
}while(0)

#define __adc_set_en_r2r(adc_r, val)              __adc_set_bitval (adc_r, 0, (val))
#define __adc_set_en_chop(adc_r, val)             __adc_set_bitval (adc_r, 1, (val))
#define __adc_set_gtune(adc_r, val)               __adc_set_bitsval(adc_r, 3, 4, (val))
#define __adc_set_ldoctrl(adc_r, val)             __adc_set_bitsval(adc_r, 5, 6, (val))
#define __adc_set_fchop(adc_r, val)               __adc_set_bitsval(adc_r, 7, 8, (val))
#define __adc_set_sel_inp(adc_r, val)             __adc_set_bitsval(adc_r, 9, 12, (val))
#define __adc_set_sel_vcm(adc_r, val)             __adc_set_bitsval(adc_r, 13, 16, (val))
#define __adc_set_en_count_sar(adc_r, val)        __adc_set_bitval (adc_r, 17, (val))
#define __adc_set_en_dem_sar(adc_r, val)          __adc_set_bitval (adc_r, 18, (val))
#define __adc_set_set_sar_buf(adc_r, val)         __adc_set_bitval (adc_r, 19, (val))
#define __adc_set_en_sar_ckdelay(adc_r, val)      __adc_set_bitsval(adc_r, 20, 23, (val))

#define __adc_get_intr(adc_r)                     ((adc_r)->adc_intr & 0x1fff)
#define __adc_clr_intr(adc_r, val)                ((adc_r)->adc_intr = (val) & 0x1fff)
#define __adc_en_int(adc_r, mask)                 ((adc_r)->adc_intr_mask = (mask))

#define __adc_sw_start(adc_r, val)                __adc_set_bitval((adc_r)->adc_cfg0, 0, (val))
#define __adc_stop(adc_r, val)                    __adc_set_bitval((adc_r)->adc_cfg0, 1, (val))

#define __adc_set_t1(adc_r, val)                  __adc_set_bitsval((adc_r)->adc_timing_cfg0, 0, 3, (val))
#define __adc_set_t2(adc_r, val)                  __adc_set_bitsval((adc_r)->adc_timing_cfg0, 4, 7, (val))
#define __adc_set_t3(adc_r, val)                  __adc_set_bitsval((adc_r)->adc_timing_cfg0, 8, 11, (val))
#define __adc_set_t4(adc_r, val)                  __adc_set_bitsval((adc_r)->adc_timing_cfg0, 12, 15, (val))
#define __adc_set_t5(adc_r, val)                  __adc_set_bitsval((adc_r)->adc_timing_cfg1, 0, 11, (val))
#define __adc_set_t6(adc_r, val)                  __adc_set_bitsval((adc_r)->adc_timing_cfg1, 12, 17, (val))
#define __adc_set_cic_cycle(adc_r, val)           __adc_set_bitsval((adc_r)->adc_timing_cfg0, 16, 23, (val))
#define __adc_set_smp(adc_r, val)                 __adc_set_bitsval((adc_r)->adc_timing_cfg1, 20, 23, (val))
#define __adc_set_rst(adc_r, val)                 __adc_set_bitsval((adc_r)->adc_timing_cfg1, 24, 27, (val))

#define __adc_set_cont(adc_cfg1, val)             __adc_set_bitval(adc_cfg1, 0, (val))
#define __adc_set_data_align(adc_cfg1, val)       __adc_set_bitval(adc_cfg1, 1, (val))
#define __adc_set_hwstart_en(adc_cfg1, val)       __adc_set_bitval(adc_cfg1, 2, (val))
#define __adc_set_scan_dir(adc_cfg1, val)         __adc_set_bitval(adc_cfg1, 3, (val))
#define __adc_set_start_src(adc_cfg1, val)        __adc_set_bitsval(adc_cfg1, 4, 7, (val))
#define __adc_set_triger_edge(adc_cfg1, val)      __adc_set_bitsval(adc_cfg1, 8, 9, (val))
#define __adc_set_discen(adc_cfg1, val)           __adc_set_bitval(adc_cfg1, 10, (val))
#define __adc_set_sari_bypass(adc_cfg1, val)      __adc_set_bitval(adc_cfg1, 11, (val))
#define __adc_set_sarq_bypass(adc_cfg1, val)      __adc_set_bitval(adc_cfg1, 12, (val))
#define __adc_set_swap_enable(adc_cfg1, val)      __adc_set_bitval(adc_cfg1, 13, (val))
#define __adc_set_test_mode(adc_cfg1, val)        __adc_set_bitval(adc_cfg1, 14, (val))

#define __adc_set_chn_sel(adc_r, val)             __adc_set_bitsval((adc_r)->adc_chselr, 0, 8, (val))

#define __adc_set_fifo_en(adc_r, val)             __adc_set_bitval((adc_r)->adc_fifo_con, 0, (val))
#define __adc_set_fifo_rstn(adc_r, val)           __adc_set_bitval((adc_r)->adc_fifo_con, 1, (val))
#define __adc_set_dma_en(adc_r, val)              __adc_set_bitval((adc_r)->adc_dma_con, 0, (val))
#define __adc_set_fifo_thres(adc_r, val)          __adc_set_bitsval((adc_r)->adc_rxfifo_th, 0, 3, (val))

#define _adc_SetTestData(val)                     __adc_set_bitsval(ADCD0.adc->adc_test_data, 0, 9, (val))

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if PLATFORM_ADC_USE_ADC0 && !defined(__DOXYGEN__)
extern ADCDriver ADCD0;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void adc_lld_init(void);

  int adc_lld_start(ADCDriver *adcp, const adc_attr_t *config);
  void adc_lld_stop(ADCDriver *adcp);

  int adc_lld_set_timing(ADCDriver *adcp, const adc_timing_t *pTiming);

  int adc_lld_add_channel(ADCDriver *adcp, adc_channel_t chn, const adc_chn_para_t *pChnPara);
  int adc_lld_delete_channel(ADCDriver *adcp, adc_channel_t chn);
  uint32_t adc_lld_getChnData(ADCDriver *adcp, adc_channel_t chn, systime_t timeout);
  
  int adc_lld_start_conversion(ADCDriver *adcp);
  int adc_lld_start_conversionWithDMA(ADCDriver *adcp, uint8_t *pDmaBuf, uint32_t DmaLen);
  int adc_lld_dmaWaitForDone(ADCDriver *adcp, systime_t timeout);
  int adc_lld_stop_conversion(ADCDriver *adcp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_ADC */

#endif /* _ADC_LLD_H_ */

/** @} */
