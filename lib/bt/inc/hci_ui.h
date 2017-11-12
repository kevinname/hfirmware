/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
Module Name:
	hci_ui.h
Abstract:
	This module defines HCI interface structure, macro & functions.
---------------------------------------------------------------------------*/

#ifndef G_HCI_UI_H
#define G_HCI_UI_H

#include "hci_stru.h"

#ifdef CONFIG_DEBUG 
//#define HCI_DBG_ALL 					1/* Use to control hci_dbg.c compile */
#endif/* #ifdef CONFIG_DEBUG  */

/*---------------------------------------------------------------------------*/
/* 						HCI Transport Layer Interface		       	  	     */
/*---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
In the function Func_Startup & Func_StartSco :
	if return values is 0, means successfully;	
	if return values is not 0, means not successfully;
In the function Func_Send :
	return values is the number of bytes you send successfully;	
In the function Func_GetType :
	return values is the AMP type MACROs;	
-----------------------------------------------------------------------------*/
typedef int (Func_Receive) 				(HANDLE host_hdl, UINT8 *hdr, UINT8 hdr_len, UINT8 *pdu, UINT32 pdu_len);

typedef struct _TL_CallbackStru
{
	Func_Receive *Receive;	
} TL_CallbackStru;

typedef int (Func_Startup) 				(HANDLE tl_hdl, TL_CallbackStru *pCallback, HANDLE host_hdl);
typedef void (Func_Shutdown) 			(HANDLE tl_hdl);
typedef int (Func_Send) 				(HANDLE tl_hdl, UINT8 *pBuf, UINT32 len);
typedef int (Func_StartSco) 			(HANDLE tl_hdl, UINT16 sco_hdl);
typedef void (Func_StopSco) 			(HANDLE tl_hdl, UINT16 sco_hdl);
typedef unsigned char (Func_GetType) 	(HANDLE tl_hdl);

typedef struct _TransportLayerStru
{
	Func_GetType *GetType;
	Func_Startup *Startup;
	Func_Shutdown *Shutdown;
	Func_Send *Send;
	Func_StartSco *StartSco;
	Func_StopSco *StopSco;
} TransportLayerStru;

#define HCI_AMPID_BREDR  						0x00/* type/id in struct HCI_LocalStru */
#define HCI_OPCODE(g, c)						(((g) << 10) | (c))
#define HCI_OGF(o)								((UINT8)((o) >> 10))
#define HCI_OCF(o)								((o) & ((1 << 10) - 1))

enum {/* OGF for Command Code */
	HCI_OGF_LINKCONTROL = 						0x01,
	HCI_OGF_LINKPOLICY = 						0x02,
	HCI_OGF_CONTROLLER_BASEBAND =				0x03,
	HCI_OGF_INFORMATIONAL_PARAM = 				0x04,
	HCI_OGF_STATUS_PARAM = 						0x05,
	HCI_OGF_TESTING = 							0x06,
	HCI_OGF_LE = 								0x08,
	HCI_OGF_VENDOR = 							0x3F
};

/*---------------------------------------------------------------------------*/
/* 							HCI UI Interface structure		       	  	     */
/*---------------------------------------------------------------------------*/
enum {/* cap in struct HCI_AddressStru */
	HCI_ADDRESS_CAP_LE = 						0x01,
	HCI_ADDRESS_CAP_BREDR = 					0x02,
	HCI_ADDRESS_CAP_MAIN = 						(HCI_ADDRESS_CAP_BREDR | HCI_ADDRESS_CAP_LE),/* AMPID = 0 */
	HCI_ADDRESS_CAP_80211 = 					0x04,
	HCI_ADDRESS_CAP_UWB = 						0x08,
	HCI_ADDRESS_CAP_AMP = 						(HCI_ADDRESS_CAP_80211 | HCI_ADDRESS_CAP_UWB)/* AMPID != 0 */
};

struct HCI_AddressStru {
	UINT8 cap;									/* HCI_ADDRESS_CAP_LE */
	UINT8 atype; 								/* HCI_OWN_ADDRESS_TYPE_PUBLIC_DEVICE_ADDRESS */
	UINT8 bd[BD_ADDR_LEN];						/* 6octets, 48bits */
};

struct HCI_ACLConnection_ResponseUIStru {		/* UI Input in HCI_ACLConnection_Response */
	struct HCI_Connection_RequestEvStru *ev;
	UINT32 opscode;								/* etc: HCI_OPS_REJECT_CONNECTION_REQUEST, HCI_OPS_ACCEPT_CONNECTION_REQUEST */
	UINT8 ampid;
};

