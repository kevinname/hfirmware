
/*---------------------------------------------------------------------------
Description:
	HFP Sample.
---------------------------------------------------------------------------*/
#include "app_main.h"

#ifdef CONFIG_HFP

static HANDLE g_hf_server_hdl = NULL;
static UINT16 g_sco_handle = 0xFFFF; /* Assume one SCO connection */
typedef struct sco_pkt_stats_s {
  uint32_t tx_prev;
  uint32_t rx_prev;
  uint32_t tx;
  uint32_t rx;
  uint32_t rx_comp;
} sco_pkt_stats_t;
static sco_pkt_stats_t m_sco;

static struct HCI_CbkLineStru sco_hci_table[] = 
{
	{HCI_EVO_SCO_DATA_IND, 				APP_SCODataInd}
};

static uint32_t g_len;
static UINT8 s_hfp_multi_link_num = 2;
static UINT8 s_hsp_multi_link_num = 2;
static UINT8 s_hfp_free_link_num = 0;
static UINT8 s_hsp_free_link_num = 0;
static UINT16 s_hfp_call_state = BT_HFP_CALL_STATUS_STANDBY;
static HANDLE s_hfp_cur_conn_handle = NULL;
static UINT8 s_hf_reconnect = 0;
static UINT8 s_hfp_use_pre_sco = 0;
static UINT8 s_hfp_cur_call_number[HFP_PHONENUM_MAX_DIGITS];
static UINT8 s_hfp_last_call_number[HFP_PHONENUM_MAX_DIGITS];
static UINT8 s_hfp_incoming_call = 0;
static UINT8 s_hfp_reg_sco = 0;
static UINT8 s_hfp_incoming_call_ring = 0;
static UINT8 s_hfp_spk_vol      = 9; // 0-15
static UINT8 s_hfp_mic_vol      = 12; // 0-15

/*******************************************************************
*																	*
********************************************************************/

#if HAL_USE_AUDIO
static int app_hfp_ply_level2db(char level)
{
  int min = audioPlayGetVolumeMin(AUDIO_PLY_SRC_HFP);
  int max = audioPlayGetVolumeMax(AUDIO_PLY_SRC_HFP);
  //int curdb = hs_audio_volGet(AVOL_DEV_HFP);
  int vol = (min+ (max - min) * level / 15);
  //hs_printf("hfp:min=%d,max=%d, cur=%d, level=%d---->%d \r\n", 
  //           min, max, curdb ,level, vol);
  return vol;
}

static int app_hfp_ply_db2level(int db)
{
   int min = audioPlayGetVolumeMin(AUDIO_PLY_SRC_HFP);
   int max = audioPlayGetVolumeMax(AUDIO_PLY_SRC_HFP);
   return ((db-min)*15)/(max-min);
}

static int app_hfp_rec_level2db(char level)
{
  int min = audioRecordGetVolumeMin(AUDIO_REC_SRC_HFP);
  int max = audioRecordGetVolumeMax(AUDIO_REC_SRC_HFP);
  return min + (max - min) * level / 15;
}
#endif /* HAL_USE_AUDIO */

void hsc_HfpSetSpkVol(UINT8 level)
{
#if HAL_USE_AUDIO
  if (level == 0)
    audioPlayMute();
  else
    audioPlayUnmute();
    
  s_hfp_spk_vol = level;
  if (App_AudioLocalTonePlay() == 0) // when play tone, do not set local vol
  {
    hs_audio_volSet(AVOL_DEV_HFP, app_hfp_ply_level2db(level));
    //hs_printf("HFP: spk vol=%d\r\n", s_hfp_spk_vol);
  }
#endif
}

static void hsc_HfpSetMicVol(UINT8 level)
{
#if HAL_USE_AUDIO
  if ((level == 0) && (s_hfp_mic_vol > 0))
    audioRecordMute();
  else if (level > 0 && s_hfp_mic_vol == 0)
    audioRecordUnmute();
   
  s_hfp_mic_vol = level;
  app_hfp_rec_level2db(level);
  //audioRecordSetVolume(app_hfp_rec_level2db(level));
   
  debug("HFP: mic vol=%d\r\n", s_hfp_mic_vol);
#endif
}

#if HAL_USE_AUDIO
void hsc_HfpRecordDataHandle(void)
{
  UINT8 *rec_buf1, *rec_buf2;
  UINT32 rec_len1, rec_len2;

  while(1) {
    if (App_AudioRecordStop() == 1) {
      break;
    }
    if (App_AudioPlayLocalStop() == 1) {
      osDelay(10);
      continue;
    }

    /* 3.75ms per 60 bytes in EV3; 7.5ms per 120 bytes in 2-EV3 */
    rec_len1 = audioRecGetDataBuffer(&rec_buf1, HFP_SCO_DATA_SIZE, MS2ST(BT_I2S_REC_BLOCK_TIME));
    /* BT_I2S_REC_BLOCK_SIZE=1200, i.e. 75ms */
    if ((rec_len1 > 0) && (rec_len1 < HFP_SCO_DATA_SIZE)) {
      rec_len2 = audioRecGetDataBuffer(&rec_buf2, HFP_SCO_DATA_SIZE-rec_len1, MS2ST(BT_I2S_REC_BLOCK_TIME));
    }
    else {
      rec_len2 = 0;
    }

    if (rec_len1 > 0) {
      uint8_t hdr_buf[3];
      uint8_t *pdu_buf;

      /* directly send SCO via HCI rather than submit SCO to host FSM */
      //App_HFP_SendSCOPacket(rec_buf, rec_len);
      hdr_buf[0] = (uint8_t)g_sco_handle;
      hdr_buf[1] = (uint8_t)(g_sco_handle >> 8);
      hdr_buf[2] = rec_len1+rec_len2;
      pdu_buf = (uint8_t *)HCI_Generic_Get_Rx_Buf(HCI_TLPKT_SCODATA, rec_len1+rec_len2, hdr_buf);
      if (NULL != pdu_buf) {
        memcpy(pdu_buf, rec_buf1, rec_len1);
        if (rec_len2 > 0)
          memcpy(pdu_buf+rec_len1, rec_buf2, rec_len2);
        HCI_Generic_Commit_Rx_Buf(HCI_TLPKT_SCODATA);
      }

      audioRecGetDataDone(rec_buf1, rec_len1);
      if (rec_len2 > 0)
        audioRecGetDataDone(rec_buf2, rec_len2);
      m_sco.tx++;
    }
    else { // rec_len = 0
      debug("HFP: record get buffer len = 0\r\n");
    }
  }
}
#endif /* HAL_USE_AUDIO */

void hsc_HFP_SpkVolAdd(void)
{
#if HAL_USE_AUDIO
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);
  //UINT16 v_hfp_status = hsc_HFP_GetState();
  //if (v_hfp_status == BT_HFP_CALL_STATUS_STANDBY)
  //  return;

  s_hfp_spk_vol++;
  if (s_hfp_spk_vol >= 15)
    s_hfp_spk_vol = 15;
  hsc_HfpSetSpkVol(s_hfp_spk_vol);

  if (connection != NULL) 
  {
    HFP_HF_SetSpkVol(connection->profile_inst_handle, s_hfp_spk_vol);
  }
#endif
}

void hsc_HFP_SpkVolSub(void)
{
#if HAL_USE_AUDIO
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);
  //UINT16 v_hfp_status = hsc_HFP_GetState();
  //if (v_hfp_status == BT_HFP_CALL_STATUS_STANDBY)
  //  return;

  if (s_hfp_spk_vol > 0)
    s_hfp_spk_vol--;
  hsc_HfpSetSpkVol(s_hfp_spk_vol);

  if (connection != NULL) 
  {
    HFP_HF_SetSpkVol(connection->profile_inst_handle, s_hfp_spk_vol);
  }
#endif
}

