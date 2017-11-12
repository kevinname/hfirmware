/*
    bootloader - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    config/config.h
 * @brief   config include file.
 * @details 
 *
 * @addtogroup  config
 * @details 
 * @{
 */

#ifndef __CONFIG_DATA_H__
#define __CONFIG_DATA_H__

/**
 * @brief config data index define.
 */
typedef enum 
{
  /* wupp */
  HS_CFG_INFO                       = 0x00, /*!< 0. the config data info index          */
  HS_CFG_BAW_AREAD                  ,       /*!< 1. backup write area                   */
  HS_CFG_RESERVE                    ,       /*!< 2. Reserve                             */

  HS_CFG_SYS_INFO                   ,       /*!< 3. system running manage info          */
  HS_CFG_SYS_RESERVE                ,       /*!< 4. Reserve                             */
  HS_CFG_SYS_BUTTON_IDENTIFY        ,       /*!< 5. button event identify               */
  HS_CFG_SYS_LED_ACTION             ,       /*!< 6. led action                          */
  HS_CFG_SYS_BUTTON_BIND_EVENT      ,       /*!< 7. the config of button bind event     */
  HS_CFG_SYS_EVENT_BIND_LED         ,       /*!< 8. the config of event/status bind     */

  HS_CFG_SYS_RESERVE1               ,       /*!< 9. RESERVE                             */

  HS_CFG_TONE_INFO                  ,       /*!< a. tone info                           */
  HS_CFG_SOUND_INFO                 ,       /*!< b. sound info                          */
  HS_CFG_EVENT_BIND_SOUND           ,       /*!< c. sound bind event                    */

  HS_CFG_RING_INFO                  ,       /*!< d. ring info                           */
  HS_CFG_RING_SPACE_BASE            ,       /*!< e. ring space base                     */
  
  HS_CFG_PAD_CONFIG                 ,       /*!< f. pin mux config                      */

  /* Classic: chenzy */
  HS_CFG_CLASSIC_DEV                = 0x10, /*!< Class BT         */
  HS_CFG_CLASSIC_PROFILE            ,       /*!<                  */
  HS_CFG_CLASSIC_ATTR               ,       /*!<                  */
  HS_CFG_CLASSIC_ADVANCED           ,       /*!<                  */
  HS_CFG_CLASSIC_ACTION             ,       /*!<                  */
  HS_CFG_CLASSIC_PAIRINFO           ,       /*!<                  */
  HS_CFG_CLASSIC_HID                ,       /*!<                  */
  HS_CFG_CLASSIC_HID_INFO           ,

  /* BLE: liq */
  HS_CFG_BLE_COMMON                 = 0x18, /*!< Low Energy BT    */
  HS_CFG_BLE_KEY                    ,       /*!< BLE key          */
  HS_CFG_BLE_PROFILE                ,       /*!<                  */
  HS_CFG_BLE_BRIDGE                 ,       /*!<                  */
  HS_CFG_BLE_DATABASE               ,       /*!<                  */

  /* Baseband & Radio: luw, yaoyf */
  HS_CFG_BT_HC                      = 0x20, /*!< BT Baseband + Host Controller */
  HS_CFG_BT_RF                      ,       /*!<                  */

  /* Production: luw, lihw, jiangp */
  HS_CFG_PROD_TYPE                  = 0x30, /*!< Production Type  */

  HS_CFG_PROD_CONTROLLER            ,       /*!< Host Controller  */
  HS_CFG_PROD_AUDIO                 ,       /*!< audio paramter  */
  HS_CFG_PROD_AUDIO_AEC             ,       /*!< alg: AEC         */
  HS_CFG_PROD_AUDIO_NS              ,       /*!< alg: NS          */
  HS_CFG_PROD_AUDIO_LPC             ,       /*!< alg: LPC         */

  /* Misc: lihw, jiangp, chenzy */
  HS_CFG_MISC_MP3                   = 0x38, /*!< MP3 Player       */
  HS_CFG_MISC_USB                   = 0x39, /*!< 0~3byte, USB Host & Device; 4~7 byte, USB customized VID/PID*/

  HS_CFG_MISC_CLK , 
  HS_CFG_MISC_PMU , 
  HS_CFG_MISC_ADC ,
  HS_CFG_MISC_FM                    ,
  HS_CFG_MISC_UART                  ,       /*!< UART settings for communication port */
  HS_CFG_MISC_SDC                   ,
  HS_CFG_MISC_ANA                   ,       /*!< 320 bits regs in ANALOGUE of SPI */
  HS_CFG_MISC_REGS                  ,       /*!< 10 regs of AHB */
  HS_CFG_MISC_LINEIN                ,
  HS_CFG_MISC_USBAUD                ,

  /* Infrared_Remote_Control   Dou.Yuntao*/
  HS_CFG_SYS_NEC_BIND_EVENT         = 0x45,

  HS_CFG_RECORDER_INFO              = 0x50,       /*!< 0x50  */
  HS_CFG_SPEAKER_INFO               ,

  HS_CFG_RESERVED                   = 0x7F,

  HS_CFG_MAX                        ,       /*!< max index of config data              */
}hs_cfg_index_t;