struct HCI_Synchronous_ConnectionStru {			/* Common part for HCI_Set_SynchronousParam */
	UINT32 transmit_bandwidth;
	UINT32 receive_bandwidth;
	UINT16 max_latency;
	UINT16 voice_setting;
	UINT8 retransmission_effort;
	UINT16 packet_type;
};

struct HCI_Synchronous_Connection_ResponseUIStru {/* UI Input in HCI_Synchronous_Connection_Response */
	struct HCI_Connection_RequestEvStru *ev;	/* Event received */
	struct HCI_Synchronous_ConnectionStru par;	/* eSCO Parameters */
	UINT32 opscode;								/* etc: HCI_OPS_REJECT_SYNCHRONOUS_CONNECTION_REQUEST, HCI_OPS_ACCEPT_SYNCHRONOUS_CONNECTION_REQUEST */
	UINT8 ampid;
};

struct HCI_Create_SCOUIStru {					/* UI Input in HCI_Synchronous_Connection_Response */
	struct HCI_AddressStru addr;				/* Remote addr */
	struct HCI_Synchronous_ConnectionStru par;	/* eSCO Parameters */
};

/*-----------------------------------------------------------------------------
Description:
	Upper protocol callback function for HCI.
	hdl: 		(arg length(2octets) << 16) | ampid(1octet).
	context: 	upper defined address.
	arg: 		parameter for this op_ev.
	op_ev:		(opcode(passive is 0x0000) << 16) | evindex(include HCI_EVO_ or HCI_EVI)
-----------------------------------------------------------------------------*/
typedef void (HCI_CbkFunc)(HANDLE hdl, void *context, void *arg, UINT32 op_ev);

struct HCI_CbkLineStru {/* UI cbk func jump martix */
	UINT16 id;				/* HCI_OPC_ or HCI_EVI_(EVO) */
	void *func;				/* HCI_CbkFunc for external, HCI_CbkTableFunc for internal */
};

struct HCI_CbkNodeStru {
	const struct HCI_CbkLineStru *table;
	UINT32 size;
};

struct HCI_CommandErrorStru {					/* cbk arg for HCI_EVO_COMMAND_ERROR, also for L2 command finish/cancel */
	UINT16 status;								/* Extended by HCI_STATUS_NO_FLOWCONTROL, include COMMAND_STATUS failed */
	UINT16 opcode;								/* HCI_OPC_ */
	UINT8 req[1];								/* Varable length, visit as void * */
};

/*-----------------------------------------------------------------------------
Description:
	Level 2 Command Node Queue finished Callback.
-----------------------------------------------------------------------------*/
typedef void (HCI_L2QueueCbkFunc)(HANDLE hdl, void *context, struct HCI_CommandErrorStru *arg, UINT16 len, UINT16 opcode/* The last one */);

struct HCI_CommandQueueStru {					/* Level 2 command queue node */
	struct ListStru list;						/* node is struct HCI_CommandQueueNodeStru */
	HCI_L2QueueCbkFunc *cbk;					/* Use to cbk when queue finished */
	void *context;								/* cbk arg by caller */
	UINT8 mask;									/* Reserved, temp for ampid */
};

enum {/* mask in struct HCI_CommandQueueNodeStru */
	HCI_CMDLIST_NODE_MANDATORY = 	0x01,		/* set this if this cmd failed, all queue cmds will be failed. not set for OPTIONAL */
	HCI_CMDLIST_NODE_PAR_FREEBYL1 = 0x02		/* command already goto Level 1 sys, par pointer keeps for match L2 to L1 */
};

struct HCI_CommandQueueNodeStru {
	void *par;									/* L2 do not free this, deliver to L1 core */
	UINT32 opscode;								/* etc: HCI_OPS_ACCEPT_CONNECTION_REQUEST, req << 24 | ret or evi << 16 | opcode | mask */
	UINT8 mask;									/* etc: HCI_CMDLIST_NODE_MANDATORY */
};

union HCI_Set_Event_Filter_ConditionInUnion {
	struct HCI_Set_Event_Filter_ConditionInStru {
		UINT8 class_of_device[3];
		UINT8 class_of_device_mask[3];
	} c;
	UINT8 bd[6];
};

struct HCI_Set_Event_FilterInStru {				/* UI Interface for HCI_Set_Event_Filter */
	UINT8 filter_type;
	UINT8 filter_condition_type;				/* optional */
	union HCI_Set_Event_Filter_ConditionInUnion condition;
	UINT8 auto_accept_flag;						/* optional */
};

