#include "lib.h"
#include "bt_config.h"
#include "bt_os.h"

//-----------------------------------------------------
static bt_host_config_t   *p_host_cfg;
static bt_hc_sys_config_t *p_sys_config;
static bt_hc_phy_config_t *p_sys_rf_cfg;

static const char devName[] = {0xe6, 0xb1, 0x89, 0xe5, 0xa4, 0xa9, 0xe4, 0xb8, 0x8b, 'H', 'S', '6', '6', '0', '1', '\0'}; //ººÌìÏÂ6601
static const uint8_t classOfDevice[] = {0x08, 0x04, 0x20};    // Hands-free, Card Audio
//static const uint8_t classOfDevice[] = {0x24, 0x04, 0x1c};  //Portable Audio
//static const uint8_t classOfDevice[] = {0x40, 0x25, 0x00};  //HID

#if HS_USE_BT == HS_BT_AUDIO
static uint8_t isConfigInit = 0;

static const struct AppBtActionHandleStru btActionHandle[] =
{
  { APP_CFG_EVENT_ENTER_PAIR,                              APP_PairEntry },
  { APP_CFG_EVENT_RESET_PAIR_LIST,                     APP_PairResetList },
#ifdef CONFIG_HFP
  { APP_CFG_EVENT_HFP_CONENCT_BACK,                  App_FsmHfpReconnect },
  { APP_CFG_EVENT_HFP_ANSWER,                             hsc_HFP_Answer },
  { APP_CFG_EVENT_HFP_REJECT,                             hsc_HFP_Reject },
  { APP_CFG_EVENT_HFP_CACEL,                              hsc_HFP_Reject },
  { APP_CFG_EVENT_HFP_LAST_REDIAL,                        hsc_HFP_Redial },
  { APP_CFG_EVENT_HFP_ATTACH_NUMBER_TO_TAG,        hsc_HFP_DialAttachTag },
  { APP_CFG_EVENT_HFP_VOICE_RECOGNITION,     hsc_HFP_VoiceRecognitionReq },
  { APP_CFG_EVENT_HFP_CALL_ON_HOLD,             hsc_HFP_HoldIncomingCall },
  { APP_CFG_EVENT_HFP_ACCEPT_HOLD,        hsc_HFP_AcceptHeldIncomingCall },
  { APP_CFG_EVENT_HFP_REJECT_HOLD,        hsc_HFP_RejectHeldIncomingCall },
  { APP_CFG_EVENT_HFP_VOL_ADD,                         hsc_HFP_SpkVolAdd },
  { APP_CFG_EVENT_HFP_VOL_SUB,                         hsc_HFP_SpkVolSub },
  { APP_CFG_EVENT_HFP_MIC_ADD,                         hsc_HFP_MicVolAdd },
  { APP_CFG_EVENT_HFP_MIC_SUB,                         hsc_HFP_MicVolSub },
  { APP_CFG_EVENT_HFP_DISABLE_AG_EC_NR,              hsc_HFP_DisableNREC },
  { APP_CFG_EVENT_HFP_3WAY_RELEASE_ALL,       hsc_HFP_3WayReleaseAllHeldCall },
  { APP_CFG_EVENT_HFP_3WAY_ACCEPT_WAIT_RELEASE,hsc_HFP_3WayReleaseActiveCall },
  { APP_CFG_EVENT_HFP_3WAY_ACCEPT_WAIT_HOLD,      hsc_HFP_3WayHoldActiveCall },
  { APP_CFG_EVENT_HFP_3WAY_ADD_HOLD,                 hsc_HFP_3WayAddHeldCall },
  { APP_CFG_EVENT_HFP_3WAY_ECT,                          hsc_HFP_3WayEct },
  { APP_CFG_EVENT_HFP_DISCONNECT,                  hsc_HFP_DisconnectAll },
  { APP_CFG_EVENT_HFP_SCO,                             hsc_HFP_SCOHandle },
#endif
#ifdef CONFIG_A2DP
  { APP_CFG_EVENT_A2DP_CONNECT_BACK,                App_FsmA2dpReconnect },
  { APP_CFG_EVENT_A2DP_SOURCE_SELECT,              hsc_A2DP_SourceSelect },
  { APP_CFG_EVENT_A2DP_VOL_ADD,                       hsc_A2DP_SpkVolAdd },
  { APP_CFG_EVENT_A2DP_VOL_SUB,                       hsc_A2DP_SpkVolSub },
  { APP_CFG_EVENT_A2DP_PLAY,                               hsc_A2DP_Play },
  { APP_CFG_EVENT_A2DP_PAUSE,                             hsc_A2DP_Pause },
  { APP_CFG_EVENT_A2DP_STOP,                               hsc_A2DP_Stop },
  { APP_CFG_EVENT_A2DP_MUTE,                               hsc_A2DP_Mute },
  { APP_CFG_EVENT_A2DP_FORWORD,                         hsc_A2DP_Forward },
  { APP_CFG_EVENT_A2DP_BACKWORD,                       hsc_A2DP_Backward },
  { APP_CFG_EVENT_A2DP_FFWD_PRESS,                    hsc_A2DP_FFWDPress },
  { APP_CFG_EVENT_A2DP_FFWD_RELEASE,                hsc_A2DP_FFWDRelease },
  { APP_CFG_EVENT_A2DP_RWD_PRESS,                   hsc_A2DP_RewindPress },
  { APP_CFG_EVENT_A2DP_RWD_RELEASE,               hsc_A2DP_RewindRelease },
  { APP_CFG_EVENT_A2DP_STREAM_PLAY,                  hsc_A2DP_StreamPlay },
  { APP_CFG_EVENT_A2DP_STREAM_PAUSE,                 hsc_A2DP_StreamStop },
  { APP_CFG_EVENT_A2DP_STREAM_GET_CONFIG,       hsc_A2DP_StreamGetConfig },
  { APP_CFG_EVENT_A2DP_STREAM_RECONFIG,          hsc_A2DP_StreamReConfig },
  { APP_CFG_EVENT_A2DP_STREAM_DISCONNECT,      hsc_A2DP_StreamDisconnect },
  { APP_CFG_EVENT_A2DP_STREAM_ABORT,                hsc_A2DP_StreamAbort },
  { APP_CFG_EVENT_A2DP_DISCONNECT,                hsc_A2DP_DisconnectAll },
#endif
#ifdef CONFIG_SPP
  { APP_CFG_EVENT_SPP_CONNECT,                       App_FsmSppReconnect },
  { APP_CFG_EVENT_SPP_DISCONNECT,                                   NULL },
#endif
#ifdef CONFIG_HID
  { APP_CFG_EVENT_HID_CONNECT,                       App_FsmHidReconnect },
  { APP_CFG_EVENT_HID_DISCONNECT,                                   NULL },
  { APP_CFG_EVENT_HID_SEND_KEY1,                        App_HID_SendKey1 },
  { APP_CFG_EVENT_HID_SEND_KEY2,                        App_HID_SendKey2 },
  { APP_CFG_EVENT_HID_SEND_KEY3,                        App_HID_SendKey3 },
  { APP_CFG_EVENT_HID_SEND_KEY4,                        App_HID_SendKey4 },
#endif
#ifdef CONFIG_AVRCP
  { APP_CFG_EVENT_AVRCP_RECONNECT,                 App_FsmAvrcpReconnect },
  { APP_CFG_EVENT_AVRCP_PLAY,                             hsc_AVRCP_Play },
  { APP_CFG_EVENT_AVRCP_PAUSE,                           hsc_AVRCP_Pause },
  { APP_CFG_EVENT_AVRCP_STOP,                             hsc_AVRCP_Stop },
  { APP_CFG_EVENT_AVRCP_REWIND,                         hsc_AVRCP_Rewind },
  { APP_CFG_EVENT_AVRCP_FORWARD,                       hsc_AVRCP_Forward },
  { APP_CFG_EVENT_AVRCP_BACKWARD,                     hsc_AVRCP_Backward },
  { APP_CFG_EVENT_AVRCP_FFWD,                             hsc_AVRCP_FFWD },
  { APP_CFG_EVENT_AVRCP_VOLUP,                           hsc_AVRCP_VolUp },
  { APP_CFG_EVENT_AVRCP_VOLDOWN,                       hsc_AVRCP_VolDown },
#endif
  { APP_CFG_EVENT_RECONNECT,                            App_FsmReconnect },
  { APP_CFG_EVENT_DISCONNECT_ALL,                       App_FsmDisconnectAll },
  { APP_BT_VOL_ADD,                                     hsc_BtVolAdd},
  { APP_BT_VOL_SUB,                                     hsc_BtVolSub},
  { APP_CFG_EVENT_MAX,                                              NULL },
};

