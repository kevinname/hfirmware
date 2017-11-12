/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Module Name:
	avrcp_ui.h
Abstract:
	This module defines all of the AVRCP 1.4 advanced control internal functions.
-----------------------------------------------------------------------------*/

#ifndef AVRCP_UI_H
#define AVRCP_UI_H

/*---------------------------------------------------------------------------*/
/* 								AVRCP Global Vars 							 */
/*---------------------------------------------------------------------------*/
typedef void (AVRCP_Callback)(UINT8 *bd, UINT32 event, UINT8 *param, UINT16 len);/* callback to upper application */

/* 3bits grouping structure defination, max value is 7 */
enum {/* can not use 0 as nothing more */
	STRU_UINT8 = 1,		/* 1, 001 */
	STRU_UINT16,		/* 2, 010 */
	STRU_UINT32,		/* 3, 011 */
	STRU_1NUM,			/* 4, 100 */
	STRU_2NUM,			/* 5, 101 */
	STRU_8BYTES,		/* 6, 110 */
	STRU_POINTER,		/* 7, 111 */
};

/* when item is num type, following 2bits represent the max sub structure item length, UINT indicate that is not a structure */
#define STRU_NUMUINT			0x00/* 00, max item length in sub structure is UINT8 or not a sub structure */
#define STRU_NUMSTRU2			0x01/* 01, max item length in sub structure is UINT16  */
#define STRU_NUMSTRU4			0x02/* 02, max item length in sub structure is UINT32 */

/*---------------------------------------------------------------------------*/
/* 							 	Global Resource Structure					 */
/*---------------------------------------------------------------------------*/
struct AVRCP_PassThroughStru {		/* PassThrough and GroupNavigation */
	UINT8 	state_flag;  				/* Button state(0: pressed 1: released), e.g AVRCP_STATE_FLAG_BUTTON_PUSHED */
	UINT8 	op_id;        				/* Pass through operation_id e.g AVRCP_OPID_PLAY */
	UINT16	vendor_unique_id;			/* e.g. AVRCP_BGN_NEXTGROUP */
};

struct AVRCP_ConnectReqStru {
	UINT8	bd_addr[BD_ADDR_LEN];	
	UINT8	browsing_support;			/* remote device supports browsing channel or not */
};

#define AVRCP_CHANNEL_INITIATOR 				AVCTP_CHANNEL_INITIATOR
#define AVRCP_CHANNEL_RESPONDER 				AVCTP_CHANNEL_RESPONDER

#if 1
struct AVRCP_ConnectCompleteStru {/* Used for event AVRCP_EV_CONNECT_COMPLETE/AVRCP_EV_DISCONNECT_COMPLETE */
	UINT8 	bd_addr[BD_ADDR_LEN];
	UINT8 	channel;					/* e.g AVRCP_CONTROL_CHANNEL */
	UINT8 	result;						/* e.g BT_SUCCESS */
	UINT8	side;						/* e.g AVRCP_CHANNEL_INITIATOR */
	UINT16 	mtu;						/* outgoing browsing mtu, for browsing channel */
};
#else 
#define AVRCP_ConnectCompleteStru  AVCTP_ConnectCompleteStru
#endif

struct AVRCP_GeneralRejectCfmStru {/* if 4bits rsp_type is reject, for app */
	UINT8 	error_code;
	UINT16 	ev_pduid;
};

/*---------------------------------------------------------------------------*/
/* 							Structure defination for AVRCP interface		 */
/*---------------------------------------------------------------------------*/
struct AVRCP_GetCapabilitiesRspStru {/* 6.4 */
	UINT8 	id;
	UINT8 	count;
	UINT8 	capability[1];/* 3...3*n for COMPANY_ID, 2...n for EVENTS_SUPPORTED */
};
#define AVRCP_MASK_GETCAPABILITIESRSP_COMPANY_ID  			(STRU_UINT8 | (STRU_1NUM << 3) | (STRU_NUMUINT << 6) | (STRU_UINT8 << 8) | (STRU_UINT8 << 11) | (STRU_UINT8 << 14))
#define AVRCP_MASK_GETCAPABILITIESRSP_EVENTS_SUPPORTED  	(STRU_UINT8 | (STRU_1NUM << 3) | (STRU_NUMUINT << 6) | (STRU_UINT8 << 8))

struct AVRCP_NumIDStru {/* 6.5 */
	UINT8 	num;
	UINT8 	id[1];
};
#define AVRCP_MASK_NUMID  (STRU_1NUM | (STRU_NUMUINT << 3) | (STRU_UINT8 << 5))

struct AVRCP_IDPairStru {/* 6.5.3 SubStru */
	UINT8 	attr_id;
	UINT8 	value_id;
};
#define AVRCP_MASK_IDPAIR  (STRU_UINT8 | (STRU_UINT8 << 3))

struct AVRCP_NumIDPairStru {/* 6.5.3 */
	UINT8 	num;
	struct 	AVRCP_IDPairStru id[1];
};
#define AVRCP_MASK_NUMIDPAIR  (STRU_1NUM | (STRU_NUMUINT << 3) | (AVRCP_MASK_IDPAIR << 5))

struct AVRCP_IDStringStru {/* 6.5.5 SubStru */
	UINT8 	id;/* attr_id or value_id */
	UINT16 	characterset_id;
	UINT8 	len;
	UINT8 	*string;
};
#define AVRCP_MASK_IDSTRING  (STRU_UINT8 | (STRU_UINT16 << 3) | (STRU_POINTER << 6) | (STRU_UINT8 << 9))

struct AVRCP_NumIDStringStru {/* 6.5.5 */
	UINT8 	num;
	struct 	AVRCP_IDStringStru id_string[1];
};
#define AVRCP_MASK_NUMIDSTRING  (STRU_1NUM | (STRU_NUMSTRU4 << 3) | (AVRCP_MASK_IDSTRING << 5))

struct AVRCP_IDNumIDStru {/* 6.5.6 */
	UINT8 	attr_id;
	UINT8 	num;
	UINT8 	value_id[1];
};
#define AVRCP_MASK_IDNUMID  (STRU_UINT8 | (STRU_1NUM << 3) | (STRU_NUMUINT << 6) | (STRU_UINT8 << 8))

struct AVRCP_NumCSIDStru {/* 6.5.7 */
	UINT8 	num;
	UINT16 	characterset_id[1];
};
#define AVRCP_MASK_NUMCSID  (STRU_1NUM | (STRU_NUMUINT << 3) | (STRU_UINT16 << 5))