enum {/* flag in struct HCI_InquiryInStru */
	HCI_INQUIRY_FLAG_ENTER_PERIOIDIC_MODE = 	0,
	HCI_INQUIRY_FLAG_EXIT_PERIOIDIC_MODE = 		1,
	HCI_INQUIRY_FLAG_INQUIRY = 					2,
	HCI_INQUIRY_FLAG_INQUIRY_CANCEL = 			3,

	HCI_INQUIRY_FLAG_CLASS_OF_DEVICE = 			0x80,/* Set_Event_Filter */
	HCI_INQUIRY_FLAG_BD = 						0x40,/* Set_Event_Filter */
	HCI_INQUIRY_FLAG_SET_EVENT_FILTER = 		(HCI_INQUIRY_FLAG_CLASS_OF_DEVICE | HCI_INQUIRY_FLAG_BD)
};

struct HCI_InquiryInStru {						/* input arg for HCI_Inquiry */
	void *cbk;
	void *context;
	UINT16 max_period_len;
	UINT16 min_period_len;
	UINT8 lap[3];
	UINT8 inquiry_len;
	UINT8 num_responses;

	UINT8 ampid;								/* Only for Primy BREDR 0? */
	UINT8 flag;									/* HCI_INQUIRY_FLAG_xxx */
	union HCI_Set_Event_Filter_ConditionInUnion condition;
};

struct HCI_EIR_DataNodeStru {/* Node in HCI_Extended_Inquiry_Response_Decode */
	UINT8 len;				/* 1 type + x length */
	UINT8 eir_data_type;	/* etc:HCI_EIR_DATATYPE_FLAGS */
	union {

		/* HCI_EIR_DATATYPE_FLAGS */
		UINT8 flags;

		/* HCI_EIR_DATATYPE_MORE_16BIT_SERVICE_UUIDS, 
		HCI_EIR_DATATYPE_COMPLETE_LIST_OF_16BIT_SERVICE_UUIDS, 
		HCI_EIR_DATATYPE_SERVICE_SOLICITATION_LIST_OF_16_BIT_SERVICE_UUIDS */
		UINT8 uuid16[2];

		/* HCI_EIR_DATATYPE_MORE_32BIT_SERVICE_UUIDS, 
		HCI_EIR_DATATYPE_COMPLETE_LIST_OF_32BIT_SERVICE_UUIDS */
		UINT8 uuid32[4];

		/* HCI_EIR_DATATYPE_MORE_128BIT_SERVICE_UUIDS, 
		HCI_EIR_DATATYPE_COMPLETE_LIST_OF_128BIT_SERVICE_UUIDS, 
		HCI_EIR_DATATYPE_SERVICE_SOLICITATION_LIST_OF_128_BIT_SERVICE_UUIDS */
		UINT8 uuid128[16];

		/* HCI_EIR_DATATYPE_SHORTENED_LOCAL_NAME, 
		HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME */
		struct HCI_EIR_DataNameStru {
			UINT8 data[1];/* len - 1 octets */
		} name;

		/* HCI_EIR_DATATYPE_TX_POWER_LEVEL */
		UINT8 tx_power_level;/* -127 to +127dBm */

		UINT8 class_of_device[3];/* HCI_EIR_DATATYPE_SIMPLE_PAIRING_CLASS_OF_DEVICE */
		UINT8 simple_pairing_hash_c[16];/* HCI_EIR_DATATYPE_SIMPLE_PAIRING_HASH_C */
		UINT8 simple_pairing_randomizer_r[16];/* HCI_EIR_DATATYPE_SIMPLE_PAIRING_RANDOMIZER_R */
		UINT8 tk_value[16];/* 128bits, Value as used in pairing over LE Physical channel HCI_EIR_DATATYPE_TK_VALUE */
		UINT8 oob_flags;/* HCI_EIR_DATATYPE_SECURITY_MANAGER_OOB_FLAGS */

		UINT8 addr[6]; /* HCI_EIR_DATATYPE_PUBLIC_TARGET_ADDRESS/HCI_EIR_DATATYPE_RANDOM_TARGET_ADDRESS */

		UINT8 appearence[2]; /* HCI_EIR_DATATYPE_APPEARANCE */
		
		/* HCI_EIR_DATATYPE_SLAVE_CONNECTION_INTERVAL_RANGE */
		struct HCI_EIR_DataSlaveConnectionIntervalRangeStru {
			UINT8 conn_interval_min[2];
			UINT8 conn_interval_max[2];
		} slave_connection_interval_range;/* The first 2 octets defines the minimum value for the connection interval, the second 2oct for maximum */

