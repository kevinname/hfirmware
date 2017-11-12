/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Module Name:
	Sbc.h
Abstract:
	This file provides the basic functions of SBC Audio Codec.
-----------------------------------------------------------------------------*/

#ifndef _SBC_H_
#define _SBC_H_

#include "global.h"

/*---------------------------------------------------------------------------*/
/* 								SBC Codec MACROS 		 					 */
/*---------------------------------------------------------------------------*/
#define MAX_CH								2
#define MAX_SF								4
#define MAX_SB								8
#define MAX_BLK 							16

#define SBC_SF_16							0
#define SBC_SF_32							1
#define SBC_SF_44							2
#define SBC_SF_48							3

#define SBC_BLOCKS_4						0
#define SBC_BLOCKS_8						1
#define SBC_BLOCKS_12						2
#define SBC_BLOCKS_16						3
#define SBC_BLOCKS_15						4/* MSBC for HFP */

#define SBC_BLOCK_NUM(x)					(((x) + 1) << 2)

/* channel mode */
#define SBC_CHANNEL_MODE_MONO				0
#define SBC_CHANNEL_MODE_DUAL				1
#define SBC_CHANNEL_MODE_STEREO				2
#define SBC_CHANNEL_MODE_JOINT_STEREO		3
#define SBC_CHANNEL_NUM(x)					((x) == 0 ? 1 : 2)

/* allocation method */
#define SBC_AM_LOUDNESS						0
#define SBC_AM_SNR							1

/* subbands */
#define SBC_SUBBANDS_4						0
#define SBC_SUBBANDS_8						1
#define SBC_SUBBAND_NUM(x)					((x) == 0 ? 4 : 8)
#define SBC_SUBBANDS(y)						((y) == 4 ? 0 : 1)
#define SBC_MAXFRMLEN						532

#define SBC_STREAMSIZEF_DESTINATION			0
#define SBC_STREAMSIZEF_SOURCE				1
#define SBC_HEADER_LEN						4

/*---------------------------------------------------------------------------*/
/* 							SBC Codec Optmized Porting 	 					 */
/*---------------------------------------------------------------------------*/
#ifdef CONFIG_OS_WIN32
#define SIZEOF_INT 		4
#define SBC_ASM 		1
#endif

#ifdef CONFIG_PATCH_HUNTERSUN
#if defined(CONFIG_OS_UC) || defined(_CHIBIOS_RT_)
#define SIZEOF_INT 		4
//#define SBC_ASM 		1
#endif
#endif

#if SIZEOF_INT >= 4
typedef signed int 		sbc_fixed;
typedef unsigned int 	sbc_fixed_uint;
#else
typedef signed long 	sbc_fixed;
typedef unsigned long 	sbc_fixed_uint;
#endif

typedef unsigned long 	sbc_cache32;
//typedef signed __int64 	sbc_fixed64;
typedef signed long long int 	sbc_fixed64;

typedef signed int 		sbc_fixed64hi;
typedef unsigned int 	sbc_fixed64lo;

#define FRACBITS				30						/* 28, 29 Recommand, 30 For X86 High Accuracy For DCT&Polyanalysis */
#define TOSHORT_X				14						/* 12~14, Must Below (32-16PCM), Increase This will Enhance Polyalalysis`s Accuracy, Modify Note That Scale_factor 4 Bits */
#define ASFRACBITS_X			14						/* 0~TOSHORT_X, Must Below (32-16SF), Increase This Will Enhance Subband Sample`s Accuracy */
#define DECODER_X				15						/* Post_Reverse Reconstruction To Save Accuracy For Syn */

//#define TOSHORT(x)			((x) >> (TOSHORT_X - ASFRACBITS_X))		/* Analysis Last Step */
#define TOSHORT(x)				(x)
#define ENCODER_FLOAT2FIX(x)	((sbc_fixed)(sbc_fixed64)((x) * (double) (1L << FRACBITS) + 0.5))