void hsc_HFP_SpkVolChange(int db)
{
#if HAL_USE_AUDIO
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);
    
  s_hfp_spk_vol = app_hfp_ply_db2level(db);
  //hs_printf("hfp:%d,%d,%d, %x\r\n", s_hfp_spk_vol, db, App_AudioPlayLocalStop(), connection);
  if (App_AudioPlayLocalStop() == 0) // when play tone, do not set local vol
  {
    hs_audio_volSet(AVOL_DEV_HFP, db);
  }

  if (connection != NULL) 
  {
    HFP_HF_SetSpkVol(connection->profile_inst_handle, s_hfp_spk_vol);
  }
#endif

}

void hsc_HFP_MicVolAdd(void)
{
#if HAL_USE_AUDIO
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);
  UINT16 v_hfp_status = hsc_HFP_GetState();
  if (v_hfp_status == BT_HFP_CALL_STATUS_STANDBY)
    return;

  s_hfp_mic_vol++;
  if (s_hfp_mic_vol >= 15)
    s_hfp_mic_vol = 15;
  hsc_HfpSetMicVol(s_hfp_mic_vol);

  if (connection != NULL) 
  {
    HFP_HF_SetMicVol(connection->profile_inst_handle, s_hfp_mic_vol);
  }
#endif
}

void hsc_HFP_MicVolSub(void)
{
#if HAL_USE_AUDIO
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);
  UINT16 v_hfp_status = hsc_HFP_GetState();
  if (v_hfp_status == BT_HFP_CALL_STATUS_STANDBY)
    return;

  if (s_hfp_mic_vol > 0)
    s_hfp_mic_vol--;
  hsc_HfpSetMicVol(s_hfp_mic_vol);

  if (connection != NULL) 
  {
    HFP_HF_SetMicVol(connection->profile_inst_handle, s_hfp_mic_vol);
  }
#endif
}


UINT32 APP_SCODataInd(HANDLE hdl, void *context, struct BuffStru *buf, UINT32 op_ev)
{
    UINT16 data_len = DATASIZE(buf);
    //UINT16 conn_hdl;
    UINT16 packet_len;
    UINT8 *p;

    (void)hdl;
    (void)context;
    (void)op_ev;    

    APP_StopSendScoTimer();
#if HAL_USE_AUDIO
    App_AudioStart(I2S_SAMPLE_8K, I2S_PCMMODE_MONO, BT_I2S_HFP_PLY_START_TH);
    if (App_AudioPlayLocalStop() == 1) {
        FREE(buf);
        return 0;
    }
    App_AudioRecordStart();
#endif
    
    /* The incoming buffer may contain multiple SCO packets */
    while (data_len > 0) {
       p = BUFDATA(buf); /* SCO packet begin with 3bytes HCI SCO Header */
       //conn_hdl = ((UINT16)*p + ((UINT16)*(p + 1) << 8)) & 0x0FFF;/* SCO Connection Handle, Little Endian */
       packet_len = *(p + 2);/* SCO data size */
       
       if (data_len >= packet_len) {
          p += 3;
          /* TODO: Play the voice data */ 
#if HAL_USE_AUDIO
{
    UINT8 *buffer;
    UINT32 re_len;
    INT32 pre_len;
    UINT16 this_audio_len = packet_len;
    UINT8 *this_audio_buf = p;
    UINT8 *temp_buffer;
    systime_t timeout = MS2ST(8);

    //App_AudioStart(I2S_SAMPLE_8K, I2S_PCMMODE_MONO, BT_I2S_HFP_PLY_START_TH);
    if (App_AudioPlayLocalStop() == 0) {

        // play
        g_len += this_audio_len;
        pre_len = this_audio_len;
        temp_buffer = this_audio_buf;
        while(pre_len >0) {
           if (App_AudioPlayLocalStop() == 1) {
              g_len = 0;
              m_sco.rx = m_sco.tx = m_sco.rx_comp = 0;
              break;
           }
           re_len = audioPlyGetDataBuffer(&buffer, pre_len, timeout);           
           //if (re_len > 0) {
              //memcpy(buffer, temp_buffer, re_len);
              audioPlyCpyData(buffer, temp_buffer, re_len);
              audioPlySendDataDone(buffer, re_len);         
              pre_len -= re_len;
              temp_buffer += re_len;
           //}
        };

        m_sco.rx++;
        /* android phone: skip 300 pkts */
        if(g_len == (HFP_SCO_DATA_SIZE*300)) {
          m_sco.rx_prev = m_sco.rx / (HFP_SCO_DATA_SIZE / packet_len);
          m_sco.tx_prev = m_sco.tx;
          m_sco.rx = m_sco.tx = m_sco.rx_comp = 0;
          hs_audio_enAec();
        }
        if (App_AudioPlayLocalStop() == 1) {
          g_len = 0;
          m_sco.rx = m_sco.tx = m_sco.rx_comp = 0;
        }
        if(g_len > (HFP_SCO_DATA_SIZE*300)) {
          uint32_t rx_cnt = m_sco.rx / (HFP_SCO_DATA_SIZE / packet_len);
          uint32_t delta = rx_cnt > m_sco.tx ? 0 : m_sco.tx - rx_cnt;
          uint8_t *ply_buf1, *ply_buf2;
          uint8_t ply_len1, ply_len2;

          /* AEC algorithm supports the max latency of two inputs is 32ms */
          if (delta > 5) {
            m_sco.rx += (HFP_SCO_DATA_SIZE / packet_len);
            m_sco.rx_comp++;

            /* BT_I2S_PLY_BLOCK_SIZE=2400, i.e. 150ms */
            //hs_printf("comp=%u: t=%d,r=%d; ", m_sco.rx_comp, m_sco.tx, m_sco.rx);
            ply_len1 = audioPlyGetDataBuffer(&ply_buf1, HFP_SCO_DATA_SIZE, MS2ST(BT_I2S_HFP_PLY_BLOCK_TIME));
            memset(ply_buf1, 0x00, ply_len1);
            /* always call audioPlySendDataDone() even if submitted size is 0 */
            audioPlySendDataDone(ply_buf1, ply_len1);
            //hs_printf(" %d ", ply_len1);
            if ((ply_len1 > 0) && (ply_len1 < HFP_SCO_DATA_SIZE)) {
              ply_len2 = audioPlyGetDataBuffer(&ply_buf2, HFP_SCO_DATA_SIZE-ply_len1, MS2ST(BT_I2S_HFP_PLY_BLOCK_TIME));
              memset(ply_buf2, 0x00, ply_len2);
              audioPlySendDataDone(ply_buf2, ply_len2);
            }
            else {
              ply_len2 = 0;
            }

            //hs_printf(" %d\r\n", ply_len2);
          }
        }
    }
    else
    {
       g_len = 0;
       m_sco.rx = m_sco.tx = m_sco.rx_comp = 0;
       break;
    }
}
#endif /* HAL_USE_AUDIO */
          data_len -= packet_len + 3;				
       } else {
          /* Packet error - size mismatch */
          break;
       }
       BuffRes(buf, (INT16)(packet_len + 3));
    }
    FREE(buf);
    return 0;
}

/*******************************************************************
*																	*
********************************************************************/
void HFSppConnectCfm(struct HFP_ConnectEvStru *in)
{
	struct HFP_ConnectEvStru *param = NEW(sizeof(struct HFP_ConnectEvStru));
	memcpy(param, in, sizeof(struct HFP_ConnectEvStru));
	App_SCH_FsmEvent(App_Hfp_FsmSppConnectCfm, param);
}

/*******************************************************************
*																	*
********************************************************************/
void HFSppConnectInd(struct HFP_ConnectEvStru *in)
{
	struct HFP_ConnectEvStru *param = NEW(sizeof(struct HFP_ConnectEvStru));
	memcpy(param, in, sizeof(struct HFP_ConnectEvStru));
	App_SCH_FsmEvent(App_Hfp_FsmSppConnectInd, param);
}

