#ifndef _APP_A2DP_H
#define _APP_A2DP_H

#define APP_A2DP_STATE_IDLE   0x0001
#define APP_A2DP_STATE_OPEN   0x0002
#define APP_A2DP_STATE_START  0x0004
#define APP_A2DP_STATE_PAUSE  0x0008
#define APP_A2DP_STATE_CLOSE  0x0010

/* APP A2DP FSM */
void App_A2dp_FsmStreamCreateInd(struct FsmInst *fi, UINT8 event, struct AVDTP_StreamCreateStru *in);
void App_A2dp_FsmStreamOpenInd(struct FsmInst *fi, UINT8 event, struct A2DP_OpenedStru *in);
void App_A2dp_FsmStreamDoneInd(struct FsmInst *fi, UINT8 event, struct A2DP_Stream_DoneStru *in);
void App_A2dp_FsmConnect(struct FsmInst *fi, UINT8 event, struct A2DP_ConnectReqStru *in);
void App_A2dp_FsmDisconnect(struct FsmInst *fi, UINT8 event, UINT16 *in);

/* APP A2DP UI */
void App_A2DP_RegSnkService(void);
void App_A2DP_Start(void);
void App_A2DP_Connect(UINT8 *bd);
void App_A2DP_Disconnect(UINT16 connection_handle);

// by AVRCP
void hsc_A2dpSetState(UINT8 *bd, UINT16 state);
// by CODEC decoder
BOOL hsc_A2dpIsSkip(HANDLE stream_handle) __attribute__((used));

// for a2dp multi-link
void hsc_A2dpStartMulti(UINT8 num);
UINT8 hsc_A2dpGetFreeLinkCount(void);
void hsc_A2dpDisableAudioStreamHandle(void);
void hsc_A2dpEnableAudioStreamHandle(void);

void hsc_A2dpHandleData(UINT16 len, UINT8 *buf);
void hsc_A2DP_ClearThreshold();
#endif
