/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
Module Name:
	rfcomm_ui.h
Abstract:
	This module defines the rfcomm internal tool functions.
---------------------------------------------------------------------------*/

#ifndef RFCOMM_UIH_
#define RFCOMM_UIH_

typedef void (RFCOMM_CbkFuc)(UINT16 msg, void *arg);/* callback to upper protocol */

struct RFCOMM_RegServerDLCIStru {
	UINT8 	server_channel;			/* Input, then output to cfm */
	UINT8 	auto_credit;			/* RFCOMM Internal credits control */
	UINT8 	init_credit;			/* max 7, 0~7 */
	UINT16 	mfs;					/* ? need this? */
	void 	*cbk;
};

struct RFCOMM_ConnectStru {			/* For connect_req ui */
	UINT8 	bd[BD_ADDR_LEN];
	UINT8 	server_channel;			/* server dlci */
	UINT8 	init_credit;			/* max 7, 0~7 */
	UINT8 	auto_credit;			/* RFCOMM_DLCIX_MASK_INTERNAL_AUTOCREDITS */
	UINT16	mfs;					/* PN param, 1 to 32768, default is 127, only include information field. */
	void 	*cbk;
};

struct RFCOMM_ConnectEvStru {
	UINT8 	bd[BD_ADDR_LEN];
	UINT8 	server_channel;
	UINT32	handle;					/* RFCOMM connection handle, formed by L2CAP CID and DLCI */
	UINT16 	result;
	UINT16 	mfs;
	UINT16	credit_incoming;		/* and give init_credit? */
	UINT16	credit_outgoing;
};

struct RFCOMM_DisconnectReqStru {
	UINT32 	handle;					/* RFCOMM handle */
};

struct RFCOMM_UIH_TestStru {		/* arg in RFCOMMS_UIH_Test_Req */
	UINT32 handle;					/* RFCOMM handle */
	UINT16 value_len;
	UINT8 value[1];
};

struct RFCOMM_UIH_FCStru {			/* arg in RFCOMMS_UIH_FC_Req */
	UINT32 handle;					/* RFCOMM handle */
	UINT8 is_on;					/* on=1, off=0 */
};

struct RFCOMM_UIH_MSCStru {			/* arg in RFCOMMS_UIH_MSC_Req */
	UINT32 handle;					/* RFCOMM handle */
	UINT8 v24;
	UINT8 break_signal;
};

struct RFCOMM_UIH_RPNStru {			/* arg in RFCOMMS_UIH_RPN_Req */
	UINT32 handle;					/* RFCOMM handle */
	UINT8 bitrate;
	UINT8 databit;					/* D */
	UINT8 stop;
	UINT8 parity;					/* P */
	UINT8 parity_type;
	UINT8 flow_control_type;
	UINT8 xon;
	UINT8 xoff;
	UINT16 pm;
};

struct RFCOMM_UIH_RLSStru {			/* arg in RFCOMMS_UIH_RLS_Req */
	UINT32 handle;					/* RFCOMM handle */
	UINT8 line_status;				/* L1-L4 bits */
};

enum {/* For RFCOMM_EventCBK */
	RFCOMM_EV_CONNECT_CFM = 0x01,	/* DLCI x created, active side */
	RFCOMM_EV_CONNECT_IND,			/* DLCI x created, passive side */
	RFCOMM_EV_CONNECT_COMPLETE,		/* DLCI x connect complete */
	RFCOMM_EV_DISCONNECT_COMPLETE,	/* DLCI x disconnect complete, these events use struct RFCOMM_ConnectEvStru */
	RFCOMM_EV_REGSERVER_DLCI_CFM,	/* Server register local DLCI confirm, struct RFCOMM_RegServerDLCIStru */
	RFCOMM_EV_DATA_IND,				/* DLCI x UIH received. */
	RFCOMM_EV_UIH_TEST_CFM,			/* receive a UIH Test response */
	RFCOMM_EV_UIH_FC_IND,			/* receive a UIH FC On/Off request */
	RFCOMM_EV_UIH_MSC_IND,			/* receive a UIH MSC request */
	RFCOMM_EV_UIH_MSC_CFM,			/* receive a UIH MSC response */
	RFCOMM_EV_UIH_RPN_IND,			/* receive a UIH RPN request */
	RFCOMM_EV_UIH_RPN_CFM,			/* receive a UIH RPN response */
	RFCOMM_EV_UIH_RLS_IND,			/* receive a UIH RLS request */
	RFCOMM_EV_UIH_RLS_CFM			/* receive a UIH RLS response */	
};