/*******************************************************************
*																	*
********************************************************************/
void HFSlcEstablishInd(struct HFP_SLCConnectInfoStru *in)
{
	struct HFP_SLCConnectInfoStru *param = NEW(sizeof(struct HFP_SLCConnectInfoStru));
	memcpy(param, in, sizeof(struct HFP_SLCConnectInfoStru));
	App_SCH_FsmEvent(App_Hfp_FsmSlcEstablishInd, param);
}

/*******************************************************************
*																	*
********************************************************************/
void HFSlcReleaseInd(struct HFP_SLCConnectInfoStru *in)
{
	struct HFP_SLCConnectInfoStru *param = NEW(sizeof(struct HFP_SLCConnectInfoStru));
	memcpy(param, in, sizeof(struct HFP_SLCConnectInfoStru));
	App_SCH_FsmEvent(App_Hfp_FsmSlcReleaseInd, param);
}

/*******************************************************************
*																	*
********************************************************************/
void HfpSppDisconnectInd(struct HFP_ConnectEvStru *in)
{
	struct HFP_ConnectEvStru *param = NEW(sizeof(struct HFP_ConnectEvStru));
	memcpy(param, in, sizeof(struct HFP_ConnectEvStru));
	App_SCH_FsmEvent(App_Hfp_FsmSppDisconnectInd, param);
}

/*******************************************************************
*																	*
********************************************************************/
void HfpSppStateChangedInd(void *hdl, UINT8 event)
{
	struct AppHfpNewState *param = NEW(sizeof(struct AppHfpNewState));
	param->handle = hdl;
	param->event = event;
	App_SCH_FsmEvent(App_Hfp_FsmStateChangedInd, param);
}

/*******************************************************************
*																	*
********************************************************************/
void HFCallWatingInd(void *hdl, struct HFP_PhoneInfoStru *in)
{
	struct AppHfpWaitingCall *param = NEW(sizeof(struct AppHfpWaitingCall));
	param->handle = hdl;
	strcpy(param->number, (const char *)in->number);
	App_SCH_FsmEvent(App_Hfp_FsmCallWaitingInd, param);
}

/*******************************************************************
*																	*
********************************************************************/
void HFRingInd(UINT8 in_band)
{
   UINT8 ring = 1;
   debug("HFP-->HFRingInd [%d]\r\n", in_band);
   if (in_band == 1) {
#if HS_USE_CONF
        /*	TODO: Generate local ring tone */
        hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_IN_BAND_RING);
#endif
        s_hfp_incoming_call_ring = 1;
        ring = 0;
   } /* else, in band ring tone */
   else
   {
       s_hfp_incoming_call_ring = 0;
   }

#if HS_USE_CONF
   if (ring == 1) {
      hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_RING);
   }
#endif
}

/*******************************************************************
*																	*
********************************************************************/
void HFAudioConnectInd(UINT16 sco_handle)
{
	debug("HFP-->HFAudioConnectInd [%d]\r\n", sco_handle);
	if (g_sco_handle == 0xFFFF) {
		g_sco_handle = sco_handle;
		/*	TODO: Switch local audio path to Bluetooth SCO/eSCO link */
	}
	/* else, another SCO connection is created */
       #ifdef CONFIG_A2DP
	if (App_CFG_GetPtsEnable() == 0 && hsc_A2DP_GetState() == APP_A2DP_STATE_START) {
		hsc_A2dpDisableAudioStreamHandle();
        hsc_A2DP_StreamPause();
        hsc_A2DP_Stop();
    }
       #endif
       
       #if HAL_USE_AUDIO
       App_AudioStart(I2S_SAMPLE_8K, I2S_PCMMODE_MONO, BT_I2S_HFP_PLY_START_TH);
       if (App_AudioPlayLocalStop() == 0) {
          App_AudioRecordStart(); 
       }

       g_len = 0;
       #endif 

       #if HAL_USE_AUDIO
       //hsc_HfpSetMicVol(s_hfp_mic_vol);
       //hsc_HfpSetSpkVol(s_hfp_spk_vol);
       {
           struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);
           if ( connection != NULL)
            HFP_HF_SetSpkVol(connection->profile_inst_handle, s_hfp_spk_vol);
       }
       #endif

       /* accept incoming call at phone side */
       //s_hfp_call_state = BT_HFP_CALL_STATUS_ONGOING;

       //App_SetBtState2(APP_BT_STATE_CONNECTED_CALLING);
       if (s_hfp_use_pre_sco == 1)
          APP_StartSendScoTimer();
       hs_cfg_sysSendMsg(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_SCO_STATUS, (void*)1);
}
/*******************************************************************
*																	*
********************************************************************/
void HFSetAGBatteryValue(UINT8 val)
{
	#if HS_USE_CONF
	//if (val == 1 && s_hfp_ag_battery_val == 2) {
	if (val == 1) {
		;//hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_STATUS, HS_CFG_STATUS_PHONE_BATTERY_EMPTY);
	}
    #endif
	//s_hfp_ag_battery_val = val;
}
/*******************************************************************
*																	*
********************************************************************/
void hsc_HFP_NotifyBatteryChange(UINT8 val)
{
   char cmd[30];
   struct AppConnInst *connection;
   if (App_GetBtState() < APP_BT_STATE_CONNECTED) {
     return;
   }
   connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);
   //hs_printf("bat change=%d\r\n", val);
   if (connection != NULL) {
      val = val>9?9:val;
      memset(cmd, 0, 30);
      sprintf(cmd, "AT+IPHONEACCEV=1,1,%d\r", val);
      HFP_ExtendCmd(connection->profile_inst_handle, (void*)cmd, strlen(cmd), 6000);
   }
}

/*******************************************************************
*																	*
********************************************************************/
void hsc_HFP_SendBattery(void)
{
   UINT8 value = 0;

#if HAL_USE_ADC /*&& HS_ADC_USE_BAT*/
#if defined(__nds32__)
   const hs_drvhal_cfg_t *pstDrvCfg = hs_boardGetDrvCfg();
   float curBatVol = hs_adc_getBatteryVolt();
   
   curBatVol = (curBatVol-pstDrvCfg->fBatEmptyVolt)/(pstDrvCfg->fBatFullVolt - pstDrvCfg->fBatEmptyVolt);
   value = (UINT8)(curBatVol*10);
#else
   value = (UINT8)app_pm_battery_getValue();
#endif
#endif
   //hs_printf("hsc_HFP_SendBattery\r\n");
   hsc_HFP_NotifyBatteryChange(value);
}

/*******************************************************************
*																	*
********************************************************************/
static void hsc_HfpFsmBattery(struct FsmInst *fi, UINT8 event, UINT16 *in)
{
    (void)fi;
    (void)event;
    (void)in;
    struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);
	char cmd[30];
	if (connection != NULL) {
       memset(cmd, 0, 30);
	   sprintf(cmd, "AT+XAPL=HS-6600-0100,%d\r", 1);
	   HFP_ExtendCmd(connection->profile_inst_handle, cmd, strlen(cmd), 6000);
	}
}
/*******************************************************************
*																	*
********************************************************************/
void hsc_HFP_BatteryChanged(uint16_t u16Msg, void *parg)
{
	(void)u16Msg;
  const hs_drvhal_cfg_t *pstDrvCfg = hs_boardGetDrvCfg();
    float value = (float)((uint32_t)parg);

    value = (value - pstDrvCfg->fBatEmptyVolt)/(pstDrvCfg->fBatFullVolt - pstDrvCfg->fBatEmptyVolt);
    //hs_printf("hsc_HFP_BatteryChanged\r\n");
    hsc_HFP_NotifyBatteryChange((UINT8)(value*10));
}

