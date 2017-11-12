/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
/*
    PengJiang, 20140710
*/

#ifndef _USB_LLD_H_
#define _USB_LLD_H_

#if HAL_USE_USB

#define USB_POWER_CONTROL_BY_HW             0x00000000
#define USB_POWER_CONTROL_BY_REG            0x00000001
#define USB_POWER_CONTROL_BY_GPIO           0x00000002

#if !defined(HS_USB_USB_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define HS_USB_USB_IRQ_PRIORITY           3
#endif

#if !defined(HS_USB_USB_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define HS_USB_USB_DMA_IRQ_PRIORITY       2
#endif


/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   Maximum endpoint address.
 */
#define USB_ENDOPOINTS_NUMBER               7
#define USB_MAX_ENDPOINTS                   USB_ENDOPOINTS_NUMBER

#define USB_CTRL_EP_PKT_SIZ                 64
#define USB_BULK_EP_PKT_SIZ                 64
#define USB_INT_EP_PKT_SIZ                  64


#define USB_DMA_CH_AUD_OUT                  2
#define USB_DMA_CH_AUD_IN                   3
#define USB_DMA_CH_STOR_OUT                 0
#define USB_DMA_CH_STOR_IN                  1
#define USB_DMA_MAX_CHANNELS                4

#define DMA_MODE_NONE                       0
#define DMA_MODE_0                          1
#define DMA_MODE_1                          2

#define USB_MODE_HOST                       0x00
#define USB_MODE_DEV                        0x01
#define USB_MODE_UNKNOWN                    0xff

#define USB_POWER_VBUS_NONE                 0x00
#define USB_POWER_VBUS_VALID                0x01
#define USB_POWER_VBUS_SESS_VALID           0x02
#define USB_POWER_VBUS_LO_VALID             0x04




#define USB_EPxFIFO_OFFSET(ep)               (0x20+(ep)*4)

/**
 * @brief   This device requires the address change after the status packet.
 */
#define USB_SET_ADDRESS_MODE                USB_LATE_SET_ADDRESS

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   USB1 driver enable switch.
 * @details If set to @p TRUE the support for USB1 is included.
 * @note    The default is @p TRUE.
 */


/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of an IN endpoint state structure.
 */
typedef struct {
  /**
   * @brief   Buffer mode, queue or linear.
   */
  bool_t                        txqueued;
  /**
   * @brief   Requested transmit transfer size.
   */
  size_t                        txsize;
  /**
   * @brief   Transmitted bytes so far.
   */
  size_t                        txcnt;
  union {
    struct {
      /**
       * @brief   Pointer to the transmission linear buffer.
       */
      const uint8_t             *txbuf;
    } linear;
    struct {
      /**
       * @brief   Pointer to the output queue.
       */
      output_queue_t           *txqueue;
    } queue;
    /* End of the mandatory fields.*/
  } mode;
  uint8_t                      dma_mode;
} USBInEndpointState;

/**
 * @brief   Type of an OUT endpoint state structure.
 */
typedef struct {
  /**
   * @brief   Buffer mode, queue or linear.
   */
  bool_t                        rxqueued;
  /**
   * @brief   Requested receive transfer size.
   */
  size_t                        rxsize;
  /**
   * @brief   Received bytes so far.
   */
  size_t                        rxcnt;
  union {
    struct {
      /**
       * @brief   Pointer to the receive linear buffer.
       */
      uint8_t                   *rxbuf;
    } linear;
    struct {
      /**
       * @brief   Pointer to the input queue.
       */
      input_queue_t            *rxqueue;
    } queue;
  } mode;
  /* End of the mandatory fields.*/
  /**
   * @brief   Number of packets to receive.
   */
  uint8_t                      dma_mode;
} USBOutEndpointState;

typedef struct{
    uint8_t         dma_ch[2];
    uint8_t         dma_mode[2];
    usbepcallback_t dma_outep_cb;
    usbepcallback_t dma_inep_cb;
}usb_dma_info;

/**
 * @brief   Type of an USB endpoint configuration structure.
 * @note    Platform specific restrictions may apply to endpoints.
 */
