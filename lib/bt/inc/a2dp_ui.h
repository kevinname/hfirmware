/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Module Name:
	a2dp_ui.h
Abstract:
	This file provides A2DP APP user interface Functions.
-----------------------------------------------------------------------------*/
#ifndef _A2DP_UI_H
#define _A2DP_UI_H

#include "global.h"
#include "avdtp_ui.h"

/*---------------------------------------------------------------------------*/
/* 									Misc						     		 */
/*---------------------------------------------------------------------------*/
/* SEP Type */
#define A2DP_TSEP_SRC				0x00 
#define A2DP_TSEP_SNK				0x01

#define A2DP_CODECCAPS_LEN			0x10
#define A2DP_PAYLOAD_HEADER			0x0F

/* Codec Type */
#define A2DP_CODEC_SBC				0x00/* now only support sbc codec. 2011.11.2 */
#define A2DP_CODEC_MPEG12 			0x01
#define A2DP_CODEC_AAC	 			0x02
#define A2DP_CODEC_ATRAC	 		0x04
#define A2DP_CODEC_NONA2DP	 		0xFF

enum {/* Stream object mask */
	A2DP_STREAM_MASK_INITIATOR = 	0x01,
	A2DP_STREAM_MASK_SRC = 			0x02/* Source, otherwise is sink */
};

enum {/* SRCENTRY object mask */
	A2DP_STREAM_STATUS_OPEN = 1,
	A2DP_STREAM_STATUS_STREAMING = 2
};

enum {/* BT-SIG assign number https://www.bluetooth.org/Technical/AssignedNumbers/host_operating.htm */
	A2DP_MEDIATYPE_AUDIO = 			0x00,
	A2DP_MEDIATYPE_VIDEO = 			0x01,
	A2DP_MEDIATYPE_MULTIMEDIA = 	0x02
};

/*---------------------------------------------------------------------------*/
/* 								A2DP Structure							     */
/*---------------------------------------------------------------------------*/
#define A2DP_RegCbkStru 			AVDTP_RegCbkStru
#define A2DP_SEPStru 				AVDTP_SEPStru
#define A2DP_StreamCreateStru 		AVDTP_StreamCreateStru
#define A2DP_SEIDStru 				AVDTP_SEIDStru
#define A2DP_ConnectReqStru         AVDTP_ConnectReqStru
#define A2DP_SetConfigStru          AVDTP_SetConfigStru
#define A2DP_ServiceCapStru 		AVDTP_ServiceCapStru
#define A2DP_OpenedStru       		AVDTP_ReadyToStartStru
#define A2DP_StartSuspendIndStru    AVDTP_StartSuspendIndStru
#define A2DP_Stream_DoneStru        AVDTP_Stream_DoneStru

struct A2DP_OpenCodecInfoIndStru {
	HANDLE stream_handle;
	UINT16 pcm_framesize;
	UINT16 sbc_framesize;
	UINT8  min_numframe;
	UINT8  is_snk;
#ifdef CONFIG_PATCH_HUNTERSUN
        UINT8  info[3];
#endif
};

struct A2DP_Stream_DataIndStru {
	HANDLE stream_handle;
	struct BuffStru *data;
	UINT16 UsedDataLen;
};

enum {/* Service Category */
	A2DP_SC_MEDIA_TRANSPORT = 1,				/* 0000,0001 */
	A2DP_SC_REPORTING,							/* 0000,0010 */
	A2DP_SC_RECOVERY,							/* 0000,0011 */
	A2DP_SC_CONTENT_PROTECTION,					/* 0000,0100 */
	A2DP_SC_HEADER_COMPRESSION,					/* 0000,0101 */
	A2DP_SC_MULTIPLEXING,						/* 0000,0110 */
	A2DP_SC_MEDIA_CODEC,						/* 0000,0111 */
	A2DP_SC_DELAY_REPORTING						/* 0000,1000 */
};

/*-----------------------------------------------------------------------------
Description:
	A2DP SBC codec specific information.
Format:
	7			6			5			4			3			2			1			0
	Sampling Frequency(4bits)						Channel Mode(4bits)
	Block Length(4bits)								Subbands(2bits)			Allocation Method(2bits)
	Min Bitpool(1octet)
	Max Bitpool(1octet)
-----------------------------------------------------------------------------*/
enum {/* Octet1 */
	A2DP_SBC_SF_16000 = 						0x80,/* sampling_freq, do not need to shift again, already in macro */
	A2DP_SBC_SF_32000 = 						0x40,
	A2DP_SBC_SF_44100 = 						0x20,/* SRC must support 44.1 or 48, SNK must support both */
	A2DP_SBC_SF_48000 = 						0x10,
	A2DP_SBC_SF_ALL = 							0xF0,
	A2DP_SBC_CHMODE_MONO = 						0x08,/* channel_mode */
	A2DP_SBC_CHMODE_DUAL_CHANNEL = 				0x04,
	A2DP_SBC_CHMODE_STEREO = 					0x02,
	A2DP_SBC_CHMODE_JOINT_STEREO = 				0x01,
	A2DP_SBC_CHMODE_ALL = 						0x0F
};