struct AVRCP_IDNum4IDStru {/* 6.6 */
	UINT8 	identifier[8];
	UINT8 	num;
	UINT32 	attr_id[1];
};
#define AVRCP_MASK_IDNUM4ID  (STRU_8BYTES | (STRU_1NUM << 3) | (STRU_NUMUINT << 6) | (STRU_UINT32 << 8))

struct AVRCP_4IDStringStru {/* 6.6.1 SubStru */
	UINT32 	attr_id;
	UINT16 	characterset_id;
	UINT16 	len;
	UINT8 	*value;
};
#define AVRCP_MASK_4IDSTRING  (STRU_UINT32 | (STRU_UINT16 << 3) | (STRU_POINTER << 6) | (STRU_UINT16 << 9))

struct AVRCP_Num4IDValueStru {/* 6.6.1 */
	UINT8 	num;
	struct 	AVRCP_4IDStringStru id_value[1];
};
#define AVRCP_MASK_NUM4IDVALUE  (STRU_1NUM | (STRU_NUMSTRU4 << 3) | (AVRCP_MASK_4IDSTRING << 5))

struct AVRCP_GetPlayStatusRspStru {/* 6.7 Notification PDUs	*/
	UINT32 	song_length;
	UINT32 	song_position;
	UINT8 	play_status;
};
#define AVRCP_MASK_GETPLAYSTATUSRSP  (STRU_UINT32 | (STRU_UINT32 << 3) | (STRU_UINT8 << 6))

struct AVRCP_RegisterNotifReqStru {/* 6.7.2 */
	UINT8 	event_id;
	UINT32 	playback_interval;/* Applicable only for EventID EVENT_PLAYBACK_POS_CHANGED */
};
#define AVRCP_MASK_REGISTERNOTIFREQ  (STRU_UINT8 | (STRU_UINT32 << 3))

struct AVRCP_RegisterNotifRspStru {/* 6.7.2 */
	UINT8 	event_id;
	UINT8 	param[1];/* param specfied by event_id */
};/* This structure dealed alone */

struct AVRCP_PlaybackPosChangedNotifStru {
	UINT8 	event_id;
	UINT32 	pos;
};/* NOTICE: This is only an UINT32 sub mask, so no defination for this stru */

struct AVRCP_UIDsChangedNotifStru {
	UINT8 	event_id;
	UINT16 	uid_counter;
};/* NOTICE: This is only an UINT16 sub mask, so no defination for this stru */

struct AVRCP_AddressedPlayerChangedNotifStru {/* 6.9.2 */
	UINT8 	event_id;
	UINT16 	player_id;
	UINT16 	uid_counter;	
};
#define AVRCP_MASK_ADDRESSEDPLAYERCHANGEDNOTIF  (STRU_UINT16 | (STRU_UINT16 << 3))/* NOTICE: This is a partial mask */

struct AVRCP_FolderPairStru {/* 6.9.3 sub structure */
	UINT16 	folder_name_len;/* The length in octets of the folder name which follows. */
	UINT8 	*folder_name;
};
#define AVRCP_MASK_FOLDERPAIR  (STRU_POINTER | (STRU_UINT16 << 3))

struct AVRCP_SetBrowsedPlayerRspStru { /* 6.9.3 */
	UINT8 	status;/* Status as defined in 6.15.3 */
	UINT16 	uid_counter;
	UINT32 	items_num;
	UINT16 	characterset_id;
	UINT8 	folder_depth;/* The number of Folder Name Length/Folder Name pairs which follow */
	struct AVRCP_FolderPairStru folder[1];
};
#define AVRCP_MASK_SETBROWSEDPLAYERRSP  (STRU_UINT8 | (STRU_UINT16 << 3) | (STRU_UINT32 << 6) | (STRU_UINT16 << 9) | (STRU_1NUM << 12) | (STRU_NUMSTRU4 << 15) | (AVRCP_MASK_FOLDERPAIR << 17))

struct AVRCP_GetItemAttributesRspStru {/* 6.10.2.3.1 Attribute Value Entry */
	UINT8 	status;
	UINT8 	attr_num;/* 1bytes */
	struct 	AVRCP_4IDStringStru entry[1];/* An Attribute Value Entry list as defined in 6.10.2.3.1 */
};
#define AVRCP_MASK_GETITEMATTRIBUTESRSP  (STRU_UINT8 | (STRU_1NUM << 3) | (STRU_NUMSTRU4 << 6) | (AVRCP_MASK_4IDSTRING << 8))

struct AVRCP_ChangePathReqStru {/* 6.10.4 */
	UINT16 	uid_counter;
	UINT8 	direction;
	UINT8 	folder_uid[8];
};
#define AVRCP_MASK_CHANGEPATHREQ  (STRU_UINT16 | (STRU_UINT8 << 3) | (STRU_8BYTES << 6))

struct AVRCP_ChangePathRspStru {/* 6.10.4.1 */ 
	UINT8 	status;
	UINT32 	items_num;
};
#define AVRCP_MASK_CHANGEPATHRSP  (STRU_UINT8 | (STRU_UINT32 << 3))

struct AVRCP_MediaPlayerItemStru {/* 6.10.2.1 Media Player Item */
	UINT8 	item_type;/* AVRCP_ITEMTYPE_MEDIAPLAYER_ITEM */
	UINT16 	item_len;/* 2 + 1 + 4 + 1 + 16 + 2 + 2 + name_len */
	UINT16 	player_id;
	UINT8 	major_player_type;
	UINT32	player_subtype;
	UINT8 	play_status;
	UINT8 	feature_bitmask[16];
	UINT16 	characterset_id;/* e.g AVRCP_CHARACTERSETID_UTF8 */
	UINT16 	name_len;
	UINT8 	*name;
};
#define AVRCP_MASK_MEDIAPLAYERITEM	(STRU_UINT8 | (STRU_UINT16 << 3) | (STRU_UINT16 << 6) | (STRU_UINT8 << 9) | (STRU_UINT32 << 12) | (STRU_UINT8 << 15) | (STRU_8BYTES << 18) | (STRU_8BYTES << 21) | (STRU_UINT16 << 24) | (STRU_POINTER << 27) | (STRU_UINT16 << 30))

struct AVRCP_FolderItemStru {/* 6.10.2.2 Folder Item */
	UINT8 	item_type;/* AVRCP_ITEMTYPE_FOLDER_ITEM */
	UINT16 	item_len;/* 8 + 1 + 1 + 2 + 2 + name_len */
	UINT8 	folder_uid[8];
	UINT8 	folder_type;
	UINT8 	is_playable;
	UINT16 	characterset_id;
	UINT16 	name_len;
	UINT8 	*name;
};
#define AVRCP_MASK_FOLDERITEM	(STRU_UINT8 | (STRU_UINT16 << 3) | (STRU_8BYTES << 6) | (STRU_UINT8 << 9) | (STRU_UINT8 << 12) | (STRU_UINT16 << 15) | (STRU_POINTER << 18) | (STRU_UINT16 << 21))

