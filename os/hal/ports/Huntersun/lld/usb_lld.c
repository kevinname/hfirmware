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


#include <string.h>

#include "ch.h"
#include "hal.h"
#include "bmem.h"
#include "cmsis_os.h"

#if HAL_USE_USB

#include "usb_lld.h"
#if HAL_USE_USB_HOST_STORAGE
#include "usb_h_lld.h"
#endif

void hs66xx_usb_dma_irq_channel(USBDriver *usbp, uint8_t bchannel);

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

USBDriver USBD1;


/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   EP0 state.
 * @note    It is an union because IN and OUT endpoints are never used at the
 *          same time for EP0.
 */
static union {
  /**
   * @brief   IN EP0 state.
   */
  USBInEndpointState in;
  /**
   * @brief   OUT EP0 state.
   */
  USBOutEndpointState out;
} ep0_state;


/**
 * @brief   Buffer for the EP0 setup packets.
 */
static uint8_t ep0setup_buffer[8];

/**
 * @brief   EP0 initialization structure.
 */
static const USBEndpointConfig ep0config = {
  USB_EP_MODE_TYPE_CTRL,
  _usb_ep0setup,
  _usb_ep0in,
  _usb_ep0out,
  USB_CTRL_EP_PKT_SIZ,
  USB_CTRL_EP_PKT_SIZ,
  &ep0_state.in,
  &ep0_state.out,
  1,
  ep0setup_buffer,
  {
    {0xFF, 0xFF},
    {DMA_MODE_NONE, DMA_MODE_NONE},
    NULL,
    NULL,
  },
  0, 0
};

const Fifo_Cfg fifo_cfg_ep0[] =
{
    //EP0 out
    {
        0x00,
        0,
        0,
        64,
    },
    //EP0 In
    {
        0x80,
        0,
        0,
        64,
    },
};

#define USB_MAILBOX_SIZE 3
static mailbox_t g_ep1_mbox, g_ep2_mbox,g_dma_storage_mbox;
static msg_t g_message[3][USB_MAILBOX_SIZE];

uint8_t g_UsbMode = USB_MODE_UNKNOWN;
#define USB_THRERAD_EXIT  0x6601U


static void UsbEp1Thread(void* arg)
{
    USBDriver *usbp = (USBDriver *)arg;;
    msg_t     msg;

    chRegSetThreadName("UsbEp1Thread");
    while(arg)
    {
        chMBFetch(&g_ep1_mbox, &msg, TIME_INFINITE);
        if(USB_THRERAD_EXIT == msg)
          break;
        
        chSysLock();
        
        if(msg==FALSE){
            _usb_isr_invoke_out_cb(usbp,1);
        }
        else{
            _usb_isr_invoke_in_cb(usbp,1);
        }
        
        chSysUnlock();
    }
}

static void UsbEp2Thread(void* arg)
{
    USBDriver *usbp = (USBDriver *)arg;;
    msg_t     msg;

    chRegSetThreadName("UsbEp2Thread");
    while(arg)
    {
        chMBFetch(&g_ep2_mbox, &msg, TIME_INFINITE);
        if(USB_THRERAD_EXIT == msg)
          break;
        
        chSysLock();
        
        if(msg==FALSE){
            _usb_isr_invoke_out_cb(usbp,2);
        }
        else {
            _usb_isr_invoke_in_cb(usbp,2);
        }
        
        chSysUnlock();
    }
}

static void UsbDmaStorageThread(void * arg)
{
    USBDriver *usbp = (USBDriver *)arg;;
    msg_t     msg;

    chRegSetThreadName("UsbDmaThread");
    while(arg)
    {
        chMBFetch(&g_dma_storage_mbox, &msg, TIME_INFINITE);
        if(USB_THRERAD_EXIT == msg)
          break;

        chSysLock();
        hs66xx_usb_dma_irq_channel(usbp, msg);
        chSysUnlock();
    }
}


/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
void hsusb_ep_select(uint8_t epnum)
{
    HS_USB->USB_INDEX = epnum&0x0f;
}

void hsusb_ep_int_enable(USBDriver *usbp, uint8_t epnum, uint8_t IsTx)
{
    uint16_t IntEn = IsTx?HS_USB->USB_INTTXEN:HS_USB->USB_INTRXEN;

    osalDbgAssert(usbp, "hsusb_ep_int_enable()");

    IntEn |= 1<<epnum;
    if(IsTx)
    {
        HS_USB->USB_INTTXEN = IntEn;
    }
    else
    {
        HS_USB->USB_INTRXEN = IntEn;
    }
}
void hsusb_start(USBDriver *usbp)
{
    osalDbgAssert(usbp,"hsusb_start()");

	/*  Set INT enable registers, enable interrupts */
    HS_USB->USB_INTTXEN = 0x0000;
    HS_USB->USB_INTRXEN = 0x0000;
    hsusb_ep_int_enable(usbp, 0, TRUE);
    HS_USB->USB_INTUSBEN = 0xf7;
    HS_USB->USB_ADDR = 0x00;

}

void hsusb_fifo_setup(const Fifo_Cfg* cfg)
{
    uint16_t siz = cfg->FifoSize;
    uint16_t offset = cfg->StartAddr;
    uint16_t regval = 0;
    uint8_t  epnum = cfg->EpNum&0x7f;

    osalDbgAssert(cfg, "hsusb_fifo_setup()");

    if(siz<8||(siz%8))
    {
        return;
    }
    switch(siz)
    {
        case 16:
            regval = 1;
            break;
        case 32:
            regval = 2;
            break;
        case 64:
            regval = 3;
            break;
        case 128:
            regval = 4;
            break;
        case 256:
            regval = 5;
            break;
        case 8:
        default:
            regval = 0;
            break;
    }
    regval <<=13;
    regval |= (offset>>3);
    if(cfg->DoubleBuf)
    {
        regval|= 1<<12;
    }
    hsusb_ep_select(epnum);
    if(cfg->EpNum&0x80)
    {
        HS_USB->USB_TXFIFO = regval;
    }
    else
    {
        HS_USB->USB_RXFIFO = regval;
    }
}

