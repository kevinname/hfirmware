#include "usbsubdev_hs6200.h"
#include <string.h>

#if HAL_USE_USB_BULK_HS6200

#include <string.h>
#include <stdlib.h>
#include "hal.h"
#include "chprintf.h"
#include "cfg_define.h"
#include "lib.h"
#include "HS6200_test_sys.h"
#include "HS6200_Analog_Test.h"
#include "HS6200_types.h"
#include "nRF24L01_common.h"
#include "nRF24L01_X.h"
#include "C8051F_USB.h"

#define THD_STATE_SUSPENDED     2

static uint8_t g_usbHs6200State = USB_HS6200_STOP;
mailbox_t g_hs6200_mbox ;
thread_t * g_thread_write = NULL;
uint8_t g_ReadBuf[USB_HS6200_BULK_OUT_PKTSIZ] = {0};
uint8_t g_ReadBufLen = 0;
msg_t g_sample_message[1][4];

extern void usb_recevice_process(void);
extern void usb_Transmitted_process(void);
extern uint8_t string3_hs6200[];

//NumBytes: 0 - 4096
//send data to usb host
uint16_t Block_Write (uint8_t *Buffer, uint16_t NumBytes)
{
    uint8_t bytesTrans = 0;
    osalDbgAssert(Buffer,
                "Block_Write()");
    chSysLock();
    while(bytesTrans<NumBytes)
    {
        uint8_t len = ((NumBytes-bytesTrans)>USB_HS6200_BULK_IN_PKTSIZ)?USB_HS6200_BULK_IN_PKTSIZ:(NumBytes-bytesTrans);
        usbPrepareTransmit(&USBD1, 
                           USB_HS6200_BULK_IN_EP, 
                           &Buffer[bytesTrans], 
                           len, 
                           DMA_MODE_NONE);
        usbStartTransmitI(&USBD1, USB_HS6200_BULK_IN_EP);

        
        g_thread_write = chThdGetSelfX();
        chSchGoSleepS(THD_STATE_SUSPENDED);
        
        bytesTrans +=len; 
    }
    chSysUnlock();
    return bytesTrans;
}


//NumBytes: 0 - 64
//get data from usb host
uint8_t Block_Read (uint8_t *Buffer, uint8_t NumBytes)
{
    uint8_t bytesRecv = 0;
    uint8_t i;
    osalDbgAssert(Buffer,
                "Block_Read()");

    if(!NumBytes)
    {
        return 0;
    }
    
    bytesRecv = usb_get_rx_data_len(USB_HS6200_BULK_OUT_EP);
    usbPrepareReceive(&USBD1, 
                      USB_HS6200_BULK_OUT_EP, 
                      g_ReadBuf, 
                      bytesRecv, 
                      DMA_MODE_NONE);
    usbStartReceiveI(&USBD1,USB_HS6200_BULK_OUT_EP);
    bytesRecv = (bytesRecv>NumBytes)?NumBytes:bytesRecv;
    for(i=0;i<bytesRecv;i++)
    {
        Buffer[i] = g_ReadBuf[i];
    }

    return bytesRecv;
}

void UsbHs6200_StartUsb()
{
    g_usbHs6200State = USB_HS6200_STOP;
    //UsbHs6200_SetMailBox(pmbox);
//    usbSetUsbCfg(&usbcfg);
    //usbStart(&USBD1);
    //usbConnectBus();  
    
} 

void UsbHs6200_StopUsb(void)
{
    usbStop(&USBD1);
    usbDisconnectBus(&USBD1);
    g_usbHs6200State = USB_HS6200_STOP;
} 

//bool_t UsbHs6200_IsReady(void)
//{
//    return g_usbHs6200State == USB_HS6200_READY&&g_hs6200_mbox;
//}
//
//void UsbHs6200_SetMailBox(mailbox_t *pmbox)
//{
//    osalDbgAssert(pmbox,
//                "UsbHs6200_SetMailBox()");
//    g_hs6200_mbox = pmbox;
//}
    