struct AVRCP_MediaElementItemStru {/* 6.10.2.3 Media Element Item */
	UINT8 	item_type; /* AVRCP_ITEMTYPE_MEDIAELEMENT_ITEM */
	UINT16 	item_len;/* 8 + 1 + 2 + 2 + name_len + 1 + attr_num * (4 + 2 + 2 + len) */
	UINT8 	element_uid[8];/* UID as defined in 6.10.3 */
	UINT8 	media_type;/* e.g AVRCP_MEDIATYPE_AUDIO */
	UINT16 	characterset_id;/* e.g AVRCP_CHARACTERSETID_UTF8 */
	UINT16 	name_len;
	UINT8 	*name;
	UINT8 	attr_num;
	struct 	AVRCP_4IDStringStru entry[1];/* this is an array */
};
#define AVRCP_MASK_MEDIAELEMENTITEM_1	(STRU_UINT8 | (STRU_UINT16 << 3) | (STRU_8BYTES << 6) | (STRU_UINT8 << 9) | (STRU_UINT16 << 12) | (STRU_POINTER << 15) | (STRU_UINT16 << 18))
#define AVRCP_MASK_MEDIAELEMENTITEM_2	(STRU_1NUM | (STRU_NUMSTRU4 << 3) | (AVRCP_MASK_4IDSTRING << 5))

struct AVRCP_GetFolderItemsRspStru {/* 6.10.4.2 */
	UINT8 	status;/* e.g AVRCP_ERROR_SUCCESSFUL */
	UINT16 	uid_counter;
	UINT16 	items_num;/* 2bytes */
	UINT8 	*item;
};/* This structure will coding manually */

struct AVRCP_GetFolderItemsReqStru {/* 6.10.4.2 */
	UINT8 	scope;/* Scope as defined in section 6.10.1 e.g AVRCP_SCOPE_MEDIAPLAYER_LIST */
	UINT32 	start_item;
	UINT32 	end_item;
	UINT8 	attr_count;
	UINT32 	attr_id[1];/* Attribute Count Attributes as defined in 23 e.g list AVRCP_MA_TITLEOF_MEDIA */
};
#define AVRCP_MASK_GETFOLDERITEMSREQ  (STRU_UINT8 | (STRU_UINT32 << 3) | (STRU_UINT32 << 6) | (STRU_1NUM << 9) | (STRU_NUMUINT << 12) | (STRU_UINT32 << 14))

struct AVRCP_GetItemAttributesReqStru {/* 6.10.4.3 */
	UINT8 	scope;/* e.g AVRCP_SCOPE_MEDIAPLAYER_LIST */
	UINT8 	uid[8];
	UINT16 	uid_counter;
	UINT8 	attr_num;
	UINT32 	attr_id[1];/* e.g list AVRCP_MA_TITLEOF_MEDIA */
};
#define AVRCP_MASK_GETITEMATTRIBUTESREQ  (STRU_UINT8 | (STRU_8BYTES << 3) | (STRU_UINT16 << 6) | (STRU_1NUM << 9) | (STRU_NUMUINT << 12) | (STRU_UINT32 << 14))

struct AVRCP_SearchReqStru {/* 6.11 */
	UINT16 	characterset_id;
	UINT16 	len;
	UINT8 	string[1];
};
#define AVRCP_MASK_SEARCHREQ  	(STRU_UINT16 | (STRU_2NUM << 3) | (STRU_NUMUINT << 6) | (STRU_UINT8 << 8))

struct AVRCP_SearchRspStru {/* 6.11 */
	UINT8 	status;
	UINT16 	uid_counter;
	UINT32 	items_num;
};
#define AVRCP_MASK_SEARCHRSP  (STRU_UINT8 | (STRU_UINT16 << 3) | (STRU_UINT32 << 6))

struct AVRCP_PlayItemReqStru {/* 6.12.1 */
	UINT8 	scope;
	UINT8 	uid[8];
	UINT16 	uid_counter;
};
#define AVRCP_MASK_PLAYITEMREQ  (STRU_UINT8 | (STRU_8BYTES << 3) | (STRU_UINT16 << 6))

/*---------------------------------------------------------------------------*/
/* 								Events for application grouping				 */
/*---------------------------------------------------------------------------*/
/* Events grouping for AVRCP callback to upper application, UINT16 MSB */
/* event: <4bit ctype/rsp>,<1bit channel>,<2bit grouping reserved>,<1bit cr>,<8bit pdu_id> */
enum {
	AVRCP_EV_GROUP_COMMAND = 0x00,/* 0x00, AVCTP_COMMAND_FRAME */
	AVRCP_EV_GROUP_RESPONSE = 0x01,/* 0x01, AVCTP_RESPONSE_FRAME */
	AVRCP_EV_GROUP_CONNECTION = 0x02,				/* 0x02 */
};

/* Connection event group events */
enum {
	AVRCP_EV_CONNECT_COMPLETE = (AVRCP_EV_GROUP_CONNECTION << 8) | 0x01,
	AVRCP_EV_DISCONNECT_COMPLETE,					/* 0x02 */
};

/*---------------------------------------------------------------------------*/
/* 									tlcr_pduid bit fields					 */
/*---------------------------------------------------------------------------*/
#define AVRCP_SET_TL(s, v)						(s) = (((s) & (~0x000F0000)) | ((v) << 16))
#define AVRCP_SET_CHANNEL(s, v)					(s) = (((s) & (~0x00000800)) | ((v) << 11))
#define AVRCP_SET_PDUID(s, v)					(s) = (((s) & (~0x000000FF)) | (v))
#define AVRCP_SET_4BIT_CR(s, v)					(s) = (((s) & (~0x0000F000)) | ((v) << 12))
#define AVRCP_SET_1BIT_CR(s, v)					(s) = (((s) & (~0x00000100)) | ((v) << 8))
#define AVRCP_SET_CBK_GROUPING(s, v)			(s) = (((s) & (~0x00000300)) | ((v) << 8))
#define AVRCP_SET_RSPTYPE(s, v)					AVRCP_SET_4BIT_CR(s, v); AVRCP_SET_1BIT_CR(s, AVCTP_RESPONSE_FRAME)
#define AVRCP_SET_1BITCR_RSP_PDUID(p)			(((AVCTP_RESPONSE_FRAME) << 8) | (p))
#define AVRCP_SET_1BITCR_REQ_PDUID(p)			(((AVCTP_COMMAND_FRAME) << 8) | (p))

