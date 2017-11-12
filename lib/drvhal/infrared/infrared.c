/*
    drvhal - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/drvhal/infrared.c
 * @brief   pwm file.
 * @details 
 *
 * @addtogroup  lib drvhal
 * @details 
 * @{
 */

#include "lib.h"

#define INFRARED_CAPTURE_BITWIDTH   32

#define INFRARED_CAPTURE_TRIGER     (0x1u << 0)
#define INFRARED_CAPTURE_DECODE     (0x1u << 1)


typedef struct
{
  ICUConfig         stIcuCfg;
  osThreadId        pstThd;

  uint8_t           u8IsBusy;
  
  uint16_t          u16Offset;
  uint32_t          u32Code;
  
  const hs_infrared_para_t   *pstPinPara;
  const hs_infrared_dec_t    *pstDecTable;
  uint32_t                    u32DecTableLen;
}hs_infrared_t;

static const hs_infrared_para_t g_stPinPara = 
{
  PB5, 2, 1, PAD_FUNC_TIMER2_3
};

static const hs_infrared_dec_t  g_stDecTable[] = 
{
  {8, 0x95, 0, 30},
  {8, 0x95, 0, 31},
  {8, 0x95, 0, 32},
  {8, 0x95, 0, 33},
};

const hs_infrared_para_t *hs_infrared_getPinPara(void)
{
  return &g_stPinPara;
}

const hs_infrared_dec_t  *hs_infrared_getDecTable(void)
{
  return g_stDecTable;
}

uint32_t hs_infrared_getDecTableLen(void)
{
  return sizeof(g_stDecTable) / sizeof(hs_infrared_dec_t);
}

static hs_infrared_t *g_pstInfrared;

static void _infrared_padInit(const hs_infrared_para_t *pstPara)
{
  hs_padinfo_t stPad;

  if(!pstPara)
    return ;

  stPad.u16PadIdx = pstPara->u8PinIdx;
  stPad.u8PadDir  = CFG_PAD_DIR_INPUT;
  stPad.u8PadDrvCap = 3;
  stPad.u8PadMode = pstPara->u8FunIdx;
  stPad.u8PadPull = CFG_PAD_PULL_NO;

  hs_pad_config(&stPad);
}

static void _infrared_icuCb(ICUDriver *icup)
{
  hs_infrared_t *pstInfrared = g_pstInfrared;
  uint32_t u32BitWidth;
  
  u32BitWidth = icuGetWidth(icup);
  if((u32BitWidth > 430) && (u32BitWidth < 500))
    return ;

  if((u32BitWidth > 160) && (u32BitWidth < 186))
    pstInfrared->u32Code |= 1;

  pstInfrared->u32Code  <<= 1;
  pstInfrared->u16Offset += 1;

  oshalSignalSet(pstInfrared->pstThd, INFRARED_CAPTURE_DECODE);
}

static void _infrared_rxTriger(ioportid_t port, uint8_t pad)
{
  (void)port;
  (void)pad;
  hs_infrared_t *pstInfrared = g_pstInfrared;
  
  if((!pstInfrared) || (pstInfrared->u8IsBusy))
    return ;

  oshalSignalSet(pstInfrared->pstThd, INFRARED_CAPTURE_TRIGER);
}

static ICUDriver * _infrared_getIcuHandler(hs_infrared_t *pstInfrared)
{
  ICUDriver *pstIcuDrv;

  pstIcuDrv = 
    #if HS_ICU_USE_TIM0
    (pstInfrared->pstPinPara->u8IcuIdx == 0) ? &ICUD0 : 
    #endif
      #if HS_ICU_USE_TIM1
      (pstInfrared->pstPinPara->u8IcuIdx == 1) ? &ICUD1 : 
      #endif
        #if HS_ICU_USE_TIM2
        (pstInfrared->pstPinPara->u8IcuIdx == 2) ? &ICUD2 : 
        #endif
          NULL;
        
  return pstIcuDrv;
}

static void _infrared_initIcuCfg(hs_infrared_t *pstInfrared)
{
  ICUConfig *pstIcuCfg = &pstInfrared->stIcuCfg;

  pstInfrared->pstPinPara  = hs_infrared_getPinPara();
  pstInfrared->pstDecTable = hs_infrared_getDecTable();
  pstInfrared->u32DecTableLen = hs_infrared_getDecTableLen();
  
  pstIcuCfg->mode      = ICU_INPUT_ACTIVE_HIGH;
  pstIcuCfg->frequency = 100000;
  pstIcuCfg->width_cb  = _infrared_icuCb;

  pstIcuCfg->channel   = pstInfrared->pstPinPara->u8ChnIdx;
  pstIcuCfg->dier      = 0;
}

static void _infrared_decReinit(hs_infrared_t *pstInfrared)
{
  pstInfrared->u16Offset = 0;
  pstInfrared->u32Code   = 0;
}

void _infrared_serThread(void *arg)
{
  hs_infrared_t *pstInfrared = (hs_infrared_t *)arg;
  ICUDriver *pstIcuDrv = _infrared_getIcuHandler(pstInfrared);
  osEvent eStaus;

  chRegSetThreadName("infraredService");
  if(!pstIcuDrv)
    return ;
  
  icuStart(pstIcuDrv, &pstInfrared->stIcuCfg);
  
  while(1)
  {
    oshalSignalClear(pstInfrared->pstThd, INFRARED_CAPTURE_TRIGER);
    oshalSignalWait(INFRARED_CAPTURE_TRIGER, -1);

    _infrared_decReinit(pstInfrared);
    pstInfrared->u8IsBusy = 1;
    
		icuEnable(pstIcuDrv);

    oshalSignalClear(pstInfrared->pstThd, INFRARED_CAPTURE_DECODE);
    eStaus = oshalSignalWait(INFRARED_CAPTURE_DECODE, 10000);
    if(osEventTimeout != eStaus.status)
    {
      hs_printf("Decode:0x%08x \r\n", pstInfrared->u32Code);
    }

    icuDisable(pstIcuDrv);
    pstInfrared->u8IsBusy = 0;    
  }

  icuStop(pstIcuDrv);
}

void hs_infrared_serStart(uint16_t u16Idx, void *parg)
{
  (void)u16Idx;
  (void)parg;
  hs_infrared_t *pstInfrared;
  osThreadDef_t thdDef;

  pstInfrared = hs_malloc(sizeof(hs_infrared_t), __MT_Z_GENERAL);
  if(!pstInfrared)
    return ;

  _infrared_initIcuCfg(pstInfrared);
  _infrared_padInit(pstInfrared->pstPinPara);

  thdDef.pthread   = (os_pthread)_infrared_serThread;
  thdDef.stacksize = 1024;
  thdDef.tpriority = osPriorityNormal;
  pstInfrared->pstThd = oshalThreadCreate(&thdDef, pstInfrared);
  if(!pstInfrared->pstThd)
  {
    hs_free(pstInfrared);
    return ;
  }

  g_pstInfrared = pstInfrared;
  palRegEvent(pstInfrared->pstPinPara->u8PinIdx, FALLING_EDGE, (ioevent_t)_infrared_rxTriger);
}

