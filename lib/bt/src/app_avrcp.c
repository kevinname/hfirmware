/*---------------------------------------------------------------------------
Description:
	AVRCP Sample.
---------------------------------------------------------------------------*/
#include "app_main.h"

#ifdef CONFIG_AVRCP

/* AVRCP Connection related information */
struct App_AvrcpConnectionInst {
	UINT16 brw_chnl_mtu;        /* Outgoing MTU for Browsing Channel */
#if defined(CONFIG_PATCH_HUNTERSUN)
	UINT16 conn_hdl;            /* multi-phone support */
	UINT8  vol;
#endif
};

#if defined(CONFIG_PATCH_HUNTERSUN)

#define APP_AVRCP_VOL_MAX     127
#define APP_AVRCP_VOL_STEP    8 //127/15

static UINT8 s_app_avrcp_vol = 0x32; //00-0x7F: 0-100%, -50--0db
static UINT16 s_app_avrcp_conn_handle = 0;

void APP_AvrcpIndCbk(UINT8 *bd, UINT32 event, UINT8 *param, UINT16 len);
static void hsc_AvrcpVolNotifRsp(UINT8 rsp_code, UINT8 event_id);
static void hsc_AvrcpTGRegisterNotifRsp(UINT8 *bd, UINT8 rsp_code, UINT8 event_id, UINT32 interval);
static void hsc_AvrcpRegisterNotifReq(UINT8 *bd, UINT8 event_id);

static int hsc_AudioLevel2DB(char level)
{
#if HAL_USE_AUDIO
  int min = audioPlayGetVolumeMin(AUDIO_PLY_SRC_AVRCP);
  int max = audioPlayGetVolumeMax(AUDIO_PLY_SRC_AVRCP);
  //int curdb = hs_audio_volGet(AVOL_DEV_NOR);
  int vol = (min+ (max - min) * level / APP_AVRCP_VOL_MAX);
  //hs_printf("a2dp:min=%d,max=%d, cur=%d, level=%d--->%d \r\n", 
  //          min, max, curdb ,level, vol);
  return vol;
#else
  return 0;
#endif
}

static int hsc_AudioDB2level(int db)
{
   int min = audioPlayGetVolumeMin(AUDIO_PLY_SRC_AVRCP);
   int max = audioPlayGetVolumeMax(AUDIO_PLY_SRC_AVRCP);
   return ((db-min)*APP_AVRCP_VOL_MAX)/(max-min);
}

void hsc_AvrcpSetSpkVol(UINT8 level)
{
#if HAL_USE_AUDIO
  //UINT16 a2dpState = APP_BT_STATE_NULL;
  if (level <= 0) {
    audioPlayMute();
  }
  else {
    audioPlayUnmute();
  }

  s_app_avrcp_vol = level;
    
  //a2dpState = hsc_A2DP_GetState();
  //if (a2dpState >= APP_A2DP_STATE_START
  //   && App_AudioPlayLocalStop()==0 // when play tone, do not set local vol
  //)
  {
    //hs_printf("%d,\r\n", hsc_AudioLevel2DB(level));
    hs_audio_volSet(AVOL_DEV_NOR, hsc_AudioLevel2DB(level));
  }
#endif
}