enum {/* RFCOMM reason error code */
	ER_RFCOMM_DLCI0_DONE = 0x80,/* Bitmask, to protect DLCI0 done twice when DLCIx done is called by DLCI0 Done */
		
	ER_RFCOMM_SUCCESS = 0x00,
	ER_RFCOMM_L2CAP_CONNECT_FAILED = 0x01,/* 0x00 leave to OK */
	ER_RFCOMM_L2CAP_DISCONNECT_COMPLETE,
	ER_RFCOMM_UA_IND_DISC,
	ER_RFCOMM_DM_IND,
	ER_RFCOMM_CONNECTION_DISCONNECT_REQ,
	ER_RFCOMM_DLCI_DISCONNECT_REQ,
	ER_RFCOMM_DLCI_UA_IND_DISC,
	ER_RFCOMM_DLCI_DM_IND,
	ER_RFCOMM_DLCI_DISC_IND,
	ER_RFCOMM_DLCI_UNREG_SERVER,
	ER_RFCOMM_RTX_TIMEOUT,
	ER_RFCOMM_CONNECT_RSP_FAILED,	/* Upper do */
	ER_RFCOMM_CONNECT_REQ_DLCIX_DUPLICATE,/* BD and Server_channel duplicate connect_req. */
	ER_RFCOMM_CONNECT_REQ_INVALID_SERVERCHANNEL,
	ER_RFCOMM_DATA_IND_ERROR,
	ER_RFCOMM_SECURITY_FAILED,
	ER_RFCOMM_NSC_CFM,
	ER_RFCOMM_DONE
};

enum {/* Bitrate in UIH-RPN */
	RFCOMM_UIH_RPN_BBIT_2400 = 		0x00,/* 0,0,0,0,0,0,0,0 */
	RFCOMM_UIH_RPN_BBIT_4800 = 		0x01,/* 1,0,0,0,0,0,0,0 */
	RFCOMM_UIH_RPN_BBIT_7200 = 		0x02,/* 0,1,0,0,0,0,0,0 */
	RFCOMM_UIH_RPN_BBIT_9600 = 		0x03,/* 1,1,0,0,0,0,0,0 */
	RFCOMM_UIH_RPN_BBIT_19200 = 	0x04,/* 0,0,1,0,0,0,0,0 */
	RFCOMM_UIH_RPN_BBIT_38400 = 	0x05,/* 1,0,1,0,0,0,0,0 */
	RFCOMM_UIH_RPN_BBIT_57600 = 	0x06,/* 0,1,1,0,0,0,0,0 */
	RFCOMM_UIH_RPN_BBIT_115200 = 	0x07,/* 1,1,1,0,0,0,0,0 */
	RFCOMM_UIH_RPN_BBIT_230400 = 	0x08/* 0,0,0,1,0,0,0,0 */
};

enum {/* Databits in UIH-RPN */
	RFCOMM_UIH_RPN_D_5BITS = 		0x00,/* 00 */
	RFCOMM_UIH_RPN_D_6BITS = 		0x01,/* 01 */
	RFCOMM_UIH_RPN_D_7BITS = 		0x02,/* 10 */
	RFCOMM_UIH_RPN_D_8BITS = 		0x03/* 11 */
};

enum {/* Parity-Type in UIH-RPN */
	RFCOMM_UIH_RPN_PT_ODD = 		0x00,/* 00 */
	RFCOMM_UIH_RPN_PT_EVEN = 		0x01,/* 01 */
	RFCOMM_UIH_RPN_PT_MARK = 		0x02,/* 10 */
	RFCOMM_UIH_RPN_PT_SPACE = 		0x03/* 11 */
};

enum {/* PM Mask in RPN */
	RFCOMM_RPN_PM_BITRATE = 		0x0001,/* Low bit 1 */
	RFCOMM_RPN_PM_DATABITS = 		0x0002,/* Low bit 2 */
	RFCOMM_RPN_PM_STOP = 			0x0004,/* Low bit 3 */
	RFCOMM_RPN_PM_PARITY = 			0x0008,/* Low bit 4 */
	RFCOMM_RPN_PM_PARITY_TYPE = 	0x0010,/* Low bit 5 */
	RFCOMM_RPN_PM_XON = 			0x0020,/* Low bit 6 */
	RFCOMM_RPN_PM_XOFF = 			0x0040,/* Low bit 7 */
	