#define AVRCP_GET_TL(s)							(UINT8)(((s) & (0x000F0000)) >> 16)
#define AVRCP_GET_CHANNEL(s)					(UINT8)(((s) & (0x00000800)) >> 11)
#define AVRCP_GET_PDUID(s)						(UINT8)(((s) & (0x000000FF)))
#define AVRCP_GET_1BITCR_PDUID(s)				(UINT16)(((s) & (0x000001FF)))
#define AVRCP_GET_4BIT_CR(s)					(UINT8)(((s) & (0x0000F000)) >> 12)
#define AVRCP_GET_1BIT_CR(s)					(UINT8)(((s) & (0x00000100)) >> 8)
#define AVRCP_GET_CBK_GROUPING(s)				(UINT8)(((s) & (0x00000300)) >> 8)

/*---------------------------------------------------------------------------*/
/* 								AVRCP Common Macros						     */
/*---------------------------------------------------------------------------*/
#define AVRCP_CT								0x01/* Controller */
#define AVRCP_TG								0x02/* Target */
#define AVRCP_CHARACTERSETID_UTF8				0x006A/* CharacterSetID, UTF-8 */
#define AVRCP_ALL_TG_FEATURE					(AVTG_CATEGORY_1 | AVTG_CATEGORY_2 | AVTG_CATEGORY_3 | AVTG_CATEGORY_4 | AVTG_FEATURE_BROWSING | AVTG_FEATURE_PLAYER_SET | AVTG_FEATURE_GROUP_NAVI | AVTG_FEATURE_MultiMedia)	/* TG Category 1,2,3,4 Browsing*/
#define AVRCP_ALL_CT_FEATURE					(AVCT_CATEGORY_1 | AVCT_CATEGORY_2 | AVCT_CATEGORY_3 | AVCT_CATEGORY_4 | AVCT_FEATURE_BROWSING)	/* TG Category 1,2,3,4 Browsing*/
#define AVRCP_CONTROL_CHANNEL					0x00 /* AVCTP_CONTROL_CHANNEL */
#define AVRCP_BROWSING_CHANNEL					0x01 /*	AVCTP_BROWSING_CHANNEL */

/*---------------------------------------------------------------------------*/
/* 									IEEE 1394 AV/C Macros					 */
/*---------------------------------------------------------------------------*/
/* AV/C Command Types, 4 Bits */
#define AVRCP_CTYPE_CONTROL 										0x00
#define AVRCP_CTYPE_STATUS 											0x01
#define AVRCP_CTYPE_SPECIFIC_INQUIRY 								0x02
#define AVRCP_CTYPE_NOTIFY 											0x03
#define AVRCP_CTYPE_GERNERAL_INQUIRY 								0x04

/* AV/C Response Code, 4 Bits */
#define AVRCP_RSP_NOT_IMPLEMENTED 									0x08/* Caution: this marco used to judge command/response borading line */
#define AVRCP_RSP_ACCEPTED 											0x09
#define AVRCP_RSP_REJECTED 											0x0A
#define AVRCP_RSP_IN_TRANSITION 									0x0B
#define AVRCP_RSP_STABLE 											0x0C/* Implemented */
#define AVRCP_RSP_CHANGED 											0x0D/* for notification */
#define AVRCP_RSP_INTERIM 											0x0F

/* AV/C Subunit Types */
#define AVRCP_SUBTYPE_VIDEO_MONITOR 								0x00
#define AVRCP_SUBTYPE_DISC_RECORDER  								0x03
#define AVRCP_SUBTYPE_TAPE_RECORDER 								0x04
#define AVRCP_SUBTYPE_TUNER 										0x05
#define AVRCP_SUBTYPE_VIDEO_CAMERA 									0x07
#define AVRCP_SUBTYPE_PANEL 										0x09
#define AVRCP_SUBTYPE_VENDOR_UNIQUE 								0x1C
#define AVRCP_SUBTYPE_EXTENDED 										0x1E
#define AVRCP_SUBTYPE_UNIT 											0x1F

/* AV/C Subunit IDs */
#define AVRCP_SUBID_0 												0 
#define AVRCP_SUBID_1 												1
#define AVRCP_SUBID_2 												2 
#define AVRCP_SUBID_3 												3 
#define AVRCP_SUBID_4 												4 
#define AVRCP_SUBID_EXTENDED 										5 
#define AVRCP_SUBID_IGNORE 											7 

/* AV/C Unit Commands */
#define AVRCP_COM_UNIT_INFO 										0x30
#define AVRCP_COM_SUBUNIT_INFO 										0x31

/* AV/C Subunit Commands */
#define AVRCP_COM_PASS_THROUGH 										0x7C

/* AV/C Common Unit and Subunit Commands */
#define AVRCP_COM_VENDOR_DEPENDENT 									0x00

/*---------------------------------------------------------------------------*/
/* 								22.3 Pass Through command	     			 */
/*---------------------------------------------------------------------------*/
#define AVRCP_STATE_FLAG_BUTTON_PUSHED								0x0
#define AVRCP_STATE_FLAG_BUTTON_RELEASED							0x1

/* AV/C Panel Commands, Pass Through command operation_id */
#define AVRCP_OPID_SELECT 											0x00
#define AVRCP_OPID_UP					 							0x01
#define AVRCP_OPID_DOWN 											0x02
#define AVRCP_OPID_LEFT 											0x03
#define AVRCP_OPID_RIGHT 											0x04
#define AVRCP_OPID_RIGHT_UP											0x05			
#define AVRCP_OPID_RIGHT_DOWN										0x06
#define AVRCP_OPID_LEFT_UP											0x07
#define AVRCP_OPID_LEFT_DOWN										0x08
#define AVRCP_OPID_ROOTMENU 										0x09
#define AVRCP_OPID_SETUP_MENU										0x0a
#define AVRCP_OPID_CONTENTS_MENU									0x0b
#define AVRCP_OPID_FAVORITE_MENU									0x0c
#define AVRCP_OPID_EXIT												0x0d

#define AVRCP_OPID_0												0x20
#define AVRCP_OPID_1												0x21
#define AVRCP_OPID_2												0x22
#define AVRCP_OPID_3												0x23
#define AVRCP_OPID_4												0x24
#define AVRCP_OPID_5												0x25
#define AVRCP_OPID_6												0x26
#define AVRCP_OPID_7												0x27
#define AVRCP_OPID_8												0x28
#define AVRCP_OPID_9												0x29
#define AVRCP_OPID_DOT												0x2a
#define AVRCP_OPID_ENTER											0x2b
#define AVRCP_OPID_CLEAR											0x2c