typedef struct {
  /**
   * @brief   Type and mode of the endpoint.
   */
  uint32_t                      ep_mode;
  /**
   * @brief   Setup packet notification callback.
   * @details This callback is invoked when a setup packet has been
   *          received.
   * @post    The application must immediately call @p usbReadPacket() in
   *          order to access the received packet.
   * @note    This field is only valid for @p USB_EP_MODE_TYPE_CTRL
   *          endpoints, it should be set to @p NULL for other endpoint
   *          types.
   */
  usbepcallback_t               setup_cb;
  /**
   * @brief   IN endpoint notification callback.
   * @details This field must be set to @p NULL if the IN endpoint is not
   *          used.
   */
  usbepcallback_t               in_cb;
  /**
   * @brief   OUT endpoint notification callback.
   * @details This field must be set to @p NULL if the OUT endpoint is not
   *          used.
   */
  usbepcallback_t               out_cb;
  /**
   * @brief   IN endpoint maximum packet size.
   * @details This field must be set to zero if the IN endpoint is not
   *          used.
   */
  uint16_t                      in_maxsize;
  /**
   * @brief   OUT endpoint maximum packet size.
   * @details This field must be set to zero if the OUT endpoint is not
   *          used.
   */
  uint16_t                      out_maxsize;
  /**
   * @brief   @p USBEndpointState associated to the IN endpoint.
   * @details This structure maintains the state of the IN endpoint.
   */
  USBInEndpointState            *in_state;
  /**
   * @brief   @p USBEndpointState associated to the OUT endpoint.
   * @details This structure maintains the state of the OUT endpoint.
   */
  USBOutEndpointState           *out_state;
  /* End of the mandatory fields.*/
  /**
   * @brief   Reserved field, not currently used.
   * @note    Initialize this field to 1 in order to be forward compatible.
   */
  uint16_t                      ep_buffers;
  /**
   * @brief   Pointer to a buffer for setup packets.
   * @details Setup packets require a dedicated 8-bytes buffer, set this
   *          field to @p NULL for non-control endpoints.
   */
  uint8_t                       *setup_buf;

  usb_dma_info                  dma_info;

  bool_t                        inEp_thd_handle_mode;//TRUE, handle in trhead; FALSE, handle in interrupt
  bool_t                        outEp_thd_handle_mode;//TRUE, handle in trhead; FALSE, handle in interrupt
} USBEndpointConfig;



/**
 * @brief   Type of an USB driver configuration structure.
 */
typedef struct {
  /**
   * @brief   USB events callback.
   * @details This callback is invoked when an USB driver event is registered.
   */
  usbeventcb_t                  event_cb;
  /**
   * @brief   Device GET_DESCRIPTOR request callback.
   * @note    This callback is mandatory and cannot be set to @p NULL.
   */
  usbgetdescriptor_t            get_descriptor_cb;
  /**
   * @brief   Requests hook callback.
   * @details This hook allows to be notified of standard requests or to
   *          handle non standard requests.
   */
  usbreqhandler_t               requests_hook_cb;
  /**
   * @brief   Start Of Frame callback.
   */
  usbcallback_t                 sof_cb;
  /* End of the mandatory fields.*/
} USBConfig;

/**
 * @brief   Structure representing an USB driver.
 */
struct USBDriver {
  /**
   * @brief   Driver state.
   */
  usbstate_t                    state;
  /**
   * @brief   Current configuration data.
   */
  const USBConfig               *config;
  /**
   * @brief   Bit map of the transmitting IN endpoints.
   */
  uint16_t                      transmitting;
  /**
   * @brief   Bit map of the receiving OUT endpoints.
   */
  uint16_t                      receiving;
  /**
   * @brief   Active endpoints configurations.
   */
  const USBEndpointConfig       *epc[USB_MAX_ENDPOINTS + 1];
  /**
   * @brief   Fields available to user, it can be used to associate an
   *          application-defined handler to an IN endpoint.
   * @note    The base index is one, the endpoint zero does not have a
   *          reserved element in this array.
   */
  void                          *in_params[USB_MAX_ENDPOINTS];
  /**
   * @brief   Fields available to user, it can be used to associate an
   *          application-defined handler to an OUT endpoint.
   * @note    The base index is one, the endpoint zero does not have a
   *          reserved element in this array.
   */
  void                          *out_params[USB_MAX_ENDPOINTS];
  /**
   * @brief   Endpoint 0 state.
   */
  usbep0state_t                 ep0state;
  /**
   * @brief   Next position in the buffer to be transferred through endpoint 0.
   */
  uint8_t                       *ep0next;
  /**
   * @brief   Number of bytes yet to be transferred through endpoint 0.
   */
  size_t                        ep0n;
  /**
   * @brief   Endpoint 0 end transaction callback.
   */
  usbcallback_t                 ep0endcb;
  /**
   * @brief   Setup packet buffer.
   */
  uint8_t                       setup[8];
  /**
   * @brief   Current USB device status.
   */
  uint16_t                      status;
  /**
   * @brief   Assigned USB address.
   */
  uint8_t                       address;
  /**
   * @brief   Current USB device configuration.
   */
  uint8_t                       configuration;
#if defined(USB_DRIVER_EXT_FIELDS)
  USB_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief   Pointer to the next address in the packet memory.
   */
  //uint32_t                      pmnext;
  //HS_USB_Type*                usb_reg;
  HS_USB_DMA_Type*            usb_dma_reg;