		/* HCI_EIR_DATATYPE_SERVICE_DATA */
		struct HCI_EIR_DataServiceDataStru {
			UINT8 uuid16[2];
			UINT8 data[1];/* len - 1 - 2 octets */
		} service_data;

		/* HCI_EIR_DATATYPE_MANUFACTURER_SPECIFIC_DATA */
		struct HCI_EIR_DataManufacturerDataStru {
			UINT8 company_identifier_code[2];
			UINT8 data[1];/* len - 1 - 2 octets */
		} manufacturer_data;
	} u;
};/* all member UINT8, so no pendding at all */

/*---------------------------------------------------------------------------*/
/* 							Callback for HCI_EVO_Events		       	  	     */
/*---------------------------------------------------------------------------*/
enum {
	HCI_LOCALCONTROLLER_TLREG_FINISH,/* Local born without buffer size */
	HCI_LOCALCONTROLLER_REMOVE,
	HCI_LOCALCONTROLLER_NOTIFY_BUFFER_SIZE,
	HCI_LOCALCONTROLLER_NOTIFY_AMPSTATUS,
	HCI_LOCALCONTROLLER_NOTIFY_AMPINFO,
	HCI_LOCALCONTROLLER_NOTIFY_BREDR_LE_CAP
};

struct HCI_LocalController_ChangedStru {	/* par of HCI_EVO_LOCALCONTROLLER_CHANGED */
	HANDLE tl_hdl;
	struct HCI_AddressStru addr;			/* Local Read_BDADDR for BD and feature to get cap */
	UINT16 max_pdusize;						/* 1500 */
	UINT8 ampid;							/* HCI_AMPID_BREDR */
	UINT8 action;							/* etc: HCI_LOCALCONTROLLER_TLREG_FINISH */
	UINT8 amp_status;						/* HCI_AMP_STATUS_NO_CAPACITY */
	UINT8 device_type;						/* HCI_DEVICE_TYPE_BREDR, AMP_CONTROLLER_TYPE_BREDR */
};

#define HCI_MPN		2 /* 2*uint,64bits,enough? */

typedef struct {
	UINT32 num[HCI_MPN];
} HCI_MPInt;

enum {/* dcounter[x] in struct HCI_LocalStru */
	HCI_DCOUNTER_SEND,
	HCI_DCOUNTER_RECV
};

struct HCI_InfoRead_LocalStru {
	struct HCI_AddressStru addr;			/* HCI_ADDR_PUBLIC_STATIC only? */
	HCI_MPInt dcounter[2];					/* etc: HCI_DCOUNTER_SEND */
	UINT8 hci_version;
	UINT8 ampid;
	UINT8 lmp_features[2][8];
};

struct HCI_Remote_InfoStru {
	struct HCI_AddressStru addr;
	UINT8 class_of_device[3];

	/* BR/EDR Range: -128  <= N <= 127 (signed integer) Units: dB */
	/* AMP: Range: AMP type specific (signed integer)  Units: dBm */
	/* LE: Range: -127 to 20, 127 (signed integer) Units: dBm */
	UINT8 rssi;
	UINT8 eir_len;
	UINT8 eir[1];							/* Variable length */
};

struct HCI_L2CAP_DataCfmStru {				/* QoS DataCfm */
	UINT16 cid;
	UINT16 octets;
};

struct HCI_InfoRead_LinkSecurityStru {		/* = struct HCI_ACLLinkSecurityStru part */
	UINT16 mask;							/* etc: HCI_SECURITY_AUTHENTICATE, HCI_SECURITY_LINKKEY */
};

enum {/* result of struct HCI_Security_AuthorReqStru */
	HCI_SECURITY_AUTHOR_PASS,
	HCI_SECURITY_AUTHOR_NOTALLOW
};

struct HCI_Security_AuthorReqStru {			/* par of HCI_EVO_AUTHORIZATION_REQUEST */
	struct HCI_AddressStru addr[2];			/* x in HCI_LOCAL */
	UINT16 hcihdl;							/* Based on a link object */
	UINT16 channel; 						/* PSM for L2CAP; Server channel for RFCOMM */
	UINT8 ampid;							/* Local AMPID */
	UINT8 proto_id;							/* etc: PROT_L2CAP, PROT_RFCOMM */
	UINT8 result;							/* etc: HCI_SECURITY_AUTHOR_PASS */
};
	
