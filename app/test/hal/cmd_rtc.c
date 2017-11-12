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

#include <string.h>
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "chrtclib.h"
#include "stdlib.h"
#include "rtc_lld.h"
#include "lib.h"

/*===========================================================================*/
/* RTC driver test code.                                                     */
/*===========================================================================*/

#if HAL_USE_RTC

/* Manually reloaded test alarm period.*/
#define RTC_ALARMPERIOD   10

static BSEMAPHORE_DECL(alarm_sem, 0);
static RTCTime timestamp;

static void string_to_StructTm(const char *Str,  struct tm *tv_tim)
{
  if((NULL == Str) || (NULL == tv_tim))
    return;

  char ch[5];
 
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

/**
 * @brief     unix time covert to alarm BCD.
 *
 * @param[in] tv_tim    Pointer to unix time structure.
 * @param[in] pAlarm    Pointer to RTC alarm structure.
 * @param[in] mask      alarm mach mask 
 *
 * @api
 */
static void time_to_alarmBCD(struct tm *timp, RTCAlarm *pAlarm, uint32_t mask)
{
  uint32_t v = 0;
  
  pAlarm->tv_datetime = 0x00;
  
  v = timp->tm_mday;
  pAlarm->tv_datetime |= ((v / 10) << RTC_ALARM_DT_OFFSET) & RTC_ALRM_DT;
  pAlarm->tv_datetime |= ((v % 10) << RTC_ALARM_DU_OFFSET) & RTC_ALRM_DU;

  v = timp->tm_hour;
  pAlarm->tv_datetime |= ((v / 10) << RTC_ALARM_HT_OFFSET) & RTC_ALRM_HT;
  pAlarm->tv_datetime |= ((v % 10) << RTC_ALARM_HU_OFFSET) & RTC_ALRM_HU;

  v = timp->tm_min;
  pAlarm->tv_datetime |= ((v / 10) << RTC_ALARM_MT_OFFSET) & RTC_ALRM_MNT;
  pAlarm->tv_datetime |= ((v % 10) << RTC_ALARM_MU_OFFSET) & RTC_ALRM_MNU;

  v = timp->tm_sec;
  pAlarm->tv_datetime |= ((v / 10) << RTC_ALARM_ST_OFFSET) & RTC_ALRM_ST;
  pAlarm->tv_datetime |= ((v % 10) << RTC_ALARM_SU_OFFSET) & RTC_ALRM_SU; 

  pAlarm->tv_datetime |= mask;  
  
}

static void func_sleep(void){
#if defined(__arm__) || defined(__CC_ARM) || defined(__ICCARM__)
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
//  PWR->CR |= (PWR_CR_PDDS | PWR_CR_LPDS | PWR_CR_CSBF | PWR_CR_CWUF);
  RTCD0.id_rtc->ISR &= ~(RTC_ISR_ALRBF | RTC_ISR_ALRAF | RTC_ISR_WUTF | RTC_ISR_TAMP1F |
                RTC_ISR_TSOVF | RTC_ISR_TSF);
  __WFE();
  NVIC_EnableIRQ(IRQ_SYSTICK);
#endif
}

static void RTC_Callback(RTCDriver *rtcp, rtcevent_t event) {

  (void)rtcp;

  switch (event) {
  case RTC_EVENT_ALARMA:
//  beep_on();
  //  palSetPad(IOPORT1, GPIO1_LED2);
    break;
  case RTC_EVENT_ALARMB:
//  beep_on(); 
 //   palSetPad(IOPORT1, GPIO1_LED3);
    break;
  case RTC_EVENT_TIMSTAMP_OVERFLOW:
  //  palSetPad(IOPORT1, GPIO1_LED4);
    break;
  case RTC_EVENT_AUTO_WAKEUP:
    rtcSetPeriodicWakeup(&RTCD0, NULL);
    NVIC_EnableIRQ(IRQ_SYSTICK);
 //   palTogglePad(IOPORT1, GPIO1_LED1);
    break;
  case RTC_EVENT_TIMSTAMP:
    rtc_lld_get_timestamp(rtcp, &timestamp);
    RTCD0.id_rtc->CR &= ~(RTC_CR_TSE | RTC_CR_TSIE | RTC_CR_TSEDGE);
    RTCD0.id_rtc->TAFCR &= ~RTC_TAFCR_TSINSEL;
    chSysLockFromISR();
    chBSemSignalI(&alarm_sem);
    chSysUnlockFromISR();
    break;
  }
}

#endif /* HAL_USE_RTC */

static void _rtc_alarmed(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;
  (void)parg;
  uint8_t day, hour, min, sec;
  struct tm tm;
  static int count = 0;

  hs_rtc_getAlarm(&day, &hour, &min, &sec);
  hs_rtc_getTim(&tm);
  hs_printf("beep ON: %02x %02x:%02x:%02x\n", day, hour, min, sec);
  hs_printf("%s\n", asctime(&tm));

  count++;
  if (count == 1)
    hs_rtc_setAlarm(0x80, 0x80, 0x80, 0x45);
  else if (count == 2)
    hs_rtc_setAlarm(0x80, 0x80, 0x80, 0x01);
  if (count >= 3) {
    count = 0;
    hs_cfg_sysCancelListenMsg(HS_CFG_EVENT_RTC_ALARM, _rtc_alarmed);
  }
}

void cmd_rtc(BaseSequentialStream *chp, int argc, char *argv[]) {
#if HAL_USE_RTC
  struct tm timp;
  RTCAlarm alarmspec;
  
  if (!argc) {
      /* set rtc to 2014-07-30-23-00-00*/
      string_to_StructTm("20140730230000", &timp);
      rtcSetTimeTm(&RTCD0, &timp);

      chprintf(chp, "Warning: time_t size in %d-bit int, about %u years since y1970\r\n",
               sizeof(time_t)*8, (1u<<(sizeof(time_t)*8-1))/3600/24/365);
#ifndef RUN_RTL_IN_SERVER
      /* sleep 5s */
      chThdSleepSeconds(5);
#else
      /* sleep 1s */
      chThdSleepMilliseconds(1000);
#endif	    
      /* read rtc if 2014-07-30-23-00-05 ok */
      rtcGetTimeTm(&RTCD0, &timp);
        if (((timp.tm_year+1900) == 2014) && ((timp.tm_mon+1) == 7) && (timp.tm_mday == 30)
	   && (timp.tm_hour == 23) && (timp.tm_min == 0)) {
#ifndef RUN_RTL_IN_SERVER
		if ((timp.tm_sec > 3)& (timp.tm_sec < 10))
#else
		if ((timp.tm_sec > 0) && (timp.tm_sec < 3))
#endif
          {
			chprintf(chp, "rtc is pass. time is %s\r\n", asctime(&timp));
		} else{
			chprintf(chp, "rtc is failed. time is %s\r\n", asctime(&timp));	    
		}
	} else
		chprintf(chp, "rtc is failed. time is %s\r\n", asctime(&timp));	    
	       
        return;
   }

  if (strcmp(argv[0], "xosc") == 0) {
    /* switch to external Oscillator at 32768Hz on PB5 */
    palSetPadMode(IOPORT1, 5, PAL_MODE_ALTERNATE(5) | PAL_MODE_INPUT | PAL_MODE_DRIVE_CAP(3));
    HS_PMU->PMU_CLOCK_MUX |= 0x10000000;
    return;
  }
  if (strcmp(argv[0], "xtal") == 0) {
    /* switch to local Crystal at 32000Hz derived from 16MHz */
    HS_PMU->PMU_CLOCK_MUX &= ~0x10000000;
    return;
  }
   
  /* rtc calendar and time funtion */
  if (strcmp(argv[0], "drvhal") == 0) {
    uint8_t min;
    hs_rtc_getTim(&timp);
    hs_printf("%s\n", asctime(&timp));

    hs_rtc_setTim("20161122191700");
    hs_rtc_getTim(&timp);
    hs_printf("%s\n", asctime(&timp));

    hs_cfg_sysListenMsg(HS_CFG_EVENT_RTC_ALARM, _rtc_alarmed);
    min = timp.tm_min + 1;
    min = ((min / 10) << 4) | (min % 10); //toBCD
    hs_rtc_setAlarm(0x80, 0x80, min, 0x30);
    return;
  }

  if (strcmp(argv[0], "date") == 0){
    if((argc == 2) && strcmp(argv[1], "get") == 0) {
      rtcGetTimeTm(&RTCD0, &timp);
      chprintf(chp, "%u%s", mktime(&timp), " - unix time\r\n");
      chprintf(chp, "%s%s",asctime(&timp)," - formatted time string\r\n");
    }
    else if((argc == 3) && strcmp(argv[1], "set") == 0) {
      string_to_StructTm(argv[2], &timp);
      rtcSetTimeTm(&RTCD0, &timp);
    }
    return;
  }
  
  /* alarm funtion */
  if (strcmp(argv[0], "alarm") == 0){
    if(strcmp(argv[1], "get") == 0) {
      if(strcmp(argv[2], "A") == 0)
        rtcGetAlarm(&RTCD0, 1, &alarmspec);  //Alarm A
      else if(strcmp(argv[2], "B") == 0)
        rtcGetAlarm(&RTCD0, 0, &alarmspec);  //Alarm B
      else
        goto ERROR;
      
      chprintf(chp, "%D%s", alarmspec.tv_datetime, " - alarm in HS66xx internal format\r\n");
      return;   
    }
    else if(strcmp(argv[1], "set") == 0) {
      if(argc == 3) {              
        rtcGetTimeTm(&RTCD0, &timp);
        timp.tm_hour += RTC_ALARMPERIOD;
       
        /* covert [current time + 10h] to alarmBCD format & alarm mach by minute */
        time_to_alarmBCD(&timp, &alarmspec, RTC_ALRMBR_MSK1 |
                         RTC_ALRMAR_MSK3 | RTC_ALRMAR_MSK4);  

        if(strcmp(argv[2], "A") == 0) {
          rtcSetAlarm(&RTCD0, 1, &alarmspec);  //Alarm A
          while ((RTCD0.id_rtc->ISR & RTC_ISR_ALRAF) == 0);
          RTCD0.id_rtc->ISR &= ~RTC_ISR_ALRAF; 
          chprintf(chp, "Alarm A Interrupt\r\n");
          return;          
        }
        else if(strcmp(argv[2], "B") == 0) {
          rtcSetAlarm(&RTCD0, 0, &alarmspec);  //Alarm B
          while ((RTCD0.id_rtc->ISR & RTC_ISR_ALRBF) == 0); 
          RTCD0.id_rtc->ISR &= ~RTC_ISR_ALRBF; 
          chprintf(chp, "Alarm B Interrupt\r\n");
          return;
        }
        else
          goto ERROR;
      }
      else if(argc == 4) {
        RTCD0.callback = RTC_Callback; 
        
        string_to_StructTm(argv[3], &timp);
       
        /* covert the time to alarmBCD format. alarm mach by date & hour & minute */        
        time_to_alarmBCD(&timp, &alarmspec, RTC_ALRMBR_MSK1);
                               
        
        if(strcmp(argv[2], "A") == 0) {
          rtcSetAlarm(&RTCD0, 1, &alarmspec);  //Alarm A  
          return;
        }
        else if(strcmp(argv[2], "B") == 0) {
          rtcSetAlarm(&RTCD0, 0, &alarmspec);  //Alarm B
          return;
        }
        else
          goto ERROR;                
      }
      
    }
    else {
      goto ERROR;
    }
  }
  
  /* Wakeup funtion */
  if (strcmp(argv[0], "wakeup") == 0){
    RTCWakeup RTCWakup;
    if((argc == 2) && strcmp(argv[1], "get") == 0) {
        rtcGetPeriodicWakeup(&RTCD0, &RTCWakup);
        chprintf(chp, "%D%s", RTCWakup.wakeup," -- wakeup in HS66xx internal format\r\n");
      return;    
    }
    else if((argc == 3) && strcmp(argv[1], "set") == 0) {
      RTCD0.callback = RTC_Callback; 
      /* halt heartbeat, resume it in callback */
      NVIC_DisableIRQ(IRQ_SYSTICK);
      
      RTCWakup.wakeup = RTC_WAKEUP_CLK_DIV_16 << 16; /* RTC/16 source */
      RTCWakup.wakeup |= atoi(argv[2]) & 0xffff;
      rtcSetPeriodicWakeup(&RTCD0, &RTCWakup);  
      return;
    }
    else if((argc == 3) && strcmp(argv[1], "sleep") == 0) {
      RTCD0.callback = NULL;
      /* halt heartbeat, resume it after WFI */
      NVIC_DisableIRQ(IRQ_SYSTICK);
      
      RTCWakup.wakeup = RTC_WAKEUP_CLK_CK_SPRE_1Hz << 16; /* select 1 Hz clock source */
      RTCWakup.wakeup |= atoi(argv[2]) & 0xffff;
      rtcSetPeriodicWakeup(&RTCD0, &RTCWakup);  
      func_sleep();  
      return;
    }
    else {
      goto ERROR;
    }
  }
    
  /* timestamp funtion */
  if (strcmp(argv[0], "stamp") == 0){
      RTCD0.callback = RTC_Callback;  
      
      chBSemObjectInit(&alarm_sem, TRUE);
      if(strcmp(argv[1], "0") == 0)
        rtc_timestamp_init(&RTCD0, 0);  //AF1  rising edge
      else if(strcmp(argv[1], "1") == 0)
        rtc_timestamp_init(&RTCD0, 1);  //AF1  falling edge
      else if(strcmp(argv[1], "2") == 0)
        rtc_timestamp_init(&RTCD0, 2);  //AF2  rising edge
      else if(strcmp(argv[1], "3") == 0)
        rtc_timestamp_init(&RTCD0, 3);  //AF2  falling edge
      
      while (TRUE){
    
        /* Wait until alarm callback signaled semaphore.*/
        chBSemWait(&alarm_sem);
        chprintf(chp, "TSDR=%d TSTR=%d, TSSSR=%d\r\n", timestamp.tv_date,
                 timestamp.tv_time, timestamp.tv_msec);
        break;
      }
      return; 
  }
  
  /* digital calibration funtion */
  if (strcmp(argv[0], "calib") == 0){  
      RTCD0.id_rtc->CR |= (RTC_CR_COE | RTC_CR_COSEL );  //enable calibration output  RTC_CR_DCE

      if(strcmp(argv[1], "1") == 0){
        RTCD0.id_rtc->CALR = 0x000C004;
        return;
      }
      else if(strcmp(argv[1], "2") == 0){
        RTCD0.id_rtc->CALR = 0x0000C0f0;
        return;
      }
      else if(strcmp(argv[1], "3") == 0) {
        RTCD0.id_rtc->CALR = 0x0000A0ff;  
        return;
      }
      else 
        goto ERROR;      
  }
  
ERROR:
  
  chprintf(chp, "Usage: rtc [target] [options]...\r\n"); 
  chprintf(chp, "target:%s\r\n", 
                        "\tdate   --date test\r\n"
                        "\talarm  --alarm test\r\n"
                        "\twakeup --wakeup test\r\n"
                        "\tstamp  --timestamp test\r\n"
                        "\tcalib  --digital calibration test");
  chprintf(chp,"%s\r\n","Example: rtc date get  --get RTC current time\r\n"
        "\trtc alarm set 20141001153045 --set alarm at Oct 1 15:30:45 CST 2014");  
#endif                
}
