/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
Module Name:
    hfp_ui.h
Abstract:
	This module defines the common API for both AG and HF device.
------------------------------------------------------------------------*/

#ifndef HFP_UI_H
#define HFP_UI_H

#include "stdlib.h"
#include "global.h"
#include "hci_ui.h"	/* include HCI and GAP to use SCO */
#include "sdap_ui.h"
#include "rfcomm_ui.h"

typedef UINT8 (*HFP_Callback)(void *hdl, UINT8 event, UINT8 *param, UINT32 len);/* callback to upper profile */

enum {/* HFP_Connection_Done reason, used in HFP_EV_DISCONNECT_COMPLETE & HFP_EV_CONNECT_COMPLETE */
	ER_HFP_RFCOMM_SUCCESS = 0x00,
	ER_HFP_RFCOMM_CONNECT_FAILED,
	ER_HFP_MULTICONNECT_BETWEEN_TWOENDS,
	ER_HFP_DISCONNECT_REQ_BEFORE_HFPCONNECT,
	ER_HFP_SDAP_FAILED,
	ER_HFP_SLC_TIMEOUT,
	ER_HFP_CLIENT_STOP,
	ER_HFP_RFCOMM_DISCONNECT_COMPLETE,/* Disconnect_Ind */
	ER_HFP_SERVER_STOP,
	ER_HFP_DONE
};

/*---------------------------------------------------------------------------*/
/* 							 		HF / AG Role					 	     */
/*---------------------------------------------------------------------------*/
enum {
	/* Bitmask */
	HFP_ROLE_SERVER = 							0x04,
	HFP_ROLE_HIGH_VERSION = 					0x02,/* HFP/HST */
	HFP_ROLE_HF_SIDE = 							0x01,/* HF/AG */

	/* Final Role */
	HFP_ROLE_HF_SERVER = 			  			(HFP_ROLE_SERVER | HFP_ROLE_HIGH_VERSION | HFP_ROLE_HF_SIDE),	/* 0x07 */
	HFP_ROLE_HF_CLIENT = 			  			(HFP_ROLE_HF_SIDE | HFP_ROLE_HIGH_VERSION),						/* 0x03 */
	HFP_ROLE_AG_SERVER = 			  			(HFP_ROLE_SERVER | HFP_ROLE_HIGH_VERSION),						/* 0x06 */
	HFP_ROLE_AG_CLIENT = 			 		 	(HFP_ROLE_HIGH_VERSION),										/* 0x02 */
	HFP_ROLE_HS_SERVER = 			  			(HFP_ROLE_SERVER | HFP_ROLE_HF_SIDE),							/* 0x05 */
	HFP_ROLE_HS_CLIENT = 		  	  			(HFP_ROLE_HF_SIDE),												/* 0x01 */
	HFP_ROLE_VG_SERVER = 				  		(HFP_ROLE_SERVER),												/* 0x04 */
	HFP_ROLE_VG_CLIENT = 				      	(0)						
};

/*---------------------------------------------------------------------------*/
/* 							 		HF / AG BRSF Features	       		     */
/*---------------------------------------------------------------------------*/
enum {/* BRSF feature mask ID for HF, defined in spec */
	/* AT+BRSF */
	HFP_BRSF_HF_NREC = 							0x00000001,/* 0, EC and/or NR function */
	HFP_BRSF_HF_3WAYCALL = 						0x00000002,/* 1, Call waiting and 3-way calling */
	HFP_BRSF_HF_CLIP = 							0x00000004,/* 2, CLI presentation capability */
	HFP_BRSF_HF_BVRA = 							0x00000008,/* 3, Voice recognition activation */
	HFP_BRSF_HF_RMTVOLCTRL = 					0x00000010,/* 4, Remote volume control */
	HFP_BRSF_HF_ENHANCED_CALLSTATUS = 			0x00000020,/* 5, Enhanced call status */
	HFP_BRSF_HF_ENHANCED_CALLCONTROL = 			0x00000040,/* 6, Enhanced call control */
	HFP_BRSF_HF_CODEC_NEGOTIATION = 			0x00000080,/* 7, HFP1.6 defined */
	HFP_BRSF_HF_ALL = 							0x000000FF,/* 0~7 Bits */
	HFP_BRSF_HF_DEFAULT = 						0x00000000,/* HF Server Default BRSF Features, P79 */

	/* SDP */
	HFP_SDP_HF_NREC = 							0x00000001,/* 0, EC and/or NR function */
	HFP_SDP_HF_3WAYCALL = 						0x00000002,/* 1, Call waiting and 3-way calling */
	HFP_SDP_HF_CLIP = 							0x00000004,/* 2, CLI presentation capability */
	HFP_SDP_HF_BVRA = 							0x00000008,/* 3, Voice recognition activation */
	HFP_SDP_HF_RMTVOLCTRL = 					0x00000010,/* 4, Remote volume control */
	HFP_SDP_HF_WIDE_BAND_SPEECH = 				0x00000020 /* 5, Wide band speech */
};

enum {/* BRSF feature mask ID for AG, defined in spec */
	/* +BRSF */
	HFP_BRSF_AG_3WAYCALL = 						0x00000001,/* 0, Three-way calling */
	HFP_BRSF_AG_NREC = 							0x00000002,/* 1, EC and/or NR function */
	HFP_BRSF_AG_BVRA = 							0x00000004,/* 2, Voice recognition function */
	HFP_BRSF_AG_INBANDRING = 					0x00000008,/* 3, In-band ring tone capability */
	HFP_BRSF_AG_BINP = 							0x00000010,/* 4, Attach a number to a voice tag */
	HFP_BRSF_AG_REJECT_CALL = 					0x00000020,/* 5, Ability to reject a call */
	HFP_BRSF_AG_ENHANCED_CALLSTATUS = 			0x00000040,/* 6, Enhanced call status */
	HFP_BRSF_AG_ENHANCED_CALLCONTROL = 			0x00000080,/* 7, Enhanced call control */
	HFP_BRSF_AG_EXTENDED_ERRORRESULT = 			0x00000100,/* 8, Extended Error Result Codes */
	HFP_BRSF_AG_CODEC_NEGOTIATION = 			0x00000200,/* 9, HFP1.6 defined */
	HFP_BRSF_AG_ALL = 							0x000003FF,/* 0~9 Bits */
	HFP_BRSF_AG_DEFAULT = 						0x00000009,/* AG Server Default BRSF Features, P80 */

