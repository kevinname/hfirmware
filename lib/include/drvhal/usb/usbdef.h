#ifndef _USB_DEF_H_
#define _USB_DEF_H_

#include "ch.h"
#include "hal.h"

#define USB_ALL_DEVICE HAL_USE_USB_SERIAL\
                       ||HAL_USE_USB_AUDIO\
                       ||HAL_USE_USB_STORAGE\
                       ||HAL_USE_USB_HID_KB_MS\
                       ||HAL_USE_USB_BULK_HS6200\
                       ||HAL_USE_USB_HCI_DONGLE\
                       ||HAL_USE_USB_HID_SERIAL

#if USB_ALL_DEVICE

/*===========================================================================*/
/* USB related stuff.                                                        */
/*===========================================================================*/

#define USB_VID_HS              0xF012 
#define USB_VID_HS_HCI          0x0A12

#define USB_PID_HS_SERIAL       0x0001
#define USB_PID_HS_STORAGE_ONLY 0x6002
#define USB_PID_HS_AUDIO_ONLY   0x0003
#define USB_PID_HS_COMPOSITE    0x0004
#define USB_PID_HS_HID          0x0005
#define USB_PID_HS_HCI          0x0001//0x0006
#define USB_PID_HS_HID_SERIAL   0x6600

/*
 * Endpoints to be used for USBD1.
 */
#define USB_DEV_MODE_NONE             0
#define USB_DEV_MODE_SERIAL_ONLY      1
#define USB_DEV_MODE_COMPOSITE        2//composite for audio and storage
#define USB_DEV_MODE_HID_KB_MS        3 
#define USB_DEV_MODE_HS6200           4 
#define USB_DEV_MODE_HCI_DONGLE       5
#define USB_DEV_MODE_HID_SERIAL       6

//#define SUPPORT_AUDIO           TRUE
//#define SUPPORT_STORAGE         FALSE
#define USBD1_IF_NUM_AC         0
#define USBD1_IF_NUM_AS_OUT     1
#define USBD1_IF_NUM_AS_IN      2
#if HAL_USE_USB_AUDIO
#define USBD1_IF_NUM_BULK       3
#else
#define USBD1_IF_NUM_BULK       0
#endif

#define USBD1_ISO_OUT_EP        1 //audio out endpoint
#define USBD1_ISO_IN_EP         USBD1_ISO_OUT_EP //audio in endpoint
#define USBD1_BULK_OUT_EP       2 //storage out endpoint
#define USBD1_BULK_IN_EP        USBD1_BULK_OUT_EP //storage in endpoint
#define USBD1_DATA_REQUEST_EP           1
#define USBD1_DATA_AVAILABLE_EP         1
#define USBD1_INTERRUPT_REQUEST_EP      2

//#define SUPPORT_REMOTE_WAKEUP

#define USB_STR_DESC_IDX_VENDOR   1
#define USB_STR_DESC_IDX_PROD     2

#define CFG_LEN_USB_STR           32


typedef struct _IF_NUM
{
    uint8_t Ac_If_Num;
    uint8_t As_Out_IfNum;
    uint8_t As_In_IfNum;
    uint8_t Serial_IfNum;
    uint8_t Storage_If_Num;
}If_Num;

typedef struct _EP_NUM
{
    uint8_t As_Out_EpNum;
    uint8_t As_In_EpNum;
}As_Ep_Num;

#endif

#endif
