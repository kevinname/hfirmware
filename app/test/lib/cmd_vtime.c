#include <string.h>
#include <stdlib.h> 
#include "lib.h"

typedef struct {
  BaseSequentialStream *chp;
  osThreadId pThd;
  uint32_t cnt;
}_vtimer_arg_t;

static uint32_t g_cnt;

void _vtimer_cb(void const *arg){
  _vtimer_arg_t *parg = (_vtimer_arg_t *)arg;

  g_cnt ++;
  chprintf(parg->chp, "[%04d]Thread Timer coming!\r\n", g_cnt);  
  if(g_cnt >= parg->cnt)
    oshalSignalSet(parg->pThd, 1);
}

void cmd_vtimer(BaseSequentialStream *chp, int argc, char *argv[]) {

  osTimerDef_t stTmDef;
  osTimerId pstTimer;
  uint32_t ms;
  os_timer_type type;
  osStatus sts;
  _vtimer_arg_t *parg;

  if((argc != 2) && (argc != 3)) {
    chprintf(chp, "Usage: vtimer type ms [cnt]\r\n\ttype: 0-once 1-periodic\r\n");
    return;
  }

  type = atoi(argv[0]); 
  ms   = atoi(argv[1]); 
  
  parg = hs_malloc(sizeof(_vtimer_arg_t), __MT_Z_GENERAL);
  if(type == 0)
    parg->cnt = 1;
  else
  {    
    if(argc == 3)
      parg->cnt = atoi(argv[2]);  
    else
      parg->cnt = 20;
  }
  
  parg->chp  = chp;
  parg->pThd = curthread();
  g_cnt = 0;  

  stTmDef.ptimer = _vtimer_cb;
  pstTimer = oshalTimerCreate(&stTmDef, type, (void *)parg);
  sts = oshalTimerStart(pstTimer, ms);
  if(sts != osOK){
    chprintf(chp, "Start virt-timer error!\r\n");
    return;
  }

  chprintf(chp, "Waiting for virt-timer ......\r\n");
  oshalSignalWait(0, -1);
  oshalTimerDelete(pstTimer);
  hs_free(parg);

  chprintf(chp, "Virt-timer test over!\r\n\r\n");
}


