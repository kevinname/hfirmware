/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

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

#include "ch.h"
#include "hal.h"
#include "stdlib.h"
#include "lib.h"

#if HAL_USE_RTC || defined(__DOXYGEN__)

/**
 * @file    chrtclib.c
 * @brief   RTC time conversion utilities code.
 *
 * @addtogroup chrtclib
 * @{
 */

#include <time.h>
#include "chrtclib.h"

#if defined(HS6601)
#define STM32_RTC_HAS_SUBSECONDS TRUE
#define STM32_RTC_IS_CALENDAR    TRUE
#endif

#if !defined(__GNUC__)
static MUTEX_DECL(mtx_localtime);

static struct tm *localtime_r(const time_t *tim, struct tm *result)
{
  struct tm *t;

  chMtxLock(&mtx_localtime);
  t = localtime(tim);
  if (t)
    *result = *t;
  chMtxUnlock();

  return t ? result : NULL;
}
#endif

#if (defined(STM32F4XX) || defined(STM32F2XX) || defined(STM32L1XX) || \
     defined(STM32F30X) || defined(STM32F37X) || \
     defined(STM32F1XX) || defined(STM32F10X_MD) || defined(STM32F10X_LD) || \
     defined(STM32F10X_HD) || defined(HS6601) || defined(__DOXYGEN__))
#if STM32_RTC_IS_CALENDAR
/**
 * @brief   Converts from STM32 BCD to canonicalized time format.
 *
 * @param[out] timp     pointer to a @p tm structure as defined in time.h
 * @param[in] timespec  pointer to a @p RTCTime structure
 *
 * @notapi
 */
static void stm32_rtc_bcd2tm(struct tm *timp, RTCTime *timespec) {
  uint32_t tv_time = timespec->tv_time;
  uint32_t tv_date = timespec->tv_date;

#if CH_DBG_ENABLE_CHECKS
  timp->tm_isdst = 0;
  timp->tm_wday  = 0;
  timp->tm_mday  = 0;
  timp->tm_yday  = 0;
  timp->tm_mon   = 0;
  timp->tm_year  = 0;
  timp->tm_sec   = 0;
  timp->tm_min   = 0;
  timp->tm_hour  = 0;
#endif

  timp->tm_isdst = -1;

  timp->tm_wday = (tv_date & RTC_DR_WDU) >> RTC_DR_WDU_OFFSET;
  if (timp->tm_wday == 7)
    timp->tm_wday = 0;

  timp->tm_mday =  (tv_date & RTC_DR_DU) >> RTC_DR_DU_OFFSET;
  timp->tm_mday += ((tv_date & RTC_DR_DT) >> RTC_DR_DT_OFFSET) * 10;

  timp->tm_mon  =  (tv_date & RTC_DR_MU) >> RTC_DR_MU_OFFSET;
  timp->tm_mon  += ((tv_date & RTC_DR_MT) >> RTC_DR_MT_OFFSET) * 10;
  timp->tm_mon  -= 1;

  timp->tm_year =  (tv_date & RTC_DR_YU) >> RTC_DR_YU_OFFSET;
  timp->tm_year += ((tv_date & RTC_DR_YT) >> RTC_DR_YT_OFFSET) * 10;
  timp->tm_year += 2000 - 1900; /* not compatible with STM32F4 */

  timp->tm_sec  =  (tv_time & RTC_TR_SU) >> RTC_TR_SU_OFFSET;
  timp->tm_sec  += ((tv_time & RTC_TR_ST) >> RTC_TR_ST_OFFSET) * 10;

  timp->tm_min  =  (tv_time & RTC_TR_MNU) >> RTC_TR_MNU_OFFSET;
  timp->tm_min  += ((tv_time & RTC_TR_MNT) >> RTC_TR_MNT_OFFSET) * 10;

  timp->tm_hour =  (tv_time & RTC_TR_HU) >> RTC_TR_HU_OFFSET;
  timp->tm_hour += ((tv_time & RTC_TR_HT) >> RTC_TR_HT_OFFSET) * 10;
  timp->tm_hour += 12 * ((tv_time & RTC_TR_PM) >> RTC_TR_PM_OFFSET);
}

