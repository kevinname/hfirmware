/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Module Name:
	avdtp_ui.h
Abstract:
	This file defines AVDTP ui macros, functions and defainations.
-----------------------------------------------------------------------------*/

#ifndef AVDTP_UIH
#define AVDTP_UIH

typedef void (AVDTP_CbkFuc)(UINT16 msg, void *arg);/* callback to upper protocol */

/*-----------------------------------------------------------------------------
A/V Distribution Transport Protocol (AVDTP) 
	Media Type
	The table below identifies the possible values that the "Media Type" field 
	in AVDTP can have. The information element identifies the media type of 
	a stream end-point.

	3	2	1	0	Description
	0	0	0	0	Audio
	0	0	0	1	Video
	0	0	1	0	Multimedia
	X	X	X	X	All other values reserved

-----------------------------------------------------------------------------*/
enum {/* BT-SIG assign number https://www.bluetooth.org/Technical/AssignedNumbers/host_operating.htm */
	AVDTP_MEDIATYPE_AUDIO = 				0x00,
	AVDTP_MEDIATYPE_VIDEO = 				0x01,
	AVDTP_MEDIATYPE_MULTIMEDIA = 			0x02
};

enum {/* 1bit TSEP */
	AVDTP_TSEP_SRC = 						0,
	AVDTP_TSEP_SNK = 						1	
};

/*---------------------------------------------------------------------------*/
/* 							SEP Service Capabilities					     */
/*---------------------------------------------------------------------------*/

enum {/* mask in struct AVDTP_SEPStru */
	AVDTP_SEPMASK_INUSE = 					0x01,
	AVDTP_SEPMASK_GETCAPS_OUTSTANDING = 	0x02,/* Status control */
	AVDTP_SEPMASK_GETCAPS_DONE = 			0x04,
	AVDTP_SEPMASK_ISSRCSVC = 				0x10,
	AVDTP_SEPMASK_ISSNKSVC = 				0x20
};

struct AVDTP_ServiceCapStru {
	UINT8 len;								/* Service Capability Length */
	UINT8 service_capability[1];			/* Service Category(1), LOSC(1), Data(x), This is for SEP GetCapability */
};

struct AVDTP_SEPStru {
	UINT8 seid;								/* 6bits, seid of this SEP element */	
	UINT8 mask;								/* e.g: AVDTP_SEPMASK_INUSE */
	UINT8 media_type;						/* Media_type(4bits)+TSEP(1bit, SNK=1)+RFA(3bits), This is for SEP Discovery */
	struct AVDTP_ServiceCapStru sc;			/* Service Capability */
};

struct AVDTP_SEIDStru {
	UINT8 seid;								/* 6bits, seid of this SEP element */	
	UINT8 mask;								/* e.g: AVDTP_SEPMASK_INUSE */
	UINT8 media_type;						/* Media_type(4bits)+TSEP(1bit, SNK=1)+RFA(3bits), This is for SEP Discovery */
};

struct AVDTP_ConnectReqStru {
	struct ListStru perfer;					/* struct AVDTP_ServiceCapStru, perfered codec stack, shall give config params */
	void *cbk;
	UINT8 bd[BD_ADDR_LEN];
	UINT8 src;								/* Local is Source? */
	UINT16 version;							/* remote service version */
};

enum {/* Service Category */
	AVDTP_SC_MEDIA_TRANSPORT = 1,				/* 0000,0001 */
	AVDTP_SC_REPORTING,							/* 0000,0010 */
	AVDTP_SC_RECOVERY,							/* 0000,0011 */
	AVDTP_SC_CONTENT_PROTECTION,				/* 0000,0100 */
	AVDTP_SC_HEADER_COMPRESSION,				/* 0000,0101 */
	AVDTP_SC_MULTIPLEXING,						/* 0000,0110 */
	AVDTP_SC_MEDIA_CODEC,						/* 0000,0111 */
	AVDTP_SC_DELAY_REPORTING					/* 0000,1000 */
};

enum {/* AVDTP spec defined error_code */
	/* Internal used */
	AVDTP_ERROR_SUCCESS = 								0x00,
	AVDTP_ERROR_GETCAPS = 								0xFF,/* Caution, not in spec, as meaning pendding */
	AVDTP_ERROR_NOSUITABLE_SEP = 						0xFE,/* Caution, not in spec, as no SEP suitable */

	/* ACP to INT, Signal Response Header Error Codes */
	AVDTP_ERROR_BAD_HEADER_FORMAT = 					0x01,/* The request packet header format error that is not specified above ERROR_CODE */

