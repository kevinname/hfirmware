#include "app_main.h"

//#define FIXED_PIN		"0000"
static volatile UINT8 app_fsm_bt_visible_mode = 0;
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_GapRemoteNameUpdated(struct HCI_Remote_Name_Request_CompleteEvStru *in)
{
	struct HCI_Remote_Name_Request_CompleteEvStru *param = NEW(sizeof(struct HCI_Remote_Name_Request_CompleteEvStru));
	memcpy(param, in, sizeof(struct HCI_Remote_Name_Request_CompleteEvStru));
	App_SCH_FsmEvent(App_Gap_FsmRemoteNameUpdated, param);
	//MMI_RemoteNameUpdatedNotification(in->bd, in->remote_name);
}

void App_GAP_RegisterTL(HANDLE tl_handle, TransportLayerStru *func) 
{
	GAP_RegisterTransportLayerA(tl_handle, func, App_GapInitRegisterTLCfm);
}

void App_GAP_UnRegisterTL(HANDLE tl_handle)
{
    GAP_UnregisterTransportLayer(tl_handle);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_GAP_UnpairDevice(UINT8 *bd) 
{
	UINT8 *in = NEW(BD_ADDR_LEN);
	memcpy(in, bd, BD_ADDR_LEN);
	App_UI_FsmEvent(App_Gap_FsmUnpairDevice, in);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_GAP_ListCurrentConnections(void)
{
	App_UI_FsmEvent(App_Gap_FsmListCurrentConnections, NULL);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_GapInitRegisterTLCfm(HANDLE tl_handle, UINT16 result, struct GAP_RegisterTLCfmStru *cfm_par)
{
    (void)tl_handle;
    (void)cfm_par;

    if (result == HCI_STATUS_OK) {
        if ((g_bt_host_config.features.ptsTestEnable>>2)&0x01)
        {
            HCI_DUT_SetEnable(1);
        }

		/* Set local IO Capability */
		GAP_SetIOCapability(g_bt_host_config.attrs.gapIoCap);
                //GAP_SetIOCapability(HCI_IO_CAPABILITY_NOINPUTNOOUTPUT);
		/* Register common GAP callback */
		GAP_RegisterIndCbk(NULL, GAP_IND_ALL, App_GapGeneralInd);
		debug("%4d - reset HC\r\n", GetCurrTime());
		/* Start Controller initialization */
		GAP_ResetHardwareA(NULL, NULL, App_GapInitResetHardwareCfm);
		app_fsm_bt_visible_mode = 0;
	} else {
		/* TODO: Beep and shut down */
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_GapInitResetHardwareCfm(void *context, UINT16 result, void *cfm_par)
{
   (void)context;
   (void)cfm_par;
   if (result == HCI_STATUS_OK) {
      App_SetBtState(APP_BT_STATE_READY);
      /* Change device mode: general discoverable, page and inquiry scan enable, allow pairing */
      if ( g_bt_host_config.features.linkLostIntoPairEnable & 0x02) {
         if (g_bt_host_config.features.linkLostIntoPairEnable & 0x04) {
            GAP_SetVisualModeA(NULL, NULL, GAP_VM_LIMIT_DISCOV | GAP_VM_PAGE | GAP_VM_PAIR, App_GapInitSetVisualModeCfm);
         }
         else {
            GAP_SetVisualModeA(NULL, NULL, GAP_VM_DEFAULT, App_GapInitSetVisualModeCfm);
         }
         #if HS_USE_CONF
         if (hsc_GetBtMode() == BT_HOST_VAR_MODE_HID)
         {
            hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_HID_PARIABLE);
         }
         else
         {
            hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_PAIRABLE);
         }
         #endif
      }
      else {
         GAP_SetVisualModeA(NULL, NULL, 0, App_GapInitSetVisualModeCfm);
      }
   } else {
      /* TODO: Beep and shut down */
   }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_GapInitSetVisualModeCfm(void *context, UINT16 result, void *cfm_par)
{
   //UINT8 dev_class[3] = {0X08, 0X04, 0X20}; // Hands-free
   (void)context;
   (void)cfm_par;

   UINT8* dev_class;
   UINT8  cod[BT_CLASS_OF_DEVICE_LEN] = {0x40, 0x25, 0x00};
   if (hsc_GetBtMode() == BT_HOST_VAR_MODE_HID)
   {
       dev_class = cod;
   }
   else
   {
       dev_class = g_bt_host_config.device.cod;
   }
   
   if (result == HCI_STATUS_OK) {
      if ( g_bt_host_config.features.linkLostIntoPairEnable & 0x02) {
         App_SetBtState(APP_BT_STATE_PAIRMODE);
      }
      /* Change local device class */
      GAP_SetLocalDeviceClassA(NULL, NULL, dev_class, App_GapInitSetLocalDeviceClassCfm);
   } else {
      /* TODO: Beep and shut down */
   }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_GapInitSetLocalDeviceClassCfm(void *context, UINT16 result, void *cfm_par)
{
	//UCHAR dev_name[] = {0xe6, 0xb1, 0x89, 0xe5, 0xa4, 0xa9, 0xe4, 0xb8, 0x8b, '6', '6', 'x', 'x', '\0'}; //汉天下66xx
        UINT8* dev_name;
        struct AppBtHidInfoStru info;
        if (hsc_GetBtMode() == BT_HOST_VAR_MODE_HID)
        {
            hsc_CFG_GetHidInfo(&info);
            dev_name = info.name;
        }
        else
        {
            dev_name = App_CFG_GetDeviceName();
        }
	/* name_len must include '\0' */
        UINT8  name_len = (strlen((char*)dev_name)+1 > BT_NAME_LEN_MAX)?BT_NAME_LEN_MAX:strlen((char*)dev_name)+1;

        (void)context;
        (void)cfm_par;

	debug("bt name: 汉天下%s\r\n", &dev_name[9]);
	if (result == HCI_STATUS_OK) {
		/* Change local device name */
		GAP_SetLocalNameA(NULL, NULL, dev_name, name_len, App_GapInitSetLocalNameCfm);
	} else {
		/* TODO: Beep and shut down */
	}
}

void App_MPS_Start(void)
{
   struct SDAP_MPSInfoStru reg_info;
   reg_info.mask = 0;
   reg_info.ver = 0x0100;
   reg_info.mpsd_scenarios.higher_4bytes = 0;
   reg_info.mpsd_scenarios.lower_4bytes =  0x2aaa;
   reg_info.mpmd_scenarios.higher_4bytes = 0;
   reg_info.mpmd_scenarios.lower_4bytes =  0;
   reg_info.depend = 0x0E;
   SDAP_RegisterMPSService(&reg_info);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_GapInitSetLocalNameCfm(void *context, UINT16 result, void *cfm_par)
{
        (void)context;
        (void)cfm_par;

	if (result == HCI_STATUS_OK) {
        debug("%4d - start profiles\r\n", GetCurrTime());

		/* Complete Controller Initialization. */
		/* Start Profiles specific APP initialization */
        if (hsc_GetBtMode() == BT_HOST_VAR_MODE_HID)
        {
             #ifdef CONFIG_HID
             App_HID_Start(g_bt_host_config.profiles.hid);
             App_FsmAutoConnect();
             #endif
             return;
        }
#ifdef CONFIG_A2DP
		hsc_A2dpStartMulti(g_bt_host_config.profiles.a2dp);
#endif
#ifdef CONFIG_AVRCP
		hsc_AvrcpStartByRole(g_bt_host_config.profiles.avrcp);
#endif
#ifdef CONFIG_HFP
		hsc_HfpStartMulti(g_bt_host_config.profiles.hfp);
		hsc_HspStartMulti(g_bt_host_config.profiles.hsp);
#endif
#ifdef CONFIG_SPP
                App_SPP_Start(g_bt_host_config.profiles.spp);
#endif             

#ifdef CONFIG_HID
               //App_HID_Start(g_bt_host_config.profiles.hid);
#endif

//#ifdef CONFIG_MPS
	       App_MPS_Start();
//#endif
#ifdef CONFIG_BLE
	       App_BLE_Start();
	       App_BTLED_Init();
#endif
           App_FsmAutoConnect();
	} else {
		/* TODO: Beep and shut down */
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_GapDevConnected(HANDLE tl_handle, struct GAP_ConnectionEventStru *in)
{
  if (in != NULL) {
    if (in->connected) {
      App_AddAclList(tl_handle, in);

#if defined(CONFIG_PATCH_HUNTERSUN)
      {
      // send supervision timeout change
      struct HCI_Write_Link_Supervision_TimeoutStru super_timeout;
      super_timeout.connection_handle = in->acl_hdl;
      super_timeout.link_supervision_timeout = 0x3200;
      GAP_ExecuteCommandA(tl_handle, 
                         NULL,
                         HCI_OPS_WRITE_LINK_SUPERVISION_TIMEOUT, 
                         &super_timeout, 
                         sizeof(struct HCI_Write_Link_Supervision_TimeoutStru), 
                         NULL);
      }
#if HS_USE_CONF
    // send event for mp3 destroy, and sleep 20 ms
    hs_cfg_sysSendMessage(HS_CFG_MODULE_PERIP, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_GAP_CNNNECTED);     //hotplug-like event
    msleep(15);

    //hs_player_destroy(0, 0);
#endif
#endif
    }
    else {
      App_DeleteAclList(in);
#ifdef CONFIG_BLE
      App_BLE_AdvStart();
#endif
    }
  }
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_GapPinCodeReqInd(struct HCI_PIN_Code_RequestEvStru *in)
{
	struct HCI_PIN_Code_Request_ReplyStru reply;

	//reply.pin_code_len = strlen(FIXED_PIN);
	//strcpy(reply.pin_code, FIXED_PIN);
	reply.pin_code_len = BT_PIN_CODE_LEN;
	memcpy(reply.pin_code, g_bt_host_config.device.pincode, BT_PIN_CODE_LEN);
	memcpy(reply.bd, in->bd, BD_ADDR_LEN);
	GAP_ExecuteCommandA(NULL, NULL, HCI_OPS_PIN_CODE_REQUEST_REPLY, &reply,
		sizeof(struct HCI_PIN_Code_Request_ReplyStru), NULL);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_GapLinkKeyReqInd(struct HCI_Security_Link_Key_Ask4Stru *in)
{
	UINT8 *bd = NEW(BD_ADDR_LEN);
	memcpy(bd, in->remote_bd, BD_ADDR_LEN);
	App_SCH_FsmEvent(App_Gap_FsmLinkKeyReqInd, bd);
}

void App_GapSimplePairing(struct HCI_Simple_Pairing_CompleteEvStru *in)
{
   if (in->status == 0) {
      #if HS_USE_CONF
     //hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_STATUS, HS_CFG_STATUS_PAIRED);
      #endif
   }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_GapLinkKeyCreated(struct HCI_Security_Link_Key_SaveStru *in)
{
	struct HCI_Security_Link_Key_SaveStru *param = NEW(sizeof(struct HCI_Security_Link_Key_SaveStru));
	memcpy(param, in, sizeof(struct HCI_Security_Link_Key_SaveStru));
	App_SCH_FsmEvent(App_Gap_FsmLinkKeyCreated, param);
#if defined(__nds32__)
	hs_printf("linkkey:%02x%02x%02x%02x%02x%02x,%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x,%d\r\n",
#else
	debug("linkkey:%02x%02x%02x%02x%02x%02x,%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x,%d\r\n",
#endif
          in->remote_bd[5], in->remote_bd[4], in->remote_bd[3], in->remote_bd[2], in->remote_bd[1], in->remote_bd[0],
          in->link_key[15], in->link_key[14], in->link_key[13], in->link_key[12],
          in->link_key[11], in->link_key[10], in->link_key[9], in->link_key[8],
          in->link_key[7], in->link_key[6], in->link_key[5], in->link_key[4],
          in->link_key[3], in->link_key[2], in->link_key[1], in->link_key[0], in->key_type);
#ifdef CONFIG_MMI
	MMI_LinkKeyNotification(in->remote_bd, in->link_key, in->key_type);
#endif
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_GapAuthenFailure(struct GAP_AuthenticationFailureStru *in)
{
	/* In general, the APP shall notify the user the authentication failure
	   and let the user determine whether to remove the bonded relation or not.
	   For simplification, this sample remove the bonded relation directly. */
	struct GAP_AuthenticationFailureStru *param = NEW(sizeof(struct GAP_AuthenticationFailureStru));
	memcpy(param, in, sizeof(struct GAP_AuthenticationFailureStru));
	App_SCH_FsmEvent(App_Gap_FsmAuthenFailure, param);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
tGAP_IndCbk_Result App_GapGeneralInd(HANDLE tl_handle, void *context, tGAP_Ind_Type ind_type, void *parameter, UINT32 size)
{
	tGAP_IndCbk_Result result = GAP_IND_RESULT_NOT_CARE;
	
    (void)context;
    (void)size;
	switch(ind_type) {
	case GAP_IND_INQUIRY_COMPLETE: /* parameter: struct HCI_Inquiry_CompleteEvStru * */
		break;
	case GAP_IND_INQUIRY_RESULT: /* parameter: struct GAP_RemoteDeviceInfoStru * */
		break;
	case GAP_IND_CONNECTION_EVENT: /* parameter: struct GAP_ConnectionEventStru * */
		App_GapDevConnected(tl_handle, parameter);
		break;
	case GAP_IND_PIN_CODE_REQ: /* parameter: struct HCI_PIN_Code_RequestEvStru * */
		App_GapPinCodeReqInd(parameter);
		result = GAP_IND_RESULT_PENDING;
		break;
	case GAP_IND_LINK_KEY_REQUEST: /* parameter: struct HCI_Security_Link_Key_Ask4Stru * */
		App_GapLinkKeyReqInd(parameter);
		result = GAP_IND_RESULT_PENDING;
		break;
	case GAP_IND_LINK_KEY_NOTIFICATION: /* parameter: struct HCI_Security_Link_Key_SaveStru * */
		App_GapLinkKeyCreated(parameter);
		break;
	case GAP_IND_AUTHENTICATION_FAILURE: /* parameter: struct GAP_AuthenticationFailureStru * */
		App_GapAuthenFailure(parameter);
		break;
	case GAP_IND_REMOTE_DEVICE_NAME_UPDATED: /* parameter: struct HCI_Remote_Name_Request_CompleteEvStru * */
		App_GapRemoteNameUpdated(parameter);
		break;
	case GAP_IND_USER_CONFIRMATION_REQUEST: /* parameter: struct HCI_User_Confirmation_RequestEvStru * */
		result = GAP_IND_RESULT_ACCEPT;
		break;
	case GAP_IND_USER_PASSKEY_REQUEST: /* parameter: struct HCI_User_Passkey_RequestEvStru * */
		break;
	case GAP_IND_SIMPLE_PAIRING_COMPLETE: /* parameter: struct HCI_Simple_Pairing_CompleteEvStru * */
		App_GapSimplePairing(parameter);
		break;
	case GAP_IND_USER_PASSKEY_NOTIFICATION: /* parameter: struct HCI_User_Passkey_NotificationEvStru * */
		break;
	case GAP_IND_AUTHORIZATION_REQUEST: /* parameter: struct HCI_Security_AuthorReqStru * */
		break;
	case GAP_IND_AUTHORIZATION_ABORT: /* parameter: struct HCI_Security_AuthorReqStru * */
		break;
	case GAP_IND_CONNECTION_REQUEST: /* parameter: struct HCI_Connection_RequestEvStru * */
		break;
	default:
		break;
	}
	
	return result;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
#if defined(CONFIG_PATCH_HUNTERSUN)
extern UINT8 g_test_freq_drift_enable;
#endif
void App_Gap_FsmLinkKeyReqInd(struct FsmInst *fi, UINT8 event, UINT8 *in)
{
	struct AppDevInst *device;
	struct HCI_Security_Link_Key_Request_ReplyStru cmd;

        (void)fi;
        (void)event;

#if defined(CONFIG_PATCH_HUNTERSUN)
if(g_test_freq_drift_enable)//test_mode
{
  memcpy(cmd.bd, in, BD_ADDR_LEN);
  const uint8_t linkkey[]={0x06,0x77,0x5f,0x87,0x91,0x8d,0xd4,0x23,0x00,0x5d,0xf1,0xd8,0xcf,0x0c,0x14,0x2b};
  cmd.accept = 1;
  memcpy(cmd.link_key, linkkey, LINKKEYLENGTH);
  cmd.key_type = 4;
  //clear test_mode flag
  //g_test_freq_drift_enable = 0;
}
else
#endif
{
	device = App_FindBondedDevice(in);
	memcpy(cmd.bd, in, BD_ADDR_LEN);
	if (device != NULL) {
		cmd.accept = 1;
		memcpy(cmd.link_key, device->link_key, LINKKEYLENGTH);
		cmd.key_type = device->key_type;
	} else {
		cmd.accept = 0;
	}
}
	GAP_LinkKeyReply(NULL, &cmd);
	FREE(in);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_Gap_FsmLinkKeyCreated(struct FsmInst *fi, UINT8 event, struct HCI_Security_Link_Key_SaveStru *in)
{
	struct AppDevInst *device = App_FindBondedDevice(in->remote_bd);

        (void)fi;
        (void)event;

	if (device != NULL) {
		memcpy(device->link_key, in->link_key, LINKKEYLENGTH);
		device->key_type = in->key_type;
		//App_CFG_UpdatePairInfo(device);
	} else {
		App_AddBondedDevice(in->remote_bd, in->link_key, in->key_type);
	}
	App_StoreBondedDevice();
	FREE(in);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_Gap_FsmAuthenFailure(struct FsmInst *fi, UINT8 event, struct GAP_AuthenticationFailureStru *in)
{
	struct AppDevInst *device = App_FindBondedDevice(in->addr.bd);

        (void)fi;
        (void)event;

	if (device != NULL) {
        if ((in->error == HCI_STATUS_PIN_OR_KEY_MISSING) || (in->error == HCI_STATUS_AUTHENTICATION_FAILURE))
        {
            //App_FsmReconnect();
            App_DeleteBondedDevice(device);
			App_StoreBondedDevice();
        }
	}
	FREE(in);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_Gap_FsmUnpairDevice(struct FsmInst *fi, UINT8 event, UINT8 *bd)
{
	struct AppDevInst *device = App_FindBondedDevice(bd);

        (void)fi;
        (void)event;        

	if (device != NULL) {
		App_DeleteBondedDevice(device);
		App_StoreBondedDevice();
	}
	FREE(bd);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_Gap_FsmListCurrentConnections(struct FsmInst *fi, UINT8 event, void *arg)
{
#ifdef CONFIG_MMI
	struct AppConnInst *connection = NULL;

        (void)fi;
        (void)event;
        (void)arg;

	while ((connection = App_FindNextConnectionInst(connection)) != NULL) {
		MMI_ConnectionReport(connection);
	}
	MMI_ConnectionReport(NULL);
#else
    (void)fi;
    (void)event;
    (void)arg;
#endif
}

void App_Gap_FsmRemoteNameUpdated(struct FsmInst *fi, UINT8 event, struct HCI_Remote_Name_Request_CompleteEvStru *in)
{
#if 1
  (void)fi;
  (void)event;
#else
   struct AppDevInst *device = App_FindBondedDevice(in->bd);

   (void)fi;
   (void)event;

   if (device != NULL) {
      memcpy(device->name, in->remote_name, BT_REMOTE_NAME_LEN_MAX);
   }
   FREE(in);
   //App_UpdateBondedDevice();
   //App_CFG_AddPairInfo(device);
#endif
   FREE(in);
}

void App_GAP_SetVisualModes(UINT8 mode)
{
   //hs_printf("gap:mode=%d\r\n", mode);
   
   if (app_fsm_bt_visible_mode == mode)
   {
      return;
   }
   app_fsm_bt_visible_mode = mode;
   switch (mode)
   {
      case 0:
	       GAP_SetVisualModeA(NULL, NULL, 0, NULL);
           break;
      case 1:
	       GAP_SetVisualModeA(NULL, NULL, GAP_VM_DISCOV | GAP_VM_PAIR, NULL);
           break;
      case 2:
	       GAP_SetVisualModeA(NULL, NULL, GAP_VM_PAGE, NULL);
           break;
      case 3:
           #if HS_USE_CONF
           if (hsc_GetBtMode() == BT_HOST_VAR_MODE_HID)
           {
              hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_HID_PARIABLE);
           }
           else
           {
              hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_PAIRABLE);
           }
           #endif
           //hs_printf("gap:pair\r\n");
           GAP_SetVisualModeA(NULL, NULL, GAP_VM_DEFAULT, NULL);
           break;
      default:
           break;
   }
#if HS_USE_CONF
   hs_cfg_systemReqArg(HS_CFG_EVENT_BT_DISCOVERY, (void*)(INT32)mode);
#endif
}

void App_GAP_ReConnect(UINT8 *bd)
{
   //uint16_t btState = App_GetBtState();
   //if (btState >= APP_BT_STATE_CONNECTED)
   //	   return;

   debug("App_GAP_ReConnect\r\n");
   if (hsc_GetBtMode() == BT_HOST_VAR_MODE_HID)
   {
       
#ifdef CONFIG_HID
       App_HID_Connect(bd);
#endif
       return;
   }
   #ifdef CONFIG_HFP
      App_HFP_ConnectAG(bd);
   #endif  
   
   #ifdef CONFIG_A2DP
      App_A2DP_Connect(bd);
   #endif
   #ifdef CONFIG_AVRCP
      App_AVRCP_Connect(bd);
   #endif

   #ifdef CONFIG_HID
      //App_HID_Connect(bd);
   #endif
   
   #ifdef CONFIG_BLE
      App_BLE_Connect(bd);
   #endif
}