uint8_t usb_lld_check_host_mode(void)
{
  int32_t tm = 0x180000;
  while(tm--);
  
    if(HS_USB->USB_DEVCTL&0x80)
    {
        //dev mode
        return USB_MODE_DEV;
    }
    else
    {
        //hostmode
        return USB_MODE_HOST;
    }
}


/**
 * @brief   Reads from a dedicated packet buffer for OUT EP
 *
 * @param[in] udp       pointer to a @p stm32_usb_descriptor_t
 * @param[out] buf      buffer where to copy the packet data
 * @param[in] n         maximum number of bytes to copy. This value must
 *                      not exceed the maximum packet size for this endpoint.
 *
 * @notapi
 */
void usb_packet_read_to_buffer(USBDriver *usbp,usbep_t ep, uint8_t *buf, size_t n)
{
    size_t i = 0;
    volatile uint8_t *pb =  (volatile uint8_t *) (OTG_BASE+USB_EPxFIFO_OFFSET(ep));

    osalDbgAssert(usbp&&buf&&pb, "usb_packet_read_to_buffer()");

    for(i= 0;i<n;i++){
        buf[i] = *pb;
    }
}

/**
 * @brief   Reads from a dedicated packet buffer.
 *
 * @param[in] udp       pointer to a @p stm32_usb_descriptor_t
 * @param[in] iqp       pointer to an @p InputQueue object
 * @param[in] n         maximum number of bytes to copy. This value must
 *                      not exceed the maximum packet size for this endpoint.
 *
 * @notapi
 */
static void usb_packet_read_to_queue(USBDriver *usbp,usbep_t ep,input_queue_t *iqp, size_t n) {

    size_t n_backup = n;

    chDbgAssert(usbp&&iqp, "usb_packet_read_to_queue()");


    while (n > 0)
    {
        volatile uint8_t *pw = (volatile uint8_t *)(OTG_BASE+USB_EPxFIFO_OFFSET(ep));
        *iqp->q_wrptr++ = *pw;
        if (iqp->q_wrptr >= iqp->q_top)
        {
            iqp->q_wrptr = iqp->q_buffer;
        }
        n--;
    }
    /* Updating queue.*/
    //chSysLockFromIsr();
    iqp->q_counter += n_backup;
    while (queue_notempty(&iqp->q_waiting))
    {
        chSchReadyI(queue_fifo_remove(&iqp->q_waiting))->p_u.rdymsg = Q_OK;
    }
    //chSysUnlockFromIsr();

}

/**
 * @brief   Writes to a dedicated packet buffer for In EP
 *
 * @param[in] udp       pointer to a @p stm32_usb_descriptor_t
 * @param[in] buf       buffer where to fetch the packet data
 * @param[in] n         maximum number of bytes to copy. This value must
 *                      not exceed the maximum packet size for this endpoint.
 *
 * @notapi
 */
void usb_packet_write_from_buffer(USBDriver *usbp, usbep_t ep,
                                         const uint8_t *buf,
                                         size_t n)
{
    size_t i = 0;
    volatile uint8_t *pb = (volatile uint8_t *)(OTG_BASE+USB_EPxFIFO_OFFSET(ep));

    chDbgAssert(usbp&&buf&&pb, "usb_packet_write_from_buffer()");

    for(i= 0;i<n;i++){
        *pb = buf[i];
    }
}

/**
 * @brief   Writes to a dedicated packet buffer.
 *
 * @param[in] udp       pointer to a @p stm32_usb_descriptor_t
 * @param[in] buf       buffer where to fetch the packet data
 * @param[in] n         maximum number of bytes to copy. This value must
 *                      not exceed the maximum packet size for this endpoint.
 *
 * @notapi
 */
