/*
    drvhal - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/drvhal/pad.c
 * @brief   pad config file.
 * @details 
 *
 * @addtogroup  drvhal
 * @details 
 * @{
 */

#include "lib.h"

void hs_pad_config(const hs_padinfo_t *pstPadInfo)
{
  ioportid_t pstPort;
  uint32_t u32PadIdx, u32Mode = 0;
  uint8_t u8PadMode = pstPadInfo->u8PadMode;
  
  if(pstPadInfo->u16PadIdx >= PB0)
  {
    pstPort = IOPORT1;
    u32PadIdx = pstPadInfo->u16PadIdx - PB0;
  }
  else
  {
    pstPort = IOPORT0;
    u32PadIdx = pstPadInfo->u16PadIdx;
  }

  u32Mode = PAL_MODE_ALTERNATE(u8PadMode);
  u32Mode |= PAL_MODE_DRIVE_CAP(pstPadInfo->u8PadDrvCap);
  u32Mode |= pstPadInfo->u8PadDir == CFG_PAD_DIR_INPUT ? 
              PAL_HS_MODE_INPUT : PAL_HS_MODE_OUTPUT;
  u32Mode |= pstPadInfo->u8PadPull == CFG_PAD_PULL_NO ? PAL_HS_PUDR_NOPUPDR :
                pstPadInfo->u8PadPull == CFG_PAD_PULL_UP ? PAL_HS_PUDR_PULLUP :
                  PAL_HS_PUDR_PULLDOWN;
  
  palSetPadMode(pstPort, u32PadIdx, u32Mode);
}

int hs_pad_init(void)
{
  uint32_t u32Offset;
  hs_padinfo_t stPad;
  hs_cfg_res_t enRes;
 
  u32Offset = 0;
  while(1)
  {
    enRes = hs_cfg_getPartDataByIndex(HS_CFG_PAD_CONFIG, (uint8_t *)&stPad, sizeof(hs_padinfo_t), u32Offset);
    if(enRes != HS_CFG_OK)
    {
      break;
    }
    
    if(stPad.u16PadIdx > 0x20)
    {
      break;
    }

    hs_pad_config(&stPad);
    u32Offset += sizeof(hs_padinfo_t);
  }

  return (int)enRes;
}

/** @} */
