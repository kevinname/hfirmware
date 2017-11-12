#ifndef _APP_GAP_H
#define _APP_GAP_H

/* APP GAP CBK */
/* Confirmation */
void App_GapInitRegisterTLCfm(HANDLE tl_handle, UINT16 result, struct GAP_RegisterTLCfmStru *cfm_par);
void App_GapInitResetHardwareCfm(void *context, UINT16 result, void *cfm_par);
void App_GapInitSetVisualModeCfm(void *context, UINT16 result, void *cfm_par);
void App_GapInitSetLocalDeviceClassCfm(void *context, UINT16 result, void *cfm_par);
void App_GapInitSetLocalNameCfm(void *context, UINT16 result, void *cfm_par);
/* Indication */
tGAP_IndCbk_Result App_GapGeneralInd(HANDLE tl_handle, void *context, tGAP_Ind_Type ind_type, void *parameter, UINT32 size);

/* APP GAP FSM */
void App_Gap_FsmLinkKeyReqInd(struct FsmInst *fi, UINT8 event, UINT8 *in);
void App_Gap_FsmLinkKeyCreated(struct FsmInst *fi, UINT8 event, struct HCI_Security_Link_Key_SaveStru *in);
void App_Gap_FsmAuthenFailure(struct FsmInst *fi, UINT8 event, struct GAP_AuthenticationFailureStru *in);
void App_Gap_FsmUnpairDevice(struct FsmInst *fi, UINT8 event, UINT8 *bd);
void App_Gap_FsmListCurrentConnections(struct FsmInst *fi, UINT8 event, void *arg);
void App_Gap_FsmRemoteNameUpdated(struct FsmInst *fi, UINT8 event, struct HCI_Remote_Name_Request_CompleteEvStru *in);

/* APP GAP UI */
void App_GAP_RegisterTL(HANDLE tl_handle, TransportLayerStru *func);
void App_GAP_UnRegisterTL(HANDLE tl_handle);
void App_GAP_UnpairDevice(UINT8 *bd); 
void App_GAP_ListCurrentConnections(void);

/* APP GAP */
void App_GAP_SetVisualModes(UINT8 mode); // 0 is no scan; 1 is inquiry scan enable, 2 is page scan enable, 3 is inquiry and page scan enable
void App_GAP_ReConnect(UINT8 *bd);
void App_GAP_DisconnectAll(void);

#endif
