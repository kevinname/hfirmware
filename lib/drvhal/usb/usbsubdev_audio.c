#include "usbsubdev_audio.h"
#include "usbsubdev_storage.h"
#include "fatfs.h"


#if HAL_USE_USB_AUDIO || HAL_USE_USB_STORAGE
//for usb audio device
/*===========================================================================*/
/* USB related stuff.                                                        */
/*===========================================================================*/

#if HAL_USE_USB_AUDIO
const If_Num dev_if_num_audio = {
#if defined (__CC_ARM)
  USBD1_IF_NUM_AC,
  USBD1_IF_NUM_AS_OUT,
  USBD1_IF_NUM_AS_IN,
  0xff,
  0xff,
#else
  .Ac_If_Num = USBD1_IF_NUM_AC,
  .As_Out_IfNum = USBD1_IF_NUM_AS_OUT,
  .As_In_IfNum = USBD1_IF_NUM_AS_IN,
  .Serial_IfNum = 0xff,
  .Storage_If_Num = 0xff,
#endif
};

const As_Ep_Num dev_ep_num_audio = {
#if defined (__CC_ARM)
  USBD1_ISO_OUT_EP,
  USBD1_ISO_IN_EP,
#else
  .As_Out_EpNum = USBD1_ISO_OUT_EP,
  .As_In_EpNum = USBD1_ISO_IN_EP,
#endif
};

/**
 * @brief   Audio IN EP1 state.
 */
static USBInEndpointState ep1instate_iso;

/**
 * @brief   AudioOUT EP1 state.
 */
static USBOutEndpointState ep1outstate_iso;

/**
 * @brief   EP1 initialization structure (both IN and OUT).
 */
static const USBEndpointConfig ep1config_iso = {
  USB_EP_MODE_TYPE_ISOC,
  NULL,
  usbaudDataTransmitted,
  usbaudDataReceived,
  MAXSIZE_2CH_16BIT_16K_LOW+(MAXSIZE_2CH_16BIT_16K_HIGH*(1<<8)),
  MAXSIZE_2CH_16BIT_48K_LOW+(MAXSIZE_2CH_16BIT_48K_HIGH*(1<<8)),
  &ep1instate_iso,
  &ep1outstate_iso,
  1,
  NULL,
  {
    { USB_DMA_CH_AUD_OUT, USB_DMA_CH_AUD_IN},
    { DMA_MODE_0, DMA_MODE_0},
    usbaud_dma_outep_cb,
    usbaud_dma_inep_cb,
  },
  TRUE,
  TRUE
};

//fifo config for EPx
const Fifo_Cfg fifo_cfg_iso[] =
{
  //EP1 out
  {
    0x01,
    0,
    64,
    256,
  },
  //EP1 IN
  {
    0x81,
    0,
    320,
    64,
  },

};

const alt_setting_info ply_alt_setting[] =
{
  {
    0,
    0,
    0,
    0
  },
  {
#if defined (__CC_ARM)
    2,
    16,
    4,
    SR_44K|SR_48K|SR_16K|SR_8K,
#else
    .ChNum = 2,
    .Bits = 16,
    .SrNum = 4,
    .SrBitMap = SR_44K|SR_48K|SR_16K|SR_8K,
#endif
  }
};

const alt_setting_info rec_alt_setting[] =
{
  {
    0,
    0,
    0,
    0
  },
  {
#if defined (__CC_ARM)
    2,
    16,
    3,
    SR_8K|SR_16K|SR_11K,
#else
    .ChNum = 2,
    .Bits = 16,
    .SrNum = 3,
    .SrBitMap = SR_8K|SR_16K|SR_11K,
#endif
  }
};

const as_if_info if_info[2] =
{
  //play info
  {
#if defined (__CC_ARM)
    TRUE,
    USBD1_IF_NUM_AS_OUT,
    USBD1_ISO_OUT_EP,
    { 0, {-20, -20}},
    { 0xE200, 0x0000, 0x0100},
    2,
    ply_alt_setting,
#else
    .IsPly = TRUE,
    .IfNum = USBD1_IF_NUM_AS_OUT,
    .EpNum = USBD1_ISO_OUT_EP,
    .defVol = {0, {-20, -20}},
    .volminmax = {0xE200, 0x0000, 0x0100},
    .AltSettingNum = 2,
    .Settings = ply_alt_setting,
#endif
  },
  //rec info
  {
#if defined (__CC_ARM)
    FALSE,
    USBD1_IF_NUM_AS_IN,
    USBD1_ISO_IN_EP,
    { 0, {-20, -20}},
    { 0x0000, 0x1E00, 0x0100},
    2,
    rec_alt_setting,
#else
    .IsPly = FALSE,
    .IfNum = USBD1_IF_NUM_AS_IN,
    .EpNum = USBD1_ISO_IN_EP,
    .defVol = {0, {-20, -20}},
    .volminmax = {0x0000, 0x1E00, 0x0100},
    .AltSettingNum = 2,
    .Settings = rec_alt_setting,
#endif
  }
};

#endif

/*
 * Handles the USB driver global events.
 */
#if HAL_USE_USB_AUDIO
void usb_event_audio(USBDriver *usbp, usbevent_t event) {

  chDbgAssert(usbp, "null pointer");

  switch (event) {
    case USB_EVENT_RESET:
    fifo_config(sizeof(fifo_cfg_iso)/sizeof(Fifo_Cfg), fifo_cfg_iso);
    usb_audio_reset(usbp, if_info);
    return;
    case USB_EVENT_ADDRESS:
    return;
    case USB_EVENT_CONFIGURED:
    //chSysLockFromIsr();

    /* Enables the endpoints specified into the configuration.
     Note, this callback is invoked from an ISR so I-Class functions
     must be used.*/
    usbInitEndpointI(usbp, USBD1_ISO_OUT_EP, &ep1config_iso);

    //chSysUnlockFromIsr();
    return;
    case USB_EVENT_SUSPEND:
    return;
    case USB_EVENT_WAKEUP:
    return;
    case USB_EVENT_STALLED:
    return;
  }
  return;
}
#endif

/*
 * USB Device Descriptor.
 */
#if HAL_USE_USB_AUDIO&&HAL_USE_USB_STORAGE
#define USB_PID   USB_PID_HS_COMPOSITE
#elif HAL_USE_USB_AUDIO&&(!HAL_USE_USB_STORAGE)
#define USB_PID   USB_PID_HS_AUDIO_ONLY
#else
#define USB_PID   USB_PID_HS_STORAGE_ONLY
#endif

