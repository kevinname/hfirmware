#include <string.h>
#include <stdlib.h> 
#include "lib.h"

typedef struct {
  BaseSequentialStream *chp;
  osPriority prio;
  osSemaphoreId sem_id;
  osThreadId pThd;
  uint32_t num;
  uint32_t test_cnt;
}_sem_arg_t;

static uint32_t g_cnt;

static void _semTest_Thd1(void *arg)
{
  _sem_arg_t *parg = (_sem_arg_t *)arg;
  osPriority prio = parg->prio + 64;
  uint32_t i;

  chprintf(parg->chp, "Thread1 start to running......\r\n");
  for(i=0; i<parg->num; i++){
    if(parg->test_cnt == 0){
      oshalSemaphoreWait(parg->sem_id, -1);
    }

    g_cnt++;
    chprintf(parg->chp, "+");
    osThreadSetPriority(curthread(), --prio);
    chprintf(parg->chp, "%dA ", g_cnt);
    if(prio == 1) prio = parg->prio + 64;

    if(parg->test_cnt == 0){
      oshalSemaphoreRelease(parg->sem_id);
    }
  }

  oshalSignalSet(parg->pThd, 1);

  if(parg->test_cnt == 0){
    oshalSemaphoreWait(parg->sem_id, -1);
  }
  chprintf(parg->chp, "\r\nThread1 run over......\r\n");
  if(parg->test_cnt == 0){
    oshalSemaphoreRelease(parg->sem_id);
  }
}

static void _semTest_Thd2(void *arg)
{
  _sem_arg_t *parg = (_sem_arg_t *)arg;
  osPriority prio = parg->prio + 64;
  uint32_t i;

  chprintf(parg->chp, "Thread2 start to running......\r\n\r\n");
  for(i=0; i<parg->num; i++){
    if(parg->test_cnt == 0){
      oshalSemaphoreWait(parg->sem_id, -1);
    }

    g_cnt--;
    chprintf(parg->chp, "-");
    osThreadSetPriority(curthread(), --prio);
    chprintf(parg->chp, "%dB ", g_cnt);
    if(prio == 1) prio = parg->prio + 64;

    if(parg->test_cnt == 0){
      oshalSemaphoreRelease(parg->sem_id);
    }
  }

  oshalSignalSet(parg->pThd, 2);
  if(parg->test_cnt == 0){
    oshalSemaphoreWait(parg->sem_id, -1);
  }
  chprintf(parg->chp, "\r\nThread2 run over......\r\n");
  if(parg->test_cnt == 0){
    oshalSemaphoreRelease(parg->sem_id);
  }
}

void cmd_semTest(BaseSequentialStream *chp, int argc, char *argv[]) {

  osSemaphoreDef_t semdef;
  _sem_arg_t *parg;
  osThreadId thd1, thd2;
  osThreadDef_t thdDef1, thdDef2;
  uint32_t i;

  if(argc > 1) {
    chprintf(chp, "Usage: sema [num]\r\n");
    return;
  }  
  
  parg = hs_malloc(sizeof(_sem_arg_t), __MT_Z_GENERAL);
  if(parg == NULL) {
    chprintf(chp, "malloc memory error\r\n");
    return;
  }

  if(argc == 1)
    parg->num = atoi(argv[0]);  
  else
    parg->num = 20;
  parg->chp  = chp;  
  parg->pThd = curthread();   

  thdDef1.pthread   = (os_pthread)_semTest_Thd1;
  thdDef1.stacksize = 512;
  thdDef1.tpriority = osPriorityRealtime;

  thdDef2.pthread   = (os_pthread)_semTest_Thd2;
  thdDef2.stacksize = 512;
  thdDef2.tpriority = osPriorityRealtime;

  for(i=0; i<2; i++) { 
    g_cnt = 0; 
    parg->test_cnt = i; 
    parg->sem_id = oshalSemaphoreCreate(&semdef, 1);
    parg->prio = osPriorityRealtime;

    chprintf(chp, "\r\n\r\n****** Waiting for semaphore test %d......\r\n", i);
    
    thd1 = oshalThreadCreate(&thdDef1, parg);  
    thd2 = oshalThreadCreate(&thdDef2, parg);    
    
    oshalSignalWait(3, -1);
    
    oshalThreadTerminate(thd1);
    oshalThreadTerminate(thd2);
    oshalSemaphoreDelete(parg->sem_id);
  } 
  
  hs_free(parg);
  chprintf(chp, "****** Semaphore test over!\r\n\r\n");
}


