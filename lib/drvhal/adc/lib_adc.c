/*
    drvhal - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/drvhal/lib_adc.c
 * @brief   pwm file.
 * @details 
 *
 * @addtogroup  lib drvhal
 * @details 
 * @{
 */

#include "lib.h"

#if HAL_USE_ADC

#define ADC_DATA_WIDTH          16
#define ADC_VALIDDATA_WIDTH     12

#define ADC_SIGNED_MASK         (1u << (ADC_VALIDDATA_WIDTH - 1))
#define ADC_DATA_MASK           ((1u << ADC_VALIDDATA_WIDTH) - 1)
#define ADC_DATA_COMPLEMENT     (((1u << (ADC_DATA_WIDTH - ADC_VALIDDATA_WIDTH)) - 1) << ADC_VALIDDATA_WIDTH)

#define ADC_MDA_MAXLEN          (2*1024)

#define __adckey_getRange()     (g_pstAdcPara->pstDrvCfg->fAdcKeyRange)

typedef struct
{
  uint8_t     u8Reserve[8];
  float       fOffset;
  float       fGainMod;
  uint32_t    u32Chk;
}hs_adc_adjust_t;

typedef struct
{
  uint8_t   trigerMode;
  uint8_t   trigerSrc;
  uint8_t   swpEnable;
  uint8_t   adcKeyPin;
  uint8_t   adcKeyPinEx;

  uint8_t   batStatus;
  uint8_t   batStatusCnt;
  
  uint8_t   tempStatus;
  uint8_t   tempStatusCnt;

  float     mvoffset;
  float     gainMod;

  uint32_t  counter;
  float     batVolt;
  float     batVoltTo;

  adc_chn_para_t *pstChnPara[ADC_CHANNEL_NUM];
  const hs_drvhal_cfg_t *pstDrvCfg;
}hs_adcpara_t;

static hs_adcpara_t *g_pstAdcPara;

void hs_cfg_sysBnPressed(ioportid_t port, uint8_t pad);

static float const g_fGain[4] = {0.5, 1, 2, 4};

float _adc_getAdcKeyVolt(uint8_t chn)
{
  int16_t s16Data;
  float vi, vd;

  s16Data = hs_adc_getInteger(hs_adc_getChnData(chn));

  vd = s16Data * 1000.0 / 2048;
  vi = (vd - g_pstAdcPara->mvoffset) * g_pstAdcPara->gainMod / g_fGain[g_pstAdcPara->pstDrvCfg->eAdcKeyGain]; 

  return vi;
}

uint16_t _adc_getBatteryStatus(float vi)
{
  uint16_t u16Status = 0;

  if(vi >= g_pstAdcPara->pstDrvCfg->fBatFullVolt)
    u16Status = HS_CFG_EVENT_BATTERY_FULL;
  else if(vi >= (0.90 * g_pstAdcPara->pstDrvCfg->fBatFullVolt))
    u16Status = HS_CFG_EVENT_BATTERY_NEARFULL;
  else if(vi >= (g_pstAdcPara->pstDrvCfg->fBatFullVolt + g_pstAdcPara->pstDrvCfg->fBatEmptyVolt) / 2)
    u16Status = HS_CFG_EVENT_BATTERY_HALFFULL;
  else if(vi <= (1.1 * g_pstAdcPara->pstDrvCfg->fBatEmptyVolt))
    u16Status = HS_CFG_EVENT_BATTERY_NEAREMPTY;
  else
    u16Status = 0;
  
  return u16Status;
}

uint16_t _adc_getTempStatus(float temp)
{
  uint16_t u16Status = 0;

  if(temp >= g_pstAdcPara->pstDrvCfg->fTempMaxAlert)
    u16Status = HS_CFG_EVENT_TEMPERATURE_HIGHALERT;
  else if(temp <= g_pstAdcPara->pstDrvCfg->fTempMinAlert)
    u16Status = HS_CFG_EVENT_TEMPERATURE_LOWALERT;
  else
    u16Status = 0;
  
  return u16Status;
}