	/* ACP to INT, Signal Response Payload Format Error Codes */
	AVDTP_ERROR_BAD_LENGTH = 							0x11,/* The request packet length is not match the assumed length */
	AVDTP_ERROR_BAD_ACP_SEID = 							0x12,/* The requested command indicates an invalid ACP SEID (not addressable) */
	AVDTP_ERROR_SEP_IN_USE = 							0x13,/* The SEP is in use */
	AVDTP_ERROR_SEP_NOT_INUSE = 						0x14,/* The SEP is not in use */
	AVDTP_ERROR_BAD_SERV_CATEGORY = 					0x17,/* The value of Service Category in the request packet is not defined in AVDTP */
	AVDTP_ERROR_BAD_PAYLOAD_FORMAT = 					0x18,/* The requested command has an incorrect payload format (Format errors not specified in this ERROR_CODE) */
	AVDTP_ERROR_NOT_SUPPORTED_COMMAND = 				0x19,/* The requested command is not supported by the device */
	AVDTP_ERROR_INVALID_CAPABILITIES = 					0x1A,/* The reconfigure command is an attempt to reconfigure a transport service capabilities of the SEP. Reconfigure is only permitted for application service capabilities */

	/* ACP to INT, Signal Response Transport Service Capabilities Error Codes */
	AVDTP_ERROR_BAD_RECOVERY_TYPE = 					0x22,/* The requested Recovery Type is not defined in AVDTP */
	AVDTP_ERROR_BAD_MEDIA_TRANSPORT_FORMAT = 			0x23,/* The format of Media Transport Capability is not correct */
	AVDTP_ERROR_BAD_RECOVERY_FORMAT = 					0x25,/* The format of Recovery Service Capability is not correct */
	AVDTP_ERROR_BAD_ROHC_FORMAT = 						0x26,/* The format of Header Compression Service Capability is not correct */
	AVDTP_ERROR_BAD_CP_FORMAT = 						0x27,/* The format of Content Protection Service Capability is not correct */
	AVDTP_ERROR_BAD_MULTIPLEXING_FORMAT = 				0x28,/* The format of Multiplexing Service Capability is not correct */
	AVDTP_ERROR_UNSUPPORTED_CONFIGURATION = 			0x29,/* Configuration not supported */

	/* ACP to INT, Procedure Error Codes */
	/* Indicates that the ACP state machine is in an invalid state in order to process the signal. This also includes the situation 
	when an INT receives a request for the same command that it is currently expecting a response for */
	AVDTP_ERROR_BAD_STATE = 							0x31,

	/* ACP-Upper Layer to INT-AVDTP Signal Response Error Codes */
	AVDTP_ERROR_BAD_SERVICE = 							0x80,
	AVDTP_ERROR_INSUFFICIENT_RESOURCE = 				0x81
	/* 0xC0~0xFF is reserved for the profile residing on top GAVDP */
};
#define AVDTP_ERROR_FAILED(s)			(((s) != AVDTP_ERROR_SUCCESS) && ((s) != AVDTP_ERROR_GETCAPS))

enum {/* media_codec_type in Service Media Codec, Audio Codec ID assigned by SIG */
	AVDTP_CODEC_SBC = 									0x00,
	AVDTP_CODEC_MP3 = 									0x01,
	AVDTP_CODEC_AAC = 									0x02,
	AVDTP_CODEC_ATRAC = 								0x04,
	AVDTP_CODEC_NONA2DP = 								0xFF
};

enum {/* media_codec_type in Service Media Codec, Video Codec ID assigned by SIG */
	AVDTP_CODEC_H263BASELINE = 							0x01,
	AVDTP_CODEC_MPEG4VSP = 								0x02,
	AVDTP_CODEC_H263PROFILE3 = 							0x03,
	AVDTP_CODEC_H263PROFILE8 = 							0x04,
	AVDTP_CODEC_NONVDP = 								0xFF
};

enum {/* Callback event in AVDTP_EventCBK */
	AVDTP_EV_REGSEP,
	AVDTP_EV_SEPDISCOVERY,
	AVDTP_EV_GETCAPABILITIES,
	AVDTP_EV_SETCONFIGURATION,
	AVDTP_EV_GETCONFIGURATION,
	AVDTP_EV_STREAMOPEN,
	AVDTP_EV_STREAMSTART,
	AVDTP_EV_STREAMSUSPEND,
	AVDTP_EV_STREAMCLOSE,
	AVDTP_EV_STREAMABORT,
	AVDTP_EV_RECONFIGURE,
	AVDTP_EV_DATAIND,
	AVDTP_EV_STREAMDONE,
	/* new add event */
	AVDTP_EV_STREAMCREATE,/* upper use this ev to create entity */
	AVDTP_EV_STREAMDELAYREPORTING
};

/* AVDTP is big endian */
#define AVDTP_DECODE2BYTE(ptr) 	(UINT16)(*(UINT8 *)((ptr) + 1) | (*(UINT8 *)(ptr) << 8))
#define AVDTP_DECODE4BYTE(ptr)\
	(UINT32)(*(UINT8 *)((ptr) + 3) | (*(UINT8 *)((ptr) + 2) << 8) \
	| (*(UINT8 *)((ptr) + 1) << 16) | (*(UINT8 *)(ptr) << 24))
