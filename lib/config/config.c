/*
    bootloader - Copyright (C) 2012~2014 HunterSun Technologies
                 pingping.wu@huntersun.com.cn
 */

/**
 * @file    config/system/config.c
 * @brief   config system manage file.
 * @details 
 *
 * @addtogroup  config
 * @details 
 * @{
 */
#include "lib.h"
#include "tone/cfg_audio.h"
#include "tone/cfg_ring.h"
#include "tone/cfg_tone.h"

void hs_bt_handle(uint16_t u16Idx, void *parg);
void hs_pmu_chipPowerOffRdy(uint16_t msg, void *parg);


static void _cfg_Test_0(uint16_t u16Idx, void *parg) __attribute__ ((section (".bttext"), optimize(s), noinline));
static void _cfg_Test_0(uint16_t u16Idx, void *parg)
{
  cfg_dbg("Test %d event happened!", u16Idx);
  (void)u16Idx;
  (void)parg;
}

static void _cfg_Test_1(uint16_t u16Idx, void *parg) __attribute__ ((section (".mp3text"), optimize(s), noinline));
static void _cfg_Test_1(uint16_t u16Idx, void *parg)
{
  cfg_dbg("Test %d event happened!", u16Idx);
  (void)u16Idx;
  (void)parg;
}

/****************************************************************************************
 * event table define
 ****************************************************************************************/

/* 0, HS_CFG_MODULE_BOOTLOADER    */
static const hs_cfg_mess_oper_t g_stSysBootEventOper[] =
{  
  { HS_CFG_EVENT_NONE,                  NULL },
};

/* 1, HS_CFG_MODULE_SYS    */
static const hs_cfg_mess_oper_t g_stSystemEventOper[] =
{
  #if HS_USE_BT
  { HS_CFG_EVENT_BT_AUDIO_START,              hs_bt_start },
  { HS_CFG_EVENT_BT_AUDIO_STOP,               hs_bt_stop  },
  { HS_CFG_EVENT_BT_HID_START,                hs_bt_hid_start },
  { HS_CFG_EVENT_BT_HID_STOP,                 hs_bt_hid_stop  },
  #endif

  { HS_CFG_EVENT_RECORDER_SW,                 hs_recorder_switch },
  { HS_CFG_EVENT_SPEAKER_SW,                  hs_speaker_sw },

  { HS_CFG_EVENT_PRE_POWEROFF,                hs_pmu_chipPrePowerOff },
  { HS_CFG_EVENT_PMU_POWEROFF,                hs_pmu_chipPowerOff   },
  { HS_CFG_EVENT_PMU_DEEPSLEEP,               hs_pmu_chipDeepSleep  },  

  { HS_CFG_EVENT_NONE,                        NULL },
};

/* 2, HS_CFG_MODULE_PERIP    */
static const hs_cfg_mess_oper_t g_stSysPeripEventOper[] =
{
  #if HS_SHELL_USE_USBSERIAL && defined(TEST_ENABLE)
  { HS_CFG_EVENT_USB_SERIALRDY,         hs_shell_usbSerialOpen  },
  { HS_CFG_EVENT_USB_SERIALCLS,         hs_shell_usbSerialClose },
  #endif

  #if HAL_USE_USB_SERIAL
  { HS_CFG_EVENT_USB_SERIALSTART,       hs_usb_serialOpen       },
  { HS_CFG_EVENT_USB_SERIALCLOSE,       hs_usb_serialClose      },
  #endif   
  
  { HS_CFG_EVENT_NONE,                  NULL },
};

/* 3, HS_CFG_MODULE_BLE    */
static const hs_cfg_mess_oper_t g_stSysBleEventOper[] =
{
  { HS_CFG_EVENT_NONE,                  NULL },
};

/* 4, HS_CFG_MODULE_BT_CLASSIC    */
static const hs_cfg_mess_oper_t g_stSysBtClassicEventOper[] =
{
  #if HAL_USE_AUDIO
  //{ HS_CFG_EVENT_RING_RECSTART,         hs_cfg_ringRecordStart },
  //{ HS_CFG_EVENT_RING_RECSTOP,          hs_cfg_ringRecordStop  },
  //{ HS_CFG_EVENT_RING_PLAY,             hs_cfg_ringPlay },
  #endif

#if HAL_USE_AUDIO    //HFP ring incoming call
  {HS_CFG_EVENT_BT_INCOMING_CALL,      hs_cfg_ringIncomingCallStart},
  {HS_CFG_EVENT_BT_CALL_REJECTED,      hs_cfg_ringIncomingCallStop},
  {HS_CFG_EVENT_BT_CALL_ENDED,         hs_cfg_ringIncomingCallStop},
#endif

  #if HS_USE_BT
  //{ HS_CFG_EVENT_BATTERY_CHANGE,        hsc_HFP_SendBattery() },
  { HS_CFG_EVENT_BT_CONTROL_CMD,           hs_bt_handle },
#endif
  { HS_CFG_EVENT_NONE,                                 NULL },
};

