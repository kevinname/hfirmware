#ifndef __LIB_DRVHAL_H__
#define __LIB_DRVHAL_H__

#include "pad.h"
#include "pmu.h"
#include "lib_pwm.h"
#include "lib_adc.h"
#include "lib_sd.h"
#include "lib_audio.h"
#include "usbdev.h"
#include "chrtclib.h"
#include "lib_i2c.h"

typedef struct
{
  uint8_t     u8PdType;         /* when no power, 0-really power-down 1-deep sleep instead of power-down */
  uint8_t     u8PowerOnMode;    

  uint8_t     u8SdDetPin;
  uint8_t     u8SdDetLvl;       /* active level */
  
  uint8_t     u8AuxDetPin;
  uint8_t     u8AuxDetLvl;      /* active level */  

  uint8_t     u8AdcKeyPin;
  uint8_t     u8AdcKeyPinEx;
  
  uint8_t     u8SdEnable;       /* 0-disable auto init sd-card */
  uint8_t     u8SdDrvCap;       /* 0~3 */
  
  uint8_t     u8UsbEnable;      /* 0-all usb device disable in sdk, include host and device */
  uint8_t     u8UsbAudioEn;     /* 0-disable usb-audio, if u8UsbEnable == 1 */  

  uint16_t    u16PdDelay;       /* unit: s; the time to power down when reporting low-power */

  hs_adcgain_t  eAdcKeyGain;
  float       fAdcKeyRange;  

  float       fBatFullVolt;
  float       fBatEmptyVolt;

  float       fTempMaxAlert;
  float       fTempMinAlert;
}hs_drvhal_cfg_t;

#ifdef __cplusplus
extern "C" {
#endif

int  hs_drvhal_init(void);
void hs_drvhal_uninit(void);

const hs_drvhal_cfg_t *hs_boardGetDrvCfg(void);


#ifdef __cplusplus
}
#endif

#endif
 /** @} */