/**
 * @brief message of status or event generated when system running.
 */
typedef enum
{
  HS_CFG_SYS_EVENT      = 1,
  HS_CFG_SYS_STATUS     ,
}hs_cfg_mess_type_t;

/**
 * @brief module define.
 */
typedef enum
{
  HS_CFG_MODULE_DOWN                  = 0,          /*!< 0. bootloader download                     */
  HS_CFG_MODULE_SYS                   ,             /*!< 1. system                                  */
  HS_CFG_MODULE_PERIP                 ,             /*!< 2. peripheral                              */
  HS_CFG_MODULE_BLE                   ,             /*!< 3. ble                                     */
  HS_CFG_MODULE_BT_CLASSIC            ,             /*!< 4. bluetooth classic                       */
  HS_CFG_MODULE_PLAYER                ,             /*!< 5. player                                  */

  HS_CFG_MODULE_USER1                 = 8,
  HS_CFG_MODULE_USER2                 ,
  HS_CFG_MODULE_USER3                 ,
  HS_CFG_MODULE_USER4                 ,
  HS_CFG_MODULE_USER5                 ,

  HS_CFG_MODULE_TEST                  = 0x10,       /*!< 16. test module                            */  
  HS_CFG_MODULE_NUM                                 /*!< the end                                    */
}hs_cfg_mod_t;


/**
 * @brief event enum here.
 */