void _adc_serviceOpen(hs_adcpara_t *pstPara)
{
  adc_attr_t stAttr = {0};
  uint32_t i;

  stAttr.hw_en       = pstPara->trigerMode;
  stAttr.timer_sel   = pstPara->trigerSrc;
  stAttr.sarq_bypass = 0;
  stAttr.swap_enable = pstPara->swpEnable;
  stAttr.test_mode   = 0;
  if(0 != adcStart(ADC_HANDLER, &stAttr))
    goto __adc_serviceOpen_error;

  for(i=0; i<ADC_CHANNEL_NUM; i++)
  {
    if(pstPara->pstChnPara[i])
      adcAddChannel(ADC_HANDLER, i, pstPara->pstChnPara[i]);
  }

  return ;

__adc_serviceOpen_error:
  hs_adc_close();
  return ;
}

int16_t hs_adc_getInteger(uint16_t data)
{
  data &= ADC_DATA_MASK;

  if(!(data & ADC_SIGNED_MASK))
    return data;

  return (int16_t)(data | ADC_DATA_COMPLEMENT);
}

void hs_adc_close(void)
{
  uint32_t i;

  adcAcquireBus(ADC_HANDLER);
  if(g_pstAdcPara)
  {
    adcStopConversion(ADC_HANDLER);

    for(i=0; i<ADC_CHANNEL_NUM; i++)
    {
      if(g_pstAdcPara->pstChnPara[i])
        adcDeleteChannel(ADC_HANDLER, i);
    }
  }

  adcStop(ADC_HANDLER);
  adcReleaseBus(ADC_HANDLER);
}

void _adc_getAdjust(hs_adcpara_t *pstAdcPara)
{
  hs_adc_adjust_t *pstAdjust = (hs_adc_adjust_t *)0x800ff000u;
  uint8_t *pu8Ptr = (uint8_t *)pstAdjust;
  uint32_t i, u32Sum = 0;

  for(i=0; i<(sizeof(hs_adc_adjust_t) - 4); i++)
    u32Sum += pu8Ptr[i];

  if(u32Sum == pstAdjust->u32Chk)
  {
    pstAdcPara->gainMod  = pstAdjust->fGainMod;
    pstAdcPara->mvoffset = pstAdjust->fOffset;
  }
  else
  {
    pstAdcPara->gainMod  = 1.0;
    pstAdcPara->mvoffset = 0.0;
  }
}

void hs_adc_getCalAdjust(uint8_t en)
{
  if(en)
    _adc_getAdjust(g_pstAdcPara);
  else
  {
    g_pstAdcPara->gainMod  = 1.0;
    g_pstAdcPara->mvoffset = 0.0;
  }

  hs_printf("offset:%f, gain: %f\r\n", g_pstAdcPara->mvoffset, g_pstAdcPara->gainMod);
}

/*
 * @brief                 request to open adc
 *                      
 * @param[in] trigerMode  0-software, 1-hardware, suggest to use software mode.
 * @param[in] swpEnable   0-disable,  1-enable
 *                        .
 */
int hs_adc_open(uint8_t trigerMode, uint8_t swpEnable)
{
  if(!g_pstAdcPara)
  {
    g_pstAdcPara = (hs_adcpara_t *)hs_malloc(sizeof(hs_adcpara_t), __MT_Z_GENERAL);
    if(!g_pstAdcPara)
      return -1;
    
    _adc_getAdjust(g_pstAdcPara);
    hs_printf("offset:%f, gain: %f\r\n", g_pstAdcPara->mvoffset, g_pstAdcPara->gainMod);
  }

  g_pstAdcPara->trigerMode   = trigerMode;
  g_pstAdcPara->swpEnable    = swpEnable;
  if(trigerMode)
    g_pstAdcPara->trigerSrc  = 
                          #if HS_PWM_USE_TIM2
                          2 * 4;
                          #else 
                            #if HS_PWM_USE_TIM1
                            1 * 4;
                            #else 
                              #if HS_PWM_USE_TIM0
                              0;
                              #else
                              12;
                              #endif
                            #endif
                          #endif
    
  _adc_serviceOpen(g_pstAdcPara);
  return (g_pstAdcPara ? 0 : -1);
}