  uint8_t                     remote_wake;

  uint16_t                    usb_vid;
  uint16_t                    usb_pid;
  uint32_t                    usb_pw_sess_mode;

  thread_t *ep1_thread;
  thread_t *ep2_thread;
  thread_t *dma_thread;
};

typedef struct _FIFO_CONFIG
{
    uint8_t  EpNum;//bit7 = 1 means rx endponit, bit7 = 0 means tx endpoint
    uint8_t  DoubleBuf;// = 1, means doubling buffer is used
    uint16_t StartAddr;
    uint16_t FifoSize;//if  DoubleBuf=1, then actual fifo used is 2*FifoSize
}Fifo_Cfg;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Returns the current frame number.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @return              The current frame number.
 *
 * @notapi
 */
#define usb_lld_get_frame_number(usbp) (HS_USB->USB_FRAME)

#define usb_lld_stall_out_sent()  ((HS_USB->USB_RXCSR&HSUSB_RXCSR_P_SENTSTALL)?TRUE:FALSE)


#define usb_lld_enable_pmu_charger()    (HS_PMU->ANA_CON &= ~(0x01<<5))
#define usb_lld_disable_pmu_charger()   (HS_PMU->ANA_CON |= 0x01<<5)

#define usb_lld_get_plugin()            ((((HS_SYS->USB_CTRL >> 20) & 0x7) == 7 ) ? TRUE : FALSE)


/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern USBDriver USBD1;
extern uint8_t g_UsbMode;