#define FIXED_COS_PI4			ENCODER_FLOAT2FIX(7.071067812E-001) 	/* cos(PI4)			= 7.07106781E-01 */
#define FIXED_COS_PI8			ENCODER_FLOAT2FIX(9.238795325E-001) 	/* cos(PI8)			= 9.23879533E-01 */
#define FIXED_COS_3PI8			ENCODER_FLOAT2FIX(3.826834324E-001) 	/* cos(3*PI8)	 	= 3.82683432E-01 */
#define FIXED_COS_PI8_3PI8_1	ENCODER_FLOAT2FIX(5.411961001E-001)		/* FIXED_COS_PI8 - FIXED_COS_3PI8 */
#define FIXED_COS_PI8_3PI8_2	ENCODER_FLOAT2FIX(-3.266407412E-001) 	/* (-FIXED_COS_PI8 - FIXED_COS_3PI8)/4 */
#define FIXED_I2COS_PI16		ENCODER_FLOAT2FIX(5.097955791E-001) 	/* 1/2cos(PI16)	 	= 5.09795579E-01 */
#define FIXED_I2COS_3PI16		ENCODER_FLOAT2FIX(6.013448869E-001) 	/* 1/2cos(3*PI16) 	= 6.01344887E-01 */
#define FIXED_I2COS_5PI16		ENCODER_FLOAT2FIX(8.999762231E-001) 	/* 1/2cos(5*PI16) 	= 8.99976223E-01 */
#define FIXED_I2COS_7PI16_DIV4 	ENCODER_FLOAT2FIX(6.407288619E-001)  	/* (1/2cos(7*PI16))/4 	= 6.40728862E-01 */

#define FIXED_COS_PI16			ENCODER_FLOAT2FIX(9.807852804E-001) 	/* cos(PI16)	= 0.98078528040323 */
#define FIXED_COS_3PI16			ENCODER_FLOAT2FIX(8.314696123E-001) 	/* cos(3*PI16)	= 0.83146961230255 */
#define FIXED_COS_5PI16			ENCODER_FLOAT2FIX(5.555702330E-001) 	/* cos(5*PI16)	= 0.55557023301960 */
#define FIXED_COS_7PI16			ENCODER_FLOAT2FIX(1.950903220E-001) 	/* cos(7*PI16)	= 0.19509032201613 */

#ifdef SBC_ASM
/* --- Intel --------------------------------------------------------------- */
#ifdef CONFIG_OS_WIN32
#pragma warning(push)
#pragma warning(disable: 4035)  /* no return value */
static __forceinline sbc_fixed SBC_MUL_DCT1(sbc_fixed x, sbc_fixed y)
{
	__asm {
		mov eax, x
		imul y
		shrd eax, edx, FRACBITS
	}
}

static __forceinline sbc_fixed SBC_MUL_DCT2(sbc_fixed x, sbc_fixed y)
{
	__asm {
		mov eax, x
		imul y
		shrd eax, edx, (FRACBITS - 2)
	}
}

static __forceinline sbc_fixed SBC_MUL_Q(sbc_fixed x1, sbc_fixed x2, sbc_fixed y)
{
	__asm {
		mov eax, x1
		add eax, x2
		imul y
		shrd eax, edx, (15 + 2 + ASFRACBITS_X)
	}
}

#pragma warning(pop)
#endif

/* --- ARM ----------------------------------------------------------------- */
#ifdef CONFIG_OS_WINCE
extern sbc_fixed mul30(sbc_fixed x, sbc_fixed y); 
extern sbc_fixed mul28(sbc_fixed x, sbc_fixed y);
extern sbc_fixed mul31Q(sbc_fixed x1, sbc_fixed x2, sbc_fixed y); 

#define SBC_MUL_DCT1  			mul30
#define SBC_MUL_DCT2  			mul28
#define SBC_MUL_Q  				mul31Q
#endif

#if defined(CONFIG_PATCH_HUNTERSUN) && defined(SBC_ASM)
#if defined(__GNUC__) //&& defined(__ARM__)
static __inline sbc_fixed SBC_MUL_DCT1(sbc_fixed x, sbc_fixed y)
{
    sbc_fixed 		result;

    __asm__ volatile("smull %0,%1,%3,%2; movs %0,%0,lsr#30; orr %0,%0,%1,lsl#2;" : "=&r"(result),"=&r"(y): "1"(y),"r"(x): "cc","memory");

    return result;

}
 
static __inline sbc_fixed SBC_MUL_DCT2(sbc_fixed x, sbc_fixed y)
{
    sbc_fixed 		result;
    __asm__ volatile("smull %0,%1,%3,%2; movs %0,%0,lsr#28; orr %0,%0,%1,lsl#4;" : "=&r"(result),"=&r"(y): "1"(y),"r"(x): "cc","memory");
    return result;
}

static __inline sbc_fixed SBC_MUL_Q(sbc_fixed x1, sbc_fixed x2, sbc_fixed y)
{
    sbc_fixed 		result;
    __asm__ volatile("adds %3,%3,%4 \n\t"
                     "smull %0,%1,%3,%2 \n\t"
		     "movs %0,%0,lsr#31 \n\t"
		     "orr %0,%0,%1,lsl#1"
		     : "=&r"(result),"=&r"(y)
		     : "1"(y),"r"(x1),"r"(x2)
		     : "cc","memory");

    return result;
}
#endif /* __GNUC__ */

