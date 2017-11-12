/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
Module Name:
	l2cap_ui.h
Abstract:
	This module defines the l2cap high layer protocal interfaces, 
	structure and macro.
---------------------------------------------------------------------------*/

#ifndef G_L2CAP_UI_H
#define G_L2CAP_UI_H

#include "global.h"
#include "hci_ui.h"

typedef void (L2CAP_CbkFunc)(UINT16 msg, void *arg);/* callback to upper protocol */

enum {/* Callback event to the upper protocal */
	L2CAP_EV_CONNECT_IND = 0x01,				/* 0x01 */		
	L2CAP_EV_CONNECT_CFM,						/* 0x02 */	
	L2CAP_EV_CONNECT_COMPLETE,					/* 0x03 */
	L2CAP_EV_CONNECT_PARAM,						/* 0x04 */
	L2CAP_EV_CONFIG_IND,						/* 0x05 */	
	L2CAP_EV_DISCONNECT_COMPLETE,				/* 0x06 */
	L2CAP_EV_DATA_IND,							/* 0x07 */
	L2CAP_EV_DATA_CFM,							/* 0x08, notice different L2CAP mode use different CFM manner */
	L2CAP_EV_DYNAMIC_PSM,						/* 0x09, only local is server, use this. */
	L2CAP_EV_ECHO_CFM,							/* 0x0A Echo cfm */
	L2CAP_EV_FIXCHANNEL_CONNECT_COMPLETE		/* 0x0B */
};

/*---------------------------------------------------------------------------*/
/* 							L2CAP configuration options			   	  	     */
/*---------------------------------------------------------------------------*/
struct L2CAP_QosStru {
	UINT32 	token_rate;
	UINT32 	token_bucketsize;
	UINT32 	peak_bandwidth;
	UINT32 	latency;
	UINT32 	delay_variation;
	UINT8 	flags;
	UINT8 	service_type;
};

#define L2CAP_FORCE_MODE		0x01/* bit-mask of L2CAP_RtFcStru force_flag */

struct L2CAP_RtFcStru {
	UINT16 	txwindow_size;/* modified to 2bytes as to fit extended_window_size 14 bits */
	UINT16 	retrans_timeout;
	UINT16 	monitor_timeout;
	UINT16 	max_pdusize;
	UINT8	force_flag;/* Is mode force to use or not */
	UINT8 	mode;
	UINT8 	max_transmit;
};

struct L2CAP_ExtFcStru {
	UINT32 	sdu_interarrival_time;/* BE:May increase only, GUAR:None */
	UINT32 	access_latency;/* BE:None changes permitted by responder, GUAR:May decrease only */
	UINT32 	flush_timeout;/* BE:None changes permitted by responder, GUAR:May decrease only (unless set to 0xFFFFFFFF, in which case no change is permitted) */
	UINT16 	max_sdu;/* BE:May decrease only, GUAR:None */
	UINT8 	identifier;
	UINT8 	service_type;
};

struct L2CAP_ConfigOptionStru {
	UINT32 	mask;						/* etc: L2CAP_COTMASK_MTU, which option is used or not */
	UINT32 	hint;						/* etc: L2CAP_COTMASK_MTU, if this option is a hint or not */
	struct L2CAP_QosStru	qos_flow; 	/* type = 0x03 QUALITY OF SERVICE (QOS) OPTION */
	struct L2CAP_RtFcStru 	rt_fc;		/* type = 0x04 RETRANSMISSION AND FLOW CONTROL OPTION */
	struct L2CAP_ExtFcStru 	ext_flow; 	/* type = 0x05 EXTENDED FLOW SPECIFICATION OPTION */
	UINT16 	in_mtu;						/* type = 0x01 MAXIMUM TRANSMISSION UNIT (MTU) */
	UINT16 	flush_timeout;				/* type = 0x02 FLUSH TIMEOUT OPTION */
	UINT16 	max_windowsize;				/* type = 0x06 EXTENDED WINDOW SIZE OPTION */
	UINT8 	fcs_type;					/* etc: L2CAP_FCS_16BITFCS, type = 0x07 FRAME CHECK SEQUENCE (FCS) OPTION */	
};