/*******************************************************************
*																	*
********************************************************************/
void HFAudioDisconnectInd(UINT16 sco_handle)
{
   debug("HFP-->HFAudioDisconnectInd [%d]\r\n", sco_handle);
   if (g_sco_handle == sco_handle) {
      g_sco_handle = 0xFFFF;
      /*TODO: Switch local audio path to Bluetooth SCO/eSCO link */
   }
   
   APP_StopSendScoTimer();
   
   #if HAL_USE_AUDIO
   App_AudioStop();
   /* enable DRC after bluetooth phone call */
   audioSetAdcDrcMode(3);
   audioSetDacDrcMode(3);
   #endif
   
   App_SetBtState2(NULL, APP_BT_STATE_CONNECTED_IDLE);
   
   memset(s_hfp_last_call_number, 0, HFP_PHONENUM_MAX_DIGITS);
   memcpy(s_hfp_last_call_number, s_hfp_cur_call_number, HFP_PHONENUM_MAX_DIGITS);
   s_hfp_incoming_call = 0;
   s_hfp_incoming_call_ring = 0;
   #if HAL_USE_AUDIO
   App_CFG_SetHfpVol(s_hfp_spk_vol, s_hfp_mic_vol);
   #endif
   //s_hfp_call_state = BT_HFP_CALL_STATUS_STANDBY;
   hs_cfg_sysSendMsg(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_SCO_STATUS, (void*)0);
}

/*******************************************************************
*																	*
********************************************************************/
UINT8 APP_HFEventCbk(void *hdl, UINT8 event, UINT8 *param, UINT32 len)
{
        (void)hdl;
        (void)len;
        //hs_printf("HFP: EVENT=%d\r\n", event);
        //debug("HFP: EVENT=%d\r\n", event);
	switch (event) {
		/* Server Start Confirmation */
		case HFP_EV_SERVER_START_CFM:/* param: struct HFP_RegServerStru */
			{
				struct HFP_RegServerStru *in = (struct HFP_RegServerStru *)param;
				if (in->role & HFP_ROLE_HF_SERVER) { // HF Server
					g_hf_server_hdl = hdl;
				}
			}
			break;
		/* Connect Start Confirmation */
		case HFP_EV_CONNECT_CFM:/* param: struct HFP_ConnectEvStru */
			HFSppConnectCfm((struct HFP_ConnectEvStru *)param);
			break;
		case HFP_EV_CONNECT_IND:/* param: struct HFP_ConnectEvStru */
			HFSppConnectInd((struct HFP_ConnectEvStru *)param);
			break;
		/* Connection Complete indication */
	    case HFP_EV_SPP_ESTABLISHED_IND:/* param: struct HFP_SLCConnectInfoStru */
	        break;
		case HFP_EV_SLC_ESTABLISHED_IND:/* param: struct HFP_SLCConnectInfoStru */
			((struct HFP_SLCConnectInfoStru *)param)->server_handle = hdl;
			HFSlcEstablishInd((struct HFP_SLCConnectInfoStru *)param);
			break;
		/* Connection Terminated indication */
		case HFP_EV_SLC_RELEASED_IND:/* param: HFP_SLCConnectInfoStru */
			((struct HFP_SLCConnectInfoStru *)param)->server_handle = hdl;
			HFSlcReleaseInd((struct HFP_SLCConnectInfoStru *)param);
			break;
		case HFP_EV_DISCONNECT_COMPLETE:/* param: struct HFP_ConnectEvStru */
			HfpSppDisconnectInd((struct HFP_ConnectEvStru *)param);
			break;

		/* State changed indications */
		case HFP_EV_STANDBY_IND:
		case HFP_EV_ONGOINGCALL_IND:
		case HFP_EV_INCOMINGCALL_IND:
		case HFP_EV_OUTGOINGCALL_IND:
		case HFP_EV_CALLHELD_IND:
			HfpSppStateChangedInd(hdl, event);
			break;
		case HFP_EV_CALL_WAITING_IND:/* param: struct HFP_PhoneInfoStru */
			HFCallWatingInd(hdl, (struct HFP_PhoneInfoStru *)param);
			break;

		/* Confirmation to the requests (AT Commands) from APP */
		case HFP_EV_ATCMD_RESULT_IND:/* param: struct HFP_ATCmdResultStru */
			break;

		/* Local indications */
		case HFP_EV_RINGING_IND:/* param: UINT8 in_band */
			HFRingInd(*param);
			break;
		case HFP_EV_AUDIO_CONN_ESTABLISHED_IND:/* param: UINT16 sco_hdl */
			HFAudioConnectInd(*(UINT16 *)param);
                        App_SetBtState2(hdl, APP_BT_STATE_CONNECTED_CALLING);
			break;
		case HFP_EV_AUDIO_CONN_RELEASED_IND:/* param: UINT16 sco_hdl */
			HFAudioDisconnectInd(*(UINT16 *)param);
			break;
		case HFP_EV_TERMINATE_LOCAL_RINGTONE_IND:
		        debug("\nHF-->Terminate local ring tone and un-mute the audio link!\nHF>\r\n");
			/* TODO: The application shall terminate its local ring tone here if it has been started before. */
			
			break;
		case HFP_EV_HF_MANUFACTURERID_IND:/* param: UINT8 *vendor_id */
		        //debug("\nHF-->AG Manufacturer ID retrieved: %s.\nHF>\r\n", param);
			break;
		case HFP_EV_HF_MODELID_IND:/* param: UINT8 *vendor_id */
		        //debug("\nHF-->AG Modle ID retrieved: %s.\nHF>\r\n", param);
			break;
			
		/* Indications or responses from AG */
		case HFP_EV_VOICE_RECOGN_ACTIVATED_IND:
			break;
		case HFP_EV_VOICE_RECOGN_DEACTIVATED_IND:
			break;
		case HFP_EV_NETWORK_AVAILABLE_IND:
			break;
		case HFP_EV_NETWORK_UNAVAILABLE_IND:
			break;
		case HFP_EV_ROAMING_RESET_IND:
			break;
		case HFP_EV_ROAMING_ACTIVE_IND:
			break;
		case HFP_EV_SIGNAL_STRENGTH_IND:/* param: UINT8 val */
			break;
		case HFP_EV_BATTERY_CHARGE_IND:/* param: UINT8 val 0-5*/
			HFSetAGBatteryValue(*param);
			break;
		case HFP_EV_CALLON_CHLDHOLD_ACTIVATED_IND:
			break;
		case HFP_EV_CALLON_CHLDHOLD_RELEASED_IND:
			/* Whether the call is activated or released is determined by the indication previous to
			   HFP_EV_CALLON_CHLDHOLD_RELEASED_IND. */
			break;
		case HFP_EV_MICVOL_CHANGED_IND:/* param: UINT8 vol */
            #if HAL_USE_AUDIO
			/* TODO: Change the local microphone volume. */
			hsc_HfpSetMicVol(*param);
            #endif
			break;
		case HFP_EV_SPKVOL_CHANGED_IND:/* param: UINT8 vol */
            #if HAL_USE_AUDIO
			/* TODO: Change the local speaker volume. */
            hs_cfg_sysSendMsg(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_REMOTE_VOL, (void*)((UINT32)(*param)));
            #endif
			break;
		case HFP_EV_CLIP_IND:/* param: struct HFP_PhoneInfoStru */
			{
				struct HFP_PhoneInfoStru *call_info = (struct HFP_PhoneInfoStru *)param;
				if (call_info == NULL)
					break;
#if defined(CONFIG_PATCH_HUNTERSUN)
				memset(s_hfp_cur_call_number, 0, HFP_PHONENUM_MAX_DIGITS);
				memcpy(s_hfp_cur_call_number, call_info->number, HFP_PHONENUM_MAX_DIGITS);
#if HS_USE_CONF
                if(s_hfp_incoming_call == 0)
                {
                    s_hfp_incoming_call = 1;
                    hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, 
                        HS_CFG_EVENT_BT_INCOMING_CALL);
                    hs_printf("HF-->Calling number is: %s\r\n", call_info->number);
                }
#endif
#endif
			}
			break;
		case HFP_EV_CURRENT_CALLS_IND:/* param: struct HFP_CLCCInfoStru */
		        {
				struct HFP_CLCCInfoStru *clcc = (struct HFP_CLCCInfoStru *)param;
                (void)clcc;
				debug("HF-->Call Information: <idx:%d>:<Num:%s><Type:%d><dir:%d><status:%d><mode:%d><mpty:%d>\nHF>\r\n", 
				       clcc->idx, clcc->number, clcc->type, clcc->dir, clcc->status, clcc->mode, clcc->mpty);
			}
			break;
		case HFP_EV_NETWORK_OPERATOR_IND:/* param: struct HFP_COPSInfoStru */
			debug("HFP: NETWORK_OPERATOR_IND\r\n");
			break;
		case HFP_EV_SUBSCRIBER_NUMBER_IND:/* param: struct HFP_PhoneInfoStru */
			{
				debug("\nHF-->The subscriber number is: %s\nHF>", (struct HFP_PhoneInfoStru *)param->number);
			}
			break;
		case HFP_EV_VOICETAG_PHONE_NUM_IND:/* param: struct HFP_PhoneInfoStru *info */
                        debug("HFP: Attach the phone number to a voice tag\r\n");
			/* TODO: Attach the phone number to a voice tag. */
			break;

		/* Extended result codes from AG */
		case HFP_EV_EXTEND_CMD_IND: /* param: RAW data */
		{
			UINT8 vIndex = 0;
			debug("HFP: extend cmd ind len=%d\r\n", len);

			//for (vIndex = 0; vIndex< len; vIndex++) {
			//	debug("HFP: param[%d] = %x\r\n", vIndex, param[vIndex]);
			//}
			// 0d0a +XAPL=iPhone,val 0d0a
			if(strncmp((char*)param+2, "+XAPL=", 6)==0) {
				//debug("HFP: +XAPL\r\n");	
				hsc_HFP_SendBattery();
				hs_cfg_sysListenMsg(HS_CFG_EVENT_BATTERY_CHANGED, hsc_HFP_BatteryChanged); 
				for (vIndex = 8; vIndex< len; vIndex++) {
					if (param[vIndex] == ',') {
						HFSetAGBatteryValue(param[vIndex+1]);
						break;
					}
				}
				
			}
		}
			break;
		default:
			break;
	}

	return 1;
}

