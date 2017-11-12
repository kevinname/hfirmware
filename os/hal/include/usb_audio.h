/*
 *  pengjiang, 20140714
 */

#ifndef _USB_AUDIO_H_
#define _USB_AUDIO_H_

#if HAL_USE_USB_AUDIO

//sample rate bit map
#define SR_8K       (0x0001<<0)// 48k base
#define SR_12K      (0x0001<<1)
#define SR_16K      (0x0001<<2)
#define SR_24K      (0x0001<<3)
#define SR_32K      (0x0001<<4)
#define SR_48K      (0x0001<<5)
#define SR_96K      (0x0001<<6)
#define SR_11K      (0x0100<<0)//44.1k base
#define SR_22K      (0x0100<<1)
#define SR_44K      (0x0100<<2)

#define IDX_PLY     0
#define IDX_REC     1

#define PLY_MAX_PKT_SIZ     ((48+1)*(16/8)*2)// 2ch, 16bit,48k, add 1 sample
#define REC_MAX_PKT_SIZ     ((16+1)*(16/8)*2)// 2ch, 16bit,16k, add 1 sample
#define PLY_BUF_NUM         4
#define REC_BUF_NUM         2

#define UA_CMD_NONE             0
#define UA_CMD_START_PLY        1
#define UA_CMD_STOP_PLY         2
#define UA_CMD_START_REC        3
#define UA_CMD_STOP_REC         4
#define UA_CMD_SET_VOL          5
#define UA_CMD_SET_MUTE         6
#define UA_CMD_SET_SR           7
#define UA_CMD_SET_CH           8
#define UA_CMD_AUD_DATA_ARRIVED 9
#define UA_CMD_AUD_DATA_SENT    10
#define UA_CMD_EXIT_THREAD      255

#define SEND_DATA_CNT 100

#define USB_AUDIO_STATE_STOP    0
#define USB_AUDIO_STATE_START   1

#pragma pack(push)
#pragma pack(1)
typedef struct {
  uint8_t ChNum;
  uint8_t Bits;
  uint8_t SrNum; //sample rate bit map, indicate how many Sample rate in this alt setting
  uint16_t SrBitMap;
}alt_setting_info;

typedef struct
{
  uint8_t bmute;
  int16_t wvol[2];
}vol_ctrl;

typedef struct
{
  uint16_t vol_min;
  uint16_t vol_max;
  uint16_t vol_res;
}vol_min_max;

typedef struct
{
  uint8_t ChNum;
  uint8_t Bits;
  uint16_t Sr;
  uint16_t Sr_Frac; //for 44.1k base
}audio_fmt_info;

//audio streaming interface number
typedef struct {
  uint8_t IsPly; //ply or rec interface
  uint8_t IfNum;
  usbep_t EpNum;
  vol_ctrl defVol;
  vol_min_max volminmax;
  uint8_t AltSettingNum;//including alt setting 0
  const alt_setting_info *Settings;//including alt setting 0
}as_if_info;

typedef struct {
  uint8_t cmd;
  uint16_t para1;
  uint16_t para2;
  uint32_t pval;
}usbaudio_cmd;

typedef usbaudio_cmd usbaudio_stream;

typedef struct {
  uint32_t SampleRate_Usb;
  uint16_t SampleRate_Drv;
  hs_i2s_sample_t SampleRate_I2S;
  uint16_t SampleRate_Frac; //for 44.1k base sr
}SAMPLE_RATE_CONVERT;

#pragma pack(pop)

/**
 * @brief   USB audio Driver configuration structure.
 * @details An instance of this structure must be passed to @p sduStart()
 *          in order to configure and start the driver operations.
 */
typedef struct {
  /**
   * @brief   USB driver to use.
   */
  USBDriver *usbp;
  uint8_t state;
  const as_if_info *iso_info[2]; //0 for ply, 1 for rec
  audio_fmt_info fmt_info[2];//0 for ply, 1 for rec
  vol_ctrl vol_ctl[2];//0 for ply, 1 for rec
  uint8_t if_alt_set[2];//0 for ply, 1 for rec

  uint8_t ply_buf_idx;
  //uint8_t ply_buf[PLY_BUF_NUM][PLY_MAX_PKT_SIZ];
  uint8_t *ply_buf;
  uint8_t rec_buf_idx;
  //uint8_t rec_buf[REC_BUF_NUM][REC_MAX_PKT_SIZ];
  uint8_t *rec_buf;
  uint16_t rec_frac_cnt;
  const as_if_info *info;

  thread_t *cmd_thread;
}USBAudioDriver;

