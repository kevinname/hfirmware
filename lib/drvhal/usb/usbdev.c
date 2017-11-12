/*
    drvhal - Copyright (C) 2012~2015 HunterSun Technologies
                 pingping.wu@huntersun.com.cn rewrite
 */

/**
 * @file    usb/usbdev.c
 * @brief   usb device init file.
 * @details
 *
 * @addtogroup  drvhal
 * @details
 * @{
 */

#include "lib.h"
#include <string.h>

#if USB_ALL_DEVICE

typedef struct
{
  uint8_t     u8DHoldStatus;
  uint8_t     u8DLastStatus;
  uint8_t     u8DCnt;

  uint8_t     u8HHoldStatus;
  uint8_t     u8HLastStatus;
  uint8_t     u8HCnt;

  uint8_t     u8IsDevice;
}hs_usbpara_t;


#if HAL_USE_USB_SERIAL
extern hs_usbdev_t g_stSerialDev;
#endif

#if HAL_USE_USB_BULK_HS6200
extern hs_usbdev_t g_stHS6200Dev;
#endif

#if HAL_USE_USB_AUDIO || HAL_USE_USB_STORAGE
extern hs_usbdev_t g_stAudioStorageDev;
#endif

static hs_usbdev_t *g_pstUsbDev[] =
{
  #if HAL_USE_USB_SERIAL
  &g_stSerialDev,
  #else
  NULL,
  #endif

  #if HAL_USE_USB_AUDIO || HAL_USE_USB_STORAGE
  &g_stAudioStorageDev,
  #else
  NULL,
  #endif


  #if HAL_USE_USB_BULK_HS6200
  &g_stHS6200Dev,
  #else
  NULL,
  #endif

  NULL,
};

static const hs_padinfo_t g_stPadDevice[] = 
{
  {PA14, PAD_FUNC_GPIO,    CFG_PAD_DIR_INPUT, CFG_PAD_PULL_NO,   3},
  {PB0,  PAD_FUNC_GPIO,    CFG_PAD_DIR_INPUT, CFG_PAD_PULL_NO,   3},
  {PB1,  PAD_FUNC_SD_USB,  CFG_PAD_DIR_INPUT, CFG_PAD_PULL_UP,   3},
  {PB0,  PAD_FUNC_SD_USB,  CFG_PAD_DIR_INPUT, CFG_PAD_PULL_NO,   3},
  {PA14, PAD_FUNC_SD_USB,  CFG_PAD_DIR_INPUT, CFG_PAD_PULL_UP,   3},
};

static const hs_padinfo_t g_stPadHost[]   = 
{
  {PA14, PAD_FUNC_GPIO,    CFG_PAD_DIR_INPUT, CFG_PAD_PULL_NO,   3},
  {PB0,  PAD_FUNC_GPIO,    CFG_PAD_DIR_INPUT, CFG_PAD_PULL_NO,   3},
  {PB1,  PAD_FUNC_SD_USB,  CFG_PAD_DIR_INPUT, CFG_PAD_PULL_DOWN, 3},
  {PB0,  PAD_FUNC_SD_USB,  CFG_PAD_DIR_INPUT, CFG_PAD_PULL_DOWN, 3},
  {PA14, PAD_FUNC_SD_USB,  CFG_PAD_DIR_INPUT, CFG_PAD_PULL_DOWN, 3},
};

static const hs_padinfo_t g_stPadNone[]   = 
{
  {PA14, PAD_FUNC_GPIO,    CFG_PAD_DIR_INPUT, CFG_PAD_PULL_DOWN,   3},
  {PB0,  PAD_FUNC_GPIO,    CFG_PAD_DIR_INPUT, CFG_PAD_PULL_DOWN,   3},
};

static hs_usbpara_t g_stUsbPara;

static hs_usbdevtype_t g_eCurUsbDev = USB_DEVTYPE_NODEV;//USB_DEVTYPE_AUDIO_STORAGE;//USB_DEVTYPE_SERIAL;
static uint8_t g_u8UsbFirstPos = 0xff;

void hs_adc_clrBatStatus(void);
void _usb_setpad(const hs_padinfo_t *pstPad, int cnt)
{
  int i;

  for(i=0; i<cnt; i++)
    hs_pad_config(&pstPad[i]);
}

uint8_t _usb_chkHostStatus(void)
{
  uint8_t u8Status1, u8Status2;

  u8Status1 = palReadPad(IOPORT0, PA14);
  u8Status2 = palReadPad(IOPORT1, (PB0 - PB0));

  return (u8Status1 == u8Status2 ? 0 : 1);
}