/*******************************************************************
*																	*
********************************************************************/
void App_Hfp_FsmConnectAG(struct FsmInst *fi, UINT8 event, UINT8 *bd)
{
	struct AppConnInst *connection = App_FindConnectionInstByBD(CLS_HANDSFREE_AG, bd, APP_CONNECT_IGNORE_ROLE);

        (void)fi;
        (void)event;
   
	if (NULL == connection) {
		if (s_hfp_multi_link_num > 0) {
		   App_AddConnectionInst(CLS_HANDSFREE_AG, bd, APP_CONNECT_INITIATOR);
		   HFP_Client_Start(g_hf_server_hdl, bd, NULL, HFP_ROLE_HF_CLIENT, APP_HFEventCbk);
		}
	} else {
#ifdef CONFIG_MMI
		if (connection->profile_inst_handle == NULL) {
			/* Only one HFP connection is allowed between two devices */
			MMI_HfpConnectCfm(bd, HCI_STATUS_CONNECTION_LIMIT_EXCEEDED, 0);
		} else {
			/* Return the current connection handle */
			MMI_HfpConnectCfm(bd, HCI_STATUS_OK, connection->connection_handle);
		}
#endif
	}
	FREE(bd);
}

void App_Hsp_FsmConnectAG(struct FsmInst *fi, UINT8 event, UINT8 *bd)
{
	struct AppConnInst *connection = App_FindConnectionInstByBD(CLS_HANDSFREE_AG, bd, APP_CONNECT_IGNORE_ROLE);

        (void)fi;
        (void)event;
   
	if (NULL == connection) {
		if (s_hsp_multi_link_num > 0) {
		   App_AddConnectionInst(CLS_HANDSFREE_AG, bd, APP_CONNECT_INITIATOR);
		   HFP_Client_Start(g_hf_server_hdl, bd, NULL, HFP_ROLE_HS_CLIENT, APP_HFEventCbk);
	        }
	} else {
#ifdef CONFIG_MMI
		if (connection->profile_inst_handle == NULL) {
			/* Only one HFP connection is allowed between two devices */
			MMI_HfpConnectCfm(bd, HCI_STATUS_CONNECTION_LIMIT_EXCEEDED, 0);
		} else {
			/* Return the current connection handle */
			MMI_HfpConnectCfm(bd, HCI_STATUS_OK, connection->connection_handle);
		}
#endif
	}
	FREE(bd);
}
/*******************************************************************
*																	*
********************************************************************/
void App_Hfp_FsmSppConnectCfm(struct FsmInst *fi, UINT8 event, struct HFP_ConnectEvStru *in)
{
	struct AppConnInst *connection = App_FindConnectionInstByBD(CLS_HANDSFREE_AG, in->bd, APP_CONNECT_INITIATOR);

    (void)fi;
    (void)event;
	debug("HFP-->App_Hfp_FsmSppConnectCfm\r\n");
	if (connection != NULL) {
		/* Now the APP can cancel the connection setup procedure */
		connection->profile_inst_handle = in->handle;
	}

    #if defined(CONFIG_PATCH_HUNTERSUN)
	if (s_hf_reconnect == 1) {
		App_HSP_ConnectAG(in->bd);
		s_hf_reconnect = 0;
	}
	#endif
	FREE(in);
}
/*******************************************************************
*																	*
********************************************************************/
void App_Hfp_FsmSppConnectInd(struct FsmInst *fi, UINT8 event, struct HFP_ConnectEvStru *in)
{
	/* Shall exist only one incoming connection from the same AG */
	struct AppConnInst *connection = App_AddConnectionInst(CLS_HANDSFREE_AG, in->bd, APP_CONNECT_RESPONDER);

    (void)fi;
    (void)event;
	debug("HFP-->App_Hfp_FsmSppConnectInd, handle=%x\r\n", in->handle);

	if (NULL != connection)
	{
		connection->profile_inst_handle = in->handle;
    }
	FREE(in);
}

/*******************************************************************
*																	*
********************************************************************/
void App_Hfp_FsmSlcEstablishInd(struct FsmInst *fi, UINT8 event, struct HFP_SLCConnectInfoStru *in)
{
	struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(in->server_handle);

        (void)fi;
        (void)event;
	debug("HFP-->App_Hfp_FsmSlcEstablishInd\r\n");
	if (connection != NULL) {
		if (s_hfp_reg_sco == 0) {
			HCI_Cbk_Reg(sco_hci_table, sizeof(sco_hci_table) / sizeof(struct HCI_CbkLineStru));
		}
		s_hfp_free_link_num--;
		hsc_UpdateConnectionInst(connection, APP_CONNECT_STATE_CONNECTED);
		#if defined(CONFIG_PATCH_HUNTERSUN)
		s_hfp_reg_sco++;
		App_UI_FsmEvent(hsc_HfpFsmBattery, NULL);
        HFP_HF_SetSpkVol(connection->profile_inst_handle, s_hfp_spk_vol);
		#endif
#ifdef CONFIG_MMI
		if (connection->role == APP_CONNECT_INITIATOR) {
			MMI_HfpConnectCfm(connection->bd, HCI_STATUS_OK, connection->connection_handle);
		} else {
			MMI_HfpConnectInd(connection->bd, connection->connection_handle);
		}
#endif
	}
	FREE(in);
}

/*******************************************************************
*																	*
********************************************************************/
void App_Hfp_FsmSlcReleaseInd(struct FsmInst *fi, UINT8 event, struct HFP_SLCConnectInfoStru *in)
{
	struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(in->server_handle);

        (void)fi;
        (void)event;
	debug("HFP-->App_Hfp_FsmSlcReleaseInd\r\n");
	if (connection != NULL) {
		if (s_hfp_reg_sco == 1) {
			HCI_Cbk_UnReg(sco_hci_table);
		}
		s_hfp_reg_sco--;
#ifdef CONFIG_MMI
		if (connection->state == APP_CONNECT_STATE_WAIT4_DISCONNECT) {
			MMI_HfpDisconnectCfm(HCI_STATUS_OK, connection->connection_handle);
		} else {
			MMI_HfpDisconnectInd(connection->connection_handle);
		}
#endif
		s_hfp_free_link_num++;
		//App_DeleteConnectionInst(connection);
	}
	FREE(in);
}

