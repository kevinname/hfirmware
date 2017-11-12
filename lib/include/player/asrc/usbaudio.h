#ifndef __PLAYER_USBAUDIO_H__
#define __PLAYER_USBAUDIO_H__

#if HAL_USE_USB_AUDIO

typedef struct
{
  int8_t volume;
  int8_t volume_step;

  int8_t reserve[2];
}hs_usbaudiopara_t;

typedef struct
{
  hs_usbaudiopara_t     stPara;
}hs_usbaudio_t;

void hs_usbaudio_volumeInc(hs_usbaudio_t *pstUsbaudio);
void hs_usbaudio_volumeDec(hs_usbaudio_t *pstUsbaudio);

hs_usbaudio_t *hs_usbaudio_create(void);
void hs_usbaudio_destroy(hs_usbaudio_t *pstUsbaudio);

#endif
  
#endif