enum {/* HCI Command Error Internal reason */
	HCI_STATUS_NO_FLOWCONTROL = 0x0100,		/* for HCI_OPC_HOST_NUMBER_OF_COMPLETED_PACKETS */
	HCI_STATUS_NOT_SUPPORTED,
	HCI_STATUS_RTX,
	HCI_STATUS_CANCELED,
	HCI_STATUS_LOCAL_CONTROLLER_REMOVED,
	HCI_STATUS_MITM_NOT_SATISIFIED			/* Were requested for MITM, but for IO or other reason, result is non_MITM */
};

enum {/* For HCI_Cbk Additional Event section in struct HCI_EvCBKStru, added enum for HCI_EVI_xxx */
	HCI_EVO_ACLLINK = HCI_EVI_LE_LONG_TERM_KEY_REQUEST + 1,/* 0x52, Create/Delete/1st Ready, struct HCI_LinkCbkStru */
	HCI_EVO_LELINK,										/* 0x53, LE, struct HCI_LinkCbkStru */
	HCI_EVO_ESCOLINK,									/* 0x54, eSCO, HCI_EVI_CONNECTION_REQUEST, waiting for Connection_Response */
	HCI_EVO_LOCALCONTROLLER_CHANGED,					/* 0x55, struct HCI_LocalController_ChangedStru */
	HCI_EVO_REMOTECONTROLLER_CHANGED,					/* 0x56, struct HCI_Remote_InfoStru */
	HCI_EVO_ACL_DATA_IND,								/* 0x57, struct BuffStru */
	HCI_EVO_PHY_DATA_IND,								/* 0x58, struct BuffStru */
	HCI_EVO_LE_DATA_IND,								/* 0x59, struct BuffStru */
	HCI_EVO_SCO_DATA_IND,								/* 0x5A, struct BuffStru */

	/* HCI Internal filter event from EVI_ */
	HCI_EVO_COMMAND_ERROR,								/* 0x5B, Command Terminate reason for OpCbk, struct HCI_CommandErrorStru */
	/* Shall Register OPC(Cover Error) before EVI(Cover Passive) with one function */
	
	HCI_EVO_AUTHORIZATION_REQUEST,						/* 0x5C, struct HCI_Security_AuthorReqStru */
	HCI_EVO_AUTHORIZATION_ABORT,						/* 0x5D, struct HCI_Security_AuthorReqStru */
	HCI_EVO_SECURITY_DISPLAY,							/* 0x5E, struct HCI_User_Confirmation_RequestEvStru, Popup a window and display something */
	HCI_EVO_LINK_KEY_SAVE,								/* 0x5F, struct HCI_Security_Link_Key_SaveStru, app to save bonded linkkey */
	HCI_EVO_LINK_KEY_ASK4,								/* 0x60, struct HCI_Security_Link_Key_Ask4Stru, ask for linkkey */
	HCI_EVO_PIN_CODE,									/* 0x61, struct HCI_PIN_Code_RequestEvStru, ask for PINCode */
	HCI_EVO_COMMAND_COMPLETE,							/* 0x62, return parameter in command_complete */
	HCI_EVO_LOOPBACK,									/* 0x63, struct HCI_Loopback_CommandEvStru */
	HCI_EVO_L2CAP_DATACFM								/* 0x64, struct HCI_L2CAP_DataCfmStru */
};

enum {/* Bitmask in HCI_ACLLink_Ref */
	HCI_ACLLINK_ACLOBJECT = 							0x8000,/* bd input is the acllink object, no need to find again */
	HCI_ACLLINK_LE = 									0x4000,

	/* UI interface */
	HCI_ACLLINK_REF_BREDR = 							(0x0001),
	HCI_ACLLINK_REF_LE = 								(0x0002 | HCI_ACLLINK_LE)
};

enum {/* Job in struct HCI_LinkCbkStru, for HCI_EVO_LELINK & HCI_EVO_ACLLINK */
	HCI_LINKCBK_CREATE,					/* To create a shadow obj */
	HCI_LINKCBK_DELETE,					/* Disconnected, kill shadow obj */
	HCI_LINKCBK_READY					/* 1st time connection ready */
};

enum {/* mask in HCI_Set_Channel_Classification */
	HCI_SETAFH_OCCUPY = 	0x01,/* otherwise release */
	HCI_SETAFH_SENDCMD = 	0x02/* or later */
};

enum {/* mask in struct HCI_LinkCbkStru */
	HCI_LEROLE_MASTER = 	0x02/* LE Master, otherwise is LE Slave, LE only */
};