/* 5, player */
static const hs_cfg_mess_oper_t g_stSysPlayerEventOper[] =
{
  #if HS_USE_PLAYER
  { HS_CFG_EVENT_PLAYER_DESTROY,        hs_player_destroy         },

  #if HS_USE_MP3
  { HS_CFG_EVENT_PLAYER_MP3CREATE,      hs_player_createMusic     },
  #endif

  #if HS_USE_FM
  { HS_CFG_EVENT_PLAYER_FMCREATE,       hs_player_createFm        },
  #endif

  #if HAL_USE_USB_AUDIO
  { HS_CFG_EVENT_PLAYER_USBAUDCREATE,   hs_player_createUsbaudio  },
  #endif

  #if HS_USE_AUX && HS_USE_PLAYER
  { HS_CFG_EVENT_PLAYER_AUXCREATE,      hs_player_createAux   },
  #endif
  
  { HS_CFG_EVENT_PLAYER_START,          hs_player_start       },
  { HS_CFG_EVENT_PLAYER_STOP,           hs_player_stop        },
  { HS_CFG_EVENT_PLAYER_MUTE,           hs_player_mute        },
  { HS_CFG_EVENT_PLAYER_NEXT,           hs_player_next        },
  { HS_CFG_EVENT_PLAYER_PREV,           hs_player_prev        },
  { HS_CFG_EVENT_PLAYER_VOLINC,         hs_player_volumeInc   },
  { HS_CFG_EVENT_PLAYER_VOLDEC,         hs_player_volumeDec   },
  { HS_CFG_EVENT_PLAYER_FREQINC,        hs_player_freqInc     },
  { HS_CFG_EVENT_PLAYER_FREQDEC,        hs_player_freqDec     },
  { HS_CFG_EVENT_PLAYER_FUNCSET,        hs_player_funcSet     },

  { HS_CFG_EVENT_PLAYER_VOLINCBIG,      hs_player_volumeIncBig   },
  { HS_CFG_EVENT_PLAYER_VOLDECBIG,      hs_player_volumeDecBig   },
  #endif

  { HS_CFG_EVENT_NONE,                  NULL },
};

/* 0x10, HS_CFG_MODULE_TEST    */
static const hs_cfg_mess_oper_t g_stSysTestEventOper[] =
{
  { HS_CFG_EVENT_TEST_0,                _cfg_Test_0 },
  { HS_CFG_EVENT_TEST_1,                _cfg_Test_1 },
  { HS_CFG_EVENT_NONE,                  NULL },
};

static const hs_cfg_mess_oper_t *g_pstSysEventOper[HS_CFG_MODULE_NUM] =
{
  g_stSysBootEventOper,         /*!< 0   */
  g_stSystemEventOper,          /*!< 1   */
  g_stSysPeripEventOper,        /*!< 2   */
  g_stSysBleEventOper,          /*!< 3   */
  g_stSysBtClassicEventOper,    /*!< 4   */
  g_stSysPlayerEventOper,       /*!< 5   */
  NULL,                         /*!< 6   */
  NULL,                         /*!< 7   */
  NULL,                         /*!< 8   */
  NULL,                         /*!< 9   */
  NULL,                         /*!< a   */
  NULL,                         /*!< b   */
  NULL,                         /*!< c   */
  NULL,                         /*!< d   */
  NULL,                         /*!< e   */
  NULL,                         /*!< f   */
  g_stSysTestEventOper,         /*!< 10  */
};

/****************************************************************************************
 * status table define
 ****************************************************************************************/
/* 0, HS_CFG_MODULE_BOOTLOADER    */
static const hs_cfg_mess_oper_t g_stSysBootStatusOper[] =
{
  { HS_CFG_STATUS_NONE,                 NULL },
};

/* 1, HS_CFG_MODULE_SYS    */
static const hs_cfg_mess_oper_t g_stSystemStatusOper[] =
{
  { HS_CFG_STATUS_POWER_OFF,            hs_pmu_chipPowerOff},
  { HS_CFG_STATUS_NONE,                 NULL },
};

/* 2, HS_CFG_MODULE_PERIP    */
static const hs_cfg_mess_oper_t g_stSysPeripStatusOper[] =
{  
  #if HAL_USE_FATFS
  { HS_CFG_STATUS_MEMDEV_IN,            hs_fatfs_mount        },
  { HS_CFG_STATUS_MEMDEV_OUT,           hs_fatfs_unmount      },
  #endif  
  
  { HS_CFG_STATUS_NONE,                 NULL },
};

/* 3, HS_CFG_MODULE_BLE    */
static const hs_cfg_mess_oper_t g_stSysBleStatusOper[] =
{
  { HS_CFG_STATUS_NONE,                 NULL },
};

/* 4, HS_CFG_MODULE_BT_HCI    */
static const hs_cfg_mess_oper_t g_stSysBtHciStatusOper[] =
{  
  { HS_CFG_STATUS_NONE,                 NULL },
};

static const hs_cfg_mess_oper_t *g_pstSysStatusOper[HS_CFG_MODULE_NUM] =
{
  g_stSysBootStatusOper,
  g_stSystemStatusOper,
  g_stSysPeripStatusOper,
  g_stSysBleStatusOper,
  g_stSysBtHciStatusOper,  
};

/** @} */
