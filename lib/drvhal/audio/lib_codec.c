/*
    drvhal - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/drvhal/codec.c
 * @brief   codec file.
 * @details 
 *
 * @addtogroup  lib drvhal
 * @details 
 * @{
 */

#include "lib.h"
#include "math.h"


#if HAL_USE_CODEC

enum
{
  EQ_TYPE_LOWBAND       = 0,
  EQ_TYPE_MIDBAND       ,
  EQ_TYPE_HIGHBAND      ,
};

typedef struct
{
  uint8_t     u8HoldStatus;
  uint8_t     u8LastStatus;
  uint8_t     u8Cnt;
}hs_codecryth_t;

typedef struct
{
  uint32_t    u32Samp;
  int32_t     s32Gain;

  uint32_t    u32Freq;
  uint32_t    u32Band;
}hs_codec_eqpoint_t;

typedef struct
{
  uint32_t    u32Coeff;
  uint16_t    u16Gain;
}hs_codec_eqreg_t;

typedef void (* pfnSetGain_t)(uint8_t);
typedef void (* pfnSetCoeff_t)(uint32_t);

static const pfnSetGain_t pfnSetGain[EQ_BAND_NUM] =
{
  audioSetBand1Gain,
  audioSetBand2Gain,
  audioSetBand3Gain,
  audioSetBand4Gain,
  audioSetBand5Gain,
  audioSetBand6Gain,
  audioSetBand7Gain
};

static const pfnSetCoeff_t pfnSetCoeff[EQ_BAND_NUM] =
{
  (pfnSetCoeff_t)audioSetBand1Coeff,
  audioSetBand2Coeff,
  audioSetBand3Coeff,
  audioSetBand4Coeff,
  audioSetBand5Coeff,
  audioSetBand6Coeff,
  (pfnSetCoeff_t)audioSetBand7Coeff,
};

#if HS_USE_AUX
typedef struct
{
  uint8_t     u8HoldCnt;
  uint8_t     u8HoldStatus;
  uint8_t     u8LastStatus;
  uint8_t     u8Cnt;
}hs_codecpara_t;

static hs_codecpara_t g_stCodecPara = { 10, 1, 1, 0};
static hs_codecryth_t g_stCodecCryth;

void _codec_config_audio_pa(CODECDriver *codecp, uint8_t enable);
uint16_t hsc_A2DP_GetState(void);

void _codec_scanVolume(void)
{
  hs_codecryth_t *pstCodecCryth = &g_stCodecCryth;
  uint8_t u8Status = 0 == audioGetDacRms() ? 0 : 1;

  if((u8Status == pstCodecCryth->u8HoldStatus)
    && (u8Status == pstCodecCryth->u8LastStatus))
    return ;

#if HS_USE_BT
  if(0x4 != hsc_A2DP_GetState())
    return ;
#else
  return ;
#endif
  
  if(u8Status == 1)
    _codec_config_audio_pa(&CODECD, 1);

  if(u8Status == pstCodecCryth->u8HoldStatus)
  {
    pstCodecCryth->u8Cnt ++;
    if(pstCodecCryth->u8Cnt >= 10)
    {
      pstCodecCryth->u8LastStatus = u8Status;
      if(u8Status == 0)
        _codec_config_audio_pa(&CODECD, 0);
    }
  }
  else
  {
    pstCodecCryth->u8Cnt = 0;
    pstCodecCryth->u8HoldStatus = u8Status;
  }
}


void _codec_scanJack(hs_codecpara_t *pstJack, uint8_t jackStatus)
{
  if((jackStatus == pstJack->u8HoldStatus)
    && (jackStatus == pstJack->u8LastStatus))
    return ;

  if(jackStatus == pstJack->u8HoldStatus)
  {
    pstJack->u8Cnt ++;
    if(pstJack->u8Cnt >= pstJack->u8HoldCnt)
    {
      pstJack->u8LastStatus = jackStatus;
      hs_cfg_sysReqPerip(HS_CFG_EVENT_JACK_PLUGIN + jackStatus);
      //hs_cfg_sysReqPerip(HS_CFG_EVENT_JACK_PLUGOUT - jackStatus);
    }
  }
  else
  {
    pstJack->u8Cnt = 0;
    pstJack->u8HoldStatus = jackStatus;
  }
}

static uint32_t _codec_getPlugin(void)
{
  ioportid_t pstPort;
  const hs_drvhal_cfg_t *pstDrvCfg = hs_boardGetDrvCfg();
  uint32_t u32Pin = pstDrvCfg->u8AuxDetPin;
  
  if(u32Pin >= PB0)
  {
    pstPort = IOPORT1;
    u32Pin -= PB0;
  }
  else
  {
    pstPort = IOPORT0;
  }

  palSetPadMode(pstPort, u32Pin, 
                PAL_MODE_INPUT_PULLUP | PAL_MODE_ALTERNATE(PAD_FUNC_GPIO)|PAL_MODE_DRIVE_CAP(1));
  
  return pstDrvCfg->u8AuxDetLvl == palReadPad(pstPort, u32Pin) ? 0 : 1;
}
#endif