enum {/* addr[x] in struct HCI_LinkCbkStru */
	HCI_LOCAL = 		0,
	HCI_REMOTE = 		1
};

struct HCI_LinkCbkStru {				/* Cbk arg of HCI_ACLLink_Cbk */
	void *hci;							/* Host link handle */
	UINT16 hdl;							/* Controller link handle */
	struct HCI_AddressStru addr[2];		/* HCI_LOCAL(Current used) & HCI_REMOTE(Public/Static Prefered, or CurLink) */
	UINT8 mask;							/* etc: HCI_LEROLE_MASTER, This is only valid when ready */
	UINT8 job;							/* etc: HCI_LINKCBK_CREATE */
	UINT8 reason;						/* etc: HCI_STATUS_CONNECTION_FAILED_TO_BE_ESTABLISHED */
};

struct HCI_LE_Advertising_ReportSubEvStru {/* Decoded struct HCI_LE_Advertising_ReportEvStru */
	UINT8 leevent_type;
	UINT8 address_type;
	UINT8 rssi;
	UINT8 address[6];
	UINT8 length_data;
	UINT8 data[1];						/* length_data octets */
};

struct HCI_PhysicalLinkStru {			/* in of HCI_PhysicalLink */
	void *acllink;						/* As handle */
	UINT8 *assoc;						/* Remote assoc, sub mem */
	UINT32 opscode;						/* HCI_OPS_ACCEPT_PHYSICAL_LINK or HCI_OPS_CREATE_PHYSICAL_LINK */
	UINT16 assoc_len;
	UINT8 phyhdl;
	UINT8 ampid;
};

enum {/* mode in struct HCI_SecuritySetModeStru */
	HCI_SECURITY_MODE1,					/* No Security, Forbidden in 2.1, expection is SDP */
	HCI_SECURITY_MODE2,					/* Service Level Security, 3 & 1 is part of 2, so treat as 2 */
	HCI_SECURITY_MODE3,					/* Link Level Security, HCI_OPS_WRITE_AUTHENTICATION_ENABLE */
	HCI_SECURITY_MODE4					/* Mode2, but Forced to Auth/Encrypt, expection is SDP, HCI_OPC_WRITE_SIMPLE_PAIRING_MODE */
};

struct HCI_Security_LocalIOStru {			/* Global Security IO Capabilities */
	UINT8 io_capability;					/* etc: HCI_IO_CAPABILITY_DISPLAYONLY */
};

struct HCI_Security_LocalBondableStru {		/* Local Controller Security Mode */
	UINT8 controller_id;					/* AMP ID */
	UINT8 bondable;							/* TRUE/FALSE */
};

struct HCI_Security_RemoteOOBStru {			/* Remote Controller Security OOB data */
	struct HCI_AddressStru addr;			/* Remote Addr */
	UINT8 oob_data[2][16];					/* Remote OOB data, C/R */
};

enum {/* mask in struct HCI_SecurityNodeStru */
	HCI_SECURITY_AUTHENTICATE = 		0x0001,				/* authentication, ask this device is allowed or not? */
	HCI_SECURITY_AUTHORIZE = 			0x0002,				/* authorization, popup to ask if this PSM is allowed? */
	HCI_SECURITY_ENCRYPT = 				0x0004,				/* encryption */
	HCI_SECURITY_AUTHENTICATE_MITM = 	(0x0001 | 0x0008),	/* authentication with keytype check for MITM */
	HCI_SECURITY_INCOMING = 			0x0010,				/* Incoming channel */
	
	/* Other internal masks */
	HCI_SECURITY_AUTHORIZE_CBK_GO = 	0x8000,				/* Authorization callback ongoing, for abort, only for require node */
	HCI_SECURITY_AUTHENTICATE_GO = 		0x4000,				/* Require node already go authentication, do not do again */
	HCI_SECURITY_LINKKEY = 				0x1000,				/* LinkKey validation, go for authencation for it */
	HCI_SECURITY_BONDED	= 				0x0400,
	HCI_SECURITY_BONDABLE = 			0x0200
};

enum {/* Result in Security callback */
	HCI_SECURITY_CBK_COMPLETE = 		0,
	HCI_SECURITY_CBK_PENDDING = 		(1 << 16)
};

struct HCI_SecurityInfoStru {	/* node in HCI_SECURITYLIST */
	UINT16 channel;				/* PSM or server channel */
	UINT16 mask;				/* etc: HCI_SECURITY_AUTHENTICATE */
	UINT8 proto_id;				/* etc: PROT_L2CAP */
};

