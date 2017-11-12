/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012,2013 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes ChibiOS/RT, without being obliged to provide
    the source code for any proprietary components. See the file exception.txt
    for full details of how and when the exception can be applied.
*/

/**
 * @file    serial_usb.c
 * @brief   Serial over USB Driver code.
 *
 * @addtogroup SERIAL_USB
 * @{
 */

#include "ch.h"
#include "hal.h"

#if HAL_USE_USB_SERIAL

SerialUSBDriver SDU1;

thread_t *g_thread_inep = NULL;
thread_t *g_thread_outep = NULL;

//static mutex_t g_mutex_r;
//static mutex_t g_mutex_w;

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*
 * Current Line Coding.
 */
static cdc_linecoding_t linecoding = {
  {0x00, 0xc2, 0x01, 0x00},             /* 115200 */
  LC_STOP_1, LC_PARITY_NONE, 8
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*
 * Interface implementation.
 */

static size_t write(void *ip, const uint8_t *bp, size_t n) {

  return chOQWriteTimeout(&((SerialUSBDriver *)ip)->oqueue, bp,
                          n, TIME_INFINITE);
}

static size_t read(void *ip, uint8_t *bp, size_t n) {

  return chIQReadTimeout(&((SerialUSBDriver *)ip)->iqueue, bp,
                         n, TIME_INFINITE);
}

static msg_t put(void *ip, uint8_t b) {

  return chOQPutTimeout(&((SerialUSBDriver *)ip)->oqueue, b, TIME_INFINITE);
}

static msg_t get(void *ip) {

  return chIQGetTimeout(&((SerialUSBDriver *)ip)->iqueue, TIME_INFINITE);
}

static msg_t putt(void *ip, uint8_t b, systime_t timeout) {

  return chOQPutTimeout(&((SerialUSBDriver *)ip)->oqueue, b, timeout);
}

static msg_t gett(void *ip, systime_t timeout) {

  return chIQGetTimeout(&((SerialUSBDriver *)ip)->iqueue, timeout);
}

static size_t writet(void *ip, const uint8_t *bp, size_t n, systime_t time) {

  return chOQWriteTimeout(&((SerialUSBDriver *)ip)->oqueue, bp, n, time);
}

static size_t readt(void *ip, uint8_t *bp, size_t n, systime_t time) {

  return chIQReadTimeout(&((SerialUSBDriver *)ip)->iqueue, bp, n, time);
}

static const struct SerialUSBDriverVMT vmt = {
  write, read, put, get,
  putt, gett, writet, readt
};

/**
 * @brief   Notification of data removed from the input queue.
 */
static void inotify(io_queue_t *qp) {
  size_t n;//, maxsize;
  SerialUSBDriver *sdup = chQGetLinkX(qp);

  /* If the USB driver is not in the appropriate state then transactions
     must not be started.*/
  if ((usbGetDriverStateI(sdup->config->usbp) != USB_ACTIVE) ||
      (sdup->state != SDU_READY))
    return;



      if (g_thread_outep)
      {
        n = chIQGetEmptyI(&sdup->iqueue);
        if(n>=usb_get_rx_data_len(sdup->config->bulk_out))
        {
            chSchReadyI(g_thread_outep);
            g_thread_outep = NULL;
        }
      }


}

void sduSendDataI(void *arg)
{
  size_t n;
  volatile uint16_t tx = 2;
    
  SerialUSBDriver *sdup = (SerialUSBDriver *)arg;

  chSysLockFromISR();
  n = chOQGetFullI(&sdup->oqueue);
  
  while(n)
  {
    while(tx) 
      tx = usbGetTransmitStatusI(sdup->config->usbp, sdup->config->bulk_in);    

    if(n>64)
        n = 64;
    usbPrepareQueuedTransmit(sdup->config->usbp,
                             sdup->config->bulk_in,
                             &sdup->oqueue, n, DMA_MODE_NONE);

    usbStartTransmitI(sdup->config->usbp, sdup->config->bulk_in);

    if(!sdup->config->usbp->epc[sdup->config->bulk_in])
      break;
    
	  n = chOQGetFullI(&sdup->oqueue);

    if(sdup->state == SDU_STOP)
      break;
  }
  chSysUnlockFromISR();
}

/**
 * @brief   Notification of data inserted into the output queue.
 */
static void onotify(io_queue_t *qp) {
  size_t n;
  SerialUSBDriver *sdup = chQGetLinkX(qp);

  /* If the USB driver is not in the appropriate state then transactions
     must not be started.*/
  if ((usbGetDriverStateI(sdup->config->usbp) != USB_ACTIVE) ||
      (sdup->state != SDU_READY))
    return;
  
  n = chOQGetFullI(&sdup->oqueue);
  if(0)  //(n < 50)
  {
    chVTSetI(&sdup->ovt, MS2ST(10), sduSendDataI, sdup);
    return ;
  }

  if (!usbClearTransmitStatusI(sdup->config->usbp, sdup->config->bulk_in)) 
    usbClearTransmitStatusI(sdup->config->usbp, sdup->config->bulk_in);
  
  while(n)
  {
    volatile uint16_t tx = 2;
    
    while(tx) 
      tx = usbGetTransmitStatusI(sdup->config->usbp, sdup->config->bulk_in); 

    if(n>64)
        n = 64;

    if(!sdup->config->usbp->epc[sdup->config->bulk_in])
      break;
    
    usbPrepareQueuedTransmit(sdup->config->usbp,
                             sdup->config->bulk_in,
                             &sdup->oqueue, n, DMA_MODE_NONE);

    usbStartTransmitI(sdup->config->usbp, sdup->config->bulk_in);

    n = chOQGetFullI(&sdup->oqueue);
    if(n > 0)
    {
      g_thread_inep = currp;
      chSchGoSleepS(CH_STATE_SUSPENDED);
      chThdSleepS(MS2ST(5));
    }  
	
  }
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Serial Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void sduInit(void) 
{
  sduObjectInit(&SDU1);
}

/**
 * @brief   Initializes a generic full duplex driver object.
 * @details The HW dependent part of the initialization has to be performed
 *          outside, usually in the hardware initialization code.
 *
 * @param[out] sdup     pointer to a @p SerialUSBDriver structure
 *
 * @init
 */
void sduObjectInit(SerialUSBDriver *sdup) {

  sdup->vmt = &vmt;
  chEvtObjectInit(&sdup->event);
  chVTObjectInit(&sdup->ovt);
  sdup->state = SDU_STOP;
  chIQObjectInit(&sdup->iqueue, sdup->ib, SERIAL_USB_BUFFERS_SIZE, inotify, sdup);
  chOQObjectInit(&sdup->oqueue, sdup->ob, SERIAL_USB_BUFFERS_SIZE, onotify, sdup);
}

/**
 * @brief   Configures and starts the driver.
 *
 * @param[in] sdup      pointer to a @p SerialUSBDriver object
 * @param[in] config    the serial over USB driver configuration
 *
 * @api
 */
void sduStart(SerialUSBDriver *sdup, const SerialUSBConfig *config) {
  USBDriver *usbp = config->usbp;

  osalDbgCheck(sdup != NULL);

  osalSysLock();
  osalDbgAssert((sdup->state == SDU_STOP) || (sdup->state == SDU_READY), "sduStart(), #1");
  usbp->in_params[config->bulk_in - 1]   = sdup;
  usbp->out_params[config->bulk_out - 1] = sdup;
  usbp->in_params[config->int_in - 1]    = sdup;
  sdup->config = config;
  sdup->state = SDU_READY;
  //chMtxInit(&g_mutex_r);
  //chMtxInit(&g_mutex_w);
  chSysUnlock();
}

/**
 * @brief   Stops the driver.
 * @details Any thread waiting on the driver's queues will be awakened with
 *          the message @p Q_RESET.
 *
 * @param[in] sdup      pointer to a @p SerialUSBDriver object
 *
 * @api
 */
void sduStop(SerialUSBDriver *sdup) {
  USBDriver *usbp = sdup->config->usbp;

  osalDbgCheck(sdup != NULL);

  osalSysLock();

  osalDbgAssert((sdup->state == SDU_STOP) || (sdup->state == SDU_READY), "sduStop(), #1");
  if(sdup->state == SDU_STOP)
  {
    chSysUnlock();
    return ;
  }

  /* Driver in stopped state.*/
  usbp->in_params[sdup->config->bulk_in - 1]   = NULL;
  usbp->out_params[sdup->config->bulk_out - 1] = NULL;
  usbp->in_params[sdup->config->int_in - 1]    = NULL;
  sdup->state = SDU_STOP;

  /* Queues reset in order to signal the driver stop to the application.*/
  chnAddFlagsI(sdup, CHN_DISCONNECTED);
  chIQResetI(&sdup->iqueue);
  chOQResetI(&sdup->oqueue);

  chSysUnlock();
}

/**
 * @brief   USB device configured handler.
 *
 * @param[in] sdup      pointer to a @p SerialUSBDriver object
 *
 * @iclass
 */
void sduConfigureHookI(SerialUSBDriver *sdup) {
  USBDriver *usbp = sdup->config->usbp;

  chIQResetI(&sdup->iqueue);
  chOQResetI(&sdup->oqueue);
  chnAddFlagsI(sdup, CHN_CONNECTED);

  /* Starts the first OUT transaction immediately.*/
  usbPrepareQueuedReceive(usbp, sdup->config->bulk_out, &sdup->iqueue,
                          usbp->epc[sdup->config->bulk_out]->out_maxsize, DMA_MODE_NONE);
  usbStartReceiveI(usbp, sdup->config->bulk_out);
}

/**
 * @brief   Default requests hook.
 * @details Applications wanting to use the Serial over USB driver can use
 *          this function as requests hook in the USB configuration.
 *          The following requests are emulated:
 *          - CDC_GET_LINE_CODING.
 *          - CDC_SET_LINE_CODING.
 *          - CDC_SET_CONTROL_LINE_STATE.
 *          .
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @return              The hook status.
 * @retval TRUE         Message handled internally.
 * @retval FALSE        Message not handled.
 */
bool_t class_req_handler_serial_if(USBDriver *usbp) {

  if ((usbp->setup[0] & USB_RTYPE_TYPE_MASK) == USB_RTYPE_TYPE_CLASS) {
    switch (usbp->setup[1]) {
    case CDC_GET_LINE_CODING:
      usbSetupTransfer(usbp, (uint8_t *)&linecoding, sizeof(linecoding), NULL);
      usb_finish_ep0setup(usbp, 0);
      return TRUE;
    case CDC_SET_LINE_CODING:
      usbSetupTransfer(usbp, (uint8_t *)&linecoding, sizeof(linecoding), NULL);
      usb_finish_ep0setup(usbp, 0);
      return TRUE;
    case CDC_SET_CONTROL_LINE_STATE:
      /* Nothing to do, there are no control lines.*/
      usbSetupTransfer(usbp, NULL, 0, NULL);
      usb_finish_ep0setup(usbp, 0);
      return TRUE;
    default:
      return FALSE;
    }
  }
  return FALSE;
}

/**
 * @brief   Default data transmitted callback.
 * @details The application must use this function as callback for the IN
 *          data endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 */
void sduDataTransmitted(USBDriver *usbp, usbep_t ep) 
{  
	(void)usbp;
	(void)ep;
    if (g_thread_inep!= NULL) {
        chSchReadyI(g_thread_inep);
        g_thread_inep = NULL;
        
    }

}

/**
 * @brief   Default data received callback.
 * @details The application must use this function as callback for the OUT
 *          data endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 */
void sduDataReceived(USBDriver *usbp, usbep_t ep) 
{
    size_t n, maxsize;
    SerialUSBDriver *sdup = usbp->out_params[ep - 1];

    if (sdup == NULL)
        return;


    /* Writes to the input queue can only happen when there is enough space
      to hold at least one packet.*/
   
    maxsize = usbp->epc[ep]->out_maxsize;
    n = chIQGetEmptyI(&sdup->iqueue);
    while(n<usb_get_rx_data_len(ep))
    {

        g_thread_outep = currp;
        chSchGoSleepS(CH_STATE_SUSPENDED); 
        n = chIQGetEmptyI(&sdup->iqueue);
    }

    /* The endpoint cannot be busy, we are in the context of the callback,
       so a packet is in the buffer for sure.*/
    usbPrepareQueuedReceive(usbp, ep, &sdup->iqueue, maxsize, DMA_MODE_NONE);
    usbStartReceiveI(usbp, ep);


}

/**
 * @brief   Default data received callback.
 * @details The application must use this function as callback for the IN
 *          interrupt endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 */
void sduInterruptTransmitted(USBDriver *usbp, usbep_t ep) {

  (void)usbp;
  (void)ep;
}

bool_t sduIsReady(USBDriver *usbp)
{
    return (usb_is_ready(usbp)&&(SDU1.state == SDU_READY));
}

#endif /* HAL_USE_SERIAL */

/** @} */