/**
 * @brief   Converts from canonicalized to STM32 BCD time format.
 *
 * @param[in] timp      pointer to a @p tm structure as defined in time.h
 * @param[out] timespec pointer to a @p RTCTime structure
 *
 * @notapi
 */
static void stm32_rtc_tm2bcd(struct tm *timp, RTCTime *timespec) {
  uint32_t v = 0;

  timespec->tv_date = 0;
  timespec->tv_time = 0;

  v = timp->tm_year - (2000 - 1900);  /* not compatible with STM32F4 */
  timespec->tv_date |= ((v / 10) << RTC_DR_YT_OFFSET) & RTC_DR_YT;
  timespec->tv_date |= (v % 10) << RTC_DR_YU_OFFSET;

  if (timp->tm_wday == 0)
    v = 7;
  else
    v = timp->tm_wday;
  timespec->tv_date |= (v << RTC_DR_WDU_OFFSET) & RTC_DR_WDU;

  v = timp->tm_mon + 1;
  timespec->tv_date |= ((v / 10) << RTC_DR_MT_OFFSET) & RTC_DR_MT;
  timespec->tv_date |= (v % 10) << RTC_DR_MU_OFFSET;

  v = timp->tm_mday;
  timespec->tv_date |= ((v / 10) << RTC_DR_DT_OFFSET) & RTC_DR_DT;
  timespec->tv_date |= (v % 10) << RTC_DR_DU_OFFSET;

  v = timp->tm_hour;
  timespec->tv_time |= ((v / 10) << RTC_TR_HT_OFFSET) & RTC_TR_HT;
  timespec->tv_time |= (v % 10) << RTC_TR_HU_OFFSET;

  v = timp->tm_min;
  timespec->tv_time |= ((v / 10) << RTC_TR_MNT_OFFSET) & RTC_TR_MNT;
  timespec->tv_time |= (v % 10) << RTC_TR_MNU_OFFSET;

  v = timp->tm_sec;
  timespec->tv_time |= ((v / 10) << RTC_TR_ST_OFFSET) & RTC_TR_ST;
  timespec->tv_time |= (v % 10) << RTC_TR_SU_OFFSET;
}

/**
 * @brief   Gets raw time from RTC and converts it to canonicalized format.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @param[out] timp     pointer to a @p tm structure as defined in time.h
 *
 * @api
 */
void rtcGetTimeTm(RTCDriver *rtcp, struct tm *timp) {
#if STM32_RTC_HAS_SUBSECONDS
  RTCTime timespec = {0,0,FALSE,0};
#else
  RTCTime timespec = {0,0,FALSE};
#endif

  rtcGetTime(rtcp, &timespec);
  stm32_rtc_bcd2tm(timp, &timespec);
}

/**
 * @brief   Sets RTC time.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @param[out] timp     pointer to a @p tm structure as defined in time.h
 *
 * @api
 */
void rtcSetTimeTm(RTCDriver *rtcp, struct tm *timp) {
#if STM32_RTC_HAS_SUBSECONDS
  RTCTime timespec = {0,0,FALSE,0};
#else
  RTCTime timespec = {0,0,FALSE};
#endif

  /* HS66xx HW RTC calendar range [y2000, y2100) */
  if ((timp->tm_year < (2000 - 1900)) || (timp->tm_year >= (2100 - 1900)))
    return;

  stm32_rtc_tm2bcd(timp, &timespec);
  rtcSetTime(rtcp, &timespec);
}

/**
 * @brief   Gets raw time from RTC and converts it to unix format.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @return              Unix time value in seconds.
 *
 * @api
 */
time_t rtcGetTimeUnixSec(RTCDriver *rtcp) {
#if STM32_RTC_HAS_SUBSECONDS
  RTCTime timespec = {0,0,FALSE,0};
#else
  RTCTime timespec = {0,0,FALSE};
#endif
  struct tm timp;

  rtcGetTime(rtcp, &timespec);
  stm32_rtc_bcd2tm(&timp, &timespec);

  return mktime(&timp);
}

/**
 * @brief   Sets RTC time.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @param[in] tv_sec    time specification
 * @return              Unix time value in seconds.
 *
 * @api
 */