static void usbbulkDataTransmitted(USBDriver *usbp, usbep_t ep)
{
	(void)ep;
    osalDbgAssert(usbp,
                "usbbulkDataTransmitted()");
    chDbgCheckClassI();
    if (g_thread_write != NULL) {
        chSchReadyI(g_thread_write);
        g_thread_write = NULL;
    }
    
#if USB_USE_TEST_MAC6200   //hsz
    usb_Transmitted_process();
#endif
}




//uint8_t g_tmpBuf[USB_HS6200_BULK_OUT_PKTSIZ];
static void usbbulkDataReceived(USBDriver *usbp, usbep_t ep)
{
    //uint8_t cnt = 0;
    osalDbgAssert(usbp,
                "usbbulkDataReceived()");
    chDbgCheckClassS();

    chMBPostS(&g_hs6200_mbox, usb_get_rx_data_len(ep), TIME_INFINITE);
}

void usb_hs6200_reset(USBDriver *usbp) 
{
	(void)usbp;
}

static const uint8_t device_descriptor_data_hs6200[18] = {
  USB_DESC_DEVICE       (0x0110,        /* bcdUSB (1.1).            */
                         0x00,          /* bDeviceClass.  */
                         0x00,          /* bDeviceSubClass.      */
                         0x00,          /* bDeviceProtocol.        */
                         0x40,          /* bMaxPacketSize.        */
                         USB_VID_HS6200, /* idVendor (HS).         */
                         USB_PID_HS6200, /* idProduct.      */
                         0x0100,        /* bcdDevice.                 */
                         USB_STR_DESC_IDX_VENDOR, /* iManufacturer. */
                         USB_STR_DESC_IDX_PROD,/* iProduct.*/
                         0x03,             /* iSerialNumber.             */
                         0x01)             /* bNumConfigurations.    */ 
};

const USBDescriptor device_descriptor_hs6200 = {
  sizeof device_descriptor_data_hs6200,
  device_descriptor_data_hs6200
};

static const uint8_t configuration_descriptor_data_hs6200[] = 
{
  // ===== Configuration Header Descriptor =====
  0x09,                         //bLength
  0x02,                         //bDescriptorType, CONFIGURATION
  0x20,                         //wTotalLength, low byte 
  0x00,                         //wTotalLength, high byte
  0x01,                         //bNumInterfaces, Bulk  
  0x01,                         //bConfigurationValue
  0x00,                         //iConfiguration
  0xA0,                         //bmAttributes
  0x0f,                         //bMaxPower, 30mA
  // ===== Bulk Interface =====
  0x09,                         //bLength 
  0x04,                         //bDescriptorType, INTERFACE
  USB_HS6200_IF_NUM_BULK,            //bInterfaceNumber
  0x00,                         //bAlternateSetting
  0x02,                         //bNumEndpoints
  0xff,                         //bInterfaceClass
  0x00,                         //bInterfaceSubClass
  0x00,                         //bInterfaceProtocol
  0x00,                         //iInterface
  // ===== Standard Bulk Out Endpoint Descriptor =====
  0x07,                         //bLength
  USB_DESCRIPTOR_ENDPOINT,      //bDescriptorType
  USB_HS6200_BULK_OUT_EP,       //bEndpointAddress
  0x02,                         //bmAttributes, bulk Endpoint
  USB_HS6200_BULK_OUT_PKTSIZ,   //wMaxPacketSize Low Byte
  0x00,                         //wMaxPacketSize High Byte, 
  0x00,                         //bInterval
  // ===== Standard Bulk In Endpoint Descriptor =====
  0x07,                         //bLength
  USB_DESCRIPTOR_ENDPOINT,      //bDescriptorType
  0x80|USB_HS6200_BULK_IN_EP,   //bEndpointAddress
  0x02,                         //bmAttributes, bulk Endpoint
  USB_HS6200_BULK_IN_PKTSIZ,    //wMaxPacketSize Low Byte
  0x00,                         //wMaxPacketSize High Byte, 
  0x00,                         //bInterval
};


/*
 * Configuration Descriptor wrapper.
 */
const USBDescriptor configuration_descriptor_hs6200 = {
  sizeof configuration_descriptor_data_hs6200,
  configuration_descriptor_data_hs6200
};
/*
 * U.S. English language identifier.
 */