	/* SDP */
	HFP_SDP_AG_3WAYCALL = 						0x00000001,/* 0, Three-way calling */
	HFP_SDP_AG_NREC = 							0x00000002,/* 1, EC and/or NR function */
	HFP_SDP_AG_BVRA = 							0x00000004,/* 2, Voice recognition function */
	HFP_SDP_AG_INBANDRING = 					0x00000008,/* 3, In-band ring tone capability */
	HFP_SDP_AG_BINP = 							0x00000010,/* 4, Attach a number to a voice tag */
	HFP_SDP_AG_WIDE_BAND_SPEECH = 				0x00000020 /* 5, Wide band speech */
};

enum {/* AG Indicator Map, Together with HFP_BRSF_AG_xxx macro for HF device get AG features after +BRSF received */
	HFP_FEA_AG_SERVICE =						0x00010000,
	HFP_FEA_AG_CALL =							0x00020000,
	HFP_FEA_AG_CALLSETUP =						0x00040000,
	HFP_FEA_AG_CALLHELD =						0x00080000,
	HFP_FEA_AG_SIGNAL =							0x00100000,
	HFP_FEA_AG_ROAM =							0x00200000,
	HFP_FEA_AG_BATTCHG =						0x00400000
};						

enum {/* Mapping of Codec Types to Codec ID`s */
	HFP_CODECID_CVSD = 							0x01,
	HFP_CODECID_MSBC = 							0x02
	/* Other optional codec¡®s may be added here */
};

/*---------------------------------------------------------------------------*/
/* 							HFP AT Command and Result Codes					 */
/*---------------------------------------------------------------------------*/
enum {/* Basic String Coding Mask and Index */
	HFP_CODE_MASK = 			0xFF00,				/* Mask part */
	HFP_CODE_ATP = 				0x8000,				/* AT+ */
	HFP_CODE_P = 				0x4000,				/* + */

	HFP_CODE_BIT1 = 			0x0800,				/* AT Command Distinguish, only used in sender, not for coding */
	HFP_CODE_BIT2 = 			0x0400,
	HFP_CODE_BIT3 = 			0x0200,

	HFP_CODE_ATA = 				0x0001,
	HFP_CODE_ATD = 				0x0002,
	HFP_CODE_ERROR = 			0x0003,
	HFP_CODE_OK = 				0x0004,
	HFP_CODE_NO_CARRIER = 		0x0005,
	HFP_CODE_BUSY = 			0x0006,
	HFP_CODE_NO_ANSWER = 		0x0007,
	HFP_CODE_DELAYED = 			0x0008,
	HFP_CODE_BLACKLISTED = 		0x0009,
	HFP_CODE_RING = 			0x000a,
	HFP_CODE_CCWA = 			0x000b,
	HFP_CODE_CHLD = 			0x000c,
	HFP_CODE_CHUP = 			0x000d,
	HFP_CODE_CIND = 			0x000e,
	HFP_CODE_CLCC = 			0x000f,
	HFP_CODE_COPS = 			0x0010,
	HFP_CODE_CMEE = 			0x0011,
	HFP_CODE_CME_ERROR = 		0x0012,
	HFP_CODE_CLIP = 			0x0013,
	HFP_CODE_CMER = 			0x0014,
	HFP_CODE_CIEV = 			0x0015,
	HFP_CODE_VTS = 				0x0016,
	HFP_CODE_CNUM = 			0x0017,
	HFP_CODE_BIA = 				0x0018,
	HFP_CODE_BINP = 			0x0019,
	HFP_CODE_BLDN = 			0x001a,
	HFP_CODE_BVRA = 			0x001b,
	HFP_CODE_BRSF = 			0x001c,
	HFP_CODE_NREC = 			0x001d,
	HFP_CODE_VGM = 				0x001e,
	HFP_CODE_VGS = 				0x001f,
	HFP_CODE_BSIR = 			0x0020,
	HFP_CODE_BTRH = 			0x0021,
	HFP_CODE_BCC = 				0x0022,
	HFP_CODE_BCS = 				0x0023,
	HFP_CODE_BAC = 				0x0024,
	
	HFP_CODE_CKPD = 			0x0025,
	HFP_CODE_CGMM = 			0x0026,
	HFP_CODE_CGMI = 			0x0027,

	HFP_CODE_EXTCMD =  			0x0088    /* Extended at-command, HF/AG use it */
};