static const uint8_t device_descriptor_data_composite[18] = {
  USB_DESC_DEVICE (0x0110, /* bcdUSB (1.1).            */
      0x00, /* bDeviceClass (CDC).  */
      0x00, /* bDeviceSubClass.      */
      0x00, /* bDeviceProtocol.        */
      0x40, /* bMaxPacketSize.        */
      USB_VID_HS, /* idVendor (HS).         */
      USB_PID, /* idProduct.      */
      0x0010, /* bcdDevice.                 */
      #if 1
      USB_STR_DESC_IDX_VENDOR, /* iManufacturer. */
      USB_STR_DESC_IDX_PROD,/* iProduct.*/
      3, /* iSerialNumber.             */
      #else
      0,
      0,
      0,
      #endif

      1) /* bNumConfigurations.    */
};

const USBDescriptor device_descriptor_composite = {
  sizeof device_descriptor_data_composite,
  device_descriptor_data_composite
};


static const uint8_t configuration_descriptor_data_composite_all[] =
{
  // ===== Configuration Header Descriptor =====
  0x09,                         //bLength
  0x02,                         //bDescriptorType, CONFIGURATION
  0xCD+7+7+9,                   //wTotalLength, low byte
  0x00,                         //wTotalLength, high byte
  0x04,                       //bNumInterfaces, AC + AS Out + AS In + Bulk
  0x01,                         //bConfigurationValue
  0x00,                         //iConfiguration
#ifdef SUPPORT_REMOTE_WAKEUP
  0xA0,                         //bmAttributes, remote wakeup
#else
  0x80,                         //bmAttributes
#endif
  0x32,                         //bMaxPower, 100mA
  // ===== Standard Audio Control Interface =====
  0x09,                         //bLength
  0x04,                         //bDescriptorType, INTERFACE
  USBD1_IF_NUM_AC,              //bInterfaceNumber
  0x00,                         //bAlternateSetting
  0x00,                         //bNumEndpoints, only Use EP0
  AUDIO_CLASS_CODE,             //bInterfaceClass, AUDIO interface class
  AUDIO_CONTROL,                //bInterfaceSubClass, AUDIOCONTROL subclass
  0x00,                         //bInterfaceProtocol
  0x00,                         //iInterface
  // ===== Class-specific AC Interface Head =====
  0x0A,                         //bLength
  CS_INTERFACE,                 //bDescriptorType
  HEADER,                       //bDescriptorSubtype
  0x00,                         //bcdADC Low Byte
  0x01,                         //bcdADC High Byte
  0x48,                         //wTotalLength Low Byte
  0x00,                         //wTotalLength High Byte
  0x02,                         //bInCollection, AS out + AS in
  USBD1_IF_NUM_AS_OUT,          //baInterfaceNr(1), AS Out
  USBD1_IF_NUM_AS_IN,           //baInterfaceNr(2), AS In
  // ===== Input Terminal Descriptor =====
  0x0C,                         //bLength
  CS_INTERFACE,                 //bDescriptorType
  INPUT_TERMINAL,               //bDescriptorSubtype
  UNITID_INPUTL_USB_OUT,        //bTerminalID
  TERM_TYPE_USBSTREAMING_LOW,   //wTerminalType Low Byte, USB Streaming
  TERM_TYPE_USBSTREAMING_HIGH,  //wTerminalType High Bye
  0x00,                         //bAssocTerminal
  0x02,                         //bNrChannels, 2 logical output channels
  0x03,                         //wChannelConfig Low Byte
  0x00,                         //wChannelConfig High Byte
  0x00,                         //iChannelNames
  0x00,                         //iTerminal
  // ===== Input Terminal Descriptor =====
  0x0C,                         //bLength
  CS_INTERFACE,                 //bDescriptorType
  INPUT_TERMINAL,               //bDescriptorSubtype
  UNITID_INPUT_MIC_IN,          //bTerminalID
  TERM_TYPE_MICROPHONE_LOW,     //wTerminalType Low Byte
  TERM_TYPE_MICROPHONE_HIGH,    //wTerminalType High Bye
  0x00,                         //bAssocTerminal
  0x02,                         //bNrChannels
  0x03,                         //wChannelConfig Low Byte
  0x00,                         //wChannelConfig High Byte
  0x00,                         //iChannelNames
  0x00,                         //iTerminal
  // ===== OutPut Terminal Descriptor =====
  0x09,                         //bLength
  CS_INTERFACE,                 //bDescriptorType
  OUTPUT_TERMINAL,              //bDescriptorSubtype
  UNITID_OUTPUT_LINE_OUT,       //bTerminalID
  TERM_TYPE_SPEAKER_LOW,        //wTerminalType Low Byte, Speaker
  TERM_TYPE_SPEAKER_HIGH,       //wTerminalType High Bye
  0x00,                         //bAssocTerminal
  UNITID_FEATURE_DAC_VOLUME,    //bSourceID
  0x00,                         //iTerminal
  // ===== OutPut Terminal Descriptor =====
  0x09,                         //bLength
  CS_INTERFACE,                 //bDescriptorType
  OUTPUT_TERMINAL,              //bDescriptorSubtype
  UNITID_OUTPUT_USB_IN,         //bTerminalID
  TERM_TYPE_USBSTREAMING_LOW,   //wTerminalType Low Byte, USB Streaming
  TERM_TYPE_USBSTREAMING_HIGH,  //wTerminalType High Bye
  0x00,                         //bAssocTerminal, according to MS white paper
  UNITID_FEATURE_ADC_VOLUME,    //bSourceID
  0x00,                         //iTerminal
  // ===== Feature Unit Descriptor =====
  0x0A,                         //bLength
  CS_INTERFACE,                 //bDescriptorType
  FEATURE_UNIT,                 //bDescriptorSubtype
  UNITID_FEATURE_DAC_VOLUME,    //bUnitID
  UNITID_INPUTL_USB_OUT,        //bSourceID
  0x01,                         //bControlSize
  MUTE_CONTROL_BITMAP|VOLUME_CONTROL_BITMAP,          //bmaControl(0), master channel, mute
  0x00,//VOLUME_CONTROL_BITMAP,        //bmaControl(1), left channel, volume
  0x00,//VOLUME_CONTROL_BITMAP,        //bmaControl(2), right channel, volume
  0x00,                         //iFeature
  // ===== Feature Unit Descriptor =====
  0x0A,                         //bLength
  CS_INTERFACE,                 //bDescriptorType
  FEATURE_UNIT,                 //bDescriptorSubtype
  UNITID_FEATURE_ADC_VOLUME,    //bUnitID
  UNITID_INPUT_MIC_IN,          //bSourceID
  0x01,                         //bControlSize
  MUTE_CONTROL_BITMAP|VOLUME_CONTROL_BITMAP,          //bmaControl(0), master channel, mute
  0x00,//VOLUME_CONTROL_BITMAP,        //bmaControl(1), left channel, volume
  0x00,//VOLUME_CONTROL_BITMAP,        //bmaControl(2), right channel, volume
  0x00,                         //iFeature
  // ===== Standard AudioStreaming Interface =====
  0x09,                         //bLength
  USB_DESCRIPTOR_INTERFACE,     //bDescriptorType
  USBD1_IF_NUM_AS_OUT,          //bInterfaceNumber
  0x00,                         //bAlternateSetting, Alt. setting 0 (Zero-bandwidth)
  0x00,                         //bNumEndpoints, only Use EP0
  AUDIO_CLASS_CODE,             //bInterfaceClass, AUDIO interface class
  AUDIO_STREAMING,              //bInterfaceSubclass, AUDIOSTREAMING subclass
  0x00,                         //bInterfaceProtocol
  0x00,                         //iInterface
  // ===== Standard AudioStreaming Interface =====
  0x09,                         //bLength
  USB_DESCRIPTOR_INTERFACE,     //bDescriptorType
  USBD1_IF_NUM_AS_OUT,          //bInterfaceNumber
  0x01,                         //bAlternateSetting
  0x01,                         //bNumEndpoints, Use EP0+EP1
  AUDIO_CLASS_CODE,             //bInterfaceClass, AUDIO interface class
  AUDIO_STREAMING,              //bInterfaceSubclass, AUDIOSTREAMING subclass
  0x00,                         //bInterfaceProtocol
  0x00,                         //iInterface
  // ===== Class-specific Audio Streaming Interface =====
  0x07,                         //bLength
  CS_INTERFACE,                 //bDescriptorType
  AS_GENERAL,                   //bDescriptorSubtype
  UNITID_INPUTL_USB_OUT,        //bTerminalLink
  0x01,                         //bDelay
  FORMAT_PCM_LOW,               //bFormatTag Low Byte, PCM
  FORMAT_PCM_HIGH,              //bFormatTag High Byte
  // ===== Type 1 Format Type Descriptor =====
  0x14,                         //bLength
  CS_INTERFACE,                 //bDescriptorType
  FORMAT_TYPE,                  //bDescriptorSubtype
  FORMAT_TYPE_I,                //bFormatType
  0x02,                         //bNrChannels
  SUBFRAMESIZE_16BIT,           //bSubFrameSize
  BITRESOLUTION_16BIT,          //bBitResolution
  0x02+1+1,
  SAMPLERATE_48K_BYTE0,         //tLowerSamFreq, 48k
  SAMPLERATE_48K_BYTE1,         //tLowerSamFreq
  SAMPLERATE_48K_BYTE2,         //tLowerSamFreq
  SAMPLERATE_44K_BYTE0,        //tLowerSamFreq, 44.1k
  SAMPLERATE_44K_BYTE1,        //tLowerSamFreq
  SAMPLERATE_44K_BYTE2,        //tLowerSamFreq
  SAMPLERATE_16K_BYTE0,        //tLowerSamFreq, 44.1k
  SAMPLERATE_16K_BYTE1,        //tLowerSamFreq
  SAMPLERATE_16K_BYTE2,        //tLowerSamFreq
  SAMPLERATE_8K_BYTE0,        //tLowerSamFreq, 44.1k
  SAMPLERATE_8K_BYTE1,        //tLowerSamFreq
  SAMPLERATE_8K_BYTE2,        //tLowerSamFreq
  // ===== Standard Endpoint Descriptor =====
  0x07,                         //bLength
  USB_DESCRIPTOR_ENDPOINT,      //bDescriptorType
  USBD1_ISO_OUT_EP,             //bEndpointAddress, OUT EP1, Bit7(0=out 1=in)
  0x09,                         //bmAttributes, adaptive isochronous Endpoint
  MAXSIZE_2CH_16BIT_48K_LOW,    //wMaxPacketSize Low Byte
  MAXSIZE_2CH_16BIT_48K_HIGH,   //wMaxPacketSize High Byte,
  0x01,                         //bInterval, One packet every frame
  // ===== Class specific Endpoint Descriptor =====
  0x07,                         //bLength
  CS_ENDPOINT,                  //bDescriptorType
  EP_GENERAL,                   //bDescriptorSubtype
  SAMPLING_FREQ_CONTROL,        //bmAttributes
  0x01,                         //bLockDelayUnits, unit is milliseconds
  0x01,                         //wLockDelay
  0x00,                         //wLockDelay
  // ===== Standard AudioStreaming Interface =====
  0x09,                         //bLength
  USB_DESCRIPTOR_INTERFACE,     //bDescriptorType
  USBD1_IF_NUM_AS_IN,           //bInterfaceNumber
  0x00,                         //bAlternateSetting, Alt. setting 0
  0x00,                         //bNumEndpoints, only Use EP0
  AUDIO_CLASS_CODE,             //bInterfaceClass, AUDIO interface class
  AUDIO_STREAMING,              //bInterfaceSubclass, AUDIOSTREAMING subclass
  0x00,                         //bInterfaceProtocol
  0x00,                         //iInterface
  // ===== Standard AudioStreaming Interface =====
  0x09,                         //bLength
  USB_DESCRIPTOR_INTERFACE,     //bDescriptorType
  USBD1_IF_NUM_AS_IN,           //bInterfaceNumber
  0x01,                         //bAlternateSetting
  0x01,                         //bNumEndpoints, Use EP0+EP81
  AUDIO_CLASS_CODE,             //bInterfaceClass, AUDIO interface class
  AUDIO_STREAMING,              //bInterfaceSubclass, AUDIOSTREAMING subclass
  0x00,                         //bInterfaceProtocol
  0x00,                         //iInterface
  // ===== Class-specific Audio Streaming Interface =====
  0x07,                         //bLength
  CS_INTERFACE,                 //bDescriptorType
  AS_GENERAL,                   //bDescriptorSubtype
  UNITID_OUTPUT_USB_IN,         //bTerminalLink
  0x01,                         //bDelay
  FORMAT_PCM_LOW,               //bFormatTag Low Byte, PCM
  FORMAT_PCM_HIGH,              //bFormatTag High Byte
  // ===== Type 1 Format Type Descriptor =====
  0x11,                         //bLength
  CS_INTERFACE,                 //bDescriptorType
  FORMAT_TYPE,                  //bDescriptorSubtype
  FORMAT_TYPE_I,                //bFormatType
  0x02,                         //bNrChannels
  SUBFRAMESIZE_16BIT,           //bSubFrameSize
  BITRESOLUTION_16BIT,          //bBitResolution
  0x02+1,                         //bSamFreqType
  SAMPLERATE_16K_BYTE0,         //tLowerSamFreq
  SAMPLERATE_16K_BYTE1,         //tLowerSamFreq
  SAMPLERATE_16K_BYTE2,         //tLowerSamFreq
  SAMPLERATE_8K_BYTE0,          //tLowerSamFreq
  SAMPLERATE_8K_BYTE1,          //tLowerSamFreq
  SAMPLERATE_8K_BYTE2,          //tLowerSamFreq
  SAMPLERATE_11K_BYTE0,          //tLowerSamFreq
  SAMPLERATE_11K_BYTE1,          //tLowerSamFreq
  SAMPLERATE_11K_BYTE2,          //tLowerSamFreq
  // ===== Standard Endpoint Descriptor =====
  0x07,                         //bLength
  USB_DESCRIPTOR_ENDPOINT,      //bDescriptorType
  0x80|USBD1_ISO_IN_EP,         //bEndpointAddress, IN EP2, Bit7(0=out 1=in)
  0x05,                         //bmAttributes, asynch isochronous Endpoint
  MAXSIZE_2CH_16BIT_16K_LOW,    //wMaxPacketSize Low Byte
  MAXSIZE_2CH_16BIT_16K_HIGH,   //wMaxPacketSize High Byte,
  0x01,                         //bInterval, One packet every frame
  // ===== Class specific Endpoint Descriptor =====
  0x07,                         //bLength
  CS_ENDPOINT,                  //bDescriptorType
  EP_GENERAL,                   //bDescriptorSubtype
  SAMPLING_FREQ_CONTROL,        //bmAttributes
  0x00,                         //bLockDelayUnits, unit is milliseconds
  0x00,                         //wLockDelay
  0x00,                         //wLockDelay
  // ===== Bulk Interface =====
  0x09,                         //bLength
  0x04,                         //bDescriptorType, INTERFACE
  USBD1_IF_NUM_BULK,            //bInterfaceNumber
  0x00,                         //bAlternateSetting
  0x02,                         //bNumEndpoints
  MASS_STORAGE_CLASS_CODE,      //bInterfaceClass
  MASS_STORAGE_SUBCLASS_SCSI,   //bInterfaceSubClass
  MASS_STORAGE_IFPROTOCAL_BULK, //bInterfaceProtocol
  0x04,                         //iInterface
  // ===== Standard Bulk Out Endpoint Descriptor =====
  0x07,                         //bLength
  USB_DESCRIPTOR_ENDPOINT,      //bDescriptorType
  USBD1_BULK_OUT_EP,            //bEndpointAddress
  MASS_STORAGE_ENDPOINT_BULK,   //bmAttributes, bulk Endpoint
  0x40,                         //wMaxPacketSize Low Byte
  0x00,                         //wMaxPacketSize High Byte,
  0x00,                         //bInterval
  // ===== Standard Bulk In Endpoint Descriptor =====
  0x07,                         //bLength
  USB_DESCRIPTOR_ENDPOINT,      //bDescriptorType
  0x80|USBD1_BULK_IN_EP,        //bEndpointAddress
  MASS_STORAGE_ENDPOINT_BULK,   //bmAttributes, bulk Endpoint
  0x40,                         //wMaxPacketSize Low Byte
  0x00,                         //wMaxPacketSize High Byte,
  0x00,                         //bInterval
};