int hs_adc_addChn(adc_channel_t chn, hs_adcgain_t gain, hs_adcvcm_t vcm, uint8_t chopper_en)
{
  adc_chn_para_t *pstChnPara;

  adcAcquireBus(ADC_HANDLER);
  if((!g_pstAdcPara) || (chn >= ADC_CHANNEL_NUM)
     || (gain >= ADC_GAIN_MAX) || (vcm >= ADC_VCM_MAX))
  {
    adcReleaseBus(ADC_HANDLER);
    return -1;
  }

  if(g_pstAdcPara->pstChnPara[chn])
  {
    adcDeleteChannel(ADC_HANDLER, chn);
    hs_free(g_pstAdcPara->pstChnPara[chn]);
    g_pstAdcPara->pstChnPara[chn] = NULL;
  }

  pstChnPara = (adc_chn_para_t *)hs_malloc(sizeof(adc_chn_para_t), __MT_Z_GENERAL);
  if(!pstChnPara)
  {
    adcReleaseBus(ADC_HANDLER);
    return -1;
  }

  pstChnPara->en_chop        = chopper_en;
  pstChnPara->gtune          = gain;
  pstChnPara->ldoctrl        = 1;
  pstChnPara->sel_inp        = 2;
  pstChnPara->sel_vcm        = vcm;
  pstChnPara->en_count_sar   = 1;
  pstChnPara->en_dem_sar     = 1;
  pstChnPara->en_sar_ckdelay = 9;
  pstChnPara->en_r2r         = 1;
  
  g_pstAdcPara->pstChnPara[chn] = pstChnPara;
  if(0 != adcAddChannel(ADC_HANDLER, chn, pstChnPara))
  {
    g_pstAdcPara->pstChnPara[chn] = NULL;
    hs_free(pstChnPara);
    adcReleaseBus(ADC_HANDLER);
    return -1;
  }

  adcReleaseBus(ADC_HANDLER);
  return 0;
}

int hs_adc_delChn(uint8_t chn)
{
  adcAcquireBus(ADC_HANDLER);
  
  if((!g_pstAdcPara) || (chn >= ADC_CHANNEL_NUM))
  {
    adcReleaseBus(ADC_HANDLER);
    return -1;
  }

  adcDeleteChannel(ADC_HANDLER, chn);
  hs_free(g_pstAdcPara->pstChnPara[chn]);
  g_pstAdcPara->pstChnPara[chn] = NULL;

  adcReleaseBus(ADC_HANDLER);
  return 0;
}

uint32_t hs_adc_getChnData(uint8_t chn)
{
  uint32_t val;

  adcAcquireBus(ADC_HANDLER);
  
  if((!g_pstAdcPara) || (chn >= ADC_CHANNEL_NUM)
      || (!g_pstAdcPara->pstChnPara[chn]))
  {
    adcReleaseBus(ADC_HANDLER);
    return 0;
  }

  adcStartConversion(ADC_HANDLER);

  val = adcGetChnData(ADC_HANDLER, chn, TIME_INFINITE);

  adcReleaseBus(ADC_HANDLER);
  return val;
}