enum {/* HF Send AT Command */
	HFP_HFC_ATA = 				HFP_CODE_ATA,										/* ATA */
	HFP_HFC_ATD = 				HFP_CODE_ATD,										/* ATDdd...dd; ATD>nnn...; */
	HFP_HFC_CCWA = 				HFP_CODE_ATP | HFP_CODE_CCWA,						/* AT+CCWA=1 AT+CCWA=0 */
    /* HFP_HFC_CHLD_0, etc. are for compatibility */
	HFP_HFC_CHLD = 				HFP_CODE_ATP | HFP_CODE_CHLD,						/* AT+CHLD=x (0,1,2,3,4) */	
	HFP_HFC_CHLD_0 = 			HFP_CODE_ATP | HFP_CODE_CHLD | HFP_CODE_BIT3,		/* AT+CHLD=0 Held Call Release */	
	HFP_HFC_CHLD_1 = 			HFP_CODE_ATP | HFP_CODE_CHLD | HFP_CODE_BIT2,		/* AT+CHLD=1 Release Specified Active Call */	
	HFP_HFC_CHLD_2 = 			HFP_CODE_ATP | HFP_CODE_CHLD | HFP_CODE_BIT2 | HFP_CODE_BIT3,/* AT+CHLD=2 Call Held or Active/Held Position Swap */	
	HFP_HFC_CHLD_3 = 			HFP_CODE_ATP | HFP_CODE_CHLD | HFP_CODE_BIT1,	    /* AT+CHLD=3 Adds a held call to the conversation */	
	HFP_HFC_CHLD_4 = 			HFP_CODE_ATP | HFP_CODE_CHLD | HFP_CODE_BIT1 | HFP_CODE_BIT3,/* AT+CHLD=4 Connects the two calls and disconnects the subscriber from both calls */	
	HFP_HFC_CHLD_T = 			HFP_CODE_ATP | HFP_CODE_CHLD | HFP_CODE_BIT1 | HFP_CODE_BIT2,/* AT+CHLD=? */	
	HFP_HFC_CHUP = 				HFP_CODE_ATP | HFP_CODE_CHUP,						/* AT+CHUP */
	HFP_HFC_CIND_MAP = 			HFP_CODE_ATP | HFP_CODE_CIND,						/* AT+CIND=? */
	HFP_HFC_CIND_STATUS = 		HFP_CODE_ATP | HFP_CODE_CIND | HFP_CODE_BIT1,		/* AT+CIND? */
	HFP_HFC_CLCC = 				HFP_CODE_ATP | HFP_CODE_CLCC,						/* AT+CLCC */
	HFP_HFC_COPS_SETFORMAT = 	HFP_CODE_ATP | HFP_CODE_COPS,						/* AT+COPS=3,0 */
	HFP_HFC_COPS_NAME = 		HFP_CODE_ATP | HFP_CODE_COPS | HFP_CODE_BIT1,		/* AT+COPS? */
	HFP_HFC_CMEE = 				HFP_CODE_ATP | HFP_CODE_CMEE,						/* AT+CMEE=1 */
	HFP_HFC_CLIP = 				HFP_CODE_ATP | HFP_CODE_CLIP,						/* AT+CLIP=1 Calling Line Identification notification */
	HFP_HFC_CMER_0 = 			HFP_CODE_ATP | HFP_CODE_CMER,						/* AT+CMER=3,0,0,0 Disable indicator events reporting */
	HFP_HFC_CMER_1 = 			HFP_CODE_ATP | HFP_CODE_CMER | HFP_CODE_BIT1,		/* AT+CMER=3,0,0,1 Enable indicator events reporting */
	HFP_HFC_VTS = 				HFP_CODE_ATP | HFP_CODE_VTS,						/* AT+VTS=<DTMF> */
	HFP_HFC_CNUM = 				HFP_CODE_ATP | HFP_CODE_CNUM,						/* AT+CNUM Retrieve Subscriber Number Information */
	HFP_HFC_BIA = 				HFP_CODE_ATP | HFP_CODE_BIA,						/* AT+BIA=[[<indrep 1>][,[<indrep 2>][,¡­[,[<indrep n>]]]]]] */
	HFP_HFC_BINP = 				HFP_CODE_ATP | HFP_CODE_BINP,						/* AT+BINP=1 */
	HFP_HFC_BLDN = 				HFP_CODE_ATP | HFP_CODE_BLDN,						/* AT+BLDN (Bluetooth Last Dialed Number) */
	HFP_HFC_BVRA_0 = 			HFP_CODE_ATP | HFP_CODE_BVRA,						/* AT+BVRA=0 Disable Bluetooth Voice Recognition Activation */
	HFP_HFC_BVRA_1 = 			HFP_CODE_ATP | HFP_CODE_BVRA | HFP_CODE_BIT1,		/* AT+BVRA=1 Enable Bluetooth Voice Recognition Activation */
	HFP_HFC_BRSF = 				HFP_CODE_ATP | HFP_CODE_BRSF,						/* AT+BRSF=<HF supported features bitmap> */
	HFP_HFC_NREC = 				HFP_CODE_ATP | HFP_CODE_NREC,						/* AT+NREC=0 Disable Noise Reduction and Echo Canceling */
	HFP_HFC_VGM = 				HFP_CODE_ATP | HFP_CODE_VGM,						/* AT+VGM=<gain> 0-15 */
	HFP_HFC_VGS = 				HFP_CODE_ATP | HFP_CODE_VGS,						/* AT+VGS=<gain> 0-15 */
	HFP_HFC_BTRH_0 = 			HFP_CODE_ATP | HFP_CODE_BTRH,						/* AT+BTRH=0, Bluetooth Response and Hold Feature */
	HFP_HFC_BTRH_1 = 			HFP_CODE_ATP | HFP_CODE_BTRH | HFP_CODE_BIT1,		/* AT+BTRH=1, Bluetooth Response and Hold Feature */
	HFP_HFC_BTRH_2 = 			HFP_CODE_ATP | HFP_CODE_BTRH | HFP_CODE_BIT2,		/* AT+BTRH=2, Bluetooth Response and Hold Feature */
	HFP_HFC_BTRH_QUERY = 		HFP_CODE_ATP | HFP_CODE_BTRH | HFP_CODE_BIT1 | HFP_CODE_BIT2,/* AT+BTRH?, Bluetooth Response and Hold Feature */
	HFP_HFC_BCC = 				HFP_CODE_ATP | HFP_CODE_BCC,						/* AT+BCC (Bluetooth Codec Connection) */
	HFP_HFC_BCS = 				HFP_CODE_ATP | HFP_CODE_BCS,						/* AT+BCS= <u>  (u is a Codec ID) Bluetooth Codec Selection */
	HFP_HFC_BAC = 				HFP_CODE_ATP | HFP_CODE_BAC,						/* AT+BAC= [<u1>[,<u2>[,...[,<un>]]]] Bluetooth Available Codecs */
	HFP_HFC_CKPD_200 = 			HFP_CODE_ATP | HFP_CODE_CKPD,						/* AT+CKPD=200 */
	HFP_HFC_CGMI = 				HFP_CODE_ATP | HFP_CODE_CGMI,						/* AT+CGMI */
	HFP_HFC_CGMM = 				HFP_CODE_ATP | HFP_CODE_CGMM,						/* AT+CGMM */
};