//audio only
static const uint8_t configuration_descriptor_data_composite_audio[] =
{
  // ===== Configuration Header Descriptor =====
  0x09,                         //bLength
  0x02,                         //bDescriptorType, CONFIGURATION
  0xCD,                         //wTotalLength, low byte
  0x00,                         //wTotalLength, high byte
  0x03,                         //bNumInterfaces, AC + AS Out + AS In
  0x01,                         //bConfigurationValue
  0x00,                         //iConfiguration
#ifdef SUPPORT_REMOTE_WAKEUP
  0xA0,                         //bmAttributes, remote wakeup
#else
  0x80,                         //bmAttributes
#endif
  0x32,                         //bMaxPower, 100mA
  // ===== Standard Audio Control Interface =====
  0x09,                         //bLength
  0x04,                         //bDescriptorType, INTERFACE
  USBD1_IF_NUM_AC,              //bInterfaceNumber
  0x00,                         //bAlternateSetting
  0x00,                         //bNumEndpoints, only Use EP0
  AUDIO_CLASS_CODE,             //bInterfaceClass, AUDIO interface class
  AUDIO_CONTROL,                //bInterfaceSubClass, AUDIOCONTROL subclass
  0x00,                         //bInterfaceProtocol
  0x00,                         //iInterface
  // ===== Class-specific AC Interface Head =====
  0x0A,                         //bLength
  CS_INTERFACE,                 //bDescriptorType
  HEADER,                       //bDescriptorSubtype
  0x00,                         //bcdADC Low Byte
  0x01,                         //bcdADC High Byte
  0x48,                         //wTotalLength Low Byte
  0x00,                         //wTotalLength High Byte
  0x02,                         //bInCollection, AS out + AS in
  USBD1_IF_NUM_AS_OUT,          //baInterfaceNr(1), AS Out
  USBD1_IF_NUM_AS_IN,           //baInterfaceNr(2), AS In
  // ===== Input Terminal Descriptor =====
  0x0C,                         //bLength
  CS_INTERFACE,                 //bDescriptorType
  INPUT_TERMINAL,               //bDescriptorSubtype
  UNITID_INPUTL_USB_OUT,        //bTerminalID
  TERM_TYPE_USBSTREAMING_LOW,   //wTerminalType Low Byte, USB Streaming
  TERM_TYPE_USBSTREAMING_HIGH,  //wTerminalType High Bye
  0x00,                         //bAssocTerminal
  0x02,                         //bNrChannels, 2 logical output channels
  0x03,                         //wChannelConfig Low Byte
  0x00,                         //wChannelConfig High Byte
  0x00,                         //iChannelNames
  0x00,                         //iTerminal
  // ===== Input Terminal Descriptor =====
  0x0C,                         //bLength
  CS_INTERFACE,                 //bDescriptorType
  INPUT_TERMINAL,               //bDescriptorSubtype
  UNITID_INPUT_MIC_IN,          //bTerminalID
  TERM_TYPE_MICROPHONE_LOW,     //wTerminalType Low Byte
  TERM_TYPE_MICROPHONE_HIGH,    //wTerminalType High Bye
  0x00,                         //bAssocTerminal
  0x02,                         //bNrChannels
  0x03,                         //wChannelConfig Low Byte
  0x00,                         //wChannelConfig High Byte
  0x00,                         //iChannelNames
  0x00,                         //iTerminal
  // ===== OutPut Terminal Descriptor =====
  0x09,                         //bLength
  CS_INTERFACE,                 //bDescriptorType
  OUTPUT_TERMINAL,              //bDescriptorSubtype
  UNITID_OUTPUT_LINE_OUT,       //bTerminalID
  TERM_TYPE_SPEAKER_LOW,        //wTerminalType Low Byte, Speaker
  TERM_TYPE_SPEAKER_HIGH,       //wTerminalType High Bye
  0x00,                         //bAssocTerminal
  UNITID_FEATURE_DAC_VOLUME,    //bSourceID
  0x00,                         //iTerminal
  // ===== OutPut Terminal Descriptor =====
  0x09,                         //bLength
  CS_INTERFACE,                 //bDescriptorType
  OUTPUT_TERMINAL,              //bDescriptorSubtype
  UNITID_OUTPUT_USB_IN,         //bTerminalID
  TERM_TYPE_USBSTREAMING_LOW,   //wTerminalType Low Byte, USB Streaming
  TERM_TYPE_USBSTREAMING_HIGH,  //wTerminalType High Bye
  0x00,                         //bAssocTerminal, according to MS white paper
  UNITID_FEATURE_ADC_VOLUME,    //bSourceID
  0x00,                         //iTerminal
  // ===== Feature Unit Descriptor =====
  0x0A,                         //bLength
  CS_INTERFACE,                 //bDescriptorType
  FEATURE_UNIT,                 //bDescriptorSubtype
  UNITID_FEATURE_DAC_VOLUME,    //bUnitID
  UNITID_INPUTL_USB_OUT,        //bSourceID
  0x01,                         //bControlSize
  MUTE_CONTROL_BITMAP|VOLUME_CONTROL_BITMAP,          //bmaControl(0), master channel, mute
  0x00,//VOLUME_CONTROL_BITMAP,        //bmaControl(1), left channel, volume
  0x00,//VOLUME_CONTROL_BITMAP,        //bmaControl(2), right channel, volume
  0x00,                         //iFeature
  // ===== Feature Unit Descriptor =====
  0x0A,                         //bLength
  CS_INTERFACE,                 //bDescriptorType
  FEATURE_UNIT,                 //bDescriptorSubtype
  UNITID_FEATURE_ADC_VOLUME,    //bUnitID
  UNITID_INPUT_MIC_IN,          //bSourceID
  0x01,                         //bControlSize
  MUTE_CONTROL_BITMAP|VOLUME_CONTROL_BITMAP,          //bmaControl(0), master channel, mute
  0x00,//VOLUME_CONTROL_BITMAP,        //bmaControl(1), left channel, volume
  0x00,//VOLUME_CONTROL_BITMAP,        //bmaControl(2), right channel, volume
  0x00,                         //iFeature
  // ===== Standard AudioStreaming Interface =====
  0x09,                         //bLength
  USB_DESCRIPTOR_INTERFACE,     //bDescriptorType
  USBD1_IF_NUM_AS_OUT,          //bInterfaceNumber
  0x00,                         //bAlternateSetting, Alt. setting 0 (Zero-bandwidth)
  0x00,                         //bNumEndpoints, only Use EP0
  AUDIO_CLASS_CODE,             //bInterfaceClass, AUDIO interface class
  AUDIO_STREAMING,              //bInterfaceSubclass, AUDIOSTREAMING subclass
  0x00,                         //bInterfaceProtocol
  0x00,                         //iInterface
  // ===== Standard AudioStreaming Interface =====
  0x09,                         //bLength
  USB_DESCRIPTOR_INTERFACE,     //bDescriptorType
  USBD1_IF_NUM_AS_OUT,          //bInterfaceNumber
  0x01,                         //bAlternateSetting
  0x01,                         //bNumEndpoints, Use EP0+EP1
  AUDIO_CLASS_CODE,             //bInterfaceClass, AUDIO interface class
  AUDIO_STREAMING,              //bInterfaceSubclass, AUDIOSTREAMING subclass
  0x00,                         //bInterfaceProtocol
  0x00,                         //iInterface
  // ===== Class-specific Audio Streaming Interface =====
  0x07,                         //bLength
  CS_INTERFACE,                 //bDescriptorType
  AS_GENERAL,                   //bDescriptorSubtype
  UNITID_INPUTL_USB_OUT,        //bTerminalLink
  0x01,                         //bDelay
  FORMAT_PCM_LOW,               //bFormatTag Low Byte, PCM
  FORMAT_PCM_HIGH,              //bFormatTag High Byte
  // ===== Type 1 Format Type Descriptor =====
  0x14,                         //bLength
  CS_INTERFACE,                 //bDescriptorType
  FORMAT_TYPE,                  //bDescriptorSubtype
  FORMAT_TYPE_I,                //bFormatType
  0x02,                         //bNrChannels
  SUBFRAMESIZE_16BIT,           //bSubFrameSize
  BITRESOLUTION_16BIT,          //bBitResolution
  0x02+1+1,
  SAMPLERATE_48K_BYTE0,         //tLowerSamFreq, 48k
  SAMPLERATE_48K_BYTE1,         //tLowerSamFreq
  SAMPLERATE_48K_BYTE2,         //tLowerSamFreq
  SAMPLERATE_44K_BYTE0,        //tLowerSamFreq, 44.1k
  SAMPLERATE_44K_BYTE1,        //tLowerSamFreq
  SAMPLERATE_44K_BYTE2,        //tLowerSamFreq
  SAMPLERATE_16K_BYTE0,        //tLowerSamFreq, 44.1k
  SAMPLERATE_16K_BYTE1,        //tLowerSamFreq
  SAMPLERATE_16K_BYTE2,        //tLowerSamFreq
  SAMPLERATE_8K_BYTE0,        //tLowerSamFreq, 44.1k
  SAMPLERATE_8K_BYTE1,        //tLowerSamFreq
  SAMPLERATE_8K_BYTE2,        //tLowerSamFreq
  // ===== Standard Endpoint Descriptor =====
  0x07,                         //bLength
  USB_DESCRIPTOR_ENDPOINT,      //bDescriptorType
  USBD1_ISO_OUT_EP,             //bEndpointAddress, OUT EP1, Bit7(0=out 1=in)
  0x09,                         //bmAttributes, adaptive isochronous Endpoint
  MAXSIZE_2CH_16BIT_48K_LOW,    //wMaxPacketSize Low Byte
  MAXSIZE_2CH_16BIT_48K_HIGH,   //wMaxPacketSize High Byte,
  0x01,                         //bInterval, One packet every frame
  // ===== Class specific Endpoint Descriptor =====
  0x07,                         //bLength
  CS_ENDPOINT,                  //bDescriptorType
  EP_GENERAL,                   //bDescriptorSubtype
  SAMPLING_FREQ_CONTROL,        //bmAttributes
  0x01,                         //bLockDelayUnits, unit is milliseconds
  0x01,                         //wLockDelay
  0x00,                         //wLockDelay
  // ===== Standard AudioStreaming Interface =====
  0x09,                         //bLength
  USB_DESCRIPTOR_INTERFACE,     //bDescriptorType
  USBD1_IF_NUM_AS_IN,           //bInterfaceNumber
  0x00,                         //bAlternateSetting, Alt. setting 0
  0x00,                         //bNumEndpoints, only Use EP0
  AUDIO_CLASS_CODE,             //bInterfaceClass, AUDIO interface class
  AUDIO_STREAMING,              //bInterfaceSubclass, AUDIOSTREAMING subclass
  0x00,                         //bInterfaceProtocol
  0x00,                         //iInterface
  // ===== Standard AudioStreaming Interface =====
  0x09,                         //bLength
  USB_DESCRIPTOR_INTERFACE,     //bDescriptorType
  USBD1_IF_NUM_AS_IN,           //bInterfaceNumber
  0x01,                         //bAlternateSetting
  0x01,                         //bNumEndpoints, Use EP0+EP81
  AUDIO_CLASS_CODE,             //bInterfaceClass, AUDIO interface class
  AUDIO_STREAMING,              //bInterfaceSubclass, AUDIOSTREAMING subclass
  0x00,                         //bInterfaceProtocol
  0x00,                         //iInterface
  // ===== Class-specific Audio Streaming Interface =====
  0x07,                         //bLength
  CS_INTERFACE,                 //bDescriptorType
  AS_GENERAL,                   //bDescriptorSubtype
  UNITID_OUTPUT_USB_IN,         //bTerminalLink
  0x01,                         //bDelay
  FORMAT_PCM_LOW,               //bFormatTag Low Byte, PCM
  FORMAT_PCM_HIGH,              //bFormatTag High Byte
  // ===== Type 1 Format Type Descriptor =====
  0x11,                         //bLength
  CS_INTERFACE,                 //bDescriptorType
  FORMAT_TYPE,                  //bDescriptorSubtype
  FORMAT_TYPE_I,                //bFormatType
  0x02,                         //bNrChannels
  SUBFRAMESIZE_16BIT,           //bSubFrameSize
  BITRESOLUTION_16BIT,          //bBitResolution
  0x02+1,                         //bSamFreqType
  SAMPLERATE_16K_BYTE0,         //tLowerSamFreq
  SAMPLERATE_16K_BYTE1,         //tLowerSamFreq
  SAMPLERATE_16K_BYTE2,         //tLowerSamFreq
  SAMPLERATE_8K_BYTE0,          //tLowerSamFreq
  SAMPLERATE_8K_BYTE1,          //tLowerSamFreq
  SAMPLERATE_8K_BYTE2,          //tLowerSamFreq
  SAMPLERATE_11K_BYTE0,          //tLowerSamFreq
  SAMPLERATE_11K_BYTE1,          //tLowerSamFreq
  SAMPLERATE_11K_BYTE2,          //tLowerSamFreq
  // ===== Standard Endpoint Descriptor =====
  0x07,                         //bLength
  USB_DESCRIPTOR_ENDPOINT,      //bDescriptorType
  0x80|USBD1_ISO_IN_EP,         //bEndpointAddress, IN EP2, Bit7(0=out 1=in)
  0x05,                         //bmAttributes, asynch isochronous Endpoint
  MAXSIZE_2CH_16BIT_16K_LOW,    //wMaxPacketSize Low Byte
  MAXSIZE_2CH_16BIT_16K_HIGH,   //wMaxPacketSize High Byte,
  0x01,                         //bInterval, One packet every frame
  // ===== Class specific Endpoint Descriptor =====
  0x07,                         //bLength
  CS_ENDPOINT,                  //bDescriptorType
  EP_GENERAL,                   //bDescriptorSubtype
  SAMPLING_FREQ_CONTROL,        //bmAttributes
  0x00,                         //bLockDelayUnits, unit is milliseconds
  0x00,                         //wLockDelay
  0x00,                         //wLockDelay
};