extern USBAudioDriver USBAUD1;

#ifdef __cplusplus
extern "C" {
#endif
  void usbaudInit(void);
  void usbaudObjectInit(USBAudioDriver *usbaudp,USBDriver *usbp);
  void usbaudStart(USBAudioDriver *usbaudp);
  void usbaudStop(void);
  bool_t usbaud_class_req_handler_if(USBDriver *usbp);
  bool_t usbaud_class_req_handler_ep(USBDriver *usbp, bool_t IsPly);
  bool_t usbaud_std_req_handler_set_interface(USBDriver *usbp,bool_t IsPly);
  void usbaudDataTransmitted(USBDriver *usbp, usbep_t ep);
  void usbaudDataReceived(USBDriver *usbp, usbep_t ep);
  void usbaud_dma_outep_cb(USBDriver *usbp, usbep_t ep);
  void usbaud_dma_inep_cb(USBDriver *usbp, usbep_t ep);
  void usb_audio_reset(USBDriver *usbp, const as_if_info* info);
  void usbaudSetAsIfInfo(USBAudioDriver *usbaudp, const as_if_info* info);
#ifdef __cplusplus
}
#endif

#endif

//USB Audio class definition 1.0 macros
//Reference USB Device Class Definition for Audio Devices Release 1.0 SPEC p.99
//Audio Device Class Codes
//A.1 Audio Interface Class Code
#define AUDIO_CLASS_CODE                    (0x01)

//A.2 Audio Interface Subclass Codes
#define SUBCLASS_UNDEFINED                  (0x00)
#define AUDIO_CONTROL                       (0x01)
#define AUDIO_STREAMING                     (0x02)
#define MIDI_STREAMING                      (0x03)

//A.3 Audio Interface Protocol Codes
#define PR_PROTOCOL_UNDEFINED               (0x00)

//A.4 Audio Class-Specific Descriptor Types
#define CS_UNDEFINED                        (0x20)
#define CS_DEVICE                           (0x21)
#define CS_CONFIGURATION                    (0x22)
#define CS_STRING                           (0x23)
#define CS_INTERFACE                        (0x24)
#define CS_ENDPOINT                         (0x25)

//A.5 Audio Class-Specific AC Interface Descriptor Subtypes
#define AC_DESCRIPTOR_UNDEFINED             (0x00)
#define HEADER                              (0x01)
#define INPUT_TERMINAL                      (0x02)
#define OUTPUT_TERMINAL                     (0x03)
#define MIXER_UNIT                          (0x04)
#define SELECTOR_UNIT                       (0x05)
#define FEATURE_UNIT                        (0x06)
#define PROCESSING_UNIT                     (0x07)
#define EXTENSION_UNIT                      (0x08)

//A.6 Audio CLass-Specific AS Interface Descriptor Subtypes
#define AS_DESCRIPTOR_UNDEFINED             (0x00)
#define AS_GENERAL                          (0x01)
#define FORMAT_TYPE                         (0x02)
#define FORMAT_SPECIFIC                     (0x03)

//A.7 Processing Unit Process Types
#define PROCESS_UNDEFINED                   (0x00)
#define UP_DOWNMIX_PROCESS                  (0x01)
#define DOLBY_PROLOGIC_PROCESS              (0x02)
#define THREE_D_STEREO_EXTENDER_PROCESS     (0x03)
#define REVERBERATION_PROCESS               (0x04)
#define CHORUS_PROCESS                      (0x05)
#define DYN_RANGE_COMP_PROCESS              (0x06)

//A.8 Audio Class-Specific Endpoint Descriptor Subtypes
#define DESCRIPTOR_UNDEFINED                (0x00)
#define EP_GENERAL                          (0x01)

//A.9 Audio Class-Specific Request Codes
#define REQUEST_CODE_UNDEFINED              (0x00)
#define SET_CUR                             (0x01)
#define SET_MIN                             (0x02)
#define SET_MAX                             (0x03)
#define SET_RES                             (0x04)
#define SET_MEM                             (0x05)
#define GET_CUR                             (0x81)
#define GET_MIN                             (0x82)
#define GET_MAX                             (0x83)
#define GET_RES                             (0x84)
#define GET_MEM                             (0x85)
#define GET_STAT                            (0xFF)