void rtcSetTimeUnixSec(RTCDriver *rtcp, time_t tv_sec) {
#if STM32_RTC_HAS_SUBSECONDS
  RTCTime timespec = {0,0,FALSE,0};
#else
  RTCTime timespec = {0,0,FALSE};
#endif
  struct tm timp;

  localtime_r(&tv_sec, &timp);
  /* HS66xx HW RTC calendar range [y2000, y2100) */
  if ((timp.tm_year < (2000 - 1900)) || (timp.tm_year >= (2100 - 1900)))
    return;

  stm32_rtc_tm2bcd(&timp, &timespec);
  rtcSetTime(rtcp, &timespec);
}

/**
 * @brief   Gets raw time from RTC and converts it to unix format.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @return              Unix time value in microseconds.
 *
 * @api
 */
uint64_t rtcGetTimeUnixUsec(RTCDriver *rtcp) {
#if STM32_RTC_HAS_SUBSECONDS
  uint64_t result = 0;
  RTCTime timespec = {0,0,FALSE,0};
  struct tm timp;

  rtcGetTime(rtcp, &timespec);
  stm32_rtc_bcd2tm(&timp, &timespec);

  result = (uint64_t)mktime(&timp) * 1000000;
  return result + timespec.tv_msec * 1000;
#else
  return (uint64_t)rtcGetTimeUnixSec(rtcp) * 1000000;
#endif
}

#else /* STM32_RTC_IS_CALENDAR */
/**
 * @brief   Gets raw time from RTC and converts it to canonicalized format.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @param[out] timp     pointer to a @p tm structure as defined in time.h
 *
 * @api
 */
void rtcGetTimeTm(RTCDriver *rtcp, struct tm *timp) {
  RTCTime timespec = {0,0};

  rtcGetTime(rtcp, &timespec);
  localtime_r((time_t *)&(timespec.tv_sec), timp);
}

/**
 * @brief   Sets RTC time.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @param[out] timp     pointer to a @p tm structure as defined in time.h
 *
 * @api
 */
void rtcSetTimeTm(RTCDriver *rtcp, struct tm *timp) {
  RTCTime timespec = {0,0};

  timespec.tv_sec = mktime(timp);
  timespec.tv_msec = 0;
  rtcSetTime(rtcp, &timespec);
}

/**
 * @brief   Gets raw time from RTC and converts it to unix format.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @return              Unix time value in seconds.
 *
 * @api
 */
time_t rtcGetTimeUnixSec(RTCDriver *rtcp) {
  RTCTime timespec = {0,0};

  rtcGetTime(rtcp, &timespec);
  return timespec.tv_sec;
}

/**
 * @brief   Sets RTC time.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @param[in] tv_sec    time specification
 * @return              Unix time value in seconds.
 *
 * @api
 */
void rtcSetTimeUnixSec(RTCDriver *rtcp, time_t tv_sec) {
  RTCTime timespec = {0,0};

  timespec.tv_sec = tv_sec;
  timespec.tv_msec = 0;
  rtcSetTime(rtcp, &timespec);
}

/**
 * @brief   Gets raw time from RTC and converts it to unix format.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @return              Unix time value in microseconds.
 *
 * @api
 */
uint64_t rtcGetTimeUnixUsec(RTCDriver *rtcp) {
#if STM32_RTC_HAS_SUBSECONDS
  uint64_t result = 0;
  RTCTime timespec = {0,0};

  rtcGetTime(rtcp, &timespec);
  result = (uint64_t)timespec.tv_sec * 1000000;
  return result + timespec.tv_msec * 1000;
#else
  return (uint64_t)rtcGetTimeUnixSec(rtcp) * 1000000;
#endif
}

/**
 * @brief   Get current time in format suitable for usage in FatFS.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @return              FAT time value.
 *
 * @api
 */
uint32_t rtcGetTimeFatFromCounter(RTCDriver *rtcp) {
  uint32_t fattime;
  struct tm timp;

  rtcGetTimeTm(rtcp, &timp);

  fattime  = (timp.tm_sec)       >> 1;
  fattime |= (timp.tm_min)       << 5;
  fattime |= (timp.tm_hour)      << 11;
  fattime |= (timp.tm_mday)      << 16;
  fattime |= (timp.tm_mon + 1)   << 21;
  fattime |= (timp.tm_year - 80) << 25;

  return fattime;
}
#endif /* STM32_RTC_IS_CALENDAR */
#endif /* (defined(STM32F4XX) || defined(STM32F2XX) || defined(STM32L1XX) || defined(STM32F1XX)) */