#define AVDTP_ENCODE2BYTE(ptr, data)\
	*(UINT8 *)(ptr) = (UINT8)(((data) & 0xFF00) >> 8);\
	*(UINT8 *)((ptr) + 1) = (UINT8)((data) & 0x00FF);
#define AVDTP_ENCODE4BYTE(ptr, data)\
	*(UINT8 *)(ptr) = (UINT8)(((data) & 0xFF000000) >> 24);\
	*(UINT8 *)((ptr) + 1) = (UINT8)(((data) & 0x00FF0000) >> 16);\
	*(UINT8 *)((ptr) + 2) = (UINT8)(((data) & 0x0000FF00) >> 8);\
	*(UINT8 *)((ptr) + 3) = (UINT8)((data) & 0x000000FF);

/*-----------------------------------------------------------------------------
Description:
	Input structures.
-----------------------------------------------------------------------------*/
struct AVDTP_RegCbkStru {/* input param in AVDTP_RegCbk */
	UINT8 media_type;/* e.g AVDTP_MEDIATYPE_AUDIO. No Shift */
	void *cbk;
};

struct AVDTP_StreamCreateStru {
	void *stream_handle;
	UINT8 remote_bd[BD_ADDR_LEN];
	UINT8 mask;/* src or snk */
};

struct AVDTP_Stream_DoneStru {
	void *stream_handle;
	UINT8 mask;
	UINT8 reason;
};

struct AVDTP_SetConfigStru {/* param in AVDTP_ACTIVE_EV_SETCONFIGURATION cbk */
	void *stream_handle;
	UINT16 media_mtu;
	UINT8 bd[BD_ADDR_LEN];
	UINT8 media_type;/* Media_type(4bits)+TSEP(1bit), AVDTP_TSEP_SRC+RFA(3bits), e.g AVDTP_MEDIATYPE_AUDIO */
	UINT8 reconfig_flag;
	struct AVDTP_ServiceCapStru sc;
};

struct AVDTP_ReadyToStartStru {
	void *stream_handle;
	UINT16 media_mtu;
	UINT16 cid;
	UINT8 initiator;
	UINT8 media_type;/* a2dpapp_src_opencodec needs */
	//struct AVDTP_ServiceCapStru *selected_sc;
};

struct AVDTP_StartSuspendIndStru {
	void *stream_handle;
	UINT8 mask;
};

struct AVDTP_Stream_DataIndStru {
	void *stream_handle;
	struct BuffStru *data;
};

struct AVDTP_Stream_DelayReportingIndStru {
	void *stream_handle;
	UINT16 delay_value;
	UINT8 acp_seid;
};

enum {/* signal id (6bits) */
	AVDTP_RFA = 								0x00,/* General Reject */
	AVDTP_DISCOVER = 							0x01,
	AVDTP_GET_CAPABILITIES = 					0x02,
	AVDTP_SET_CONFIGURATION = 					0x03,
	AVDTP_GET_CONFIGURATION = 					0x04,
	AVDTP_RECONFIGURE = 						0x05,
	AVDTP_OPEN = 								0x06,
	AVDTP_START = 								0x07,
	AVDTP_CLOSE	= 								0x08,
	AVDTP_SUSPEND = 							0x09,
	AVDTP_ABORT = 								0x0A,
	AVDTP_SECURITY_CONTROL = 					0x0B,
	AVDTP_GET_ALL_CAPABILITIES = 				0x0C,
	AVDTP_DELAY_REPORT = 						0x0D
};

FUNC_EXPORT void AVDTP_Init(void);
FUNC_EXPORT void AVDTP_Done(void);
FUNC_EXPORT void AVDTP_RegCbk(struct AVDTP_RegCbkStru *in);
FUNC_EXPORT void AVDTP_Register_SEP(struct AVDTP_SEPStru *in);
FUNC_EXPORT void AVDTP_UnRegister_SEP(UINT8 seid);
FUNC_EXPORT void AVDTP_Connect_Req(struct AVDTP_ConnectReqStru *in);
FUNC_EXPORT void AVDTP_Disconnect_Req(void *stream_handle);
FUNC_EXPORT void AVDTP_GetConfiguration_Req(void *stream_handle);
FUNC_EXPORT void AVDTP_StreamStart_Req(void *stream_handle);
FUNC_EXPORT void AVDTP_StreamSuspend_Req(void *stream_handle);
FUNC_EXPORT void AVDTP_Reconfigure_Req(struct AVDTP_SetConfigStru *in);
FUNC_EXPORT void AVDTP_StreamWrite(HANDLE hdl, struct BuffStru *buf);

#ifdef CONFIG_PATCH_HUNTERSUN
FUNC_EXPORT void AVDTP_StreamAbort_ReqEx(void *stream_handle);
#endif
#endif