static const uint8_t configuration_descriptor_data_composite_stor[] =
{
  // ===== Configuration Header Descriptor =====
  0x09,                         //bLength
  0x02,                         //bDescriptorType, CONFIGURATION
  0x20,                         //wTotalLength, low byte
  0x00,                         //wTotalLength, high byte
  0x01,                         //bNumInterfaces, Bulk
  0x01,                         //bConfigurationValue
  0x00,                         //iConfiguration
#ifdef SUPPORT_REMOTE_WAKEUP
  0xA0,                         //bmAttributes, remote wakeup
#else
  0x80,                         //bmAttributes
#endif
  0x32,                         //bMaxPower, 100mA
  // ===== Bulk Interface =====
  0x09,                         //bLength
  0x04,                         //bDescriptorType, INTERFACE
  USBD1_IF_NUM_BULK,            //bInterfaceNumber
  0x00,                         //bAlternateSetting
  0x02,                         //bNumEndpoints
  MASS_STORAGE_CLASS_CODE,      //bInterfaceClass
  MASS_STORAGE_SUBCLASS_SCSI,   //bInterfaceSubClass
  MASS_STORAGE_IFPROTOCAL_BULK, //bInterfaceProtocol
  0x04,                         //iInterface
  // ===== Standard Bulk Out Endpoint Descriptor =====
  0x07,                         //bLength
  USB_DESCRIPTOR_ENDPOINT,      //bDescriptorType
  USBD1_BULK_OUT_EP,            //bEndpointAddress
  MASS_STORAGE_ENDPOINT_BULK,   //bmAttributes, bulk Endpoint
  0x40,                         //wMaxPacketSize Low Byte
  0x00,                         //wMaxPacketSize High Byte,
  0x00,                         //bInterval
  // ===== Standard Bulk In Endpoint Descriptor =====
  0x07,                         //bLength
  USB_DESCRIPTOR_ENDPOINT,      //bDescriptorType
  0x80|USBD1_BULK_IN_EP,        //bEndpointAddress
  MASS_STORAGE_ENDPOINT_BULK,   //bmAttributes, bulk Endpoint
  0x40,                         //wMaxPacketSize Low Byte
  0x00,                         //wMaxPacketSize High Byte,
  0x00,                         //bInterval
};


