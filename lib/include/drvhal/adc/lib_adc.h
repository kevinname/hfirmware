/*
    drvhal - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/drvhal/lib_adc.h
 * @brief   adc include file.
 * @details 
 *
 * @addtogroup  lib drvhal
 * @details 
 * @{
 */
#ifndef __LIB_ADC_H__
#define __LIB_ADC_H__

#include "lib.h"

typedef enum 
{
  ADC_GAIN_MULTI_0P5                = 0,
  ADC_GAIN_MULTI_1                  ,
  ADC_GAIN_MULTI_2                  ,
  ADC_GAIN_MULTI_4                  ,
  
  ADC_GAIN_MAX                      ,
}hs_adcgain_t;

typedef enum 
{
  ADC_VCM_SEL_000mV                 = 0,
  ADC_VCM_SEL_150mV                 ,
  ADC_VCM_SEL_300mV                 ,
  ADC_VCM_SEL_450mV                 ,
  ADC_VCM_SEL_600mV                 ,
  ADC_VCM_SEL_750mV                 ,
  ADC_VCM_SEL_900mV                 ,
  ADC_VCM_SEL_1050mV                ,
  ADC_VCM_SEL_1200mV                ,
  ADC_VCM_SEL_1350mV                ,
  ADC_VCM_SEL_1500mV                ,
  ADC_VCM_SEL_1650mV                ,
  ADC_VCM_SEL_1800mV                ,
  ADC_VCM_SEL_1950mV                ,
  ADC_VCM_SEL_2100mV                ,
  ADC_VCM_SEL_2250mV                ,
  ADC_VCM_MAX
}hs_adcvcm_t;

#define ADC_HANDLER         (&ADCD0)

#ifdef __cplusplus
extern "C" {
#endif

#if HAL_USE_ADC
int  hs_adc_open(uint8_t trigerMode, uint8_t swpEnable);
void hs_adc_close(void);

int  hs_adc_addChn(adc_channel_t chn, hs_adcgain_t gain, hs_adcvcm_t vcm, uint8_t chopper_en);
int  hs_adc_delChn(uint8_t chn);

int  hs_adc_getDataByDma(uint16_t u16ChnMap, uint8_t *pu8Buf, uint32_t u32Len);
uint32_t hs_adc_getChnData(uint8_t chn);

int16_t hs_adc_getInteger(uint16_t data);
void hs_adc_adjust(uint16_t msg, void *parg);
void hs_adc_getAdjust(float *mvoffset, float *gainMod);

void hs_adc_init(void);
void hs_adc_scan(void);
uint32_t hs_adc_getKeyMap(void);
float hs_adc_getBatteryVolt(void);
float hs_adc_getTemperature(void);

#endif

#ifdef __cplusplus
}
#endif


#endif
 /** @} */
