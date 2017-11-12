/*---------------------------------------------------------------------------
Description:
	A2DP Sample.
---------------------------------------------------------------------------*/
#include "app_main.h"

#ifdef CONFIG_A2DP

extern cnt_t osMessageGetUseCount(osMessageQId queue_id);

/* A2DP routines */
enum {/* mask in struct A2DPAPP_SEPStru */
	APP_A2DP_SEPMASK_ISSRCSVC = 				0x10,
	APP_A2DP_SEPMASK_ISSNKSVC = 				0x20
};

#if defined(CONFIG_PATCH_HUNTERSUN)

extern osMessageQId app_thread_a2dp_msgid;

static UINT32 m_pts_last_tick = 0;
static UINT8  m_pts_test_enable = 0;
static UINT32 m_ptsmps = 0;

static HANDLE m_audio_stream_handle = NULL;
static UINT8 s_a2dp_multi_link_num = 1;
static UINT8 s_a2dp_free_link_num  = 1;

static INT32 s_a2dp_sample_rate     = I2S_SAMPLE_44P1K;
static INT32 s_a2dp_sample_channels = I2S_PCMMODE_STEREO;

static UINT16 s_a2dp_play_state = APP_A2DP_STATE_IDLE;
static HANDLE s_a2dp_stream_handle = NULL;
static struct A2DP_SetConfigStru s_a2dp_stream_config;

#define HS_A2DP_PLAY_THRESHOLD_LEVEL (BT_I2S_A2DP_PLY_BLOCK_SIZE/4*BT_I2S_A2DP_PLY_BLOCK_NUM-240)
//#define HS_A2DP_PLAY_THRESHOLD_LEVEL (BT_I2S_A2DP_PLY_BLOCK_SIZE/8*BT_I2S_A2DP_PLY_BLOCK_NUM)

#ifndef APP_A2DP_DATA_USE_FULL_CPY
#define APP_A2DP_CONSEAL_DATA_LEN 4096
static const UINT8 s_a2dp_Conseal_data[APP_A2DP_CONSEAL_DATA_LEN] = {0};
#endif

static UINT8 s_a2dp_play_queue_num = 0;
static UINT16 s_a2dp_play_queue_buffer = 0;
static UINT8  s_a2dp_data_check_count = 0;
static void hsc_A2dpStreamPauseByHandle(HANDLE stream_handle);
static void hsc_A2dpStreamStartByHandle(HANDLE stream_handle);
static void hsc_A2dpStreamSuspendByHandle(HANDLE stream_handle);

void hsc_A2DP_ClearThreshold(void)
{
    s_a2dp_play_queue_num = 0;
    s_a2dp_play_queue_buffer = 0;
}

UINT16 hsc_A2DP_GetState(void)
{
  return s_a2dp_play_state;
}

void hsc_A2DP_Play(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_a2dp_stream_handle);

  if (App_CFG_GetPtsEnable())
    A2DP_StreamStart(s_a2dp_stream_handle);

  if (connection == NULL)
    return;

  App_AVRCP_Play_Pushed(connection->bd);
  App_AVRCP_Play_Released(connection->bd);
  s_a2dp_play_state = APP_A2DP_STATE_START;
  hsc_AvrcpSetLocalVol();
  #if HAL_USE_AUDIO
  audioPlayUnmute();
  #endif
  debug("%s\r\n", __FUNCTION__);
}

void hsc_A2DP_Stop(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_a2dp_stream_handle);

  if (App_CFG_GetPtsEnable())
    A2DP_StreamSuspend(s_a2dp_stream_handle);

  if (connection == NULL)
    return;

  App_AVRCP_Stop_Pushed(connection->bd);
  App_AVRCP_Stop_Released(connection->bd);
  s_a2dp_play_state = APP_A2DP_STATE_IDLE;
  #if HAL_USE_AUDIO
  audioPlayMute();
  #endif
  debug("%s\r\n", __FUNCTION__);
}

void hsc_A2DP_Pause(void)
{
  hsc_A2dpStreamPauseByHandle(s_a2dp_stream_handle);

  s_a2dp_play_state = APP_A2DP_STATE_PAUSE;
  #if HAL_USE_AUDIO
  audioPlayMute();
  #endif
  //hsc_A2DP_ClearThreshold();
  debug("%s\r\n", __FUNCTION__);
}

void hsc_A2DP_Mute(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_a2dp_stream_handle);

  if (connection == NULL)
    return;

  App_AVRCP_Mute_Pushed(connection->bd);
  App_AVRCP_Mute_Released(connection->bd);
  debug("%s\r\n", __FUNCTION__);
}

void hsc_A2DP_SpkVolAdd(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_a2dp_stream_handle);

  if (connection == NULL)
    return;

  hsc_AvrcpChangeVol(connection->bd, TRUE);
  debug("%s\r\n", __FUNCTION__);
}