/*
 * Configuration Descriptor wrapper.
 */
const USBDescriptor configuration_descriptor_composite_all = {
  sizeof configuration_descriptor_data_composite_all,
  configuration_descriptor_data_composite_all
};
const USBDescriptor configuration_descriptor_composite_audio = {
  sizeof configuration_descriptor_data_composite_audio,
  configuration_descriptor_data_composite_audio
};
const USBDescriptor configuration_descriptor_composite_stor = {
  sizeof configuration_descriptor_data_composite_stor,
  configuration_descriptor_data_composite_stor
};


/*
 * U.S. English language identifier.
 */
static const uint8_t string0_2[] = {
  USB_DESC_BYTE(4),                     /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  USB_DESC_WORD(0x0409)                 /* wLANGID (U.S. English).          */
};

/*
 * Vendor string.
 */
static const uint8_t string1_2[] = {
  USB_DESC_BYTE(20),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'H', 0, 'u', 0, 'n', 0, 't', 0, 'e', 0, 'r', 0, 's', 0, 'u', 0,
  'n', 0
};

/*
 * Device Description string.
 */
#if HAL_USE_USB_AUDIO&&HAL_USE_USB_STORAGE
static const uint8_t string2_2[] = {
  USB_DESC_BYTE(58),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'H', 0, 'S', 0, '6', 0, '6', 0, '0', 0, '1', 0, ' ', 0,
  'U', 0, 'S', 0, 'B', 0, ' ', 0,
  'A', 0, 'u', 0, 'd', 0, 'i', 0, 'o', 0, ' ', 0, 'a', 0, 'n', 0, 'd', 0, ' ', 0,
  'S', 0, 't', 0, 'o', 0, 'r', 0, 'a', 0, 'g', 0, 'e', 0,
};
#elif HAL_USE_USB_AUDIO&&(!HAL_USE_USB_STORAGE)
static const uint8_t string2_2[] = {
  USB_DESC_BYTE(34),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'H', 0, 'S', 0, '6', 0, '6', 0, '0', 0, '1', 0, ' ', 0,
  'U', 0, 'S', 0, 'B', 0, ' ', 0,
  'A', 0, 'u', 0, 'd', 0, 'i', 0, 'o', 0,
};
#else
static const uint8_t string2_2[] = {
  USB_DESC_BYTE(38),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'H', 0, 'S', 0, '6', 0, '6', 0, '0', 0, '1', 0, ' ', 0,
  'U', 0, 'S', 0, 'B', 0, ' ', 0,
  'S', 0, 't', 0, 'o', 0, 'r', 0, 'a', 0, 'g', 0, 'e', 0,
};
#endif