#define AVRCP_OPID_CHANNEL_UP 										0x30
#define AVRCP_OPID_CHANNEL_DOWN 									0x31
#define AVRCP_OPID_PREVIOUS_CHANNEL									0x32
#define AVRCP_OPID_SOUND_SELECT										0x33
#define AVRCP_OPID_INPUT_SELECT										0x34
#define AVRCP_OPID_DISPLAY_INFORMATION								0x35
#define AVRCP_OPID_HELP												0x36
#define AVRCP_OPID_PAGE_UP											0x37
#define AVRCP_OPID_PAGE_DOWN										0x38

#define AVRCP_OPID_POWER 											0x40
#define AVRCP_OPID_VOLUME_UP 										0x41
#define AVRCP_OPID_VOLUME_DOWN 										0x42
#define AVRCP_OPID_MUTE												0x43
#define AVRCP_OPID_PLAY 											0x44
#define AVRCP_OPID_STOP 											0x45
#define AVRCP_OPID_PAUSE 											0x46
#define AVRCP_OPID_RECORD 											0x47
#define AVRCP_OPID_REWIND 											0x48
#define AVRCP_OPID_FAST_FORWARD 									0x49
#define AVRCP_OPID_EJECT 											0x4a
#define AVRCP_OPID_FORWARD 											0x4b
#define AVRCP_OPID_BACKWARD 										0x4c

#define AVRCP_OPID_ANGLE											0x50
#define AVRCP_OPID_SUBPICTURE										0x51
#define AVRCP_OPID_F1												0x52
#define AVRCP_OPID_F2												0x53
#define AVRCP_OPID_F3												0x54
#define AVRCP_OPID_F4												0x55
#define AVRCP_OPID_F5												0x56
#define AVRCP_OPID_VENDORUNIQUE 									0x7e

/*---------------------------------------------------------------------------*/
/* 						4.4.1 PASS THROUGH Command PDU IDs						   */
/*---------------------------------------------------------------------------*/
#define AVRCP_PDUID_GET_CAPABILITIES 								0x10  /* AV/C STATUS */
#define AVRCP_PDUID_LIST_PLAYER_APPLICATION_SETTING_ATTRIBUTES		0x11  /* AV/C STATUS */
#define AVRCP_PDUID_LIST_PLAYER_APPLICATION_SETTING_VALUES			0x12  /* AV/C STATUS */
#define AVRCP_PDUID_GET_CURRENTPLAYER_APPLICATION_SETTING_VALUE 	0x13  /* AV/C STATUS */
#define AVRCP_PDUID_SET_CURRENTPLAYER_APPLICATION_SETTING_VALUE		0x14  /* AV/C CONTROL */
#define AVRCP_PDUID_GET_PLAYER_APPLICATION_SETTING_ATTRIBUTE_TEXT 	0x15  /* AV/C STATUS */
#define AVRCP_PDUID_GET_PLAYER_APPLICATION_SETTING_VALUE_TEXT	 	0x16  /* AV/C STATUS */
#define AVRCP_PDUID_INFORM_DISPLAYABLE_CHARACTERSET	 				0x17  /* AV/C CONTROL */
#define AVRCP_PDUID_INFORM_BATTERYSTATUS_OF_CT		 				0x18  /* AV/C CONTROL */			
#define AVRCP_PDUID_GET_ELEMENT_ATTRIBUTES						 	0x20  /* AV/C STATUS */
#define AVRCP_PDUID_GET_PLAY_STATUS									0x30  /* AV/C STATUS */
#define AVRCP_PDUID_REGISTER_NOTIFICATION				 			0x31  /* AV/C NOTIFY */
#define AVRCP_PDUID_REQUEST_CONTINUING_RESPONSE		 				0x40  /* AV/C CONTROL */
#define AVRCP_PDUID_ABORT_CONTINUING_RESPONSE			 			0x41  /* AV/C CONTROL */
#define AVRCP_PDUID_SET_ABSOLUTE_VOLUME					 			0x50  /* AV/C CONTROL */
#define AVRCP_PDUID_SET_ADDRESSED_PLAYER					 		0x60  /* AV/C CONTROL */
#define AVRCP_PDUID_SET_BROWSED_PLAYER								0x70  /* Browsing */
#define AVRCP_PDUID_GET_FOLDER_ITEMS								0x71  /* Browsing */
#define AVRCP_PDUID_CHANGE_PATH										0x72  /* Browsing */
#define AVRCP_PDUID_GET_ITEM_ATTRIBUTES								0x73  /* Browsing */
#define AVRCP_PDUID_PLAY_ITEM										0x74  /* AV/C CONTROL */
#define AVRCP_PDUID_SEARCH											0x80  /* Browsing */
#define AVRCP_PDUID_ADDTO_NOWPLAYING								0x90  /* AV/C CONTROL */
#define AVRCP_PDUID_GENERAL_REJECT									0xA0  /* Browsing */

/* These 'PDUID' defination is only used for upper layer event parse */
#define AVRCP_PDUID_UNITINFO										0xFC
#define AVRCP_PDUID_SUBUNITINFO										0xFD
#define AVRCP_PDUID_PASSTHROUGH										0xFE

/*---------------------------------------------------------------------------*/
/* 								6.4 Capabilities PDUs						         		   */
/*---------------------------------------------------------------------------*/
#define AVRCP_CAPABILITYID_COMPANY_ID								0x2
#define AVRCP_CAPABILITYID_EVENTS_SUPPORTED							0x3

/*---------------------------------------------------------------------------*/
/* 								6.5 Player application settings PDUs			         		   */
/*---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
Description:
	24 Appendix F: List of defined Player Application Settings and Values.
-----------------------------------------------------------------------------*/
#define AVRCP_PASA_ILLEGAL											0x00
#define AVRCP_PASA_EQUALIZER_ONOFF_STATUS							0x01
#define AVRCP_PASA_REPEAT_MODE_STATUS								0x02
#define AVRCP_PASA_SHUFFLE_ONOFF_STATUS								0x03
#define AVRCP_PASA_SCAN_ONOFF_STATUS								0x04
/* PASA 0x05 - 0x7F Reserved for future use. 0x80 - 0xFF Provided for TG driven static media player menu extension by CT */

/* 0x01 Equalizer ON/OFF status */
#define AVRCP_EQUALIZER_OFF											0x01
#define AVRCP_EQUALIZER_ON											0x02

