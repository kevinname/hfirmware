/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio
                 Copyright (C) 2014 HunterSun Technologies
                 wei.lu@huntersun.com.cn

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
/*
   Concepts and parts of this file have been contributed by Uladzimir Pylinsky
   aka barthess.
 */

/**
 * @file    hs66xx/rtc_lld.h
 * @brief   HS66xx RTC subsystem low level driver header.
 *
 * @addtogroup RTC
 * @{
 */

#ifndef _RTC_LLD_H_
#define _RTC_LLD_H_

#if HAL_USE_RTC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   Two alarm comparator available.
 */
#define RTC_ALARMS                  2

/**
 * @brief   Data offsets in RTC date and time registers.
 */
#define RTC_TR_PM_OFFSET    22
#define RTC_TR_HT_OFFSET    20
#define RTC_TR_HU_OFFSET    16
#define RTC_TR_MNT_OFFSET   12
#define RTC_TR_MNU_OFFSET   8
#define RTC_TR_ST_OFFSET    4
#define RTC_TR_SU_OFFSET    0

#define RTC_DR_YT_OFFSET    20
#define RTC_DR_YU_OFFSET    16
#define RTC_DR_WDU_OFFSET   13
#define RTC_DR_MT_OFFSET    12
#define RTC_DR_MU_OFFSET    8
#define RTC_DR_DT_OFFSET    4
#define RTC_DR_DU_OFFSET    0

/**
 * @brief   Data offsets in RTC alarm date and time registers.
 */

#define RTC_ALRM_DT                        ((uint32_t)0x30000000)
#define RTC_ALRM_DU                        ((uint32_t)0x0F000000)
#define RTC_ALRM_HT                        ((uint32_t)0x00300000)
#define RTC_ALRM_HU                        ((uint32_t)0x000F0000)
#define RTC_ALRM_MNT                       ((uint32_t)0x00007000)
#define RTC_ALRM_MNU                       ((uint32_t)0x00000F00)
#define RTC_ALRM_ST                        ((uint32_t)0x00000070)
#define RTC_ALRM_SU                        ((uint32_t)0x0000000F)

#define RTC_ALARM_DATE_MASK_OFFSET   31
#define RTC_ALARM_DT_OFFSET          28
#define RTC_ALARM_DU_OFFSET          24
#define RTC_ALARM_HOUR_MASK_OFFSET   23
#define RTC_ALARM_HT_OFFSET          20
#define RTC_ALARM_HU_OFFSET          16
#define RTC_ALARM_MIN_MASK_OFFSET    15
#define RTC_ALARM_MT_OFFSET          12
#define RTC_ALARM_MU_OFFSET          8
#define RTC_ALARM_SEC_MASK_OFFSET    7
#define RTC_ALARM_ST_OFFSET          4
#define RTC_ALARM_SU_OFFSET          0

/**
 * @brief   RTC Wakeup clock selection
 */

#define RTC_WAKEUP_CLK_DIV_16       0
#define RTC_WAKEUP_CLK_DIV_8        1
#define RTC_WAKEUP_CLK_DIV_4        2
#define RTC_WAKEUP_CLK_DIV_2        3
#define RTC_WAKEUP_CLK_CK_SPRE_1Hz  4
#define RTC_WAKEUP_CLK_ADD_WUT      6

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   This RTC implementation supports callbacks.
 */
#define RTC_SUPPORTS_CALLBACKS      FALSE//TRUE

/**
 * @name    Configuration options
 * @{
 */
/*
 * RTC driver system settings.
 */
#if !defined(HS_RTC_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define HS_RTC_IRQ_PRIORITY      3
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !defined(RTC_USE_INTERRUPTS) || defined(__DOXYGEN__)
#define RTC_USE_INTERRUPTS                FALSE
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a structure representing an RTC alarm time stamp.
 */
typedef struct RTCAlarm RTCAlarm;

/**
 * @brief   Type of a structure representing an RTC wakeup period.
 */
typedef struct RTCWakeup RTCWakeup;

/**
 * @brief   Type of a structure representing an RTC callbacks config.
 */
typedef struct RTCCallbackConfig RTCCallbackConfig;

/**
 * @brief   Type of an RTC alarm.
 * @details Meaningful on platforms with more than 1 alarm comparator.
 */
typedef uint32_t rtcalarm_t;
/**
 * @brief   Type of an RTC event.
 */
typedef enum {
  RTC_EVENT_AUTO_WAKEUP = 0,            /** Triggered every second.         */
  RTC_EVENT_ALARMA = 1,                 /** Triggered on alarm A.           */
  RTC_EVENT_ALARMB = 2,                 /** Triggered on alarm B.           */  
  RTC_EVENT_TIMSTAMP_OVERFLOW = 3,      /** Triggered on TimeStamp overflow.*/
  RTC_EVENT_TIMSTAMP = 4                /** Triggered on TimeStamp occur.  */ 
} rtcevent_t;

/**
 * @brief   Type of a generic RTC callback.
 */
typedef void (*rtccb_t)(RTCDriver *rtcp, rtcevent_t event);

/**
 * @brief   Structure representing an RTC time stamp.
 */
struct RTCTime {
  /**
   * @brief RTC date register in BCD format.
   */
  uint32_t tv_date;
  /**
   * @brief RTC time register in BCD format.
   */
  uint32_t tv_time;
  /**
   * @brief Set this to TRUE to use 12 hour notation.
   */
  bool_t h12;
  /**
   * @brief Fractional part of time.
   */
  uint32_t tv_msec;
};

/**
 * @brief   Structure representing an RTC alarm time stamp.
 */
struct RTCAlarm {
  /**
   * @brief Date and time of alarm in BCD.
   */
  uint32_t tv_datetime;
};

/**
 * @brief   Structure representing an RTC periodic wakeup period.
 */
struct RTCWakeup {
  /**
   * @brief   RTC WUTR register.
   * @details Bits [15:0] contain value of WUTR register
   *          Bits [18:16] contain value of WUCKSEL bits in CR register
   *
   * @note    ((WUTR == 0) || (WUCKSEL == 3)) is forbidden combination.
   */
  uint32_t wakeup;
};

/**
 * @brief   Structure representing an RTC driver.
 */
struct RTCDriver{
  /**
   * @brief Pointer to the RTC registers block.
   */
  HS_RTC_Type                *id_rtc;
  
  /**
   * @brief Callback pointer.
   */
  rtccb_t           callback;  
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(__DOXYGEN__)
extern RTCDriver RTCD0;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void rtc_lld_init(void);
  void rtc_lld_set_time(RTCDriver *rtcp, const RTCTime *timespec);
  void rtc_lld_get_time(RTCDriver *rtcp, RTCTime *timespec);
  void rtc_timestamp_init(RTCDriver *rtcp, int No);
  void rtc_lld_get_timestamp(RTCDriver *rtcp, RTCTime *timespec); 
  void rtc_lld_set_alarm(RTCDriver *rtcp,
                         rtcalarm_t alarm,
                         const RTCAlarm *alarmspec);
  void rtc_lld_get_alarm(RTCDriver *rtcp,
                         rtcalarm_t alarm,
                         RTCAlarm *alarmspec);
  void rtcSetPeriodicWakeup(RTCDriver *rtcp, RTCWakeup *wakeupspec);
  void rtcGetPeriodicWakeup(RTCDriver *rtcp, RTCWakeup *wakeupspec);
  //void rtc_lld_set_callback(RTCDriver *rtcp, rtccb_t callback);
  uint32_t rtc_lld_get_time_fat(RTCDriver *rtcp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_RTC */

#endif /* _RTC_LLD_H_ */

/** @} */