//A10.1 Terminal Control Selectors
#define TE_CONTROL_UNDEFINED                (0x00)
#define COPY_PROTECT_CONTROL                (0x01)

//A10.2 Feature Unit Control Selectors
#define FU_CONTROL_UNDEFINED                (0x00)
#define MUTE_CONTROL                        (0x01)
#define VOLUME_CONTROL                      (0x02)
#define BASS_CONTROL                        (0x03)
#define MID_CONTROL                         (0x04)
#define TREBLE_CONTROL                      (0x05)
#define GRAPHIC_EQUALIZER_CONTROL           (0x06)
#define AGC_CONTROL                         (0x07)
#define DELAY_CONTROL                       (0x08)
#define BASS_BOOST_CONTROL                  (0x09)
#define LOUDNESS_CONTROL                    (0x0A)

//A10.3.1 Up/Down-mix Processing Unit Control Selectors
#define UD_CONTROL_UNDEFINED                (0x00)
#define UD_ENABLE_CONTROL                   (0x01)
#define UD_MODE_SELECT_CONTROL              (0x02)

//A10.3.2 Dolby Prologic Processing Unit Control Selectors
#define DP_CONTROL_UNDEFINED                (0x00)
#define DP_ENABLE_CONTROL                   (0x01)
#define DP_MODE_SELECT_CONTROL              (0x02)

//A10.3.3 3D Stereo Extender Processing Unit Control Selectors
#define THREE_D_CONTROL_UNDEFINED           (0x00)
#define THREE_D_ENABLE_CONTROL              (0x01)
#define SPACIOUSNESS_CONTROL                (0x03)

//A10.3.4 Reverberation Process Unit Control Selectors
#define RV_CONTROL_UNDEFINED                (0x00)
#define RV_ENABLE_CONTROL                   (0x01)
#define REVERB_LEVEL_CONTROL                (0x02)
#define REVERB_TIME_CONTROL                 (0x03)
#define REVERB_FEEDBACK_CONTROL             (0x04)

//A10.3.5 Chorus Process Unit Control Selectors
#define CH_CONTROL_UNDEFINED                (0x00)
#define CH_ENABLE_CONTROL                   (0x01)
#define CHORUS_LEVEL_CONTROL                (0x02)
#define CHORUS_RATE_CONTROL                 (0x03)
#define CHORUS_DEPTH_CONTROL                (0x04)

//A10.3.6 Dynamic Range Comopressor Processing Unit Control Selectors
#define DR_CONTROL_UNDEFINED                (0x00)
#define DR_ENABLE_CONTROL                   (0x01)
#define COMPRESSION_RATE_CONTROL            (0x02)
#define MAXAMPL_CONTROL                     (0x03)
#define THRESHOLD_CONTROL                   (0x04)
#define ATTACK_TIME                         (0x05)
#define RELEASE_TIME                        (0x06)

//A10.4 Extension Unit Control Selectors
#define XU_CONTROL_UNDEFINED                (0x00)
#define XU_ENABLE_CONTROL                   (0x01)

//A10.5 Endpoint Control Selectors
#define EP_CONTROL_UNDEFINED                (0x00)
#define SAMPLING_FREQ_CONTROL               (0x01)
#define PITCH_CONTROL                       (0x02)

//table 2-1 in terminal type spec
// usb streaming: 0x0101
#define TERM_TYPE_USBSTREAMING_LOW     (0x01)
#define TERM_TYPE_USBSTREAMING_HIGH    (0x01)
// microphone: 0x0201
#define TERM_TYPE_MICROPHONE_LOW        (0x01)
#define TERM_TYPE_MICROPHONE_HIGH       (0x02)
// micarray: 0x0205
#define TERM_TYPE_MICARRAY_LOW          (0x05)
#define TERM_TYPE_MICARRAY_HIGH         (0x02)
// speaker: 0x0301
#define TERM_TYPE_SPEAKER_LOW           (0x01)
#define TERM_TYPE_SPEAKER_HIGH          (0x03)
// digital output (spdif): 0x0602,
// peng, 010, 20081110, change from 0605 to 0602
#define TERM_TYPE_SPDIF_LOW             (0x02)
#define TERM_TYPE_SPDIF_HIGH            (0x06)