struct L2CAP_ConfigUIStru {
	UINT16 	cid;
	UINT16 	config_flag;				/* continue flag, is only for ind->rsp direction, req->cfm will not fill */
	UINT16 	result;						/* for rsp input */	
	UINT8 	identifier;					/* this id is only for ind->rsp direction, req->cfm will not fill */	
	struct L2CAP_ConfigOptionStru option;
};

/*---------------------------------------------------------------------------*/
/* 								L2CAP UI Interface			       	 	     */
/*---------------------------------------------------------------------------*/
enum {/* amp_mask in L2CAP_ConnectStru amp_mask */
	L2CAP_CONNECTMASK_AMP_DESIRED = 		0x01,
	L2CAP_CONNECTMASK_AMP_CREATEDIRECT = 	0x02
};

struct L2CAP_ConnectStru {
	UINT16 	cid;						/* Local CID */
	UINT16 	psm;						/* Single PSM, the connection target PSM */
	UINT16 	result;						/* Only for rsp or cfm */
	UINT16 	status;						/* Only for rsp, result is pendding should fill this status */
	UINT8	amp_mask;					/* e.g L2CAP_CONNECTMASK_AMP_DESIRED */
	UINT8	*cbk;
	struct HCI_AddressStru addr;		/* Remote Addr */
	struct L2CAP_ConfigOptionStru config;	/* Only for connect req, rsp, input config_req param before config procedure */
};

struct L2CAP_PSMCbkStru {
	L2CAP_CbkFunc 	*cb;				/* Callback Function pointer */
	UINT16 			psm;				/* Protocol/Service Multiplexer, or Fixed-CID */
	UINT8			mask;				/* this PSM is it a fixed channel like ATT? */
};

enum {/* mask in struct L2CAP_PSMCbkStru */
	L2CAP_CBK_FIXED_CHANNEL = 0x01,		/* PSM value is fixed-CID, Parity used to check Fixed-CID or PSM type. */
	L2CAP_CBK_DYNAMIC_PSM = 0x04,		/* Only when local is server, use this */
	L2CAP_CBK_FIXED_PSM = 0x00				/* Fixed-PSM or Dynamic-PSM, used internal */
};

#define L2CAP_MIN_DYNAMIC_PSM											0x1001
#define L2CAP_FIXEDCHANNEL_SIGNALLING 									0x0001/* Fixed channels are available as soon as a BR/EDR the ACL-U or LE-U logical link transport is set up. */
#define L2CAP_FIXEDCHANNEL_UCD 											0x0002/* Fixed channels shall only run over BR/EDR ACL-U or LE-U logical links and shall not be moved. */
#define L2CAP_FIXEDCHANNEL_AMPMANAGER	 								0x0003
#define L2CAP_FIXEDCHANNEL_ATTRIBUTE_PROTOCOL							0x0004
#define L2CAP_FIXEDCHANNEL_LESIGNALING	 								0x0005/* If Fixed Channel LESignaling is supported then Fixed Channels ATT and SMP shall be supported */
#define L2CAP_FIXEDCHANNEL_SECURITY_MANAGER_PROTOCOL					0x0006/* TBD, LE */
#define L2CAP_FIXEDCHANNEL_AMPTEST										0x003F

enum {/* L2CAP channels CID */
	L2CAP_DYNAMICCHANNEL_MIN = 				0x0040,/* 1-0x003F are FIXED-Channels */
	L2CAP_DYNAMICCHANNEL_MAX = 				0xFFFF
};

struct L2CAP_DisconnectReqStru {
	UINT16 	cid;/* Local CID */
	UINT16 	psm;
};

struct L2CAP_DisconnectCompleteStru {
	UINT16 	cid;/* Local CID */
	UINT16	reason;/* error code, refer to ER_L2CAP_RTXTIMEOUT set... */
	struct HCI_AddressStru addr;/* Remote Addr */
	UINT8 	side;
};

struct L2CAP_ConnectParamStru {
	struct L2CAP_ConfigOptionStru incoming;/* Final configuration */
	struct L2CAP_ConfigOptionStru outgoing;
	UINT32 total_bandwidth;
	UINT32 max_guaranteed_bandwidth;
	UINT8  tl_type;/* etc: AMP_CONTROLLER_TYPE_BREDR */
};

struct L2CAP_ConnectCompleteStru {
	struct L2CAP_ConnectParamStru info;
	struct HCI_AddressStru addr;/* Remote Addr */
	UINT16 	cid;/* Local CID */
	UINT16 	psm;
	UINT16	result;/* 0 is success, error refer to ER_L2CAP_RTXTIMEOUT set... */
	UINT8	side;/* L2CAP_CHANNEL_INITIATOR or L2CAP_CHANNEL_RESPONDER */
};