void hsc_A2DP_SpkVolSub(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_a2dp_stream_handle);

  if (connection == NULL)
    return;

  hsc_AvrcpChangeVol(connection->bd, FALSE);
  debug("%s\r\n", __FUNCTION__);
}

void hsc_A2DP_SpkVolChange(int db)
{
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_a2dp_stream_handle);

  if (connection == NULL)
    return;

  hsc_AvrcpVolChange(connection->bd, db);
}

void hsc_A2DP_Forward(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_a2dp_stream_handle);

  if (connection == NULL)
    return;

  App_AVRCP_Forward_Pushed(connection->bd);
  App_AVRCP_Forward_Released(connection->bd);
  debug("%s\r\n", __FUNCTION__);
}

void hsc_A2DP_Backward(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_a2dp_stream_handle);

  if (connection == NULL)
    return;

  App_AVRCP_Backward_Pushed(connection->bd);
  App_AVRCP_Backward_Released(connection->bd);
  debug("%s\r\n", __FUNCTION__);
}

void hsc_A2DP_FFWDPress(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_a2dp_stream_handle);

  if (connection == NULL)
    return;

  App_AVRCP_FFWD_Pushed(connection->bd);
  debug("%s\r\n", __FUNCTION__);
}

void hsc_A2DP_FFWDRelease(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_a2dp_stream_handle);

  if (connection == NULL)
    return;

  App_AVRCP_FFWD_Released(connection->bd);
  debug("%s\r\n", __FUNCTION__);
}

void hsc_A2DP_RewindPress(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_a2dp_stream_handle);

  if (connection == NULL)
    return;

  App_AVRCP_Rewind_Pushed(connection->bd);
  debug("%s\r\n", __FUNCTION__);
}

void hsc_A2DP_RewindRelease(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_a2dp_stream_handle);

  if (connection == NULL)
    return;

  App_AVRCP_Rewind_Released(connection->bd);
  debug("%s\r\n", __FUNCTION__);
}

void hsc_A2DP_SourceSelect(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_a2dp_stream_handle);

  if (connection == NULL)
    return;

  App_AVRCP_Select_Pushed(connection->bd);
  App_AVRCP_Select_Released(connection->bd);
  debug("%s\r\n", __FUNCTION__);
}

void hsc_A2DP_DisconnectAll(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_a2dp_stream_handle);

  if (connection == NULL)
    return;

  App_A2DP_Disconnect(connection->connection_handle);
  debug("%s\r\n", __FUNCTION__);
}

void hsc_A2DP_StreamPlay(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_a2dp_stream_handle);

  if (connection == NULL)
    return;

  A2DP_StreamStart(s_a2dp_stream_handle);
  s_a2dp_play_state = APP_A2DP_STATE_START;
  #if HAL_USE_AUDIO
  audioPlayUnmute();
  #endif
  debug("%s\r\n", __FUNCTION__);
}

void hsc_A2DP_StreamStop(void)
{
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_a2dp_stream_handle);

  if (connection == NULL)
    return;

  A2DP_StreamSuspend(s_a2dp_stream_handle);
  s_a2dp_play_state = APP_A2DP_STATE_PAUSE;
  #if HAL_USE_AUDIO
  audioPlayMute();
  #endif
  //hsc_A2DP_ClearThreshold();
  debug("%s\r\n", __FUNCTION__);
}

void hsc_A2DP_StreamPause(void)
{   
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(s_a2dp_stream_handle);

  if (connection == NULL)
    return;

  A2DP_StreamSuspend(s_a2dp_stream_handle);
  s_a2dp_play_state = APP_A2DP_STATE_PAUSE;
  #if HAL_USE_AUDIO
  audioPlayMute();
  #endif
  //hsc_A2DP_ClearThreshold();
  debug("%s\r\n", __FUNCTION__);
}

void hsc_A2DP_StreamGetConfig(void)
{
  A2DP_GetConfiguration(s_a2dp_stream_handle);
}

void hsc_A2DP_StreamReConfig(void)
{
  A2DP_StreamReconfigure(&s_a2dp_stream_config);
}

void hsc_A2DP_StreamDisconnect(void)
{
  A2DP_Disconnect(s_a2dp_stream_handle);
}

void hsc_A2DP_StreamAbort(void)
{
  AVDTP_StreamAbort_ReqEx(s_a2dp_stream_handle);
}


void hsc_A2dpStartMulti(UINT8 num)
{
  if (num > 0) {
    s_a2dp_multi_link_num = num;
    s_a2dp_free_link_num = num;
    App_A2DP_Start();
  }
}

UINT8 hsc_A2dpGetFreeLinkCount(void)
{
  return s_a2dp_free_link_num;
}

/*
 * called by AVRCP to update A2DP state
 */