int hs_adc_getDataByDma(uint16_t u16ChnMap, uint8_t *pu8Buf, uint32_t u32Len)
{
  uint32_t i, u32TmpMap;
  uint32_t u32DmaLen;
  uint8_t *pu8Ptr;
  int res = 0;

  adcAcquireBus(ADC_HANDLER);
  if(u16ChnMap != 0)
  {
    u32TmpMap = u16ChnMap;
    for(i=0; i<ADC_CHANNEL_NUM; i++)
    {
      if(((u32TmpMap & 1) == 0) && (g_pstAdcPara->pstChnPara[i]))
        adcDeleteChannel(ADC_HANDLER, i);

      u32TmpMap >>= 1;
    }
  }

  u32DmaLen = u32Len > ADC_MDA_MAXLEN ? ADC_MDA_MAXLEN : u32Len;
  pu8Ptr = (uint8_t *)hs_malloc(u32DmaLen, __MT_Z_DMA);
  if(!pu8Ptr)
  {
    adcReleaseBus(ADC_HANDLER);
    return -1;
  }

  while(u32Len)
  {
    res = adcStartConversionWithDMA(ADC_HANDLER, (uint8_t *)pu8Ptr, u32DmaLen);
    if(res != 0)
      break;

    res = adcDmaWaitForDone(ADC_HANDLER, TIME_INFINITE);
    if(res != 0)
      break;

    memcpy(pu8Buf, pu8Ptr, u32DmaLen);
    pu8Buf += u32DmaLen;
    u32Len -= u32DmaLen;

    u32DmaLen = u32Len > ADC_MDA_MAXLEN ? ADC_MDA_MAXLEN : u32Len;
  }

  hs_free(pu8Ptr);
  if(u16ChnMap != 0)
  {
    u32TmpMap = u16ChnMap;
    for(i=0; i<ADC_CHANNEL_NUM; i++)
    {
      if(((u32TmpMap & 1) == 0) && (g_pstAdcPara->pstChnPara[i]))
        adcAddChannel(ADC_HANDLER, i, g_pstAdcPara->pstChnPara[i]);

      u32TmpMap >>= 1;
    }
  }

  adcReleaseBus(ADC_HANDLER);
  return res;
}

void hs_adc_adjust(uint16_t msg, void *parg)
{
  (void)msg;
  uint32_t voltMv = (uint32_t)parg;
  uint16_t i, data;
  int sdata;
  adc_channel_t chn;

  if(0 != hs_adc_open(0, 0))
    return ;

  voltMv = voltMv == 0 ? 4000 : voltMv;
  chn = ADC_CHANNEL_CHIP_VCM;
  if(0 == hs_adc_addChn(chn, ADC_TEST_GAIN_AMP_MULTI_1,
                             ADC_VCM_SEL_000mV, 0))
  {
    data = 0;
    for(i=0; i<32; i++)
    {
      data += hs_adc_getChnData(chn);
      msleep(1);
    }

    data /= 32;
    sdata = hs_adc_getInteger(data);
    g_pstAdcPara->mvoffset = (float)sdata / 2048 * 1000.0;
  }

  hs_adc_delChn(chn);
  
  chn = ADC_CHANNEL_CHIP_BATTERY;
  if(0 == hs_adc_addChn(chn, ADC_TEST_GAIN_AMP_MULTI_0P5,
                             ADC_VCM_SEL_000mV, 0))
  {
    float volt;
    
    data = 0;
    for(i=0; i<32; i++)
    {
      data += hs_adc_getChnData(chn);
      msleep(1);
    }

    data /= 32;
    sdata = hs_adc_getInteger(data);

    volt = (float)sdata / 2048 * 1000.0 - g_pstAdcPara->mvoffset;
    g_pstAdcPara->gainMod = voltMv / (volt * 2 * 3);
  }

  hs_adc_delChn(chn);
  adcStop(ADC_HANDLER);

  hs_printf("offset:%f, gain_mod:%f \r\n", g_pstAdcPara->mvoffset, g_pstAdcPara->gainMod);
}

void hs_adc_getAdjust(float *mvoffset, float *gainMod)
{
  if(!g_pstAdcPara)
  {
    *mvoffset = 0;
    *gainMod  = 1;
    return ;
  }
  
  *mvoffset = g_pstAdcPara->mvoffset;
  *gainMod  = g_pstAdcPara->gainMod;
}