/*******************************************************************
*																	*
********************************************************************/
void App_Hfp_FsmSppDisconnectInd(struct FsmInst *fi, UINT8 event, struct HFP_ConnectEvStru *in)
{
	struct AppConnInst *connection = NULL;

        (void)fi;
        (void)event;
	debug("HFP-->App_Hfp_FsmSppDisconnectInd\r\n");
	if (in->handle != NULL) {
		connection = App_FindConnectionInstByLowerHandle(in->handle);
		//} else {
	}
    /* czy: if no handle, or found no connection by handle */
	if (connection == NULL) {
		/* This is confirmation to a previous connection request */
		connection = App_FindConnectionInstByBD(CLS_HANDSFREE_AG, in->bd, APP_CONNECT_INITIATOR);
	}
	if (connection != NULL) {
#ifdef CONFIG_MMI
		if (connection->role == APP_CONNECT_INITIATOR) {
			MMI_HfpConnectCfm(connection->bd, HCI_STATUS_UNSPECIFIED_ERROR, 0);
		}
#endif
		App_DeleteConnectionInst(connection);
		s_hfp_cur_conn_handle = NULL;
		s_hfp_call_state = BT_HFP_CALL_STATUS_STANDBY; // call hfp_disconnect when call ongoing. next paly a2dp no vol
		hs_cfg_sysCancelListenMsg(HS_CFG_EVENT_BATTERY_CHANGED, hsc_HFP_BatteryChanged);
	}
	if (s_hf_reconnect == 1) {
		App_HSP_ConnectAG(in->bd);
		s_hf_reconnect = 0;
	}
	FREE(in);
	//s_hfp_battery_handle = NULL;
}

/*******************************************************************
*																	*
********************************************************************/
void App_Hfp_FsmStateChangedInd(struct FsmInst *fi, UINT8 event, struct AppHfpNewState *in)
{
	struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(in->handle);

        (void)fi;
        (void)event;
	debug("hfp:call state change:%d, state=%d, conn=%x\n", in->event, s_hfp_call_state, connection);
	if (connection != NULL) {
		switch (in->event) {
		case HFP_EV_STANDBY_IND:
#if HS_USE_CONF
            if (s_hfp_call_state == BT_HFP_CALL_STATUS_ONGOING) {
                hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_CALL_ENDED);
                debug("hfp: call end\r\n");
            }
            else if (s_hfp_call_state == BT_HFP_CALL_STATUS_INCOMING) {
                hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_CALL_REJECTED);
                debug("hfp: call rejected\r\n");
            }
            else if (s_hfp_call_state == BT_HFP_CALL_STATUS_OUTGOING) {
                hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_CALL_ENDED);
                debug("hfp: call cancel\r\n");
            }
#endif /* HS_USE_CONF */
            s_hfp_call_state = BT_HFP_CALL_STATUS_STANDBY;
            s_hfp_incoming_call = 0;
#ifdef CONFIG_MMI
            MMI_HfpStandbyInd(connection->connection_handle);
#endif
            break;
		case HFP_EV_ONGOINGCALL_IND:
#if HS_USE_CONF
			hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_CALL_ACTIVE);
#endif
            s_hfp_call_state = BT_HFP_CALL_STATUS_ONGOING;
#ifdef CONFIG_MMI
			MMI_HfpOngoingCallInd(connection->connection_handle);
#endif
			break;
		case HFP_EV_OUTGOINGCALL_IND:
            s_hfp_call_state = BT_HFP_CALL_STATUS_OUTGOING;
#ifdef CONFIG_MMI
			MMI_HfpOutgoingCallInd(connection->connection_handle);
#endif
			break;
		case HFP_EV_INCOMINGCALL_IND:
            s_hfp_call_state = BT_HFP_CALL_STATUS_INCOMING;
#ifdef CONFIG_MMI
			MMI_HfpIncomingCallInd(connection->connection_handle);
#endif
			break;
		case HFP_EV_CALLHELD_IND:
#if HS_USE_CONF
			hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_CALL_HOLD);
#endif
            s_hfp_call_state = BT_HFP_CALL_STATUS_HELD;
#ifdef CONFIG_MMI
			MMI_HfpCallHeldInd(connection->connection_handle);
#endif
			break;
		}
        s_hfp_cur_conn_handle = in->handle;
	}
	FREE(in);
}

/*******************************************************************
*																	*
********************************************************************/
void App_Hfp_FsmCallWaitingInd(struct FsmInst *fi, UINT8 event, struct AppHfpWaitingCall *in)
{
	struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(in->handle);
     
        (void)fi;
        (void)event;

	if (connection != NULL) {
        s_hfp_call_state = BT_HFP_CALL_STATUS_WAIT;
        s_hfp_cur_conn_handle = in->handle;
#ifdef CONFIG_MMI
		MMI_HfpCallWaitingInd(connection->connection_handle, in->number);
#endif
	}
	FREE(in);
}

/*******************************************************************
*																	*
********************************************************************/
void App_Hfp_FsmDisconnect(struct FsmInst *fi, UINT8 event, UINT16 *in)
{
	struct AppConnInst *connection = NULL;

    (void)fi;
    (void)event;
    (void)in;
	//if (*in != 0) {
	connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);
	if (connection != NULL) {
		if ((connection->profile_inst_handle != NULL) && (connection->service_class == CLS_HANDSFREE_AG)) {
			connection->state = APP_CONNECT_STATE_WAIT4_DISCONNECT; /* Wait for Disconnect Complete Confirmation */
			HFP_Client_Stop(connection->profile_inst_handle);
			hs_cfg_sysCancelListenMsg(HS_CFG_EVENT_BATTERY_CHANGED, hsc_HFP_BatteryChanged);
		} else {
			connection = NULL; /* Unexpected disconnect request */
		}
	}
	//}
#ifdef CONFIG_MMI
	if (connection == NULL) {
		MMI_HfpDisconnectCfm(HCI_STATUS_UNKNOWN_CONNECTION_IDENTIFIER, *in);
	}
#endif
	if (in != NULL)
	{
		FREE(in);
	}
}

/*******************************************************************
*																	*
********************************************************************/
void App_Hfp_FsmAnswer(struct FsmInst *fi, UINT8 event, UINT16 *in)
{
	struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);

        (void)fi;
        (void)event;

	if (connection != NULL) {
		if (connection->profile_inst_handle != NULL) {
			HFP_HF_AnswerCall(connection->profile_inst_handle);
		}
	}
	if (in != NULL)
	{
		FREE(in);
	}
}

/*******************************************************************
*																	*
********************************************************************/
void App_Hfp_FsmReject(struct FsmInst *fi, UINT8 event, UINT16 *in)
{
	struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);

        (void)fi;
        (void)event;       
 
	if (connection != NULL) {
		if (connection->profile_inst_handle != NULL) {
			HFP_HF_CancelCall(connection->profile_inst_handle);
		}
	}
	if (in != NULL)
	{
		FREE(in);
	}
}

/*******************************************************************
*																	*
********************************************************************/
void App_Hfp_FsmDial(struct FsmInst *fi, UINT8 event, struct AppHfpDial *in)
{
	struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);

        (void)fi;
        (void)event;

	if (connection != NULL) {
		if (connection->profile_inst_handle != NULL) {
			if (in->number[0] != 0) {
				HFP_HF_Dial(connection->profile_inst_handle, in->number);
			} else {
				HFP_HF_LastNumRedial(connection->profile_inst_handle);
			}
		}
	}
	if (in != NULL)
	{
		FREE(in);
	}
}

