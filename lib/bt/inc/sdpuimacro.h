/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
    sdpuimacro.h
Abstract:
	This files exports SDP API constants.
---------------------------------------------------------------------------*/
#ifndef _IVTSDP_UI_MACRO_H_
#define _IVTSDP_UI_MACRO_H_


/* Data type descriptor */
#define DEMODE_NULL			0
#define DEMODE_UINT			1
#define DEMODE_INT			2
#define DEMODE_UUID			3
#define DEMODE_STRING		4
#define DEMODE_BOOL			5
#define DEMODE_DESEQ		6
#define DEMODE_DEALT		7
#define DEMODE_URL			8

/* Data size descriptor */
#define DEMODE_1_BYTE		0
#define DEMODE_2_BYTES		1
#define DEMODE_4_BYTES		2
#define DEMODE_8_BYTES		3
#define DEMODE_16_BYTES		4
#define DEMODE_ADD_8BITS	5
#define DEMODE_ADD_16BITS	6
#define DEMODE_ADD_32BITS	7

/* Mask for OpenInfoStru */
#define OPEN_IN_MTU					0x00000001
#define OPEN_LINK_TO				0x00000002
#define OPEN_FLUSH_TO				0x00000004
#define OPEN_QOS_FLAGS				0x00000018
#define OPEN_QOS_SERVICE_TYPE		0x00000028
#define OPEN_QOS_TOKEN_RATE			0x00000048
#define OPEN_QOS_TOKEN_B_SIZE		0x00000088
#define OPEN_QOS_PEAK_BANDWIDTH		0x00000108
#define OPEN_QOS_LATENCY			0x00000208
#define OPEN_QOS_DELAY_VARIATION	0x00000408
#define OPEN_QOS_MASK				0x00000008

/* SDP Transaction type */
#define TRANS_SS					0x00000001
#define TRANS_SA					0x00000002
#define TRANS_SSA					0x00000003

/* Mask for stop rule */
#define STOP_MAXCOUNT				0x00000010
#define STOP_DURATION				0x00000020

/* Mask for ServiceAttributeID */
#define ATTRID_SINGLE				1
#define ATTRID_RANGE				2

/* UUID type */
#define UUID_16						1
#define UUID_32						2
#define UUID_128					4
#define UUID_LAST					0xF

/* Attribute ID */
#define SRV_RECORD_HANDLE			0x0000
#define SRV_CLSID_LIST				0x0001
#define SRV_RECORD_STATE			0x0002
#define SRV_ID						0x0003
#define PROT_DESCRIPTION_LIST		0x0004
#define BROWSE_GROUP_LIST			0x0005
#define LANG_BASE_ATTR_ID_LIST		0x0006
#define SRV_INFO_TIME_TO_LIVE		0x0007
#define SRV_AVAILABILITY			0x0008
#define PROF_DESCRIPTION_LIST		0x0009
#define DOC_URL						0x000A
#define CLIENT_EXEC_URL				0x000B
#define ICON_URL					0x000C
#define ADDITIONAL_PROT_DESC_LIST	0x000D
#define OFFSET_SRV_NAME				0x0000
#define OFFSET_SRV_DESCRIPTION		0x0001
#define OFFSET_PROVIDER_NAME		0x0002

#define VER_NUM_LIST				0x0200
#define SRV_DB_STATE				0x0201

#define GROUP_ID					0x0200

#define REM_AUDIO_VOL_CTRL			0x0302
#define EXT_NETWORK					0x0301
#define SRV_VERSION					0x0300
#define SUP_DATA_STORE_LST			0x0301

#define FAX_CLS_1					0x0302
#define FAX_CLS_2_0					0x0303
#define FAX_CLS_2					0x0304
#define AUDIO_FEEDBACK				0x0305

#define SUP_FMT_LST					0x0303

#define IP_SUBNET					0x0200
#define SECURITY_DESCRIPTION		0x030A
#define NET_ACCESS_TYPE				0x030B
#define MAX_NET_ACCESS_RATE			0x030C

#define HCRP_1284ID					0x0300
#define HCRP_DEVICE_NAME			0x0302
#define HCRP_FRIENDLY_NAME			0x0304

#define MPS_MPSD_SCENARIOS          0x0200
#define MPS_MPMD_SCENARIOS          0x0201
#define MPS_SUPPORT_PP              0x0202

/* Service type */
#define CLS_SERVER_SERVICE			0x1000
#define CLS_BROWSE_GROUP			0x1001
#define CLS_PUBLIC_BROWSE_GROUP		0x1002
#define CLS_SERIAL_PORT				0x1101
#define CLS_LAN_ACCESS				0x1102
#define CLS_DIALUP_NET				0x1103
#define CLS_IRMC_SYNC				0x1104
#define CLS_OBEX_OBJ_PUSH			0x1105
#define CLS_OBEX_FILE_TRANS			0x1106
#define CLS_IRMC_SYNC_CMD			0x1107
#define CLS_HEADSET					0x1108
#define CLS_CORDLESS_TELE			0x1109
#define CLS_AUDIO_SOURCE			0x110A	
#define CLS_AUDIO_SINK				0x110B
#define CLS_AVRCP_TG				0x110C
#define CLS_ADV_AUDIO_DISTRIB		0x110D
#define CLS_AVRCP_CT				0x110E
#define CLS_VIDEO_CONFERENCE		0x110F
#define CLS_INTERCOM				0x1110
#define CLS_FAX						0x1111
#define CLS_HEADSET_AG				0x1112
#define CLS_WAP						0x1113
#define CLS_WAP_CLIENT				0x1114
#define CLS_PAN_PANU				0x1115
#define CLS_PAN_NAP					0x1116
#define CLS_PAN_GN					0x1117
#define CLS_DIRECT_PRINT			0x1118
#define CLS_REF_PRINT				0x1119
#define CLS_IMAGING					0x111A
#define CLS_IMAG_RESPONDER			0x111B
#define CLS_IMAG_AUTO_ARCH			0x111C
#define CLS_IMAG_REF_OBJ			0x111D
#define CLS_HANDSFREE				0x111E
#define CLS_HANDSFREE_AG			0x111F
#define CLS_HCRP					0x1125
#define CLS_HCR_PRINT				0x1126
#define CLS_HCR_SCAN				0x1127
#define CLS_MPS                                 0x113B
#define CLS_PNP_INFO				0x1200
#define CLS_GENERIC_NET				0x1201
#define CLS_GENERIC_FILE_TRANS		0x1202
#define CLS_GENERIC_AUDIO			0x1203
#define CLS_GENERIC_TELE			0x1204