static const uint8_t string0_hs6200[] = {
  USB_DESC_BYTE(4),                     /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  USB_DESC_WORD(0x0409)                 /* wLANGID (U.S. English).          */
};

/*
 * Vendor string.
 */
static const uint8_t string1_hs6200[] = {
   0x1A,
   USB_DESCRIPTOR_STRING,
   'S',0,
   'i',0,
   'l',0,
   'i',0,
   'c',0,
   'o',0,
   'n',0,
   ' ',0,
   'L',0,
   'a',0,
   'b',0,
   's',0
};

/*
 * Device Description string.
 */
static const uint8_t string2_hs6200[] = {
   0x10,
   USB_DESCRIPTOR_STRING,
   'U',0,
   'S',0,
   'B',0,
   ' ',0,
   'A',0,
   'P',0,
   'I',0
};

uint8_t string3_hs6200[]= {
  0x0A,
  USB_DESCRIPTOR_STRING,
  'D',  0,
  'E',  0,
  'V',  0,
  '0',  0,
};

const USBDescriptor strings_hs6200[] = {
  {sizeof string0_hs6200, string0_hs6200},
  {sizeof string1_hs6200, string1_hs6200},
  {sizeof string2_hs6200, string2_hs6200},
  {10                   , string3_hs6200},
};


const If_Num dev_if_num_hs6200= {
  0xff, //.Ac_If_Num
  0xff, //.As_Out_IfNum
  0xff, //.As_In_IfNum 
  0xff, //.Serial_IfNum
  USB_HS6200_IF_NUM_BULK,
};

/**
 * @brief   EP2 initialization structure (IN only).
 */
const USBEndpointConfig ep2config_hs6200 = {
  USB_EP_MODE_TYPE_BULK,
  NULL,
  usbbulkDataTransmitted,
  usbbulkDataReceived,
  0x0040,
  0x0040,
  &ep2instate_hs6200,
  &ep2outstate_hs6200,
  1,
  NULL,
  {
    {0xFF, 0xFF},
    {DMA_MODE_NONE, DMA_MODE_NONE},
    NULL,
    NULL,    
  },
  FALSE,
  TRUE
};

/**
 * @brief   IN EP2 state.
 */
USBInEndpointState ep2instate_hs6200;
/**
 * @brief   OUT EP2 state.
 */
USBOutEndpointState ep2outstate_hs6200;

const Fifo_Cfg fifo_cfg_hs6200[] = 
{
    //EP2 out
    {
        0x02,
        0,
        64,
        64,
    },
    //EP2 IN
    {
        0x82,
        0,
        128,
        64,
    }   
};