/* 0x02 Repeat Mode status */
#define AVRCP_REPEAT_MODE_OFF										0x01
#define AVRCP_REPEAT_MODE_SINGLE_TRACK_REPEAT						0x02
#define AVRCP_REPEAT_MODE_ALL_TRACK_REPEAT							0x03
#define AVRCP_REPEAT_MODE_GROUP_REPEAT								0x04

/* 0x03 Shuffle ON/OFF status */
#define AVRCP_SHUFFLE_OFF											0x01
#define AVRCP_SHUFFLE_ALL_TRACKS_SHUFFLE							0x02
#define AVRCP_SHUFFLE_GROUP_SHUFFLE									0x03

/* 0x04 Scan ON/OFF status */
#define AVRCP_SCAN_OFF												0x01
#define AVRCP_SCAN_ALL_TRACKS_SCAN									0x02
#define AVRCP_SCAN_GROUP_SCAN										0x03

/* Battery Status, Table 6.24: InformBatteryStatusOfCT command */
#define AVRCP_BATTERYSTATUS_NORMAL									0x0
#define AVRCP_BATTERYSTATUS_WARNING									0x1
#define AVRCP_BATTERYSTATUS_CRITICAL								0x2
#define AVRCP_BATTERYSTATUS_EXTERNAL								0x3
#define AVRCP_BATTERYSTATUS_FULL_CHARGE								0x4

/*---------------------------------------------------------------------------*/
/* 								6.7 Notification PDUs			         						   */
/*---------------------------------------------------------------------------*/
#define AVRCP_PLAYSTATUS_STOPPED									0x00
#define AVRCP_PLAYSTATUS_PLAYING									0x01
#define AVRCP_PLAYSTATUS_PAUSED										0x02
#define AVRCP_PLAYSTATUS_FWD_SEEK									0x03
#define AVRCP_PLAYSTATUS_REV_SEEK									0x04/* 0x05-0xfe are reserved */
#define AVRCP_PLAYSTATUS_ERROR										0xFF

#define AVRCP_NOTSUPPORT_VALUE										0xFFFFFFFF/* SongLength / SongPosition not supported */

/*-----------------------------------------------------------------------------
Description:
	25 Appendix H: List of defined notification events.
-----------------------------------------------------------------------------*/
#define AVRCP_EVENT_PLAYBACK_STATUS_CHANGED							0x01
#define AVRCP_EVENT_TRACK_CHANGED									0x02
#define AVRCP_EVENT_TRACK_REACHED_END								0x03/* If any action (e.g. GetElementAttributes) is undertaken on the CT as reaction to the EVENT_TRACK_REACHED_END, the CT should register the EVENT_TRACK_REACHED_END again before initiating this action in order to get informed about intermediate changes regarding the track status. */
#define AVRCP_EVENT_TRACK_REACHED_START								0x04/* If any action (e.g. GetElementAttributes) is undertaken on the CT as reaction to the EVENT_TRACK_REACHED_START, the CT should register the EVENT_TRACK_REACHED_START again before initiating this action in order to get informed about intermediate changes regarding the track status. */
#define AVRCP_EVENT_PLAYBACK_POS_CHANGED							0x05
#define AVRCP_EVENT_BATT_STATUS_CHANGED								0x06
#define AVRCP_EVENT_SYSTEM_STATUS_CHANGED							0x07
#define AVRCP_EVENT_PLAYER_APPLICATION_SETTING_CHANGED				0x08
#define AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED						0x09/* If the NowPlaying folder is browsed as reaction to the EVENT_NOW_PLAYING_CONTENT_CHANGED, the CT should register the EVENT_NOW_PLAYING_CONTENT_CHANGED again before browsing the NowPlaying folder in order to get informed about intermediate changes in that folder. */
#define AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED						0x0A/* If the Media Player List is browsed as reaction to the EVENT_AVAILABLE_PLAYERS_CHANGED, the CT should register the EVENT_AVAILABLE_PLAYERS_CHANGED again before browsing the Media Player list in order to get informed about intermediate changes of the available players. */
#define AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED						0x0B
#define AVRCP_EVENT_UIDS_CHANGED									0x0C/* If the Media Player Virtual Filesystem is browsed as reaction to the EVENT_UIDS_CHANGED, the CT should register the EVENT_UIDS_CHANGED again before browsing the Media Player Virtual Filesystem in order to get informed about intermediate changes within the fileystem. */
#define AVRCP_EVENT_VOLUME_CHANGED									0x0D/* 0x0e-0xFF reserved for future use */

/* System status for AVRCP_EVENT_SYSTEM_STATUS_CHANGED */
#define AVRCP_SYSTEM_POWER_ON										0x00
#define AVRCP_SYSTEM_POWER_OFF										0x01
#define AVRCP_SYSTEM_UNPLUGGED										0x02

/*---------------------------------------------------------------------------*/
/* 								6.10 Media Content Navigation         						   */
/*---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
Description:
	6.10.1 There are four scopes in which media content navigation may take place. 
	Table 6.45: Scopes summarizes them and they are described in more detail in the 
	following sections.
-----------------------------------------------------------------------------*/
#define AVRCP_SCOPE_MEDIAPLAYER_LIST								0x00/* Media Player Item, Contains all available media players */
#define AVRCP_SCOPE_MEDIAPLAYER_VIRTUAL_FILESYSTEM					0x01/* Folder Item and Media Element Item, The virtual filesystem containing the media content of the browsed player */
#define AVRCP_SCOPE_MEDIAPLAYER_SEARCH								0x02/* Media Element Item, The results of a search operation on the browsed player */
#define AVRCP_SCOPE_MEDIAPLAYER_NOWPLAYING							0x03/* Media Element Item, The Now Playing list (or queue) of the addressed player */

/* Item Type - 1 Octet */
#define AVRCP_ITEMTYPE_MEDIAPLAYER_ITEM								0x01
#define AVRCP_ITEMTYPE_FOLDER_ITEM									0x02
#define AVRCP_ITEMTYPE_MEDIAELEMENT_ITEM							0x03

/*-----------------------------------------------------------------------------
Description:
	23 Appendix E: List of Media Attributes.
-----------------------------------------------------------------------------*/
#define AVRCP_MA_ILLEGAL											0x00/* should not be used */
#define AVRCP_MA_TITLEOF_MEDIA										0x01/* Any text encoded in specified character set */
#define AVRCP_MA_NAMEOF_ARTIST										0x02/* Any text encoded in specified character set */
#define AVRCP_MA_NAMEOF_ALBUM										0x03/* Any text encoded in specified character set */
#define AVRCP_MA_NUMBEROF_MEDIA										0x04/* Numeric ASCII text with zero suppresses, ex. Track number of the CD */
#define AVRCP_MA_TOTALNUMBEROF_MEDIA								0x05/* Numeric ASCII text with zero suppresses */
#define AVRCP_MA_GENRE												0x06/* Any text encoded in specified character set */
#define AVRCP_MA_PLAYING_TIME										0x07/* Playing time in millisecond, 2min30sec->150000, 08-0xFFFFFFFF reserved for future use */

