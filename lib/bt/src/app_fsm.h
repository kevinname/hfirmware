#ifndef _APP_FSM_H
#define _APP_FSM_H

#define APP_CONNECT_INITIATOR	1
#define APP_CONNECT_RESPONDER	2
#define APP_CONNECT_IGNORE_ROLE	3

#define APP_CONNECT_STATE_CONNECTED			1
#define APP_CONNECT_STATE_WAIT4_DISCONNECT	2

// bt hfp thread event
#define APP_BT_HFP_EVENT_SLEEP            0x01
#define APP_BT_HFP_EVENT_WAKEUP           0x02
#define APP_BT_HFP_EVENT_DATA             0x04
#define APP_BT_HFP_EVENT_EXIT             0x80

// bt a2dp play
#define BT_I2S_A2DP_PLY_BLOCK_SIZE        512 //4096//2400
#define BT_I2S_A2DP_PLY_BLOCK_NUM         48//6

#define BT_I2S_HFP_PLY_BLOCK_SIZE         1200
#define BT_I2S_HFP_PLY_BLOCK_NUM          16
#define BT_I2S_HFP_PLY_START_TH           BT_I2S_HFP_PLY_BLOCK_SIZE
#define BT_I2S_HFP_PLY_BLOCK_TIME         (BT_I2S_HFP_PLY_BLOCK_SIZE/8/2+5)

#define BT_I2S_REC_BLOCK_SIZE             1200
#define BT_I2S_REC_BLOCK_NUM              3
#define BT_I2S_REC_BLOCK_TIME             (BT_I2S_REC_BLOCK_SIZE/8/2+5)

/* APP Device Instance */
struct AppDevInst {
	UINT8 bd[BD_ADDR_LEN];
	UINT8 link_key[LINKKEYLENGTH];
	UINT8 key_type;
};

/* APP Connection Instance */
struct AppConnInst {
	void   *profile_inst_handle;
	UINT16 connection_handle;
	UINT16 service_class;			/* Peer Service Class */
	UINT8  bd[BD_ADDR_LEN];
	UINT8  role;					/* APP_CONNECT_INITIATOR, APP_CONNECT_RESPONDER */
	UINT8  state;					/* APP_CONNECT_STATE_WAIT4_DISCONNECT */
};

struct AppPowerModeStru {
	struct HCI_AddressStru addr;		/* Address of the device to which the connection event related */
	UINT16                 acl_hdl;		/* The connection handle of the ACL link */
	UINT8                  state;            /* the connection mode */
	struct FsmTimer*       ft;
	void*                  tl_handle;
};

/* APP FSM user data */
struct AppFsmUserData {
	struct ListStru bonded_device_list;	/* bonded device list. AppDevInst */
	struct ListStru	connection_list;	/* connection list. AppConnInst */
	struct ListStru acl_list;               /* acl link list. AppPowerModeStru*/
	UINT16			handle_base;
};

//#define APP_A2DP_DATA_USE_FULL_CPY
#ifdef APP_A2DP_DATA_USE_FULL_CPY
struct hscA2DPMessageStru {
    UINT8  msg;
    UINT16 len;
    UINT8 data[1];
};
#else
struct hscA2DPMessageStru {
    UINT8  msg;
    UINT16 len;
    UINT8  *data;
};
#endif
extern bt_host_config_t g_bt_host_config;
extern bt_host_status_t g_bt_host_status;

/* APP FSM UI */
void App_FsmInit(UINT8 mode);
void App_UI_FsmEvent(void *func, void *arg);
void App_SCH_FsmEvent(void *func, void *arg);

struct AppDevInst *App_FindBondedDevice(UINT8 *bd);
void App_AddBondedDevice(UINT8 *bd, UINT8 *link_key, UINT8 key_type);
void App_DeleteBondedDevice(struct AppDevInst *inst);
void App_StoreBondedDevice(void);
//void App_UpdateBondedDevice(void);

struct AppConnInst *App_FindConnectionInstByBD(UINT16 service_class, UINT8 *bd, UINT8 role);
struct AppConnInst *App_FindConnectionInstByUpperHandle(UINT16 connection_handle);
struct AppConnInst *App_FindConnectionInstByLowerHandle(void *profile_inst_handle);
struct AppConnInst *App_FindNextConnectionInst(struct AppConnInst *connection);
struct AppConnInst *App_AddConnectionInst(UINT16 service_class, UINT8 *bd, UINT8 role);
void App_DeleteConnectionInst(struct AppConnInst *inst);
void hsc_UpdateConnectionInst(struct AppConnInst *inst, UINT8 state);

// acl link list
void App_AddAclList(HANDLE tl_handle,struct GAP_ConnectionEventStru *acl);
void App_DeleteAclList(struct GAP_ConnectionEventStru *acl);

// bluetooth state 
void     App_SetBtState2(void* handle, UINT16 state);
void     App_SetBtState(UINT16 state);

// bluetooth audio
void App_AudioStart(INT32 smaple, INT32 mode, UINT32 size);
void App_AudioRecordStart(void);
void App_AudioStop(void);
UINT8 App_AudioPlayLocalStop(void);
UINT8 App_AudioRecordStop(void);
UINT8 App_AudioLocalTonePlay(void);

// hfp thread audio
void APP_AudioScheduleLoop(void);
void APP_AudioThreadSleep(void);
void APP_AudioThreadWakeup(void);

// hfp battery
//void APP_FsmBattery(void);

// for jvvi, comio, send sco when sco connect
void APP_StartSendScoTimer(void);
void APP_StopSendScoTimer(void);

UINT8 hsc_GetBtMode(void);

//void hsc_A2dpThradSendDataMessage(UINT8 *data);
void hsc_A2dpThradSendDataMessage(UINT16 len, UINT8 *data);
#endif
