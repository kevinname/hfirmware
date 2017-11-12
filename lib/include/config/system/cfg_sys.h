/*
    bootloader - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    config/cfg_sys.h
 * @brief   config system manage include file.
 * @details 
 *
 * @addtogroup  config
 * @details 
 * @{
 */
#ifndef __CFG_SYS_H__
#define __CFG_SYS_H__

#include "ch.h"
/*===========================================================================*/
/* macros default.                                                           */
/*===========================================================================*/
#define MESSAGE_QUEUE_CNT         30
#define FAST_EVENT_MASK           0x8000u
/*===========================================================================*/
/* data structure and data type.                                             */
/*===========================================================================*/


typedef enum
{
  BN_TYPE_SINGLE_CLICK            = 0,    /*!< 0 */
  BN_TYPE_DOUBLE_CLICK            ,       /*!< 1 */
  BN_TYPE_LONG_PRESS              ,       /*!< 2 */
  BN_TYPE_VERYLONG_PRESS          ,       /*!< 3 */
  BN_TYPE_COMBINATION_KEY         ,       /*!< 4 */

  BN_TYPE_REPEAT                  ,       /*!< 5 */
  BN_TYPE_PRESS_DOWN              ,       /*!< 6 */
  BN_TYPE_PRESS_RELEASE           ,       /*!< 7 */

  BN_TYPE_INVALID                 ,       /*!< 8 */
}hs_cfg_bn_type_t;


typedef struct
{
  uint8_t       u8BnBindEvtCnt;             /*!< the count of button action bind event    */
  uint8_t       u8EvtBindLedCnt;            /*!< the count of event bind led action index */
  uint8_t       u8LedActionCnt;             /*!< the count of led action                  */
}hs_cfg_sys_info_t;

typedef struct hs_cfg_bn_attr hs_cfg_bn_attr_t;
struct hs_cfg_bn_attr
{
  uint32_t      u32BitMap;                  /*!< gpio bitmap have used to bind button               */  
  uint16_t      u16DoubleClickInterval;     /*!< The interval time between double click, unit: ms   */
  uint16_t      u16SingleRepeatInterval;    /*!< The min interval time between two single click     */
  uint16_t      u16LongPressTime;           /*!< The time at long press click, unit: ms             */
  uint16_t      u16VeryLongPressTime;       /*!< The time at very long press click, unit: ms        */

  uint16_t      u16RepeatTime;              /*!< The time for repeat event, unit: ms        */

  uint8_t       u8PressLevel;               /*!< The gpio value when button clicked                 */  
  uint8_t       u8DebounceTime;             /*!< debounce time, unit: ms                            */
  uint8_t       u8DebounceNumber;           /*!< debounce number                                    */
} __PACKED_GCC;

typedef struct
{
  uint16_t      u16PeroidTime;              /*!< The time of led on during a action                 */
  uint16_t      u16DutyTime;                /*!< The time of a action                               */
  uint8_t       u8RepeatCnt;                /*!< repeat count of the action                         */
  uint8_t       u8LedOnLevel;               /*!< The gpio value when led on                         */
}hs_cfg_led_action_t;

typedef struct hs_cfg_bn_event hs_cfg_bn_event_t;
struct hs_cfg_bn_event
{
  uint32_t      u32BnMask;                  /*!< button mask bits which have been pressed           */
  uint8_t       u8BnType;                   /*!< button action type                                 */  
  uint8_t       u8EventMod;                 /*!< event module                                       */
  uint16_t      u16Event;                   /*!< event                                              */
  uint32_t      u32Arg;                     /*!< argument of the event                              */
} __PACKED_GCC;

typedef struct hs_cfg_event_led hs_cfg_event_led_t;
struct hs_cfg_event_led
{
  uint8_t       u8MessType;                 /*!< event or status type                               */    
  uint8_t       u8LedIndex;                 /*!< led index to do action                             */  
  uint8_t       u8LedPadMuxIdx;             /*!< pad muxing selected of gpio binding the led        */
  uint8_t       u8LedActionIdx;             /*!< led action index                                   */ 
  uint16_t      u16Message;                 /*!< message                                            */  
} __PACKED_GCC;

typedef void (*hs_evtFunction_t)(uint16_t u16Msg, void *parg);
typedef struct
{
  uint16_t          u16MessType;            /*!< event or status type                               */
  hs_evtFunction_t  pfnOper;                /*!< event or status type corresponding operation       */
}hs_cfg_mess_oper_t;