struct L2CAP_ConnectParamChangeStru {
	struct L2CAP_ConnectParamStru info;
	UINT16 	cid;
};

struct L2CAP_FixedCIDConnectCompleteStru {
	struct HCI_AddressStru addr;/* Remote Addr */
	UINT32 	extfea_mask;
	UINT16 	hci_hdl;/* for data_req */
	UINT16	result;/* error code, refer to ER_L2CAP_RTXTIMEOUT set... */
	UINT16 	gmtu;
	UINT8 	fixchansup[8];/* 0x0000-0x003F CID bit-mask */
};

struct L2CAP_EchoStru {/* Req/Cfm only */
	struct HCI_AddressStru addr;/* Remote Addr */
	UINT8 	*cbk;
	UINT16 	length;
	UINT8 	data[1];
};

struct L2CAP_ConnectionParameterUpdateStru {
	struct HCI_AddressStru addr;/* Remote Addr */
	UINT16 interval_min;
	UINT16 interval_max;
	UINT16 slave_latency;
	UINT16 timeout_multiplier;
};

struct L2CAP_DataCfmStru {/* param of L2CAP_EV_DATA_CFM */
	UINT16 cid;
	UINT16 credit;
};

enum {/* Macros used in AMP GET INFO RESPONSE amp_status, also when EV_L2CAP_HCILOCALAMPCHANGED_IND */
	/* Add for HCI_AMP_STATUS_AVAILABLE_BUT_POWERED_DOWN set */
	L2CAP_AMPSTATUS_REMOVED = 0xEE,
	L2CAP_AMPSTATUS_NEWADD/* Only internal L2cap use */
};

enum {/* Channel establishment direction */
	L2CAP_CHANNEL_RESPONDER = 0,
	L2CAP_CHANNEL_INITIATOR = 1
};

enum {/* data transmission mode, get for ERT_MODE_OUTGOING/ERT_MODE_INCOMING */
	L2CAP_MODE_BASIC = 			0x00,/* Basic mode */
	L2CAP_MODE_RTX = 			0x01,/* Retransmision mode, DONOT USE */
	L2CAP_MODE_FC = 			0x02,/* Flow control mode, DONOT USE */
	L2CAP_MODE_ERTX = 			0x03,/* Enhanced Retransmission Mode */
	L2CAP_MODE_SM = 			0x04/* Streaming Mode */
};

enum {/* L2CAP Configuartion option type, used internal */
	L2CAP_COT_MTU = 			0x01,
	L2CAP_COT_FLUSHTIMEOUT = 	0x02,
	L2CAP_COT_QOS = 			0x03,
	L2CAP_COT_RTFC = 			0x04,
	L2CAP_COT_FCS = 			0x05,
	L2CAP_COT_EXTFLOWSPEC = 	0x06,/* ExtFlowSpec is used instead of FlushTO and QoS when both devices support ExtFLowSpec. */
	L2CAP_COT_EXTWINSIZE = 		0x07/* Notice: this is the last option we knows... */
};

/* L2CAP Configuration option type mask for upper protocol */
#define L2CAP_COTMASK_MTU							(1 << (L2CAP_COT_MTU - 1))
#define L2CAP_COTMASK_FLUSHTIMEOUT					(1 << (L2CAP_COT_FLUSHTIMEOUT - 1))
#define L2CAP_COTMASK_QOS							(1 << (L2CAP_COT_QOS - 1))
#define L2CAP_COTMASK_RTFC							(1 << (L2CAP_COT_RTFC - 1))
#define L2CAP_COTMASK_FCS							(1 << (L2CAP_COT_FCS - 1))
#define L2CAP_COTMASK_EXTFLOWSPEC					(1 << (L2CAP_COT_EXTFLOWSPEC - 1))/* The Extended Flow Specification for BR/EDR bit in the Extended Feature Mask is used to indicate support for BR/EDR QoS mode */
#define L2CAP_COTMASK_EXTWINSIZE					(1 << (L2CAP_COT_EXTWINSIZE - 1))
#define L2CAP_COTMASK_ALLKNOWN						(L2CAP_COTMASK_MTU | L2CAP_COTMASK_FLUSHTIMEOUT | L2CAP_COTMASK_QOS | L2CAP_COTMASK_RTFC | L2CAP_COTMASK_FCS | L2CAP_COTMASK_EXTFLOWSPEC | L2CAP_COTMASK_EXTWINSIZE)
/* (L2CAP_CONFIG_MAXOPTION_TYPEMASK << 1) - 1 */