static void usb_packet_write_from_queue(USBDriver *usbp, usbep_t ep,
                                        output_queue_t *oqp, size_t n) {

    size_t n_backup = n;

    osalDbgAssert(usbp&&oqp,"usb_packet_write_from_queue()");

    while (n > 0)
    {
        volatile uint8_t *pb = (volatile uint8_t *)(OTG_BASE+USB_EPxFIFO_OFFSET(ep));

        *pb = *oqp->q_rdptr++;

        if (oqp->q_rdptr >= oqp->q_top)
        {
            oqp->q_rdptr = oqp->q_buffer;
        }

        n--;
    }


    /* Updating queue. Note, the lock is done in this unusual way because this
        function can be called from both ISR and thread context so the kind
        of lock function to be invoked cannot be decided beforehand.*/

    oqp->q_counter += n_backup;
    while (queue_notempty(&oqp->q_waiting))
    {
        chSchReadyI(queue_fifo_remove(&oqp->q_waiting))->p_u.rdymsg = Q_OK;
    }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/
void usb_global_handler(USBDriver *usbp, uint8_t int_usb)
{

    uint8_t power;

    osalDbgAssert(usbp, "func()");

    power = HS_USB->USB_POWER;
    if(int_usb & HSUSB_INTR_RESUME)
    {
        //handle resume
        if (!(power & HSUSB_POWER_VBUSVAL))
        {
            uint8_t int_usb_new = int_usb;
            int_usb_new |= HSUSB_INTR_DISCONNECT;
            int_usb_new &= ~HSUSB_INTR_SUSPEND;
            HS_USB->USB_INTUSB = int_usb_new;
        }
        _usb_isr_invoke_event_cb(usbp, USB_EVENT_WAKEUP);
    }
    if (int_usb & HSUSB_INTR_SESSREQ)
    {
        //handle session
    }
    if (int_usb & HSUSB_INTR_VBUSERROR)
    {
        //handle vbus error
    }
    if (int_usb & HSUSB_INTR_SUSPEND)
    {
        //handle suspend
        _usb_isr_invoke_event_cb(usbp, USB_EVENT_SUSPEND);
        //test suspend/resume
        //HS_SYS->USB_CTRL|=(0x0001<<8);
    }
    //if (int_usb & HSUSB_INTR_CONNECT)
    //{

    //}
    if(int_usb & HSUSB_INTR_DISCONNECT)
    {
        g_UsbMode = USB_MODE_UNKNOWN;
        _usb_reset(usbp);
        _usb_isr_invoke_event_cb(usbp, USB_EVENT_RESET);
    }
    if (int_usb & HSUSB_INTR_RESET)
    {
        _usb_reset(usbp);
        _usb_isr_invoke_event_cb(usbp, USB_EVENT_RESET);
        //HS_USB->USB_POWER|=0x01;//test suspend/resume
    }
}


void usb_ep0_handler(USBDriver *usbp)
{
    osalDbgAssert(usbp, "func()");

    hsusb_ep_select(0);

    if(HS_USB->USB_CSR0&HSUSB_CSR0_P_SENTSTALL)
    {
        usb_lld_clear_in(0);
        //usb_lld_clear_out(usbp, 0);
    }

    if(usbp->ep0state == USB_EP0_WAITING_SETUP)
    {
        if(HS_USB->USB_CSR0&HSUSB_CSR0_RXPKTRDY)
        {
            _usb_isr_invoke_setup_cb(usbp, 0);
        }
        else
        {
        }
    }
    else
    {
        if((usbp->setup[0] & USB_RTYPE_DIR_MASK) == USB_RTYPE_DIR_DEV2HOST)
        {
            _usb_isr_invoke_in_cb(usbp, 0);
        }
        else
        {
            _usb_isr_invoke_out_cb(usbp, 0);
        }
    }
}

void usb_tx_epx_handler(USBDriver *usbp, uint8_t ep_num)
{

    osalDbgAssert(usbp, "func()");

    hsusb_ep_select(ep_num);
    if(HS_USB->USB_TXCSR&HSUSB_TXCSR_P_SENTSTALL)
    {
        usb_lld_clear_in(ep_num);
        usbp->transmitting &= ~(1 <<ep_num);
        return;
    }

    if(ep_num == 1)
    {
        if(usbp->epc[ep_num]->inEp_thd_handle_mode)
        {
            chMBPostI(&g_ep1_mbox, TRUE);
        }
        else
        {
            _usb_isr_invoke_in_cb(usbp,ep_num);
        }


    }
    else if(ep_num == 2)
    {

        if(usbp->epc[ep_num]->inEp_thd_handle_mode)
        {
            chMBPostI(&g_ep2_mbox, TRUE);
        }
        else
        {
            _usb_isr_invoke_in_cb(usbp,ep_num);
        }
    }
    else
    {
        _usb_isr_invoke_in_cb(usbp,ep_num);
    }
}

void usb_rx_epx_handler(USBDriver *usbp, uint8_t ep_num)
{

    osalDbgAssert(usbp, "func()");

    hsusb_ep_select(ep_num);
    if(HS_USB->USB_RXCSR&HSUSB_RXCSR_P_SENTSTALL)
    {
        usb_lld_clear_out(ep_num);
        return;
    }

    if(ep_num == 1)
    {
        if(usbp->epc[ep_num]->outEp_thd_handle_mode)
        {
            chMBPostI(&g_ep1_mbox, FALSE);
        }
        else
        {
             _usb_isr_invoke_out_cb(usbp,ep_num);
        }


    }
    else if(ep_num == 2)
    {

        if(usbp->epc[ep_num]->outEp_thd_handle_mode)
        {
            chMBPostI(&g_ep2_mbox, FALSE);
        }
        else
        {
             _usb_isr_invoke_out_cb(usbp,ep_num);
        }
    }
    else
    {
         _usb_isr_invoke_out_cb(usbp,ep_num);
    }
}

static void hs66xx_usb_irq(USBDriver *usbp)
{
    uint8_t int_usb = HS_USB->USB_INTUSB;
    uint16_t int_tx = HS_USB->USB_INTTX;
    uint16_t int_rx = HS_USB->USB_INTRX;


    osalDbgAssert(usbp, "func()");

    if(g_UsbMode == USB_MODE_UNKNOWN)
    {
        if(int_usb&(HSUSB_INTR_RESET|HSUSB_INTR_CONNECT))
        {
            g_UsbMode = usb_lld_check_host_mode();
        }
        else
        {
            return;
        }
    }

#if HAL_USE_USB_HOST_STORAGE

    if(g_UsbMode == USB_MODE_HOST)
    {
        hs66xx_usb_host_irq(int_usb, int_tx, int_rx);
    }
    else
    {
#endif
        if(int_usb)
        {
            usb_global_handler(usbp, int_usb);
        }

        if(int_rx)
        {
           	uint16_t reg = int_rx >> 1;
            uint8_t ep_num = 1;
            while (reg)
            {
                 if (reg & 1)
                 {
                    usb_rx_epx_handler(usbp, ep_num);//ep out
                 }
                 reg >>= 1;
                 ep_num++;
            }
        }

        if(int_tx)
        {
            if(int_tx&0x0001)
            {
                usb_ep0_handler(usbp);
            }
            else
            {
            	uint16_t reg = int_tx >> 1;
            	uint8_t ep_num = 1;
            	while (reg)
                {
            		if (reg & 1)
                    {
                        usb_tx_epx_handler(usbp, ep_num);//ep in
                    }
            		reg >>= 1;
            		ep_num++;
            	}
            }
        }
#if HAL_USE_USB_HOST_STORAGE
    }
#endif
}

/**
 * @brief   USB interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(OTG_IRQHandler)
{
    CH_IRQ_PROLOGUE();
    chSysLockFromISR();
    hs66xx_usb_irq(&USBD1);
    chSysUnlockFromISR();
    CH_IRQ_EPILOGUE();
}

//uint8_t g_dmaw_flag = TRUE;
void hs66xx_usb_dma_irq_channel(USBDriver *usbp, uint8_t bchannel)
{
    usbep_t ep = 1;
    uint8_t ep_dir = 0;
    USBOutEndpointState *osp = NULL;
    USBInEndpointState *isp = NULL;

    osalDbgAssert(usbp, "func()");

    //chSysLock();
    while(ep<USB_MAX_ENDPOINTS + 1)
    {
        if((usbp->epc[ep]) && (usbp->epc[ep]->dma_info.dma_ch[0] == bchannel))
        {
            ep_dir = 0;//out
            break;
        }
        if((usbp->epc[ep]) && (usbp->epc[ep]->dma_info.dma_ch[1] == bchannel))
        {
            ep_dir = 1;
            break;
        }
        ep++;
    }
    if(ep == USB_MAX_ENDPOINTS+1)
    {
        //chSysUnlock();
        return;
    }
    if(ep_dir == 0)//out ep
    {
        if(!usbp->epc[ep])
          return ;
        
        osp = usbp->epc[ep]->out_state;
        hsusb_ep_select(ep);
        if(osp->dma_mode == DMA_MODE_0)
        {
            uint16_t csr = HS_USB->USB_RXCSR;
            csr &= ~(HSUSB_RXCSR_AUTOCLEAR|HSUSB_RXCSR_DMAENAB|HSUSB_RXCSR_DMAMODE);
            csr &= ~HSUSB_RXCSR_RXPKTRDY;
            HS_USB->USB_RXCSR = csr;
            if(usbp->epc[ep]->dma_info.dma_outep_cb)
            {
                usbp->epc[ep]->dma_info.dma_outep_cb(usbp, ep);
            }
        }
        else if(osp->dma_mode == DMA_MODE_1)
        {
            uint16_t csr = HS_USB->USB_RXCSR;
            HS_USB->USB_INTRXEN |= (1<<ep);
            //USBDMAREG(bchannel).USB_DMA_CTL  = 0;
            USBDMA(bchannel)->USB_DMA_CTL  = 0;
            csr &= ~(HSUSB_RXCSR_AUTOCLEAR|HSUSB_RXCSR_DMAENAB|HSUSB_RXCSR_DMAMODE);
            //if(g_dmaw_flag)
            //{
                //if(!USBREG->USB_RXCOUNT1)
                //{
                 //   csr &= ~HSUSB_RXCSR_RXPKTRDY;
                //}
            //}
            HS_USB->USB_RXCSR = csr;

            //g_testcnt_3 = 11;
            if(usbp->epc[ep]->dma_info.dma_outep_cb)
            {
                usbp->epc[ep]->dma_info.dma_outep_cb(usbp, ep);
            }
        }
    }
    else
    {
        if(!usbp->epc[ep])
          return ;
        
        isp = usbp->epc[ep]->in_state;
        hsusb_ep_select(ep);
        if(isp->dma_mode == DMA_MODE_0)
        {
            uint16_t csr = HS_USB->USB_TXCSR;
            if(usbp->epc[ep]->dma_info.dma_inep_cb)
            {
                usbp->epc[ep]->dma_info.dma_inep_cb(usbp, ep);
            }
            csr &= ~(HSUSB_TXCSR_AUTOSET|HSUSB_TXCSR_DMAENAB|HSUSB_TXCSR_DMAMODE);
            csr |= HSUSB_TXCSR_TXPKTRDY;
            HS_USB->USB_TXCSR = csr;
            //isp->dma_mode = DMA_MODE_NONE;
        }
        else if(isp->dma_mode == DMA_MODE_1)
        {
            uint16_t csr = HS_USB->USB_TXCSR;
            csr &= ~(HSUSB_TXCSR_AUTOSET|HSUSB_TXCSR_DMAENAB|HSUSB_TXCSR_DMAMODE);
            //csr |= HSUSB_TXCSR_TXPKTRDY;
            HS_USB->USB_TXCSR = csr;
            //USBDMAREG(bchannel).USB_DMA_CTL  = 0;
            USBDMA(bchannel)->USB_DMA_CTL  = 0;
            if(usbp->epc[ep]->dma_info.dma_inep_cb)
            {
                usbp->epc[ep]->dma_info.dma_inep_cb(usbp, ep);
            }
        }
    }
    //chSysUnlock();
}


static void hs66xx_usb_dma_irq(USBDriver *usbp)
{

    osalDbgAssert(usbp, "func()");

#if HAL_USE_USB_HOST_STORAGE
    if(g_UsbMode == USB_MODE_HOST)
    {
        hs66xx_usb_host_dma_irq();
    }
    else
    {
#endif
        uint8_t bRegVal = USBDMAREGINT;
        msg_t bchannel;
        for(bchannel=0;bchannel<USB_DMA_MAX_CHANNELS;bchannel++)
        {
            if(bRegVal&(1<<bchannel))
            {
                if(bchannel==USB_DMA_CH_AUD_OUT||bchannel==USB_DMA_CH_AUD_IN)
                {

                    hs66xx_usb_dma_irq_channel(usbp, bchannel);

                }
                else
                {
                    if(bchannel==USB_DMA_CH_STOR_OUT)
                    {
                        hs66xx_usb_dma_irq_channel(usbp, bchannel);
                    }
                    else
                    {
                        chMBPostI(&g_dma_storage_mbox, bchannel);
                    }
                }
            }
        }
#if HAL_USE_USB_HOST_STORAGE
    }
#endif

}

/**
 * @brief   USB dma interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(USBDMA_IRQHandler)
{
    CH_IRQ_PROLOGUE();
    chSysLockFromISR();
    hs66xx_usb_dma_irq(&USBD1);
    chSysUnlockFromISR();
    CH_IRQ_EPILOGUE();
}



/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level USB driver initialization.
 *
 * @notapi
 */
void usb_lld_init(void)
{

    /* Driver initialization.*/
    usbObjectInit(&USBD1);
    //g_UsbMode = check_host_mode(&USBD1);
    HS_SYS->USB_CTRL &= ~(1 << 16);
}

/**
 * @brief   Configures and activates the USB peripheral.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */

void usb_lld_start(USBDriver *usbp)
{

    osalDbgAssert(usbp, "func()");

    if (usbp->state == USB_STOP)
    {
        nvicDisableVector(IRQ_USBHOST);
        cpmDisableUSB();
        cpmResetUSB();
        cpmEnableUSB();
        /* Reset procedure enforced on driver start.*/
        _usb_reset(usbp);
        chMBObjectInit(&g_ep1_mbox, g_message[0], USB_MAILBOX_SIZE);
        chMBObjectInit(&g_ep2_mbox, g_message[1], USB_MAILBOX_SIZE);
        chMBObjectInit(&g_dma_storage_mbox, g_message[2], USB_MAILBOX_SIZE);

        usbp->ep1_thread = chThdCreateFromHeap(NULL, 1024, NORMALPRIO, UsbEp1Thread, (void*)&USBD1);
        usbp->ep2_thread = chThdCreateFromHeap(NULL, 1024, NORMALPRIO, UsbEp2Thread, (void*)&USBD1);        

        usb_lld_enable_int();
        usb_lld_enable_dma_int();
    }
    /* Configuration.*/
}

/**
 * @brief   Deactivates the USB peripheral.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
void usb_lld_stop(USBDriver *usbp)
{

    osalDbgAssert(usbp, "func()");

    if(usbp->ep1_thread){
      chMBPost(&g_ep1_mbox, USB_THRERAD_EXIT, TIME_INFINITE);
      osThreadTerminate(usbp->ep1_thread);
      usbp->ep1_thread = NULL;
    }

    if(usbp->ep2_thread){
      chMBPost(&g_ep2_mbox, USB_THRERAD_EXIT, TIME_INFINITE);
      osThreadTerminate(usbp->ep2_thread);
      usbp->ep2_thread = NULL;
    }

  /* If in ready state then disables the USB clock.*/
    if (usbp->state == USB_STOP)
    {
        nvicDisableVector(IRQ_USBHOST);
        nvicDisableVector(IRQ_USB_DMA);
        cpmDisableUSB();
    }
}