uint32_t hs_adc_getKeyMap(void)
{
  float vi, vf;
  uint32_t i, u32KeyMap = 0;

  if(!g_pstAdcPara)
    return 0;

  if((g_pstAdcPara->adcKeyPin <= ADC_CHANNEL_EXTERN_PIN4)
    && (g_pstAdcPara->adcKeyPin >= ADC_CHANNEL_EXTERN_PIN0))
  {
    vi = _adc_getAdcKeyVolt(g_pstAdcPara->adcKeyPin);
    vf = 1000.0 / g_fGain[g_pstAdcPara->pstDrvCfg->eAdcKeyGain]; 
    if(vi <= (vf - __adckey_getRange()))
    {
      for(i=0; i<HS_ADCKEY_MAXNUM; i++)
      {
        if(abs(vi - hs_boardGetKeyVolt(i)) < __adckey_getRange())
          u32KeyMap |= (1u << (24 + i));
      }
    }
  }

  if((g_pstAdcPara->adcKeyPinEx <= ADC_CHANNEL_EXTERN_PIN4)
    && (g_pstAdcPara->adcKeyPinEx >= ADC_CHANNEL_EXTERN_PIN0))
  {
    vi = _adc_getAdcKeyVolt(g_pstAdcPara->adcKeyPinEx);
    vf = 1000.0 / g_fGain[g_pstAdcPara->pstDrvCfg->eAdcKeyGain]; 
    if(vi <= (vf - __adckey_getRange()))
    {
      for(i=0; i<HS_ADCKEY_MAXNUM; i++)
      {
        if(abs(vi - hs_boardGetKeyVolt(i)) < __adckey_getRange())
          u32KeyMap |= (1u << (16 + i));
      }
    }
  }    

#if 0
  vi = _adc_getAdcKeyVolt(g_pstAdcPara->adcKeyPin);
  vf = 1000.0 / boardGetAdcKeyGain();
  if(vi > (vf - __adckey_getRange()))
    return 0;
      
  for(i=0; i<HS_ADCKEY_MAXNUM; i++)
  {
    if(abs(vi - hs_boardGetKeyVolt(i)) < __adckey_getRange())
      return (1u << (PIN_NUM + i));
  }
#endif

  return u32KeyMap;
}

float hs_adc_getBatteryVolt(void)
{
  int16_t s16Data;
  float vi, vd;

  s16Data = hs_adc_getInteger(hs_adc_getChnData(ADC_CHANNEL_CHIP_BATTERY));

  vd = s16Data * 1000.0 / 2048;
  vi = (vd - g_pstAdcPara->mvoffset) * g_pstAdcPara->gainMod / 0.5;
  vi *= 3;
  
  return vi;
}

float hs_adc_getTemperature(void)
{
  int16_t s16Data;
  float vi, vd, vt;

  s16Data = hs_adc_getInteger(hs_adc_getChnData(ADC_CHANNEL_CHIP_TEMPERATURE));

  vd = s16Data * 1000.0 / 2048;
  vi = (vd - g_pstAdcPara->mvoffset) * g_pstAdcPara->gainMod / 1;
  vt = (740 - vi) / 1.425 + 55;
  
  return vt;
}

void hs_adc_clrBatStatus(void)
{
  g_pstAdcPara->batStatus = 0;
}

void hs_adc_scanBn(void)
{
  float vi, vf;

  if(!g_pstAdcPara)
    return ;
  
  if((g_pstAdcPara->adcKeyPin <= ADC_CHANNEL_EXTERN_PIN4)
    && (g_pstAdcPara->adcKeyPin >= ADC_CHANNEL_EXTERN_PIN0))
  {
    vi = _adc_getAdcKeyVolt(g_pstAdcPara->adcKeyPin);

    vf = 1000.0 / g_fGain[g_pstAdcPara->pstDrvCfg->eAdcKeyGain]; 
    if(vi < (vf - 50))
    {
      hs_cfg_sysBnPressed(0, 0);
      //hs_printf("vi:%f\r\n", vi);
    }
  }

  if((g_pstAdcPara->adcKeyPinEx <= ADC_CHANNEL_EXTERN_PIN4)
    && (g_pstAdcPara->adcKeyPinEx >= ADC_CHANNEL_EXTERN_PIN0))
  {
    vi = _adc_getAdcKeyVolt(g_pstAdcPara->adcKeyPinEx);

    vf = 1000.0 / g_fGain[g_pstAdcPara->pstDrvCfg->eAdcKeyGain]; 
    if(vi < (vf - 50))
    {
      hs_cfg_sysBnPressed(0, 0);
      //hs_printf("vi:%f\r\n", vi);
    }
  }    
}

