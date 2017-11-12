/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/
 
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
	sdap_uifunc.h
Abstract:
	Export UI functions.
-------------------------------------------------------------------------------------------------*/
#ifndef _BT_SDAP_UIFUNC_H
#define _BT_SDAP_UIFUNC_H


#define SDAPSVCATTRSTRULEN			9	/*sizeof(attr_id) + sizeof(align_byte) + sizeof(attr_val.size) + sizeof(attr_val.descriptor)*/
#define _SDAPSVCATTRSTRULEN(attr)		((WORD)(SDAPSVCATTRSTRULEN + ((attr)->attr_val.size ? (attr)->attr_val.size : 1) + (attr)->align_byte))

/*++++++++++++++++++++ SDAP Service Register/Unregister UI Definition ++++++++++++++++++++*/
#ifdef CONFIG_SDAP
DWORD	SDAP_Init(void);
void	SDAP_Done(void);
const char* SDAP_GetVersion(void);
UCHAR* SDAP_MallocMemory(int size);
void SDAP_FreeMemory(UCHAR *buffer);
#endif

#ifdef CONFIG_SDAP_SERVER
WORD SDAP_AllocateSvcHdl(DWORD *handle);
WORD SDAP_RegisterAttribute(DWORD *svc_hdl,struct SDAP_SvcAttrStru *attr);
WORD SDAP_ServiceUnregister(DWORD svc_hdl);

struct SDAP_SvcAttrStru     *MakeUIntA(WORD id, UCHAR mode, const UCHAR *val);
struct SDAP_DataEleStru   	*MakeUIntDE(UCHAR mode, const UCHAR *val);
struct SDAP_SvcAttrStru  	*MakeIntA(WORD id, UCHAR mode, const UCHAR *val);
struct SDAP_DataEleStru	    *MakeIntDE(UCHAR mode, const UCHAR *val);
struct SDAP_SvcAttrStru  	*MakeUUIDA(WORD id, UCHAR mode, const struct SDAP_UUIDStru *val);
struct SDAP_DataEleStru      *MakeUUIDDE(UCHAR mode, const struct SDAP_UUIDStru *val);
struct SDAP_SvcAttrStru  	*MakeBoolA(WORD id, UCHAR mode, const UCHAR *val);
struct SDAP_DataEleStru      *MakeBoolDE(UCHAR mode, const UCHAR *val);
struct SDAP_SvcAttrStru  	*MakeStringA(WORD id, const UCHAR *src);
struct SDAP_DataEleStru      *MakeStringDE(const UCHAR *src);
struct SDAP_SvcAttrStru  	*MakeURLA(WORD id, const UCHAR *src);
struct SDAP_DataEleStru      *MakeURLDE(const UCHAR *src);
struct SDAP_SvcAttrStru    	*MakeDESeqA(WORD id, const char *szTypes,...);
struct SDAP_DataEleStru      *MakeDESeqDE(const char *szTypes,...);
struct SDAP_SvcAttrStru  	*MakeDEAltA(WORD id, const char *szTypes,...);
struct SDAP_DataEleStru      *MakeDEAltDE(const char *szTypes,...);


WORD SDAP_ModifyStringAttrib(DWORD svc_hdl, UCHAR *str_attrib, WORD attrib_id);
WORD SDAP_ModifySvcAvailAttrib(DWORD svc_hdl, UCHAR cur_clis, UCHAR max_clis);
WORD SDAP_RegisterGroup(DWORD *svc_hdl, DWORD mask, struct SDAP_UUIDStru *grp_id);