enum {
	HFP_AGC_OK = 				HFP_CODE_OK,
	HFP_AGC_ERROR = 			HFP_CODE_ERROR,
	HFP_AGC_NO_CARRIER = 		HFP_CODE_NO_CARRIER,
	HFP_AGC_BUSY = 				HFP_CODE_BUSY,
	HFP_AGC_NO_ANSWER = 		HFP_CODE_NO_ANSWER,
	HFP_AGC_DELAYED = 			HFP_CODE_DELAYED,
	HFP_AGC_BLACKLISTED = 		HFP_CODE_BLACKLISTED,
	HFP_AGC_RING = 				HFP_CODE_RING,

	HFP_AGC_CCWA = 				HFP_CODE_P | HFP_CODE_CCWA,							/* +CCWA: <number>,<type> */
	HFP_AGC_CIND = 				HFP_CODE_P | HFP_CODE_CIND,							/* +CIND: <> */
	HFP_AGC_CHLD = 				HFP_CODE_P | HFP_CODE_CHLD,							/* +CHLD: (0,1,1x,2,2x,3,4) */
	HFP_AGC_CLCC = 				HFP_CODE_P | HFP_CODE_CLCC,							/* +CLCC: <id1>,<dir>,<stat>,<mode>,<mpty>[,<number>,<type>] */
	HFP_AGC_COPS = 				HFP_CODE_P | HFP_CODE_COPS,							/* +COPS:<mode>,<format>,<operator> */
	HFP_AGC_CME_ERROR = 		HFP_CODE_P | HFP_CODE_CME_ERROR,					/* +CME ERROR: <err> */
	HFP_AGC_CLIP = 				HFP_CODE_P | HFP_CODE_CLIP,							/* +CLIP: <number>, type> */
	HFP_AGC_CIEV = 				HFP_CODE_P | HFP_CODE_CIEV,							/* +CIEV: <ind>,<value> indicator events reporting */
	HFP_AGC_CNUM = 				HFP_CODE_P | HFP_CODE_CNUM,							/* +CNUM: [<alpha>],<number>, <type>,[<speed> ,<service>]  */
	HFP_AGC_BINP = 				HFP_CODE_P | HFP_CODE_BINP,							/* +BINP: <dataresp1>...<datarespn>  */
	HFP_AGC_BVRA = 				HFP_CODE_P | HFP_CODE_BVRA,							/* +BVRA: <vrect> */
	HFP_AGC_BRSF = 				HFP_CODE_P | HFP_CODE_BRSF,							/* +BRSF: <AG supported features bitmap> */
	HFP_AGC_VGM = 				HFP_CODE_P | HFP_CODE_VGM,							/* +VGM:<gain>, +VGM=<gain> */
	HFP_AGC_VGS = 				HFP_CODE_P | HFP_CODE_VGS,							/* +VGS:<gain>, +VGS=<gain> */
	HFP_AGC_BSIR = 				HFP_CODE_P | HFP_CODE_BSIR,							/* +BSIR: <bsir> 0,1, 0 = the AG provides no in-band ring tone */
	HFP_AGC_BTRH = 				HFP_CODE_P | HFP_CODE_BTRH,							/* +BTRH: <n> */
	HFP_AGC_BCS = 				HFP_CODE_P | HFP_CODE_BCS,							/* +BCS: <u> (Bluetooth Codec Selection) */
};

/*---------------------------------------------------------------------------*/
/* 					 CME Error Code and Standard Error Code for APP	 	     */
/*---------------------------------------------------------------------------*/
/* This CME ERROR Code is only for APP Reference. More Code reference to GSM Spec. */
#define HFP_APPCMEERR_AGFAILURE											0
#define HFP_APPCMEERR_NOCONN2PHONE										1
#define HFP_APPCMEERR_OPERATION_NOTALLOWED								3
#define HFP_APPCMEERR_OPERATION_NOTSUPPORTED							4
#define HFP_APPCMEERR_PHSIMPIN_REQUIRED									5
#define HFP_APPCMEERR_SIMNOT_INSERTED									10
#define HFP_APPCMEERR_SIMPIN_REQUIRED									11
#define HFP_APPCMEERR_SIMPUK_REQUIRED									12
#define HFP_APPCMEERR_SIM_FAILURE										13
#define HFP_APPCMEERR_SIM_BUSY											14
#define HFP_APPCMEERR_INCORRECT_PASSWORD								16
#define HFP_APPCMEERR_SIMPIN2_REQUIRED									17
#define HFP_APPCMEERR_SIMPUK2_REQUIRED									18
#define HFP_APPCMEERR_MEMORY_FULL										20
#define HFP_APPCMEERR_INVALID_INDEX										21
#define HFP_APPCMEERR_MEMORY_FAILURE									23
#define HFP_APPCMEERR_TEXTSTRING_TOOLONG								24
#define HFP_APPCMEERR_INVALID_CHARACTERS_INTEXTSTRING 	    			25
#define HFP_APPCMEERR_DIAL_STRING_TOOLONG								26
#define HFP_APPCMEERR_INVALID_CHARACTERS_INDIALSTRING					27
#define HFP_APPCMEERR_NETWORK_NOSERVICE									30
#define HFP_APPCMEERR_NETWORK_TIMEOUT									31
#define HFP_APPCMEERR_NETWORK_NOTALLOWED_EMERGENCYCALL_ONLY				32