/*
 * Serial Number string.
 */
static const uint8_t string3_2[] = {
  USB_DESC_BYTE(26),                     /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  '0' + CH_KERNEL_MAJOR, 0,
  '.',                   0,
  '0' + CH_KERNEL_MINOR, 0,
  '.',                   0,
  '0' + CH_KERNEL_PATCH, 0,
  '.',                   0,
  '0',                   0,
  '0',                   0,
  '0',                   0,
  '0',                   0,
  '0',                   0,
  '0',                   0,
};

static const uint8_t string4_2[] = {
  USB_DESC_BYTE(54),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'H', 0, 'S', 0, '6', 0, '6', 0, '0', 0, '1', 0, ' ', 0, 'M', 0,
  'a', 0, 's', 0, 's', 0, ' ', 0, 'S', 0, 't', 0, 'o', 0, 'r', 0,
  'a', 0, 'g', 0, 'e', 0, ' ', 0, 'D', 0, 'e', 0, 'v', 0, 'i', 0,
  'c', 0, 'e', 0
};

/*
 * Strings wrappers array.
 */
const USBDescriptor strings_composite[] = {
  { sizeof string0_2, string0_2},
  { sizeof string1_2, string1_2},
  { sizeof string2_2, string2_2},
  { sizeof string3_2, string3_2},
  { sizeof string4_2, string4_2},
};