#ifdef CONFIG_SPP_SERVER
WORD SDAP_RegisterSPPService(UCHAR *svc_name, struct SDAP_SPPInfoStru *reg_info);
#else
#define SDAP_RegisterSPPService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)
#endif
#ifdef CONFIG_HFP_HF
WORD SDAP_RegisterHEPHSService(UCHAR *svc_name, struct SDAP_HEPHSInfoStru *reg_info);
#else
#define SDAP_RegisterHEPHSService(name, reg)		(ERROR_SDAP_API_UNAVAILABLE)
#endif
#define SDAP_RegisterHEPService(name, reg)			SDAP_RegisterHEPHSService(name, reg)
#ifdef CONFIG_HFP_AG
WORD SDAP_RegisterHEPAGService(UCHAR *svc_name, struct SDAP_HEPAGInfoStru *reg_info);
#else
#define SDAP_RegisterHEPAGService(name, reg)		(ERROR_SDAP_API_UNAVAILABLE)
#endif
#ifdef CONFIG_HFP_HF
WORD SDAP_RegisterHFPHFService(UCHAR *svc_name, struct SDAP_HFPHFInfoStru *reg_info);
#else
#define SDAP_RegisterHFPHFService(name, reg)		(ERROR_SDAP_API_UNAVAILABLE)
#endif
#define SDAP_RegisterHFPService(name, reg)			SDAP_RegisterHFPHFService(name, reg)
#ifdef CONFIG_HFP_AG
WORD SDAP_RegisterHFPAGService(UCHAR *svc_name, struct SDAP_HFPAGInfoStru *reg_info);
#else
#define SDAP_RegisterHFPAGService(name, reg)	 	(ERROR_SDAP_API_UNAVAILABLE)	
#endif
#ifdef CONFIG_AVRCP
WORD SDAP_RegisterAVRCPService(UCHAR *svc_name, struct SDAP_AVRCPInfoStru *reg_info);
#define SDAP_RegisterAVCTService(name, reg)		SDAP_RegisterAVRCPService(name, reg); 
#define SDAP_RegisterAVTGService(name, reg)		SDAP_RegisterAVRCPService(name, reg); 
#else
#define SDAP_RegisterAVRCPService(name, reg)		(ERROR_SDAP_API_UNAVAILABLE)
#define SDAP_RegisterAVCTService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)	
#define SDAP_RegisterAVTGService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)	
#endif
#ifdef CONFIG_A2DP
WORD SDAP_RegisterA2DPService(UCHAR *svc_name, struct SDAP_A2DPInfoStru *reg_info);
#define SDAP_RegisterA2DPSrcService(name, reg)		SDAP_RegisterA2DPService(name, reg); 
#define SDAP_RegisterA2DPSnkService(name, reg)		SDAP_RegisterA2DPService(name, reg); 
#else
#define SDAP_RegisterA2DPService(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)
#define SDAP_RegisterA2DPSrcService(name, reg)		(ERROR_SDAP_API_UNAVAILABLE)	
#define SDAP_RegisterA2DPSnkService(name, reg)		(ERROR_SDAP_API_UNAVAILABLE)	
#endif
#ifdef CONFIG_DI_SERVER
WORD SDAP_RegisterDIService(UCHAR *svc_name, struct SDAP_DIInfoStru *reg_info);
#else
#define SDAP_RegisterDIService(name, reg)				(ERROR_SDAP_API_UNAVAILABLE)
#endif
#ifdef CONFIG_PBAP_CLIENT
WORD SDAP_RegisterPBAPPCEService(UCHAR *svc_name, struct SDAP_PBAPPCEInfoStru *reg_info);
#else
#define SDAP_RegisterPBAPPCEService(name, reg)		(ERROR_SDAP_API_UNAVAILABLE)
#endif

#ifdef CONFIG_HID
WORD SDAP_RegisterHIDService(UCHAR *svc_name, UCHAR* svc_desc, UCHAR* pro_name, struct SDAP_HIDInfoStru *reg_info);
#define SDAP_RegisterHIDDeviceService(name, desc, pro, reg)                SDAP_RegisterHIDService(name, desc, pro, reg); 
#define SDAP_RegisterHIDHostService(name, desc, pro, reg)                  SDAP_RegisterHIDService(name, desc, pro, reg); 
#else
#define SDAP_RegisterHIDService(name, desc, pro, reg)                      (ERROR_SDAP_API_UNAVAILABLE)
#define SDAP_RegisterHIDDeviceService(name, desc, pro, reg)                (ERROR_SDAP_API_UNAVAILABLE)	
#define SDAP_RegisterHIDHostService(name, desc, pro, reg)                  (ERROR_SDAP_API_UNAVAILABLE)
#endif