enum {/* Octet2 */
	A2DP_SBC_BLOCK_4 = 							0x80,/* block_len, do not need to shift */
	A2DP_SBC_BLOCK_8 = 							0x40,
	A2DP_SBC_BLOCK_12 = 						0x20,
	A2DP_SBC_BLOCK_16 = 						0x10,
	A2DP_SBC_BLOCK_ALL = 						0xF0,
	A2DP_SBC_SUBBAND_4 = 						0x08,/* subbands, do not need to shift */
	A2DP_SBC_SUBBAND_8 = 						0x04,
	A2DP_SBC_SUBBAND_ALL = 						0x0C,
	A2DP_SBC_SNR = 								0x02,/* allocation_method, do not need to shift */
	A2DP_SBC_LOUDNESS = 						0x01,/* Default, most headphone only support Loudness */
	A2DP_SBC_ALLOCATION_ALL = 					0x03
};

/*-----------------------------------------------------------------------------
Description:
	A2DP MP3 codec specific information.
Format:
	7			6			5			4			3			2			1			0
	Layer(3bits)						CRC(1bit)	Channel Mode(4bits)
	RFA(1bit)	MPF(1bit)	Sampling Frequency(6bits)
	VBR(1bit)	Bit Rate(7bit...)
	Bit Rate(...1octet)
-----------------------------------------------------------------------------*/
enum {/* Octet1 */
	A2DP_MP3_LAYER1 = 							0x80,/* Layer */
	A2DP_MP3_LAYER2 = 							0x40,
	A2DP_MP3_LAYER3 = 							0x20,
	A2DP_MP3_CRC_PROTECTION = 					0x10,/* CRC Protection */
	A2DP_MP3_CHMODE_MONO = 						0x08,/* Channel mode */
	A2DP_MP3_CHMODE_DUAL_CHANNEL = 				0x04,
	A2DP_MP3_CHMODE_STEREO = 					0x02,
	A2DP_MP3_CHMODE_JOINT_STEREO = 				0x01
};

enum {/* Octet2 */
	A2DP_MP3_MPF_SUPPORT_MPF2 = 				0x40,/* MPF2 support is 1, otherwise is 0 */
	A2DP_MP3_SF_16000 = 						0x20,
	A2DP_MP3_SF_22050 = 						0x10,
	A2DP_MP3_SF_24000 = 						0x08,
	A2DP_MP3_SF_32000 = 						0x04,
	A2DP_MP3_SF_44100 = 						0x02,
	A2DP_MP3_SF_48000 = 						0x01
};

enum {/* Octet3 */
	A2DP_MP3_VBR_SUPPORT = 						0x80,
	A2DP_MP3_BITRATE_1110 = 					0x40,
	A2DP_MP3_BITRATE_1101 = 					0x20,
	A2DP_MP3_BITRATE_1100 = 					0x10,
	A2DP_MP3_BITRATE_1011 = 					0x08,
	A2DP_MP3_BITRATE_1010 = 					0x04,
	A2DP_MP3_BITRATE_1001 = 					0x02,
	A2DP_MP3_BITRATE_1000 = 					0x01
};

enum {/* Octet4 */
	A2DP_MP3_BITRATE_0111 = 					0x80,
	A2DP_MP3_BITRATE_0110 = 					0x40,
	A2DP_MP3_BITRATE_0101 = 					0x20,
	A2DP_MP3_BITRATE_0100 = 					0x10,
	A2DP_MP3_BITRATE_0011 = 					0x08,
	A2DP_MP3_BITRATE_0010 = 					0x04,
	A2DP_MP3_BITRATE_0001 = 					0x02,
	A2DP_MP3_BITRATE_0000 = 					0x01
};

/*---------------------------------------------------------------------------*/
/* 					Event for Upper Application					             */
/*---------------------------------------------------------------------------*/
enum {/* Callback event in A2DP_EventCBK */
	A2DP_EV_REGSEP,
	A2DP_EV_SEPDISCOVERY,
	A2DP_EV_GETCAPABILITIES,
	A2DP_EV_SETCONFIGURATION,
	A2DP_EV_GETCONFIGURATION,
	A2DP_EV_STREAMOPEN,
	A2DP_EV_STREAMSTART,
	A2DP_EV_STREAMSUSPEND,
	A2DP_EV_STREAMCLOSE,
	A2DP_EV_STREAMABORT,
	A2DP_EV_RECONFIGURE,
	A2DP_EV_DATAIND,
	A2DP_EV_STREAMDONE,
	/* new add event */
	A2DP_EV_STREAMCREATE,
	A2DP_EV_OPENCODECINFO/*  */
};