/*
 * Handles the GET_DESCRIPTOR callback. All required descriptors must be
 * handled here.
 */
static const USBDescriptor *usb_audio_get_descriptor(USBDriver *usbp,
    uint8_t dtype,
    uint8_t dindex,
    uint16_t lang)
{
  chDbgAssert(usbp, "serial_get_descriptor()");
  (void)lang;

  uint8_t hasaudio = 0;
  uint8_t hasstorage = 0;

  switch (dtype) {
    case USB_DESCRIPTOR_DEVICE:
#if HAL_USE_USB_AUDIO||HAL_USE_USB_STORAGE
    return &device_descriptor_composite;
#endif

    case USB_DESCRIPTOR_CONFIGURATION:
#if (HAL_USE_USB_AUDIO||HAL_USE_USB_STORAGE)

#endif
#if HAL_USE_USB_AUDIO
    if(USBAUD1.state == USB_AUDIO_STATE_START)
    {
      hasaudio = 1;
    }
#endif
#if HAL_USE_USB_STORAGE
    if(USBSTORAGE1.state == USB_STORAGE_STATE_START)
    {
      hasstorage = 1;
    }
#endif
#if (HAL_USE_USB_AUDIO||HAL_USE_USB_STORAGE)
    if(hasstorage&&hasaudio)
    {
      return &configuration_descriptor_composite_all;
    }
    else if(hasaudio&&(!hasstorage))
    {
      return &configuration_descriptor_composite_audio;
    }
    else if(hasstorage&&(!hasaudio))
    {
      return &configuration_descriptor_composite_stor;
    }
#endif

    case USB_DESCRIPTOR_STRING:
#if HAL_USE_USB_AUDIO||HAL_USE_USB_STORAGE
      return &strings_composite[dindex];
#endif
  }

  return NULL;
}