#if defined(__ICCARM__)
inline sbc_fixed SBC_MUL_DCT1(sbc_fixed x, sbc_fixed y)
{
    __asm("SMULL R0, R1, R0, R1");
    __asm("MOVS  R0, R0, LSR #30");
    __asm("ORR   R0, R0, R1, LSL #2");
}
 
inline sbc_fixed SBC_MUL_DCT2(sbc_fixed x, sbc_fixed y)
{
    __asm("SMULL R0, R1, R0, R1");
    __asm("MOVS  R0, R0, LSR #28");
    __asm("ORR   R0, R0, R1, LSL #4");
}

inline sbc_fixed SBC_MUL_Q(sbc_fixed x1, sbc_fixed x2, sbc_fixed y)
{
    __asm("ADDS  R0, R0, R1");
    __asm("SMULL R4, R2, R0, R2");
    __asm("MOVS  R4, R4, LSR #31");
    __asm("ORR   R0, R4, R2, LSL #1");
}
#endif /* __ICCARM__ */

#if defined(__CC_ARM)
__inline sbc_fixed SBC_MUL_DCT1(sbc_fixed x, sbc_fixed y)
{
    register sbc_fixed 		result;
    
    __asm 
    { 
    	SMULL result, y, x, y ;
    	MOVS  result, result, LSR #30;
    	ORR  result, result, y, LSL #2;  
    }
    return result;

}
 
__inline sbc_fixed SBC_MUL_DCT2(sbc_fixed x, sbc_fixed y)
{
    register sbc_fixed 		result;
    
    __asm 
    { 
    	SMULL result, y, x, y ;
    	MOVS  result, result, LSR #28;
    	ORR  result, result, y, LSL #4;  
    }
    return result;
}

__inline sbc_fixed SBC_MUL_Q(sbc_fixed x1, sbc_fixed x2, sbc_fixed y)
{
    register sbc_fixed 		result;
    __asm__
    {
    	ADDS x1, x1, x2;
    	SMULL result, y, x1, y;
    	MOVS result, result, LSR #31;
    	ORR result, result, y, LSL #1;
    }
    return result;
}
#endif /* __CC_ARM */
#endif

#else /* No ASM, Starand C */
/* --- Starand C ------------------------------------------------------------- */
#define SBC_MUL_DCT1(x, y)  		(sbc_fixed)(((sbc_fixed64)(x) * (sbc_fixed64)(y) + (1L << (FRACBITS - 1))) >> FRACBITS)
#define SBC_MUL_DCT2(x, y)  		(sbc_fixed)(((sbc_fixed64)(x) * (sbc_fixed64)(y) + (1L << (FRACBITS - 3))) >> (FRACBITS - 2))
#define SBC_MUL_Q(x1, x2, y) 		(sbc_fixed)(((sbc_fixed64)((x1) + (x2)) * (sbc_fixed64)(y)) >> (15 + 2 + ASFRACBITS_X))
#endif

/* --- Polyanalysis MLA ------------------------------------------------------- */
#define MLA(hi, lo, x, y) \
{\
	sbc_fixed64 r64 = (sbc_fixed64)(((sbc_fixed64)(x) * (sbc_fixed64)(y)));\
	r64 += ((sbc_fixed64)(hi) << 32) | (sbc_fixed64)(lo);\
	(hi) = (sbc_fixed)((r64) >> 32);\
	(lo) = (sbc_fixed)(r64);\
}

#define MLZ(hi, lo) \
{\
	sbc_fixed64 o64 = ((sbc_fixed64)(hi) << 32) | (sbc_fixed64)(lo);\
	(lo) = (sbc_fixed)(((o64) + (1L << (FRACBITS - TOSHORT_X - 1))) >> (FRACBITS - TOSHORT_X));\
}

#define MLZ_D(hi, lo) \
{\
	sbc_fixed64 o64 = ((sbc_fixed64)(hi) << 32) | (sbc_fixed64)(lo);\
	(lo) = (sbc_fixed)(((o64) + (1L << (FRACBITS - 1))) >> (FRACBITS));\
}

#define MLN(hi, lo) {\
	sbc_fixed64 temp;\
	temp = -(((sbc_fixed64)(hi) << 32) | (sbc_fixed64)(lo));\
	hi = (sbc_fixed)(temp >> 32);\
	lo = (sbc_fixed)temp;\
}