	RFCOMM_RPN_PM_X_INPUT = 		0x0100,/* High bit 1 */
	RFCOMM_RPN_PM_X_OUTPUT = 		0x0200,/* High bit 2 */
	RFCOMM_RPN_PM_PTR_INPUT = 		0x0400,/* High bit 3 */
	RFCOMM_RPN_PM_PTR_OUTPUT = 		0x0800,/* High bit 4 */
	RFCOMM_RPN_PM_RTC_INPUT = 		0x1000,/* High bit 5 */
	RFCOMM_RPN_PM_RTC_OUTPUT = 		0x2000,/* High bit 6 */
	
	RFCOMM_RPN_PM_DEFAULT = 		0x3F7F/* Low 7bits, High 6bits */
};

enum {/* Line 234 bits in UIH-RLS */
	RFCOMM_UIH_RLS_LINE_OVERRUN = 			0x01,/* 100 */
	RFCOMM_UIH_RPN_LINE_PARITY = 			0x02,/* 010 */
	RFCOMM_UIH_RPN_LINE_FRAMEERROR = 		0x04/* 001 */
};

enum {/* FC */
	RFCOMM_UIH_RPN_FC_XONXOFF_ON_INPUT = 	0x01,
	RFCOMM_UIH_RPN_FC_XONXOFF_ON_OUTPUT = 	0x02,
	RFCOMM_UIH_RPN_FC_RTR_ON_INPUT = 		0x04,
	RFCOMM_UIH_RPN_FC_RTR_ON_OUTPUT = 		0x08,
	RFCOMM_UIH_RPN_FC_RTC_ON_INPUT = 		0x10,
	RFCOMM_UIH_RPN_FC_RTC_ON_OUTPUT = 		0x20
};

/*---------------------------------------------------------------------------
Description:
	This function used to encode a Modem Status Command (MSC) V24 field.
	
Bit 1. The EA bit is set to 1 in the last octet of the sequence; in other octets EA is set to 0. 
If only one octet is transmitted EA is set to 1.
Bit 2.Flow Control (FC). The bit is set to 1(one) when the device is unable to accept frames.
Bit 3. Ready To Communicate (RTC). The bit is set to 1 when the device is ready to communicate.
Bit 4. Ready To Receive (RTR). The bit is set to 1 when the device is ready to receive data.
Bit 5. Reserved for future use. Set to zero by the sender, ignored by the receiver.
Bit 6. Reserved for future use. Set to zero by the sender, ignored by the receiver.
Bit 7. Incoming call indicator (IC). The bit is set to 1 to indicate an incoming call.
Bit 8. Data Valid (DV). The bit is set to 1 to indicate that valid data is being sent	
---------------------------------------------------------------------------*/
#define RFCOMM_Compose_UIH_MSC_V24(rtc, rtr, ic, dv) (RFCOMM_EA | (1 << 1) | ((rtc) << 2) | ((rtr) << 3) | ((ic) << 6) | ((dv) << 7))

void RFCOMM_Init(void);
void RFCOMM_Done(void);
void RFCOMM_RegServer(struct RFCOMM_RegServerDLCIStru *in);
void RFCOMM_UnRegServer(struct RFCOMM_RegServerDLCIStru *in);
void RFCOMM_Connect_Req(struct RFCOMM_ConnectStru *in);
void RFCOMM_Connect_Rsp(struct RFCOMM_ConnectEvStru *in);
void RFCOMM_Disconnect_Req(struct RFCOMM_DisconnectReqStru *in);
void RFCOMM_Data_Req(struct BuffStru *in);
struct BuffStru *RFCOMM_DataNew(UINT16 len, UINT8 credit, UINT32 handle);

void RFCOMM_UIH_Test_Req(struct RFCOMM_UIH_TestStru *in);
void RFCOMM_UIH_FC_Req(struct RFCOMM_UIH_FCStru *in);
void RFCOMM_UIH_MSC_Req(struct RFCOMM_UIH_MSCStru *in);
void RFCOMM_UIH_RPN_Req(struct RFCOMM_UIH_RPNStru *in);
void RFCOMM_UIH_RLS_Req(struct RFCOMM_UIH_RLSStru *in);

#endif