/*-----------------------------------------------------------------------------
Description:
	6.10.2.1 Media Player Item
-----------------------------------------------------------------------------*/
/* Major Player Type - 1 Octet */
#define AVRCP_MAJORPLAYERTYPE_AUDIO									0x01
#define AVRCP_MAJORPLAYERTYPE_VIDEO									0x02
#define AVRCP_MAJORPLAYERTYPE_BROADCASTING_AUDIO					0x04
#define AVRCP_MAJORPLAYERTYPE_BROADCASTING_VIDEO					0x08

/* Player Sub Type - 4 Octets */
#define AVRCP_PLAYERSUBTYPE_NONE									0x00000000
#define AVRCP_PLAYERSUBTYPE_AUDIOBOOK								0x00000001
#define AVRCP_PLAYERSUBTYPE_PODCAST									0x00000002

/* Feature Bit Mask - 16 Octets */
#define AVRCP_FBM_OCTET_ALL											0xFF

/* Octet 0 */
#define AVRCP_FBM_SELECT											0x01
#define AVRCP_FBM_UP												0x02
#define AVRCP_FBM_DOWN												0x04
#define AVRCP_FBM_LEFT												0x08
#define AVRCP_FBM_RIGHT												0x10
#define AVRCP_FBM_RIGHTUP											0x20
#define AVRCP_FBM_RIGHTDOWN											0x40
#define AVRCP_FBM_LEFTUP											0x80

/* Octet 1 */
#define AVRCP_FBM_LEFTDOWN											0x01
#define AVRCP_FBM_ROOTMENU											0x02
#define AVRCP_FBM_SETUPMENU											0x04
#define AVRCP_FBM_CONTENTSMENU										0x08
#define AVRCP_FBM_FAVORITEMENU										0x10
#define AVRCP_FBM_EXIT												0x20
#define AVRCP_FBM_0													0x40
#define AVRCP_FBM_1													0x80

/* Octet 2 */
#define AVRCP_FBM_2													0x01
#define AVRCP_FBM_3													0x02
#define AVRCP_FBM_4													0x04
#define AVRCP_FBM_5													0x08
#define AVRCP_FBM_6													0x10
#define AVRCP_FBM_7													0x20
#define AVRCP_FBM_8													0x40
#define AVRCP_FBM_9													0x80

/* Octet 3 */
#define AVRCP_FBM_DOT												0x01
#define AVRCP_FBM_ENTER												0x02
#define AVRCP_FBM_CLEAR												0x04
#define AVRCP_FBM_CHANNELUP											0x08
#define AVRCP_FBM_CHANNELDOWN										0x10
#define AVRCP_FBM_PREVIOUSCHANNEL									0x20
#define AVRCP_FBM_SOUNDSELECT										0x40
#define AVRCP_FBM_INPUTSELECT										0x80

/* Octet 4 */
#define AVRCP_FBM_DISPLAY_INFORMATION								0x01
#define AVRCP_FBM_HELP												0x02
#define AVRCP_FBM_PAGEUP											0x04
#define AVRCP_FBM_PAGEDOWN											0x08
#define AVRCP_FBM_POWER												0x10
#define AVRCP_FBM_VOLUMEUP											0x20
#define AVRCP_FBM_VOLUMEDOWN										0x40
#define AVRCP_FBM_MUTE												0x80

/* Octet 5 */
#define AVRCP_FBM_PLAY												0x01
#define AVRCP_FBM_STOP												0x02
#define AVRCP_FBM_PAUSE												0x04
#define AVRCP_FBM_RECORD											0x08
#define AVRCP_FBM_REWIND											0x10
#define AVRCP_FBM_FASTFORWARD										0x20
#define AVRCP_FBM_EJECT												0x40
#define AVRCP_FBM_FORWARD											0x80

/* Octet 6 */
#define AVRCP_FBM_BACKWARD											0x01
#define AVRCP_FBM_ANGLE												0x02
#define AVRCP_FBM_SUBPICTURE										0x04
#define AVRCP_FBM_F1												0x08
#define AVRCP_FBM_F2												0x10
#define AVRCP_FBM_F3												0x20
#define AVRCP_FBM_F4												0x40
#define AVRCP_FBM_F5												0x80

/* Octet 7 */
#define AVRCP_FBM_VENDOR_UNIQUE										0x01
#define AVRCP_FBM_BASIC_GROUP_NAVIGATION							0x02
#define AVRCP_FBM_ADVANCED_CONTROL_PLAYER							0x04
#define AVRCP_FBM_BROWSING											0x08
#define AVRCP_FBM_SEARCHING											0x10
#define AVRCP_FBM_ADDTO_NOWPLAYING									0x20
#define AVRCP_FBM_UIDS_UNIQUE_INPLAYERBROWSE_TREE					0x40
#define AVRCP_FBM_ONLY_BROWSABLE_WHEN_ADDRESSED						0x80

/* Octet 8 */
#define AVRCP_FBM_ONLY_SEARCHABLE_WHEN_ADDRESSED					0x01
#define AVRCP_FBM_NOWPLAYING										0x02
#define AVRCP_FBM_UIDPERSISTENCY									0x04

/*-----------------------------------------------------------------------------
Description:
	6.10.2.2 Folder Item
-----------------------------------------------------------------------------*/
/* Folder Type - 1 Octet */
#define AVRCP_FOLDERTYPE_MIXED										0x00
#define AVRCP_FOLDERTYPE_TITLES										0x01
#define AVRCP_FOLDERTYPE_ALBUMS										0x02
#define AVRCP_FOLDERTYPE_ARTISTS									0x03
#define AVRCP_FOLDERTYPE_GENRES										0x04
#define AVRCP_FOLDERTYPE_PLAYLISTS									0x05
#define AVRCP_FOLDERTYPE_YEARS										0x06

/* Is Playable - 1 Octet */
#define AVRCP_ISPLAYABLE_CANNOT										0x00
#define AVRCP_ISPLAYABLE_CAN										0x01

/* Media Type - 1 Octet */
#define AVRCP_MEDIATYPE_AUDIO										0x00
#define AVRCP_MEDIATYPE_VIDEO										0x01

/*---------------------------------------------------------------------------*/
/* 								6.10.4 Browsing Commands       				 */
/*---------------------------------------------------------------------------*/
#define AVRCP_DIRECTION_FOLDER_UP									0x00
#define AVRCP_DIRECTION_FOLDER_DOWN									0x01