/*******************************************************************
*																	*
********************************************************************/
void hsc_HfpStartMulti(UINT8 num)
{
   UINT32 sdp_hdl;
   struct HFP_ConnectParamStru param;
   /* Hands-free HF server */
   UINT32 features = HFP_BRSF_HF_3WAYCALL|HFP_BRSF_HF_CLIP|HFP_BRSF_HF_BVRA|HFP_BRSF_HF_RMTVOLCTRL|
	   HFP_BRSF_HF_ENHANCED_CALLSTATUS|HFP_BRSF_HF_ENHANCED_CALLCONTROL; //|HFP_BRSF_HF_CODEC_NEGOTIATION;
   s_hfp_multi_link_num = num;
   s_hfp_free_link_num  = num;
   if (num > 0)
      HFP_Server_Start(features, 1, &sdp_hdl, HFP_ROLE_HF_SERVER, s_hfp_multi_link_num, APP_HFEventCbk);

   #if HAL_USE_AUDIO
   //s_hfp_spk_vol = g_bt_host_config.attrs.hfpSpkVol;
   //hs_audio_volRestore(AVOL_DEV_HFP);
   s_hfp_spk_vol = app_hfp_ply_db2level(hs_audio_volGet(AVOL_DEV_HFP));
   s_hfp_mic_vol = g_bt_host_config.attrs.hfpMicVol;
   #endif

   s_hfp_use_pre_sco = ((g_bt_host_config.attrs.hfpScoConnCfgEnable>>1) & 0x01);
   param.enable = (g_bt_host_config.attrs.hfpScoConnCfgEnable & 0x01);
   param.retranseffort = g_bt_host_config.attrs.hfpScoScoConnCfgRetransEffort;
   param.latency = g_bt_host_config.attrs.hfpScoConnCfgMaxLatency;
   HFP_HF_SetDefaultLatencyAndRetrans(param);
}

void hsc_HspStartMulti(UINT8 num)
{
   UINT32 sdp_hdl;
  
   /* Handset HF server */
   UINT32 features = HFP_BRSF_HF_3WAYCALL|HFP_BRSF_HF_CLIP|HFP_BRSF_HF_BVRA|HFP_BRSF_HF_RMTVOLCTRL|
                     HFP_BRSF_HF_ENHANCED_CALLSTATUS|HFP_BRSF_HF_ENHANCED_CALLCONTROL;
   s_hsp_multi_link_num = num;
   s_hsp_free_link_num  = num;
   if (s_hsp_free_link_num>0)
      HFP_Server_Start(features, 1, &sdp_hdl, HFP_ROLE_HS_SERVER, s_hsp_multi_link_num, APP_HFEventCbk);
}
/*******************************************************************
*																	*
********************************************************************/
void App_HFP_ConnectAG(UINT8 *bd)
{
	UINT8 *param = NEW(BD_ADDR_LEN);
	memcpy(param, bd, BD_ADDR_LEN);
	App_UI_FsmEvent(App_Hfp_FsmConnectAG, param);

        if (s_hfp_multi_link_num > 0)
	   s_hf_reconnect = 1;
	else
           App_HSP_ConnectAG(bd);
}

void App_HSP_ConnectAG(UINT8 *bd)
{
	UINT8 *param = NEW(BD_ADDR_LEN);
	memcpy(param, bd, BD_ADDR_LEN);
	App_UI_FsmEvent(App_Hsp_FsmConnectAG, param);
}
/*******************************************************************
*																	*
********************************************************************/
void hsc_HFP_Disconnect(void)
{
	//UINT16 *in = NEW(sizeof(UINT16));
	//*in = connection_handle;
	App_UI_FsmEvent(App_Hfp_FsmDisconnect, NULL);
}

/*******************************************************************
*																	*
********************************************************************/
void hsc_HFP_Answer(void)  //(UINT16 connection_handle)
{
	//UINT16 *in = NEW(sizeof(UINT16));
	//*in = s_hfp_cur_conn_handle; //connection_handle;
	App_UI_FsmEvent(App_Hfp_FsmAnswer, NULL);

	debug("App_HFP_Answer\r\n");
}

/*******************************************************************
*																	*
********************************************************************/
void hsc_HFP_Reject(void) //(UINT16 connection_handle)
{
	//UINT16 *in = NEW(sizeof(UINT16));
	//*in = s_hfp_cur_conn_handle; //connection_handle;
	App_UI_FsmEvent(App_Hfp_FsmReject, NULL);

        debug("App_HFP_Reject\r\n");
}

/*******************************************************************
*																	*
********************************************************************/
void hsc_HFP_Redial(void) //(UINT16 connection_handle, UINT8 *number)
{
   struct AppHfpDial *in = NEW(sizeof(struct AppHfpDial));
   //in->connection_handle = s_hfp_cur_conn_handle; //connection_handle;
   //strcpy((char*)in->number, (char*)number);
   memset(in->number, 0, HFP_PHONENUM_MAX_DIGITS);
   App_UI_FsmEvent(App_Hfp_FsmDial, in);
   hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_REDIAL);
        debug("App_HFP_Dial\r\n");
}

/*******************************************************************
*																	*
********************************************************************/
void App_HFP_SendSCOPacket(UINT8 *data, UINT16 size)
{
	struct BuffStru *buf = BuffNew((size + 4), 0); /* 4 : HCI SCO Packet Header Length */

	/* SCO packet header */
	BUFDATA(buf)[0] = HCI_TLPKT_SCODATA; /* HCI SCO Packet Type */
	BUFDATA(buf)[1] = (UINT8)g_sco_handle;
	BUFDATA(buf)[2] = (UINT8)(g_sco_handle >> 8);
	BUFDATA(buf)[3] = size;
	/* PCM data */
	memcpy(BUFDATA(buf) + 4, data, size);
	HCI_SCOData_Send(buf, g_sco_handle);
}

UINT16 hsc_HFP_GetState(void)
{
   return s_hfp_call_state;
}

void hsc_HFP_VoiceRecognitionReq(void)
{
   struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);

   if (connection != NULL) 
   {
      if (connection->profile_inst_handle != NULL) 
      {
         HFP_HF_VoiceRecognitionReq(connection->profile_inst_handle, 1);

         debug("App_HFP_VoiceRecognitionReq\r\n");
      }
   }
}

void hsc_HFP_DialWithNum(char *number)
{
   struct AppHfpDial *in = NEW(sizeof(struct AppHfpDial));
   //in->connection_handle = s_hfp_cur_conn_handle;
   
   strcpy((char*)in->number, (char*)number);
   memset(in->number, 0, HFP_PHONENUM_MAX_DIGITS);
   App_UI_FsmEvent(App_Hfp_FsmDial, in);

        debug("App_HFP_DialWithNum\r\n");
}

void App_Hfp_FsmDialMem(struct FsmInst *fi, UINT8 event, struct AppHfpDial *in)
{
	struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);

        (void)fi;
        (void)event;

	if (connection != NULL && connection->profile_inst_handle != NULL && in->number[0] != 0) {
            HFP_HF_MemNumDial(connection->profile_inst_handle, in->number);
	}
	FREE(in);
}

void hsc_HFP_DialWithMemory(void)
{
   struct AppHfpDial *in = NEW(sizeof(struct AppHfpDial));
   //in->connection_handle = s_hfp_cur_conn_handle;
   
   // TODO , where the number
   //strcpy((char*)in->number, (char*)number);
   memset(in->number, 0, HFP_PHONENUM_MAX_DIGITS);
   App_UI_FsmEvent(App_Hfp_FsmDialMem, in);

        debug("App_HFP_DialWithMemory\r\n");
}

void hsc_HFP_DialAttachTag(void)
{
   struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);

   if (connection != NULL) 
   {
      if (connection->profile_inst_handle != NULL) 
      {
	 HFP_HF_VoiceTagPhoneNumReq(connection->profile_inst_handle);

         debug("App_HFP_DialAttachTag\r\n");
      }
   }
}

