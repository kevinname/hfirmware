/*
    bootloader - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    config/tone/cfg_tone.h
 * @brief   tone manage include file.
 * @details 
 *
 * @addtogroup  config
 * @details 
 * @{
 */
#ifndef __CFG_TONE_H__
#define __CFG_TONE_H__

/*===========================================================================*/
/* macros default.                                                           */
/*===========================================================================*/
#define CFG_TONE_4K_MAXNUM            32
#define CFG_TONE_DEFAULT_MASK         0x8000

#define CFG_TONE_ERROR_ADDR           0xffffffff

/*===========================================================================*/
/* data structure and data type.                                             */
/*===========================================================================*/
enum
{
  CFG_SBC_CHNMODE_MONO        = 0,
  CFG_SBC_CHNMODE_DUALCHN     ,
  CFG_SBC_CHNMODE_STEREO      ,
  CFG_SBC_CHNMODE_JOINTSTEREO ,
};

typedef struct
{
  uint32_t   u32BitRate;        /*!< sample rate    */
  int16_t    s16ToneVolume;     /*!< tone volume in dB */  
  uint16_t   u16DataLen;
  uint16_t   u16LoopNum;        /*!< loop number    */
  uint16_t   u16Delay;          /*!< unit: s    */ 
  
  uint8_t    u8Blocks;          /*!<     */
  uint8_t    u8SubBands;        /*!<     */
  uint8_t    u8BitPool;         /*!<     */
  uint8_t    u8ChnMode;         /*!<     */
}hs_cfg_sound_info_t;

typedef struct 
{
  uint8_t    u8MessType;        /*!< event or status type                               */  
  uint8_t    u8Reserv;          /*!< reserve                                            */  
  uint16_t   u16Message;        /*!< message                                            */  
  uint16_t   u16ToneIndex;      /*!< tone index to be played                            */  
}hs_cfg_sound_event_t;

typedef struct
{
  uint8_t    u8ToneEnable;      /*!< tone enable: 0-disable, 1-enable    */  
  uint8_t    u8Sound4kNum;      /*!<     */
  uint8_t    u8Sound8kNum;      /*!<     */
  uint8_t    u8Sound16kNum;     /*!<     */
  uint8_t    u8SndBindEvent;    /*!< the number of sound bind event    */   

  uint8_t    u8SndMonoMode;     /*!< mono mode, 0-one channel, 1-two channel same    */  
}hs_cfg_tone_info_t;



/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/




#ifdef __cplusplus
extern "C" {
#endif

hs_cfg_res_t hs_cfg_toneDoEvent(hs_cfg_mess_type_t m_type, uint16_t message, uint8_t inte);

#ifdef __cplusplus
}
#endif


#endif
 /** @} */