void usb_lld_dmaServiceStart(USBDriver *usbp)
{
  usbp->dma_thread = chThdCreateFromHeap(NULL, 1024, NORMALPRIO, UsbDmaStorageThread, (void*)usbp);
}

void usb_lld_dmaServiceStop(USBDriver *usbp)
{
  if(usbp->dma_thread){
    chMBPost(&g_dma_storage_mbox, USB_THRERAD_EXIT, TIME_INFINITE);

    osThreadTerminate(usbp->dma_thread);
    usbp->dma_thread = NULL;
  }
}

/**
 * @brief   USB low level reset routine.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
void usb_lld_reset(USBDriver *usbp)
{
    osalDbgAssert(usbp, "func()");

    usbp->epc[0] = &ep0config;
    fifo_config(2, fifo_cfg_ep0);

}

/**
 * @brief   Sets the USB address.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
void usb_lld_set_address(USBDriver *usbp)
{

    osalDbgAssert(usbp, "func()");

    HS_USB->USB_ADDR = usbp->address;
}

/**
 * @brief   Enables an endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_init_endpoint(USBDriver *usbp, usbep_t ep)
{

    const USBEndpointConfig *epcp = usbp->epc[ep];
    uint16_t regval;

    osalDbgAssert(usbp&&epcp, "func()");

    //enable EPx int
    hsusb_ep_select(ep);
    if(epcp->in_cb)
    {
        if(ep)
        {
            uint16_t pktsize = epcp->in_maxsize/8;
            HS_USB->USB_TXMAXP = pktsize;
            regval = HSUSB_TXCSR_CLRDATATOG;
            if (HS_USB->USB_TXCSR&HSUSB_TXCSR_FIFONOTEMPTY)
            {
            	regval |= HSUSB_TXCSR_FLUSHFIFO;
            }
        	if((epcp->ep_mode & USB_EP_MODE_TYPE) == USB_EP_MODE_TYPE_ISOC)
            {
        		regval |= HSUSB_TXCSR_P_ISO;
            }

            /* set twice in case of double buffering */
            HS_USB->USB_TXCSR = regval;
            /* REVISIT may be inappropriate w/o FIFONOTEMPTY ... */
            HS_USB->USB_TXCSR = regval;
        }
    }
    if(epcp->out_cb)
    {
        if(ep)
        {
            uint16_t pktsize = epcp->out_maxsize/8;
            HS_USB->USB_RXMAXP = pktsize;
            regval = HSUSB_RXCSR_FLUSHFIFO | HSUSB_RXCSR_CLRDATATOG;
        	if ((epcp->ep_mode & USB_EP_MODE_TYPE) == USB_EP_MODE_TYPE_ISOC)
            {
        		regval |= HSUSB_TXCSR_P_ISO;
            }

            /* set twice in case of double buffering */
            HS_USB->USB_RXCSR = regval;
            /* REVISIT may be inappropriate w/o FIFONOTEMPTY ... */
            HS_USB->USB_RXCSR = regval;
        }
    }
}

