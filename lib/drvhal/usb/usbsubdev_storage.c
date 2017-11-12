#include "usbsubdev_storage.h"
#include "lib.h"

#if HAL_USE_USB_STORAGE

const If_Num dev_if_num_storage= {
  0xff, //.Ac_If_Num
  0xff, //.As_Out_IfNum
  0xff, //.As_In_IfNum 
  0xff, //.Serial_IfNum
  USBD1_IF_NUM_BULK, //.Storage_If_Num
};

/**
 * @brief   EP2 initialization structure .
 */
const USBEndpointConfig ep2config_bulk = {
  USB_EP_MODE_TYPE_BULK,
  NULL,
  usbstorageDataTransmitted,
  usbstorageDataReceived,
  0x0040,
  0x0040,
  &ep2instate_bulk,
  &ep2outstate_bulk,
  1,
  NULL,
  {
    {USB_DMA_CH_STOR_OUT, USB_DMA_CH_STOR_IN},
    {DMA_MODE_0, DMA_MODE_1},
    usbstorage_dma_outep_cb,
    usbstorage_dma_inep_cb,    
  },
  TRUE,
  TRUE
};

/**
 * @brief   IN EP2 state.
 */
USBInEndpointState ep2instate_bulk;
/**
 * @brief   OUT EP2 state.
 */
USBOutEndpointState ep2outstate_bulk;

const Fifo_Cfg fifo_cfg_bulk[] = 
{
    //EP2 out
    {
        0x02,
        0,
        384,
        64,
    },
    //EP2 IN
    {
        0x82,
        0,
        448,
        64,
    }   
};

void usb_event_storage(USBDriver *usbp, usbevent_t event) {

    chDbgAssert(usbp,
              "func()");
    
  switch (event) {
  case USB_EVENT_RESET:
    fifo_config(sizeof(fifo_cfg_bulk)/sizeof(Fifo_Cfg), fifo_cfg_bulk);
    usb_storage_reset(usbp);
    return;
  case USB_EVENT_ADDRESS:
    return;
  case USB_EVENT_CONFIGURED:
    //chSysLockFromIsr();
    /* Enables the endpoints specified into the configuration.
       Note, this callback is invoked from an ISR so I-Class functions
       must be used.*/
    usbInitEndpointI(usbp, USBD1_BULK_OUT_EP,&ep2config_bulk);
    //usbInitEndpointI(usbp, 1,&ep2config_bulk);
    //chSysUnlockFromIsr();

    chSysUnlockFromISR();
    //hs_cfg_sysReqPerip(HS_CFG_EVENT_USB_STORAGERDY);
    chSysLockFromISR();
    
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