#ifdef CONFIG_IAP2 // add by zhiyuan.chen 20161226
WORD SDAP_RegisterIAP2Service(UCHAR *svc_name, struct SDAP_IAPInfoStru *reg_info);
#else
#define SDAP_RegisterIAP2Service(name, reg)			(ERROR_SDAP_API_UNAVAILABLE)
#endif
#endif

WORD SDAP_RegisterMPSService(struct SDAP_MPSInfoStru *reg_info); 
/*++++++++++++++++++++++++ SDAP Service Search UI Definition +++++++++++++++++++++++++*/
#ifdef CONFIG_SDAP_CLIENT
WORD SDAP_OpenSearch(UCHAR* bd, WORD *trans_hdl);
WORD SDAP_CloseSearch(WORD trans_hdl);
WORD SDAP_Search(struct SDP_SearchReqStru *sreq, struct SDP_SearchCfmStru *scfm);
WORD SDAP_SearchEx(struct SDP_SearchReqStru *sreq, struct SDP_SearchCfmStru *scfm);
WORD SDAP_ServiceBrowse(struct SDAP_BrowseInfoStru *info, WORD *size, DWORD *hdl_buf);
WORD SDAP_GetServiceAttribute(WORD trans_hdl, DWORD svc_hdl, WORD id, struct SDAP_DataEleStru *value);
WORD SDAP_ServiceSearch(struct SDAP_SearchInfoStru *info, DWORD svc_hdl, DWORD *size, UCHAR *attr_buf);
WORD SDAP_GetStringAttribute(WORD trans_hdl, DWORD svc_hdl, WORD id, DWORD *size, UCHAR *str);
WORD SDAP_GetServiceInfo(WORD trans_hdl, DWORD *uuid, struct SDAP_GeneralInfoStru *sdx_info);
struct SDAP_GeneralInfoStru *SDAP_GetServiceInfoEx(WORD trans_hdl, DWORD svc_hdl, DWORD *svc_type);
WORD SDAP_GetServiceInfo2(WORD trans_hdl, WORD svc_type, struct SDAP_UUIDStru *uuid, struct SDAP_GeneralInfoStru *sdx_info);
WORD SDAP_GetAllAttributes(WORD trans_hdl, DWORD svc_hdl, struct SDP_SearchCfmStru *sCfm);

struct SDAP_GeneralInfoStru *SDAP_CreateSvcInfoStru(DWORD buf_size, UCHAR *buf, DWORD svc_hdl, DWORD svc_type);

/*
	app_context: application defined context data used to identify the request.
    bd:     Device address of the SDP server.
    status: Status of the connection setup procedure, it can be one of,
            ERROR_SDAP_OPEN_PENDING - The connection request is sent and waiting
            for response. The application can cancel the connection request in
            this state.
            ERROR_SDAP_SUCCESS - The connection with the server is created successfully.
            Other error codes - Fail to create a connection with the server.
    trans_hdl: Transaction handle used to identify the connection.
*/
typedef void (SDAP_OpenSearchRsp_Func)(void *app_context, UCHAR *bd, WORD status, WORD trans_hdl);

/*
	app_context: application defined context data used to identify the request.
    trans_hdl: transaction handle of the service discovery session to be closed.
    status: Status of the disconnection procedure, it can be one of,
            ERROR_SDAP_SUCCESS - Remove one reference or release the connection
                                 successfully.
            ERROR_SDP_INVALID_TRANSHANDLE - trans_hdl is an illegal transation handle.
*/
typedef void (SDAP_CloseSearchRsp_Func)(void *app_context, WORD trans_hdl, WORD status);