void hsc_A2dpSetState(UINT8 *bd, UINT16 state)
{
  struct AppConnInst *connection = App_FindConnectionInstByBD(CLS_AUDIO_SOURCE, bd, APP_CONNECT_IGNORE_ROLE);

  debug("A2DP_SetState: %x:%x:%x:%x:%x:%x, state=%d\r\n", bd[5],bd[4],bd[3],bd[2],bd[1],bd[0],state);
  if (connection != NULL) {
    if (state == APP_A2DP_STATE_START)
      hsc_A2dpStreamStartByHandle(connection->profile_inst_handle);
    else if (state == APP_A2DP_STATE_PAUSE)
      hsc_A2dpStreamSuspendByHandle(connection->profile_inst_handle);
  }
}

void hsc_A2dpDisableAudioStreamHandle(void)
{
  m_audio_stream_handle = NULL;
}

void hsc_A2dpEnableAudioStreamHandle(void)
{
  m_audio_stream_handle = s_a2dp_stream_handle;
}

/*
 * called in A2DP_CodecDecode() to skip SBC decoder if
 * HFP's SCO connection is established;
 * interrupted by tone indicator;
 * interrupted by another A2DP stream;
 */
BOOL hsc_A2dpIsSkip(HANDLE stream_handle)
{
  if ((m_audio_stream_handle == NULL) ||
      (m_audio_stream_handle != stream_handle))
    return TRUE;

  if (m_pts_test_enable) {
    if (0 == m_pts_last_tick)
      m_pts_last_tick = GetCurrTime();

    if ((GetCurrTime() - m_pts_last_tick) >= 10) {
      m_pts_last_tick = GetCurrTime();
    }
    else {
      /* skip decoder if PTS tester DoS attack */
      return TRUE;
    }
  }

  return FALSE;
}

static void hsc_A2dpStreamPauseByHandle(HANDLE stream_handle)
{   
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(stream_handle);

  debug("%s h=%08lx\r\n", __FUNCTION__, stream_handle);
  if (App_CFG_GetPtsEnable())
    A2DP_StreamSuspend(stream_handle);

  if (connection == NULL)
    return;

  App_AVRCP_Pause_Pushed(connection->bd);
  App_AVRCP_Pause_Released(connection->bd);
}

static void hsc_A2dpStreamStartByHandle(HANDLE stream_handle)
{
  struct AppConnInst *connection;
  debug("StreamStart: h1=%08lx,h2=%08lx,state=%d\r\n",s_a2dp_stream_handle, stream_handle, s_a2dp_play_state);
  if (s_a2dp_stream_handle != NULL && stream_handle != NULL && s_a2dp_stream_handle != stream_handle && s_a2dp_play_state == APP_A2DP_STATE_START) 
  {
    hsc_A2dpStreamPauseByHandle(stream_handle); // prev play, next pause. only the first play
    // App_A2DP_Pause(); // next play, prev pause, only the next play
    return;
  }

  if (stream_handle != NULL)
    m_audio_stream_handle = stream_handle;

  connection = App_FindConnectionInstByLowerHandle(stream_handle);
  if (connection)
    hsc_AvrcpSwitchLocalVol(connection->bd);

  s_a2dp_stream_handle = stream_handle;
  //if (s_a2dp_play_state != APP_A2DP_STATE_START)
  {
    s_a2dp_play_state = APP_A2DP_STATE_START;
    #if HAL_USE_AUDIO
    /* don't disturb current tone indicator? */
	//app_AudioStart(s_a2dp_sample_rate, s_a2dp_sample_channels, -1/*1024 inside*/);
    audioPlayUnmute();
    #endif
    #if HS_USE_CONF
    hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_MUSIC_STREAMING);
    #endif
  }
}

static void hsc_A2dpStreamSuspendByHandle(HANDLE stream_handle)
{
  debug("StreamSuspend: h1=%08lx,h2=%08lx,state=%d\r\n",s_a2dp_stream_handle, stream_handle, s_a2dp_play_state);
  if (stream_handle == s_a2dp_stream_handle && s_a2dp_play_state == APP_A2DP_STATE_START) 
  {
    s_a2dp_play_state = APP_A2DP_STATE_PAUSE;
    #ifdef CONFIG_HFP
    if (hsc_HFP_GetState() <= BT_HFP_CALL_STATUS_CONNECT) 
    {
      #if HAL_USE_AUDIO
      audioPlayMute();
      hsc_A2dpDisableAudioStreamHandle();
      App_AudioStop();
      //hsc_A2DP_ClearThreshold(); 
      #endif
      #if HS_USE_CONF
      hs_cfg_sysSendMessage(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, HS_CFG_EVENT_BT_MUSIC_PAUSED);
      #endif
    }
    #endif
  }
}

static void hsc_A2dpStreamStartInd(struct A2DP_StartSuspendIndStru* in)
{
  if (in != NULL)
    hsc_A2dpStreamStartByHandle(in->stream_handle);
  else
    hsc_A2dpStreamStartByHandle(NULL);   
}

