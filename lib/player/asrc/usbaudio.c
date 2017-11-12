#include <lib.h>

#if HAL_USE_USB_AUDIO && HS_USE_PLAYER

hs_usbaudio_t *_usbaudio_create(void)
{
  hs_usbaudio_t *pstUsbaudio;
  hs_usbaudiopara_t *pstUsbaudioPara;

  pstUsbaudio = (hs_usbaudio_t *)hs_malloc(sizeof(hs_usbaudio_t), __MT_Z_GENERAL);
  if(!pstUsbaudio)
    return NULL;

  pstUsbaudioPara = &pstUsbaudio->stPara;
  if(HS_CFG_OK != hs_cfg_getDataByIndex(HS_CFG_MISC_USBAUD, (uint8_t *)pstUsbaudioPara, sizeof(hs_usbaudiopara_t)))
  {
    pstUsbaudioPara->volume              = 0;
    pstUsbaudioPara->volume_step         = 6;
  }

  return pstUsbaudio;
}

void _usbaudio_storageClose(uint16_t u16Msg, void *parg)
{
  (void)u16Msg;
  uint32_t type = (uint32_t)parg;

  if(type == FATFS_MEMDEV_SD)
    hs_cfg_playerReq(HS_CFG_EVENT_PLAYER_DESTROY);
}

void hs_usbaudio_volumeInc(hs_usbaudio_t *pstUsbaudio)
{
  if(!pstUsbaudio)
    return ;

  pstUsbaudio->stPara.volume = hs_audio_volGet(AVOL_DEV_NOR) + pstUsbaudio->stPara.volume_step;
  hs_ao_setVol(NULL, pstUsbaudio->stPara.volume);
}

void hs_usbaudio_volumeDec(hs_usbaudio_t *pstUsbaudio)
{
  if(!pstUsbaudio)
    return ;

  pstUsbaudio->stPara.volume = hs_audio_volGet(AVOL_DEV_NOR) - pstUsbaudio->stPara.volume_step;
  hs_ao_setVol(NULL, pstUsbaudio->stPara.volume);
}

hs_usbaudio_t *hs_usbaudio_create(void)
{
  hs_usbaudio_t *pstUsbaudio;

  pstUsbaudio = _usbaudio_create();
  if(!pstUsbaudio)
    return NULL;
  
  if(0 != hs_usb_open(USB_DEVTYPE_AUDIO_STORAGE))
  {
    hs_usbaudio_destroy(pstUsbaudio);
    return NULL;
  }

  hs_pmu_flushDisable(1);
  return pstUsbaudio;
}

void hs_usbaudio_destroy(hs_usbaudio_t *pstUsbaudio)
{
  hs_pmu_flushDisable(0);
  if(pstUsbaudio)
  {
    hs_usb_close(USB_DEVTYPE_AUDIO_STORAGE);
    hs_free(pstUsbaudio);
  }
}

#endif