enum {/* Connect Response result */
	L2CAP_CONNECT_SUCCESS = 						0x0000,
	L2CAP_CONNECT_PENDING = 						0x0001,
	L2CAP_CONNECT_PSM_NOTSUPPORTED = 				0x0002,
	L2CAP_CONNECT_SECURITYBLOCK = 					0x0003,
	L2CAP_CONNECT_NORESOURCESAVAILABLE = 			0x0004,
	L2CAP_CONNECT_CONTROLLERID_NOTSUPPORT = 		0x0005,
	L2CAP_CONNECT_INVALID_SOURCECID = 				0x0006,
	L2CAP_CONNECT_SOURCECID_ALREADY_ALLOCATED = 	0x0007
};

enum {/* Connect status */
	L2CAP_CONNECT_STATUS_NO_FURTHER = 				0x0000,
	L2CAP_CONNECT_STATUS_AUTHENTICATION_PENDDING = 	0x0001,
	L2CAP_CONNECT_STATUS_AUTHORIZATION_PENDDING = 	0x0002
};

enum {/* Configuration response result */
	L2CAP_CONFIG_RESULT_SUCCESS = 					0x0000,
	L2CAP_CONFIG_RESULT_UNACCEPTABLE_PARAM = 		0x0001,
	L2CAP_CONFIG_RESULT_REJECTED = 					0x0002,
	L2CAP_CONFIG_RESULT_UNKNOWN_OPTIONS = 			0x0003,
	L2CAP_CONFIG_RESULT_PENDDING = 					0x0004,
	L2CAP_CONFIG_RESULT_FLOWSPEC_REJECTED = 		0x0005
};

/* Configuration flag input */
#define L2CAP_CONFIG_FLAG_CONTINUE					0x0001

enum {/* L2CAP info type in info_req/info_rsp */
	L2CAP_INFOTYPE_CLMTU = 							0x0001,/* Data Len 2=1<<1 */
	L2CAP_INFOTYPE_EXTFEA = 						0x0002,/* Data Len 4=1<<2 */
	L2CAP_INFOTYPE_FIXCHANNEL = 					0x0003/* Data Len 8=1<<3 */
};

enum {/* L2CAP Extended Flow Spec Option service_type */
	L2CAP_EFS_SERVICETYPE_NOTRAFFIC = 				0x00,
	L2CAP_EFS_SERVICETYPE_BESTEFFORT = 				0x01,
	L2CAP_EFS_SERVICETYPE_GUARANTEED = 				0x02
};

enum {/* L2CAP OPT-FCS option */
	L2CAP_FCS_NOFCS = 								0x00,
	L2CAP_FCS_16BITFCS = 							0x01
};