/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/


#ifdef __cplusplus
extern "C" {
#endif

void hs_cfg_sysInit(void);

/*
 * @brief               send message to system manage when entering some status 
 *                      or some event generated
 *                      
 * @param[in] m_mod     message module
 * @param[in] m_type    message type
 * @param[in] message   message content
 * @param[in] parg      param of event sending to service 
 *                      .
 * @return              0-ok, other-some error happened
 */
hs_cfg_res_t hs_cfg_sysSendMsg(hs_cfg_mod_t m_mod, hs_cfg_mess_type_t m_type, uint16_t message, void *parg);

/*
 * @brief               listening a message 
 *                      if the message have happened, event function will be called.
 *                      
 * @param[in] message   message 
 * @param[in] fnOper    event function handler
 *                      .
 * @return              0-ok, other-some error happened
 */
hs_cfg_res_t hs_cfg_sysListenMsg(uint16_t message, hs_evtFunction_t fnOper);

/*
 * @brief               cancel listen a message 
 *                      if the message have been listened, cancel the message.
 *                      
 * @param[in] message   message 
 * @param[in] fnOper    event function handler
 *                      .
 * @return              no
 */
void hs_cfg_sysCancelListenMsg(uint16_t message, hs_evtFunction_t fnOper);

/*
 * @brief               send message to system manage when entering some status 
 *                      or some event generated
 *                      
 * @param[in] m_mod     message module
 * @param[in] m_type    message type
 * @param[in] message   message content
 *                      .
 * @return              0-ok, other-some error happened
 */
hs_cfg_res_t hs_cfg_sysSendMessage(hs_cfg_mod_t m_mod, hs_cfg_mess_type_t m_type, uint16_t message);

void hs_cfg_sysClearMsg(void);

#ifdef __cplusplus
}
#endif

static inline hs_cfg_res_t hs_cfg_sysPeripStsChange(uint16_t u16Msg, void *parg)
{
  return hs_cfg_sysSendMsg(HS_CFG_MODULE_PERIP, HS_CFG_SYS_STATUS, u16Msg, parg);
}

static inline hs_cfg_res_t hs_cfg_systemStsChange(uint16_t u16Msg)
{
  return hs_cfg_sysSendMsg(HS_CFG_MODULE_SYS, HS_CFG_SYS_STATUS, u16Msg, NULL);
}

/* system */
static inline hs_cfg_res_t hs_cfg_systemReq(uint16_t u16Msg)
{
  return hs_cfg_sysSendMsg(HS_CFG_MODULE_SYS, HS_CFG_SYS_EVENT, u16Msg, NULL);
}

static inline hs_cfg_res_t hs_cfg_systemReqArg(uint16_t u16Msg, void *parg)
{
  return hs_cfg_sysSendMsg(HS_CFG_MODULE_SYS, HS_CFG_SYS_EVENT, u16Msg, parg);
}

static inline hs_cfg_res_t hs_cfg_sysReqPerip(uint16_t u16Msg)
{
  return hs_cfg_sysSendMsg(HS_CFG_MODULE_PERIP, HS_CFG_SYS_EVENT, u16Msg, NULL);
}

static inline hs_cfg_res_t hs_cfg_sysReqPeripArg(uint16_t u16Msg, void *parg)
{
  return hs_cfg_sysSendMsg(HS_CFG_MODULE_PERIP, HS_CFG_SYS_EVENT, u16Msg, parg);
}

static inline hs_cfg_res_t hs_cfg_btReq(uint16_t u16Msg)
{
  return hs_cfg_sysSendMsg(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, u16Msg, NULL);
}

static inline hs_cfg_res_t hs_cfg_btReqArg(uint16_t u16Msg, void *parg)
{
  return hs_cfg_sysSendMsg(HS_CFG_MODULE_BT_CLASSIC, HS_CFG_SYS_EVENT, u16Msg, parg);
}

static inline hs_cfg_res_t hs_cfg_playerReq(uint16_t u16Msg)
{
  return hs_cfg_sysSendMsg(HS_CFG_MODULE_PLAYER, HS_CFG_SYS_EVENT, u16Msg, NULL);
}

static inline hs_cfg_res_t hs_cfg_playerReqArg(uint16_t u16Msg, void *parg)
{
  return hs_cfg_sysSendMsg(HS_CFG_MODULE_PLAYER, HS_CFG_SYS_EVENT, u16Msg, parg);
}

#endif
 /** @} */
