/*
    bootloader - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    config/tone/cfg_ring.h
 * @brief   call ring include file.
 * @details 
 *
 * @addtogroup  config
 * @details 
 * @{
 */
#ifndef __CFG_RING_H__
#define __CFG_RING_H__

/*===========================================================================*/
/* macros default.                                                           */
/*===========================================================================*/
#define CFG_RING_WRITE_BLOCKSIZE    SIZE_4K
#define CFG_RING_CALLPHONE_LEN      32

#define CFG_RING_SBC_BLOCKS         16
#define CFG_RING_SBC_SUBBANDS       8
#define CFG_RING_SBC_BITPOOL        16
#define CFG_RING_SBC_CHNNUM         2

#define CFG_RING_SBC_FRAMELEN       \
  (4 + (4 * CFG_RING_SBC_SUBBANDS * CFG_RING_SBC_CHNNUM) / 8 + (CFG_RING_SBC_BLOCKS * CFG_RING_SBC_BITPOOL + 7) / 8)

#define CFG_RING_PCM_FRAMELEN       \
  (CFG_RING_SBC_BLOCKS * CFG_RING_SBC_SUBBANDS * CFG_RING_SBC_CHNNUM * 2)

#define CFG_RING_REC_THREAD_PRIO    (NORMALPRIO + 10)
/*===========================================================================*/
/* data structure and data type.                                             */
/*===========================================================================*/
typedef enum
{
  CFG_RING_TYPE_NAME         = 0,
  CFG_RING_TYPE_TTS          ,
}hs_cfg_ring_type_t;

enum
{
  CFG_RING_NOT_CALLED       = 0,
  CFG_RING_HAVE_CALLED      ,
};

enum
{
  CFG_RING_NAME_RECORD_START    = 0x80,
  CFG_RING_NAME_RECORD_STOP     ,
};

typedef struct
{  
  uint32_t   u32SampleRate;     /*!< ring sample rate    */
  uint32_t   u32SpaceSize;      /*!< space of total rings     */
  uint16_t   u16OneSize;        /*!< space of a ring     */  
  uint16_t   u16RingType;       /*!< ring type 0-name voice 1-tts    */  
  uint16_t   u16SaveCnt;        /*!< the count of saved ring     */
}hs_cfg_ring_info_t;

typedef struct
{  
  uint32_t   u32ValidLen;     
  uint8_t    u8TelPhone[CFG_RING_CALLPHONE_LEN];
}hs_cfg_ring_header_t;

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/




#ifdef __cplusplus
extern "C" {
#endif

void hs_cfg_ringPlay(uint8_t u8Idx, void *parg);
void hs_cfg_ringRecordStart(uint8_t u8Idx, void *parg);
void hs_cfg_ringRecordStop(uint8_t u8Idx, void *parg);
void hs_cfg_ringComing(uint16_t u16Idx, void*parg);

//----incoming call feature----
void hs_cfg_ringIncomingCallStart(uint16_t u8Idx, void *parg);
void hs_cfg_ringIncomingCallStop(uint16_t u8Idx, void *parg);

#ifdef __cplusplus
}
#endif


#endif
 /** @} */
