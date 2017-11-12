#include <string.h>
#include <stdlib.h> 
#include "lib.h"

typedef struct {
  BaseSequentialStream *chp;
  osMessageQId mess;
  osThreadId pThd;
}_message_arg_t;

static const char g_c1[] = "Hi, How are you?";
static const char g_c2[] = "Fine, Thanks!";

static void _messageTest_RxThd(void *arg)
{
  _message_arg_t *parg = (_message_arg_t *)arg;
  uint32_t info; 
  char *ptr;
  osEvent res;
  osStatus sts;

  chprintf(parg->chp, "[Rx]Waiting for a message......\r\n");
  res = oshalMessageGet(parg->mess, -1);
  if(res.status != osEventMessage) {
    chprintf(parg->chp, "[Rx]Get message error!\r\n");
  }
  else {
    ptr = (char *)res.value.v;
    chprintf(parg->chp, "[Rx]Get message: %s \r\n", ptr);
  }

  chprintf(parg->chp, "[Rx]Message Sending......\r\n");
  
  info = (uint32_t)g_c2;
  sts = oshalMessagePut(parg->mess, info, -1);
  if(sts != osOK) {
    chprintf(parg->chp, "[Rx]Send message error!\r\n");
  }

  oshalSignalSet(parg->pThd, 1);
  chprintf(parg->chp, "[Rx]Thread run over!\r\n");
}

static void _messageTest_TxThd(void *arg)
{
  _message_arg_t *parg = (_message_arg_t *)arg;
  uint32_t info; 
  char *ptr;
  osEvent res;
  osStatus sts;

  chprintf(parg->chp, "[Tx]Message Sending......\r\n");
  info = (uint32_t)g_c1;
  sts = oshalMessagePut(parg->mess, info, -1);
  if(sts != osOK) {
    chprintf(parg->chp, "[Tx]Send message error!\r\n");
  }

  chprintf(parg->chp, "[Tx]Waiting for a message......\r\n");
  res = oshalMessageGet(parg->mess, -1);
  if(res.status != osEventMessage) {
    chprintf(parg->chp, "[Tx]Get message error!\r\n");
  }
  else {
    ptr = (char *)res.value.v;
    chprintf(parg->chp, "[Tx]Get message: %s \r\n", ptr);
  }

  oshalSignalSet(parg->pThd, 2);
  chprintf(parg->chp, "[Tx]Thread run over......\r\n");
}

void cmd_messageTest(BaseSequentialStream *chp, int argc, char *argv[]) {

  osMessageQDef_t mess;
  _message_arg_t *parg;
  osThreadId thd1, thd2;
  osThreadDef_t thdDef1, thdDef2;

  (void)argv;
  if(argc != 0) {
    chprintf(chp, "Usage: message\r\n");
    return;
  }  
  
  parg = hs_malloc(sizeof(_message_arg_t), __MT_Z_GENERAL);
  if(parg == NULL) {
    chprintf(chp, "malloc memory error\r\n");
    return;
  }

  mess.queue_sz = 10 * sizeof(int);
  mess.item_sz  = sizeof(int);
  mess.items = hs_malloc(mess.queue_sz, __MT_Z_GENERAL);

  parg->chp  = chp;  
  parg->pThd = curthread();   
  parg->mess = oshalMessageCreate(&mess, NULL);
  if(parg->mess == NULL) {
    chprintf(chp, "Create message error\r\n");
    return;
  }  

  thdDef1.pthread   = (os_pthread)_messageTest_RxThd;
  thdDef1.stacksize = 512;
  thdDef1.tpriority = osPriorityNormal;

  thdDef2.pthread   = (os_pthread)_messageTest_TxThd;
  thdDef2.stacksize = 512;
  thdDef2.tpriority = osPriorityNormal;    
    
  thd1 = oshalThreadCreate(&thdDef1, parg);  
  thd2 = oshalThreadCreate(&thdDef2, parg);    

  chprintf(chp, "\r\n****** Waiting for message test ......\r\n\r\n");
  oshalSignalWait(3, -1);
    
  oshalThreadTerminate(thd1);
  oshalThreadTerminate(thd2);

  hs_free(mess.items);
  oshalMessageFree(parg->mess);
  
  hs_free(parg);
  chprintf(chp, "\r\n****** Message test over!\r\n\r\n");
}