typedef enum 
{
  HS_CFG_EVENT_NONE                                 = 0x00,         /*!< event message base start           */  

  HS_CFG_EVENT_BT_HID_START                         = 0x0E,         /*!< 0x0E. bt HID start                 */
  HS_CFG_EVENT_BT_HID_STOP                          ,               /*!< 0x0F. bt HID stop                  */
  HS_CFG_EVENT_BT_AUDIO_START                       = 0x10,         /*!< 0x10. bt audio start               */
  HS_CFG_EVENT_BT_AUDIO_STOP                        ,               /*!< 0x11. bt audio stop                */
  
  HS_CFG_EVENT_PMU_POWERON                          ,               /*!< 0x12. power on                     */
  HS_CFG_EVENT_PMU_POWEROFF                         ,               /*!< 0x13. power off                    */
  HS_CFG_EVENT_PMU_DEEPSLEEP                        ,               /*!< 0x14. deep sleep                   */

  HS_CFG_EVENT_RECORDER_SW                          ,               /*!< 0x15. audio recorder start or stop       */
  HS_CFG_EVENT_PRE_POWEROFF                         ,               /*!< 0x16. prepare works before power off     */
  HS_CFG_EVENT_SPEAKER_SW                           ,               /*!< 0x17. speaker start or stop              */

  HS_CFG_EVENT_BATTERY_FULL                         = 0x18,         /*!< 0x18. battery is full                  */
  HS_CFG_EVENT_BATTERY_NEARFULL                     ,               /*!< 0x19. battery near full                */
  HS_CFG_EVENT_BATTERY_HALFFULL                     ,               /*!< 0x1A. battery half full                */
  HS_CFG_EVENT_BATTERY_NEAREMPTY                    ,               /*!< 0x1B. battery near empty / low alert   */
  HS_CFG_EVENT_BATTERY_CHANGED                      ,               /*!< 0x1C. battery volt changing            */
  
  HS_CFG_EVENT_BATTERY_CHARGING                     ,               /*!< 0x1D. battery charging                 */
  HS_CFG_EVENT_BATTERY_CHARGEOUT                    ,               /*!< 0x1E. battery charging                 */

  HS_CFG_EVENT_TEMPERATURE_HIGHALERT                ,               /*!< 0x1F. high temperature alert           */
  HS_CFG_EVENT_TEMPERATURE_LOWALERT                 ,               /*!< 0x20. low temperature alert            */

  HS_CFG_EVENT_RTC_ALARM                            ,               /*!< 0x21, rtc is alarmed                   */

  // bluetooth event
  HS_CFG_EVENT_BT_CONTROL_CMD                       = 0x30,         /*!< 0x30, unified bt events            */
  
  /* Bluetooth - link */
  HS_CFG_EVENT_BT_PAIRABLE                          = 0x40,         /*!< 0x40. ready for pair                  */
  HS_CFG_EVNET_BT_PAIRED                            ,               /*!< 0x41. paired                            */
  HS_CFG_EVENT_BT_GAP_CNNNECTED                     ,               /*!< 0x42. gap connected                 */
  HS_CFG_EVENT_BT_CONNECTED                         ,               /*!< 0x43. connected                       */
  HS_CFG_EVENT_BT_DISCONNECTED                      ,               /*!< 0x44. disconnected                   */
  HS_CFG_EVENT_BT_LOST_LINK_ALERT                   ,               /*!< 0x45. lost link alert                  */
  HS_CFG_EVENT_BT_DISCOVERY                         ,               /*!< 0x46. disvover                         */
  HS_CFG_EVENT_BT_HID_PARIABLE                      ,               /*!< 0x47. hid ready for pair               */
  HS_CFG_EVENT_BT_HID_CONNECTED                     ,               /*!< 0x48. hid connected                    */
  HS_CFG_EVENT_BT_HID_DISCONNECTED                  ,               /*!< 0x49. hid disconnected                 */
  
  /* Bluetooth - audio */
  HS_CFG_EVENT_BT_RING                              = 0x50,         /*!< 0x50. Ring                            */
  HS_CFG_EVENT_BT_CALL_REJECTED                     ,               /*!< 0x51. call rejected                   */
  HS_CFG_EVENT_BT_CALL_ENDED                        ,               /*!< 0x52. call ended                      */
  HS_CFG_EVENT_BT_CALL_ACTIVE                       ,               /*!< 0x53. call active                     */
  HS_CFG_EVENT_BT_CALL_HOLD                         ,               /*!< 0x54. call hold                       */
  HS_CFG_EVENT_BT_MUSIC_STREAMING                   ,               /*!< 0x55. music is streaming              */
  HS_CFG_EVENT_BT_MUSIC_PAUSED                      ,               /*!< 0x56. music is paused                 */
  HS_CFG_EVENT_BT_IN_BAND_RING                      ,               /*!< 0x57. incoming call with in band ring */
  HS_CFG_EVENT_BT_HID_KEY                           ,               /*!< 0x58, send hid key                    */
  HS_CFG_EVENT_BT_INCOMING_CALL                     ,               /*!< 0x59. incoming call                   */
  HS_CFG_EVENT_BT_SCO_STATUS                        ,               /*!< 0x5a. sco connect/disconnect          */
  HS_CFG_EVENT_BT_REMOTE_VOL                        ,               /*!< 0x5b. remote vol change               */
  HS_CFG_EVENT_BT_REDIAL                            ,               /*!< 0x5c. hfp redial                      */

  /* Bluetooth - Misc */
  HS_CFG_EVENT_BT_LOW_POWER                         = 0x60,         /*!<. low power                       */

  HS_CFG_EVENT_USB_SERIALRDY                        = 0x80,
  HS_CFG_EVENT_USB_SERIALCLS                        ,
  HS_CFG_EVENT_USB_SERIALSTART                      ,  
  HS_CFG_EVENT_USB_SERIALCLOSE                      ,

  HS_CFG_EVENT_USB_STORAGERDY                       ,
  HS_CFG_EVENT_USB_STORAGEOUT                       ,

  HS_CFG_EVENT_USBDEVICE_PLUGIN                     = 0x86,         /*!< 0x86. usb-device jack plug in          */
  HS_CFG_EVENT_USBDEVICE_PLUGOUT                    ,               /*!< 0x87. usb-device jack plug out         */

  HS_CFG_EVENT_JACK_PLUGIN                          ,               /*!< 0x88. aux jack plug in          */
  HS_CFG_EVENT_JACK_PLUGOUT                         ,               /*!< 0x89. aux jack plug out         */

  HS_CFG_EVENT_USBHOST_PLUGIN                       ,               /*!< 0x8A. usb-host jack plug in          */
  HS_CFG_EVENT_USBHOST_PLUGOUT                      ,               /*!< 0x8B. usb-host jack plug out         */

  HS_CFG_EVENT_MEMDEV_IN                            ,               /*!< 0x8C. memory device plug in     */
  HS_CFG_EVENT_MEMDEV_OUT                           ,               /*!< 0x8D. memory device plug out    */  
  
  HS_CFG_EVENT_VOL_MAXALARM                         ,               /*!< 0x8E. volume max alarm     */
  HS_CFG_EVENT_VOL_MINALARM                         ,               /*!< 0x8F. volume min alarm    */  
    
  HS_CFG_EVENT_FM_SEARCH_BEGIN                      = 0x90,         /*!< 0x90. fm search begin                 */
  HS_CFG_EVENT_FM_SEARCHING                         ,               /*!< 0x91. fm search in progress           */
  HS_CFG_EVENT_FM_SEARCH_END                        ,               /*!< 0x92. fm search end                   */
  HS_CFG_EVENT_FM_FREQ_IND                          ,               /*!< 0x93. fm current frequency indicator  */
  HS_CFG_EVENT_FM_CHAN_IND                          ,               /*!< 0x94. fm current channel indicator    */

  HS_CFG_EVENT_INPUT_DIG                            ,               /*!< 0x95. a digit input    */

  HS_CFG_EVENT_PLAYER_DESTROY                       = 0xD0,         /*!< 0xd0. player destroy */
  HS_CFG_EVENT_PLAYER_MP3CREATE                     ,               /*!< 0xd1. music start */
  HS_CFG_EVENT_PLAYER_FMCREATE                      ,               /*!< 0xd2. fm start */
  HS_CFG_EVENT_PLAYER_USBAUDCREATE                  ,               /*!< 0xd3. usb audio start */
  HS_CFG_EVENT_PLAYER_AUXCREATE                     ,               /*!< 0xd4. line-in start */
  
  HS_CFG_EVENT_PLAYER_START                         = 0xD6,         /*!< 0xd6. player start or pause */
  HS_CFG_EVENT_PLAYER_STOP                          ,               /*!< 0xd7. player stop */
  HS_CFG_EVENT_PLAYER_MUTE                          ,               /*!< 0xd8. player mute or unmute */
  HS_CFG_EVENT_PLAYER_NEXT                          ,               /*!< 0xd9. player next item */
  HS_CFG_EVENT_PLAYER_PREV                          ,               /*!< 0xda. player prev item */
  HS_CFG_EVENT_PLAYER_VOLINC                        ,               /*!< 0xdb. player increase volume */
  HS_CFG_EVENT_PLAYER_VOLDEC                        ,               /*!< 0xdc. player decrease volume */
  HS_CFG_EVENT_PLAYER_FREQINC                       ,               /*!< 0xdd. player increase freqency for fm */
  HS_CFG_EVENT_PLAYER_FREQDEC                       ,               /*!< 0xde. player increase freqency for fm */
  HS_CFG_EVENT_PLAYER_FUNCSET                       ,               /*!< 0xdf. set the function of player */

  HS_CFG_EVENT_PLAYER_MUSIC_SD                      ,               /*!< 0xe0. player sd mode */
  HS_CFG_EVENT_PLAYER_MUSIC_UDISK                   ,               /*!< 0xe1. player udisk mode */

  HS_CFG_EVENT_PLAYER_VOLINCBIG                     ,               /*!< 0xe2. player increase volume */
  HS_CFG_EVENT_PLAYER_VOLDECBIG                     ,               /*!< 0xe3. player decrease volume */

  HS_CFG_EVENT_TEST_0                               = 0xF0,         /*!< 0xf0. test event             */
  HS_CFG_EVENT_TEST_1                               ,
  
  HS_CFG_EVENT_USER_BEGAN                           = 0x100,
  //......  

  HS_CFG_EVENT_USER_MODE_MUSIC                      = 0x380,
  HS_CFG_EVENT_USER_MODE_FM                         , 
  HS_CFG_EVENT_USER_MODE_AUX                        ,  
  
  HS_CFG_EVENT_USER_END                             = 0x3FF,
}hs_cfg_event_type_t;

/**
 * @brief status enum here.
 */
typedef enum
{
  HS_CFG_STATUS_NONE                = 0,            /*!< status message base start          */

  HS_CFG_STATUS_POWER_OFF           = 0x10,         /*!<. power off                       */

  /* System - codec */
  HS_CFG_STATUS_VOLUME_MAX          = 0x38,         /*!<. max volume                      */
  HS_CFG_STATUS_VOLUME_MIN          ,               /*!<. min volume                      */
  HS_CFG_STATUS_MP3_NOT_FOUND       ,               /*!<. mp3 not found in TF/U-disk      */

  HS_CFG_STATUS_MEMDEV_IN           ,
  HS_CFG_STATUS_MEMDEV_OUT          ,

}hs_cfg_status_type_t;

#include "./base/cfg_define.h"
#include "cfg_main.h"
#include "./system/cfg_sys.h"
#include "./base/cfg_hal.h"
#include "./base/cfg_cachemem.h"

#endif 

/** @} */