/**
 * @brief   Disables all the active endpoints except the endpoint zero.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
void usb_lld_disable_endpoints(USBDriver *usbp)
{
    osalDbgAssert(usbp, "func()");
}

/**
 * @brief   Returns the status of an OUT endpoint.
 *
 * @param[in] ep        endpoint number
 * @return              The endpoint status.
 * @retval EP_STATUS_DISABLED The endpoint is not active.
 * @retval EP_STATUS_STALLED  The endpoint is stalled.
 * @retval EP_STATUS_ACTIVE   The endpoint is active.
 *
 * @notapi
 */
usbepstatus_t usb_lld_get_status_out(usbep_t ep)
{
    uint16_t regval = 0;

    hsusb_ep_select(ep);
    if(ep)
    {
        regval = HS_USB->USB_TXCSR;
        if(regval&HSUSB_TXCSR_P_SENTSTALL)
        {
            return EP_STATUS_STALLED;
        }

    }
    else
    {
        regval = HS_USB->USB_CSR0;
        if(regval&HSUSB_CSR0_P_SENTSTALL)
        {
            return EP_STATUS_STALLED;
        }
    }
    return EP_STATUS_ACTIVE;
}

/**
 * @brief   Returns the status of an IN endpoint.
 *
 * @param[in] ep        endpoint number
 * @return              The endpoint status.
 * @retval EP_STATUS_DISABLED The endpoint is not active.
 * @retval EP_STATUS_STALLED  The endpoint is stalled.
 * @retval EP_STATUS_ACTIVE   The endpoint is active.
 *
 * @notapi
 */
usbepstatus_t usb_lld_get_status_in(usbep_t ep)
{

    uint16_t regval = 0;

    hsusb_ep_select(ep);
    if(ep)
    {
        regval = HS_USB->USB_RXCSR;
        if(regval&HSUSB_RXCSR_P_SENTSTALL)
        {
            return EP_STATUS_STALLED;
        }

    }
    else
    {
        regval = HS_USB->USB_CSR0;
        if(regval&HSUSB_CSR0_P_SENTSTALL)
        {
            return EP_STATUS_STALLED;
        }
    }
    return EP_STATUS_ACTIVE;

}

