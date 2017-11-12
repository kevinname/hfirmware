#include "app_main.h"

extern cnt_t osMessageGetUseCount(osMessageQId queue_id);

static struct FsmInst *app_fsm_inst;
static volatile UINT16 app_fsm_bt_state = APP_BT_STATE_NULL;
static volatile UINT8 app_fsm_link_lost_retry_count = 0;
static volatile UINT8 app_fsm_auto_connect = 0;
static volatile UINT8 app_fsm_bt_mode = BT_HOST_VAR_MODE_AUDIO;
static UINT8 app_fsm_lost_retry_bd[BD_ADDR_LEN];

#define APP_FSM_SNIFF_DATA_STATE_LOSE     0x00
#define APP_FSM_SNIFF_DATA_STATE_IDLE     0x01
#define APP_FSM_SNIFF_DATA_STATE_BUSY     0x02
#define APP_FSM_SNIFF_DATA_STATE_SNIFF    0x03
#define APP_FSM_SNIFF_DATA_STATE_SUBRATE  0x04

static struct FsmTimer* app_fsm_pair_timer = NULL;
static struct FsmTimer* app_fsm_link_lost_retry = NULL;
static struct FsmTimer* app_fsm_power_on_reconnect = NULL;
#ifdef CONFIG_AVRCP
static struct FsmTimer* app_fsm_avrcp_connect_timer = NULL;
#endif
#ifdef CONFIG_HFP
//static struct FsmTimer* app_fsm_hfp_battery_timer = NULL;
static struct FsmTimer* app_fsm_hfp_sco_timer = NULL;
#endif

#if HAL_USE_AUDIO

#if defined(CONFIG_HFP) || defined(CONFIG_A2DP)
static hs_audio_config_t app_fsm_i2sCfg;
volatile UINT8 app_fsm_audio_start  = 0;
static volatile UINT8 app_fsm_play_remote_close = 0;
static volatile UINT8 app_fsm_play_local_close = 0;

static hs_audio_config_t app_fsm_record_i2sCfg;
static volatile UINT8 app_fsm_record_start = 0;
static volatile UINT8 app_fsm_record_remote_close = 0;
static volatile UINT8 app_fsm_record_local_close = 0;
static thread_t *app_thread_hfp_audio = NULL;
static volatile UINT8 app_thread_wakeup = 0;

osMessageQId app_thread_a2dp_msgid = NULL;

static volatile UINT32 app_fsm_audio_play_size = 0;
#endif /* defined(CONFIG_HFP) || defined(CONFIG_A2DP) */
#endif /* HAL_USE_AUDIO */