void _usb_hostOpen(void)
{
  #if HAL_USE_USB_HOST_STORAGE
  _usb_setpad(g_stPadHost, sizeof(g_stPadHost)/sizeof(hs_padinfo_t));
  usbSetPowerSessMode(&USBD1, USB_POWER_CONTROL_BY_REG);
  usb_h_start(&USBHOST0);
  #endif
}

void _usb_hostClose(void)
{
  #if HAL_USE_USB_HOST_STORAGE
  usb_h_stop(&USBHOST0);
  _usb_setpad(g_stPadNone, sizeof(g_stPadNone)/sizeof(hs_padinfo_t));
  #endif
}
void _usb_reset(USBDriver *usbp);

void _usb_scanDevicePlug(void)
{
  hs_usbpara_t *pstUsbDev = &g_stUsbPara;
  const hs_drvhal_cfg_t *pstDrvCfg = hs_boardGetDrvCfg();
  uint8_t usbStatus = usbChkPlugin() ? 1 : 0;

  if((usbStatus == pstUsbDev->u8DHoldStatus)
    && (usbStatus == pstUsbDev->u8DLastStatus))
    return ;
  
  if(usbStatus == pstUsbDev->u8DHoldStatus)
  {
    pstUsbDev->u8DCnt ++;
    if(pstUsbDev->u8DCnt >= 3)
    {
      pstUsbDev->u8DLastStatus = usbStatus;
      hs_adc_clrBatStatus();
      if(usbStatus)
      {
        usbChargerEn();
        hs_cfg_sysReqPerip(HS_CFG_EVENT_BATTERY_CHARGING);

        if(pstDrvCfg->u8UsbAudioEn)
        {
          _usb_setpad(g_stPadDevice, sizeof(g_stPadDevice)/sizeof(hs_padinfo_t));
          usbEnableData();
        }
      }
      else
      {
        hs_cfg_sysReqPerip(HS_CFG_EVENT_BATTERY_CHARGEOUT);

        if(pstDrvCfg->u8UsbAudioEn)
          _usb_setpad(g_stPadNone, sizeof(g_stPadNone)/sizeof(hs_padinfo_t));
      }
    }
  }
  else
  {
    pstUsbDev->u8DCnt = 0;
    pstUsbDev->u8DHoldStatus = usbStatus;
  }
}

void _usb_scanHostPlug(void)
{
  hs_usbpara_t *pstUsbHost = &g_stUsbPara;
  const hs_drvhal_cfg_t *pstDrvCfg = hs_boardGetDrvCfg();
  uint8_t usbStatus = _usb_chkHostStatus();

  if(g_u8UsbFirstPos == 0xff)
    g_u8UsbFirstPos = usbStatus;

  if((usbStatus == pstUsbHost->u8HHoldStatus)
    && (usbStatus == pstUsbHost->u8HLastStatus))
    return ;
  
  if(usbStatus == pstUsbHost->u8HHoldStatus)
  {
    pstUsbHost->u8HCnt ++;
    if(pstUsbHost->u8HCnt >= 10)
    {
      pstUsbHost->u8HLastStatus = usbStatus;   

      hs_printf("usb status:%d, charging:%d\r\n", usbStatus, pstUsbHost->u8DLastStatus);

      if(pstDrvCfg->u8UsbAudioEn)
      {
        if(usbStatus)
        {
          if(pstUsbHost->u8DLastStatus == 0)
          {
            pstUsbHost->u8IsDevice = 0;
            _usb_hostOpen();
            hs_pmu_enSleep(0);

            if(g_u8UsbFirstPos == 0)
              hs_cfg_sysReqPerip((HS_CFG_EVENT_USBHOST_PLUGOUT - usbStatus) | FAST_EVENT_MASK);
            else
              hs_cfg_sysReqPerip(HS_CFG_EVENT_USBHOST_PLUGOUT - usbStatus);
          }
          else
          {
            pstUsbHost->u8IsDevice = 1;
            hs_cfg_sysReqPerip(HS_CFG_EVENT_USBDEVICE_PLUGIN);
          }
        }
        else
        {
          if(pstUsbHost->u8IsDevice == 0)
          {
            _usb_hostClose();
            hs_pmu_enSleep(1);
            hs_cfg_sysReqPerip(HS_CFG_EVENT_USBHOST_PLUGOUT - usbStatus);
          }
          else
          {
            hs_cfg_sysReqPerip(HS_CFG_EVENT_USBDEVICE_PLUGOUT);
          }
        }
      }
      else
      {
        if(usbStatus)
        {
          pstUsbHost->u8IsDevice = 0;
          _usb_hostOpen();
          hs_pmu_enSleep(0);
          hs_cfg_sysReqPerip(HS_CFG_EVENT_USBHOST_PLUGOUT - usbStatus);
        }
        else
        {
          _usb_hostClose();
          hs_pmu_enSleep(1);
          hs_cfg_sysReqPerip(HS_CFG_EVENT_USBHOST_PLUGOUT - usbStatus);
        }
      }
    }
  }
  else
  {
    pstUsbHost->u8HCnt = 0;
    pstUsbHost->u8HHoldStatus = usbStatus;
  }
}