#define GRP_ROOT_ID					0x1002

/* PSM value */
#define PROTOCOL_SDP				0x0001
#define PROTOCOL_UDP				0x0002
#define PROTOCOL_RFCOMM				0x0003
#define PROTOCOL_TCP				0x0004
#define PROTOCOL_TCSBIN				0x0005
#define PROTOCOL_TCS_AT				0x0006
#define PROTOCOL_TCSBIN_LESS		0x0007
#define PROTOCOL_OBEX				0x0008
#define PROTOCOL_IP					0x0009
#define PROTOCOL_FTP				0x000A
#define PROTOCOL_HTTP				0x000C
#define PROTOCOL_WSP				0x000E
#define PROTOCOL_BNEP				0x000F
#define PROTOCOL_HIDP				0x0011
#define PROTOCOL_HCR_CONTROL		0x0012
#define PROTOCOL_HIDP_INTERRUPT		        0x0013
#define PROTOCOL_HCR_DATA			0x0014
#define PROTOCOL_HCR_NOTIFY			0x0016
#define PROTOCOL_AVCTP				0x0017
#define PROTOCOL_AVDTP				0x0019
#define PROTOCOL_L2CAP				0x0100

/* Possible value for LaguageBaseAttributeIDList attribute */
#define LANGUAGE_ENGLISH			0x656E
#define ENCODING_UFT8				0x006A
#define LANG_BASE_PRI				0x0100

/* Identification number of SDP UI request/conform pair */
#define SDP_UI_REGISTER			0
#define SDP_UI_UNREGISTER		1
#define SDP_UI_ALLOCATEHANDLE	2
#define SDP_UI_OPENSEARCH		3
#define SDP_UI_SERVICESEARCH	4
#define SDP_UI_CLOSESEARCH		5

/* Erro Code */
#define ERROR_SDPSPEC_INVALID_VERSION					0x0001		/*spec.*/
#define ERROR_SDPSPEC_INVALID_SERVICEHANDLE				0x0002		/*spec.*/
#define ERROR_SDPSPEC_INVALID_REQUEST_SYNTAX			0x0003		/*spec.*/
#define ERROR_SDPSPEC_INVALID_PDUSIZE					0x0004		/*spec.*/
#define ERROR_SDPSPEC_INVALID_CONTINUATIONSTATE 		0x0005		/*spec.*/
#define ERROR_SDPSPEC_INSUFFICIENT_RESOURCE				0x0006		/*spec.*/

#define UNKNOWN_ERROR_SDP					PROT_SDP_BASE+10						/*210*/

#define ERROR_SDP_INVALID_VERSION					UNKNOWN_ERROR_SDP + 0x0001		/*211*/		
#define ERROR_SDP_INVALID_SERVICEHANDLE				UNKNOWN_ERROR_SDP + 0x0002		/*212*/		
#define ERROR_SDP_INVALID_REQUEST_SYNTAX			UNKNOWN_ERROR_SDP + 0x0003		/*213*/		
#define ERROR_SDP_INVALID_PDUSIZE					UNKNOWN_ERROR_SDP + 0x0004		/*214*/		
#define ERROR_SDP_INVALID_CONTINUATIONSTATE 		UNKNOWN_ERROR_SDP + 0x0005		/*215*/		
#define ERROR_SDP_INSUFFICIENT_RESOURCE				UNKNOWN_ERROR_SDP + 0x0006		/*216*/		

#define ERROR_SDP_OPEN_PENDING				UNKNOWN_ERROR_SDP+7						/*217*/
#define ERROR_SDP_INVALID_TRANSHANDLE		UNKNOWN_ERROR_SDP+8						/*218*/
#define ERROR_SDP_CONFIG_FAILURE			UNKNOWN_ERROR_SDP+9						/*219*/
#define ERROR_SDP_TIMER_EXPIRED				UNKNOWN_ERROR_SDP+10					/*220*/
#define ERROR_SDP_EXCEEDING_MTU				UNKNOWN_ERROR_SDP+11					/*221*/
#define ERROR_SDP_INVALID_UUID				UNKNOWN_ERROR_SDP+12					/*222*/
#define ERROR_SDP_NO_MORE_SERVICE			UNKNOWN_ERROR_SDP+13					/*223*/
#define ERROR_SDP_INVALID_RESPONSE			UNKNOWN_ERROR_SDP+14					/*224*/
#define ERROR_SDP_INVALID_OPERATION			UNKNOWN_ERROR_SDP+15					/*225*/
#define ERROR_SDP_BUFFER_TOO_SMALL			UNKNOWN_ERROR_SDP+20					/*230*/

#endif

