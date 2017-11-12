/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio
              Copyright (C) 2015 Huntersun Technologies
              wei.lu@huntersun.com.cn

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

#ifndef _BT_CONFIG_H_
#define _BT_CONFIG_H_

#include "lib.h"
#include "bthost_uapi.h"
#include "bthc_uapi.h"

typedef void (*APP_CFG_ActionFunc)(void);

struct AppBtActionHandleStru {
  uint32_t actionIndex;
  APP_CFG_ActionFunc eventAction;
};

/*===========================================================================*/
/* APP CFG API                                                               */
/*===========================================================================*/

void     App_CFG_Init(uint8_t mode);
void     App_CFG_Uninit(void);

uint8_t  *App_CFG_GetDeviceName(void);
uint8_t  *App_CFG_GetDeviceAddr(void);

#define App_CFG_GetPtsEnable()    (g_bt_host_config.features.ptsTestEnable & 0x01)
#define App_CFG_GetPtsMpsEnable() (g_bt_host_config.features.ptsTestEnable & 0x02)
#define App_CFG_GetHfpStereoEnable() (g_bt_host_config.attrs.hfpStereoEnable & 0x01)

// event ->action
APP_CFG_ActionFunc App_CFG_GetAction(uint8_t index);

// pair info
uint8_t App_CFG_GetPairInfo(uint8_t index, void *info);
void App_CFG_AddPairInfo(void *info);
void App_CFG_ClearPairInfo(void);
void App_CFG_UpdatePairInfo(void *info);

// backup linkkey
void hsc_StoreBackUpLinkKey(uint8_t* data);
void hsc_HandleBackUpLinkKey(void);
void hsc_StoreBackUpHIDLinkKey(uint8_t* data);
void hsc_HandleBackUpHIDLinkKey(void);

// vol
void App_CFG_SetHfpVol(uint8_t spkval, uint8_t micval);
void hsc_CFG_SaveA2dpVol(uint8_t val);

// hid info
void hsc_CFG_GetHidInfo(struct AppBtHidInfoStru *info);

// host controller
void App_CFG_GetBtHcRf(uint8_t mode);
#if HS_USE_BT == HS_BT_DATA_TRANS
void App_CFG_GetBtHost(void* arg);
void App_CFG_SetBtHostVisibility(uint8_t visibility);
void App_CFG_SetBtHostName(uint8_t len, uint8_t *name);
#endif
#endif
