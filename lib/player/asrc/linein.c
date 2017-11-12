#include <lib.h>

#if HS_USE_AUX && HS_USE_PLAYER

static hs_linein_t * g_pstAux;

void _linein_thread(void* arg)
{
  hs_linein_t *pstLinein = (hs_linein_t *)arg;
  uint32_t u32RecLen;
  uint8_t *pu8Ptr;

  chRegSetThreadName("auxService");
  hs_printf("aux starting...\r\n");  
  
  while(1)
  {
    u32RecLen = audioRecGetDataBuffer(&pu8Ptr, 1200, -1);
    if(u32RecLen > 0)
      hs_ao_fetchData(pu8Ptr, u32RecLen);

    audioRecGetDataDone(pu8Ptr, u32RecLen);

    if(pstLinein->u32Start == 0)
      break;
  }
  
  hs_printf("aux stop...\r\n");  
}

hs_linein_t *_linein_create(void)
{
  hs_linein_t *pstLinein;
  hs_lineinpara_t *pstLineinPara;

  pstLinein = (hs_linein_t *)hs_malloc(sizeof(hs_linein_t), __MT_Z_GENERAL);
  if(!pstLinein)
    return NULL;

  pstLineinPara = &pstLinein->stPara;
  if(HS_CFG_OK != hs_cfg_getDataByIndex(HS_CFG_MISC_LINEIN, (uint8_t *)pstLineinPara, sizeof(hs_lineinpara_t)))
  {
    pstLineinPara->volume              = 1;    
  }

  return pstLinein;
}

void hs_linein_stop(hs_linein_t *pstLinein)
{
  if(!pstLinein || !pstLinein->u32Start)
    return ;

  pstLinein->u32Start = 0;  

  if(pstLinein->pstThd)
  {
    oshalThreadTerminate(pstLinein->pstThd);
    pstLinein->pstThd = NULL;
  }

  hs_ao_stop(pstLinein->pstAo); 
  audioRecordStop(); 
}

void hs_linein_start(hs_linein_t *pstLinein)
{
  osThreadDef_t stThdDef;
  
  if(!pstLinein)
    return ;

  if(pstLinein->u32Start) 
  {
    hs_linein_stop(pstLinein);
    return;
  }

  pstLinein->u32Start = 1;
  audioRecordStart(NULL, NULL);
  audioSetRecordSource(AUDIO_RECORD_LINEIN);
  audioRecordSetVolume(0);

  msleep(200);
  
  hs_ao_start(pstLinein->pstAo);   

  stThdDef.pthread   = (os_pthread)_linein_thread;
  stThdDef.stacksize = 512;
  stThdDef.tpriority = osPriorityNormal;     
  pstLinein->pstThd  = oshalThreadCreate(&stThdDef, pstLinein);    
}

void hs_linein_volumeInc(hs_linein_t *pstLinein)
{
  if(!pstLinein)
    return ;

  if(pstLinein->u32Start == 0)
    hs_linein_start(pstLinein);

  pstLinein->stPara.volume = hs_audio_volGet(AVOL_DEV_NOR) + pstLinein->stPara.volume_step;
  hs_ao_setVol(pstLinein->pstAo, pstLinein->stPara.volume);
}

void hs_linein_volumeDec(hs_linein_t *pstLinein)
{
  if(!pstLinein)
    return ;

  if(pstLinein->u32Start == 0)
    hs_linein_start(pstLinein);

  pstLinein->stPara.volume = hs_audio_volGet(AVOL_DEV_NOR) - pstLinein->stPara.volume_step;
  hs_ao_setVol(pstLinein->pstAo, pstLinein->stPara.volume);
}

hs_linein_t *hs_linein_create(hs_ao_t *pstAo)
{
  hs_linein_t *pstLinein;

  pstLinein = _linein_create();
  if(!pstLinein)
    return NULL;

  hs_pmu_flushDisable(1);
  pstLinein->pstAo = pstAo;

  g_pstAux = pstLinein;
  return pstLinein;
}

void hs_linein_destroy(hs_linein_t *pstLinein)
{
  hs_pmu_flushDisable(0);

  g_pstAux = NULL;
  if(pstLinein)
  {
    hs_linein_stop(pstLinein);
    hs_free(pstLinein);
  }
}

uint8_t hs_linein_isStarting(void)
{
  if(g_pstAux == NULL)
    return 0;

  return g_pstAux->u32Start;
}

#endif