void hsc_HFP_GetManuModel(void)
{
   struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);

   if (connection != NULL) 
   {
      if (connection->profile_inst_handle != NULL) 
      {
	 HFP_HF_GetPhoneManuModel(connection->profile_inst_handle);

         debug(" App_HFP_GetManuModel\r\n");
      }
   }
}

void hsc_HFP_GetSubscriberNumber(void)
{
   struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);

   if (connection != NULL) 
   {
      if (connection->profile_inst_handle != NULL) 
      {
	 HFP_HF_GetSubscriberNumber(connection->profile_inst_handle);

         debug(" App_HFP_GetSubscriberNumbe\r\n");
      }
   }
}

void hsc_HFP_NetworkOperatorReq(void)
{
   struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);

   if (connection != NULL) 
   {
      if (connection->profile_inst_handle != NULL) 
      {
	 HFP_HF_NetworkOperatorReq(connection->profile_inst_handle);

         debug(" App_HFP_NetworkOperatorReq\r\n");
      }
   }
}

void hsc_HFP_GetCurrentCalls(void)
{
   struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);

   if (connection != NULL) 
   {
      if (connection->profile_inst_handle != NULL) 
      {
	 HFP_HF_GetCurrentCalls(connection->profile_inst_handle);

         debug(" App_HFP_GetCurrentCalls\r\n");
      }
   }
}

void hsc_HFP_DisableNREC(void)
{
   struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);

   if (connection != NULL) 
   {
      if (connection->profile_inst_handle != NULL) 
      {
	 HFP_HF_DisableNREC(connection->profile_inst_handle);

         debug(" hsc_HFP_DisableNREC\r\n");
      }
   }
}

void hsc_HFP_TxDTMF(char digit)
{
   struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);

   if (connection != NULL) 
   {
      if (connection->profile_inst_handle != NULL) 
      {
         HFP_HF_TxDTMF(connection->profile_inst_handle, digit);
         debug("hsc_HFP_TxDTMF\r\n");
      }
   }
}

void hsc_HFP_HoldIncomingCall(void)
{
   struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);

   if (connection != NULL) 
   {
      if (connection->profile_inst_handle != NULL) 
      {
         HFP_HF_HoldIncomingCall(connection->profile_inst_handle);
         debug("hsc_HFP_HoldIncomingCall\r\n");
      }
   }
}

void hsc_HFP_AcceptHeldIncomingCall(void)
{
   struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);

   if (connection != NULL) 
   {
      if (connection->profile_inst_handle != NULL) 
      {
         HFP_HF_AcceptHeldIncomingCall(connection->profile_inst_handle);
         debug("hsc_HFP_AcceptHeldIncomingCall\r\n");
      }
   }
}

void hsc_HFP_RejectHeldIncomingCall(void)
{
   struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);

   if (connection != NULL) 
   {
      if (connection->profile_inst_handle != NULL) 
      {
         HFP_HF_RejectHeldIncomingCall(connection->profile_inst_handle);
         debug("hsc_HFP_RejectHeldIncomingCall\r\n");
      }
   }
}

void hsc_HFP_3WayReleaseAllHeldCall(void)
{
   struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);

   if (connection != NULL) 
   {
      if (connection->profile_inst_handle != NULL) 
      {
         HFP_HF_3WayCallingHandler(connection->profile_inst_handle, 0, 0);
         debug("hsc_HFP_ReleaseAllHeldCall\r\n");
      }
   }
}

void hsc_HFP_3WayReleaseActiveCall(void)
{
   struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);

   if (connection != NULL) 
   {
      if (connection->profile_inst_handle != NULL) 
      {
         HFP_HF_3WayCallingHandler(connection->profile_inst_handle, 1, 0);
         debug("hsc_HFP_ReleaseActiveCall\r\n");
      }
   }
}

void hsc_HFP_3WayHoldActiveCall(void)
{
   struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);

   if (connection != NULL) 
   {
      if (connection->profile_inst_handle != NULL) 
      {
         HFP_HF_3WayCallingHandler(connection->profile_inst_handle, 2, 0);
         debug("hsc_HFP_HoldActiveCall\r\n");
      }
   }
}

void hsc_HFP_3WayAddHeldCall(void)
{
   struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);

   if (connection != NULL) 
   {
      if (connection->profile_inst_handle != NULL) 
      {
         HFP_HF_3WayCallingHandler(connection->profile_inst_handle, 3, 0);
         debug("hsc_HFP_AddHeldCall\r\n");
      }
   }
}

void hsc_HFP_3WayEct(void)
{
   struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);

   if (connection != NULL) 
   {
      if (connection->profile_inst_handle != NULL) 
      {
         HFP_HF_3WayCallingHandler(connection->profile_inst_handle, 4, 0);
         debug("hsc_HFP_3WayEct\r\n");
      }
   }
}

void hsc_HFP_SaveQuickNum(void)
{
   struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);

   if (connection != NULL) 
   {
      if (connection->profile_inst_handle != NULL) 
      {
         debug("hsc_HFP_SaveQuickNum\r\n");
      }
   }
}

void hsc_HFP_ExtendedATCmd(char *cmd)
{
   struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);

   if (connection != NULL) 
   {
      if (connection->profile_inst_handle != NULL) 
      {
         HFP_ExtendCmd(connection->profile_inst_handle, cmd, strlen(cmd), 6000);
         debug("hsc_HFP_ExtendedATCmd\r\n");
      }
   }
}

UINT8* hsc_HFP_GetCurCallNum(void)
{
	return s_hfp_cur_call_number;
}

UINT8* hsc_HFP_GetLastCallNum(void)
{
	return s_hfp_last_call_number;
}

void hsc_HFP_DisconnectAll(void)
{
	hsc_HFP_Disconnect();
}

void App_Hfp_FsmSco(struct FsmInst *fi, UINT8 event, UINT16 *in)
{
	struct AppConnInst *connection = NULL;

        (void)fi;
        (void)event;
#if 0
	if (*in != 0) {
		connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);
		if (connection != NULL) {
			if ((connection->profile_inst_handle != NULL) && (connection->service_class == CLS_HANDSFREE_AG)) {
				connection->state = APP_CONNECT_STATE_WAIT4_DISCONNECT; /* Wait for Disconnect Complete Confirmation */
				HFP_HF_AudioConnTrans(connection->profile_inst_handle);
			} else {
				connection = NULL; /* Unexpected disconnect request */
			}
		}
	}
	if (connection == NULL) {
		HFP_HF_AudioConnTrans(NULL);
	}
#else
    connection = App_FindConnectionInstByLowerHandle(s_hfp_cur_conn_handle);
    if (connection != NULL && connection->profile_inst_handle != NULL)
    {
        hs_printf("sco 1\r\n");
        HFP_HF_AudioConnTrans(connection->profile_inst_handle);
    }
    else
    {
        hs_printf("sco 2\r\n");
        HFP_HF_AudioConnTrans(NULL);
    }
#endif
	if (in != NULL)
	{
		FREE(in);
	}
}

void hsc_HFP_SCOHandle(void)
{
	//UINT16 *in = NEW(sizeof(UINT16));
	//*in = s_hfp_cur_conn_handle;
	App_UI_FsmEvent(App_Hfp_FsmSco, NULL);
}

UINT8 hsc_HfpGetFreeLinkCount(void)
{
	return s_hfp_free_link_num;
}

void hsc_HfpSetLocalMicVol(void)
{
#if HAL_USE_AUDIO
  //hs_printf("hsc_HfpSetLocalVol \r\n");
  //hsc_HfpSetSpkVol(s_hfp_spk_vol);
  hsc_HfpSetMicVol(s_hfp_mic_vol);
#endif
}

UINT8 hsc_HFp_GetRingType(void)
{
    return s_hfp_incoming_call_ring;
}
#endif