UINT8 hsc_AvrcpGetSpkVol(void)
{
    return s_app_avrcp_vol;
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void hsc_AvrcpStartByRole(UINT8 role)
{
  if (role == 1) {
    AVRCP_ServerStart(AVRCP_CT, 0);
  }
  else if (role == 2) {
    AVRCP_ServerStart(AVRCP_TG, 0);
  }
  else if (role == 3) {
    AVRCP_ServerStart(AVRCP_TG | AVRCP_CT, 0);
  }

  if (role > 0) {
    AVRCP_RegCbk((UINT8 *)APP_AvrcpIndCbk);
  }
  //s_app_avrcp_vol = g_bt_host_config.attrs.a2dpSpkVol;
  //hs_audio_volRestore(AVOL_DEV_NOR);
  s_app_avrcp_vol = hsc_AudioDB2level(hs_audio_volGet(AVOL_DEV_NOR));
}

//static UINT32 msg_vol_last_tt;
//static UINT8 g_tone_interval = 1;
void hsc_AvrcpUpVol(void)
{
   s_app_avrcp_vol = s_app_avrcp_vol+APP_AVRCP_VOL_STEP;
   s_app_avrcp_vol = s_app_avrcp_vol>127?127:s_app_avrcp_vol;
   audioPlayUnmute();
   hs_audio_volSet(AVOL_DEV_NOR, hsc_AudioLevel2DB(s_app_avrcp_vol));
}

void hsc_AvrcpDownVol(void)
{
   s_app_avrcp_vol = s_app_avrcp_vol>APP_AVRCP_VOL_STEP?(s_app_avrcp_vol-APP_AVRCP_VOL_STEP):0;
   if (s_app_avrcp_vol == 0)
    audioPlayMute();
   hs_audio_volSet(AVOL_DEV_NOR, hsc_AudioLevel2DB(s_app_avrcp_vol));
}

void hsc_AvrcpChangeVol(UINT8* bd, BOOL up)
{
  UINT8 vol = 0;

  if (up) {
    #if 0
    if (s_app_avrcp_vol>=127) {
#if HS_USE_CONF
      /* don't play tone frequently */
      if (GetCurrTime() - msg_vol_last_tt > MS2ST(g_tone_interval * 1000)) {
        hs_cfg_sysSendMessage(HS_CFG_MODULE_SYS, HS_CFG_SYS_STATUS, HS_CFG_STATUS_VOLUME_MAX);
        msg_vol_last_tt = GetCurrTime();
      }
#endif
      return;
    }
    #endif
    s_app_avrcp_vol = s_app_avrcp_vol+APP_AVRCP_VOL_STEP;
    s_app_avrcp_vol = s_app_avrcp_vol>127?127:s_app_avrcp_vol;
    App_AVRCP_VolumeUp_Pushed(bd);
    App_AVRCP_VolumeUp_Released(bd);
  }
  else {
    #if 0
    if (s_app_avrcp_vol<=0) {
#if HS_USE_CONF
      hs_cfg_sysSendMessage(HS_CFG_MODULE_SYS, HS_CFG_SYS_STATUS, HS_CFG_STATUS_VOLUME_MIN);
#endif
      return;
    }
    #endif
    s_app_avrcp_vol = s_app_avrcp_vol>APP_AVRCP_VOL_STEP?(s_app_avrcp_vol-APP_AVRCP_VOL_STEP):0;
    App_AVRCP_VolumeDown_Pushed(bd);
    App_AVRCP_VolumeDown_Released(bd);
  }
  hsc_AvrcpSetSpkVol(s_app_avrcp_vol);
  /* patch15 for HS6600B102: avoid mute suddenly */
  vol = s_app_avrcp_vol;
  //hs_printf("ACRCP: hsc_AvrcpChangeVol =%d\r\n", s_app_avrcp_vol);
  AVRCP_AdvancedControl_Req(bd, AVRCP_PDUID_SET_ABSOLUTE_VOLUME, &vol, sizeof(UINT8)); 
  hsc_AvrcpVolNotifRsp(AVRCP_RSP_INTERIM, AVRCP_EVENT_VOLUME_CHANGED);
  //hsc_AvrcpRegisterNotifReq(bd, AVRCP_EVENT_VOLUME_CHANGED);
}

void hsc_AvrcpVolChange(UINT8* bd, int db)
{
  UINT8 vol = hsc_AudioDB2level(db);

  if (s_app_avrcp_vol<=vol) {
    s_app_avrcp_vol = vol;
    App_AVRCP_VolumeUp_Pushed(bd);
    App_AVRCP_VolumeUp_Released(bd);
  }
  else {
    s_app_avrcp_vol = vol;
    App_AVRCP_VolumeDown_Pushed(bd);
    App_AVRCP_VolumeDown_Released(bd);
  }
  hsc_AvrcpSetSpkVol(s_app_avrcp_vol);
  /* patch15 for HS6600B102: avoid mute suddenly */
  vol = s_app_avrcp_vol;
  //hs_printf("ACRCP: hsc_AvrcpChangeVol =%d\r\n", s_app_avrcp_vol);
  AVRCP_AdvancedControl_Req(bd, AVRCP_PDUID_SET_ABSOLUTE_VOLUME, &vol, sizeof(UINT8)); 
  hsc_AvrcpVolNotifRsp(AVRCP_RSP_INTERIM, AVRCP_EVENT_VOLUME_CHANGED);
  //hsc_AvrcpRegisterNotifReq(bd, AVRCP_EVENT_VOLUME_CHANGED);
}

void hsc_AvrcpSetLocalVol(void)
{
  s_app_avrcp_vol = hsc_AudioDB2level(hs_audio_volGet(AVOL_DEV_NOR));
  //hs_printf("setlocalvol =%d", s_app_avrcp_vol);
}

/* called on A2DP Stream Start by either of mobile phone */
void hsc_AvrcpSwitchLocalVol(UINT8 *bd)
{
  struct AppConnInst *connection = App_FindConnectionInstByBD(CLS_AVRCP_CT, bd, APP_CONNECT_IGNORE_ROLE);
  struct App_AvrcpConnectionInst *inst;

  /* HS6600A4: avoid that the volume become very low or mute,
     because if some phones connect to bt device during playing music,
     there is no AVRCP connection but A2DP connection. */
  if (connection == NULL)
    return;

  inst = (struct App_AvrcpConnectionInst *)(connection->profile_inst_handle);
  if (inst != NULL)
  {
    s_app_avrcp_conn_handle = inst->conn_hdl;
    //s_app_avrcp_vol = inst->vol;
    s_app_avrcp_vol = hsc_AudioDB2level(hs_audio_volGet(AVOL_DEV_NOR));
    //hs_printf("AVRCP: current handle=%d, vol= %d\r\n", s_app_avrcp_conn_handle, s_app_avrcp_vol);
  }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void hsc_AvrcpFsmChangeVol(struct FsmInst *fi, UINT8 event, void* in)
{
  (void)fi;
  (void)event;
  (void)in;
   struct AppConnInst *connection = App_FindConnectionInstByUpperHandle(s_app_avrcp_conn_handle);
   if (connection != NULL ) {
      hsc_AvrcpChangeVol(connection->bd, TRUE);
   }
}

/*
 * PTS
 */
void hsc_AVRCP_Play(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByUpperHandle(s_app_avrcp_conn_handle);
  if (connection != NULL ) {
    App_AVRCP_Play_Pushed(connection->bd);
    App_AVRCP_Play_Released(connection->bd);
  }
}

void hsc_AVRCP_Pause(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByUpperHandle(s_app_avrcp_conn_handle);
  if (connection != NULL ) {
    App_AVRCP_Pause_Pushed(connection->bd);
    App_AVRCP_Pause_Released(connection->bd);
  }
}

void hsc_AVRCP_Stop(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByUpperHandle(s_app_avrcp_conn_handle);
  if (connection != NULL ) {
    App_AVRCP_Stop_Pushed(connection->bd);
    App_AVRCP_Stop_Released(connection->bd);
  }
}

void hsc_AVRCP_Rewind(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByUpperHandle(s_app_avrcp_conn_handle);
  if (connection != NULL ) {
    App_AVRCP_Rewind_Pushed(connection->bd);
    App_AVRCP_Rewind_Released(connection->bd);
  }
}

void hsc_AVRCP_Forward(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByUpperHandle(s_app_avrcp_conn_handle);
  if (connection != NULL ) {
    App_AVRCP_Forward_Pushed(connection->bd);
    App_AVRCP_Forward_Released(connection->bd);
  }
}

void hsc_AVRCP_Backward(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByUpperHandle(s_app_avrcp_conn_handle);
  if (connection != NULL ) {
    App_AVRCP_Backward_Pushed(connection->bd);
    App_AVRCP_Backward_Released(connection->bd);
  }
}

void hsc_AVRCP_FFWD(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByUpperHandle(s_app_avrcp_conn_handle);
  if (connection != NULL ) {
    App_AVRCP_FFWD_Pushed(connection->bd);
    App_AVRCP_FFWD_Released(connection->bd);
  }
}

void hsc_AVRCP_VolUp(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByUpperHandle(s_app_avrcp_conn_handle);
  if (connection != NULL ) {
    hsc_AvrcpChangeVol(connection->bd, TRUE);
  }
}

void hsc_AVRCP_VolDown(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByUpperHandle(s_app_avrcp_conn_handle);
  if (connection != NULL ) {
    hsc_AvrcpChangeVol(connection->bd, FALSE);
  }
}

void hsc_AVRCP_DisconnectAll(void)
{
  App_AVRCP_Disconnect(s_app_avrcp_conn_handle);
  debug("%s\r\n", __FUNCTION__);
}

/*
 * TG side: mobile phone generally
 */

static void hsc_AvrcpTGGetCapabilitiesRsp(UINT8 *bd, UINT32 event, UINT8 cap_id)
{
  /* bd, event, cap_id come from the callback input, volume is the actual volume value set */
  struct AVRCP_GetCapabilitiesRspStru *data;
  UINT16 size;
 
  switch (cap_id) {
  case AVRCP_CAPABILITYID_COMPANY_ID:
    size = 1 + 1 + 3; /* Count = 1, Capability = Count * 3 */
    data = NEW(size);
    data->count = 1;
    data->capability[0] = 0x00;
    data->capability[1] = 0x19;
    data->capability[2] = 0x58;
    break;

  case AVRCP_CAPABILITYID_EVENTS_SUPPORTED:
    size = 1 + 1 + 2; /* Count = 2 */
    data = NEW(size);
    data->count = 2;
    data->capability[0] = AVRCP_EVENT_BATT_STATUS_CHANGED;
    data->capability[1] = AVRCP_EVENT_VOLUME_CHANGED;
    break;

  default:
    data = NULL;
    break;
  }

  if (data != NULL) {
    data->id = cap_id;
    AVRCP_AdvancedControl_Rsp(bd, event, data, size);
    FREE(data);
  } else {
    AVRCP_GeneralReject_Rsp(bd, AVRCP_ERROR_INVALID_PARAMETER, event);
  }
  debug("AVRCP: GetCapabilitiesRsp cap_id=%d, event=%d\r\n", cap_id, event);
}

static void hsc_AvrcpSetVolByAddr(UINT8* bd, UINT8 vol)
{
  struct AppConnInst *connection = App_FindConnectionInstByBD(CLS_AVRCP_CT, bd, APP_CONNECT_IGNORE_ROLE);
  struct App_AvrcpConnectionInst *inst;
  if (connection != NULL) {
    inst = (struct App_AvrcpConnectionInst *)(connection->profile_inst_handle);
    if (inst == NULL)
      return;

    inst->vol = vol;
    if (s_app_avrcp_conn_handle == inst->conn_hdl)
    {
        UINT32 remote_vol = ((UINT32)vol | (1<<8));
        hs_cfg_sysSendMsg(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_REMOTE_VOL, (void*)(remote_vol));
    }
  }
}

static void hsc_AvrcpTGSetAbsoluteVolRsp(UINT8 *bd, UINT32 event, UINT8 volume)
{
  /* bd, event come from the callback input, volume is the actual volume value set */
  AVRCP_AdvancedControl_Rsp(bd, event, &volume, 1);
  hsc_AvrcpSetVolByAddr(bd, volume);
}

static void hsc_AvrcpRegisterNotifReq(UINT8 *bd, UINT8 event_id)
{
  /* don't NEW for in, refer to AVRCP_RegisterNotification_Cfm() */
  struct AVRCP_RegisterNotifReqStru in;

  in.event_id = event_id;
  if (event_id == AVRCP_EVENT_PLAYBACK_POS_CHANGED) {
    in.playback_interval = 0x12345678;
  } else {
    in.playback_interval = 0;
  }
   
  AVRCP_AdvancedControl_Req(bd, AVRCP_PDUID_REGISTER_NOTIFICATION, (UINT8 *)(&in), sizeof(struct AVRCP_RegisterNotifReqStru));
}

static void hsc_AvrcpTGRegisterNotifRsp(UINT8 *bd, UINT8 rsp_code, UINT8 event_id, UINT32 interval)
{
  UINT32 tlcr_pdu_id = AVRCP_PDUID_REGISTER_NOTIFICATION;
  struct AVRCP_RegisterNotifRspStru rsp;
  (void)bd;
  (void)interval;
  
  if (event_id == AVRCP_EVENT_VOLUME_CHANGED) {
    AVRCP_SET_RSPTYPE(tlcr_pdu_id, rsp_code);
    rsp.event_id = event_id;
    //s_app_avrcp_vol = (UINT8)interval;
    rsp.param[0] = s_app_avrcp_vol;
   
 #if 0 
    remote_vol = ((UINT32)s_app_avrcp_vol | (1<<8));
    hs_cfg_sysSendMsg(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_REMOTE_VOL, (void*)(remote_vol));
 #endif
  }
  else {
    AVRCP_SET_RSPTYPE(tlcr_pdu_id, AVRCP_RSP_NOT_IMPLEMENTED); //AVRCP_RSP_REJECTED 
    rsp.event_id = event_id;
    debug("AVRCP: RegisterNotifRsp not implemented event=%d,\r\n", event_id);
  }
  AVRCP_RegisterNotif_Rsp(tlcr_pdu_id, &rsp, 2);
}

static void hsc_AvrcpVolNotifRsp(UINT8 rsp_code, UINT8 event_id)
{
  UINT32 tlcr_pdu_id = AVRCP_PDUID_REGISTER_NOTIFICATION;
  struct AVRCP_RegisterNotifRspStru rsp;
  
  if (event_id == AVRCP_EVENT_VOLUME_CHANGED) {
    AVRCP_SET_RSPTYPE(tlcr_pdu_id, rsp_code);
    rsp.event_id = event_id;
    rsp.param[0] = s_app_avrcp_vol;
  }
  
  AVRCP_RegisterNotif_Rsp(tlcr_pdu_id, &rsp, 2);
}

static void hsc_AvrcpTGPassThroughRsp(UINT8 *bd, struct AVRCP_PassThroughStru *param, UINT32 event)
{
  if ((param != NULL) && 
      ((param->op_id == AVRCP_OPID_PLAY) ||
       (param->op_id == AVRCP_OPID_STOP) ||
       (param->op_id == AVRCP_OPID_PAUSE))) {
    if (param->state_flag == 1)
    {
      if (param->op_id == AVRCP_OPID_PLAY)
      {
        debug("PH:start\r\n");
        hsc_A2dpSetState(bd, APP_A2DP_STATE_START);
      }
      else
      {
        debug("PH:pause\r\n");
        hsc_A2dpSetState(bd, APP_A2DP_STATE_PAUSE);
      }
    }
  }
  else
  {
    AVRCP_SET_RSPTYPE(event, AVRCP_RSP_NOT_IMPLEMENTED);	
    AVRCP_PassThrough_Rsp(bd, param, event);
  }
}

static void hsc_AvrcpCTRegisterNotifInd(UINT8* bd, UINT8* rsp, UINT32 event)
{
  UINT8 rsp_type;
  UINT8 value;

  if (rsp == NULL)
    return;

  rsp_type = AVRCP_GET_4BIT_CR(event);
  if ((rsp[0] == AVRCP_EVENT_PLAYBACK_STATUS_CHANGED) &&
      ((rsp_type == AVRCP_RSP_CHANGED) || (rsp_type == AVRCP_RSP_INTERIM))) {
    value = rsp[1];
    if (value == AVRCP_PLAYSTATUS_PLAYING)
    {
      debug("RG:start\r\n");
      hsc_A2dpSetState(bd, APP_A2DP_STATE_START);
    }
    /* HS6600A4: omit the STOPPED notification from Legend's ZUK or Huawei Honor6+ mobile phone */
    else if (/*value == AVRCP_PLAYSTATUS_STOPPED || */value == AVRCP_PLAYSTATUS_PAUSED)
    {
      debug("RG:pause\r\n");
      hsc_A2dpSetState(bd, APP_A2DP_STATE_PAUSE);
    }
    else
    {
      debug("RG:value=%d,rsp_type=%d\r\n",value,rsp_type);
    }
  }
  else if ((rsp[0] == AVRCP_EVENT_VOLUME_CHANGED) && (rsp_type == AVRCP_RSP_CHANGED)) {
    //hs_printf("notify volume change\r\n");
    // change vol
    hsc_AvrcpSetVolByAddr(bd, rsp[1]);
    // re register vol
    hsc_AvrcpRegisterNotifReq(bd, AVRCP_EVENT_VOLUME_CHANGED);
  }
  else if (rsp[0] == AVRCP_EVENT_VOLUME_CHANGED) {
    hs_printf("notify volume change %d\r\n", rsp_type);
  }
}

#endif /* #if defined(CONFIG_PATCH_HUNTERSUN) */

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Description:
	This sample function is used by app to register callback to AVRCP.
	event: 	<8bit reserved>,
			<4bit reserved>,<4bit tl>,
			<4bit ctype/rsp>,<1bit channel>,<1bit grouping reserved>,<2bit cr/grouping>,
			<8bit pdu_id>
-----------------------------------------------------------------------------*/
void APP_AvrcpIndCbk(UINT8 *bd, UINT32 event, UINT8 *param, UINT16 len)
{
	(void)len;
	UINT8 rsp_type;

	//debug("AVRCP: event=%d\r\n", event);
	switch (AVRCP_GET_CBK_GROUPING(event)) {/* grouping */
		case AVRCP_EV_GROUP_CONNECTION:/* Connection */
			switch (event) {
				case AVRCP_EV_CONNECT_COMPLETE: /* param: struct AVRCP_ConnectCompleteStru */
					App_SCH_FsmEvent(App_Avrcp_FsmConnectComplete, param);
					param = NULL;
					break;
				case AVRCP_EV_DISCONNECT_COMPLETE: /* param: struct AVRCP_ConnectCompleteStru */
					App_SCH_FsmEvent(App_Avrcp_FsmDisconnectComplete, param);
					param = NULL;
					break;
			}
			break;

		case AVRCP_EV_GROUP_RESPONSE:/* Confirmation - CT side */
			rsp_type = AVRCP_GET_4BIT_CR(event);
			if ((rsp_type == AVRCP_RSP_REJECTED) || (rsp_type == AVRCP_RSP_NOT_IMPLEMENTED)) { /* param: struct AVRCP_GeneralRejectCfmStru */
			} else {
				switch (AVRCP_GET_PDUID(event)) {
					case AVRCP_PDUID_GET_CAPABILITIES: /* param: struct AVRCP_GetCapabilitiesRspStru */
						break;
					case AVRCP_PDUID_LIST_PLAYER_APPLICATION_SETTING_ATTRIBUTES: /* param: struct AVRCP_NumIDStru */
						break;
					case AVRCP_PDUID_LIST_PLAYER_APPLICATION_SETTING_VALUES: /* param: struct AVRCP_NumIDStru */
						break;
					case AVRCP_PDUID_GET_CURRENTPLAYER_APPLICATION_SETTING_VALUE: /* param: struct AVRCP_NumIDPairStru */
						break;
					case AVRCP_PDUID_SET_CURRENTPLAYER_APPLICATION_SETTING_VALUE:
						break;
					case AVRCP_PDUID_GET_PLAYER_APPLICATION_SETTING_ATTRIBUTE_TEXT: /* param: struct AVRCP_NumIDStringStru */
						break;
					case AVRCP_PDUID_GET_PLAYER_APPLICATION_SETTING_VALUE_TEXT: /* param: struct AVRCP_NumIDStringStru */
						break;
					case AVRCP_PDUID_INFORM_DISPLAYABLE_CHARACTERSET:
						break;
					case AVRCP_PDUID_INFORM_BATTERYSTATUS_OF_CT:
						break;
					case AVRCP_PDUID_GET_ELEMENT_ATTRIBUTES: /* param: struct AVRCP_Num4IDValueStru */
						break;
					case AVRCP_PDUID_GET_PLAY_STATUS: /* param: struct AVRCP_GetPlayStatusRspStru */
						break;
					case AVRCP_PDUID_REGISTER_NOTIFICATION: /* param: struct AVRCP_RegisterNotifRspStru */
						hsc_AvrcpCTRegisterNotifInd(bd, param, event);
						break;
					case AVRCP_PDUID_REQUEST_CONTINUING_RESPONSE:
						break;
					case AVRCP_PDUID_ABORT_CONTINUING_RESPONSE:
						break;
				        case AVRCP_PDUID_SET_ABSOLUTE_VOLUME: /* param: UINT8 */
						break;
					case AVRCP_PDUID_SET_ADDRESSED_PLAYER: /* param: UINT8 */
						break;
					case AVRCP_PDUID_SET_BROWSED_PLAYER: /* param: struct AVRCP_SetBrowsedPlayerRspStru */
					case AVRCP_PDUID_GET_FOLDER_ITEMS: /* param: struct AVRCP_GetFolderItemsRspStru */
					case AVRCP_PDUID_CHANGE_PATH: /* param: struct AVRCP_ChangePathRspStru */
					case AVRCP_PDUID_GET_ITEM_ATTRIBUTES: /* param: struct AVRCP_GetItemAttributesRspStru */
					case AVRCP_PDUID_SEARCH: /* param: struct AVRCP_SearchRspStru */
						break;
					case AVRCP_PDUID_PLAY_ITEM: /* param: UINT8 */
						break;
					case AVRCP_PDUID_ADDTO_NOWPLAYING: /* param: UINT8 */
						break;
					case AVRCP_PDUID_GENERAL_REJECT: /* param: UINT8 */
						break;
					case AVRCP_PDUID_PASSTHROUGH: /* param: struct AVRCP_PassThroughStru */
						break;							
					default:
						break;
				}
			}
			break;

		case AVRCP_EV_GROUP_COMMAND:/* Indication - TG side */
#if defined(CONFIG_PATCH_HUNTERSUN)
			switch (AVRCP_GET_PDUID(event)) {
				case AVRCP_PDUID_GET_CAPABILITIES:
					hsc_AvrcpTGGetCapabilitiesRsp(bd, event, *param);
					break;
				case AVRCP_PDUID_REGISTER_NOTIFICATION:
					{
						struct AVRCP_RegisterNotifReqStru *req = (struct AVRCP_RegisterNotifReqStru *)param;
						hsc_AvrcpTGRegisterNotifRsp(bd, AVRCP_RSP_INTERIM, req->event_id, req->playback_interval);
					}
					break;
				case AVRCP_PDUID_SET_ABSOLUTE_VOLUME:
					hsc_AvrcpTGSetAbsoluteVolRsp(bd, event, *param);
					break;

				case AVRCP_PDUID_PASSTHROUGH:
					/* Auomatically reject Passthrough commands */
					hsc_AvrcpTGPassThroughRsp(bd, (struct AVRCP_PassThroughStru *)param, event);
					//AVRCP_SET_RSPTYPE(event, AVRCP_RSP_NOT_IMPLEMENTED);	
					//AVRCP_PassThrough_Rsp(bd, (struct AVRCP_PassThroughStru *)param, event);
					break;
				case AVRCP_PDUID_SET_BROWSED_PLAYER:
				case AVRCP_PDUID_GET_FOLDER_ITEMS:
				case AVRCP_PDUID_CHANGE_PATH:
				case AVRCP_PDUID_GET_ITEM_ATTRIBUTES:
				case AVRCP_PDUID_SEARCH:
					/* Automatically reject the Browsing commands */
					{
					UINT8 err_code =  AVRCP_ERROR_INVALID_COMMAND;
					AVRCP_AdvancedControl_Rsp(bd, event, &err_code, 1);
					debug("AVRCP:event=%d, invald command\r\n", event);
					}
					break;
				default:
					/* Automatically reject the AV/C commands */
					AVRCP_GeneralReject_Rsp(bd, AVRCP_ERROR_INVALID_COMMAND, event);
					break;
			}
#endif
			break;

		default:
			break;
	}
	AVRCP_FreeEventParam(event, param);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_AVRCP_SearchTGCbk(void *app_context, UCHAR *bd, WORD status, WORD svc_type, struct SDAP_GeneralInfoStru *sdx_info)
{
	(void)app_context;
	(void)svc_type;
	struct AppConnInst *connection = App_FindConnectionInstByBD(CLS_AVRCP_CT, bd, APP_CONNECT_IGNORE_ROLE);

	if (connection != NULL) {
		/* in->side shall be AVRCP_CHANNEL_INITIATOR */
		if (connection->state == APP_CONNECT_STATE_CONNECTED) {
			/* Be connected while searching remote TG service */
#if defined(CONFIG_MMI)
			MMI_AvrcpConnectCfm(bd, HCI_STATUS_OK, connection->connection_handle);
#endif
		} else {
			if (status == ERROR_SDAP_SUCCESS) {
				struct AVRCP_ConnectReqStru *in = NEW(sizeof(struct AVRCP_ConnectReqStru));
				memcpy(in->bd_addr, bd, BD_ADDR_LEN);
			    if (((struct SDAP_AVRCPInfoStru *)sdx_info)->sup_cg & AVCT_FEATURE_BROWSING) {
				    in->browsing_support = 1;
				} else {
				    in->browsing_support = 0;
				}
				AVRCP_ConnectReq(in);
			} else {
#if defined(CONFIG_MMI)
				MMI_AvrcpConnectCfm(bd, HCI_STATUS_UNKNOWN_CONNECTION_IDENTIFIER, 0);
#endif
				FREE(connection->profile_inst_handle);
				App_DeleteConnectionInst(connection);
			}
		}
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_Avrcp_FsmConnect(struct FsmInst *fi, UINT8 event, UINT8 *bd)
{
	(void)fi;
	(void)event;
	struct AppConnInst *connection = App_FindConnectionInstByBD(CLS_AVRCP_CT, bd, APP_CONNECT_IGNORE_ROLE);
	if (NULL == connection) {
		App_AddConnectionInst(CLS_AVRCP_CT, bd, APP_CONNECT_INITIATOR);
		SDAP_GetServiceInfoBDReq(bd, CLS_AVRCP_TG, NULL, 0, NULL, App_AVRCP_SearchTGCbk);
	} else {
#if defined(CONFIG_MMI)
		if (connection->profile_inst_handle == NULL) {
			/* Only one AVRCP connection is allowed between two devices */
			MMI_AvrcpConnectCfm(bd, HCI_STATUS_CONNECTION_LIMIT_EXCEEDED, 0);
		} else {
			/* Return the current connection handle */
			MMI_AvrcpConnectCfm(bd, HCI_STATUS_OK, connection->connection_handle);
		}
#endif
	}
	FREE(bd);
}

/*******************************************************************
*																	*
********************************************************************/
static void App_Avrcp_FsmDisconnect(struct FsmInst *fi, UINT8 event, UINT16 *in)
{
	(void)fi;
	(void)event;
	struct AppConnInst *connection = NULL;
	if (*in != 0) {
		connection = App_FindConnectionInstByUpperHandle(*in);
		if (connection != NULL) {
			if ((connection->profile_inst_handle != NULL) && (connection->service_class == CLS_AVRCP_CT)) {
				connection->state = APP_CONNECT_STATE_WAIT4_DISCONNECT; /* Wait for Disconnect Complete Confirmation */
				AVRCP_DisconnectReq(connection->bd);
			} else {
				connection = NULL; /* Unexpected disconnect request */
			}
		}
	}
#if defined(CONFIG_MMI)
	if (connection == NULL) {
		MMI_AvrcpDisconnectCfm(HCI_STATUS_UNKNOWN_CONNECTION_IDENTIFIER, *in);
	}
#endif
	FREE(in);
#if HAL_USE_AUDIO
	hsc_CFG_SaveA2dpVol(s_app_avrcp_vol);
#endif
}

void App_Avrcp_FsmConnectComplete(struct FsmInst *fi, UINT8 event, struct AVRCP_ConnectCompleteStru *in)
{
	(void)fi;
	(void)event;
	/* Only one AVRCP connection is allowed between two devices, either side may be CT and TG at the same time */
#if defined(CONFIG_MMI)
	UINT16 app_cbk = 0;
	UINT16 result = HCI_STATUS_OK;
	UINT16 connection_handle = 0;
#endif
	struct App_AvrcpConnectionInst *inst;
	struct AppConnInst *connection = App_FindConnectionInstByBD(CLS_AVRCP_CT, in->bd_addr, APP_CONNECT_IGNORE_ROLE);

	if (in->result == BT_SUCCESS) {
#if defined(CONFIG_PATCH_HUNTERSUN)
		if (connection == NULL) {
			connection = App_AddConnectionInst(CLS_AVRCP_CT, in->bd_addr, APP_CONNECT_RESPONDER);
		}
		if (connection->profile_inst_handle == NULL) {
			inst = NEW(sizeof(struct App_AvrcpConnectionInst));
			memset(inst, 0, sizeof(struct App_AvrcpConnectionInst));
			connection->profile_inst_handle = inst;
		} else {
			inst = connection->profile_inst_handle;
		}
#else
		if (connection == NULL) {
			connection = App_AddConnectionInst(CLS_AVRCP_CT, in->bd_addr, APP_CONNECT_RESPONDER);
			inst = NEW(sizeof(struct App_AvrcpConnectionInst));
			memset(inst, 0, sizeof(struct App_AvrcpConnectionInst));
			connection->profile_inst_handle = inst;
		} else {
			inst = connection->profile_inst_handle;
		}
#endif
		if (in->channel == AVRCP_CONTROL_CHANNEL) {
#if defined(CONFIG_PATCH_HUNTERSUN)
			hsc_UpdateConnectionInst(connection, APP_CONNECT_STATE_CONNECTED);
#endif
#if defined(CONFIG_MMI)
			connection->state = APP_CONNECT_STATE_CONNECTED;
			connection_handle = connection->connection_handle;
			app_cbk = 1;
#endif
#if HAL_USE_AUDIO
			if (hsc_A2DP_GetState() != APP_A2DP_STATE_START)
			{
				s_app_avrcp_conn_handle = connection->connection_handle;
			}
#endif
		} else { /* Shall be AVRCP_BROWSING_CHANNEL */
			inst->brw_chnl_mtu = in->mtu;
		}
#if defined(CONFIG_PATCH_HUNTERSUN)
		inst->conn_hdl = connection->connection_handle;
		inst->vol = s_app_avrcp_vol;
		//memcpy(inst->bd, in->bd_addr, BD_ADDR_LEN);
		hsc_AvrcpRegisterNotifReq(in->bd_addr, AVRCP_EVENT_PLAYBACK_STATUS_CHANGED);
		// delete register vol change event, for: first download code to flash, the auk connect device, play audio, the player error.
		//hsc_AvrcpRegisterNotifReq(in->bd_addr, AVRCP_EVENT_VOLUME_CHANGED);
#endif
	} else {
		if (in->channel == AVRCP_CONTROL_CHANNEL) {
			if (connection != NULL) {
#if defined(CONFIG_MMI)
				result = HCI_STATUS_UNSPECIFIED_ERROR;
				app_cbk = 1;
#endif
#if defined(CONFIG_PATCH_HUNTERSUN)
				inst = (struct App_AvrcpConnectionInst *)(connection->profile_inst_handle);
				if (inst != NULL)
				{
					if (inst->conn_hdl == s_app_avrcp_conn_handle)
					{
						s_app_avrcp_conn_handle = 0;
					}
				}
#endif
				FREE(connection->profile_inst_handle);
				App_DeleteConnectionInst(connection);
			}
		} /* Ignore Browsing channel failure */
	}
#if defined(CONFIG_MMI)
	if (app_cbk) {
		if (in->side == AVRCP_CHANNEL_INITIATOR) {
			MMI_AvrcpConnectCfm(in->bd_addr, result, connection_handle);
		} else {
			MMI_AvrcpConnectInd(in->bd_addr, connection_handle);
		}
	}
#endif
	FREE(in);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_Avrcp_FsmDisconnectComplete(struct FsmInst *fi, UINT8 event, struct AVRCP_ConnectCompleteStru *in)
{
	(void)fi;
	(void)event;
	struct AppConnInst *connection = App_FindConnectionInstByBD(CLS_AVRCP_CT, in->bd_addr, APP_CONNECT_IGNORE_ROLE);

	if (connection != NULL) {
#if defined(CONFIG_MMI)
		if (connection->state == APP_CONNECT_STATE_WAIT4_DISCONNECT) {
			MMI_AvrcpDisconnectCfm(HCI_STATUS_OK, connection->connection_handle);
		} else {
			MMI_AvrcpDisconnectInd(connection->connection_handle);
		}
#endif
		FREE(connection->profile_inst_handle);
		App_DeleteConnectionInst(connection);
	}
	FREE(in);
#if HAL_USE_AUDIO
    hsc_CFG_SaveA2dpVol(s_app_avrcp_vol);
#endif
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_Avrcp_FsmPassThrougReq(struct FsmInst *fi, UINT8 event, struct App_PassThroughReq *in)
{
	(void)fi;
	(void)event;
	struct AppConnInst *connection = App_FindConnectionInstByBD(CLS_AVRCP_CT, in->bd, APP_CONNECT_IGNORE_ROLE);
	if (connection == NULL) {
		while ((connection = App_FindNextConnectionInst(connection)) != NULL) {
			if ((connection->service_class == CLS_AVRCP_CT) &&
			   (connection->state == APP_CONNECT_STATE_CONNECTED)) {
				break;
			}
		}
	} else {
		if (connection->state != APP_CONNECT_STATE_CONNECTED) {
			connection = NULL;
		}
	}
	if (connection != NULL) {
		AVRCP_PassThrough_Req(connection->bd, &in->parameter);
	} else {
#if defined(CONFIG_MMI)
		MMI_AvrcpPassThroughCfm(connection->bd, HCI_STATUS_UNSPECIFIED_ERROR);
#endif
	}
	FREE(in);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_AVRCP_Start(void)
{
	AVRCP_ServerStart(AVRCP_CT, 0);
	AVRCP_RegCbk((UINT8 *)APP_AvrcpIndCbk);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_AVRCP_Connect(UINT8 *bd)
{
	UINT8 *in = NEW(BD_ADDR_LEN);
	memcpy(in, bd, BD_ADDR_LEN);
	App_UI_FsmEvent(App_Avrcp_FsmConnect, in);
}

/*******************************************************************
*																	*
********************************************************************/
void App_AVRCP_Disconnect(UINT16 connection_handle)
{
	UINT16 *in = NEW(sizeof(UINT16));
	*in = connection_handle;
	App_UI_FsmEvent(App_Avrcp_FsmDisconnect, in);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_AVRCP_ButtonPushed(UINT8 *bd, UINT8 op_id, UINT8 vendor_id)
{
	struct App_PassThroughReq *in;
	
	in = NEW(sizeof(struct App_PassThroughReq));
	if (bd != NULL) {
		memcpy(in->bd, bd, BD_ADDR_LEN);
	} else {
		memset(in->bd, 0, BD_ADDR_LEN);
	}
	in->parameter.vendor_unique_id = vendor_id;
	in->parameter.state_flag = AVRCP_STATE_FLAG_BUTTON_PUSHED;
	in->parameter.op_id = op_id;
#if defined(CONFIG_PATCH_HUNTERSUN)
	App_Avrcp_FsmPassThrougReq(NULL/*fi*/, 0/*event*/, in);
#else
	App_SCH_FsmEvent(App_Avrcp_FsmPassThrougReq, in);
#endif
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_AVRCP_ButtonReleased(UINT8 *bd, UINT8 op_id, UINT8 vendor_id)
{
	struct App_PassThroughReq *in;
	
	in = NEW(sizeof(struct App_PassThroughReq));
	if (bd != NULL) {
		memcpy(in->bd, bd, BD_ADDR_LEN);
	} else {
		memset(in->bd, 0, BD_ADDR_LEN);
	}
	in->parameter.vendor_unique_id = vendor_id;
	in->parameter.state_flag = AVRCP_STATE_FLAG_BUTTON_RELEASED;
	in->parameter.op_id = op_id;
#if defined(CONFIG_PATCH_HUNTERSUN)
	App_Avrcp_FsmPassThrougReq(NULL/*fi*/, 0/*event*/, in);
#else
	App_SCH_FsmEvent(App_Avrcp_FsmPassThrougReq, in);
#endif
}

#endif /* CONFIG_AVRCP */