void _codec_eqCal(uint8_t u8BandType, hs_codec_eqpoint_t *pstPoint, hs_codec_eqreg_t *pstRegV)
{
  double pi = 3.141592653589793;
  double coeff_bit = 16384;
  double v0;
  double coeff_a=0;
  double coeff_b=0;
  double temp1, temp;

  if(pstPoint->u32Freq >= (pstPoint->u32Samp / 2))
  {
    pstRegV->u16Gain  = 0;
    pstRegV->u32Coeff = 0;
    return ;
  }

  v0 = pow((double)10,(double)((double)pstPoint->s32Gain / (double)20));
  temp1 = (double)(pi * pstPoint->u32Freq / pstPoint->u32Samp);

  if(u8BandType == EQ_TYPE_LOWBAND)
  {
    if(pstPoint->s32Gain >= 0)
      coeff_a = (tan(temp1)-1)/(tan(temp1)+1);
    else
      coeff_a = (tan(temp1)-v0)/(tan(temp1)+v0);
	}
  else if(u8BandType == EQ_TYPE_HIGHBAND)
  {
    if(pstPoint->s32Gain >= 0)
      coeff_a = (tan(temp1)-1)/(tan(temp1)+1);
    else
      coeff_a = (v0*tan(temp1)-1)/(v0*tan(temp1)+1);
	}
  else
  {
    temp = (double)(pi * pstPoint->u32Band / pstPoint->u32Samp);
    if(pstPoint->s32Gain >= 0)
      coeff_a = (tan(temp)-1)/(tan(temp)+1);
    else
      coeff_a = (tan(temp)-v0)/(tan(temp)+v0);

    temp1 = (double)(2 * pi * pstPoint->u32Freq / pstPoint->u32Samp);
    coeff_b = (-cos(temp1))*(1-coeff_a);
	}

  temp = coeff_a * coeff_bit + 0.5;
  pstRegV->u32Coeff = (uint16_t)(floor(temp)) << 16;

  temp = coeff_b * coeff_bit + 0.5;
  pstRegV->u32Coeff |= (uint16_t)(floor(temp)) & 0xffff;

  temp = (v0 - 1) / 2;
  temp = temp * coeff_bit + 0.5;
  pstRegV->u16Gain = (short)(floor(temp));
}

void hs_codec_setPointEq(uint8_t pointIdx, uint32_t freq, int32_t gain, uint32_t bandWidth)
{
  hs_codec_eqpoint_t stPoint;
  hs_codec_eqreg_t stRegV;
  uint32_t samp = audioGetPlaySampleRate();

  if(pointIdx >= EQ_BAND_NUM)
    return ;
  
  if(!samp)
    samp = 44100;

  gain = gain > 12 ? 12 : gain;
  gain = gain < -12 ? -12 : gain;

  stPoint.u32Samp = samp;
  stPoint.u32Freq = freq;
  stPoint.s32Gain = gain;
  stPoint.u32Band = bandWidth;

  if(pointIdx == 0)
  {
    _codec_eqCal(EQ_TYPE_LOWBAND, &stPoint, &stRegV);
    stRegV.u32Coeff >>= 16;
  }
  else if(pointIdx == EQ_BAND_NUM - 1)
  {
    _codec_eqCal(EQ_TYPE_HIGHBAND, &stPoint, &stRegV);
    stRegV.u32Coeff >>= 16;
  }
  else
  {
    _codec_eqCal(EQ_TYPE_MIDBAND, &stPoint, &stRegV);
  }

  pfnSetGain[pointIdx](stPoint.s32Gain);
  pfnSetCoeff[pointIdx](stRegV.u32Coeff);

  audioSetEqEnable(1);
}

void hs_codec_setEq(const hs_codec_eqpara_t *pstEq)
{
  hs_codec_eqpoint_t stPoint;
  hs_codec_eqreg_t stRegV;
  uint32_t i;
  uint32_t samp = audioGetPlaySampleRate();

  if(!pstEq)
    return ;

  if(!samp)
	  samp = 44100;

  stPoint.u32Samp = samp;
  for(i=0; i<EQ_BAND_NUM; i++)
  {
    stPoint.u32Freq = pstEq->u32Freq[i];
    stPoint.s32Gain = pstEq->s32Gain[i];

    stPoint.s32Gain = stPoint.s32Gain > 12 ? 12 : stPoint.s32Gain;
    stPoint.s32Gain = stPoint.s32Gain < -12 ? -12 : stPoint.s32Gain;

    if(i == 0)
    {
      _codec_eqCal(EQ_TYPE_LOWBAND, &stPoint, &stRegV);
      stRegV.u32Coeff >>= 16;
    }
    else if(i == EQ_BAND_NUM - 1)
    {
      _codec_eqCal(EQ_TYPE_HIGHBAND, &stPoint, &stRegV);
      stRegV.u32Coeff >>= 16;
    }
    else
    {
      stPoint.u32Band = (pstEq->u32Freq[i] - pstEq->u32Freq[i-1]) / 2;
      _codec_eqCal(EQ_TYPE_MIDBAND, &stPoint, &stRegV);
    }

    pfnSetGain[i](stPoint.s32Gain);
    pfnSetCoeff[i](stRegV.u32Coeff);
  }

  audioSetEqEnable(1);
}

void hs_codec_scan(void)
{
  #if HS_USE_AUX
  _codec_scanJack(&g_stCodecPara, (_codec_getPlugin() & 1));
  #endif

  _codec_scanVolume();
}

__USED bool hs_codec_getPlugin(void)
{
  return ((g_stCodecPara.u8HoldStatus == g_stCodecPara.u8LastStatus) 
             && (g_stCodecPara.u8HoldStatus == 0));
}

#endif

/** @} */