#ifdef __cplusplus
extern "C" {
#endif
  void usb_lld_init(void);
  void usb_lld_start(USBDriver *usbp);
  void usb_lld_stop(USBDriver *usbp);
  void usb_lld_dmaServiceStart(USBDriver *usbp);
  void usb_lld_dmaServiceStop(USBDriver *usbp);    
  void usb_lld_reset(USBDriver *usbp);
  void usb_lld_set_address(USBDriver *usbp);
  void usb_lld_init_endpoint(USBDriver *usbp, usbep_t ep);
  void usb_lld_disable_endpoints(USBDriver *usbp);
  usbepstatus_t usb_lld_get_status_in(usbep_t ep);
  usbepstatus_t usb_lld_get_status_out(usbep_t ep);
  void usb_lld_read_setup(usbep_t ep, uint8_t *buf);
  void usb_lld_prepare_receive(USBDriver *usbp, usbep_t ep);
  void usb_lld_prepare_transmit(USBDriver *usbp, usbep_t ep);
  void usb_lld_start_out(USBDriver *usbp, usbep_t ep);
  void usb_lld_start_in(USBDriver *usbp, usbep_t ep);
  void usb_lld_stall_out(usbep_t ep);
  void usb_lld_stall_in(usbep_t ep);
  void usb_lld_clear_out(usbep_t ep);
  void usb_lld_clear_in(usbep_t ep);
  void hsusb_start(USBDriver *usbp);
  void hsusb_fifo_setup(const Fifo_Cfg* cfg);
  void usb_lld_servedsetup(uint8_t bHasDataPhase);

  void usb_lld_flush_setup_fifo(void);
  void hsusb_ep_int_enable(USBDriver *usbp, uint8_t epnum, uint8_t IsTx);
  void usb_dma_config_ch(USBDriver *usbp,
                                uint8_t  ch,
                                uint16_t packetSiz,
                                uint8_t  mode,
                                uint8_t  epDirIn,
                                uint8_t  epNum,
                                uint32_t buf_addr,
                                uint32_t len);
//void usb_lld_WriteDmaMode1Patch(USBDriver *usbp, usbep_t ep);
void hsusb_ep_select(uint8_t epnum);
void hsusb_ep_int_enable(USBDriver *usbp, uint8_t epnum, uint8_t IsTx);
void usb_lld_connect_bus(USBDriver *usbp);
void usb_lld_disconnect_bus(USBDriver *usbp);
void usb_packet_write_from_buffer(USBDriver *usbp, usbep_t ep,
                                         const uint8_t *buf,
                                         size_t n);
void usb_packet_read_to_buffer(USBDriver *usbp,usbep_t ep, uint8_t *buf, size_t n) ;
void usb_ldd_clear_datatoggle(usbep_t ep, uint8_t IsInEp);
uint8_t usb_lld_stall_in_sent(usbep_t ep);
uint8_t usb_lld_get_rx_data_len(usbep_t ep);
uint8_t usb_lld_check_host_mode(void);
void usb_lld_enable_int(void);
void usb_lld_enable_dma_int(void);
void usb_lld_remote_wakeup(USBDriver *usbp);
uint8_t usb_lld_read_power_reg(void);
void usb_lld_set_power_sess_mode(USBDriver *usbp, uint32_t PwSessMode);
void usb_lld_enData(USBDriver *usbp);

#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* HS usb declarations.                                                    */
/*===========================================================================*/
/*
 * Common USB registers
 */
/* POWER */
#define      HSUSB_POWER_ISOUPDATE	0x80
#define      HSUSB_POWER_VBUSVAL     0x40
#define      HSUSB_POWER_VBUSSESS    0x20
#define      HSUSB_POWER_VBUSLO      0x10
#define      HSUSB_POWER_RESET	    0x08
#define      HSUSB_POWER_RESUME	    0x04
#define      HSUSB_POWER_SUSPENDM	0x02
#define      HSUSB_POWER_ENSUSPEND	0x01

/* INTRUSB */
#define      HSUSB_INTR_SUSPEND      0x01
#define      HSUSB_INTR_RESUME       0x02
#define      HSUSB_INTR_RESET		0x04
#define      HSUSB_INTR_BABBLE	    0x04
#define      HSUSB_INTR_SOF		    0x08
#define      HSUSB_INTR_CONNECT	    0x10
#define      HSUSB_INTR_DISCONNECT	0x20
#define      HSUSB_INTR_SESSREQ	    0x40
#define      HSUSB_INTR_VBUSERROR	0x80	/* For SESSION end */
#define      HSUSB_INTR_RELEASE   0x8000


/* DEVCTL */
#define      HSUSB_DEVCTL_BDEVICE	0x80
#define      HSUSB_DEVCTL_FSDEV	    0x40
#define      HSUSB_DEVCTL_LSDEV      0x20
#define      HSUSB_DEVCTL_PUCON		0x10
#define      HSUSB_DEVCTL_PDCON		0x08
#define      HSUSB_DEVCTL_HM		    0x04
#define      HSUSB_DEVCTL_HR		    0x02
#define      HSUSB_DEVCTL_SESSION	0x01


/*
 * USB INDEXED registers
 */
/* CSR0 */
#define HSUSB_CSR0_FLUSHFIFO	0x0100
#define HSUSB_CSR0_TXPKTRDY	0x0002
#define HSUSB_CSR0_RXPKTRDY	0x0001
/* CSR0 in Peripheral mode */
#define HSUSB_CSR0_P_SVDSETUPEND	0x0080
#define HSUSB_CSR0_P_SVDRXPKTRDY	0x0040
#define HSUSB_CSR0_P_SENDSTALL	0x0020
#define HSUSB_CSR0_P_SETUPEND	0x0010
#define HSUSB_CSR0_P_DATAEND	0x0008
#define HSUSB_CSR0_P_SENTSTALL	0x0004
/* CSR0 in Host mode */
#define HSUSB_CSR0_H_DIS_PING		0x0800
#define HSUSB_CSR0_H_WR_DATATOGGLE	0x0400	/* Set to allow setting: */
#define HSUSB_CSR0_H_DATATOGGLE		0x0200	/* Data toggle control */
#define HSUSB_CSR0_H_NAKTIMEOUT		0x0080
#define HSUSB_CSR0_H_STATUSPKT		0x0040
#define HSUSB_CSR0_H_REQPKT		0x0020
#define HSUSB_CSR0_H_ERROR		0x0010
#define HSUSB_CSR0_H_SETUPPKT		0x0008
#define HSUSB_CSR0_H_RXSTALL		0x0004
/* CSR0 bits to avoid zeroing (write zero clears, write 1 ignored) */
#define HSUSB_CSR0_P_WZC_BITS	\
	(HSUSB_CSR0_P_SENTSTALL)
#define HSUSB_CSR0_H_WZC_BITS	\
	(HSUSB_CSR0_H_NAKTIMEOUT | HSUSB_CSR0_H_RXSTALL \
	| HSUSB_CSR0_RXPKTRDY)

/* TXCSR in Peripheral and Host mode */
#define HSUSB_TXCSR_AUTOSET		0x8000
#define HSUSB_TXCSR_MODE			0x2000
#define HSUSB_TXCSR_DMAENAB		0x1000
#define HSUSB_TXCSR_FRCDATATOG	0x0800
#define HSUSB_TXCSR_DMAMODE		0x0400
#define HSUSB_TXCSR_CLRDATATOG	0x0040
#define HSUSB_TXCSR_FLUSHFIFO	0x0008
#define HSUSB_TXCSR_FIFONOTEMPTY	0x0002
#define HSUSB_TXCSR_TXPKTRDY		0x0001
/* TXCSR in Peripheral mode */
#define HSUSB_TXCSR_P_ISO		    0x4000
#define HSUSB_TXCSR_P_INCOMPTX		0x0080
#define HSUSB_TXCSR_P_SENTSTALL		0x0020
#define HSUSB_TXCSR_P_SENDSTALL		0x0010
#define HSUSB_TXCSR_P_UNDERRUN		0x0004
/* TXCSR in Host mode */
#define HSUSB_TXCSR_H_WR_DATATOGGLE	0x0200
#define HSUSB_TXCSR_H_DATATOGGLE		0x0100
#define HSUSB_TXCSR_H_NAKTIMEOUT		0x0080
#define HSUSB_TXCSR_H_RXSTALL		0x0020
#define HSUSB_TXCSR_H_ERROR		    0x0004

/* TXCSR bits to avoid zeroing (write zero clears, write 1 ignored) */
#define HSUSB_TXCSR_P_WZC_BITS	\
	( HSUSB_TXCSR_P_SENTSTALL | HSUSB_TXCSR_P_UNDERRUN \
        | HSUSB_TXCSR_FIFONOTEMPTY)
#define HSUSB_TXCSR_H_WZC_BITS	\
	(HSUSB_TXCSR_H_NAKTIMEOUT | HSUSB_TXCSR_H_RXSTALL \
	| HSUSB_TXCSR_H_ERROR | HSUSB_TXCSR_FIFONOTEMPTY)

/* RXCSR in Peripheral and Host mode */
#define HSUSB_RXCSR_AUTOCLEAR		0x8000
#define HSUSB_RXCSR_DMAENAB		    0x2000
#define HSUSB_RXCSR_DMAMODE		    0x1000
#define HSUSB_RXCSR_CLRDATATOG		0x0080
#define HSUSB_RXCSR_FLUSHFIFO		0x0010
#define HSUSB_RXCSR_DATAERROR		0x0008
#define HSUSB_RXCSR_FIFOFULL		    0x0002
#define HSUSB_RXCSR_RXPKTRDY		    0x0001
/* RXCSR in Peripheral mode */
#define HSUSB_RXCSR_P_ISO		    0x4000
#define HSUSB_RXCSR_P_SENTSTALL		0x0040
#define HSUSB_RXCSR_P_SENDSTALL		0x0020
#define HSUSB_RXCSR_P_OVERRUN		0x0004
/* RXCSR in Host mode */
#define HSUSB_RXCSR_H_AUTOREQ		0x4000
#define HSUSB_RXCSR_H_RXSTALL		0x0040
#define HSUSB_RXCSR_H_REQPKT		0x0020
#define HSUSB_RXCSR_H_ERROR		0x0004
/* RXCSR bits to avoid zeroing (write zero clears, write 1 ignored) */
#define HSUSB_RXCSR_P_WZC_BITS	\
	(HSUSB_RXCSR_P_SENTSTALL | HSUSB_RXCSR_P_OVERRUN \
	| HSUSB_RXCSR_RXPKTRDY)
#define HSUSB_RXCSR_H_WZC_BITS	\
	(HSUSB_RXCSR_H_RXSTALL | HSUSB_RXCSR_H_ERROR \
	| HSUSB_RXCSR_DATAERROR | HSUSB_RXCSR_RXPKTRDY)

#define HSUSB_TYPE_PROTO		0x30	/* Implicitly zero for ep0 */
#define HSUSB_TYPE_PROTO_SHIFT	4
#define HSUSB_TYPE_REMOTE_END	0xf	/* Implicitly zero for ep0 */

#define HSUSB_FIFO_ADD_SHIFT             0
#define HSUSB_FIFO_SIZE_SHIFT            13
/* Allocate for double-packet buffering (effectively doubles assigned _SIZE) */
#define HSUSB_FIFOSZ_DPB	        0x100
/* Allocation size (8, 16, 32, ... 4096) */
#define HSUSB_FIFOSZ_SIZE	0x07

#define USBDMAREGINT *((volatile uint32_t *)(OTG_BASE + 0x200))
#define USBDMA(a) ((HS_USB_DMA_Type *)(OTG_BASE + 0x200 + (0x10 * a)))

#endif /* HAL_USE_USB */

#endif /* _USB_LLD_H_ */

/** @} */