enum {/* A2DP spec defined error code */
	A2DP_ERROR_INVALID_CODEC_TYPE = 					0xC1,
	A2DP_ERROR_NOT_SUPPORTED_CODEC_TYPE = 				0xC2,
	A2DP_ERROR_INVALID_SAMPLING_FREQUENCY = 			0xC3,
	A2DP_ERROR_NOT_SUPPORTED_SAMPLING_FREQUENCY = 		0xC4,
	A2DP_ERROR_INVALID_CHANNEL_MODE = 					0xC5,
	A2DP_ERROR_NOT_SUPPORTED_CHANNEL_MODE = 			0xC6,

	/* SBC */
	A2DP_ERROR_INVALID_SUBBANDS = 						0xC7,/* None or multiple values have been selected for Number of Subbands */
	A2DP_ERROR_NOT_SUPPORTED_SUBBANDS = 				0xC8,
	A2DP_ERROR_INVALID_ALLOCATION_METHOD = 				0xC9,
	A2DP_ERROR_NOT_SUPPORTED_ALLOCATION_METHOD = 		0xCA,
	A2DP_ERROR_INVALID_MINIMUM_BITPOOL_VALUE = 			0xCB,
	A2DP_ERROR_NOT_SUPPORTED_MINIMUM_BITPOOL_VALUE = 	0xCC,
	A2DP_ERROR_INVALID_MAXIMUM_BITPOOL_VALUE = 			0xCD,
	A2DP_ERROR_NOT_SUPPORTED_MAXIMUM_BITPOOL_VALUE = 	0xCE,

	/* MP3 */
	A2DP_ERROR_INVALID_LAYER = 							0xCF,
	A2DP_ERROR_NOT_SUPPORTED_LAYER = 					0xD0,
	A2DP_ERROR_NOT_SUPPORTED_CRC = 						0xD1,
	A2DP_ERROR_NOT_SUPPORTED_MPF = 						0xD2,
	A2DP_ERROR_NOT_SUPPORTED_VBR = 						0xD3,
	A2DP_ERROR_INVALID_BIT_RATE = 						0xD4,
	A2DP_ERROR_NOT_SUPPORTED_BIT_RATE = 				0xD5,
	A2DP_ERROR_INVALID_OBJECT_TYPE = 					0xD6,
	A2DP_ERROR_NOT_SUPPORTED_OBJECT_TYPE = 				0xD7,
	A2DP_ERROR_INVALID_CHANNELS = 						0xD8,
	A2DP_ERROR_NOT_SUPPORTED_CHANNELS = 				0xD9,

	/* ATRAC */
	A2DP_ERROR_INVALID_VERSION = 						0xDA,
	A2DP_ERROR_NOT_SUPPORTED_VERSION = 					0xDB,
	A2DP_ERROR_NOT_SUPPORTED_MAXIMUM_SUL = 				0xDC,
	A2DP_ERROR_INVALID_BLOCK_LENGTH = 					0xDD,/* 0xDE-0xDF RFD */
	
	A2DP_ERROR_INVALID_CP_TYPE = 						0xE0,
	A2DP_ERROR_INVALID_CP_FORMAT = 						0xE1/* 0xE2-0xFF RFD */	
};

/*---------------------------------------------------------------------------*/
/* 							Supported Features, For SDAP					 */
/*---------------------------------------------------------------------------*/
#define A2DP_PLAYER		    		0x0001
#define A2DP_MICROPHONE				0x0002
#define A2DP_TUNER		        	0x0004
#define A2DP_MIXER			       	0x0008
#define A2DP_HEADPHONE		       	0x0001
#define A2DP_SPEAKER		       	0x0002
#define A2DP_RECORDER		        0x0004
#define A2DP_AMPLIFIER		        0x0008

/*---------------------------------------------------------------------------*/
/* 							Init/Done, Service, CallBack					 */
/*---------------------------------------------------------------------------*/
FUNC_EXPORT void A2DP_Init(void);
FUNC_EXPORT void A2DP_Done(void);
FUNC_EXPORT UINT32 A2DP_RegisterService(UINT8 svc_type, UINT16 features);
FUNC_EXPORT void A2DP_UnregisterService(UINT32 svr_hdl);
FUNC_EXPORT void A2DP_RegCbk(struct A2DP_RegCbkStru *in);
/*---------------------------------------------------------------------------*/
/* 							SEP Maintaining Procedures						 */
/*---------------------------------------------------------------------------*/
FUNC_EXPORT void A2DP_Register_SEP(struct A2DP_SEPStru *in); 
FUNC_EXPORT void A2DP_UnRegister_SEP(UINT8 local_seid);
FUNC_EXPORT void A2DP_Connect(struct A2DP_ConnectReqStru *in);
FUNC_EXPORT void A2DP_Disconnect(void *stream_handle);
FUNC_EXPORT void A2DP_GetConfiguration(void *stream_handle);
FUNC_EXPORT void A2DP_StreamStart(void *stream_handle);
FUNC_EXPORT void A2DP_StreamSuspend(void *stream_handle);
FUNC_EXPORT void A2DP_StreamReconfigure(struct A2DP_SetConfigStru *in);

#endif