#define MAX_IF_NUM_COMPOSITE   4
uint8_t g_altset_composite[MAX_IF_NUM_COMPOSITE] = {0, 0, 0, 0};
bool_t usb_audio_req_handler(USBDriver *usbp)
{
  bool_t ret = FALSE;

  chDbgAssert(usbp, "null pointer");

  if ((usbp->setup[0] & USB_RTYPE_TYPE_MASK) == USB_RTYPE_TYPE_CLASS)
  {

    if((usbp->setup[0]&USB_RTYPE_RECIPIENT_MASK)==USB_RTYPE_RECIPIENT_INTERFACE)
    {
#if HAL_USE_USB_AUDIO
      if(usbp->setup[4] == dev_if_num_audio.Ac_If_Num)
      {
        ret = usbaud_class_req_handler_if(usbp);
      }
#endif
#if HAL_USE_USB_STORAGE
      if(usbp->setup[4] == dev_if_num_storage.Storage_If_Num)
      {
        //TODO
        ret = usbstorage_class_req_handler_if(usbp);
      }
#endif
    }
    else if((usbp->setup[0]&USB_RTYPE_RECIPIENT_MASK)==USB_RTYPE_RECIPIENT_ENDPOINT)
    {
#if HAL_USE_USB_AUDIO

      if(usbp->setup[4] == dev_ep_num_audio.As_Out_EpNum
          ||usbp->setup[4] == (0x80|dev_ep_num_audio.As_In_EpNum))
      {
        ret = usbaud_class_req_handler_ep(usbp,
            usbp->setup[4] == dev_ep_num_audio.
            As_Out_EpNum);
      }

#endif
    }
  }
  else if((usbp->setup[0] & USB_RTYPE_TYPE_MASK) == USB_RTYPE_TYPE_STD)
  {
    if(usbp->setup[1] == USB_REQ_SET_INTERFACE)
    {
      if(usbp->setup[4]<MAX_IF_NUM_COMPOSITE)
      {
        g_altset_composite[usbp->setup[4]] = usbp->setup[2];
      }

#if HAL_USE_USB_AUDIO
      if(usbp->setup[4]==dev_if_num_audio.As_In_IfNum||usbp->setup[4]==dev_if_num_audio.
          As_Out_IfNum)
      {
        ret = usbaud_std_req_handler_set_interface(usbp, (usbp->setup[4]==dev_if_num_audio
                .As_Out_IfNum)? TRUE:FALSE);
      }
      else if(usbp->setup[4]==dev_if_num_audio.Ac_If_Num&&usbp->setup[2]==0)
      {
        usbSetupTransfer(usbp, NULL, 0, NULL);
        usb_finish_ep0setup(usbp, 0);
        ret = TRUE;
      }
      else
#endif
      {
#if HAL_USE_USB_STORAGE
        if(usbp->setup[4]==dev_if_num_storage.Storage_If_Num&&usbp->setup[2]==0)
        {
          usbSetupTransfer(usbp, NULL, 0, NULL);
          usb_finish_ep0setup(usbp, 0);
          ret = TRUE;
        }
#endif
      }
    }
    else if(usbp->setup[1] == USB_REQ_GET_INTERFACE)
    {

      if(usbp->setup[4]<MAX_IF_NUM_COMPOSITE)
      {
        usbSetupTransfer(usbp, &g_altset_composite[usbp->setup[4]], 1, NULL);
        usb_finish_ep0setup(usbp, 0);
        ret = TRUE;
      }

    }
  }

  return ret;
}

void usb_event_composite(USBDriver *usbp, usbevent_t event) {
  
#if HAL_USE_USB_AUDIO
  if(USBAUD1.state == USB_AUDIO_STATE_START)      
  {
    usb_event_audio(usbp,event); 
  }
#endif

#if HAL_USE_USB_STORAGE
  if(USBSTORAGE1.state == USB_STORAGE_STATE_START)
  {
    usb_event_storage(usbp,event); 
  }
#endif
}

const USBConfig usbAudioStorageCfg = {
  usb_event_composite,
  usb_audio_get_descriptor,
  usb_audio_req_handler,
  NULL
};

bool usbStorageSdInsert(void)
{
  return hs_fatfs_isMount(FATFS_MEMDEV_SD);
}

int32_t usbAudioStorage_open(const void * pArg)
{
  (void)pArg;

#if HAL_USE_USB_AUDIO
  usbaudObjectInit(&USBAUD1, &USBD1);
  usbaudSetAsIfInfo(&USBAUD1, if_info);
  usbaudStart(&USBAUD1);
#endif

#if HAL_USE_USB_STORAGE
  usbstorageObjectInit(&USBSTORAGE1, &USBD1);   
  usbstorageSetSdcCfg(&USBSTORAGE1, NULL);
  usbstorageStart(&USBSTORAGE1, usbStorageSdInsert); 
#endif

  return 0;
}

void usbAudioStorage_close(void)
{
#if HAL_USE_USB_AUDIO
  usbaudStop();
#endif

#if HAL_USE_USB_STORAGE
  usbstorageStop(); 
#endif
}

hs_usbdev_t g_stAudioStorageDev =
{
  &usbAudioStorageCfg,
  usbAudioStorage_open,
  usbAudioStorage_close,
  0,
};

#endif