/**
 * @brief   Reads a setup packet from the dedicated packet buffer.
 * @details This function must be invoked in the context of the @p setup_cb
 *          callback in order to read the received setup packet.
 * @pre     In order to use this function the endpoint must have been
 *          initialized as a control endpoint.
 * @post    The endpoint is ready to accept another packet.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 * @param[out] buf      buffer where to copy the packet data
 *
 * @notapi
 */
void usb_lld_read_setup(usbep_t ep, uint8_t *buf)
{
    uint8_t i=0;
    volatile uint8_t *pb = (volatile uint8_t *)(OTG_BASE+USB_EPxFIFO_OFFSET(0));

    (void)ep;
    for(i=0;i<8;i++)
    {
        buf[i] = *pb;
    }
}

/**
 * @brief   Prepares for a receive operation for OUT EP
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_prepare_receive(USBDriver *usbp, usbep_t ep)
{
    osalDbgAssert(usbp, "func()");
    (void)ep;
}

/**
 * @brief   Prepares for a transmit operation for IN ep
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_prepare_transmit(USBDriver *usbp, usbep_t ep)
{
    size_t n;
    USBInEndpointState *isp = usbp->epc[ep]->in_state;

    osalDbgAssert(usbp&&isp, "func()");

    nds32_dcache_flush();
    /* Transfer initialization.*/
    //hsusb_ep_select(usbp, ep);
    n = isp->txsize-isp->txcnt;
    if (n > (size_t)usbp->epc[ep]->in_maxsize)
    {
        n = (size_t)usbp->epc[ep]->in_maxsize;
    }

    if (isp->txqueued)
    {
        usb_packet_write_from_queue(usbp, ep,
                                isp->mode.queue.txqueue, n);
        isp->txcnt+=n;
    }
    else
    {
        hsusb_ep_select(ep);
        if(isp->dma_mode == DMA_MODE_NONE)
        {
            uint16_t csr = HS_USB->USB_TXCSR;
            csr &= ~(HSUSB_TXCSR_DMAMODE|HSUSB_TXCSR_AUTOSET|HSUSB_TXCSR_DMAENAB);
            HS_USB->USB_TXCSR = csr;
            usb_packet_write_from_buffer(usbp, ep,
                                         &isp->mode.linear.txbuf[isp->txcnt], n);
            isp->txcnt+=n;
        }
        else if(isp->dma_mode == DMA_MODE_0)
        {
            uint16_t csr = HS_USB->USB_TXCSR;
            csr |= HSUSB_TXCSR_DMAENAB;
            HS_USB->USB_TXCSR = csr;
            csr &= ~HSUSB_TXCSR_DMAMODE;
            HS_USB->USB_TXCSR = csr;
            usb_dma_config_ch(usbp,
                              usbp->epc[ep]->dma_info.dma_ch[1],
                              0,
                              DMA_MODE_0,
                              1,
                              ep,
                              (uint32_t)&isp->mode.linear.txbuf[isp->txcnt],
                              n);
            isp->txcnt+=n;
        }
        else if(isp->dma_mode == DMA_MODE_1)
        {
            uint16_t csr = HS_USB->USB_TXCSR;

            usb_dma_config_ch(usbp,
                              usbp->epc[ep]->dma_info.dma_ch[1],
                              64,
                              DMA_MODE_1,
                              1,
                              ep,
                              (uint32_t)isp->mode.linear.txbuf,
                              isp->txsize);
            isp->txcnt=isp->txsize;
            csr |= HSUSB_TXCSR_DMAMODE|HSUSB_TXCSR_AUTOSET|HSUSB_TXCSR_DMAENAB;
            HS_USB->USB_TXCSR = csr;
        }


    }
}