static void _rtc_string2StructTm(const char *Str,  struct tm *tv_tim)
{
  if((NULL == Str) || (NULL == tv_tim))
    return;

  char ch[5] = {0};
 
  tv_tim->tm_isdst = -1;
  
  strncpy(ch, Str, 4);
  tv_tim->tm_year = atol(ch) - 1900;
  
  memset(ch, 0, 4);
  strncpy(ch, Str+4, 2);
  tv_tim->tm_mon = atoi(ch) - 1;

  strncpy(ch, Str+6, 2);
  tv_tim->tm_mday = atoi(ch);
  
  strncpy(ch, Str+8, 2);
  tv_tim->tm_hour = atoi(ch);
  
  strncpy(ch, Str+10, 2);
  tv_tim->tm_min = atoi(ch);

  strncpy(ch, Str+12, 2);
  tv_tim->tm_sec = atoi(ch);
}

void hs_rtc_setTim(const char *tm)
{
  struct tm timp;
  char dt[32] = "20161001000000";

  if(tm)
    strcpy(dt, tm);

  _rtc_string2StructTm(dt, &timp);
  rtcSetTimeTm(&RTCD0, &timp);
}

void hs_rtc_getTim(struct tm *timp)
{
  rtcGetTimeTm(&RTCD0, timp);
}

static void RTC_Callback(RTCDriver *rtcp, rtcevent_t event)
{
  (void)rtcp;

  if ((event == RTC_EVENT_ALARMA) ||
      (event == RTC_EVENT_ALARMB)) {
    hs_cfg_systemReq(HS_CFG_EVENT_RTC_ALARM);
  }
}

/**
 * @brief   Set alarm.
 *
 * @param[in] day  day of the month in BCD format if 0x40 is not set: 0x01~0x31 (1~31);
 *                 day of the week  in BCD format if 0x40 is set: 0x40 | (0x01~0x07) (Monday~Sunday);
 *                 omit if 0x80 is set.
 * @param[in] hour hours in BCD format:   0x00~0x23 (0~23 hours, 24-hour format);
 *                 omit if 0x80 is set.
 * @param[in] min  minutes in BCD format: 0x00~0x59 (0~59 minutes);
 *                 omit if 0x80 is set.
 * @param[in] sec  seconds in BCD format: 0x00~0x59 (0~59 minutes);
 *                 omit if 0x80 is set.
 * 
 * @api
 */
void hs_rtc_setAlarm(uint8_t day, uint8_t hour, uint8_t min, uint8_t sec)
{
  RTCAlarm spec;

  RTCD0.callback = RTC_Callback;
  spec.tv_datetime = (day << 24u) | (hour << 16u) | (min << 8u) | (sec << 0u);
  rtcSetAlarm(&RTCD0, 0, &spec);
}

/**
 * @brief   Get current alarm.
 *
 * @param[out] day  day of the month in BCD format if 0x40 is not set: 0x01~0x31 (1~31);
 *                  day of the week  in BCD format if 0x40 is set: 0x40 | (0x01~0x07) (Monday~Sunday);
 *                  no meaningful if other values.
 * @param[out] hour hours in BCD format:   0x00~0x23 (0~23 hours, 24-hour format);
 *                  no meaningful if other values.
 * @param[out] min  minutes in BCD format: 0x00~0x59 (0~59 minutes)
 *                  no meaningful if other values.
 * @param[out] sec  seconds in BCD format: 0x00~0x59 (0~59 seconds)
 *                  no meaningful if other values.
 * 
 * @api
 */
void hs_rtc_getAlarm(uint8_t *day, uint8_t *hour, uint8_t *min, uint8_t *sec)
{
  RTCAlarm spec;

  rtcGetAlarm(&RTCD0, 0, &spec);
  if (day)
    *day = (spec.tv_datetime >> 24u) & 0xff;
  if (hour)
    *hour = (spec.tv_datetime >> 16u) & 0xff;
  if (min)
    *min = (spec.tv_datetime >> 8u) & 0xff;
  if (sec)
    *sec = (spec.tv_datetime >> 0u) & 0xff;
}

#endif /* HAL_USE_RTC */

/** @} */
