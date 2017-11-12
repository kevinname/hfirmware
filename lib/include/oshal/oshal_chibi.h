#ifndef __LIB_CHIBIOS_HAL__
#define __LIB_CHIBIOS_HAL__

#include "cmsis_os.h"

#define msleep(ms)      osDelay(ms);
#define curthread()     osThreadGetId()

osStatus oshalKernelInitialize(void);

__lib_static_inline osStatus oshalKernelStart(void)
{
  return osKernelStart();
}

/********************************************************************************************
 * thread
 ********************************************************************************************/
__lib_static_inline osThreadId oshalThreadCreate(const osThreadDef_t *thread_def, void *argument)
{
  return osThreadCreate(thread_def, argument);
}

__lib_static_inline osStatus oshalThreadTerminate(osThreadId thread_id)
{
  return osThreadTerminate(thread_id);
}

__lib_static_inline osStatus oshalThreadSetPriority(osThreadId thread_id, osPriority newprio)
{
  return osThreadSetPriority(thread_id, newprio);
}

/********************************************************************************************
 * virt-timer
 ********************************************************************************************/
osTimerId oshalTimerCreate(const osTimerDef_t *timer_def, os_timer_type type, void *argument);
osStatus  oshalTimerStart(osTimerId timer_id, uint32_t millisec);
osStatus  oshalTimerStop(osTimerId timer_id);
osStatus  oshalTimerDelete(osTimerId timer_id);

/********************************************************************************************
 * singal
 ********************************************************************************************/
__lib_static_inline int32_t oshalSignalSet(osThreadId thread_id, int32_t signals)
{
  return osSignalSet(thread_id, signals);
}

__lib_static_inline int32_t oshalSignalClear(osThreadId thread_id, int32_t signals)
{
  return osSignalClear(thread_id, signals);
}

__lib_static_inline osEvent oshalSignalWait(int32_t signals, uint32_t millisec)
{
  return osSignalWait(signals, millisec);
}

/********************************************************************************************
 * semphore
 ********************************************************************************************/
__lib_static_inline osSemaphoreId oshalSemaphoreCreate(const osSemaphoreDef_t *semaphore_def, int32_t count)
{
  return osSemaphoreCreate(semaphore_def, count);
}

__lib_static_inline int32_t oshalSemaphoreWait(osSemaphoreId semaphore_id, uint32_t millisec)
{
  return osSemaphoreWait(semaphore_id, millisec);
}

__lib_static_inline osStatus oshalSemaphoreRelease(osSemaphoreId semaphore_id)
{
  return osSemaphoreRelease(semaphore_id);
}

__lib_static_inline osStatus oshalSemaphoreDelete(osSemaphoreId semaphore_id)
{
  return osSemaphoreDelete(semaphore_id);
}

/********************************************************************************************
 * 
 ********************************************************************************************/
__lib_static_inline osMutexId oshalMutexCreate(const osMutexDef_t *mutex_def)
{
  return osMutexCreate(mutex_def);
}

__lib_static_inline osStatus oshalMutexWait(osMutexId mutex_id, uint32_t millisec)
{
  return osMutexWait(mutex_id, millisec);
}

__lib_static_inline osStatus oshalMutexRelease(osMutexId mutex_id)
{
  return osMutexRelease(mutex_id);
}

__lib_static_inline osStatus oshalMutexDelete(osMutexId mutex_id)
{
  return osMutexDelete(mutex_id);
}

/********************************************************************************************
 * message
 ********************************************************************************************/
osMessageQId oshalMessageCreate(osMessageQDef_t *queue_def, osThreadId thread_id);

__lib_static_inline osStatus oshalMessagePut(osMessageQId queue_id, uint32_t info, uint32_t millisec)
{
  return osMessagePut(queue_id, info, millisec);
}

__lib_static_inline osEvent oshalMessageGet(osMessageQId queue_id, uint32_t millisec)
{
  return osMessageGet(queue_id, millisec);
}

osStatus oshalMessageFree(osMessageQId queue_id);

void oshalMessageReset(osMessageQId queue_id);



#endif