#define HFP_APPERR_TIMEOUT												200
#define HFP_APPERR_ERROR												201
#define HFP_APPERR_NOCARRIER											202
#define HFP_APPERR_BUSY													203
#define HFP_APPERR_NOANSWER												204
#define HFP_APPERR_DELAYED												205
#define HFP_APPERR_BLACKLISTED											206
#define HFP_APPERR_OK													255

/*---------------------------------------------------------------------------*/
/* 							 	AT Command /Response Events to APP	         */
/*---------------------------------------------------------------------------*/
enum HFP_APP_EventCodeEnum {/* HFP_SetState Callback to Application Event Code */
	/* SLC */
    HFP_EV_SLC_ESTABLISHED_IND = 1,					/* 1 HFP Service Level connection created */
	HFP_EV_SLC_RELEASED_IND,   						/* 2 HFP connection released */
	
	/* SCO */
	HFP_EV_AUDIO_CONN_ESTABLISHED_IND,				/* 3 SCO audio connection with HF is established */
	HFP_EV_AUDIO_CONN_RELEASED_IND,					/* 4 SCO audio connection with HF is released */

	/* Status Changed Indication */
	HFP_EV_STANDBY_IND,      						/* 5 Standby Menu, the incoming call or outgoing call or ongoing call is canceled */
	HFP_EV_ONGOINGCALL_IND,							/* 6 Ongoing Call Menu, a call (incoming call or outgoing call) is established (ongoing) */
	HFP_EV_INCOMINGCALL_IND, 						/* 7 Incoming Call(Ringing) Menu, a call is incoming, 2009.11.27 to replace HFP_EV_RING_IND */
	HFP_EV_OUTGOINGCALL_IND,						/* 8 Outgoing Call Menu, an outgoing call is being established, 3Way in Guideline P91 */
	HFP_EV_CALLHELD_IND,							/* 9 BTRH Hold Menu, +BTRH:0, AT+BTRH=0, incoming call is put on hold */
	HFP_EV_CALL_WAITING_IND,						/* 10 Call Waiting Menu, +CCWA, When Call=Active, call waiting notification */
	HFP_EV_TBUSY_IND,								/* 11 GSM Network Remote Busy, TBusy Timer Activated */

	/* AG & HF APP Gereral Event Indication */
	HFP_EV_GENERATE_INBAND_RINGTONE_IND,			/* 12 AG Only, Generate the in-band ring tone */
	HFP_EV_TERMINATE_LOCAL_RINGTONE_IND,			/* 13 AG & HF Terminate local generated ring tone or the in-band ring tone */
	HFP_EV_VOICE_RECOGN_ACTIVATED_IND,				/* 14 +BVRA:1, voice recognition activated indication or HF request to start voice recognition procedure */
	HFP_EV_VOICE_RECOGN_DEACTIVATED_IND,			/* 15 +BVRA:0, voice recognition deactivated indication or requests AG to deactivate the voice recognition procedure */
	HFP_EV_NETWORK_AVAILABLE_IND,					/* 16 +CIEV:<service><value>, cellular network is available */
	HFP_EV_NETWORK_UNAVAILABLE_IND,					/* 17 +CIEV:<service><value>, cellular network is unavailable */
	HFP_EV_ROAMING_RESET_IND,						/* 18 +CIEV:<roam><value>, roaming is not active */
	HFP_EV_ROAMING_ACTIVE_IND,						/* 19 +CIEV:<roam><value>, a roaming is active */	
	HFP_EV_SIGNAL_STRENGTH_IND,						/* 20 +CIEV:<signal><value>, signal strength indication */	
	HFP_EV_BATTERY_CHARGE_IND,						/* 21 +CIEV:<battchg><value>, battery charge indication */
	HFP_EV_CALLON_CHLDHOLD_ACTIVATED_IND,			/* 22 +CIEV:<callheld><1>, Call on CHLD Held to be or has been actived. */
	HFP_EV_CALLON_CHLDHOLD_RELEASED_IND,			/* 23 +CIEV:<callheld><0>, Call on CHLD Held to be or has been released. */	
	HFP_EV_MICVOL_CHANGED_IND,						/* 24 +VGM, AT+VGM, microphone volume changed indication */
	HFP_EV_SPKVOL_CHANGED_IND,						/* 25 +VGS, AT+VGS, speaker volume changed indication */		

	/* HF OK and Error Code, Indicate Command Cycle Finished */
	HFP_EV_ATCMD_RESULT_IND,						/* 26 HF Received OK, Error/+CME ERROR from AG or Wait for AG Response Timeout */
		
	/* HF APP, Call Related, AG Send information to HF */
	HFP_EV_CLIP_IND,								/* 27 +CLIP, Phone Number Indication */	
	HFP_EV_CURRENT_CALLS_IND,						/* 28 +CLCC, the current calls of AG */	
	HFP_EV_NETWORK_OPERATOR_IND,					/* 29 +COPS, the current network operator name of AG */
	HFP_EV_SUBSCRIBER_NUMBER_IND,					/* 30 +CNUM, the subscriber number of AG */
	HFP_EV_VOICETAG_PHONE_NUM_IND,					/* 31 +BINP, AG inputted phone number for voice-tag; requests AG to input a phone number for the voice-tag at the HF side */
	
