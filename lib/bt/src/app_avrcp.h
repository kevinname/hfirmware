#ifndef _APP_AVRCP_H
#define _APP_AVRCP_H

/* AVRCP FSM Argument structure */
struct App_PassThroughReq {
	UINT8						  bd[BD_ADDR_LEN];
	struct AVRCP_PassThroughStru  parameter;
};

/* APP AVRCP FSM */
void App_Avrcp_FsmConnectComplete(struct FsmInst *fi, UINT8 event, struct AVRCP_ConnectCompleteStru *in);
void App_Avrcp_FsmDisconnectComplete(struct FsmInst *fi, UINT8 event, struct AVRCP_ConnectCompleteStru *in);
void App_Avrcp_FsmPassThrougReq(struct FsmInst *fi, UINT8 event, struct App_PassThroughReq *in);
void App_Avrcp_FsmConnect(struct FsmInst *fi, UINT8 event, UINT8 *bd);

/* APP AVRCP UI */
void App_AVRCP_Start(void);
void App_AVRCP_ButtonPushed(UINT8 *bd, UINT8 op_id, UINT8 vendor_id);
void App_AVRCP_ButtonReleased(UINT8 *bd, UINT8 op_id, UINT8 vendor_id);
void App_AVRCP_Connect(UINT8 *bd);
void App_AVRCP_Disconnect(UINT16 connection_handle);

// for multi-role
void hsc_AvrcpStartByRole(UINT8 role);
// by A2DP or PTS
void hsc_AvrcpChangeVol(UINT8 *bd, BOOL key);
void hsc_AvrcpVolChange(UINT8* bd, int db);
void hsc_AvrcpSetLocalVol(void);
void hsc_AvrcpSwitchLocalVol(UINT8 *bd);
void hsc_AvrcpDownVol(void);
void hsc_AvrcpUpVol(void);
void hsc_AvrcpSetSpkVol(UINT8 level);
UINT8 hsc_AvrcpGetSpkVol(void);

/* Button Operation - Passthrough Command */
/* Select */
#define App_AVRCP_Select_Pushed(bd)			App_AVRCP_ButtonPushed(bd, AVRCP_OPID_SELECT, 0)
#define App_AVRCP_Select_Released(bd)		App_AVRCP_ButtonReleased(bd, AVRCP_OPID_SELECT, 0)
/* Up */
#define App_AVRCP_Up_Pushed(bd)				App_AVRCP_ButtonPushed(bd, AVRCP_OPID_UP, 0)
#define App_AVRCP_Up_Released(bd)			App_AVRCP_ButtonReleased(bd, AVRCP_OPID_UP, 0)
/* Down */
#define App_AVRCP_Down_Pushed(bd)			App_AVRCP_ButtonPushed(bd, AVRCP_OPID_DOWN, 0)
#define App_AVRCP_Down_Released(bd)			App_AVRCP_ButtonReleased(bd, AVRCP_OPID_DOWN, 0)
/* Left */
#define App_AVRCP_Left_Pushed(bd)			App_AVRCP_ButtonPushed(bd, AVRCP_OPID_LEFT, 0)
#define App_AVRCP_Left_Released(bd)			App_AVRCP_ButtonReleased(bd, AVRCP_OPID_LEFT, 0)
/* Left */
#define App_AVRCP_Right_Pushed(bd)			App_AVRCP_ButtonPushed(bd, AVRCP_OPID_RIGHT, 0)
#define App_AVRCP_Right_Released(bd)		App_AVRCP_ButtonReleased(bd, AVRCP_OPID_RIGHT, 0)

/* Volume Up */
#define App_AVRCP_VolumeUp_Pushed(bd)		App_AVRCP_ButtonPushed(bd, AVRCP_OPID_VOLUME_UP, 0)
#define App_AVRCP_VolumeUp_Released(bd)		App_AVRCP_ButtonReleased(bd, AVRCP_OPID_VOLUME_UP, 0)
/* Volume down */
#define App_AVRCP_VolumeDown_Pushed(bd)		App_AVRCP_ButtonPushed(bd, AVRCP_OPID_VOLUME_DOWN, 0)
#define App_AVRCP_VolumeDown_Released(bd)	App_AVRCP_ButtonReleased(bd, AVRCP_OPID_VOLUME_DOWN, 0)
/* Mute */
#define App_AVRCP_Mute_Pushed(bd)			App_AVRCP_ButtonPushed(bd, AVRCP_OPID_MUTE, 0)
#define App_AVRCP_Mute_Released(bd)			App_AVRCP_ButtonReleased(bd, AVRCP_OPID_MUTE, 0)
/* Play */
#define App_AVRCP_Play_Pushed(bd)			App_AVRCP_ButtonPushed(bd, AVRCP_OPID_PLAY, 0)
#define App_AVRCP_Play_Released(bd)			App_AVRCP_ButtonReleased(bd, AVRCP_OPID_PLAY, 0)
/* Stop */
#define App_AVRCP_Stop_Pushed(bd)			App_AVRCP_ButtonPushed(bd, AVRCP_OPID_STOP, 0)
#define App_AVRCP_Stop_Released(bd)			App_AVRCP_ButtonReleased(bd, AVRCP_OPID_STOP, 0)
/* Pause */
#define App_AVRCP_Pause_Pushed(bd)			App_AVRCP_ButtonPushed(bd, AVRCP_OPID_PAUSE, 0)
#define App_AVRCP_Pause_Released(bd)		App_AVRCP_ButtonReleased(bd, AVRCP_OPID_PAUSE, 0)
/* Record */
#define App_AVRCP_Record_Pushed(bd)			App_AVRCP_ButtonPushed(bd, AVRCP_OPID_RECORD, 0)
#define App_AVRCP_Record_Released(bd)		App_AVRCP_ButtonReleased(bd, AVRCP_OPID_RECORD, 0)
/* Rewind */
#define App_AVRCP_Rewind_Pushed(bd)			App_AVRCP_ButtonPushed(bd, AVRCP_OPID_REWIND, 0)
#define App_AVRCP_Rewind_Released(bd)		App_AVRCP_ButtonReleased(bd, AVRCP_OPID_REWIND, 0)
/* Forward */
#define App_AVRCP_Forward_Pushed(bd)		App_AVRCP_ButtonPushed(bd, AVRCP_OPID_FORWARD, 0)
#define App_AVRCP_Forward_Released(bd)		App_AVRCP_ButtonReleased(bd, AVRCP_OPID_FORWARD, 0)
/* Backward */
#define App_AVRCP_Backward_Pushed(bd)		App_AVRCP_ButtonPushed(bd, AVRCP_OPID_BACKWARD, 0)
#define App_AVRCP_Backward_Released(bd)		App_AVRCP_ButtonReleased(bd, AVRCP_OPID_BACKWARD, 0)
/* FAST_FORWARD */
#define App_AVRCP_FFWD_Pushed(bd)			App_AVRCP_ButtonPushed(bd, AVRCP_OPID_FAST_FORWARD, 0)
#define App_AVRCP_FFWD_Released(bd)		App_AVRCP_ButtonReleased(bd, AVRCP_OPID_FAST_FORWARD, 0)

#endif