extern uint8_t hsc_GetBtMode(void);

//------------------------------ the interface ----------------- 
uint8_t  *App_CFG_GetDeviceAddr(void)
{
  return (uint8_t *)&p_host_cfg->device.addr;
}

uint8_t* App_CFG_GetDeviceName(void)
{
  return p_host_cfg->device.name;
}

void App_CFG_Init(uint8_t mode)
{
  bt_host_get_config(&p_host_cfg);

  /* skip bd_addr in p_host_cfg to back-compatible old cfg data */
  if (hs_cfg_getDataByIndex(HS_CFG_CLASSIC_DEV, (uint8_t *)&p_host_cfg->device.name, sizeof(p_host_cfg->device)-BD_ADDR_LEN) != HS_CFG_OK) {
    memcpy( p_host_cfg->device.name, devName, BT_NAME_LEN_MAX);
    memset( p_host_cfg->device.pincode, '0', BT_PIN_CODE_LEN);
    memcpy( p_host_cfg->device.cod, classOfDevice, BT_CLASS_OF_DEVICE_LEN);
  }
  
  memcpy(p_host_cfg->device.addr, p_sys_config->bd_addr, BD_ADDR_LEN);
#if 1
  {
     int i;
     hs_printf("\r\nbt addr: ");
     for (i=0; i<BD_ADDR_LEN; i++)
        hs_printf("%02x", p_host_cfg->device.addr[BD_ADDR_LEN-1-i]);
     if (mode == BT_HOST_VAR_MODE_HID)
     {
        struct AppBtHidInfoStru info;
        hsc_CFG_GetHidInfo(&info);
        hs_printf("\r\nbt name: %s\r\n", info.name);
     }
     else
     {
        hs_printf("\r\nbt name: %s\r\n", p_host_cfg->device.name);
     }
  }
#endif
  if (hs_cfg_getDataByIndex(HS_CFG_CLASSIC_PROFILE, (uint8_t *)&p_host_cfg->profiles, sizeof(p_host_cfg->profiles)) != HS_CFG_OK) {
    p_host_cfg->profiles.a2dp  = 1;
    p_host_cfg->profiles.avrcp = 3;
    p_host_cfg->profiles.hfp   = 1;
    p_host_cfg->profiles.hsp   = 1;
    p_host_cfg->profiles.spp   = 1;
    p_host_cfg->profiles.hid   = 18;
  }

  if (hs_cfg_getDataByIndex(HS_CFG_CLASSIC_ATTR, (uint8_t *)&p_host_cfg->attrs, sizeof(p_host_cfg->attrs)) != HS_CFG_OK) {
    p_host_cfg->attrs.sppMaxMtu = 0x100;
    p_host_cfg->attrs.gapIoCap  = 3;
    p_host_cfg->attrs.hfpStereoEnable = 1;
    p_host_cfg->attrs.hfpSpkVol = 9;
    p_host_cfg->attrs.hfpMicVol = 15;
    p_host_cfg->attrs.a2dpSpkVol = 0x64;
    p_host_cfg->attrs.connectAlert = BT_CLASSIC_PROFILE_A2DP | BT_CLASSIC_PROFILE_HFP;
    p_host_cfg->attrs.hfpScoConnCfgEnable = 1;
    p_host_cfg->attrs.hfpScoScoConnCfgRetransEffort = 0;
    p_host_cfg->attrs.hfpScoConnCfgMaxLatency = 12;
  }

  if (hs_cfg_getDataByIndex(HS_CFG_CLASSIC_ADVANCED, (uint8_t *)&p_host_cfg->features, sizeof(p_host_cfg->features)) != HS_CFG_OK) {
    p_host_cfg->features.pairTimeOut = 180;
    p_host_cfg->features.linkLostIntoPairEnable = 5; // 0: link lost not in pair mode. 1. link lost in pair mode. 2. pair mode when power on. 4. not pair when power on
     
    // link lost alert
    p_host_cfg->features.linkLostAlertEnable = 0;
    p_host_cfg->features.linkLostAlertLevel = 7;    
    p_host_cfg->features.linkLostAlertStopTimer = 3;

    // link lost retry
    p_host_cfg->features.linkLostRetryEnable = 0;
    p_host_cfg->features.linkLostRetryCount = 3;
    p_host_cfg->features.linkLostRetryTimer = 10;

    // power on
    p_host_cfg->features.powerOnAutoConnect = 1;
    p_host_cfg->features.powerOnAutoConnectProtect = 1;
    p_host_cfg->features.powerOnAutoConnectTimer = 15;
    p_host_cfg->features.powerOnStartConnectTimer = 3000;

    // pts
    p_host_cfg->features.ptsTestEnable = 0;
      
    // sniff for host
    p_host_cfg->features.sniffEnable = 0; // 0, sniff disable, 1 sniff enable, 2 sniff subrate enable
    p_host_cfg->features.sniffIdleInterval = 40000;
    p_host_cfg->features.sniffBusyInterval = 500;
    p_host_cfg->features.sniffSuspendTimeout = 500;
      
    // sniff for hci
    p_host_cfg->features.sniffMaxInterval = 800; // 0x0006 to 0x0540
    p_host_cfg->features.sniffMinInterval = 800; // 0x0006 to 0x0540
    p_host_cfg->features.sniffAttempt = 2;     // 0x0001 to 0x7FFF
    p_host_cfg->features.sniffTimeout = 3;     // 0x0000 to 0x7FFF
      
    // sniff subrating for host
    p_host_cfg->features.sniffToSubrateTimeout = 300;
    p_host_cfg->features.sniffSubrateInterval = 500;
        
    // sniff subrating for hci
    p_host_cfg->features.sniffSubrateMaxLatency = 800;
    p_host_cfg->features.sniffSubrateMiniRemoteTimeout = 0;
    p_host_cfg->features.sniffSubrateMiniLocalTimeout = 0; 
  }

  if (mode == BT_HOST_VAR_MODE_HID)
  {
      struct AppBtHidInfoStru info;
      p_host_cfg->profiles.a2dp  = 0;
      p_host_cfg->profiles.avrcp = 0;
      p_host_cfg->profiles.hfp   = 0;
      p_host_cfg->profiles.hsp   = 0;
      p_host_cfg->profiles.spp   = 0;
      hsc_CFG_GetHidInfo(&info);
  }
  isConfigInit = 1;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
               for linkkey backup in the flash start
------------------------------------------------------------------------*/
void hsc_StoreBackUpLinkKey(uint8_t* data)
{
    uint32_t buffer_len = sizeof(hs_pair_entry_t)*BONDED_DEVICE_MAX_COUNT;
    uint8_t *pu8Ptr = (uint8_t *)hs_malloc(buffer_len+1, __MT_GENERAL);
    if (pu8Ptr == NULL)
    {
        hs_printf("store back up linkkey error!\r\n");
        return;
    }
    pu8Ptr[0] = 0x01;
    memcpy(pu8Ptr+1, data, buffer_len);
    hs_cfg_halWrite(0xfC000, pu8Ptr, buffer_len+1);
    hs_free(pu8Ptr);
    //hs_printf("backup flash\r\n");
}

void hsc_HandleBackUpLinkKey(void)
{
    // 1. compare linkkey with the backup flash
    hs_cfg_res_t res = HS_CFG_OK;
    uint32_t buffer_len = sizeof(hs_pair_entry_t)*BONDED_DEVICE_MAX_COUNT;
    uint8_t *pu8Ptr = (uint8_t *)hs_malloc(buffer_len+1, __MT_GENERAL);
    uint8_t *pu8ClassicPtr = NULL;

    //hs_printf("hsc_HandleBackUpLinkKey\r\n");

    if (pu8Ptr == NULL)
    {
        hs_printf("malloc store back up linkkey error!\r\n");
        return;
    }

    // read back up data
    hs_cfg_halRead(0xfC000, pu8Ptr, buffer_len+1);

    // the backup flash has not data
    if (pu8Ptr[0] != 0x01 ) 
    {
        hs_free(pu8Ptr);
        return;
    }
    // 2. read classic linkkey
    pu8ClassicPtr = (uint8_t *)hs_malloc(buffer_len, __MT_GENERAL);
    if (pu8ClassicPtr == NULL)
    {
        hs_printf("malloc store linkkey error!\r\n");
        goto BACKUP_ERASE;
    }
    res = hs_cfg_getPartDataByIndex(HS_CFG_CLASSIC_PAIRINFO, 
                                   pu8ClassicPtr, 
                                   buffer_len, 
                                   1);
    if (res != HS_CFG_OK)
    {
        //hs_printf("read classic linkkey error!\r\n");
        App_CFG_UpdatePairInfo(pu8Ptr+1);
        hs_cfg_flush(FLUSH_TYPE_ALL);
        goto BACKUP_ERASE;
    }
    // compare memory
    if (memcmp(pu8Ptr+1, pu8ClassicPtr, buffer_len) == 0)
    {
        //hs_printf("linkkey == !\r\n");
        goto BACKUP_ERASE;
    }
    // 3. flush flash
    App_CFG_UpdatePairInfo(pu8Ptr+1);
    hs_cfg_flush(FLUSH_TYPE_ALL);
    
BACKUP_ERASE:
    // 4. erase backup flash
    hs_cfg_halErase(0xfC000, 4*1024);

    if (pu8Ptr !=  NULL)
        hs_free(pu8Ptr);
    if (pu8ClassicPtr != NULL)
        hs_free(pu8ClassicPtr);
}

void hsc_StoreBackUpHIDLinkKey(uint8_t* data)
{
    uint32_t buffer_len = sizeof(struct AppBtHidInfoStru);
    uint8_t *pu8Ptr = (uint8_t *)hs_malloc(buffer_len+1, __MT_GENERAL);
    if (pu8Ptr == NULL)
    {
        hs_printf("store back up linkkey error!\r\n");
        return;
    }
    pu8Ptr[0] = 0x01;
    memcpy(pu8Ptr+1, data, buffer_len);
    hs_cfg_halWrite(0xfD000, pu8Ptr, buffer_len+1);
    hs_free(pu8Ptr);
}

void hsc_HandleBackUpHIDLinkKey(void)
{
    // 1. compare linkkey with the backup flash
    hs_cfg_res_t res = HS_CFG_OK;
    uint32_t buffer_len = sizeof(struct AppBtHidInfoStru);
    uint8_t *pu8Ptr = (uint8_t *)hs_malloc(buffer_len+1, __MT_GENERAL);
    uint8_t *pu8ClassicPtr = NULL;

    if (pu8Ptr == NULL)
    {
        hs_printf("malloc store back up linkkey error!\r\n");
        return;
    }

    // read back up data
    hs_cfg_halRead(0xfD000, pu8Ptr, buffer_len+1);

    // the backup flash has not data
    if (pu8Ptr[0] != 0x01 ) 
    {
        hs_free(pu8Ptr);
        return;
    }
    // 2. read classic linkkey
    pu8ClassicPtr = (uint8_t *)hs_malloc(buffer_len, __MT_GENERAL);
    if (pu8ClassicPtr == NULL)
    {
        hs_printf("malloc store linkkey error!\r\n");
        goto BACKUP_ERASE;
    }
    res = hs_cfg_getDataByIndex(HS_CFG_CLASSIC_HID_INFO, 
                                pu8ClassicPtr, 
                                sizeof(struct AppBtHidInfoStru));
    if (res != HS_CFG_OK)
    {
        //hs_printf("read classic linkkey error!\r\n");
        App_CFG_UpdatePairInfo(pu8Ptr+1);
        hs_cfg_flush(FLUSH_TYPE_ALL);
        goto BACKUP_ERASE;
    }
    // compare memory
    if (memcmp(pu8Ptr+1, pu8ClassicPtr, buffer_len) == 0)
    {
        //hs_printf("linkkey == !\r\n");
        goto BACKUP_ERASE;
    }
    // 3. flush flash
    App_CFG_UpdatePairInfo(pu8Ptr+1);
    hs_cfg_flush(FLUSH_TYPE_ALL);
    
BACKUP_ERASE:
    // 4. erase backup flash
    hs_cfg_halErase(0xfD000, 4*1024);

    if (pu8Ptr !=  NULL)
        hs_free(pu8Ptr);
    if (pu8ClassicPtr != NULL)
        hs_free(pu8ClassicPtr);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
               for linkkey backup in the flash end
------------------------------------------------------------------------*/

uint8_t App_CFG_GetPairInfo(uint8_t index, void *info)
{
#if HS_USE_CONF
  hs_pair_entry_t *p_inst = (hs_pair_entry_t*)info;
  hs_cfg_res_t res = HS_CFG_OK;

  if (info == NULL)
    return 0;
  
  if (hsc_GetBtMode() == BT_HOST_VAR_MODE_HID)
  {
      if (index == 0)
      {
          res = hs_cfg_getPartDataByIndex(HS_CFG_CLASSIC_HID_INFO, 
                                        (uint8_t *)info, 
                                        sizeof(hs_pair_entry_t), 
                                        sizeof(struct AppBtHidInfoStru)-sizeof(hs_pair_entry_t));
      }
      else
      {
          res = HS_CFG_ERR_NO_DATA;
      }
  }
  else
  {
      res = hs_cfg_getPartDataByIndex(HS_CFG_CLASSIC_PAIRINFO, (uint8_t*)p_inst, sizeof(hs_pair_entry_t), 1+index*sizeof(hs_pair_entry_t));
  }
  if (res == HS_CFG_OK) {
    int i;
    uint16_t asum = 0;
    for (i = 0; i < BD_ADDR_LEN; i++)
      asum += p_inst->bd_addr[i];
    if ((asum != 0x00) && (asum != (0xFF*6)))
      return 1; //valid entry
  }
#else
  (void)index;
  (void)info;
#endif
  return 0;
}

void App_CFG_UpdatePairInfo(void *info)
{
#if HS_USE_CONF
  if (hsc_GetBtMode() == BT_HOST_VAR_MODE_HID)
  {
      hs_cfg_setDataByIndex(HS_CFG_CLASSIC_HID_INFO, (uint8_t *)info, 
                            sizeof(hs_pair_entry_t), 
                            sizeof(struct AppBtHidInfoStru)-sizeof(hs_pair_entry_t));
  }
  else
  {
#if 1
   hs_cfg_setDataByIndex(HS_CFG_CLASSIC_PAIRINFO,
                         (uint8_t*)info,
                         sizeof(hs_pair_entry_t)*BONDED_DEVICE_MAX_COUNT,
                         1);
#else
      hs_pair_entry_t *p_inst = (hs_pair_entry_t*)info;
      hs_cfg_res_t res;
      uint8_t index = 0;
      hs_pair_entry_t inst;

      for (index = 0; index < APP_CFG_PAIR_INFO_MAX; index++) {
          res = hs_cfg_getPartDataByIndex(HS_CFG_CLASSIC_PAIRINFO, (uint8_t *)&inst.bd_addr, BD_ADDR_LEN, 1+index*sizeof(hs_pair_entry_t));
          if (res == HS_CFG_OK) {
              if (memcmp(&inst.bd_addr, &p_inst->bd_addr, BD_ADDR_LEN)==0) {
                  res = hs_cfg_setDataByIndex(HS_CFG_CLASSIC_PAIRINFO, (uint8_t*)p_inst, sizeof(hs_pair_entry_t), 1+index*sizeof(hs_pair_entry_t));
                  if (res == HS_CFG_OK) {
                      index++;
                      if (index >= APP_CFG_PAIR_INFO_MAX) index = 0;

                      hs_cfg_setDataByIndex(HS_CFG_CLASSIC_PAIRINFO, &index, 1, 0);
                      //hs_cfg_flush(FLUSH_TYPE_ALL);
                      break;
                  }
              }
          }
      }
#endif
  }
#else
   (void)info;
#endif
}

void App_CFG_AddPairInfo(void *info)
{
#if HS_USE_CONF
  if (hsc_GetBtMode() == BT_HOST_VAR_MODE_HID)
  {
      hs_cfg_setDataByIndex(HS_CFG_CLASSIC_HID_INFO, (uint8_t *)info, 
                            sizeof(hs_pair_entry_t), 
                            sizeof(struct AppBtHidInfoStru)-sizeof(hs_pair_entry_t));
  }
  else
  {
      hs_pair_entry_t *inst = (hs_pair_entry_t*)info;
      hs_cfg_res_t res;
      uint8_t index;

      res = hs_cfg_getDataByIndex(HS_CFG_CLASSIC_PAIRINFO, &index, 1);
      if (res == HS_CFG_OK) {
          res = hs_cfg_setDataByIndex(HS_CFG_CLASSIC_PAIRINFO, (uint8_t*)(inst), sizeof(hs_pair_entry_t), 1+index*sizeof(hs_pair_entry_t));
          if (res == HS_CFG_OK) {
              index++;
              if (index >= APP_CFG_PAIR_INFO_MAX) index = 0;

              hs_cfg_setDataByIndex(HS_CFG_CLASSIC_PAIRINFO, &index, 1, 0);
              //hs_cfg_flush(FLUSH_TYPE_ALL);
          }
      }
  }
#else
  (void)info;
#endif
}

void App_CFG_ClearPairInfo(void)
{
#if HS_USE_CONF
    if (hsc_GetBtMode() == BT_HOST_VAR_MODE_HID)
    {
        hs_pair_entry_t info;
        memset(&info, 0, sizeof(hs_pair_entry_t));
        hs_cfg_setDataByIndex(HS_CFG_CLASSIC_HID_INFO, (uint8_t *)(&info), 
                sizeof(hs_pair_entry_t), 
                sizeof(struct AppBtHidInfoStru)-sizeof(hs_pair_entry_t));
    }
    else
    {
        uint16_t len = sizeof(hs_pair_entry_t)*APP_CFG_PAIR_INFO_MAX+1;
        uint8_t *buffer = chHeapAlloc(NULL, len);

        if (buffer == NULL) {
            return;
        }

        memset(buffer, 0, len);
        hs_cfg_setDataByIndex(HS_CFG_CLASSIC_PAIRINFO, buffer, len, 0);
        chHeapFree(buffer);
    }
    //hs_cfg_flush(FLUSH_TYPE_ALL);
#endif
}

void App_CFG_Uninit(void)
{
#if HS_USE_CONF
#endif
}

APP_CFG_ActionFunc App_CFG_GetAction(uint8_t index)
{
#if HS_USE_CONF
  hs_cfg_res_t res = HS_CFG_OK;
  struct AppBtActionStru action;
  uint32_t i = 0;
  uint16_t btState = App_GetBtState();
  uint16_t hfpState = APP_BT_STATE_NULL;
  uint16_t a2dpState = APP_BT_STATE_NULL;
  uint16_t hidState = APP_BT_STATE_NULL;

#ifdef CONFIG_HFP
  hfpState = hsc_HFP_GetState();
#endif
#ifdef CONFIG_A2DP
  a2dpState = hsc_A2DP_GetState();
#endif

#ifdef CONFIG_HID
  hidState = App_HID_GetState();
#endif

  if (isConfigInit == 0)
    return NULL;
   
  //hs_printf("App_CFG_GetAction: index=%x, bt=%d, hfp=%d, a2dp=%d, hid=%d\r\n", index, btState, hfpState, a2dpState, hidState);

  while(res == HS_CFG_OK)
  {
    msleep(4);
    res = hs_cfg_getPartDataByIndex(HS_CFG_CLASSIC_ACTION, (uint8_t *)&action, sizeof(struct AppBtActionStru), i*sizeof(struct AppBtActionStru));
    //hs_printf("App_CFG_GetAction: index=%x, profile=%d, status=%d, action=%d, res = %d\r\n",
    //        action.eventIndex, action.eventProfiles, action.profileStatus, action.actionIndex, res);

    if ((res != HS_CFG_OK) || (action.eventIndex == 0))
      break;
    if (action.eventIndex == index && action.actionIndex <= APP_CFG_EVENT_MAX && action.eventProfiles != 0)
    {
      if (hfpState >BT_HFP_CALL_STATUS_STANDBY && action.eventProfiles == BT_CLASSIC_PROFILE_A2DP)
      {
        i++;
        continue;
      }
      if (((action.eventProfiles == BT_CLASSIC_PROFILE_GAP) && (action.profileStatus & btState)) || 
              ((action.eventProfiles == BT_CLASSIC_PROFILE_HFP) && (action.profileStatus & hfpState)) || 
              ((action.eventProfiles == BT_CLASSIC_PROFILE_A2DP) && (action.profileStatus & a2dpState)) ||
              ((action.eventProfiles == BT_CLASSIC_PROFILE_HID) && (action.profileStatus & hidState)) )
      {
        //btActionHandle[action.actionIndex].eventAction();
        uint32_t ii;
        for (ii=0; ii<(sizeof(btActionHandle)/sizeof(struct AppBtActionHandleStru)); ii++) {
          if (btActionHandle[ii].actionIndex == action.actionIndex)
            return btActionHandle[ii].eventAction;
        }
        return NULL;
      }
    }
    i++;
  }
#else
  (void)index;
#endif
   return NULL;
}

void App_CFG_SetHfpVol(uint8_t spkval, uint8_t micval)
{
#if 0
  p_host_cfg->attrs.hfpSpkVol = spkval;
  p_host_cfg->attrs.hfpMicVol = micval;

#if HS_USE_CONF
  hs_cfg_setDataByIndex(HS_CFG_CLASSIC_ATTR, (uint8_t *)&p_host_cfg->attrs, sizeof(&p_host_cfg->attrs), 0);
#endif
#else
  (void)spkval;
  (void)micval;
#endif
}

void hsc_CFG_SaveA2dpVol(uint8_t val)
{
#if 0
  p_host_cfg->attrs.a2dpSpkVol = val;
#if HS_USE_CONF
  hs_cfg_setDataByIndex(HS_CFG_CLASSIC_ATTR, (uint8_t *)&p_host_cfg->attrs, sizeof(&p_host_cfg->attrs), 0);
#endif
#else
  (void)val;
#endif
}

void hsc_CFG_GetHidInfo(struct AppBtHidInfoStru *info)
{
  if (hs_cfg_getDataByIndex(HS_CFG_CLASSIC_HID_INFO, (uint8_t *)info, sizeof(struct AppBtHidInfoStru)) != HS_CFG_OK) {
    memcpy( info->name, devName, BT_NAME_LEN_MAX);
    info->addr[0] = 0x01;
    info->addr[1] = 0xA2;
    info->addr[2] = 0x01;
    info->addr[3] = 0x66;
    info->addr[4] = 0xbf;
    info->addr[5] = 0x01;
  }
}
#endif

#if HS_USE_BT == HS_BT_DATA_TRANS
#include "btstack_uapi.h"

__PACKED struct APP_LE_Set_Advertising_ParametersStru {
	uint16_t advertising_interval_min;       /* [302],Time */
	uint16_t advertising_interval_max;       /* [303],Time */
	uint8_t advertising_type;                /* [305],Enum */
	uint8_t own_address_type;                /* [286],Enum */
	uint8_t direct_address_type;             /* [287],Enum */
	uint8_t direct_address[6];               /* [289],Val */
	uint8_t advertising_channel_map;         /* [306],Mask */
	uint8_t advertising_filter_policy;       /* [307],Enum */
}__PACKED_GCC;

__PACKED struct APP_LE_SETTING {
	uint8_t enable;
	struct APP_LE_Set_Advertising_ParametersStru adv_param;
	uint8_t adv_len;
	uint8_t adv_data[31];
	uint8_t adv_padding[2];
	uint8_t res_len;
	uint8_t res_data[31];
}__PACKED_GCC;

void App_CFG_SetBtHostVisibility(uint8_t visibility)
{
    p_host_cfg->attrs.hfpStereoEnable = visibility;
    hs_cfg_setDataByIndex(HS_CFG_CLASSIC_ATTR, (uint8_t *)&p_host_cfg->attrs, sizeof(p_host_cfg->attrs), 0);
}

void App_CFG_SetBtHostName(uint8_t len, uint8_t *name)
{
    memset( p_host_cfg->device.name, 0, BT_NAME_LEN_MAX);
    memcpy( p_host_cfg->device.name, name, len);
    hs_cfg_setDataByIndex(HS_CFG_CLASSIC_DEV, (uint8_t *)&p_host_cfg->device, sizeof(p_host_cfg->device), 0);
}

void App_CFG_SetBtHostBle(bt_host_sys_config_t *config)
{
    struct APP_LE_SETTING ble_setting;
    hs_cfg_getDataByIndex(HS_CFG_BLE_BRIDGE, (uint8_t *)&ble_setting, sizeof(struct APP_LE_SETTING));
    ble_setting.adv_param.advertising_interval_min = config->advi;

    if (config->adv_res_en & 0x04)
    {
        ble_setting.adv_param.own_address_type = 1;
    }
    if (config->adv_res_en & 0x01)
    {
        ble_setting.adv_len = LE_ADVERTISING_DATA_SIZE;
    }
    if (ble_setting.adv_len > 0)
    {
        memcpy(ble_setting.adv_data, config->adv_data, ble_setting.adv_len);
    }
    if (config->adv_res_en & 0x02)
    {
        ble_setting.res_len = LE_ADVERTISING_DATA_SIZE;
    }
    if (ble_setting.res_len > 0)
    {
        memcpy(ble_setting.res_data, config->res_data, ble_setting.res_len);
    }

    hs_cfg_setDataByIndex(HS_CFG_BLE_BRIDGE, (uint8_t *)&ble_setting, sizeof(struct APP_LE_SETTING), 0);
}

void App_CFG_GetBtHost(void* arg)
{
    uint8_t i,j;
    uint16_t sum=0;
    hs_pair_entry_t pair_info[4];
    bt_host_sys_config_t *p_bthost_cfg = (bt_host_sys_config_t*) arg;
    struct APP_LE_SETTING ble_setting;
    memset(&ble_setting, 0, sizeof(struct APP_LE_SETTING));
    memset(p_bthost_cfg, 0 , sizeof(bt_host_sys_config_t));
    
    if (hs_cfg_getDataByIndex(HS_CFG_CLASSIC_DEV, (uint8_t *)&p_host_cfg->device.name, sizeof(p_host_cfg->device)-BD_ADDR_LEN) != HS_CFG_OK) {
        memcpy( p_host_cfg->device.name, devName, BT_NAME_LEN_MAX);
        memset( p_host_cfg->device.pincode, '0', BT_PIN_CODE_LEN);
        memcpy( p_host_cfg->device.cod, classOfDevice, BT_CLASS_OF_DEVICE_LEN);
    }
    if (hs_cfg_getDataByIndex(HS_CFG_CLASSIC_ATTR, (uint8_t *)&p_host_cfg->attrs, sizeof(p_host_cfg->attrs)) != HS_CFG_OK) {
        p_host_cfg->attrs.gapIoCap  = 0x01;
        // bit 0: 0, standard scan; 1, interlaced scan
        // bit 1: 0, ssp disable; 1, ssp nable;
        // bit 2: 0, user comfirm; 1, auto accept; 
        // bit 3: 0, ble adv disenable; 1, enable; 
        // bit 4: 0, classic discoverable disnable; 1 enable;
        // bit 5: 0, classic connectable disnable; 1 enable;
        p_host_cfg->attrs.hfpStereoEnable = 0x3F;
    }

    hs_cfg_getPartDataByIndex(HS_CFG_CLASSIC_PAIRINFO, (uint8_t*)pair_info, sizeof(hs_pair_entry_t)*4, 1);
    if (hs_cfg_getDataByIndex(HS_CFG_BLE_BRIDGE, (uint8_t *)&ble_setting, sizeof(struct APP_LE_SETTING)) != HS_CFG_OK) {
        ble_setting.adv_param.advertising_interval_min = 0x0640;
    }

    memcpy(p_bthost_cfg->name, p_host_cfg->device.name, BT_NAME_LEN_MAX);
    memcpy(p_bthost_cfg->pincode, p_host_cfg->device.pincode, BT_PIN_CODE_LEN);
    memcpy(p_bthost_cfg->cod, p_host_cfg->device.cod, BT_CLASS_OF_DEVICE_LEN);
    memcpy(p_bthost_cfg->addr, p_sys_config->bd_addr, BD_ADDR_LEN);
    p_bthost_cfg->pair_list_max_entries = 4;
    p_bthost_cfg->valid = 1;
    p_bthost_cfg->io = p_host_cfg->attrs.gapIoCap;
    p_bthost_cfg->sys_padding[0] = p_host_cfg->attrs.hfpStereoEnable;
    p_bthost_cfg->advi = ble_setting.adv_param.advertising_interval_min;

    if (ble_setting.adv_param.own_address_type == 1)
    {
        p_bthost_cfg->adv_res_en |= 0x04;
    }
    if (ble_setting.adv_len > 0)
    {
        p_bthost_cfg->adv_res_en |= 0x01;
        memcpy(p_bthost_cfg->adv_data, ble_setting.adv_data, ble_setting.adv_len);
    }
    if (ble_setting.res_len > 0)
    {
        p_bthost_cfg->adv_res_en |= 0x02;
        memcpy(p_bthost_cfg->res_data, ble_setting.res_data, ble_setting.res_len);
    }
    for(i =0; i< BT_REMOTE_DB_MAX; i++)
    {
        memcpy(&p_bthost_cfg->pair_list[i], &pair_info[i], sizeof(hs_pair_entry_t));
        sum = 0;
        for(j= 0; j< LINK_KEY_LEN; j++)
        {
            sum += p_bthost_cfg->pair_list[i].link_key[j];
            if (sum>1) break;
        }
        if (sum > 1)
        {
            p_bthost_cfg->pair_list[i].valid = 1;
        }
    }
}
#endif

void App_CFG_GetBtHcRf(uint8_t mode)
{
  HCI_Generic_Config(&p_sys_config, &p_sys_rf_cfg);

#if HS_USE_CONF
  hs_cfg_getDataByIndex(HS_CFG_BT_HC, (uint8_t *)p_sys_config, sizeof(bt_hc_sys_config_t));
  hs_cfg_getDataByIndex(HS_CFG_BT_RF, (uint8_t *)p_sys_rf_cfg, sizeof(bt_hc_phy_config_t));
#if HS_USE_BT == HS_BT_AUDIO
  if (mode == BT_HOST_VAR_MODE_HID)
  {
      //struct AppBtHidInfoStru info;
      //hsc_CFG_GetHidInfo(&info);
      //memcpy(p_sys_config->bd_addr, info.addr, BD_ADDR_LEN);
      p_sys_config->bd_addr[5] = 0xBF;
  }
#else
  (void)mode;
#endif
#endif
  hs_printf("max_active_devices=%d out_acl=%dx%d in_acl=%dx%d, erroneous_data_reporting=%d plc=%d\r\n", p_sys_config->max_active_devices, p_sys_config->hc_buffer_size.aclDataPacketLength, p_sys_config->hc_buffer_size.numAclDataPackets, p_sys_config->hc_buffer_size_in.aclDataPacketLength, p_sys_config->hc_buffer_size_in.numAclDataPackets, p_sys_config->erroneous_data_reporting, p_sys_config->plc);
  if (p_sys_config->max_active_devices_in_piconet > 2) {
    p_sys_config->max_active_devices            = 5;
    p_sys_config->max_active_devices_in_piconet = 2;
  }
  if (p_sys_config->hc_buffer_size.aclDataPacketLength > 688) {
    p_sys_config->hc_buffer_size.aclDataPacketLength    = 688; //2-DH5
    p_sys_config->hc_buffer_size.numAclDataPackets      = 4;
    p_sys_config->hc_buffer_size_in.aclDataPacketLength = 688;
    p_sys_config->hc_buffer_size_in.numAclDataPackets   = 8;
  }
  p_sys_config->erroneous_data_reporting = 0;
  p_sys_config->plc = 1;
}
