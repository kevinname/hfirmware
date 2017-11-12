#ifndef _BTHOST_UAPI_H
#define _BTHOST_UAPI_H

#include <stdint.h>

#if defined __cplusplus
extern "C" {
#endif

//#include "app_tl.h"
#define HCI_TLPKT_COMMAND          				0x01
#define HCI_TLPKT_ACLDATA          				0x02
#define HCI_TLPKT_SCODATA          				0x03
#define HCI_TLPKT_EVENT            				0x04

#define APP_CFG_PAIR_INFO_MAX    6

#ifndef BD_ADDR_LEN
/**
 * @brief Length of a bluetooth device address.
 */
#define BD_ADDR_LEN 6
#endif
typedef uint8_t bd_addr_t[BD_ADDR_LEN];

#define BONDED_DEVICE_MAX_COUNT   6
/*
 * pair list
 */
#ifndef LINK_KEY_LEN
/**
 * @brief link key and its type
 */
#define LINK_KEY_LEN 16
#define LINK_KEY_STR_LEN (LINK_KEY_LEN*2)
typedef uint8_t link_key_t[LINK_KEY_LEN]; 

typedef enum {
	COMBINATION_KEY = 0,	// standard pairing
	LOCAL_UNIT_KEY,			// ?
	REMOTE_UNIT_KEY,		// ?
	DEBUG_COMBINATION_KEY,	// SSP with debug
	UNAUTHENTICATED_COMBINATION_KEY_GENERATED_FROM_P192, // SSP Simple Pairing
	AUTHENTICATED_COMBINATION_KEY_GENERATED_FROM_P192,	 // SSP Passkey, Number confirm, OOB
	CHANGED_COMBINATION_KEY,							 // Link key changed using Change Connection Lnk Key
	UNAUTHENTICATED_COMBINATION_KEY_GENERATED_FROM_P256, // SSP Simpe Pairing
	AUTHENTICATED_COMBINATION_KEY_GENERATED_FROM_P256,   // SSP Passkey, Number confirm, OOB
} link_key_type_t;
#endif

/* uapi version of struct AppDevInst */
__PACKED struct hs_pair_entry{
  bd_addr_t bd_addr;
  link_key_t link_key;
  uint8_t link_key_type;
}__PACKED_GCC;

typedef struct hs_pair_entry hs_pair_entry_t;

/* Classic Device Info */
#define BT_NAME_LEN_MAX                16
#define BT_PIN_CODE_LEN                4
#define BT_CLASS_OF_DEVICE_LEN         3

struct AppDeviceInfoStru {
  uint8_t   addr[BD_ADDR_LEN];
  uint8_t   name[BT_NAME_LEN_MAX];
  uint8_t   pincode[BT_PIN_CODE_LEN];
  uint8_t   cod[BT_CLASS_OF_DEVICE_LEN]; // class of device
};

/* Classic Profiles */
#define BT_CLASSIC_PROFILE_GAP        0x0001
#define BT_CLASSIC_PROFILE_A2DP       0x0002
#define BT_CLASSIC_PROFILE_HFP        0x0004
#define BT_CLASSIC_PROFILE_SPP        0x0008
#define BT_CLASSIC_PROFILE_HID        0x0010
#define BT_CLASSIC_PROFILE_OBEX       0x0020
#define BT_CLASSIC_PROFILE_PBAP       0x0040
#define BT_CLASSIC_PROFILE_MAP        0x0080
#define BT_CLASSIC_PROFILE_OPP        0x0100
#define BT_CLASSIC_PROFILE_AVRCP      0x0200
__PACKED struct AppClassicProfileStru {
  uint8_t a2dp; // 0:no a2dp profile; >=1 the a2dp connect number, max is 2
  uint8_t avrcp; // 0: no avrcp profile; 1: the device is CT; 2 the device is TG; 3: the device is CT&TG
  uint8_t hfp;  // 0: no hfp profile; >=1 the hfp connect number, max is 2;
  uint8_t hsp;  // 0: no hsp profile; >=1 the hsp connect number, max is 2;
  uint8_t spp;  // 0: no spp profile; >=1 the spp connect number, max is 2;
  uint8_t hid;  // 0: no hid profile: 1: is pointer; 2: is mouse; 4: Joystick; 8:Game pad; 16:keybpard; 32:keypad
                // hid usage table , with the big for different usage
  uint8_t obex;
  uint8_t pbap;
  uint8_t map;
  uint8_t opp;
} __PACKED_GCC;

/* Classic Profile Attribution */
__PACKED struct AppProfileAttrStru {
  uint16_t sppMaxMtu;
  uint8_t  gapIoCap;
  uint8_t  hfpStereoEnable;
  uint8_t  hfpSpkVol;
  uint8_t  hfpMicVol;
  uint8_t  a2dpSpkVol;
  uint16_t connectAlert;
  uint8_t  hfpScoConnCfgEnable;
  uint8_t  hfpScoScoConnCfgRetransEffort;
  uint16_t hfpScoConnCfgMaxLatency;
} __PACKED_GCC;

/* Classic Advanced features */
__PACKED struct AppBtFeaturesStru {
  // link lost alert
  uint8_t   linkLostAlertEnable;
  uint8_t   linkLostAlertLevel; 
  uint16_t  linkLostAlertStopTimer; // default 3s
  
  // link lost retry
  uint8_t   linkLostRetryEnable;
  uint8_t   linkLostRetryCount;
  uint16_t  linkLostRetryTimer; // default 10s
  
  // power on
  uint8_t   powerOnAutoConnect;
  uint8_t   powerOnAutoConnectProtect;
  uint16_t  powerOnAutoConnectTimer; // default 15s
  uint16_t  powerOnStartConnectTimer; // defalut 3s
  
  // pair
  uint16_t pairTimeOut; // default 180s
  uint8_t  linkLostIntoPairEnable;
  
  // PTS
  uint8_t ptsTestEnable; /* bit2-DUT bit1-MPS bit0-PTS */
  
  // sniff for host
  uint16_t sniffEnable;
  uint16_t sniffIdleInterval; // ms
  uint16_t sniffBusyInterval; // ms
  uint16_t sniffSuspendTimeout; // ms
  // sniff for hci
  uint16_t sniffMaxInterval; // 0x0006 to 0x0540
  uint16_t sniffMinInterval; // 0x0006 to 0x0540
  uint16_t sniffAttempt;     // 0x0001 to 0x7FFF
  uint16_t sniffTimeout;     // 0x0000 to 0x7FFF
  
  // sniff subrating for host
  uint16_t sniffToSubrateTimeout;
  uint16_t sniffSubrateInterval;
  
  // sniff subrating for hci
  uint16_t sniffSubrateMaxLatency; // default Tsniff, 0x0002 to 0xFFFE
  uint16_t sniffSubrateMiniRemoteTimeout; // default 0, 0x0000 to 0xFFFE
  uint16_t sniffSubrateMiniLocalTimeout; // default 0,  0x0000 to 0xFFFE
} __PACKED_GCC;

/* HID */
#define BT_HID_NAME_LEN_MAX            30

__PACKED struct AppBtHidAttrStru {
  uint8_t  subClass;
  uint8_t  countryCode;
  uint8_t  reportId;
  uint8_t  rsv;
  uint16_t mask;
  uint16_t releaseNum;
  uint16_t parserVersion;
  uint16_t profileVersion;
  uint16_t supervisionTimeout;
  uint16_t maxLatency;
  uint16_t minTimeout;
  uint16_t nameLen;
  uint8_t  serviceName[BT_HID_NAME_LEN_MAX];
  uint16_t nameDescLen;
  uint8_t  nameDesc[BT_HID_NAME_LEN_MAX];
  uint16_t proNameLen;
  uint8_t  proName[BT_HID_NAME_LEN_MAX];
  uint16_t descLen;
} __PACKED_GCC;

__PACKED struct AppBtHidInfoStru {
    uint8_t   addr[BD_ADDR_LEN];
    uint8_t   name[BT_NAME_LEN_MAX];
    hs_pair_entry_t remote;
} __PACKED_GCC;
/*
 * bluetooth host configuration data, which comes from flash storage
 */
struct bt_host_config_s {
  struct AppDeviceInfoStru device;
  struct AppClassicProfileStru profiles;
  struct AppProfileAttrStru attrs;
  struct AppBtFeaturesStru features;
};
typedef struct bt_host_config_s bt_host_config_t;

/*
 * bluetooth host status inside
 */
struct bt_host_status_s {
  uint32_t lpc_sco_pkt_tx_miss_cnt;
  uint8_t  gap_inquiry_result_cnt;
  uint8_t  vhci_host2hc_busy_cnt;
};
typedef struct bt_host_status_s bt_host_status_t;

void bt_host_get_config(bt_host_config_t **pp_host_cfg);
void bt_host_get_status(bt_host_status_t **pp_host_sts);

/* Bluetooth Event */
typedef enum
{
   // bluetooth manager
  APP_CFG_EVENT_ENTER_PAIR                     = 0x00,               /*!<00.  Enter Pairing                  */
  APP_CFG_EVENT_RESET_PAIR_LIST                      ,               /*!<01.  Rest Pairing List              */

  // bluetooth hfp
  APP_CFG_EVENT_HFP_CONENCT_BACK                     ,               /*!<02.  Connect Back                          */
  APP_CFG_EVENT_HFP_ANSWER                           ,               /*!<03.  Accept the Incoming Call              */
  APP_CFG_EVENT_HFP_REJECT                           ,               /*!<04.  Reject the Incoming Call              */
  APP_CFG_EVENT_HFP_CACEL                            ,               /*!<05.  Terminate the Ongoing Call            */
  APP_CFG_EVENT_HFP_LAST_REDIAL                      ,               /*!<06.  Last number re-dial from hf           */        
  APP_CFG_EVENT_HFP_CALL_WITH_NUMBER                 ,               /*!<07.  Originate a Call with memeory         */
  APP_CFG_EVENT_HFP_MEMORY_CALL                      ,               /*!<08.  Originate a Memory Call               */
  APP_CFG_EVENT_HFP_ATTACH_NUMBER_TO_TAG             ,               /*!<09.  Attach number to vocie tag            */
  APP_CFG_EVENT_HFP_VOICE_RECOGNITION                ,               /*!<0a.  Activate/Deactivate Voice Recognition */
  APP_CFG_EVENT_HFP_SAVE_QUICK_NUM                   ,               /*!<0b.  Save Quick Number                     */
  APP_CFG_EVENT_HFP_CALL_ON_HOLD                     ,               /*!<0c.  Place Incoming Call on Hold           */
  APP_CFG_EVENT_HFP_ACCEPT_HOLD                      ,               /*!<0d.  Accept Held Incoming Call             */
  APP_CFG_EVENT_HFP_REJECT_HOLD                      ,               /*!<0e.  Reject Held Incoming Call             */
  APP_CFG_EVENT_HFP_TX_DTMF                          ,               /*!<0f.  Make AG Transmit DTMF Code to GSM Network */
  APP_CFG_EVENT_HFP_VOL_ADD                          ,               /*!<10.  Speaker Volume ADD                    */
  APP_CFG_EVENT_HFP_VOL_SUB                          ,               /*!<11.  Speaker Volume SUB                    */
  APP_CFG_EVENT_HFP_MIC_ADD                          ,               /*!<12.  MIC Volume ADD                        */
  APP_CFG_EVENT_HFP_MIC_SUB                          ,               /*!<13.  MIC Volume SUB                        */
  APP_CFG_EVENT_HFP_REQUEST_MODEL_ID                 ,               /*!<14.  Request manufacturer and model ID     */
  APP_CFG_EVENT_HFP_REQUEST_CALL_LIST                ,               /*!<15.  Request Current Call List             */
  APP_CFG_EVENT_HFP_REQUEST_SUB_INFO                 ,               /*!<16.  Request Subscriber Info               */
  APP_CFG_EVENT_HFP_REQUSET_CUR_NET_NAME             ,               /*!<17.  Request Current Network Operator Name */
  APP_CFG_EVENT_HFP_DISABLE_AG_EC_NR                 ,               /*!<18.  Disable the EC and NR on AG           */
  APP_CFG_EVENT_HFP_3WAY_RELEASE_ALL                 ,               /*!<19.  3 Way Release All held                */
  APP_CFG_EVENT_HFP_3WAY_ACCEPT_WAIT_RELEASE         ,               /*!<1a.  3 Way Accecpt Waiting Releas          */
  APP_CFG_EVENT_HFP_3WAY_ACCEPT_WAIT_HOLD            ,               /*!<1b.  3 Way Accept Waiting Held             */
  APP_CFG_EVENT_HFP_3WAY_ADD_HOLD                    ,               /*!<1c.  3 Way Add Held to 3 way               */
  APP_CFG_EVENT_HFP_3WAY_ECT                         ,               /*!<1d.  3 Way ECT                             */
  APP_CFG_EVENT_HFP_EXTENDED_AT_CMD                  ,               /*!<1e.  Extended AT Command                   */
  APP_CFG_EVENT_HFP_DISCONNECT                       ,               /*!<1f.  Disconnect                            */
  APP_CFG_EVENT_HFP_SCO                              ,               /*!<20.  sco connect or disconnect             */

  // bluetooth a2dp
  APP_CFG_EVENT_A2DP_CONNECT_BACK                    ,               /*!<21.  Connect Back                          */
  APP_CFG_EVENT_A2DP_SOURCE_SELECT                   ,               /*!<22.  Source select                         */
  APP_CFG_EVENT_A2DP_VOL_ADD                         ,               /*!<23.  VOL ADD                               */
  APP_CFG_EVENT_A2DP_VOL_SUB                         ,               /*!<24.  VOL SUB                               */
  APP_CFG_EVENT_A2DP_PLAY                            ,               /*!<25.  PLAY                                  */
  APP_CFG_EVENT_A2DP_PAUSE                           ,               /*!<26.  PAUSE                                 */
  APP_CFG_EVENT_A2DP_STOP                            ,               /*!<27.  STOP                                  */
  APP_CFG_EVENT_A2DP_MUTE                            ,               /*!<28.  MUTE                                  */
  APP_CFG_EVENT_A2DP_FORWORD                         ,               /*!<29.  Forward                               */
  APP_CFG_EVENT_A2DP_BACKWORD                        ,               /*!<2a.  Backword                              */
  APP_CFG_EVENT_A2DP_FFWD_PRESS                      ,               /*!<2b.  FFWD: fast forward Press              */
  APP_CFG_EVENT_A2DP_FFWD_RELEASE                    ,               /*!<2c.  FFWD Release                          */
  APP_CFG_EVENT_A2DP_RWD_PRESS                       ,               /*!<2d.  RWD: rewind Press                     */
  APP_CFG_EVENT_A2DP_RWD_RELEASE                     ,               /*!<2e.  RWD Release                           */
  APP_CFG_EVENT_A2DP_STREAM_PLAY                     ,               /*!<2f.  stream play                           */
  APP_CFG_EVENT_A2DP_STREAM_PAUSE                    ,               /*!<30.  stream pause                          */
  APP_CFG_EVENT_A2DP_STREAM_GET_CONFIG               ,               /*!<31.  stream get config                     */
  APP_CFG_EVENT_A2DP_STREAM_RECONFIG                 ,               /*!<32.  stream reconfig                       */
  APP_CFG_EVENT_A2DP_STREAM_DISCONNECT               ,               /*!<33.  a2dp strean disconnect                */
  APP_CFG_EVENT_A2DP_STREAM_ABORT                    ,               /*!<34.  a2dp strean abort                     */
  APP_CFG_EVENT_A2DP_DISCONNECT                      ,               /*!<35.  a2dp disconnect                       */

  // bluetooth spp
  APP_CFG_EVENT_SPP_CONNECT                          ,               /*!<36.  Spp Connect                           */
  APP_CFG_EVENT_SPP_DISCONNECT                       ,               /*!<37.  Spp disconnect                           */

  // bluetooth hid
  APP_CFG_EVENT_HID_CONNECT                          ,               /*!<38.  HID Connect                           */
  APP_CFG_EVENT_HID_DISCONNECT                       ,               /*!<39.  HID disconnec                         */
  APP_CFG_EVENT_HID_SEND_KEY1                        ,               /*!<3a.  HID send key                          */
  APP_CFG_EVENT_HID_SEND_KEY2                        ,               /*!<3b.  HID send key2                         */
  APP_CFG_EVENT_HID_SEND_KEY3                        ,               /*!<3c.  HID send key                          */
  APP_CFG_EVENT_HID_SEND_KEY4                        ,               /*!<3d.  HID send key2                         */
  
  // avrcp reconnect
  APP_CFG_EVENT_AVRCP_RECONNECT                      ,               /*!<3e.  AVRCP reconnect                       */
  APP_CFG_EVENT_AVRCP_PLAY                           ,               /*!<3f.  AVRCP play                            */
  APP_CFG_EVENT_AVRCP_PAUSE                          ,               /*!<40.  AVRCP pause                           */
  APP_CFG_EVENT_AVRCP_STOP                           ,               /*!<41.  AVRCP stop                            */
  APP_CFG_EVENT_AVRCP_REWIND                         ,               /*!<42.  AVRCP rewind                          */
  APP_CFG_EVENT_AVRCP_FORWARD                        ,               /*!<43.  AVRCP forward                         */
  APP_CFG_EVENT_AVRCP_BACKWARD                       ,               /*!<44.  AVRCP backward                        */
  APP_CFG_EVENT_AVRCP_FFWD                           ,               /*!<45.  AVRCP fast forward                    */
  APP_CFG_EVENT_AVRCP_VOLUP                          ,               /*!<46.  AVRCP vol up                          */
  APP_CFG_EVENT_AVRCP_VOLDOWN                        ,               /*!<47.  AVRCP vol down                        */
  // all reconnect
  APP_CFG_EVENT_RECONNECT                            ,               /*!<48.  reconnect all profiles                */
  // disconnect all
  APP_CFG_EVENT_DISCONNECT_ALL                       ,               /*!<49.  disconnect all profiles               */
  APP_BT_VOL_ADD                                     ,               /*!<4a,  vol add                               */
  APP_BT_VOL_SUB                                     ,               /*!<4b,  vol sub                               */
  // max
  APP_CFG_EVENT_MAX                                  ,
}APP_CFG_EVENT_E;

__PACKED struct AppBtActionStru {
  uint8_t eventIndex;
  uint8_t  actionIndex;
  uint16_t eventProfiles;
  uint16_t profileStatus;
} __PACKED_GCC;

/*
 * FSM UI
 */
#define APP_BT_STATE_NULL                 0x0001
#define APP_BT_STATE_READY                0x0002
#define APP_BT_STATE_PAIRMODE             0x0004
#define APP_BT_STATE_PAIRED               0x0008
#define APP_BT_STATE_CONNECTED            0x0010
#define APP_BT_STATE_CONNECTED_IDLE       0x0010  // idle
#define APP_BT_STATE_CONNECTED_CALLING    0x0020  // hfp
#define APP_BT_STATE_CONNECTED_STREAMING  0x0040  // a2dp
#define APP_BT_STATE_CONNECTED_TRANSPORT  0x0080  // spp
#define APP_BT_STATE_CONNECTED_CONTROL    0x0100  // hid
#define APP_BT_STATE_CONNECTED_PBAP_SYNC  0x0200  // pbap
#define APP_BT_STATE_CONNECTED_MAP_SYNC   0x0400  // map
#define APP_BT_STATE_CONNECTED_OPP_SYNC   0x0800  // opp
#define APP_BT_STATE_TESTMODE             0x1000
uint16_t App_GetBtState(void);

void APP_PairEntry(void);
void APP_PairResetList(void);
uint8_t App_GetPairList(uint8_t *res_buf, uint8_t u8GetCnt);
uint8_t App_GetConnectList(uint8_t *res_buf, uint8_t u8GetCnt);

void App_FsmReconnect(void);
void App_FsmDisconnectAll(void);
void App_FsmAutoConnect(void);

void App_FsmA2dpReconnect(void);
void App_FsmHfpReconnect(void);
void App_FsmAvrcpReconnect(void);
void App_FsmHidReconnect(void);
void App_FsmSppReconnect(void);

/*
 * HFP UI
 */
#define BT_HFP_CALL_STATUS_STANDBY     0x0001
#define BT_HFP_CALL_STATUS_CONNECT     0x0002
#define BT_HFP_CALL_STATUS_ONGOING     0x0004
#define BT_HFP_CALL_STATUS_OUTGOING    0x0008
#define BT_HFP_CALL_STATUS_INCOMING    0x0010
#define BT_HFP_CALL_STATUS_HELD        0x0020
#define BT_HFP_CALL_STATUS_WAIT        0x0040
#define BT_HFP_CALL_STATUS_3WAY_HOLD   0x0080
#define BT_HFP_CALL_STATUS_3WAY_MT     0x0100
uint16_t hsc_HFP_GetState(void);

void hsc_HFP_Answer(void);
void hsc_HFP_Reject(void);
void hsc_HFP_Redial(void);
void hsc_HFP_DialWithNum(char *number);
void hsc_HFP_DialAttachTag(void);
void hsc_HFP_VoiceRecognitionReq(void);
void hsc_HFP_SaveQuickNum(void); /* TBD */
void hsc_HFP_HoldIncomingCall(void);
void hsc_HFP_AcceptHeldIncomingCall(void);
void hsc_HFP_RejectHeldIncomingCall(void);
void hsc_HFP_TxDTMF(char digit);

// vol control
void hsc_HFP_SpkVolAdd(void);
void hsc_HFP_SpkVolSub(void);
void hsc_HFP_MicVolAdd(void);
void hsc_HFP_MicVolSub(void);
void hsc_HFP_SpkVolChange(int db);

// TBD
void hsc_HFP_GetManuModel(void);
void hsc_HFP_GetCurrentCalls(void);
void hsc_HFP_GetSubscriberNumber(void);
void hsc_HFP_NetworkOperatorReq(void);

void hsc_HFP_DisableNREC(void);
void hsc_HFP_3WayReleaseAllHeldCall(void);
void hsc_HFP_3WayReleaseActiveCall(void);
void hsc_HFP_3WayHoldActiveCall(void);
void hsc_HFP_3WayAddHeldCall(void);
void hsc_HFP_3WayEct(void); // (Explicit Call Transfer)

void hsc_HFP_ExtendedATCmd(char *cmd);
void hsc_HFP_DisconnectAll(void);

// pts
void hsc_HFP_SCOHandle(void);

/*
 * A2DP UI
 */
#define APP_A2DP_STATE_IDLE   0x0001
#define APP_A2DP_STATE_OPEN   0x0002
#define APP_A2DP_STATE_START  0x0004
#define APP_A2DP_STATE_PAUSE  0x0008
#define APP_A2DP_STATE_CLOSE  0x0010
uint16_t hsc_A2DP_GetState(void);

void hsc_A2DP_Play(void);
void hsc_A2DP_Pause(void);
void hsc_A2DP_Stop(void);
void hsc_A2DP_Mute(void);
void hsc_A2DP_SpkVolAdd(void);
void hsc_A2DP_SpkVolSub(void);
void hsc_A2DP_SpkVolChange(int db);
void hsc_A2DP_Forward(void);
void hsc_A2DP_Backward(void);
void hsc_A2DP_FFWDPress(void);
void hsc_A2DP_FFWDRelease(void);
void hsc_A2DP_RewindPress(void);
void hsc_A2DP_RewindRelease(void);
void hsc_A2DP_SourceSelect(void); //send Select via AVRCP?
void hsc_A2DP_DisconnectAll(void);

void hsc_A2DP_StreamPlay(void);
void hsc_A2DP_StreamStop(void);
void hsc_A2DP_StreamPause(void);

// pts
void hsc_A2DP_StreamGetConfig(void);
void hsc_A2DP_StreamReConfig(void);
void hsc_A2DP_StreamDisconnect(void);
void hsc_A2DP_StreamAbort(void);

/*
 * AVRCP UI
 */
void hsc_AVRCP_DisconnectAll(void);

// pts
void hsc_AVRCP_Play(void);
void hsc_AVRCP_Pause(void);
void hsc_AVRCP_Stop(void);
void hsc_AVRCP_Rewind(void);
void hsc_AVRCP_Forward(void);
void hsc_AVRCP_Backward(void);
void hsc_AVRCP_FFWD(void);
void hsc_AVRCP_VolUp(void);
void hsc_AVRCP_VolDown(void);

/*
 * SPP UI
 */
#define APP_SPP_NOT_CONNECT  0x0000
#define APP_SPP_CONNECTED    0x0001
#define APP_SPP_IDLE         0x0001
#define APP_SPP_BUSY         0x0002
uint16_t App_SPP_GetState(void);

/*
 * HID UI
 */
#define APP_HID_STATE_IDLE             0x0001
#define APP_HID_STATE_CONNECTED        0x0002
#define APP_HID_STATE_BUSY             0x0004
#define APP_HID_STATE_NOT_CONNECT      0x0008
uint16_t App_HID_GetState(void);

void App_HID_SendKey1(void);
void App_HID_SendKey2(void);
void App_HID_SendKey3(void);
void App_HID_SendKey4(void);

void hsc_BtVolAdd(void);
void hsc_BtVolSub(void);
void hsc_BtVolChange(int db);
#if defined __cplusplus
}
#endif

#endif