/**
 * @brief   Starts a receive operation on an OUT endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_start_out(USBDriver *usbp, usbep_t ep)
{

    uint16_t csr;
    USBOutEndpointState *osp = usbp->epc[ep]->out_state;
    uint8_t count;

    osalDbgAssert(usbp&&osp, "func()");

    hsusb_ep_select(ep);
    if(ep==0)
    {
        uint16_t csr = HS_USB->USB_CSR0;
        if(usbp->ep0state == USB_EP0_RX)
        {
            count = HS_USB->USB_COUNT0;

            usb_packet_read_to_buffer(usbp,
                                      ep,
                                      &osp->mode.linear.rxbuf[osp->rxcnt],
                                      count);
            osp->rxcnt += count;
            if(osp->rxsize==osp->rxcnt)
            {
                csr |= HSUSB_CSR0_P_DATAEND|HSUSB_CSR0_P_SVDRXPKTRDY;
                usbp->ep0state = USB_EP0_SENDING_STS;
            }
            else
            {
                csr |= HSUSB_CSR0_P_SVDRXPKTRDY;
            }
        }
        else if(usbp->ep0state == USB_EP0_WAITING_STS)
        {
        }

        HS_USB->USB_CSR0 = csr;
    }
    else
    {
        if(osp->dma_mode == DMA_MODE_NONE)
        {
            count = HS_USB->USB_RXCOUNT1;
            if (osp->rxqueued)
                usb_packet_read_to_queue(usbp,
                                         ep,
                                         osp->mode.queue.rxqueue,
                                         count);
            else
                usb_packet_read_to_buffer(usbp,
                                          ep,
                                         &osp->mode.linear.rxbuf[osp->rxcnt],
                                         (osp->rxsize>count)?count:osp->rxsize);
            osp->rxcnt += count;
            hsusb_ep_select(ep);
            csr = HS_USB->USB_RXCSR;
            csr &= ~HSUSB_RXCSR_RXPKTRDY;
            HS_USB->USB_RXCSR = csr;
            //USBREG->USB_RXCSR = csr;
            usbp->receiving &= ~(1<<ep);
        }
        else if(osp->dma_mode == DMA_MODE_0)
        {
            uint16_t csr = HS_USB->USB_RXCSR;
            count = HS_USB->USB_RXCOUNT1;
            csr |= HSUSB_RXCSR_DMAENAB;
            HS_USB->USB_RXCSR = csr;
            csr &= ~HSUSB_RXCSR_DMAMODE;
            HS_USB->USB_RXCSR = csr;
            usb_dma_config_ch(usbp,
                              usbp->epc[ep]->dma_info.dma_ch[0],
                              0,
                              DMA_MODE_0,
                              0,
                              ep,
                              (uint32_t)&osp->mode.linear.rxbuf[osp->rxcnt],
                              count);
            osp->rxcnt += count;
        }
        else if(osp->dma_mode == DMA_MODE_1)
        {
            uint16_t csr = HS_USB->USB_RXCSR;
            //csr |= HSUSB_RXCSR_DMAENAB|HSUSB_RXCSR_AUTOCLEAR|HSUSB_RXCSR_DMAMODE;
            csr |= HSUSB_RXCSR_DMAENAB|HSUSB_RXCSR_AUTOCLEAR;//|HSUSB_RXCSR_DMAMODE;
            HS_USB->USB_RXCSR = csr;
            //USBREG->USB_INTRXEN &= ~(1<<ep);
            usb_dma_config_ch(usbp,
                              usbp->epc[ep]->dma_info.dma_ch[0],
                              64,
                              DMA_MODE_1,
                              0,
                              ep,
                              (uint32_t)&osp->mode.linear.rxbuf[osp->rxcnt],
                              osp->rxsize - osp->rxcnt);
            osp->rxcnt+=64;// = osp->rxsize;
        }
    }
}

/**
 * @brief   Starts a transmit operation on an IN endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_start_in(USBDriver *usbp, usbep_t ep)
{
    uint16_t csr;

    osalDbgAssert(usbp, ".");
    osalDbgAssert(ep <= USB_MAX_ENDPOINTS, ".");
    

    hsusb_ep_select(ep);
    if(ep==0)
    {
        csr = HS_USB->USB_CSR0;
        if(usbp->ep0state == USB_EP0_TX)
        {
            osalDbgAssert(usbp->epc[0] && usbp->epc[0]->in_state, ".");
            if(usbp->epc[0]->in_state->txcnt==usbp->epc[0]->in_state->txsize)
            {
                csr |= HSUSB_CSR0_P_DATAEND|HSUSB_CSR0_TXPKTRDY;
                usbp->ep0state = USB_EP0_WAITING_STS;
            }
            else
            {
                csr |= HSUSB_CSR0_TXPKTRDY;
                usbp->ep0state = USB_EP0_TX;
            }
        }
        else if(usbp->ep0state == USB_EP0_SENDING_STS)
        {
        }

        HS_USB->USB_CSR0 = csr;

    }
    else
    {        
        USBInEndpointState *isp = usbp->epc[ep]->in_state;
        if(!usbp->epc[ep] || !isp)
          return ;
        
        //osalDbgAssert(usbp->epc[ep] && isp, ".");
        
        if(isp->dma_mode == DMA_MODE_NONE)
        {
            csr = HS_USB->USB_TXCSR;
            csr |= HSUSB_TXCSR_TXPKTRDY;
            HS_USB->USB_TXCSR = csr;

        }
        else
        {
            //dma mode has been set elsewhere
            //csr = USBREG->USB_TXCSR;
            //csr |= HSUSB_TXCSR_TXPKTRDY;

        }

    }
}



/**
 * @brief   Brings an IN endpoint in the stalled state.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_stall_in(usbep_t ep)
{
	uint16_t csr;

    hsusb_ep_select(ep);
    if(ep)
    {
        csr = HS_USB->USB_TXCSR;
        csr |= HSUSB_TXCSR_P_SENDSTALL;
        HS_USB->USB_TXCSR = csr;
    }
    else
    {
        csr = HS_USB->USB_CSR0;
        csr |= HSUSB_CSR0_P_SENDSTALL|HSUSB_CSR0_P_SVDRXPKTRDY;
        HS_USB->USB_CSR0 = csr;
    }
}
/**
 * @brief   Brings an OUT endpoint in the stalled state.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */

void usb_lld_stall_out(usbep_t ep)
{
	uint16_t csr;

  hsusb_ep_select(ep);
    if(ep)
    {
        csr = HS_USB->USB_RXCSR;
        csr |= HSUSB_RXCSR_P_SENDSTALL;
        HS_USB->USB_RXCSR = csr;
    }
    else
    {
        csr = HS_USB->USB_CSR0;
        csr |= HSUSB_CSR0_P_SENDSTALL|HSUSB_CSR0_P_SVDRXPKTRDY;
        HS_USB->USB_CSR0 = csr;
    }
}
/**
 * @brief   Brings an IN endpoint in the active state.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */

void usb_lld_clear_in(usbep_t ep)
{

	uint16_t csr;

    hsusb_ep_select(ep);
    if(ep)
    {
        csr = HS_USB->USB_TXCSR;
        csr &= ~(HSUSB_TXCSR_P_SENDSTALL|HSUSB_TXCSR_P_SENTSTALL);
        HS_USB->USB_TXCSR = csr;
    }
    else
    {
        csr = HS_USB->USB_CSR0;
        csr &= ~(HSUSB_CSR0_P_SENDSTALL|HSUSB_CSR0_P_SENTSTALL);
        HS_USB->USB_CSR0 = csr;
    }
}