	/* AG APP, HF Request AG to do the action, only use in AG side app */
	HFP_EV_CURRENT_CALLS_REQ,						/* 32 AT+CLCC, query the list of current calls in AG. */
	HFP_EV_NETWORK_OPERATOR_FORMAT_REQ,				/* 33 AT+COPS=3,0, indicate app the network operator name should be set to long alphanumeric */
	HFP_EV_NETWORK_OPERATOR_REQ,					/* 34 AT+COPS?, requests AG to respond with +COPS response indicating the currently selected operator */
	HFP_EV_SUBSCRIBER_NUMBER_REQ,					/* 35 AT+CNUM, query the AG subscriber number information. */
	HFP_EV_VOICETAG_PHONE_NUM_REQ,					/* 36 AT+BINP, requests AG to input a phone number for the voice-tag at the HF */
	HFP_EV_CUR_INDICATOR_VAL_REQ,					/* 37 AT+CIND?, get the current indicator during the service level connection initialization procedure */
	HFP_EV_HF_DIAL_REQ,								/* 38 ATD, instructs AG to dial the specific phone number */
	HFP_EV_HF_MEM_DIAL_REQ,							/* 39 ATD>, instructs AG to dial the phone number indexed by the specific memory location of SIM card */
	HFP_EV_HF_LASTNUM_REDIAL_REQ,					/* 40 AT+BLDN, instructs AG to redial the last dialed phone number */
	HFP_EV_MANUFACTURER_REQ,						/* 41 AT+CGMI, requests AG to respond with the Manufacturer ID */
	HFP_EV_MODEL_REQ,								/* 42 AT+CGMM, requests AG to respond with the Model ID */
	HFP_EV_NREC_DISABLE_REQ,						/* 43 AT+NREC=0, requests AG to disable NREC function */	
	HFP_EV_DTMF_REQ,								/* 44 AT+VTS, instructs AG to transmit the specific DTMF code */
	HFP_EV_ANSWER_CALL_REQ,							/* 45 Inform AG app to answer the call, param is call type, incoming or BTRH hold */	
	HFP_EV_CANCEL_CALL_REQ,							/* 46 Inform AG app to cancel the call, param is call type, outgoing or incoming or ongoing */	
	HFP_EV_HOLD_CALL_REQ,							/* 47 Inform AG app to hold the incoming call (AT+BTRH=0) */
	
	/* AG 3-Way Calling */
	HFP_EV_REJECTWAITINGCALL_REQ,					/* 48 AT+CHLD=0, Release all held calls or reject waiting call. */	
	HFP_EV_ACCEPTWAITING_RELEASEACTIVECALL_REQ,		/* 49 AT+CHLD=1, Accept the held or waiting call and release all avtive calls. */
	HFP_EV_HOLDACTIVECALL_REQ,						/* 50 AT+CHLD=2, Held Specified Active Call. */
	HFP_EV_ADDONE_HELDCALL2ACTIVECALL_REQ,			/* 51 AT+CHLD=3, Add One CHLD Held Call to Active Call. */
	HFP_EV_LEAVE3WAYCALLING_REQ,					/* 52 AT+CHLD=4, Leave The 3-Way Calling. */
	
	/* Extended */
	HFP_EV_EXTEND_CMD_IND,							/* 53 Indicate app extend command received */
	HFP_EV_UPPER_DECIDE_ESTABLISH_SCO_IND,			/* 54 Ask app whether to establish SCO */
	HFP_EV_SPP_ESTABLISHED_IND,						/* 55 SPP connection created. added 2008-7-3 */
	HFP_EV_HF_MANUFACTURERID_IND,					/* 56 ManufacturerID, a null-terminated ASCII string. */
	HFP_EV_HF_MODELID_IND,							/* 57 ModelID, a null-terminated ASCII string. */

	HFP_EV_RINGING_IND,								/* 58 +RING received, play ring tone if possible, otherwise ignored. The new HFP_EV_INCOMINGCALL_IND is used to trigger Incoming Call Menu instead */
	HFP_EV_TRANSPARENT_SUBMITCIEV_IND, 				/* 59 +CIEV (call, callsetup, or callheld indicator) received, UINT8* - indicator value(10,11; 20,21,22,23; 30,31,32) */
	HFP_EV_SERVER_START_CFM,						/* 60 Server start confirmation with Server Channel applied */
	HFP_EV_GET_AGFEATURES_COMPLETE,					/* 61 Get AG features complete */
	HFP_EV_CHECKSCO_COMPLETE,						/* 62 Check SCO complete */
	HFP_EV_CONNECT_CFM,								/* 63 RFCOMM connect confirmation with instance handle, struct HFP_ConnectEvStru */					
	HFP_EV_CONNECT_IND,								/* 64 RFCOMM Connection indication */
	HFP_EV_DISCONNECT_COMPLETE,						/* 65 HFP connection released indication with reason, used to replace HFP_EV_SLC_RELEASED_IND */
	HFP_EV_THREEWAY_ERROR_IND, 						/* 66 CHLD does not succeed */
	HFP_EV_BTRH_QUERY_REQ							/* 67 HF query the status of the "Response and Hold" state of the AG (AT+BTRH?). UINT8* to return the current state: 0 - No incoming call on hold; 1 - The incoming call is on hold; Other values to respond later with HFP_AG_SetCurIndicatorVal */
};

/*---------------------------------------------------------------------------*/
/* 							 		AG Network Events/Mask					 */
/*---------------------------------------------------------------------------*/
/* AG Action Reason */
#define HFP_HF_DO									0x10	/* HF send AG a command to deal */
#define HFP_AG_DO									0x20	/* Generally AG GUI lead to */
#define HFP_AG_GSM_CANCELED_ALLCALL					0x01	/* GSM Service Unavailable */
#define HFP_AG_GSM_CANCELED_CALLSETUP				0x02	/* GSM Release the Incoming Call or Outgoing Call */
#define HFP_AG_GSM_CANCELED_LASTCALL				0x03	/* GSM Release Last Call in Call=1 */
#define HFP_AG_GSM_CANCELED_CALLHELD				0x04	/* GSM Release only held call */

/* AG answer call mode */
#define HFP_AG_PRIVATE_MODE							0x05	/* Answer the Outgoing/Incoming Call on AG */
#define HFP_AG_HANDSFREE_MODE						0x06	/* Answer the Outgoing/Incoming Call on HF */

