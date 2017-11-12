/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Module Name:
	avctp_ui.h
Abstract:
	This module defines all of the AVCTP user interfaces provided to upper layer(AVRCP).
	also defines all indication callback function provided to L2CAP.
-----------------------------------------------------------------------------*/

#ifndef AVCTP_UI_H
#define AVCTP_UI_H

#include "global.h"
#include "l2cap_ui.h"

/*---------------------------------------------------------------------------*/
/* 						AVCTP Interface Macros/Structures					 */
/*---------------------------------------------------------------------------*/
typedef void (AVCTP_CbkFuc)(UINT8 ev, void *arg);/* callback to upper profile */

/* AVCTP is big endian, this encode/decode function is ui for avrcp */
#define AVCTP_DECODE2BYTE			DECODE2BYTE_BIG
#define AVCTP_DECODE4BYTE			DECODE4BYTE_BIG
#define AVCTP_ENCODE2BYTE			ENCODE2BYTE_BIG
#define AVCTP_ENCODE4BYTE			ENCODE4BYTE_BIG

enum {/* Events for upper layer */
	AVCTP_EV_CONNECT_COMPLETE = 	0x01,
	AVCTP_EV_DISCONNECT_COMPLETE = 	0x02,
	AVCTP_EV_INVALID_PID =			0x03,/* remote donot know this pid */
	AVCTP_EV_DATA_IND =				0x04
};

struct AVCTP_RegCbkStru {/* member in pid_cbklist */
	UINT16 	pid;
	UINT8 	*cbk;
};

struct AVCTP_ConnectReqStru {/* param of connect_req and disconnect_req */
	UINT8 	bd_addr[BD_ADDR_LEN];
	UINT16  pid;
};

struct AVCTP_ConnectCompleteStru {/* Used for event AVCTP_EV_CONNECT_COMPLETE/AVCTP_EV_DISCONNECT_COMPLETE */
	UINT8 	bd_addr[BD_ADDR_LEN];
	UINT8 	channel;					/* e.g AVCTP_CONTROL_CHANNEL */
	UINT8 	result;						/* e.g BT_SUCCESS */
	UINT8	side;						/* e.g AVCTP_CHANNEL_INITIATOR */
	UINT16 	mtu;						/* outgoing browsing mtu, for browsing channel */
};

struct AVCTP_DataStru {					/* param of data_req */
	UINT8 	bd_addr[BD_ADDR_LEN];
	UINT8	channel;					/* Control or browsing channel */
	UINT8 	tl;       					/* Transaction label */
	UINT8 	msg_type; 					/* Message type:  e.g: AVCTP_COMMAND_FRAME */
	UINT32 	timeout;					/* Used for AVRCP pass timeout value to control fsm */
	UINT16	pid;     					/* Profile id */ 
	UINT16 	len;     					/* Data length */
	UINT8  	data[1]; 					/* AV/C Command or Response message */
};

/* ConnectCompleteStru side */
#define AVCTP_CHANNEL_INITIATOR					L2CAP_CHANNEL_INITIATOR/* 1 */
#define AVCTP_CHANNEL_RESPONDER					L2CAP_CHANNEL_RESPONDER/* 0 */

/* Used in struct AVCTP_DataStru to define which channel used */
#define AVCTP_CONTROL_CHANNEL					0x00
#define AVCTP_BROWSING_CHANNEL					0x01

/* C/R flag in AVCTP packet */
#define AVCTP_COMMAND_FRAME						0x00
#define AVCTP_RESPONSE_FRAME					0x01

FUNC_EXPORT void AVCTP_Init(void);
FUNC_EXPORT void AVCTP_Done(void);
FUNC_EXPORT void AVCTP_RegCbk(UINT16 pid, UINT8 *cbk);
FUNC_EXPORT void AVCTP_ConnectReq(struct AVCTP_ConnectReqStru *in);
FUNC_EXPORT void AVCTP_DisconnectReq(struct AVCTP_ConnectReqStru *in);
FUNC_EXPORT void AVCTP_DataReq(struct AVCTP_DataStru *msg);

#endif