void _usb_close(hs_usbdevtype_t dev)
{ 
  if((g_pstUsbDev[dev] != NULL)
      && (g_pstUsbDev[dev]->pDevUninit != NULL))
    g_pstUsbDev[dev]->pDevUninit();       

  usbDisconnectBus(&USBD1);
  usbStop(&USBD1);
}

int hs_usb_open(hs_usbdevtype_t dev)
{
  USBDriver *usbp = &USBD1;
  int32_t ret = 1;

  if(dev >= USB_DEVTYPE_HOST_DISK)
    return -1;

  _usb_setpad(g_stPadDevice, sizeof(g_stPadDevice)/sizeof(hs_padinfo_t));
  usbSetPowerSessMode(usbp, USB_POWER_CONTROL_BY_REG);

  hs_usb_close(g_eCurUsbDev);
  usbStart(usbp, g_pstUsbDev[dev]->pstConfig);
  if((g_pstUsbDev[dev] != NULL)
      && (g_pstUsbDev[dev]->pDevInit != NULL))
  {
    ret = g_pstUsbDev[dev]->pDevInit(g_pstUsbDev[dev]->pArg);
    if(ret != 0)
    {
      usbStop(usbp);
      return -1;
    }
  }

  usbConnectBus(usbp);
  g_eCurUsbDev = dev;
  return 0;
}

int hs_usb_close(hs_usbdevtype_t dev)
{ 
  if(dev >= USB_DEVTYPE_HOST_DISK)
    return 0;
  
  _usb_close(dev);

  if(g_eCurUsbDev == dev)
    g_eCurUsbDev = USB_DEVTYPE_NODEV;
  return 0;
}

void hs_usb_serialOpen(uint16_t u16Idx, void *parg)
{
  (void)u16Idx;
  (void)parg;

  hs_usb_open(USB_DEVTYPE_SERIAL);
}

void hs_usb_serialClose(uint16_t u16Idx, void *parg)
{
  (void)u16Idx;
  (void)parg;

  hs_usb_close(USB_DEVTYPE_SERIAL);
}

static uint8_t g_u8UsbRdySend;
void hs_usb_scanDisk(void)
{
  #if HAL_USE_USB_HOST_STORAGE
  if(usbDiskReady(&USBHOST0))
  {
    if(hs_fatfs_isMount(FATFS_MEMDEV_UDISK) || (g_u8UsbRdySend == 1))
      return ;

    g_u8UsbRdySend = 1;
    //hs_cfg_sysReqPeripArg(HS_CFG_EVENT_MEMDEV_IN, (void *)FATFS_MEMDEV_UDISK);
    hs_cfg_sysSendMsg(HS_CFG_MODULE_PERIP, HS_CFG_SYS_STATUS, HS_CFG_STATUS_MEMDEV_IN, (void *)FATFS_MEMDEV_UDISK);
  }
  else
  {
    g_u8UsbRdySend = 0;
    if(!hs_fatfs_isMount(FATFS_MEMDEV_UDISK))
      return ;
    
    //hs_cfg_sysReqPeripArg(HS_CFG_EVENT_MEMDEV_OUT, (void *)FATFS_MEMDEV_UDISK);
    hs_cfg_sysSendMsg(HS_CFG_MODULE_PERIP, HS_CFG_SYS_STATUS, HS_CFG_STATUS_MEMDEV_OUT, (void *)FATFS_MEMDEV_UDISK);
  }
  #endif
}

void hs_usb_scanPlug(void)
{
  _usb_scanDevicePlug();
  _usb_scanHostPlug();
}

#endif