enum {/* Bond in struct HCI_SecurityNodeStru */
	HCI_BOND_NOBOND = 		0,	/* No Bond */
	HCI_BOND_DEDICATED = 	1,	/* Without a L2CAP/RFCOMM channel */
	HCI_BOND_GENERAL = 		2	/* With a L2CAP/RFCOMM channel */
};

struct HCI_SecurityNodeStru {			/* node in link->sec.reqlist */
	struct HCI_SecurityInfoStru info;	/* Must in 1st item for searching */
	void *cbk;							/* Security Caller cbk, HCI_SMCbkFunc *, but dead-lock called, so void *, for KEIL compile reason */
	UINT16 hcihdl;						/* Based on a link object */
	UINT8 ampid;						/* Local AMPID */
	UINT8 bond;							/* etc: HCI_BOND_DEDICATED */
};

/*-----------------------------------------------------------------------------
Description:
	Security Request ListNode Callback.
-----------------------------------------------------------------------------*/
typedef void (HCI_SMCbkFunc)(struct HCI_SecurityNodeStru *node, UINT32 result);

struct HCI_Security_Link_Key_Request_ReplyStru {/* in of HCI_Security_LinkKey_Reply */
	UINT8 bd[BD_ADDR_LEN];				/* [123],Val */
	UINT8 link_key[16]; 				/* [173],Val */
	UINT8 key_type;						/* etc: HCI_KEY_TYPE_COMBINATION_KEY */
	UINT8 accept;						/* TRUE for Reply, FALSE for Negative Reply */
};

struct HCI_Security_Link_Key_Ask4Stru {/* HCI_EVO_LINK_KEY_ASK4 */
	UINT8 local_bd[BD_ADDR_LEN];		/* [123],Val */
	UINT8 remote_bd[BD_ADDR_LEN];		/* [123],Val */
};

struct HCI_Security_Link_Key_SaveStru {/* HCI_EVO_LINK_KEY_SAVE */
	UINT8 local_bd[BD_ADDR_LEN];		/* [123],Val */
	UINT8 remote_bd[BD_ADDR_LEN];		/* [123],Val */
	UINT8 link_key[16]; 				/* [173],Val */
	UINT8 key_type;						/* etc: HCI_KEY_TYPE_COMBINATION_KEY */
};

/* Macro components of HCI_OPS_ */
#define HCI_OPS2MASK(ops_)						((ops_) & 0x8000)/* Ret struinfo offset or EVI selector bitmask */
#define HCI_OPS2OPC(ops_)						((UINT16)((ops_) & 0x7FFF))/* OPCode, remove 0x8000 bitmask */
#define HCI_OPS2REQOFF(ops_)					((UINT8)((ops_) >> 16))/* Req struinfo offset */
#define HCI_OPS2RETEVI(ops_)					((UINT8)((ops_) >> 24))/* Ret struinfo offset or EVI */

#define HCI_SET_OFF(p, evindex_)				((p)[(evindex_) >> 3] &= ~((1 << ((evindex_) & 7))))
#define HCI_SET_BIT(p, evindex_)				((p)[(evindex_) >> 3] |= (1 << ((evindex_) & 7)))
#define HCI_CHECK_BIT(p, evindex_)				((p)[(evindex_) >> 3] & (1 << ((evindex_) & 7)))
#define NEWSTRU(stru_, a_)						stru_ *(a_) = NEW(sizeof(stru_))//TBD, move to btcommon

/*---------------------------------------------------------------------------*/
/* 									UI Functions							 */
/*---------------------------------------------------------------------------*/
void HCI_Init(void);
void HCI_Done(void);

void HCI_Cbk_Reg(const struct HCI_CbkLineStru *table, UINT32 size);
void HCI_Cbk_UnReg(const struct HCI_CbkLineStru *table);

void HCI_TL_Reg(HANDLE tl_hdl, TransportLayerStru *func);
void HCI_TL_UnReg(HANDLE tl_hdl);

void *HCI_ACLLink_Ref(struct HCI_AddressStru *l, struct HCI_AddressStru *r, UINT32 flag);
void HCI_ACLLink_UnRef(void *hci);
void HCI_Create_SCOConnection(struct HCI_Create_SCOUIStru *in);
void HCI_ACLConnection_Response(struct HCI_ACLConnection_ResponseUIStru *in);
void HCI_Synchronous_Connection_Response(struct HCI_Synchronous_Connection_ResponseUIStru *in);