enum {/* Session FSMDone reason, 0x00 leave to SUCCESS. */
	ER_L2CAP_SUCCESS = 								0x00,
	ER_L2CAP_RTXTIMEOUT = 							0x01, 			/* 0x01 */
	ER_L2CAP_BICONFIGTIMEOUT, 										/* 0x02 */
	ER_L2CAP_SECURITYBLOCKED_CONNECTREQ, 							/* 0x03 */
	ER_L2CAP_SECURITYBLOCKED_CONNECTIND, 							/* 0x04 */
	ER_L2CAP_CONNECTRSP_PSM_NOTSUPPORTED,							/* 0x05, L2CAP_CONNECT_PSM_NOTSUPPORTED 0x0002 */
	ER_L2CAP_CONNECTRSP_SECURITYBLOCK,								/* 0x06, L2CAP_CONNECT_SECURITYBLOCK 0x0003 */
	ER_L2CAP_CONNECTRSP_NORESOURCESAVAILABLE,						/* 0x07, L2CAP_CONNECT_NORESOURCESAVAILABLE 0x0004 */
	ER_L2CAP_CONNECTRSP_CONTROLLERID_NOTSUPPORT,					/* 0x08, L2CAP_CONNECT_CONTROLLERID_NOTSUPPORT 0x0005 */
	ER_L2CAP_CONNECTCFM_PSM_NOTSUPPORTED,							/* 0x09, L2CAP_CONNECT_PSM_NOTSUPPORTED 0x0002 */
	ER_L2CAP_CONNECTCFM_SECURITYBLOCK,								/* 0x0A, L2CAP_CONNECT_SECURITYBLOCK 0x0003 */
	ER_L2CAP_CONNECTCFM_NORESOURCESAVAILABLE,						/* 0x0B, L2CAP_CONNECT_NORESOURCESAVAILABLE 0x0004 */
	ER_L2CAP_CONNECTCFM_CONTROLLERID_NOTSUPPORT,					/* 0x0C, L2CAP_CONNECT_CONTROLLERID_NOTSUPPORT 0x0005 */
	ER_L2CAP_DISCONNECTIND, 										/* 0x0D */
	ER_L2CAP_HCIDISCONNECTIND, 										/* 0x0E */
	ER_L2CAP_DISCONNECTCFM, 										/* 0x0F */
	ER_L2CAP_PSMUNREG, 												/* 0x10 */
	ER_L2CAP_CMDREJ,												/* 0x11 */
	ER_L2CAP_FORCE_MODE_NOTSUPPORT,									/* 0x12 */
	ER_L2CAP_MOVE_FAILED_BREDR,										/* 0x13 Move to BREDR0 but refused by either side */

	/* Disconnect Reason */
	ER_L2CAP_AMP_CONTROLLER_LOST,									/* 0x14 */
	ER_L2CAP_ERT_DISCONNECT,										/* 0x15 */
	ER_L2CAP_CONFIG_UNACCEPT_REQCFM_TWICE,							/* 0x16 */
	ER_L2CAP_CONFIG_CFM_ERROR,										/* 0x17 */
	ER_L2CAP_CONFIG_SUCCESS_BEFORE_PENDDING,						/* 0x18 */
	ER_L2CAP_CONFIG_UNKNOWN_NOTHINT,								/* 0x19 */
	ER_L2CAP_CONFIG_REJECTED,										/* 0x1A */
	ER_L2CAP_CONFIG_UNACCEPT_INDRSP_TWICE,							/* 0x1B */
	ER_L2CAP_DISCONNECT_REQ,										/* 0x1C, Upper */

	/* Others only for callback */
	ER_L2CAP_DISCONNECT_REQ_CID_NOTFOUND,							/* 0x1D */
	ER_L2CAP_FIXED_CID_NOTSUPPORT,									/* 0x1E */
	ER_L2CAP_SESSION_CANNOTBE_ALLOCATED,							/* 0x1F */
	ER_L2CAP_DONE													/* 0x20 */
};

/*---------------------------------------------------------------------------*/
/* 								L2CAP UI Function			         	     */
/*---------------------------------------------------------------------------*/
FUNC_EXPORT void L2CAP_Init(void);
FUNC_EXPORT void L2CAP_Done(void);
FUNC_EXPORT void L2CAP_RegCbk(UINT16 psm, UINT8 *cb, UINT8 mask);
FUNC_EXPORT void L2CAP_UnRegCbk(UINT16 psm, UINT8 mask);
FUNC_EXPORT void L2CAP_ConnectReq(struct L2CAP_ConnectStru *in);
FUNC_EXPORT void L2CAP_ConnectRsp(struct L2CAP_ConnectStru *in);
FUNC_EXPORT void L2CAP_ConfigReq(struct L2CAP_ConfigUIStru *in);
FUNC_EXPORT void L2CAP_ConfigRsp(struct L2CAP_ConfigUIStru *in);
FUNC_EXPORT void L2CAP_DisconnectReq(struct L2CAP_DisconnectReqStru *in);
FUNC_EXPORT void L2CAP_DataReq(struct BuffStru *in, UINT16 cid);
FUNC_EXPORT void L2CAP_FixCIDDataReq(struct BuffStru *in, UINT32 cid_hcihdl);
FUNC_EXPORT void L2CAP_EchoReq(struct L2CAP_EchoStru *in);
FUNC_EXPORT UINT16 L2CAP_GetHCIHDL(UINT16 cid);
FUNC_EXPORT void L2CAP_Connection_Parameter_Update_Req(struct L2CAP_ConnectionParameterUpdateStru *in);

#endif