/**
 * @brief   Brings an OUT endpoint in the active state.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_clear_out(usbep_t ep)
{

	uint16_t csr;


    hsusb_ep_select(ep);
    if(ep)
    {
        csr = HS_USB->USB_RXCSR;
        csr &= ~(HSUSB_RXCSR_P_SENDSTALL|HSUSB_RXCSR_P_SENTSTALL);
        HS_USB->USB_RXCSR = csr;
    }
    else
    {
        csr = HS_USB->USB_CSR0;
        csr &= ~(HSUSB_CSR0_P_SENDSTALL|HSUSB_CSR0_P_SENTSTALL);
        HS_USB->USB_CSR0 = csr;
    }
}

void usb_lld_servedsetup(uint8_t bHasDataPhase)
{
    uint16_t csr;

    hsusb_ep_select(0);
    csr = HS_USB->USB_CSR0;
    csr |= HSUSB_CSR0_P_SVDRXPKTRDY;
    if(!bHasDataPhase)
    {
        csr |= HSUSB_CSR0_P_DATAEND;
    }
    HS_USB->USB_CSR0 = csr;
}

void usb_dma_config_ch(USBDriver *usbp,
                                    uint8_t  ch,
                                    uint16_t packetSiz,
                                    uint8_t  mode,
                                    uint8_t  epDirIn,
                                    uint8_t  epNum,
                                    uint32_t buf_addr,
                                    uint32_t len)
{
    uint16_t csr = 0;

    osalDbgAssert(usbp&&buf_addr, "func()");

    if(mode == DMA_MODE_1)
    {
        csr |= 1<<2;//mode 1
        csr |= (packetSiz>>3)<<8;//set pkt size for mode 1
    }
    csr |= (epNum<<4)|(1<<0)|(1<<3)|(epDirIn?(1<<1):0);

    USBDMA(ch)->USB_DMA_ADDR = buf_addr;
    USBDMA(ch)->USB_DMA_CNT  = len;
    USBDMA(ch)->USB_DMA_CTL  = csr;
}

//void usb_lld_WriteDmaMode1Patch(USBDriver *usbp, usbep_t ep)
//{
//    //patch of dma mode 1 for USB write(OUT EP)
//    //clear RxPktRdy bit
//    uint16_t csr;
//
//    chDbgAssert(usbp,
//              "func()",
//              "null pointer");
//
//    hsusb_ep_select(usbp, ep);
//    csr = HS_USB->USB_RXCSR;
//    csr &= ~HSUSB_RXCSR_RXPKTRDY;
//   HS_USB->USB_RXCSR = csr;
//}

void usb_lld_connect_bus(USBDriver *usbp)
{
    //HS_SYS->USB_CTRL
    //bit0 -- 1: test mode, 0: normal mode
    //bit4 -- 1: vbus_valid
    //bit5 -- 1: vbus_session
    //bit6 -- 1: vbus_lo
    //bit7 -- 1: reg control; 0: HW control    usbp->usb_pw_sess_mode
    //bit16 -- 1, turnoff hw power detect, 0, turn on
    osalDbgAssert(usbp, "usb_lld_connect_bus()");

    if(usbp->usb_pw_sess_mode == USB_POWER_CONTROL_BY_HW)
    {
        HS_SYS->USB_CTRL &= ~((0x01<<0)|(0x01<<7));
    }
    else if(usbp->usb_pw_sess_mode == USB_POWER_CONTROL_BY_REG)
    {
        /* pd_vbus_det */
        HS_SYS->USB_CTRL &= ~(1 << 16);

        HS_SYS->USB_CTRL &= ~0x01;
        HS_SYS->USB_CTRL |= (0x01<<4)|(0x01<<5)|(0x01<<6)|(0x01<<7);
    }
    else if(usbp->usb_pw_sess_mode == USB_POWER_CONTROL_BY_GPIO)
    {
        palSetPad(HS_GPIO1, 2);
        palSetPad(HS_GPIO1, 3);
        palSetPad(HS_GPIO1, 4);
    }
    else
    {
    }
}

void usb_lld_disconnect_bus(USBDriver *usbp)
{
    //HS_SYS->USB_CTRL
    //bit0 -- 1: test mode, 0: normal mode
    //bit4 -- 1: vbus_valid
    //bit5 -- 1: vbus_session
    //bit6 -- 1: vbus_lo
    //bit7 -- 1: reg control; 0: HW control    usbp->usb_pw_sess_mode
    //bit16 -- 1, turnoff hw power detect, 0, turn on

    osalDbgAssert(usbp, "usb_lld_disconnect_bus()");

    if(usbp->usb_pw_sess_mode == USB_POWER_CONTROL_BY_HW)
    {
    }
    else if(usbp->usb_pw_sess_mode == USB_POWER_CONTROL_BY_REG)
    {
        HS_SYS->USB_CTRL &= ~0x01;
        HS_SYS->USB_CTRL &= ~((0x01<<4)|(0x01<<5)|(0x01<<6)|(0x01<<7));
    }
    else if(usbp->usb_pw_sess_mode == USB_POWER_CONTROL_BY_GPIO)
    {
        palClearPad(HS_GPIO1, 2);
        palClearPad(HS_GPIO1, 3);
        palClearPad(HS_GPIO1, 4);
    }
    else
    {
    }
}

void usb_ldd_clear_datatoggle(usbep_t ep, uint8_t IsInEp)
{
    if(ep)
    {
        hsusb_ep_select(ep);
        if(IsInEp)
        {
            HS_USB->USB_TXCSR |= HSUSB_TXCSR_CLRDATATOG;
        }
        else
        {
            HS_USB->USB_RXCSR |= HSUSB_RXCSR_CLRDATATOG;
        }
    }
}

uint8_t usb_lld_stall_in_sent(usbep_t ep)
{
    if(ep)
    {
        hsusb_ep_select(ep);
        if(HS_USB->USB_TXCSR&HSUSB_TXCSR_P_SENDSTALL)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return 0;
    }
}

uint8_t usb_lld_get_rx_data_len(usbep_t ep)
{
    hsusb_ep_select(ep);
    return HS_USB->USB_RXCOUNT1;

}

void usb_lld_enable_int(void)
{

    nvicEnableVector(IRQ_USBHOST, ANDES_PRIORITY_MASK(HS_USB_USB_IRQ_PRIORITY));
}

void usb_lld_enable_dma_int(void)
{
     nvicEnableVector(IRQ_USB_DMA, ANDES_PRIORITY_MASK(HS_USB_USB_DMA_IRQ_PRIORITY));
}

//resume the usb host
void usb_lld_remote_wakeup(USBDriver *usbp)
{
    osalDbgAssert(usbp, "usb_lld_remote_wakeup()");
    if(!usbp->remote_wake)
    {
        return;
    }
    HS_USB->USB_POWER |= 0x01<<2;
    chThdSleepMilliseconds(10);
    HS_USB->USB_POWER &= ~(0x01<<2);
}

uint8_t usb_lld_read_power_reg(void)
{
    return HS_USB->USB_POWER;
}

void usb_lld_set_power_sess_mode(USBDriver *usbp, uint32_t PwSessMode)
{
    osalDbgAssert(usbp, "usb_lld_set_power_sess_mode()");

    usbp->usb_pw_sess_mode = PwSessMode;
}

void usb_lld_enData(USBDriver *usbp)
{
  nvicDisableVector(IRQ_USBHOST);
  cpmDisableUSB();
  cpmResetUSB();
  cpmEnableUSB();
  
  /* Reset procedure enforced on driver start.*/
  _usb_reset(usbp);
}


#endif /* HAL_USE_USB */