#define APP_CUDATA					((struct AppFsmUserData *)(app_fsm_inst->user_data))
#define APP_DEVICE_LIST				(APP_CUDATA->bonded_device_list)
#define APP_CONNECTION_LIST			(APP_CUDATA->connection_list)
#define APP_HANDLE_BASE				(APP_CUDATA->handle_base)
#define APP_ACL_LIST                (APP_CUDATA->acl_list)

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
static void App_CheckSniffTimer(struct FsmInst *fi, UINT8 ev, void *arg);
static void App_DelAutoConnectTimer(void);
static void App_DelLinkLostRetry(void);
static void App_StartLinkLostRetry(void);
static void App_LinkLostRetry(struct FsmInst *fi, UINT8 ev, void *arg);
static void App_DelPairTimer(struct FsmInst *fi);
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_SniffSubrateCbk(void *context, UINT16 result, struct GAP_ExecuteCommandCfmStru *cfm_par)
{
  struct HCI_Sniff_ModeStru sniffcmd;
  struct HCI_Sniff_SubratingCompStru* par = NULL;
  struct AppPowerModeStru *node = APP_ACL_LIST.head;
  
  (void)context;
  (void)cfm_par;
  
  if (cfm_par != NULL) {
     par = (struct HCI_Sniff_SubratingCompStru*)(cfm_par->event_parameter);
     
     while (node != NULL) {
       if ( node->acl_hdl == par->connection_handle) {
          break;
       }
        node = LNEXT(node);
     }
  }
  if (result == HCI_STATUS_OK && node != NULL) {
    
     sniffcmd.connection_handle = node->acl_hdl;
     sniffcmd.sniff_max_interval = g_bt_host_config.features.sniffMaxInterval;
     sniffcmd.sniff_min_interval = g_bt_host_config.features.sniffMinInterval;
     sniffcmd.sniff_attempt = g_bt_host_config.features.sniffAttempt;
     sniffcmd.sniff_timeout = g_bt_host_config.features.sniffTimeout;
     GAP_ExecuteCommandA(NULL, NULL, HCI_OPS_SNIFF_MODE, &sniffcmd, sizeof(struct HCI_Sniff_ModeStru), NULL);
  }
  if (result != HCI_STATUS_OK) {
     node->state = APP_FSM_SNIFF_DATA_STATE_SNIFF;
  }
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
static void App_SniffHandler(struct AppPowerModeStru * node)
{
   struct HCI_Sniff_ModeStru sniffcmd;
   struct HCI_Sniff_SubratingStru sniffSubrateCmd;
   tHCI_Current_Mode hcimode;
   uint16_t timeout = 0;
   
   if (node == NULL) return;
   
   if (g_bt_host_config.features.sniffEnable == 0) return;
     
   GAP_GetLinkMode(NULL, &node->addr, &hcimode);
    
   if (hcimode == HCI_CURRENT_MODE_ACTIVE) {
      if (node->state == APP_FSM_SNIFF_DATA_STATE_BUSY) {
         node->state = APP_FSM_SNIFF_DATA_STATE_IDLE;
         timeout = g_bt_host_config.features.sniffBusyInterval;
      }
      else if (node->state == APP_FSM_SNIFF_DATA_STATE_IDLE) {
         timeout = g_bt_host_config.features.sniffIdleInterval;
         node->state = APP_FSM_SNIFF_DATA_STATE_SNIFF;
      }
      else if (node->state == APP_FSM_SNIFF_DATA_STATE_SNIFF) {
         if (g_bt_host_config.features.sniffEnable > 1) {
            timeout = g_bt_host_config.features.sniffToSubrateTimeout;
            sniffSubrateCmd.connection_handle  = node->acl_hdl;
            sniffSubrateCmd.max_sniff_latency  = g_bt_host_config.features.sniffSubrateMaxLatency;
            sniffSubrateCmd.min_remote_timeout = g_bt_host_config.features.sniffSubrateMiniRemoteTimeout;
            sniffSubrateCmd.min_local_timeout  = g_bt_host_config.features.sniffSubrateMiniLocalTimeout;
            GAP_ExecuteCommandA(NULL, NULL, HCI_OPS_SNIFF_SUBRATING, &sniffSubrateCmd, sizeof(struct HCI_Sniff_SubratingStru), App_SniffSubrateCbk);
         }
         else {
            timeout = g_bt_host_config.features.sniffSuspendTimeout;
            sniffcmd.connection_handle = node->acl_hdl;
            sniffcmd.sniff_max_interval = g_bt_host_config.features.sniffMaxInterval;
            sniffcmd.sniff_min_interval = g_bt_host_config.features.sniffMinInterval;
            sniffcmd.sniff_attempt = g_bt_host_config.features.sniffAttempt;
            sniffcmd.sniff_timeout = g_bt_host_config.features.sniffTimeout;
            GAP_ExecuteCommandA(NULL, NULL, HCI_OPS_SNIFF_MODE, &sniffcmd, sizeof(struct HCI_Sniff_ModeStru), NULL);
         }
      }
      
      if (node->ft != NULL) {
         FsmDelTimer2(node->ft);
         node->ft = NULL;
      }
      node->ft = FsmAddTimerCx(app_fsm_inst, timeout, 1, App_CheckSniffTimer, node, "SNIFF", 1);
   }
   else if (hcimode == HCI_CURRENT_MODE_SNIFF) {
      if (g_bt_host_config.features.sniffEnable > 1) {
         timeout = g_bt_host_config.features.sniffSubrateInterval;
         
         if (node->state == APP_FSM_SNIFF_DATA_STATE_SNIFF) {
            node->state = APP_FSM_SNIFF_DATA_STATE_SUBRATE;
         }
      }
      else {
         timeout = g_bt_host_config.features.sniffSuspendTimeout;
         node->state = APP_FSM_SNIFF_DATA_STATE_SNIFF;
      }
      
      if (node->ft != NULL) {
         FsmDelTimer2(node->ft);
         node->ft = NULL;
      }
      node->ft = FsmAddTimerCx(app_fsm_inst, timeout, 1, App_CheckSniffTimer, node, "SNIFF", 1);
   }
   else{
      node->state = APP_FSM_SNIFF_DATA_STATE_LOSE;
   }
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
static void App_CheckSniffTimer(struct FsmInst *fi, UINT8 ev, void *arg)
{
    struct AppPowerModeStru *node = NULL;
    (void)fi;
    (void)ev;
    
    if (arg != NULL) {
       node = (struct AppPowerModeStru *)arg;
       node->ft = NULL;
       App_SniffHandler(node);
    }    
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_AddAclList(HANDLE tl_handle, struct GAP_ConnectionEventStru *acl)
{
  struct AppPowerModeStru *node = APP_ACL_LIST.head;
  
  node = APP_ACL_LIST.head;
  while (node != NULL) {
      if ( node->acl_hdl == acl->acl_hdl)
         return;
      node = LNEXT(node);
  }
  
  node = List_NodeNew(sizeof(struct AppPowerModeStru));

  memcpy(&(node->addr), &(acl->addr), sizeof(struct HCI_AddressStru));
  node->acl_hdl = acl->acl_hdl;
  node->state = APP_FSM_SNIFF_DATA_STATE_BUSY;
  node->ft = NULL;
  node->tl_handle = tl_handle;
  List_AddTail(&APP_ACL_LIST, node);

  App_SniffHandler(node);

  if (g_bt_host_config.features.linkLostRetryEnable)
  {
      App_DelLinkLostRetry();
  }
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_DeleteAclList(struct GAP_ConnectionEventStru *acl)
{
  struct AppPowerModeStru* node = APP_ACL_LIST.head;
  
  while ((node != NULL) && (node->acl_hdl != acl->acl_hdl)) {
     node = LNEXT(node);
  }
  
  if (node != NULL) {
     if (node->ft != NULL) {
       FsmDelTimer2(node->ft);
       node->ft = NULL;
     }
     List_RemoveAt(&APP_ACL_LIST, node);
     LFREE(node);
  }
  
    if (g_bt_host_config.features.linkLostRetryEnable)
    {
        if (acl->reason == HCI_STATUS_CONNECTION_TIMEOUT)
        {   
            memcpy(app_fsm_lost_retry_bd, acl->addr.bd, sizeof(app_fsm_lost_retry_bd));
            App_StartLinkLostRetry();
        }
        else
        {
            //App_LinkLostRetry(NULL, 0, NULL);
        }
    }
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
#if HAL_USE_AUDIO

#if defined(CONFIG_HFP) || defined(CONFIG_A2DP)
void App_AudioPlayCb(hs_audio_event_t enEvent)
{
	debug("App_AudioPlayCb event=%d, play=%d\r\n", enEvent, app_fsm_play_remote_close);
	if (enEvent == AUDIO_EVENT_STOPPED )
	{
		app_fsm_play_local_close = 1;
		hsc_A2dpDisableAudioStreamHandle();
	}
	else if (enEvent == AUDIO_EVENT_RESUME )
    {
        audioPlayAcquire();
        if (app_fsm_play_remote_close == 1)
        {
            app_fsm_play_remote_close = 0;
            app_fsm_play_local_close = 0;
        }
        else
        {
            if (audioPlayStart(&app_fsm_i2sCfg, app_fsm_audio_play_size, App_AudioPlayCb) !=0 )
            {
                app_fsm_play_local_close = 0;
                app_fsm_audio_start = 0;
                hsc_A2dpEnableAudioStreamHandle();
                audioPlayRelease();
                hs_printf("[BT]audio start no buffer!\r\n");
                return;
            }
            audioSetBTCfg();
            audioSetPlaySource(AUDIO_PLAY_RAM);

            if (app_fsm_i2sCfg.i2s_mode == I2S_PCMMODE_MONO && App_CFG_GetHfpStereoEnable())
            {
                audioInvertI2sOutput(TRACK_LL); /* mono */
            }
            app_fsm_play_local_close = 0;
            if (app_fsm_i2sCfg.i2s_mode == I2S_PCMMODE_STEREO) 
            {
                audioInvertI2sOutput(TRACK_LR); /* stereo */
                //audioPlySetAutoRepair(AUDIO_AUTOREPAIR_ENABLE);
                //hsc_AvrcpSetLocalVol();
                hs_audio_volRestore(AVOL_DEV_NOR);
                hsc_A2dpEnableAudioStreamHandle();
                hsc_A2DP_ClearThreshold();
            }
            else if (app_fsm_i2sCfg.i2s_mode == I2S_PCMMODE_MONO)
            {
                hsc_HfpSetLocalMicVol();
                hs_audio_volRestore(AVOL_DEV_HFP);
            }
            app_fsm_audio_start = 1;
            audioPlayUnmute();            
        }
        audioPlayRelease();

        if (app_fsm_audio_play_size == 0) // ios a2dp->tone->a2dp, stream not smooth
        {
           UINT8 *audio_buf;
           audioPlyGetDataBuffer(&audio_buf, 1, 1);
           audioPlySendDataDone(audio_buf, 1);
        }
    }
}
#endif /* defined(CONFIG_HFP) || defined(CONFIG_A2DP) */

#if defined(CONFIG_HFP)
void App_AudioRecordCb(hs_audio_event_t enEvent)
{
	debug("App_AudioRecordCb event=%d, play=%d\r\n", enEvent, app_fsm_record_remote_close);
	if (enEvent == AUDIO_EVENT_STOPPED )
	{
		app_fsm_record_local_close = 1;
		APP_AudioThreadSleep();
	}
	else if (enEvent == AUDIO_EVENT_RESUME )
	{
		if (app_fsm_record_remote_close == 1)
		{
			app_fsm_record_remote_close = 0;
		}
		else
		{
			// resume record
#if CH_KERNEL_MAJOR==2
			audioSetRecordSource(AUDIO_RECORD_MIC);
			audioRecordStart(&app_fsm_record_i2sCfg);
			audioRecRegisterEvent(App_AudioRecordCb);
#else
			audioRecordStart(&app_fsm_record_i2sCfg, App_AudioRecordCb);
            if(audioRecordStart(&app_fsm_record_i2sCfg, App_AudioRecordCb) != 0)
            {
               app_fsm_record_start = 0;
               hs_printf("audio record dma no buffer!\r\n");
               return;
            }
            app_fsm_record_start = 1;
			audioSetRecordSource(AUDIO_RECORD_MIC);
#endif
            audioInvertI2sInput(TRACK_RR);
            hsc_HfpSetLocalMicVol();
			APP_AudioThreadWakeup();
		}
		app_fsm_record_local_close = 0;
	}
}
#endif /* CONFIG_HFP */

#if defined(CONFIG_HFP) || defined(CONFIG_A2DP)
UINT8 App_AudioPlayLocalStop(void)
{
    if ((app_fsm_play_local_close == 1) || (app_fsm_audio_start == 0))
    {
        return 1;
    }
    return 0;
}
#endif

#if defined(CONFIG_HFP)
UINT8 App_AudioLocalTonePlay(void)
{
    return app_fsm_play_local_close;
}

UINT8 App_AudioRecordStop(void)
{
    if ((app_fsm_record_local_close == 1) || (app_fsm_record_start == 0))
        return 1;
    else
        return 0;
}
#endif /* CONFIG_HFP */

#if defined(CONFIG_HFP) || defined(CONFIG_A2DP)
void App_AudioStart(INT32 sample, INT32 mode, UINT32 size)
{
   uint16_t hfpState = BT_HFP_CALL_STATUS_STANDBY;
   #ifdef CONFIG_HFP
   hfpState = hsc_HFP_GetState();
   #endif
   
   if (app_fsm_play_local_close == 1)
   {
     app_fsm_audio_start = 1;
     app_fsm_i2sCfg.sample_rate  = (hs_i2s_sample_t)sample;
     app_fsm_i2sCfg.i2s_mode     = (hs_i2s_pcmmode_t)mode;
     app_fsm_i2sCfg.sample_width = I2S_BITWIDTH_16BIT;
     app_fsm_i2sCfg.ws_width     = I2S_BITWIDTH_32BIT;
     app_fsm_i2sCfg.work_mode    = I2S_WORKMODE_MASTER;
     if (mode == I2S_PCMMODE_MONO)
     {
         app_fsm_i2sCfg.frame_len    = BT_I2S_HFP_PLY_BLOCK_SIZE;
         app_fsm_i2sCfg.frame_num    = BT_I2S_HFP_PLY_BLOCK_NUM;
     }
     else
     {
         app_fsm_i2sCfg.frame_len    = BT_I2S_A2DP_PLY_BLOCK_SIZE;
         app_fsm_i2sCfg.frame_num    = BT_I2S_A2DP_PLY_BLOCK_NUM;
     }
     app_fsm_audio_play_size = size; 
     app_fsm_play_remote_close = 0;
     return;
   }

   if (app_fsm_audio_start == 1)
   {
	   if (hfpState > BT_HFP_CALL_STATUS_CONNECT && sample != I2S_SAMPLE_8K)
	   {
		   debug("App_AudioStart calling sample=%d\r\n", sample);
		   return;
	   }
	   
	   if( (hs_i2s_sample_t)sample != app_fsm_i2sCfg.sample_rate || (hs_i2s_pcmmode_t)mode != app_fsm_i2sCfg.i2s_mode)
	   {
		   audioPlayStop();
	   }
       else
       {
          return;
       }
   }
   //hs_printf("App_AudioStart sample=%d\r\n", sample);
   audioPlayAcquire();

   app_fsm_i2sCfg.sample_rate  = (hs_i2s_sample_t)sample;
   app_fsm_i2sCfg.i2s_mode     = (hs_i2s_pcmmode_t)mode;
   app_fsm_i2sCfg.sample_width = I2S_BITWIDTH_16BIT;
   app_fsm_i2sCfg.ws_width     = I2S_BITWIDTH_32BIT;
   app_fsm_i2sCfg.work_mode    = I2S_WORKMODE_MASTER;
   if (mode == I2S_PCMMODE_MONO)
   {
      app_fsm_i2sCfg.frame_len    = BT_I2S_HFP_PLY_BLOCK_SIZE;
      app_fsm_i2sCfg.frame_num    = BT_I2S_HFP_PLY_BLOCK_NUM;
   }
   else
   {
      app_fsm_i2sCfg.frame_len    = BT_I2S_A2DP_PLY_BLOCK_SIZE;
      app_fsm_i2sCfg.frame_num    = BT_I2S_A2DP_PLY_BLOCK_NUM;
   }

   app_fsm_audio_play_size = size;   

   if(audioPlayStart(&app_fsm_i2sCfg, app_fsm_audio_play_size, App_AudioPlayCb) != 0)
   {
       app_fsm_play_local_close = 0;
       app_fsm_audio_start = 0;
       audioPlayRelease();
       hs_printf("[BT]audio start no buffer!\r\n");
       return;
   }
   audioSetBTCfg();
   audioSetPlaySource(AUDIO_PLAY_RAM);
   audioPlayUnmute();
   if (mode == I2S_PCMMODE_STEREO) {
      audioInvertI2sOutput(TRACK_LR); /* stereo */
      //audioPlySetAutoRepair(AUDIO_AUTOREPAIR_ENABLE);
      //hsc_AvrcpSetLocalVol();
      hs_audio_volRestore(AVOL_DEV_NOR);
      hsc_A2dpEnableAudioStreamHandle();
      hsc_A2DP_ClearThreshold();
   }
   else if (mode == I2S_PCMMODE_MONO)
   {
       hsc_HfpSetLocalMicVol();
       hs_audio_volRestore(AVOL_DEV_HFP);
   }
            
   if (mode == I2S_PCMMODE_MONO && App_CFG_GetHfpStereoEnable()) {
      audioInvertI2sOutput(TRACK_LL); /* mono */
   }
   
   app_fsm_audio_start = 1;
   app_fsm_play_remote_close = 0;
   app_fsm_play_local_close = 0;
   audioPlayRelease();
   if (app_fsm_audio_play_size == 0) // ios a2dp->tone->a2dp, stream not smooth
   {
      UINT8 *audio_buf;
      audioPlyGetDataBuffer(&audio_buf, 1, 1);
      audioPlySendDataDone(audio_buf, 1);
   }
   debug("App_AudioStart end\r\n");
}
#endif /* defined(CONFIG_HFP) || defined(CONFIG_A2DP) */

#if defined(CONFIG_HFP)
void App_AudioRecordStart(void)
{
   if (app_fsm_record_start == 1 ) {
	   return;
   }
   debug("App_AudioRecordStart\r\n");

   app_fsm_record_start = 1;
   app_fsm_record_i2sCfg.sample_rate  = I2S_SAMPLE_8K;
   app_fsm_record_i2sCfg.i2s_mode     = I2S_PCMMODE_MONO;
   app_fsm_record_i2sCfg.sample_width = I2S_BITWIDTH_16BIT;
   app_fsm_record_i2sCfg.ws_width     = I2S_BITWIDTH_32BIT;
   app_fsm_record_i2sCfg.work_mode    = I2S_WORKMODE_MASTER;
   app_fsm_record_i2sCfg.frame_len    = BT_I2S_REC_BLOCK_SIZE;
   app_fsm_record_i2sCfg.frame_num    = BT_I2S_REC_BLOCK_NUM;

#if CH_KERNEL_MAJOR==2
   audioRecordStart(&app_fsm_record_i2sCfg);   
   audioRecRegisterEvent(App_AudioRecordCb);
#else
   if(audioRecordStart(&app_fsm_record_i2sCfg, App_AudioRecordCb) != 0)
   {
      app_fsm_record_start = 0;
      hs_printf("audio record dma no buffer!\r\n");
      return;
   }
#endif
   #if 1
   audioSetRecordSource(AUDIO_RECORD_MIC);
   audioInvertI2sInput(TRACK_RR);
   #else
   audioSetRecordSource(AUDIO_RECORD_LINEIN);
   audioInvertI2sInput(TRACK_LR);
   #endif

   app_fsm_record_remote_close = 0;
   app_fsm_record_local_close = 0;
   
   APP_AudioThreadWakeup();
}
#endif /* CONFIG_HFP */

#if defined(CONFIG_HFP) || defined(CONFIG_A2DP)
void App_AudioStop(void)
{
   //hs_printf("App_AudioStop %d, local_close=%d\r\n", app_fsm_audio_start, app_fsm_play_local_close);
    if (app_fsm_audio_start != 0)
    {
        app_fsm_audio_start = 0;
        if (app_fsm_play_local_close == 1)
        {
           app_fsm_play_remote_close = 1;
        }
        else
        {
           audioPlayStop();
        }
    }
   #ifdef CONFIG_HFP
   if (app_fsm_record_start == 1)
   {
       app_fsm_record_start = 0;
       if (app_fsm_record_local_close == 1)
       {
          app_fsm_record_remote_close = 1;
       }
       else
       {
          audioRecordStop();
          APP_AudioThreadSleep();
       }
   }
   #endif /* CONFIG_HFP */
   //hs_cfg_flush(FLUSH_TYPE_ALL);
   //audioStop();
}
#endif /* defined(CONFIG_HFP) || defined(CONFIG_A2DP) */

#endif /* HAL_USE_AUDIO */

void App_FsmReconnect(void)
{
   UINT8 index = 0;
   struct AppDevInst *inst = APP_DEVICE_LIST.head;
   struct AppConnInst *inst_conn = APP_CONNECTION_LIST.head;

   //hs_printf("App_FsmReconnect count=%d, index=%d,%d\r\n", List_Count( &APP_DEVICE_LIST), app_fsm_auto_connect, g_bt_host_config.features.linkLostRetryCount);
   if (inst == NULL 
       || ((app_fsm_auto_connect+1) > (UINT8)(List_Count( &APP_DEVICE_LIST)))
       || ((app_fsm_auto_connect+1) > g_bt_host_config.features.linkLostRetryCount)
    )
   {
      hs_printf("reconnect count over!\r\n");
      App_DelAutoConnectTimer();
      App_SetBtState(APP_BT_STATE_PAIRMODE);
      App_GAP_SetVisualModes(3);
      return;
   }
   
   while (inst != NULL) {
      //hs_printf("bd:%02x%02x%02x%02x%02x%02x\r\n", inst->bd[5], inst->bd[4], inst->bd[3], inst->bd[2], inst->bd[1], inst->bd[0]);
      if (index < app_fsm_auto_connect)
      {
         index++;
         inst = LNEXT(inst);
         continue;
      }
      
      app_fsm_auto_connect++;
      index++;
      //hs_printf("1)auto=%d\r\n", app_fsm_auto_connect);
      inst_conn = APP_CONNECTION_LIST.head;
      while(inst_conn != NULL) {
              if ((inst_conn->state == APP_CONNECT_STATE_CONNECTED) && (memcmp(inst->bd, inst_conn->bd, BD_ADDR_LEN)==0))
		      break;
	      inst_conn = LNEXT(inst_conn);
      }
 
      if (inst_conn == NULL) {
         /* HS6600A4: stop scan before start page */
         App_GAP_SetVisualModes(0);
         App_GAP_ReConnect(inst->bd); // reconnect the first bond
         return;
      }
      inst = LNEXT(inst);
   }
   App_DelAutoConnectTimer();
}

void App_FsmA2dpReconnect(void)
{
#ifdef CONFIG_A2DP
   struct AppDevInst *inst = APP_DEVICE_LIST.head;
   
   if (inst != NULL) {
      debug("bd:%02x%02x%02x%02x%02x%02x\r\n", inst->bd[5], inst->bd[4], inst->bd[3], inst->bd[2], inst->bd[1], inst->bd[0]);
      App_A2DP_Connect(inst->bd);
   }
#endif
}

void App_FsmHfpReconnect(void)
{
#ifdef CONFIG_HFP
   struct AppDevInst *inst = APP_DEVICE_LIST.head;
   
   if (inst != NULL) {
      debug("bd:%02x%02x%02x%02x%02x%02x\r\n", inst->bd[5], inst->bd[4], inst->bd[3], inst->bd[2], inst->bd[1], inst->bd[0]);
      App_HFP_ConnectAG(inst->bd);
   }
#endif
}

void App_FsmAvrcpReconnect(void)
{
#ifdef CONFIG_AVRCP
   struct AppDevInst *inst = APP_DEVICE_LIST.head;
   
   if (inst != NULL) {
      debug("bd:%02x%02x%02x%02x%02x%02x\r\n", inst->bd[5], inst->bd[4], inst->bd[3], inst->bd[2], inst->bd[1], inst->bd[0]);
      App_AVRCP_Connect(inst->bd);
   }
#endif
}

void App_FsmHidReconnect(void)
{
#ifdef CONFIG_HID
   struct AppDevInst *inst = APP_DEVICE_LIST.head;
   
   if (inst != NULL) {
      debug("bd:%02x%02x%02x%02x%02x%02x\r\n", inst->bd[5], inst->bd[4], inst->bd[3], inst->bd[2], inst->bd[1], inst->bd[0]);
      App_HID_Connect(inst->bd);
   }
#endif
}

void App_FsmSppReconnect(void)
{
#ifdef CONFIG_SPP
   struct AppDevInst *inst = APP_DEVICE_LIST.head;
   
   if (inst != NULL) {
      debug("bd:%02x%02x%02x%02x%02x%02x\r\n", inst->bd[5], inst->bd[4], inst->bd[3], inst->bd[2], inst->bd[1], inst->bd[0]);
      App_SPP_Connect(inst->bd);
   }
#endif
}

static void App_StopAutoConnectTimer(struct FsmInst *fi, UINT8 ev, void *arg)
{
   (void)fi;
   (void)ev;
   (void)arg;

   //App_GAP_SetVisualModes(3);
   //app_fsm_auto_connect = 0;
   //app_fsm_power_on_reconnect = NULL;
   //hs_printf("stop atuo\r\n");
}

static void App_DelAutoConnectTimer(void)
{
    app_fsm_auto_connect = 0;
    if (app_fsm_power_on_reconnect != NULL)
    {
        FsmDelTimer2(app_fsm_power_on_reconnect);
        app_fsm_power_on_reconnect = NULL;
    }
}

static void App_StartAutoConnectTimer(struct FsmInst *fi, UINT8 ev, void *arg)
{
   (void)fi;
   (void)ev;
   (void)arg;

   //App_GAP_SetVisualModes(3);
   App_FsmReconnect();
   //app_fsm_auto_connect = 1;
   if (g_bt_host_config.features.powerOnAutoConnectProtect) {
       app_fsm_power_on_reconnect = FsmAddTimerCx(app_fsm_inst, 
                                                 g_bt_host_config.features.powerOnAutoConnectTimer*1000, 
                                                 5, App_StopAutoConnectTimer, NULL, "AutoConnect", 5);
   }
}

void App_FsmAutoConnect(void)
{
    if (g_bt_host_config.features.powerOnAutoConnect) {
        debug("%4d - autoconnect start\r\n", GetCurrTime());
        app_fsm_power_on_reconnect = FsmAddTimerCx(app_fsm_inst, 
                                                   g_bt_host_config.features.powerOnStartConnectTimer, 
                                                   5, App_StartAutoConnectTimer, NULL, "AutoConnect", 5);
    }
}

static void App_DelLinkLostRetry(void)
{
   if (app_fsm_link_lost_retry != NULL)
   {
      FsmDelTimer2(app_fsm_link_lost_retry);
      app_fsm_link_lost_retry = NULL;
   }
   app_fsm_link_lost_retry_count = 0;
}

static void App_FsmlinkLostReconnect(void)
{
   struct AppConnInst *inst_conn = APP_CONNECTION_LIST.head;

   while(inst_conn != NULL) {
      if ((inst_conn->state == APP_CONNECT_STATE_CONNECTED) 
          && (memcmp(app_fsm_lost_retry_bd, inst_conn->bd, BD_ADDR_LEN)==0))
        {
          App_DelLinkLostRetry();
          return;
        }
      inst_conn = LNEXT(inst_conn);
   }
   
   App_GAP_SetVisualModes(0);
   App_GAP_ReConnect(app_fsm_lost_retry_bd);
}

#if 0
static void App_StopLinkLostRetry(struct FsmInst *fi, UINT8 ev, void *arg)
{
	(void)fi;
    (void)ev;
    (void)arg;
    app_fsm_link_lost_retry = NULL;
    App_DelLinkLostRetry();
}

static void App_LinkLostRetry(struct FsmInst *fi, UINT8 ev, void *arg)
{
   UINT32 tms = 0;

   (void)fi;
   (void)ev;
   (void)arg;

   //s_printf("App_LinkLostRetry timer=%x, count=%d\r\n", app_fsm_link_lost_retry, app_fsm_link_lost_retry_count);
   
   if (app_fsm_link_lost_retry == NULL)
   {
      return;
   }
   
   if (app_fsm_link_lost_retry_count == 0)
   {
      app_fsm_link_lost_retry = NULL;
      tms = g_bt_host_config.features.linkLostRetryTimer*1000;
      app_fsm_link_lost_retry = FsmAddTimerCx(app_fsm_inst, tms, 4, App_StopLinkLostRetry, NULL, "LINKLOST", 4);
   }

   //debug("App_StopLinkLostRetryTimer count=%d/%d\r\n", app_fsm_link_lost_retry_count, g_bt_host_config.features.linkLostRetryCount);

   if (app_fsm_link_lost_retry_count < g_bt_host_config.features.linkLostRetryCount)  
   {
      app_fsm_link_lost_retry_count++;
      App_FsmlinkLostReconnect();
   }
   else
   {
      App_DelLinkLostRetry();
   } 
}
#else
static void App_LinkLostRetry(struct FsmInst *fi, UINT8 ev, void *arg)
{
   UINT32 tms = 0;

   (void)fi;
   (void)ev;
   (void)arg;

   //s_printf("App_LinkLostRetry timer=%x, count=%d\r\n", app_fsm_link_lost_retry, app_fsm_link_lost_retry_count);
   
   if (app_fsm_link_lost_retry == NULL)
   {
      return;
   }
   
   if (app_fsm_link_lost_retry_count == 0)
   {
      tms = 60*1000;
   }
   else
   {
      tms = 3*60*1000;
   }
   app_fsm_link_lost_retry = FsmAddTimerCx(app_fsm_inst, tms, 4, App_LinkLostRetry, NULL, "LINKLOST", 4);

   hs_printf("App_LinkLostRetry count=%d\r\n", app_fsm_link_lost_retry_count);

   if (app_fsm_link_lost_retry_count < 3)  
   {
      app_fsm_link_lost_retry_count++;
      App_FsmlinkLostReconnect();
   }
   else
   {
      App_DelLinkLostRetry();
   } 
}
#endif

static void App_StartLinkLostRetry(void)
{
   if (app_fsm_link_lost_retry == NULL) {
      //app_fsm_link_lost_retry = FsmAddTimerCx(app_fsm_inst, 1500, 4, App_LinkLostRetry, NULL, "LINKLOST", 4);
      app_fsm_link_lost_retry = FsmAddTimerCx(app_fsm_inst, 30*1000, 4, App_LinkLostRetry, NULL, "LINKLOST", 4);
   }
}

static void App_FsmInitPairInfo(void)
{
    struct AppDevInst info;
    struct AppDevInst *inst;
    UINT8 index = 0;
    UINT8 ret = 1;

    if (app_fsm_bt_mode == BT_HOST_VAR_MODE_AUDIO)
    {
        hsc_HandleBackUpLinkKey();
        while(ret == 1 && index<BONDED_DEVICE_MAX_COUNT)
        {
            ret = App_CFG_GetPairInfo(index++, &info);
            if(ret)
            {
                inst = List_NodeNew(sizeof(struct AppDevInst));
                memcpy(inst->bd, info.bd, BD_ADDR_LEN);
                memcpy(inst->link_key, info.link_key, LINKKEYLENGTH);
                inst->key_type = info.key_type;
                List_AddTail(&APP_DEVICE_LIST, inst);
            }
        }
    }
    else
    {
        struct AppBtHidInfoStru info;
        UINT8 i;
        UINT16 asum = 0;
        memset(&info, 0, sizeof(struct AppBtHidInfoStru));
        hsc_HandleBackUpHIDLinkKey();
        hsc_CFG_GetHidInfo(&info);
        for (i = 0; i < BD_ADDR_LEN; i++)
        {
            asum += info.remote.bd_addr[i];
        }
        if ((asum != 0x00) && (asum != (0xFF*6)))
        {
            inst = List_NodeNew(sizeof(struct AppDevInst));
            memcpy(inst->bd, info.remote.bd_addr, BD_ADDR_LEN);
            memcpy(inst->link_key, info.remote.link_key, LINKKEYLENGTH);
            inst->key_type = info.remote.link_key_type;
            List_AddTail(&APP_DEVICE_LIST, inst);
        }
    }
}

static void App_FsmVolChange(UINT16 u16Msg, void *parg)
{
  (void)u16Msg;
  UINT32 vol = (UINT32)parg;
  if ((vol>>8) & 0x01) //a2dp change vol
  {
    hsc_AvrcpSetSpkVol(vol&0xFF);
  }
  else // hfp change vol
  {
    hsc_HfpSetSpkVol(vol&0xFF);
  }
}

void App_FsmInit(UINT8 mode)
{
	struct AppFsmUserData *userdata = NEW(sizeof(struct AppFsmUserData));
	memset(userdata, 0, sizeof(struct AppFsmUserData));
	userdata->handle_base = 1; /* 0 is defined as INVALID */
	app_fsm_inst = FsmInstNew(userdata, "APPFI");
    app_fsm_bt_mode = mode;

	App_FsmInitPairInfo();

    /* avoid no sound when run exit/init again */
    app_fsm_audio_start  = app_fsm_play_remote_close   = app_fsm_play_local_close = 0;
    app_fsm_record_start = app_fsm_record_remote_close = app_fsm_record_local_close = 0;
    memset((void *)&app_fsm_i2sCfg, 0x00, sizeof(app_fsm_i2sCfg));
    memset((void *)&app_fsm_record_i2sCfg, 0x00, sizeof(app_fsm_record_i2sCfg));

    hs_cfg_sysListenMsg(HS_CFG_EVENT_BT_REMOTE_VOL,  App_FsmVolChange);
}

void App_FsmWaitReconnect(void)
{
    if (app_fsm_link_lost_retry != NULL)
    {
        App_DelLinkLostRetry();
    }
    if (app_fsm_power_on_reconnect != NULL)
    {
       App_DelAutoConnectTimer();
    }
}
void App_FsmDone(void)
{
	struct AppConnInst *inst = APP_CONNECTION_LIST.head;
	struct AppPowerModeStru *node = APP_ACL_LIST.head;
    
    hs_cfg_sysCancelListenMsg(HS_CFG_EVENT_BT_REMOTE_VOL, NULL);
    
    // stop all timer;
    #ifdef CONFIG_AVRCP
    if (app_fsm_avrcp_connect_timer != NULL)
    {
        FsmDelTimer2(app_fsm_avrcp_connect_timer);
        app_fsm_avrcp_connect_timer = NULL;
    }
    #endif
    #ifdef CONFIG_HFP
    if (app_fsm_hfp_sco_timer != NULL)
    {
        FsmDelTimer2(app_fsm_hfp_sco_timer);
        app_fsm_hfp_sco_timer = NULL;
    }
    #endif
    App_DelAutoConnectTimer();
    App_DelPairTimer((struct FsmInst*)NULL);
    App_DelLinkLostRetry();
    
	/* exit host stack w/o App_Avrcp_FsmDisconnectComplete() */
	while (inst != NULL) {
		if ((inst->service_class == CLS_AVRCP_CT) && (inst->profile_inst_handle)) {
			FREE(inst->profile_inst_handle);
			inst->profile_inst_handle = NULL;
		}
		inst = LNEXT(inst);
	}
  
	while ((node != NULL)) {
		if (node->ft != NULL) {
		    FsmDelTimer2(node->ft);
		    node->ft = NULL;
		}
	}
    
	List_RemoveAll(&APP_DEVICE_LIST);
	List_RemoveAll(&APP_CONNECTION_LIST);
	List_RemoveAll(&APP_ACL_LIST);
	FREE(app_fsm_inst->user_data);
	FsmInstFree(app_fsm_inst);
	app_fsm_inst = NULL;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_UI_FsmEvent(void *func, void *arg)
{
	FsmEvent_ExternalCx(app_fsm_inst, 1, (FsmFunc *)func, arg);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_SCH_FsmEvent(void *func, void *arg)
{
	FsmEventCx(app_fsm_inst, 1, (FsmFunc *)func, arg);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
struct AppDevInst *App_FindBondedDevice(UINT8 *bd)
{
	struct AppDevInst *inst = APP_DEVICE_LIST.head;
	
	while ((inst != NULL) && memcmp(bd, inst->bd, BD_ADDR_LEN)) {
		inst = LNEXT(inst);
	}
	return inst;
}

void App_StoreBondedDevice(void)
{
   struct AppDevInst *inst = APP_DEVICE_LIST.head;
   hs_pair_entry_t devices[BONDED_DEVICE_MAX_COUNT];
   UINT8 index = 0;
   memset(devices, 0, sizeof(devices));
   
   while (inst != NULL && index<BONDED_DEVICE_MAX_COUNT) {
      memcpy(devices[index].bd_addr, inst->bd, BD_ADDR_LEN);
      memcpy(devices[index].link_key, inst->link_key, LINK_KEY_LEN);
      devices[index].link_key_type = inst->key_type;
      index++;
      inst = LNEXT(inst);
    }
    App_CFG_UpdatePairInfo((void*)devices);
    if (app_fsm_bt_mode == BT_HOST_VAR_MODE_HID)
    {
        hsc_StoreBackUpHIDLinkKey((uint8_t*)devices);
    }
    else
    {
        hsc_StoreBackUpLinkKey((uint8_t*)devices);
    }
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_AddBondedDevice(UINT8 *bd, UINT8 *link_key, UINT8 key_type)
{
	struct AppDevInst *inst = List_NodeNew(sizeof(struct AppDevInst));

	memcpy(inst->bd, bd, BD_ADDR_LEN);
	memcpy(inst->link_key, link_key, LINKKEYLENGTH);
	inst->key_type = key_type;
	//List_AddTail(&APP_DEVICE_LIST, inst);
	List_AddHead(&APP_DEVICE_LIST, inst);

	//App_CFG_AddPairInfo(inst);
    App_SetBtState(APP_BT_STATE_PAIRED);
    //App_GAP_SetVisualModes(2);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_DeleteBondedDevice(struct AppDevInst *inst)
{
	UINT8 *bd;
	struct HCI_Delete_Stored_Link_KeyStru del_key_in;

	/* Delete hci stored key*/
	bd = NEW(BD_ADDR_LEN);
	memcpy(bd, inst->bd, BD_ADDR_LEN);
        HCI_Security_LinkKey_Remove(bd);
	/* Delete host controller stored link key*/
	memcpy(del_key_in.bd, inst->bd, BD_ADDR_LEN);
	del_key_in.all_flag = 0x00;    /*Delete link key for specified BD_ADDR*/
	GAP_ExecuteCommandA(NULL, NULL, HCI_OPS_DELETE_STORED_LINK_KEY, &del_key_in, sizeof(struct HCI_Delete_Stored_Link_KeyStru), NULL);
    //FREE(bd);
	List_RemoveAt(&APP_DEVICE_LIST, inst);
	LFREE(inst);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  when device connected, must change the pos in the device list.
  next poweron, must first reconnect this device.
------------------------------------------------------------------------*/
void hsc_MoveBondeDevice(UINT8 *bd)
{
    struct AppDevInst *inst = APP_DEVICE_LIST.head;

    // do not change the head device.
    if (inst == NULL || memcmp(bd, inst->bd, BD_ADDR_LEN) == 0)
    {
        return;
    }

    // find the device in the device list
    while ((inst != NULL) && memcmp(bd, inst->bd, BD_ADDR_LEN) !=0) {
        inst = LNEXT(inst);
    }
    
    if (inst != NULL)
    {
        // remove and add head the device list
        List_RemoveAt(&APP_DEVICE_LIST, inst);
        List_AddHead(&APP_DEVICE_LIST, inst);

        // store the device to flash
        App_StoreBondedDevice();
    }
    else
    {
        hs_printf("no device! \r\n");
    }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
struct AppConnInst *App_FindConnectionInstByBD(UINT16 service_class, UINT8 *bd, UINT8 role)
{
	struct AppConnInst *inst = APP_CONNECTION_LIST.head;
	
	while ((inst != NULL) && (inst->role & role) &&
		   ((inst->service_class != service_class) || (memcmp(bd, inst->bd, BD_ADDR_LEN)))) {
		inst = LNEXT(inst);
	}
	return inst;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
struct AppConnInst *App_FindConnectionInstByUpperHandle(UINT16 connection_handle)
{
	struct AppConnInst *inst = APP_CONNECTION_LIST.head;
	
	while ((inst != NULL) && (inst->connection_handle != connection_handle)) {
		inst = LNEXT(inst);
	}
	return inst;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
struct AppConnInst *App_FindConnectionInstByLowerHandle(void *profile_inst_handle)
{
	struct AppConnInst *inst = APP_CONNECTION_LIST.head;
	
	while ((inst != NULL) && (inst->profile_inst_handle != profile_inst_handle)) {
		inst = LNEXT(inst);
	}
	return inst;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
struct AppConnInst *App_FindNextConnectionInst(struct AppConnInst *connection)
{
	if (connection == NULL) {
		return APP_CONNECTION_LIST.head;
	} else {
		return LNEXT(connection);
	}
}

struct AppConnInst *App_FindNextConnByBD(struct AppConnInst *conn)
{
	struct AppConnInst *inst = APP_CONNECTION_LIST.head;
	
	while ((inst != NULL) && ((inst == conn) || (memcmp(conn->bd, inst->bd, BD_ADDR_LEN)))) 
	{
		inst = LNEXT(inst);
	}
	return inst;
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
struct AppConnInst *App_AddConnectionInst(UINT16 service_class, UINT8 *bd, UINT8 role)
{
  struct AppConnInst *inst = List_NodeNew(sizeof(struct AppConnInst));
  inst->service_class = service_class;
  memcpy(inst->bd, bd, BD_ADDR_LEN);
  inst->role = role;
  inst->connection_handle = APP_HANDLE_BASE;
  if (APP_HANDLE_BASE < 0xFFFF) {
    APP_HANDLE_BASE++;
  } else {
    APP_HANDLE_BASE = 1;
  }
  List_AddTail(&APP_CONNECTION_LIST, inst);
  return inst;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_DeleteConnectionInst(struct AppConnInst *inst)
{
        //UINT32 count = 0;
	UINT8 hfp_free_link_num = 0, a2dp_free_link_num = 0;
	struct AppConnInst *nextinst;
	UINT8 state;

	if (List_NodeExist(&APP_CONNECTION_LIST, inst) == NULL)
	{
		debug("FSM:del conn, inst not exist\r\n");
		return;
	}
	nextinst = App_FindNextConnByBD(inst);
	state = inst->state;

	List_RemoveAt(&APP_CONNECTION_LIST, inst);
	LFREE(inst);

    if (app_fsm_bt_state == APP_BT_STATE_NULL)
    {
       return;
    }
        //count = List_Count(&APP_CONNECTION_LIST);
	//hs_printf("FSM:del conn, state1=%d, state2=%d\r\n", app_fsm_bt_state, state);

	if (nextinst == NULL && state == APP_CONNECT_STATE_CONNECTED)
	{      
      #if HS_USE_CONF
      if (app_fsm_bt_mode == BT_HOST_VAR_MODE_HID)
      {
          // earse linkkey in the last flash
          hsc_HandleBackUpHIDLinkKey();
          hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_HID_DISCONNECTED);
      }
      else
      {
          // earse linkkey in the last flash
          hsc_HandleBackUpLinkKey();
          hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_DISCONNECTED);     //hotplug-like event
      }
      App_GAP_SetVisualModes(3);
	    // link lost alert
	    if (g_bt_host_config.features.linkLostAlertEnable)
	    {
	        UINT32 tms;
	        for(tms = 0; tms < g_bt_host_config.features.linkLostAlertStopTimer; tms++) 
	        {
	            hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_LOST_LINK_ALERT);
	        }
	    }
      #endif
	}
#ifdef CONFIG_HID
   if (app_fsm_bt_mode == BT_HOST_VAR_MODE_HID)
   {
       if ( g_bt_host_config.features.linkLostIntoPairEnable & 0x01)
       {
            App_SetBtState(APP_BT_STATE_PAIRMODE);
            App_GAP_SetVisualModes(3);
       }
       else
       {
           App_SetBtState(APP_BT_STATE_READY);
       }
   }
   else
   {
#endif
	#ifdef CONFIG_HFP
    hfp_free_link_num = hsc_HfpGetFreeLinkCount();
    #endif
    #ifdef CONFIG_A2DP
    a2dp_free_link_num = hsc_A2dpGetFreeLinkCount();
    #endif

    if ((hfp_free_link_num!=0) || (a2dp_free_link_num!=0)) 
    {
        if ( g_bt_host_config.features.linkLostIntoPairEnable & 0x01) 
        {
            if ( ( (g_bt_host_config.profiles.hfp >0 && hfp_free_link_num!=0) || 
                    g_bt_host_config.profiles.hfp == 0 )
              && ( (g_bt_host_config.profiles.a2dp >0 && a2dp_free_link_num!=0) || 
                     g_bt_host_config.profiles.a2dp == 0 )
       )
            //if (hfp_free_link_num != 0 && a2dp_free_link_num != 0)
            {
                App_SetBtState(APP_BT_STATE_PAIRMODE);
            }
            else
            {
                App_GAP_SetVisualModes(2);
            }
        }
        else 
        {
           App_SetBtState(APP_BT_STATE_READY);
        }
	}
#ifdef CONFIG_HID
   }
#endif

    /* HS6600A4: retry connect_back until timeout; don't do two retries at a time */
    //hs_printf("FSM:del conn, nextinst=%x, autoconnect=%d, hfp=%d, a2dp=%d\r\n", nextinst, app_fsm_auto_connect, hfp_free_link_num, a2dp_free_link_num);
	if (nextinst == NULL && app_fsm_auto_connect && ((hfp_free_link_num!=0) || (a2dp_free_link_num!=0)))
	{
		App_FsmReconnect();
	}
}

#ifdef CONFIG_AVRCP
static void App_StopAvrcpConnectTimer(struct FsmInst *fi, UINT8 ev, void *arg)
{
  (void)fi;
  (void)ev;
    struct AppConnInst *inst;
    struct AppConnInst *connection;

    app_fsm_avrcp_connect_timer = NULL;

    if (arg != NULL) {
       inst = (struct AppConnInst *)arg;
       connection = App_FindConnectionInstByBD(CLS_AVRCP_CT, inst->bd, APP_CONNECT_IGNORE_ROLE);
       if (connection == NULL) {
           App_AVRCP_Connect(inst->bd);
       }
    }
      
}
#endif
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
UINT16 App_ConvertServToProfile(UINT16 service_class)
{
   UINT16 ret = 0; 
   switch(service_class) {
      case CLS_HANDSFREE_AG:
        ret = BT_CLASSIC_PROFILE_HFP;
        break;
      case CLS_AUDIO_SOURCE:
        ret = BT_CLASSIC_PROFILE_A2DP;
        break;
      case CLS_HID:
        ret = BT_CLASSIC_PROFILE_HID;
        break;
      case CLS_SERIAL_PORT:
        ret = BT_CLASSIC_PROFILE_SPP;
        break;
      case CLS_AVRCP_CT:
        ret = BT_CLASSIC_PROFILE_AVRCP;
        break;  
      default:
        break;
   }
   return ret;
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
#if defined(CONFIG_PATCH_HUNTERSUN)
extern UINT8 g_test_freq_drift_enable;
#endif
void hsc_UpdateConnectionInst(struct AppConnInst *inst, UINT8 state)
{
   //struct AppConnInst *insttemp = APP_CONNECTION_LIST.head;
   UINT8 hfp_free_link_num = 0, a2dp_free_link_num = 0;
   UINT16 curProfile = 0; 
   UINT16 connectedProfile = 0;
   UINT16 connectedProfileOk = 0;

   if (inst == NULL)
      return;

   inst->state = state;
#ifdef CONFIG_HID
   if (app_fsm_bt_mode == BT_HOST_VAR_MODE_HID && state == APP_CONNECT_STATE_CONNECTED)
   {
       #if HS_USE_CONF
       hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_HID_CONNECTED); //bt status indicator
       #endif
       App_SetBtState(APP_BT_STATE_CONNECTED);
       //hs_cfg_flush(FLUSH_TYPE_ALL);
       return;
   }
#endif
   if (state == APP_CONNECT_STATE_CONNECTED && App_GetBtState() < APP_BT_STATE_CONNECTED) {

      curProfile = App_ConvertServToProfile(inst->service_class);
#if 0
      while (insttemp != NULL) {
         if (insttemp->state == APP_CONNECT_STATE_CONNECTED && (memcmp(insttemp->bd, inst->bd, BD_ADDR_LEN) == 0)) {
            connectedProfile |= App_ConvertServToProfile(insttemp->service_class);
         }
          insttemp = LNEXT(insttemp);
      }
      
      if ((curProfile & g_bt_host_config.attrs.connectAlert) && (g_bt_host_config.attrs.connectAlert & connectedProfile)== g_bt_host_config.attrs.connectAlert) {
#else
      if ( curProfile & g_bt_host_config.attrs.connectAlert) { 
#endif
        // change device in the device list
        hsc_MoveBondeDevice(inst->bd);
        if (g_test_freq_drift_enable == 0)
        {
            #if HS_USE_CONF
            // send the connected event for tone
            hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_CONNECTED); //bt status indicator
            #endif
        }
        else
        {
           g_test_freq_drift_enable = 0;
        }
         App_SetBtState(APP_BT_STATE_CONNECTED);
         //App_DelLinkLostRetry();
      }
   }

   if (state == APP_CONNECT_STATE_CONNECTED && App_GetBtState() >= APP_BT_STATE_CONNECTED) 
   {
#ifdef CONFIG_HFP
      hfp_free_link_num = hsc_HfpGetFreeLinkCount();
#endif
#ifdef CONFIG_A2DP
      a2dp_free_link_num = hsc_A2dpGetFreeLinkCount();
#endif
      debug("hfp free count=%d, a2dp free count = %d\r\n", hfp_free_link_num, a2dp_free_link_num);
      if ((hfp_free_link_num==0) && (a2dp_free_link_num==0)) {
          debug("%4d - connection completed\r\n", GetCurrTime());
          App_GAP_SetVisualModes(0);
          App_DelLinkLostRetry();
      }      
   }
#ifdef CONFIG_AVRCP
   // start timer for avrcp connect
   //hs_printf("s=%x,%x\r\n", inst->service_class, app_fsm_avrcp_connect_timer);
   if (inst->service_class == CLS_AUDIO_SOURCE && app_fsm_avrcp_connect_timer == NULL) {
       app_fsm_avrcp_connect_timer = FsmAddTimerCx(app_fsm_inst, 2000, 8, App_StopAvrcpConnectTimer, inst, "AVPRCPCONN", 8);
   }
#endif 
#ifdef CONFIG_HFP
   connectedProfileOk |= BT_CLASSIC_PROFILE_HFP;
#endif
#ifdef CONFIG_A2DP
   connectedProfileOk |= BT_CLASSIC_PROFILE_A2DP;
#endif
#ifdef CONFIG_AVRCP
   connectedProfileOk |= BT_CLASSIC_PROFILE_AVRCP;
#endif
   if ((connectedProfile == connectedProfileOk) && app_fsm_auto_connect) {
   	  if ((hfp_free_link_num!=0) || (a2dp_free_link_num!=0))
   	  {
         App_FsmReconnect();
   	  }
      else
      {
         App_DelAutoConnectTimer();
      }
      
   }
}

static void App_DelPairTimer(struct FsmInst *fi)
{
   (void)fi;
 
   if (app_fsm_pair_timer != NULL)
   {
      FsmDelTimer2(app_fsm_pair_timer);
      app_fsm_pair_timer = NULL;
   }
}

static void App_StopPairTimer(struct FsmInst *fi, UINT8 ev, void *arg)
{
   (void)fi;
   (void)ev;
   (void)arg;
   
   app_fsm_pair_timer = NULL;

   App_GAP_SetVisualModes(0);
   App_SetBtState(APP_BT_STATE_READY);
}

static void App_StartPairTimer(void)
{
   UINT32 tms;

   tms = g_bt_host_config.features.pairTimeOut*1000;
   if (app_fsm_pair_timer == NULL && tms>0) {
      app_fsm_pair_timer = FsmAddTimerCx(app_fsm_inst, tms, 3, App_StopPairTimer, NULL, "PAIRT", 3);
   }
}

void App_SetBtState(UINT16 state)
{
   if (app_fsm_bt_state != state)
   {
      if (state == APP_BT_STATE_PAIRMODE)
      {
         App_StartPairTimer();
      }
      else if ( state == APP_BT_STATE_PAIRED || state == APP_BT_STATE_CONNECTED)
      {
         App_DelPairTimer(app_fsm_inst);
      }
      app_fsm_bt_state = state;
   }
}

void App_SetBtState2(void* handle, UINT16 state)
{
    struct AppConnInst *connection = NULL;
    struct AppPowerModeStru *node = APP_ACL_LIST.head;
    
    if (app_fsm_bt_state!= state && handle != NULL && state > APP_BT_STATE_CONNECTED_IDLE) {
       connection = App_FindConnectionInstByLowerHandle(handle);
       while (node != NULL) {
          if ( memcmp(connection->bd, node->addr.bd, BD_ADDR_LEN) == 0) {
             node->state = APP_FSM_SNIFF_DATA_STATE_BUSY;
             break;
          }
           node = LNEXT(node);
       }
    }
    if (app_fsm_bt_state != APP_BT_STATE_NULL)
    {
        app_fsm_bt_state = state;
    }
}

UINT16  App_GetBtState(void)
{
   return app_fsm_bt_state;
}
void APP_PairEntry(void)
{
   if (app_fsm_bt_state < APP_BT_STATE_CONNECTED)
      App_SetBtState(APP_BT_STATE_PAIRMODE);

   App_GAP_SetVisualModes(3);
}

void APP_PairResetList(void)
{
   struct AppDevInst *inst = APP_DEVICE_LIST.head;
   struct AppDevInst *curinst;

   while (inst != NULL) {
	   curinst = inst;
	   if (curinst != NULL) {
		   App_DeleteBondedDevice(curinst);
	   }

           inst = LNEXT(inst);
   }

   App_CFG_ClearPairInfo();
}

#ifdef CONFIG_HFP

void APP_AudioThreadSleep(void)
{
#if HAL_USE_AUDIO
    chEvtSignal(app_thread_hfp_audio, APP_BT_HFP_EVENT_SLEEP);
    debug("send APP_BT_HFP_EVENT_SLEEP\r\n");
#endif
}

void APP_AudioThreadWakeup(void)
{
#if HAL_USE_AUDIO
  if (app_thread_wakeup == 0) {
    chSysLock();
    chEvtSignalI(app_thread_hfp_audio, APP_BT_HFP_EVENT_WAKEUP);
    chSysUnlock();
    //debug("send APP_BT_HFP_EVENT_WAKEUP\r\n");
  }
#endif
}

void APP_AudioThreadExit(void)
{
	chSysLock();
	chEvtSignalI(app_thread_hfp_audio, APP_BT_HFP_EVENT_EXIT);
	chSysUnlock();
}

static void App_ThreadMsgHandler(UINT32 mask)
{
#if HAL_USE_AUDIO
	debug("App_ThreadMsHandle event=%d, wakeup=%d\r\n", mask, app_thread_wakeup);
    switch(mask) {
    case APP_BT_HFP_EVENT_SLEEP:
	    app_thread_wakeup = 0;
	    break;
    case APP_BT_HFP_EVENT_WAKEUP:
	    app_thread_wakeup = 1;
    case APP_BT_HFP_EVENT_DATA:
	    if (app_thread_wakeup == 1) {
	      hsc_HfpRecordDataHandle();
	    }
	    break;
    default:
	    break;
    }
#endif
}

void APP_AudioScheduleLoop(void)
{
#if HAL_USE_AUDIO
  UINT32 eventmask;

  app_thread_hfp_audio = chThdSelf();
  app_thread_wakeup = 0;

  while(1) {
     eventmask = chEvtWaitAny(ALL_EVENTS);
     App_ThreadMsgHandler(eventmask);
     //chThdSleepMilliseconds(100);
     if (eventmask & APP_BT_HFP_EVENT_EXIT)
       break;
  }
  app_thread_hfp_audio = NULL;
#endif
}
#if 0
static void App_StopHfpBatteryTimer(struct FsmInst *fi, UINT8 ev, void *arg)
{
   (void)fi;
   (void)ev;
   (void)arg;
 
   app_fsm_hfp_battery_timer = NULL;
   hsc_HfpFsmBattery(NULL, 0, NULL);
}

void APP_FsmBattery(void)
{
    if (app_fsm_hfp_battery_timer == NULL) {
        app_fsm_hfp_battery_timer = FsmAddTimerCx(app_fsm_inst, 2000, 9, App_StopHfpBatteryTimer, NULL, "HFPBATTERY", 9);
    }
}
#endif
#endif /* CONFIG_HFP */

#ifdef CONFIG_A2DP
#define HSC_A2DP_THREAD_EXIT 0x01
#define HSC_A2DP_THREAD_DATA 0x02

void hsc_A2dpThreadExit(void)
{
   osStatus res;
   struct hscA2DPMessageStru* eMsg = (struct hscA2DPMessageStru *)hs_malloc(sizeof(struct hscA2DPMessageStru), __MT_Z_GENERAL);
   if (eMsg != NULL)
   {
      hs_printf("a2dp exit messaage malloc fail!\r\n");
      eMsg->msg = HSC_A2DP_THREAD_EXIT;
   }

   do
   {
      res = oshalMessagePut(app_thread_a2dp_msgid, (UINT32)eMsg, 500);
      if (res != MSG_OK)
      {
        hs_printf("a2dp send data message fail!\r\n");
      }
      //hs_free(eMsg);
   }while(res != MSG_OK);
}

#ifdef APP_A2DP_DATA_USE_FULL_CPY
void hsc_A2dpThradSendDataMessage(UINT16 len, UINT8 *data)
{
   UINT8 mcount = 0;
   osStatus res;
   struct hscA2DPMessageStru* eMsg = NULL;
   size_t n, size;
   // malloc the message + data
   if (len > 4096) hs_printf("max len=%d\r\n", len);
   
   do {
      eMsg = (struct hscA2DPMessageStru *)hs_malloc(sizeof(struct hscA2DPMessageStru)+(len < 4096?4096:len), __MT_Z_GENERAL);
      if (eMsg == NULL && mcount >6)
      {
         n = hs_memInfo(&size, __MT_GENERAL);
         hs_printf("a2dp messaage malloc fail,%d,%u,%u,%d!\r\n", len, n, size, osMessageGetUseCount(app_thread_a2dp_msgid));
         return;
      }
      if (eMsg == NULL)
      {
        mcount++;
        msleep(5);

        // check a2dp playing
        if (App_AudioPlayLocalStop()) return;
      }
      
   }while(eMsg == NULL);
   eMsg->msg = HSC_A2DP_THREAD_DATA;
   eMsg->len = len;
   memcpy(eMsg->data, data,len);

   // send message
   do
   {
      res = oshalMessagePut(app_thread_a2dp_msgid, (UINT32)eMsg, 20);
      if (res != MSG_OK)
      {
        hs_printf("a2dp send data message fail!\r\n");
      }
      //hs_free(eMsg);
   }while(res != MSG_OK);
}
#else
void hsc_A2dpThradSendDataMessage(UINT16 len, UINT8 *data)
{
   UINT8 mcount = 0;
   osStatus res;
   struct hscA2DPMessageStru* eMsg = NULL;
   size_t n, size;
   // malloc the message + data
   if (len > 4096) hs_printf("max len=%d\r\n", len);
   
   do {
      eMsg = (struct hscA2DPMessageStru *)hs_malloc(sizeof(struct hscA2DPMessageStru), __MT_Z_GENERAL);
      if (eMsg == NULL && mcount >6)
      {
         n = hs_memInfo(&size, __MT_GENERAL);
         hs_printf("a2dp messaage malloc fail,%d,%u,%u,%d!\r\n", len, n, size, osMessageGetUseCount(app_thread_a2dp_msgid));
         return;
      }
      if (eMsg == NULL)
      {
        mcount++;
        msleep(5);
        
        // check a2dp playing
        if (App_AudioPlayLocalStop()) return;
      }
      
   }while(eMsg == NULL);
   eMsg->msg = HSC_A2DP_THREAD_DATA;
   eMsg->len = len;
   eMsg->data = data;
   
   // send message
   do
   {
      res = oshalMessagePut(app_thread_a2dp_msgid, (UINT32)eMsg, 20);
      if (res != MSG_OK)
      {
        hs_printf("a2dp send data message fail!\r\n");
      }
      //hs_free(eMsg);
   }while(res != MSG_OK);
}
#endif
//uint32_t timer1, timer2;
void hsc_A2dpScheduleLoop(osMessageQId pstA2dpMsgId)
{
  osEvent eEvt;
  struct hscA2DPMessageStru *eMsg;
  app_thread_a2dp_msgid = pstA2dpMsgId;
  while(1)
  {
    eEvt = oshalMessageGet(pstA2dpMsgId, -1);
    eMsg = (struct hscA2DPMessageStru *)eEvt.value.v;
    //timer1 = BTtimer_Get_Native_Clock();
    //hs_printf("%f,", 312.5*(timer1-timer2)/1000);
    switch(eEvt.status)
    {    
    case osEventMessage:
        
        if(eMsg == NULL || eMsg->msg == HSC_A2DP_THREAD_EXIT)
        {
            app_thread_a2dp_msgid = NULL;
            hs_free(eMsg);
            return;
        }
        
        if(eMsg->msg == HSC_A2DP_THREAD_DATA)
        {
            hsc_A2dpHandleData(eMsg->len, eMsg->data);
            hs_free(eMsg);
        }
        else
        {
            hs_printf("m=%d", eMsg->msg);
        }

      break;
    default:
      break;
    }
    //timer2 = BTtimer_Get_Native_Clock();
    //hs_printf("%f\r\n", 312.5*(timer2-timer1)/1000);
  }
}
#endif /* CONFIG_A2DP */

// for pc tools
uint8_t App_GetPairList(uint8_t *res_buf, uint8_t u8GetCnt)
{
  uint8_t u8HaveGetCnt = 0;
  struct ListStru pairlist = APP_DEVICE_LIST;
  struct AppDevInst *pair = pairlist.head; 
  uint8_t *u8Ptr = res_buf;

  while ((pair != NULL) && (u8GetCnt--))
  {
    memcpy(u8Ptr, pair, sizeof(struct AppDevInst));

    u8Ptr += sizeof(struct AppDevInst);
    u8HaveGetCnt ++;
    
    pair = LNEXT(pair); 
  }

  return u8HaveGetCnt;
}

uint8_t App_GetConnectList(uint8_t *res_buf, uint8_t u8GetCnt)
{
  uint8_t u8HaveGetCnt = 0;
  struct ListStru connlist = APP_CONNECTION_LIST;
  struct AppConnInst *conn = connlist.head;
  uint8_t *u8Ptr = res_buf;

  while ((conn != NULL) && (u8GetCnt--))
  {
    memcpy(u8Ptr, conn, sizeof(struct AppConnInst));

    u8Ptr += sizeof(struct AppConnInst);
    u8HaveGetCnt ++;
    
    conn = LNEXT(conn); 
  }

  return u8HaveGetCnt;
}

#ifdef CONFIG_HFP

// for jvvi, comio, send sco when sco connect
static void App_SendSocTimerHandler(struct FsmInst *fi, UINT8 ev, void *arg)
{
   (void)fi;
   (void)ev;
   (void)arg;
   UINT8 data[HFP_SCO_DATA_SIZE];
   memset(data, 0, HFP_SCO_DATA_SIZE);
   App_HFP_SendSCOPacket(data, HFP_SCO_DATA_SIZE);
   
   app_fsm_hfp_sco_timer = NULL;   
   /* 120/2/8k = 7.5ms */
   app_fsm_hfp_sco_timer = FsmAddTimerCx(app_fsm_inst, 8, 10, App_SendSocTimerHandler, NULL, "SENDSCO", 10);
}

void APP_StartSendScoTimer(void)
{
    if (app_fsm_hfp_sco_timer == NULL) {
        app_fsm_hfp_sco_timer = FsmAddTimerCx(app_fsm_inst, 8, 10, App_SendSocTimerHandler, NULL, "SENDSCO", 10);
    }
}
void APP_StopSendScoTimer(void)
{
   if (app_fsm_hfp_sco_timer != NULL) {
         FsmDelTimer2(app_fsm_hfp_sco_timer);
	 app_fsm_hfp_sco_timer = NULL;
    }
}

#endif /* CONFIG_HFP */

void App_FsmDisconnectAll(void)
{
#if 0
   /* disconnect ACL links */
  struct AppPowerModeStru* node = APP_ACL_LIST.head;
  while (node != NULL) {
     GAP_Disconnect(node->tl_handle, node->acl_hdl);
     node = LNEXT(node);
  }
#else
   /* disconnect profile one by one */
#ifdef CONFIG_HFP
   hsc_HFP_DisconnectAll();
#endif
#ifdef CONFIG_A2DP
   hsc_AVRCP_DisconnectAll();
   hsc_A2DP_DisconnectAll();
#endif
#ifdef CONFIG_HID
   hsc_HID_Disconnect();
#endif
#endif
}

#include "bthost_uapi.h"

bt_host_config_t g_bt_host_config;
bt_host_status_t g_bt_host_status;

void bt_host_get_config(bt_host_config_t **pp_host_cfg)
{
  if (NULL != pp_host_cfg)
    *pp_host_cfg = &g_bt_host_config;
}

void bt_host_get_status(bt_host_status_t **pp_host_sts)
{
  if (NULL != pp_host_sts)
    *pp_host_sts = &g_bt_host_status;
}

UINT8 hsc_GetBtMode(void)
{
    return app_fsm_bt_mode;
}

void hsc_BtVolAdd(void)
{
    if (app_fsm_bt_mode == BT_HOST_VAR_MODE_HID)
        return;
    
    if(App_GetBtState() < APP_BT_STATE_CONNECTED_IDLE)
    {
        hsc_AvrcpUpVol();
    }
    else if(App_GetBtState() == APP_BT_STATE_CONNECTED_IDLE
        || App_GetBtState() == APP_BT_STATE_CONNECTED_STREAMING
        )
    {
        hsc_A2DP_SpkVolAdd();
    }
    else if(App_GetBtState() == APP_BT_STATE_CONNECTED_CALLING)
    {
        hsc_HFP_SpkVolAdd();
    }
}

void hsc_BtVolSub(void)
{
   if (app_fsm_bt_mode == BT_HOST_VAR_MODE_HID)
        return;
   
   if(App_GetBtState() <= APP_BT_STATE_CONNECTED_IDLE)
   {
       hsc_AvrcpDownVol();
   }
   else if(App_GetBtState() == APP_BT_STATE_CONNECTED_STREAMING)
   {
       hsc_A2DP_SpkVolSub();
   }
   else if(App_GetBtState() == APP_BT_STATE_CONNECTED_CALLING)
   {
       hsc_HFP_SpkVolSub();
   }
}

void hsc_BtVolChange(int db)
{
    if (app_fsm_bt_mode == BT_HOST_VAR_MODE_HID)
        return;
   
   if(App_GetBtState() <= APP_BT_STATE_CONNECTED_IDLE)
   {
       hs_audio_volSet(AVOL_DEV_NOR, db);
   }
   else if(App_GetBtState() == APP_BT_STATE_CONNECTED_STREAMING)
   {
       hsc_A2DP_SpkVolChange(db);
   }
   else if(App_GetBtState() == APP_BT_STATE_CONNECTED_CALLING)
   {
       hsc_HFP_SpkVolChange(db);
   }
}