/*---------------------------------------------------------------------------*/
/* 								6.13 Volume Handling		     			 */
/*---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
Description:
	6.13 An Absolute Volume is represented in one octet. The top bit (bit 7) is reserved for 
	future definition. The volume is specified as a percentage of the maximum. The value 0x0 
	corresponds to 0%. The value 0x7F corresponds to 100%. Scaling should be applied to 
	achieve values between these two.
-----------------------------------------------------------------------------*/
#define AVRCP_ABSOLUTE_VOLUME_MIN									0x00
#define AVRCP_ABSOLUTE_VOLUME_MAX									0x7F

/*---------------------------------------------------------------------------*/
/* 								6.14 Basic Group Navigation	     			 */
/*---------------------------------------------------------------------------*/
#define AVRCP_BGN_NEXTGROUP											0x0000
#define AVRCP_BGN_PREVIOUSGROUP										0x0001

/*---------------------------------------------------------------------------*/
/* 								6.15 Error handling			     			 */
/*---------------------------------------------------------------------------*/
/* Table 6.49: List of Error Status Code, 6.15.3 Status and Error Codes */
#define AVRCP_ERROR_INVALID_COMMAND									0x00/* All */
#define AVRCP_ERROR_INVALID_PARAMETER								0x01/* All */
#define AVRCP_ERROR_SPECIFIED_PARAMETER_NOTFOUND					0x02/* All */
#define AVRCP_ERROR_INTERNAL_ERROR									0x03/* All */
#define AVRCP_ERROR_SUCCESSFUL										0x04/* All except where the response CType is AV/C REJECTED */
#define AVRCP_ERROR_UID_CHANGED										0x05/* All */
#define AVRCP_ERROR_RESERVED										0x06/* All, ??? */
#define AVRCP_ERROR_INVALID_DIRECTION								0x07/* The Direction parameter is invalid, Change Path */
#define AVRCP_ERROR_NOTA_DIRECTORY									0x08/* The UID provided does not refer to a folder item, Change Path */
#define AVRCP_ERROR_UID_DOESNOT_EXIST								0x09/* The UID provided does not refer to any currently valid item. Change Path, PlayItem, AddToNowPlaying, GetItemAttributes */
#define AVRCP_ERROR_INVALID_SCOPE									0x0A/* The scope parameter is invalid. GetFolderItems, PlayItem, AddToNowPlayer, GetItemAttributes. */
#define AVRCP_ERROR_RANGE_OUTOF_BOUNDS								0x0B/* The start of range provided is not valid, GetFolderItems */
#define AVRCP_ERROR_UID_ISA_DIRECTORY								0x0C/* The UID provided refers to a directory, which cannot be handled by this media player, PlayItem, AddToNowPlaying */
#define AVRCP_ERROR_MEDIA_INUSE										0x0D/* The media is not able to be used for this operation at this time, PlayItem, AddToNowPlaying */
#define AVRCP_ERROR_NOWPLAYING_LISTFULL								0x0E/* No more items can be added to the Now Playing List, AddToNowPlaying */
#define AVRCP_ERROR_SEARCH_NOTSUPPORTED								0x0F/* The Browsed Media Player does not support search, Search */
#define AVRCP_ERROR_SEARCH_INPROGRESS								0x10/* A search operation is already in progress, Search */
#define AVRCP_ERROR_INVALID_PLAYERID								0x11/* The specified Player Id does not refer to a valid player, SetAddressedPlayer, SetBrowsedPlayer */
#define AVRCP_ERROR_PLAYER_NOT_BROWSABLE							0x12/* The Player Id supplied refers to a Media Player which does not support browsing. SetBrowsedPlayer */
#define AVRCP_ERROR_PLAYER_NOT_ADDRESSED							0x13/* The Player Id supplied refers to a player which is not currently addressed, and the command is not able to be performed if the player is not set as addressed. Search, SetBrowsedPlayer */
#define AVRCP_ERROR_NO_VALID_SEARCH_RESULTS							0x14/* The Search result list does not contain valid entries, e.g. after being invalidated due to change of browsed player. GetFolderItems */
#define AVRCP_ERROR_NO_AVAILABLE_PLAYERS							0x15/* All */
#define AVRCP_ERROR_ADDRESSED_PLAYER_CHANGED						0x16/* Register Notification. 0x17-0xff Reserved all */
#define AVRCP_ERROR_TIMEOUT											0x88/* Used AVRCP internal, not defined in spec.. */
#define AVRCP_ERROR_COMMAND_NOT_IMPLEMENTED							0x89/* USed AVRCP internal, not defined in spec.. */

/*---------------------------------------------------------------------------*/
/* 								AVRCP Interface Functions	     			 */
/*---------------------------------------------------------------------------*/
void AVRCP_Init(void);
void AVRCP_Done(void);
UINT8 AVRCP_ServerStart(UINT8 svr_cls, UINT16 features);
UINT8 AVRCP_ServerStop(void);
void AVRCP_RegCbk(UINT8 *pfunc);
void AVRCP_ConnectReq(struct AVRCP_ConnectReqStru *in);
void AVRCP_DisconnectReq(UINT8 *bd);

void AVRCP_UnitInfo_Req(UINT8 *bd);
void AVRCP_UnitInfo_Rsp(UINT8 *bd, UINT8 *company_id, UINT32 tlcr_pdu_id);
void AVRCP_SubUnitInfo_Req(UINT8 *bd);
void AVRCP_SubUnitInfo_Rsp(UINT8 *bd, UINT32 tlcr_pdu_id);
void AVRCP_PassThrough_Req(UINT8 *bd, struct AVRCP_PassThroughStru *in);
void AVRCP_PassThrough_Rsp(UINT8 *bd, struct AVRCP_PassThroughStru *in, UINT32 tlcr_pdu_id);

void AVRCP_FreeEventParam(UINT32 event, UINT8 *param);

#ifdef CONFIG_AVRCP_14
void AVRCP_AdvancedControl_Req(UINT8 *bd, UINT32 tlcr_pdu_id, void *data, UINT16 len);
void AVRCP_AdvancedControl_Rsp(UINT8 *bd, UINT32 tlcr_pdu_id, void *data, UINT16 len);
void AVRCP_RegisterNotif_Rsp(UINT32 tlcr_pdu_id, struct AVRCP_RegisterNotifRspStru *data, UINT16 len);
void AVRCP_GeneralReject_Rsp(UINT8 *bd, UINT8 error_code, UINT32 tlcr_pdu_id);
#endif //end of CONFIG_AVRCP_14

#endif

