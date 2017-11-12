#ifndef _APP_HFP_H
#define _APP_HFP_H

#define HFP_SCO_RECV_PACKET_SIZE  120//60 //ceva is 60,120, csr 40, 80
#define HFP_SCO_DATA_SIZE         HFP_SCO_RECV_PACKET_SIZE

struct AppHfpNewState {
	void *handle;
	UINT8 event;
};

struct AppHfpWaitingCall {
	void *handle;
	char number[HFP_PHONENUM_MAX_DIGITS];
};

struct AppHfpDial {
	UINT16 connection_handle;
	UINT8 number[HFP_PHONENUM_MAX_DIGITS];
};


/* APP HFP Callback */
/* Indication */
UINT32 APP_SCODataInd(HANDLE hdl, void *context, struct BuffStru *buf, UINT32 op_ev);

/* HFP FSM Function */
void App_Hfp_FsmConnectAG(struct FsmInst *fi, UINT8 event, UINT8 *bd);
void App_Hfp_FsmSppConnectCfm(struct FsmInst *fi, UINT8 event, struct HFP_ConnectEvStru *in);
void App_Hfp_FsmSppConnectInd(struct FsmInst *fi, UINT8 event, struct HFP_ConnectEvStru *in);
void App_Hfp_FsmSlcEstablishInd(struct FsmInst *fi, UINT8 event, struct HFP_SLCConnectInfoStru *in);
void App_Hfp_FsmSlcReleaseInd(struct FsmInst *fi, UINT8 event, struct HFP_SLCConnectInfoStru *in);
void App_Hfp_FsmSppDisconnectInd(struct FsmInst *fi, UINT8 event, struct HFP_ConnectEvStru *in);
void App_Hfp_FsmStateChangedInd(struct FsmInst *fi, UINT8 event, struct AppHfpNewState *in);
void App_Hfp_FsmCallWaitingInd(struct FsmInst *fi, UINT8 event, struct AppHfpWaitingCall *in);
void App_Hfp_FsmDisconnect(struct FsmInst *fi, UINT8 event, UINT16 *in);
void App_Hfp_FsmAnswer(struct FsmInst *fi, UINT8 event, UINT16 *in);
void App_Hfp_FsmReject(struct FsmInst *fi, UINT8 event, UINT16 *in);
void App_Hfp_FsmDial(struct FsmInst *fi, UINT8 event, struct AppHfpDial *in);

/* APP HFP UI */
void App_HFP_Start(void);
void App_HFP_ConnectAG(UINT8 *bd);
void App_HSP_ConnectAG(UINT8 *bd);
void hsc_HFP_Disconnect(void);
void hsc_HFP_Answer(void);
void hsc_HFP_Reject(void);
//void App_HFP_Dial(UINT16 connection_handle, UINT8 *number);
void App_HFP_SendSCOPacket(UINT8 *data, UINT16 size);

// for HFP multi-link
UINT8 hsc_HfpGetFreeLinkCount(void);
void hsc_HfpStartMulti(UINT8 num);
void hsc_HspStartMulti(UINT8 num);

void hsc_HfpSetLocalMicVol(void);
void hsc_HfpSetSpkVol(UINT8 level);

// for caller's name indicator of incoming call
UINT8* hsc_HFP_GetCurCallNum(void);
UINT8* hsc_HFP_GetLastCallNum(void);

// hfp audio record
void hsc_HfpRecordDataHandle(void);

// hfp send battery status to phone
//void hsc_HFP_SendBattery(void);
//void hsc_HfpFsmBattery(struct FsmInst *fi, UINT8 event, UINT16 *in);
/*
   get incoming call ring type
   0: only ring
   1: inband ring
*/
UINT8 hsc_HFp_GetRingType(void);
#endif