#define MLO(hi, lo, x, y) \
{\
	sbc_fixed64 r64 = (sbc_fixed64)(((sbc_fixed64)(x) * (sbc_fixed64)(y)));\
	(hi) = (sbc_fixed)(r64 >> 32);\
	(lo) = (sbc_fixed)(r64);\
}

#define SBC_MUL_DECODER_Q(x, y)  		(sbc_fixed)(((sbc_fixed64)(x) * (sbc_fixed64)(y)) >> (FRACBITS))

/*---------------------------------------------------------------------------*/
/* 							SBC Codec Frame Context 		 				 */
/*---------------------------------------------------------------------------*/
typedef struct s_sbc_frame_context 
{
	/* analyzed frame information */
	UINT8			sampling_frequency;						/* SBC_SF_48 */
	UINT8			nrof_blocks;
	UINT8			channel_mode;							/* SBC_CHANNEL_MODE_JOINT_STEREO */
	UINT8			allocation_method;						/* SBC_AM_SNR */
	UINT8			nrof_channels;	
	UINT8			nrof_subbands;	
	UINT8			bitpool;
	UINT8			nchnsb;
	UINT8			is_sbc2pcm;	
	UINT8			is_msbc;								/* flag, sbc or msbc encode/decode */	
	UINT8			sbc_frame_info_x;	
	UINT8			crc_check;								/* Now Only Decoder */
	UINT8			padding_x;								/* Per Padding Calc */
	UINT8			phase;									/* Polyphase filterbank Phase Flag */
	UINT8			joint_x;
	UINT8			joint_x_sb;
	UINT16 			pcm_samples_per_sbc_frame;				/* = nrof_blocks * nrof_channels * nrof_subbands */		
	UINT16			framelength;	
	UINT16			incomplete_frame_len;					/* if it is zero, indicates that no incomplete frame. */
	sbc_fixed		scale_factor[MAX_CH][MAX_SB];	
	UINT8			incomplete_frame_buf[SBC_MAXFRMLEN];
	
	/* common help variables for encoder & decoder */	
	sbc_fixed 		bits[MAX_CH][MAX_SB];
	sbc_fixed	 	audio_sample[MAX_BLK][MAX_CH][MAX_SB];	/* Subband Sample Also Save here */
	sbc_fixed 		bitneed[MAX_CH][MAX_SB]; 
	sbc_fixed		fixenfilter[2][2][8][5];				/* Polyphase filterbank inputs/output, [channel][even/odd][subband][v] */
	
	/* Decoder Only */
	void (*sbc_synthesis)(void *fc1, short *pcm);
	sbc_cache32		cache;
	int				cachebits;
	
	/* Encoder Only */
	void (*sbc_analysis)(void *fc1, short *pcm);
	sbc_fixed		scale_factor_factor[MAX_CH][MAX_SB];	/* True Scale_factor */
	sbc_fixed		fixed_w_y[(MAX_CH * MAX_SB * 2)];		/* Polyphase filterbank outputs */
	sbc_fixed		sf_lv[MAX_CH][MAX_SB];					/* Quantize Use this */	
	
	UINT8 			*srcbuf;         						/* Encoder output bit stream buffer */	

	sbc_fixed_uint	after_crc_x;							/* For Constant CRC Calculate */
 } sbc_frame_context;

/*---------------------------------------------------------------------------*/
/* 							SBC Codec Misc Structure 		 				 */
/*---------------------------------------------------------------------------*/
typedef struct s_sbc_stream_header
{ 
	UINT8		*pbSrc; /* Shall be 2 octets aligned */
	UINT16      cbSrcLength; 
	UINT16      cbSrcLengthUsed; 
	UINT8		*pbDst; 
	UINT16      cbDstLength; 
	UINT16      cbDstLengthUsed;
} sbc_stream_header; 

/*---------------------------------------------------------------------------*/
/* 							SBC Codec UI Functions 		 					 */
/*---------------------------------------------------------------------------*/
sbc_frame_context *sbc_open(UINT8 *hdr/* 2 octets */, UINT8 is_sbc2pcm, UINT8 is_msbc);
void sbc_close(sbc_frame_context *fc);
void sbc_convert(sbc_frame_context *fc, sbc_stream_header *pssh);
UINT32 sbc_calc_pcm_framesize(sbc_frame_context *fc);
UINT16 sbc_calc_sbc_framesize(sbc_frame_context *fc);

#endif

