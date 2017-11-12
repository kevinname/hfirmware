/*
    drvhal - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    lib/drvhal/pmu/pmu.h
 * @brief   config pmu include file.
 * @details 
 *
 * @addtogroup  drvhal
 * @details 
 * @{
 */
#ifndef __LIB_PMU_H__
#define __LIB_PMU_H__

typedef enum
{
  PMU_STATUS_WORKING        = 0,
  PMU_STATUS_SLEEPING       ,
}hs_pmu_status_t;


#ifdef __cplusplus
extern "C" {
#endif

void hs_pmu_chipPrePowerOff(uint16_t msg, void *parg);
void hs_pmu_chipPowerOff(uint16_t msg, void *parg);
void hs_pmu_chipDeepSleep(uint16_t msg, void *parg);

void hs_pmu_cpuStatistic(void);
bool hs_pmu_isSleeping(void);
void hs_pmu_enSleep(uint8_t enble);
void hs_pmu_flushDisable(uint8_t u8Dis);

#ifdef __cplusplus
}
#endif


#endif
 /** @} */