void HCI_Command_SendAMP(void *context, void *par, UINT32 opscode, UINT8 ampid);
void HCI_Command_SendVendor(void *context, void *par, UINT16 opcode, UINT8 evi, UINT8 paramlen, UINT8 ampid);
void HCI_CommandL2_SendQueue(struct HCI_CommandQueueStru *queue);
void HCI_CommandL2_CancelQueue(void *queue/* struct HCI_CommandQueueStru * */, UINT8 ampid);

#define HCI_Command_Send(context_, par_, opscode_)		HCI_Command_SendAMP((context_), (par_), (opscode_), HCI_AMPID_BREDR)

struct HCI_CommandQueueStru *HCI_CommandL2_NewQueue(void *cbk, void *context, UINT8 ampid);
struct HCI_CommandQueueNodeStru *HCI_CommandL2_NewNode(struct ListStru *list, UINT32 opscode);
struct HCI_Set_Event_FilterStru *HCI_Set_Event_Filter(struct HCI_Set_Event_FilterInStru *in);
void HCI_Controller_Init(void *cbk, void *context, UINT8 ampid);
void *HCI_Inquiry(struct HCI_InquiryInStru *in);
void HCI_Disconnect(UINT16 connection_handle);
void HCI_ACLData_Send(struct BuffStru *data, UINT32 cid_hcihdl, UINT8 ampid);
void HCI_SCOData_Send(struct BuffStru *data, UINT16 scohdl);

UINT8 HCI_CompareAddr(struct HCI_AddressStru *s, struct HCI_AddressStru *r);
UINT8 HCI_CompareBD(struct HCI_AddressStru *addr, UINT8 *bd);
UINT8 HCI_CompareAllNum(UINT8 *p, UINT8 num, UINT32 len);

UINT8 HCI_Extended_Inquiry_Response_Encode(struct ListStru *in, UINT8 *out, UINT8 *end);
void HCI_Extended_Inquiry_Response_Decode(struct ListStru *list, UINT8 *eir, UINT8 *end);

struct HCI_InfoRead_LocalStru *HCI_InfoRead_Local(UINT8 ampid);
void HCI_InfoRead_LinkSecurity(struct HCI_InfoRead_LinkSecurityStru *out, void *acl);

struct ListStru *HCI_LE_Advertising_Report_Decode(struct HCI_LE_Advertising_ReportEvStru *ev);

void HCI_Security_SetBondable(struct HCI_Security_LocalBondableStru *in);
void HCI_Security_SetOOB(struct HCI_Security_RemoteOOBStru *in);
void HCI_Security_SetLocalIO(struct HCI_Security_LocalIOStru *in);
void HCI_Security_Request(struct HCI_SecurityNodeStru *in);
void HCI_Security_Abort(struct HCI_SecurityNodeStru *in);
void HCI_Security_Authorized(struct HCI_Security_AuthorReqStru *in);
void HCI_Security_RequireReg(struct HCI_SecurityInfoStru *in);
void HCI_Security_RequireUnReg(struct HCI_SecurityInfoStru *in);
void HCI_Security_LinkKey_Reply(struct HCI_Security_Link_Key_Request_ReplyStru *in);
void HCI_Security_LinkKey_Remove(UINT8 *bd);
void HCI_Security_PINCode_Reply(struct HCI_PIN_Code_Request_ReplyStru *in);

UINT8 HCI_Security_GetOOBFlag(void *hci);
UINT8 HCI_Security_GetLocalIO(void);
UINT8 HCI_Security_GetBondable(void);/* 3 functions for SMP */
UINT32 HCI_Security_PasskeyEntryInput(UINT8 *in/* 6 octets */);

UINT32 HCI_Command_GetLen(void *stru, UINT32 opscode);
void HCI_Set_Channel_Classification(UINT32 c/* Central Frequency, etc,2412 */, UINT32 w/* Bandwidth, 22,40 */, UINT8 mask/* etc, HCI_SETAFH_OCCUPY */);

#ifdef CONFIG_BLE
void HCI_BLE_AdvSetParam(struct HCI_LE_Set_Advertising_ParametersStru *in);
void HCI_BLE_AdvSetData(struct HCI_LE_Set_Advertising_DataStru *in);
void HCI_BLE_AdvSetEnable(struct HCI_LE_Set_Advertise_EnableStru *in);
void HCI_BLE_ScanSetResponseData(struct HCI_LE_Set_Scan_Response_DataStru *in);
#endif
void HCI_DUT_SetEnable(UINT8 en);
#endif/* G_HCI_UI_H */