void usb_event_hs6200(USBDriver *usbp, usbevent_t event) {

    osalDbgAssert(usbp,
              "usb_event_hs6200()");
    
  switch (event) {
  case USB_EVENT_RESET:
    fifo_config(sizeof(fifo_cfg_hs6200)/sizeof(Fifo_Cfg), fifo_cfg_hs6200);
    usb_hs6200_reset(usbp);
    return;
  case USB_EVENT_ADDRESS:
    return;
  case USB_EVENT_CONFIGURED:
    usbInitEndpointI(usbp, USB_HS6200_BULK_OUT_EP,&ep2config_hs6200);
    g_usbHs6200State = USB_HS6200_READY;
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

static uint8_t req_buf[1] = {0x80};
bool_t usb_hs6200_vendor_req_handler(USBDriver *usbp)
{
    bool_t ret = FALSE;
    if((usbp->setup[0] & USB_RTYPE_TYPE_MASK) == USB_RTYPE_TYPE_VENDOR) 
    {
        if(usbp->setup[6]==0x00
           &&usbp->setup[7]==0x00)
        {
            usbSetupTransfer(usbp, NULL, 0, NULL);
            usb_finish_ep0setup(usbp, 0);
            ret = TRUE;  
        }
        else if((usbp->setup[0]&USB_RTYPE_DIR_DEV2HOST) == USB_RTYPE_DIR_DEV2HOST)
        {
            if(usbp->setup[1]==0xff
               &&usbp->setup[2]==0x0b
               &&usbp->setup[3]==0x37
               &&usbp->setup[4]==0x00
               &&usbp->setup[5]==0x00
               &&usbp->setup[6]==0x01
               &&usbp->setup[7]==0x00)
            {
                usbSetupTransfer(usbp, req_buf, 1, NULL);
                usb_finish_ep0setup(usbp, 0);
                ret = TRUE;  
            }
        }
             
    }
    return ret;
}

USBDescriptor g_usbstring;
USB_CFG_INFO  g_UsbCfgInfo =
{
  0x80000006, 0xc, 0x1, 0x660101bf,
  "Huntersun",
  "HS6601 Serial"
};
/*
 * Handles the GET_DESCRIPTOR callback. All required descriptors must be
 * handled here.
 */
static const USBDescriptor *usb_hs6200_get_descriptor(USBDriver *usbp,
                                           uint8_t dtype,
                                           uint8_t dindex,
                                           uint16_t lang) {

	(void)usbp;
	 (void)lang;
  switch (dtype) {
  case USB_DESCRIPTOR_DEVICE:
        return &device_descriptor_hs6200;
  case USB_DESCRIPTOR_CONFIGURATION:
        return &configuration_descriptor_hs6200;
    case USB_DESCRIPTOR_STRING:
    {
        if(dindex==USB_STR_DESC_IDX_VENDOR&&g_UsbCfgInfo.Usb_VendorStr[0])
        {
            g_usbstring.ud_size = g_UsbCfgInfo.Usb_VendorStr[0];
            g_usbstring.ud_string = (uint8_t *)g_UsbCfgInfo.Usb_VendorStr;
            return &g_usbstring;
        }
        else if(dindex==USB_STR_DESC_IDX_PROD&&g_UsbCfgInfo.Usb_ProdStr[0])
        {
            g_usbstring.ud_size = g_UsbCfgInfo.Usb_ProdStr[0];
            g_usbstring.ud_string = (uint8_t *)g_UsbCfgInfo.Usb_ProdStr;
            return &g_usbstring;
        }
        else if (dindex <= 4)
        {
            return &strings_hs6200[dindex];
        }
        return NULL;
    }
  }
  return NULL;
}

static void UsbHs6200Thread(void *arg)
{
	(void)arg;
	msg_t msg;
	while(1)
	{
		//   usbhidp = (USBHidDriver *)arg;
		chMBFetch(&g_hs6200_mbox, &msg, TIME_INFINITE);
		//todo
		usb_recevice_process();

	}
}

static void USBRfProtocolThread(void *arg)
{
  (void)arg;
  while(1) {
	  chThdSleepMilliseconds(1);
	  USB_Protocol_Resolut();   //上下位机通讯协议解析
  }
}

extern RFDriver MAC6200_default_config;
int32_t hs6200Usb_open(const void * pArg)
{
  (void)pArg;
  osThreadDef_t thdDef;

  MAX2829_init();
  MAC6200_PHY_Enable();

  MAC6200_default_config.Role = COMROLE_PRX;
  MAC6200_Init(&MAC6200_default_config);
  Dev_Flag[DEV_0]=MAC6200_DEV;

  chMBObjectInit(&g_hs6200_mbox, g_sample_message[0], 4);
  thdDef.pthread   = (os_pthread)UsbHs6200Thread;
  thdDef.stacksize = 1024;
  thdDef.tpriority = osPriorityNormal;

  oshalThreadCreate(&thdDef, NULL);

  thdDef.pthread   = (os_pthread)USBRfProtocolThread;
  thdDef.stacksize = 1024 * 4;
  thdDef.tpriority = osPriorityNormal;

  oshalThreadCreate(&thdDef, NULL);

  return 0;
}

const USBConfig hs6200UsbCfg = {
  usb_event_hs6200,
  usb_hs6200_get_descriptor,
  usb_hs6200_vendor_req_handler,
  NULL
};

hs_usbdev_t g_stHS6200Dev =
{
  &hs6200UsbCfg,
  hs6200Usb_open,
  NULL,
  NULL,
};



#endif