void hs_adc_scan(void)
{
  uint16_t u16Status;

  if(!g_pstAdcPara)
    return ;

  hs_adc_scanBn();
  if((g_pstAdcPara->counter % 2) == 0)
  {
    float batVolt;

    batVolt = hs_adc_getBatteryVolt();
    g_pstAdcPara->batVoltTo += batVolt;

    if((g_pstAdcPara->counter % 100) == 0)
    {
      g_pstAdcPara->batVolt = g_pstAdcPara->batVoltTo / 100;
      uint32_t arg = (uint32_t)g_pstAdcPara->batVolt;
      hs_cfg_systemReqArg(HS_CFG_EVENT_BATTERY_CHANGED, (void *)arg);

      g_pstAdcPara->batVoltTo = 0;
    }
      
    u16Status = _adc_getBatteryStatus(batVolt);
    if(u16Status != g_pstAdcPara->batStatus)
    { 
      if(g_pstAdcPara->batStatusCnt ++ > 50)
      {
        if(u16Status != 0)
          hs_cfg_systemReq(u16Status);

        g_pstAdcPara->batStatusCnt = 0;
        g_pstAdcPara->batStatus = u16Status;
      }
    }
    else
    {
      g_pstAdcPara->batStatusCnt = 0;
    }
  }

  if((g_pstAdcPara->counter % 30) == 0)
  {
    u16Status = _adc_getTempStatus(hs_adc_getTemperature());
    if((u16Status != 0) && (u16Status != g_pstAdcPara->batStatus))
    {
      g_pstAdcPara->tempStatus = u16Status;
      hs_cfg_systemReq(u16Status);
    }
  }

  g_pstAdcPara->counter += 1;
}

void hs_adc_init(void)
{
  if(0 != hs_adc_open(0, 0))
    return ;  

  g_pstAdcPara->pstDrvCfg   = hs_boardGetDrvCfg();
  g_pstAdcPara->adcKeyPin   = g_pstAdcPara->pstDrvCfg->u8AdcKeyPin + ADC_CHANNEL_CHIP_VCM;
  g_pstAdcPara->adcKeyPinEx = g_pstAdcPara->pstDrvCfg->u8AdcKeyPinEx + ADC_CHANNEL_CHIP_VCM;
  
  if((g_pstAdcPara->adcKeyPin <= ADC_CHANNEL_EXTERN_PIN4)
    && (g_pstAdcPara->adcKeyPin >= ADC_CHANNEL_EXTERN_PIN0))
    hs_adc_addChn(g_pstAdcPara->adcKeyPin, g_pstAdcPara->pstDrvCfg->eAdcKeyGain, ADC_VCM_SEL_000mV, 0);

  if((g_pstAdcPara->adcKeyPinEx <= ADC_CHANNEL_EXTERN_PIN4)
    && (g_pstAdcPara->adcKeyPinEx >= ADC_CHANNEL_EXTERN_PIN0))
    hs_adc_addChn(g_pstAdcPara->adcKeyPinEx, g_pstAdcPara->pstDrvCfg->eAdcKeyGain, ADC_VCM_SEL_000mV, 0);

  hs_adc_addChn(ADC_CHANNEL_CHIP_TEMPERATURE, ADC_GAIN_MULTI_1, ADC_VCM_SEL_000mV, 0);
  hs_adc_addChn(ADC_CHANNEL_CHIP_BATTERY, ADC_GAIN_MULTI_0P5, ADC_VCM_SEL_000mV, 0);
}

#endif

/** @} */