/*
	app_context: application defined context data used to identify the request.
    trans_hdl: Transaction handle used to identify the connection.
    status: Status of the attribute search procedure, it can be one of,
            ERROR_SDAP_SUCCESS - Retrieve attribute values successfully and the
                                 service is available.
            ERROR_SDAP_SERVICE_UNAVAILABLE - Retrieve attribute values successfully
                                 but the specified service is not accepting new clients.
            Other error codes - Fail to retrieve attribute values.
	svc_type: a 16bit UUID specifies the type of the service record retrieved.
	sdx_info: Pointer to a variable of struct SDAP_GeneralInfoStru type that returns
              the attribute values.
*/
typedef void (SDAP_GetServiceInfoRsp_Func)(void *app_context, WORD trans_hdl, WORD status, WORD svc_type, struct SDAP_GeneralInfoStru *sdx_info);

/*
	app_context: application defined context data used to identify the request.
    bd:     Device address of the SDP server.
    status: Status of the attribute search procedure, it can be one of,
            ERROR_SDAP_SUCCESS - Retrieve attribute values successfully and the
                                 service is available.
            ERROR_SDAP_SERVICE_UNAVAILABLE - Retrieve attribute values successfully
                                 but the specified service is not accepting new clients.
            Other error codes - Fail to retrieve attribute values.
	svc_type: a 16bit UUID specifies the type of the service record retrieved.
	sdx_info: Pointer to a variable of struct SDAP_GeneralInfoStru type that returns
              the attribute values.
*/
typedef void (SDAP_GetServiceInfoBDRsp_Func)(void *app_context, UCHAR *bd, WORD status, WORD svc_type, struct SDAP_GeneralInfoStru *sdx_info);

void SDAP_OpenSearchReq(UCHAR *bd, void *app_context, SDAP_OpenSearchRsp_Func *rsp_func);
void SDAP_CloseSearchReq(WORD trans_hdl, void *app_context, SDAP_CloseSearchRsp_Func *rsp_func);
void SDAP_GetServiceInfoReq(WORD trans_hdl, WORD svc_type, struct SDAP_UUIDStru *uuid, 
                            DWORD svc_hdl, void *app_context, SDAP_GetServiceInfoRsp_Func *rsp_func);
void SDAP_GetServiceInfoBDReq(UCHAR *bd, WORD svc_type, struct SDAP_UUIDStru *uuid, 
                            DWORD svc_hdl, void *app_context, SDAP_GetServiceInfoBDRsp_Func *rsp_func);

#define SDAP_GetSPPSvcInfo(hdl, info)  		SDAP_GetServiceInfo2(hdl, CLS_SERIAL_PORT, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetHEPSvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_HEADSET, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetHEPHSSvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_HEADSET, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetHEPAGSvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_HEADSET_AG, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetHFPSvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_HANDSFREE, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetHFPHFSvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_HANDSFREE, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetHFPAGSvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_HANDSFREE_AG, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetAVRCPSvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, (info)->svc_type, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetAVRCTInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_AVRCP_CT, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetAVRTGInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_AVRCP_TG, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetA2DPSvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_ADV_AUDIO_DISTRIB, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetA2DPSrcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_AUDIO_SOURCE, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetA2DPSnkInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_AUDIO_SINK, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetDISvcInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_PNP_INFO, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetPBAPPSEInfo(hdl, info)		SDAP_GetServiceInfo2(hdl, CLS_PBAP_PSE, NULL, (struct SDAP_GeneralInfoStru*)info)
#define SDAP_GetHIDSvcInfo(hdl, info)           SDAP_GetServiceInfo2(hdl, CLS_HID, NULL, (struct SDAP_GeneralInfoStru*)info)
#endif

#endif