#define FORMAT_PCM_LOW             (0x01)
#define FORMAT_PCM_HIGH            (0x00)

#define FORMAT_TYPE_I              (0x01)

// bitmap for feature unit control
#define FU_CONTROL_UNDEFINED_BITMAP                (0x00)
#define MUTE_CONTROL_BITMAP                        (0x01)
#define VOLUME_CONTROL_BITMAP                      (0x02)
#define BASS_CONTROL_BITMAP                        (0x04)
#define MID_CONTROL_BITMAP                         (0x08)
#define TREBLE_CONTROL_BITMAP                      (0x10)
#define GRAPHIC_EQUALIZER_CONTROL_BITMAP           (0x20)
#define AGC_CONTROL_BITMAP                         (0x40)
#define DELAY_CONTROL_BITMAP                       (0x80)

#define SUBFRAMESIZE_24BIT    (0x03)
#define SUBFRAMESIZE_16BIT    (0x02)

#define BITRESOLUTION_24BIT   (0x18)
#define BITRESOLUTION_16BIT   (0x10)

#define SAMPLERATE_8K_BYTE0        (0x40)
#define SAMPLERATE_8K_BYTE1        (0x1F)
#define SAMPLERATE_8K_BYTE2        (0x00)
#define SAMPLERATE_12K_BYTE0       (0xE0)
#define SAMPLERATE_12K_BYTE1       (0x2E)
#define SAMPLERATE_12K_BYTE2       (0x00)
#define SAMPLERATE_16K_BYTE0       (0x80)
#define SAMPLERATE_16K_BYTE1       (0x3E)
#define SAMPLERATE_16K_BYTE2       (0x00)
#define SAMPLERATE_24K_BYTE0       (0xC0)
#define SAMPLERATE_24K_BYTE1       (0x5D)
#define SAMPLERATE_24K_BYTE2       (0x00)
#define SAMPLERATE_32K_BYTE0       (0x00)
#define SAMPLERATE_32K_BYTE1       (0x7D)
#define SAMPLERATE_32K_BYTE2       (0x00)
#define SAMPLERATE_48K_BYTE0       (0x80)
#define SAMPLERATE_48K_BYTE1       (0xBB)
#define SAMPLERATE_48K_BYTE2       (0x00)
#define SAMPLERATE_96K_BYTE0       (0x00)
#define SAMPLERATE_96K_BYTE1       (0x77)
#define SAMPLERATE_96K_BYTE2       (0x01)
#define SAMPLERATE_11K_BYTE0       (0x11)
#define SAMPLERATE_11K_BYTE1       (0x2B)
#define SAMPLERATE_11K_BYTE2       (0x00)
#define SAMPLERATE_22K_BYTE0       (0x22)
#define SAMPLERATE_22K_BYTE1       (0x56)
#define SAMPLERATE_22K_BYTE2       (0x00)
#define SAMPLERATE_44K_BYTE0      (0x44)
#define SAMPLERATE_44K_BYTE1      (0xAC)
#define SAMPLERATE_44K_BYTE2      (0x00)

#define MAXSIZE_2CH_16BIT_16K_LOW    (0x40)
#define MAXSIZE_2CH_16BIT_16K_HIGH   (0x00)

#define MAXSIZE_2CH_16BIT_48K_LOW    (0xC0)
#define MAXSIZE_2CH_16BIT_48K_HIGH   (0x00)

#define MAXSIZE_2CH_24BIT_48K_LOW    (0x20)
#define MAXSIZE_2CH_24BIT_48K_HIGH   (0x01)

#define MAXSIZE_2CH_16BIT_96K_LOW    (0x80)
#define MAXSIZE_2CH_16BIT_96K_HIGH   (0x01)

#define MAXSIZE_2CH_24BIT_96K_LOW    (0x40)
#define MAXSIZE_2CH_24BIT_96K_HIGH   (0x02)

#define UNITID_INPUTL_USB_OUT              (0x01)
#define UNITID_INPUT_MIC_IN                (0x02)
#define UNITID_OUTPUT_LINE_OUT             (0x03)
#define UNITID_OUTPUT_USB_IN               (0x04)
#define UNITID_FEATURE_DAC_VOLUME          (0x05)
#define UNITID_FEATURE_ADC_VOLUME          (0x06)

#endif