static void hsc_A2dpStreamSuspendInd(struct A2DP_StartSuspendIndStru* in)
{
  if (in != NULL)
    hsc_A2dpStreamSuspendByHandle(in->stream_handle);
  else
    hsc_A2dpStreamSuspendByHandle(NULL);   
}

static void hsc_A2dpOpenCodecInfo(struct A2DP_OpenCodecInfoIndStru *in)
{
#if HAL_USE_AUDIO
  UINT8 src;

  if (s_a2dp_play_state == APP_A2DP_STATE_CLOSE)
    s_a2dp_play_state = APP_A2DP_STATE_IDLE;

  if (in == NULL)
    return;

  src = in->info[0];
  if (src & A2DP_SBC_SF_16000) {
    s_a2dp_sample_rate = I2S_SAMPLE_16K;
  } else if (src & A2DP_SBC_SF_32000) {
    s_a2dp_sample_rate = I2S_SAMPLE_32K;
  } else if (src & A2DP_SBC_SF_44100) {
    s_a2dp_sample_rate = I2S_SAMPLE_44P1K;
  } else if (src & A2DP_SBC_SF_48000) {
    s_a2dp_sample_rate = I2S_SAMPLE_48K;
  } else {
    s_a2dp_sample_rate = I2S_SAMPLE_44P1K;
  }

  if (src & A2DP_SBC_CHMODE_MONO) {
    s_a2dp_sample_channels = I2S_PCMMODE_MONO;
  }
  else {
    s_a2dp_sample_channels = I2S_PCMMODE_STEREO;
  }

  src = in->info[1];
  if (src & A2DP_SBC_BLOCK_12) {
    debug("not support 12-bit sample width\r\n");
  }

  debug("sample rate %d, channel %d\r\n", s_a2dp_sample_rate, s_a2dp_sample_channels);
  //App_AudioSetSample(s_a2dp_sample_rate);
  //App_AudioSetMode(s_a2dp_sample_channels);
#endif
}

void hsc_A2dpHandleData(UINT16 len, UINT8 *buf)
{
#if HAL_USE_AUDIO
#ifndef APP_A2DP_DATA_USE_FULL_CPY
  struct A2DP_Stream_DataIndStru *in;
  UINT16 sbc_len;
  UINT8 *sbc_buf;
  if (len > 1)
  {
    in = (struct A2DP_Stream_DataIndStru *)buf;
    sbc_len = len;
    sbc_buf = BUFDATA(in->data);
  }
  else
  {
    sbc_len = APP_A2DP_CONSEAL_DATA_LEN;
    //memset(s_a2dp_Conseal_data, 0, APP_A2DP_CONSEAL_DATA_LEN);
    sbc_buf = (UINT8*)s_a2dp_Conseal_data;
  }
#else
  UINT16 sbc_len = len;
  UINT8 *sbc_buf = buf;
#endif
  /* pause play SBC streaming if HFP call is active */
  #ifdef CONFIG_HFP
  if (hsc_HFP_GetState() <= BT_HFP_CALL_STATUS_CONNECT
    && hsc_AvrcpGetSpkVol() > 0
    ) {
  #endif

  if (hsc_A2DP_GetState() == APP_A2DP_STATE_START)
  {
   App_SetBtState2(s_a2dp_stream_handle, APP_BT_STATE_CONNECTED_STREAMING);
   
   App_AudioStart(s_a2dp_sample_rate, s_a2dp_sample_channels, HS_A2DP_PLAY_THRESHOLD_LEVEL);

    if (App_AudioPlayLocalStop() == 0) {
      UINT8 *audio_buf;
      INT32 temp_len;
      //UINT32 audio_level = 0;
      if (s_a2dp_play_queue_num == 0)
      {
        if (s_a2dp_play_queue_buffer + sbc_len < HS_A2DP_PLAY_THRESHOLD_LEVEL*4)
        {
            s_a2dp_play_queue_buffer += sbc_len;
        }
        else
        {
            #ifdef APP_A2DP_DATA_USE_FULL_CPY
            while(osMessageGetUseCount(app_thread_a2dp_msgid)<4)
            {
                msleep(10);
            }
            #else
            while(osMessageGetUseCount(app_thread_a2dp_msgid)<5)
            {
                msleep(10);
            }
            #endif
            //hs_printf("messagecount=%d\r\n", osMessageGetUseCount(app_thread_a2dp_msgid));
            s_a2dp_play_queue_buffer = 0;
            s_a2dp_play_queue_num = 1;
        }
      }
      //hs_printf("-%d,%d,", audioGetBufferLevel()*4, osMessageGetUseCount(app_thread_a2dp_msgid));
      while (sbc_len >0) {
        if (audioGetBufferLevel() == 0) {
            HS_UART1->THR='X';
            msleep(1);
        }
        /* 3584KB/44.1khz/4B = ~20ms */
        temp_len = audioPlyGetDataBuffer(&audio_buf, sbc_len, MS2ST(20));
        //memcpy(audio_buf, sbc_buf, len);
        audioPlyCpyData(audio_buf, sbc_buf, temp_len);
        audioPlySendDataDone(audio_buf, temp_len);
        sbc_len -= temp_len;
        sbc_buf += temp_len;
        
        if (temp_len == 0 || App_AudioPlayLocalStop() == 1) {
          debug("X");
          break;
        }
      };
      //msleep(1);
      //hs_printf("%d,", audioGetBufferLevel()*4);
    }
  }
  #ifdef CONFIG_HFP
  }
  #endif
  #ifndef APP_A2DP_DATA_USE_FULL_CPY
  if (len > 1)
  {
    FREE(in->data);
    FREE(in);
  }
  #endif

#endif /* HAL_USE_AUDIO */
}