/* AG type of the call, used in HFP_EV_ANSWER_CALL_REQ, HFP_EV_CANCEL_CALL_REQ param */
#define HFP_TYPE_ALL_CALLS							0x01
#define HFP_TYPE_INCOMING_CALL						0x02
#define HFP_TYPE_HELDINCOMING_CALL					0x03
#define HFP_TYPE_OUTGOING_CALL						0x04
#define HFP_TYPE_ONGOING_CALL						0x05

/* Possible received events from GSM/CDMA cellular network */
#define HFP_NETWORK_RMT_IS_BUSY						0x01
#define HFP_NETWORK_ALERTING_RMT					0x02
#define HFP_NETWORK_INCOMING_CALL					0x03
#define HFP_NETWORK_RMT_ANSWER_CALL					0x04
#define HFP_NETWORK_SVC_UNAVAILABLE					0x05
#define HFP_NETWORK_SVC_AVAILABLE					0x06
#define HFP_NETWORK_SIGNAL_STRENGTH					0x07
#define HFP_NETWORK_ROAMING_RESET					0x08
#define HFP_NETWORK_ROAMING_ACTIVE					0x09

/*---------------------------------------------------------------------------*/
/* 							 		+CLCC Status/Mode/Multiparty   			 */
/*---------------------------------------------------------------------------*/
#define HFP_CLCC_DIR_OUTGOING						0
#define HFP_CLCC_DIR_INCOMING						1
#define HFP_CLCC_STATUS_ACTIVE						0
#define HFP_CLCC_STATUS_HELD						1
#define HFP_CLCC_STATUS_DIALLING					2 	/* outgoing calls only */
#define HFP_CLCC_STATUS_ALERTING					3 	/* outgoing calls only */
#define HFP_CLCC_STATUS_INCOMING					4 	/* incoming calls only */
#define HFP_CLCC_STATUS_WAITING						5 	/* incoming calls only */
#define HFP_CLCC_MODE_VOICE							0
#define HFP_CLCC_MODE_DATA							1
#define HFP_CLCC_MODE_FAX							2
#define HFP_CLCC_MPTY_NOTMULTIPARTY					0
#define HFP_CLCC_MPTY_MULTIPARTY					1

/*---------------------------------------------------------------------------*/
/* 		Possible values of the result member of HFP_PreSCOConnIndStru        */
/*---------------------------------------------------------------------------*/
#define HFP_CONN_SCO_DEFAULT			0x00 /* Application won't create the SCO connection */
#define HFP_CONN_SCO_PENDING			0x01 /* Application will create the SCO connection later, usually in anther thread */
#define HFP_CONN_SCO_COMP				0x02 /* Application creates the SCO connection successfully, reserved. */

#define HFP_CHLD_WITHOUT_IDXPARAM		0x00 /* For CHLD=(0,1,2,3,4) except(1<idx>,2<idx>) */
#define HFP_PHONENUM_MAX_DIGITS			32

/*---------------------------------------------------------------------------*/
/* 								HFP APP Interface Structures		         */
/*---------------------------------------------------------------------------*/
struct HFP_PreSCOConnIndStru {
	UINT16  pkt_type;  	/* [in] Packet type of the new SCO connection. */
	UINT8   blocked;    /* Reserved for compatibility, set to 0. */
	UINT8   result;   	/* [out] SCO created by the application or not. Possible values:
                       HFP_CONN_SCO_DEFAULT, HFP_CONN_SCO_PENDING. */
};

struct HFP_RegServerStru {
	UINT8  server_channel;  /* Server channel selected for the server */
	UINT8  role;		    /* Use mask to inditify HFP_ROLE_SERVER, HFP_ROLE_HIGH_VERSION, HFP_ROLE_HF_SIDE */
};

struct HFP_SLCConnectInfoStru {
    void    *server_handle;     /* Handle to the local server instance bonded with this connection */
	UINT32	remote_features;    /* BRSF & CIND Map. Set of flags e.g. HFP_BRSF_HF_NREC; HFP_BRSF_AG_NREC, HFP_FEA_AG_SERVICE */
	UINT8 	role;				/* Use mask to inditify HFP_ROLE_SERVER, HFP_ROLE_HIGH_VERSION, HFP_ROLE_HF_SIDE */
	UINT8 	bd[BD_ADDR_LEN];
	UINT8 	select_codec;
};

/* Used By +COPS */
struct HFP_COPSInfoStru {
	UINT8 	mode;				/* Current mode and provides no information with regard to the name of the operator */
	UINT8 	format;				/* The format of the operator parameter string */
	UINT8	operator_len;
	CHAR 	operator_name[1];	/* The string in alphanumeric format representing the name of the network operator */	
};

/* Used By +BINP, +CNUM, +CLIP, +CCWA */
struct HFP_PhoneInfoStru {
	UINT8 	type;				/* The format of the phone number provided */
	UINT8 	service;			/* Indicates which service this phone number relates to. Shall be either 4 (voice) or 5 (fax). */
	UINT8 	num_len;			/* the length of the phone number provided, not including ending 0 */	
	UINT8 	number[HFP_PHONENUM_MAX_DIGITS];	/* Ended by 0, subscriber number */	
	UINT8 	name_len;			/* length of alpha_str, not including ending 0 */
	UINT8 	alpha_str[1];		/* Ended by 0, String type subaddress of format specified by <cli_validity> */	
};

/* Used By +CLCC */
struct HFP_CLCCInfoStru {
	UINT8 	idx;				/* The numbering (start with 1) of the call given by the sequence of setting up or receiving the calls */
	UINT8 	dir;				/* Direction, 0=outgoing, 1=incoming */
	UINT8 	status;				/* 0=active, 1=held, 2=dialling(outgoing), 3=alerting(outgoing), 4=incoming(incoming), 5=waiting(incoming) */
	UINT8 	mode;				/* 0=voice, 1=data, 2=fax */
	UINT8 	mpty;				/* 0=not multiparty, 1=multiparty */
	UINT8	type;				/* The format of the phone number provided */
	UINT8	num_len;			/* the length of the phone number provided, not including ending 0 */	
	UINT8	number[1];			/* Endby 0, Phone number */
};

/* current state mask code for function HFP_AG_SetCurIndicatorVal */
struct HFP_CINDInfoStru {
	UINT8	service;			/* 0=unavailable, 1=available */
	UINT8	call;				/* 0=no active call, 1=have active call */
	UINT8	callsetup;			/* 0=no callsetup, 1=incoming, 2=outgoing, 3=outalert */
	UINT8 	callheld;			/* 0=no callheld, 1=active-hold, 2=onhold */
	UINT8 	signal;				/* 0~5 signal strength */
	UINT8	roam;				/* 0=no roam, 1= roam */
	UINT8	battchg;			/* 0~5 */
};

struct HFP_ATCmdResultStru {
	UINT32	cmd_code;			/* Which AT command code got a error */
	UINT32	result_code;		/* OK, CME Error Code or ERROR or TIMEOUT */
};

struct HFP_ConnectEvStru {
	void *handle;				/* Pointer to connection mem */
	UINT8 bd[BD_ADDR_LEN];
	UINT8 role; 				/* etc: HFP_ROLE_HF_CLIENT */
	UINT8 result;				/* etc: ER_HFP_RFCOMM_SUCCESS */
};

/*---------------------------------------------------------------------------*/
/* 				HFP APP Connect Establish Interface Functions			     */
/*---------------------------------------------------------------------------*/
FUNC_EXPORT void HFP_Init(void);
FUNC_EXPORT void HFP_Done(void);
FUNC_EXPORT void HFP_Server_Start(UINT32 features, UINT8 svr_chnl, UINT32 *svc_hdl, UINT8 role, UINT8 max_connection_number, HFP_Callback pfunc);
FUNC_EXPORT void HFP_Server_Stop(void *server_hdl);
FUNC_EXPORT void HFP_Client_Start(void *server_hdl, UINT8 *bd, UINT8 *sdp_attrib, UINT8 role, HFP_Callback pfunc);
FUNC_EXPORT void HFP_Client_Stop(UINT8 *hdl);

#ifdef CONFIG_HFP_HF
/*---------------------------------------------------------------------------*/
/* 								HFP HF APP Interface Functions			     */
/*---------------------------------------------------------------------------*/
FUNC_EXPORT void HFP_HF_GetAGFeatures(UINT8 *hdl);
FUNC_EXPORT void HFP_HF_Dial(UINT8 *hdl, UINT8 *phone_num);
FUNC_EXPORT void HFP_HF_MemNumDial(UINT8 *hdl, UINT8 *mem_location);
FUNC_EXPORT void HFP_HF_LastNumRedial(UINT8 *hdl);
FUNC_EXPORT void HFP_HF_SetSpkVol(UINT8 *hdl, UINT8 spk_vol);
FUNC_EXPORT void HFP_HF_SetMicVol(UINT8 *hdl, UINT8 mic_vol);
FUNC_EXPORT void HFP_HF_TxDTMF(UINT8 *hdl, UINT8 chr);
FUNC_EXPORT void HFP_HF_AudioConnTrans(UINT8 *hdl);
FUNC_EXPORT void HFP_HF_CodecConnSetup(UINT8 *hdl, UINT8 codec_type);
FUNC_EXPORT void HFP_HF_VoiceTagPhoneNumReq(UINT8 *hdl);
FUNC_EXPORT void HFP_HF_AnswerCall(UINT8 *hdl);
FUNC_EXPORT void HFP_HF_CancelCall(UINT8 *hdl);
FUNC_EXPORT void HFP_HF_VoiceRecognitionReq(UINT8 *hdl, UINT8 param);
FUNC_EXPORT void HFP_HF_DisableNREC(UINT8 *hdl);
FUNC_EXPORT void HFP_HF_SetIndicatorActivation(UINT8 *hdl, struct HFP_CINDInfoStru *mask);
FUNC_EXPORT void HFP_HF_3WayCallingHandler(UINT8 *hdl, UINT8 op_code/* 0,1,2,3,4 */, UINT8 idx);
FUNC_EXPORT void HFP_HF_GetPhoneManuModel(UINT8 *hdl);
/* HFP 1.5 */
FUNC_EXPORT void HFP_HF_NetworkOperatorReq(UINT8 *hdl);
FUNC_EXPORT void HFP_HF_SetExtendedErrors(UINT8 *hdl, UINT8 param);
FUNC_EXPORT void HFP_HF_GetResponseHoldStatus(UINT8 *hdl);
FUNC_EXPORT void HFP_HF_HoldIncomingCall(UINT8 *hdl);
FUNC_EXPORT void HFP_HF_AcceptHeldIncomingCall(UINT8 *hdl);
FUNC_EXPORT void HFP_HF_RejectHeldIncomingCall(UINT8 *hdl);
FUNC_EXPORT void HFP_HF_GetSubscriberNumber(UINT8 *hdl);
FUNC_EXPORT void HFP_HF_GetCurrentCalls(UINT8 *hdl);

#ifdef CONFIG_PATCH_HUNTERSUN
struct HFP_ConnectParamStru {
	UINT8  enable;
	UINT8  retranseffort;
	UINT16 latency;
};
FUNC_EXPORT void HFP_HF_SetDefaultLatencyAndRetrans(struct HFP_ConnectParamStru param);
#endif

#endif

/* Extend Command Send Interface for AG/HF */
FUNC_EXPORT void HFP_ExtendCmd(UINT8 *hdl, void *cmd, UINT16 len, UINT32 timeout);
FUNC_EXPORT void HFP_CheckSCO(UINT8 *hdl);

#endif