void hsc_A2DP_DataLostPacketConseal(UINT16 len, UINT8 *data)
{
    UINT16 data_sum = 0;
    UINT8 j=0;
    cnt_t message_count =0;
    
    if (data[0] != 0 || len <10 || audioGetBufferLevel()==0)
    {
        s_a2dp_data_check_count = 0;
        return;
    }

    // the data must all 0.
    for(j=0;j<10;j++)
    {
        data_sum += data[j];
    }
    for(j=100;j>110;j--)
    {
        data_sum += data[j];
    }
    for(j=len-1;j>len-10;j--)
    {
        data_sum += data[j];
    }
    if (data_sum != 0)
    {
        s_a2dp_data_check_count = 0;
        return;
    }

    //hs_printf("%d,%d-", osMessageGetUseCount(app_thread_a2dp_msgid), audioGetBufferLevel()*4);
    //return;
    //hs_printf("#%d,", s_a2dp_data_check_count);
    // Between the two songs, not the beginning of the first song
    s_a2dp_data_check_count++;
    if (s_a2dp_data_check_count > 5)
    {
        message_count = osMessageGetUseCount(app_thread_a2dp_msgid);
        hs_printf("*%d,", message_count);
        if (message_count < 3 && audioGetBufferLevel() < BT_I2S_A2DP_PLY_BLOCK_SIZE/4*BT_I2S_A2DP_PLY_BLOCK_NUM-1300)
        {
            if (message_count == 0)
            {
                App_AudioStop();
                hs_printf("a2dp Conseal stop\r\n");
            }
            else
            {
                #ifdef APP_A2DP_DATA_USE_FULL_CPY
                hsc_A2dpThradSendDataMessage(len, data);
                #else
                hsc_A2dpThradSendDataMessage(1, NULL);
                #endif
            }
        }
        #if 1
        else if (s_a2dp_play_queue_num == 1)
        {
            //HS_UART1->THR='&';
            #ifdef APP_A2DP_DATA_USE_FULL_CPY
            if (message_count < 3)
                hsc_A2dpThradSendDataMessage(len, data);
            #else
            // Conseal to count 3.
            switch(message_count)
            {
                case 0:
                    hsc_A2dpThradSendDataMessage(1, NULL);
                case 1:
                    hsc_A2dpThradSendDataMessage(1, NULL);
                case 2:
                    hsc_A2dpThradSendDataMessage(1, NULL);
                //case 3:
                //    hsc_A2dpThradSendDataMessage(1, NULL);
                    break;
                default:
                    break;
            }
            #endif
        }
        #endif
        s_a2dp_data_check_count = 0;
    }
}
#endif /* #if defined(CONFIG_PATCH_HUNTERSUN) */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_A2dpStreamDataInd(struct A2DP_Stream_DataIndStru *in)
{
#if defined(CONFIG_PATCH_HUNTERSUN)
  if (App_CFG_GetPtsMpsEnable() > 0) {
    if (m_ptsmps == App_CFG_GetPtsMpsEnable()) {
      hsc_AVRCP_Pause();
    }
    m_ptsmps++;
  }
#if HAL_USE_AUDIO
  /* pause play SBC streaming if HFP call is active */
  #ifdef CONFIG_HFP
  if (hsc_HFP_GetState() <= BT_HFP_CALL_STATUS_CONNECT
    && hsc_AvrcpGetSpkVol() > 0
    ) {
  #endif

    if (hsc_A2DP_GetState() == APP_A2DP_STATE_START)
    {
        App_SetBtState2(s_a2dp_stream_handle, APP_BT_STATE_CONNECTED_STREAMING);
       
        App_AudioStart(s_a2dp_sample_rate, s_a2dp_sample_channels, HS_A2DP_PLAY_THRESHOLD_LEVEL);
        
        if (App_AudioPlayLocalStop() == 0) {
            //hs_printf("+");
            //hsc_A2dpThradSendDataMessage((UINT8*)in);
            //return;
            #ifdef APP_A2DP_DATA_USE_FULL_CPY
            
            UINT16 sbc_len = in->UsedDataLen;
            UINT8 *sbc_buf = BUFDATA(in->data);
            hsc_A2dpThradSendDataMessage(sbc_len, sbc_buf);
            hsc_A2DP_DataLostPacketConseal(sbc_len, sbc_buf);
            if (s_a2dp_play_queue_num == 0) msleep(5);
            
            #else
            UINT16 sbc_len = in->UsedDataLen;
            UINT8 *sbc_buf = BUFDATA(in->data);
            hsc_A2dpThradSendDataMessage(sbc_len, (UINT8*)in);
            hsc_A2DP_DataLostPacketConseal(sbc_len, sbc_buf);
            if (s_a2dp_play_queue_num == 0) msleep(5);
            return;
            #endif
        }
    }
  #ifdef CONFIG_HFP
  }
  #endif
#endif /* HAL_USE_AUDIO */
  
#else
  /* TODO: Play the voice data */
#endif
  FREE(in->data);
  FREE(in);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_A2dpIndCbk(UINT8 event, void *param)
{
  //debug("A2DP: EVENT=%d\r\n", event);

  switch (event) {
  case A2DP_EV_REGSEP: /* param: struct A2DP_SEIDStru */
    FREE(param);
    break;
  case A2DP_EV_STREAMCREATE: /* param: struct AVDTP_StreamCreateStru */
    App_SCH_FsmEvent(App_A2dp_FsmStreamCreateInd, param);
    break;

  case A2DP_EV_SETCONFIGURATION: /* param: struct A2DP_SetConfigStru */
#if defined(CONFIG_PATCH_HUNTERSUN)
    /* PTS: saved for App_A2DP_ReConfig() */
    memcpy(&s_a2dp_stream_config, param, sizeof(s_a2dp_stream_config));
#endif
    FREE(param);
    break;

  case A2DP_EV_GETCONFIGURATION: /* param: struct A2DP_ServiceCapStru */
    FREE(param);
    break;

  case A2DP_EV_OPENCODECINFO: /* param: struct A2DP_OpenCodecInfoIndStru */
#if defined(CONFIG_PATCH_HUNTERSUN)
    hsc_A2dpOpenCodecInfo(param);
#endif
    FREE(param);
    break;

  case A2DP_EV_STREAMOPEN: /* param: struct A2DP_ReadyToStartStru */
    App_SCH_FsmEvent(App_A2dp_FsmStreamOpenInd, param);
    break;
  case A2DP_EV_STREAMSTART: /* param: struct A2DP_StartSuspendIndStru */
#if defined(CONFIG_PATCH_HUNTERSUN)
    hsc_A2dpStreamStartInd(param);
#endif
    FREE(param);
    break;

  case A2DP_EV_STREAMSUSPEND: /* param: struct A2DP_StartSuspendIndStru */
#if defined(CONFIG_PATCH_HUNTERSUN)
    hsc_A2dpStreamSuspendInd(param);
#endif
    FREE(param);
    break;

  case A2DP_EV_DATAIND: /* param: struct A2DP_Stream_DataIndStru */
    App_A2dpStreamDataInd(param);
    break;

  case A2DP_EV_STREAMDONE: /* param: struct A2DP_Stream_DoneStru */
    App_SCH_FsmEvent(App_A2dp_FsmStreamDoneInd, param);
    break;
  default:
    FREE(param);
    break;
  }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_A2dp_FsmStreamCreateInd(struct FsmInst *fi, UINT8 event, struct AVDTP_StreamCreateStru *in)
{
  (void)fi;
  (void)event;
  struct AppConnInst *connection = App_FindConnectionInstByBD(CLS_AUDIO_SOURCE, in->remote_bd, APP_CONNECT_IGNORE_ROLE);

  if (NULL == connection) {
    connection = App_AddConnectionInst(CLS_AUDIO_SOURCE, in->remote_bd, APP_CONNECT_RESPONDER);
#if defined(CONFIG_PATCH_HUNTERSUN)
    s_a2dp_free_link_num--;
    if (s_a2dp_play_state != APP_A2DP_STATE_START) {
      s_a2dp_stream_handle = in->stream_handle;
    }
    debug("StreamCreate: free_link=%d stream_handle=%08lx state=%d\r\n", s_a2dp_free_link_num, s_a2dp_stream_handle, s_a2dp_play_state);
#endif
  }
  /* Now the connection procedure can be canceled */
  connection->profile_inst_handle = in->stream_handle;
  FREE(in);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_A2dp_FsmStreamOpenInd(struct FsmInst *fi, UINT8 event, struct A2DP_OpenedStru *in)
{
  (void)fi;
  (void)event;
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(in->stream_handle);

  if (connection) {
#if defined(CONFIG_PATCH_HUNTERSUN)
    hsc_UpdateConnectionInst(connection, APP_CONNECT_STATE_CONNECTED);
#endif

#ifdef CONFIG_MMI
    if (connection->role == APP_CONNECT_INITIATOR) {
      MMI_A2dpConnectCfm(connection->bd, HCI_STATUS_OK, connection->connection_handle);
    } else {
      MMI_A2dpConnectInd(connection->bd, connection->connection_handle);
    }
#endif

#if defined(CONFIG_PATCH_HUNTERSUN)
    //hs_printf("StreamOpen: hOld=%08lx, hNew=%08lx, conn_h=%d, state=%d\r\n", 
    //      s_a2dp_stream_handle, in->stream_handle, connection->connection_handle, s_a2dp_play_state);
    if (s_a2dp_play_state != APP_A2DP_STATE_START) {
      s_a2dp_stream_handle = in->stream_handle;
      s_a2dp_play_state = APP_A2DP_STATE_OPEN;
      hsc_A2DP_ClearThreshold(); 
    }
#endif
  }
  FREE(in);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_A2dp_FsmStreamDoneInd(struct FsmInst *fi, UINT8 event, struct A2DP_Stream_DoneStru *in)
{
  (void)fi;
  (void)event;
  struct AppConnInst *connection = App_FindConnectionInstByLowerHandle(in->stream_handle);
   
  if (connection) {
#ifdef CONFIG_MMI
    if (connection->state == APP_CONNECT_STATE_WAIT4_DISCONNECT) {
      MMI_A2dpDisconnectCfm(HCI_STATUS_OK, connection->connection_handle);
    } else {
      MMI_A2dpDisconnectInd(connection->connection_handle);
    }
#endif

#if defined(CONFIG_PATCH_HUNTERSUN)
    debug("StreamDone\r\n");
    if (connection->profile_inst_handle == s_a2dp_stream_handle)
      s_a2dp_stream_handle = NULL;
 
    s_a2dp_free_link_num++;
    if (s_a2dp_multi_link_num == s_a2dp_free_link_num) {
      App_SetBtState2(NULL, APP_BT_STATE_CONNECTED_IDLE);
      #if HAL_USE_AUDIO
      App_AudioStop();
      #endif
      s_a2dp_play_state = APP_A2DP_STATE_IDLE;
      s_a2dp_stream_handle = NULL;
    }
#endif

    App_DeleteConnectionInst(connection);
  }
  FREE(in);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_A2dp_FsmConnect(struct FsmInst *fi, UINT8 event, struct A2DP_ConnectReqStru *in)
{
  (void)fi;
  (void)event;
  struct AppConnInst *connection = App_FindConnectionInstByBD(CLS_AUDIO_SOURCE, in->bd, APP_CONNECT_IGNORE_ROLE);

  if (NULL == connection) {
#if defined(CONFIG_PATCH_HUNTERSUN)
    s_a2dp_free_link_num--;
#endif
    App_AddConnectionInst(CLS_AUDIO_SOURCE, in->bd, APP_CONNECT_INITIATOR);
    A2DP_Connect(in);	
  } else {
#ifdef CONFIG_MMI
    if (connection->profile_inst_handle == NULL) {
      /* Only one A2DP connection is allowed between two devices */
      MMI_A2dpConnectCfm(in->bd, HCI_STATUS_CONNECTION_LIMIT_EXCEEDED, 0);
    } else {
      /* Return the current connection handle */
      MMI_A2dpConnectCfm(in->bd, HCI_STATUS_OK, connection->connection_handle);
    }
#endif    
    List_RemoveAll(&in->perfer);
    FREE(in);
  }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_A2dp_FsmDisconnect(struct FsmInst *fi, UINT8 event, UINT16 *in)
{
  (void)fi;
  (void)event;
  struct AppConnInst *connection = NULL;

  if (*in != 0) {
    connection = App_FindConnectionInstByUpperHandle(*in);
    if (connection != NULL) {
      if ((connection->profile_inst_handle != NULL) && (connection->service_class == CLS_AUDIO_SOURCE)) {
        connection->state = APP_CONNECT_STATE_WAIT4_DISCONNECT; /* Wait for Disconnect Complete Confirmation */
        A2DP_Disconnect(connection->profile_inst_handle);
      } else {
        connection = NULL; /* Unexpected disconnect request */
      }
    }
  }
  if (connection == NULL) {
#ifdef CONFIG_MMI
    MMI_A2dpDisconnectCfm(HCI_STATUS_UNKNOWN_CONNECTION_IDENTIFIER, *in);
#endif
#if defined(CONFIG_PATCH_HUNTERSUN)
    if (s_a2dp_multi_link_num == s_a2dp_free_link_num) {
      App_SetBtState2(NULL, APP_BT_STATE_CONNECTED_IDLE);
      s_a2dp_play_state = APP_A2DP_STATE_IDLE;
      s_a2dp_stream_handle = NULL;
    }
#endif
  }
  FREE(in);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_A2DP_RegSnkService(void)
{
  struct A2DP_SEPStru *sep = NULL;
  UINT8 len = 10;
#if defined(CONFIG_PATCH_HUNTERSUN)
  UINT8 index = 0;
#endif

  A2DP_RegisterService(A2DP_TSEP_SNK, A2DP_PLAYER);

#if defined(CONFIG_PATCH_HUNTERSUN)
  for ( index = 0; index < s_a2dp_multi_link_num; index++) {
#endif
    sep = List_NodeNew(sizeof(struct A2DP_SEPStru) + len);
    if (sep == NULL)
      break;
    memset(sep, 0, sizeof(struct A2DP_SEPStru) + len);
    sep->mask = APP_A2DP_SEPMASK_ISSNKSVC;
    sep->media_type = (A2DP_MEDIATYPE_AUDIO << 4) | (A2DP_TSEP_SNK << 3);
    sep->sc.len = len;
    sep->sc.service_capability[0] = A2DP_SC_MEDIA_TRANSPORT;
    sep->sc.service_capability[1] = 0;
    sep->sc.service_capability[2] = A2DP_SC_MEDIA_CODEC;
    sep->sc.service_capability[3] = 6;
    sep->sc.service_capability[4] = A2DP_MEDIATYPE_AUDIO << 4;
    sep->sc.service_capability[5] = A2DP_CODEC_SBC;
    sep->sc.service_capability[6] = A2DP_SBC_SF_44100 | A2DP_SBC_SF_48000 | A2DP_SBC_CHMODE_ALL;
    sep->sc.service_capability[7] = A2DP_SBC_BLOCK_ALL | A2DP_SBC_SUBBAND_ALL | A2DP_SBC_ALLOCATION_ALL;
    sep->sc.service_capability[8] = 2;
#if defined(CONFIG_PATCH_HUNTERSUN)
    if (App_CFG_GetPtsEnable() == 1) {
      sep->sc.service_capability[9] = 54;
      m_pts_test_enable = 1;
    }
    else {
      sep->sc.service_capability[9] = 32;
      m_pts_test_enable = 0;
    }
#else
    sep->sc.service_capability[9] = 32;
#endif
    A2DP_Register_SEP(sep);
#if defined(CONFIG_PATCH_HUNTERSUN)
  }
#endif
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_A2DP_Start(void)
{
  struct A2DP_RegCbkStru *in;
	
  in = NEW(sizeof(struct A2DP_RegCbkStru));
  in->media_type = A2DP_MEDIATYPE_AUDIO;
  in->cbk = App_A2dpIndCbk;
  A2DP_RegCbk(in);

  App_A2DP_RegSnkService();
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_A2DP_Connect(UINT8 *bd)
{
  struct A2DP_ConnectReqStru *in;
  struct A2DP_ServiceCapStru *sc;
  UINT8 *p;
  UINT8 len;
	
  in = NEW(sizeof(struct A2DP_ConnectReqStru));
  memset(in, 0, sizeof(struct A2DP_ConnectReqStru));
  memcpy(in->bd, bd, BD_ADDR_LEN);
	
  len = 10;
  sc = List_NodeNew(sizeof(struct A2DP_ServiceCapStru) + len);
  sc->len = len;
  p = sc->service_capability;
  p[0] = A2DP_SC_MEDIA_TRANSPORT;
  p[1] = 0;
  p[2] = A2DP_SC_MEDIA_CODEC;
  p[3] = 6;
  p[4] = A2DP_MEDIATYPE_AUDIO << 4;
  p[5] = A2DP_CODEC_SBC;
  p[6] = A2DP_SBC_SF_44100 | A2DP_SBC_CHMODE_STEREO;
  p[7] = A2DP_SBC_BLOCK_16 | A2DP_SBC_SUBBAND_8 | A2DP_SBC_LOUDNESS;
  p[8] = 2;
  p[9] = 33;
  List_AddTail(&in->perfer, sc);
	
  sc = List_NodeNew(sizeof(struct A2DP_ServiceCapStru) + len);
  sc->len = len;
  p = sc->service_capability;
  p[0] = A2DP_SC_MEDIA_TRANSPORT;
  p[1] = 0;
  p[2] = A2DP_SC_MEDIA_CODEC;
  p[3] = 6;
  p[4] = A2DP_MEDIATYPE_AUDIO << 4;
  p[5] = A2DP_CODEC_SBC;
  p[6] = A2DP_SBC_SF_48000 | A2DP_SBC_CHMODE_STEREO;
  p[7] = A2DP_SBC_BLOCK_16 | A2DP_SBC_SUBBAND_8 | A2DP_SBC_LOUDNESS;
  p[8] = 2;
  p[9] = 33;
  List_AddTail(&in->perfer, sc);

  App_UI_FsmEvent(App_A2dp_FsmConnect, in);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
------------------------------------------------------------------------*/
void App_A2DP_Disconnect(UINT16 connection_handle)
{
  UINT16 *in = NEW(sizeof(UINT16));
  *in = connection_handle;
  App_UI_FsmEvent(App_A2dp_FsmDisconnect, in);
}

#endif
