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

/*
 *   pengjiang, 20140730
 */
#include <string.h>
#include "ch.h"
#include "hal.h"
#include "bmem.h"
#include "cmsis_os.h"
#include "chprintf.h"

#if HAL_USE_USB_STORAGE

#include "usbdev.h"//pj, 20150623

USBStorageDriver  USBSTORAGE1;


static union type_16bit_data   t_buf; 
static bulk_data_xfer g_read_xfer;
static bulk_data_xfer g_write_xfer;
static usbep_t g_bulk_epnum = 0;
//static bool_t g_card_plugged = FALSE;
extern const  scsi_cmd   cmd_data[];
extern SDCDriver  SDCD0;

#define US_MAILBOX_SIZE 10
static mailbox_t g_read_sync_mbox, g_write_sync_mbox;
static msg_t g_message[2][US_MAILBOX_SIZE];
static mailbox_t g_write_mbox, g_read_mbox;
static msg_t g_thread_message[2][US_MAILBOX_SIZE];

static mutex_t g_mp_r;
static mutex_t g_mp_w;

thread_t *g_thread_w = NULL;

//static uint8_t g_test1 = 0;

//static WORKING_AREA(waUsbStorageReadThread, (256));
static void UsbStorageReadThread(void *arg);
//static WORKING_AREA(waUsbStorageWriteThread, (256));
static void UsbStorageWriteThread(void *arg);

#define USB_DBG 0
#if USB_DBG
#include <stdio.h>
#define USB_PRINTF(fmt,args...)	printf(fmt, ##args)
#else
#define USB_PRINTF(a)
#endif

#define usb_memset(a,b,c) memset(a,b,c)
#define usb_memcpy(a,b,c) memcpy(a,b,c)

#if 0
void usb_memset(uint8_t* buf, uint8_t val, size_t len)
{
    size_t i;


    chDbgAssert(buf,
                "usb_memset()",
                "null pointer");
    
    for(i=0;i<len;i++)
    {
        buf[i] = val;
    }
}

void usb_memcpy(uint8_t* descbuf, uint8_t* srcbuf, size_t len)
{
    size_t i;

    chDbgAssert(descbuf&&srcbuf,
                "usb_memcpy()",
                "null pointer");
    
    for(i=0;i<len;i++)
    {
        descbuf[i] = srcbuf[i];
    }
}
#endif

void usbCleanWriteBuf(w_data_buf *pwb)
{
  if(pwb->pbuf[0])
    usb_memset(pwb->pbuf[0], 0, BUF_SIZ_DATA_W);

  if(pwb->pbuf[1])
    usb_memset(pwb->pbuf[1], 0, BUF_SIZ_DATA_W);

  usb_memset((uint8_t *)pwb + 8, 0, sizeof(w_data_buf) - 8);
}

void usbCleanReadBuf(r_data_buf *prb)
{
  if(prb->pbuf[0])
    usb_memset(prb->pbuf[0], 0, BUF_SIZ_DATA_R);

  if(prb->pbuf[1])
    usb_memset(prb->pbuf[1], 0, BUF_SIZ_DATA_R);

  if(prb->pbuf[2])
    usb_memset(prb->pbuf[2], 0, BUF_SIZ_DATA_R);

  usb_memset((uint8_t *)prb + 12, 0, sizeof(r_data_buf) - 12);
}


void wakeup_w_thread(void)
{
    if (g_thread_w != NULL) {
        chSchReadyI(g_thread_w);
        g_thread_w = NULL;
        
    }
    else
    {
        //g_testcnt_1 = 1;
    }
}

static void clear_data_xfer_para(bulk_data_xfer* data_xfer)
{
    chDbgAssert(data_xfer,
                "clear_data_xfer_para()");
    usb_memset(data_xfer, 0 ,sizeof(*data_xfer));
}

static void set_data_xfer_para(bulk_data_xfer* data_xfer, 
                            uint32_t        block, 
                            uint32_t        count, 
                            uint32_t        count_done)
{
    chDbgAssert(data_xfer,
                "set_data_xfer_para()");

    data_xfer->block = block;
    data_xfer->count = count;
    data_xfer->count_done = count_done;
}

static void stall_inep_request(USBStorageDriver *usbstoragep, uint8_t wait)
{
    uint8_t epnum = 2;

    chDbgAssert(usbstoragep,
                "stall_inep_request()");
    
    usb_stall_in(epnum);
    setEpStatus(epnum, TRUE, TRUE); 
    if(wait)
    {
        while(TRUE == usb_stall_in_sent(epnum))
        {  
            chSysUnlock();
            chThdSleepMilliseconds(10); 
            chSysLock();
        }
        usb_clear_datatoggle(epnum, TRUE);
    }
    
}
    
static size_t inquiry_cmd(USBStorageDriver *usbstoragep, uint8_t* cmd, uint8_t cmd_idx)
{
    uint8_t	*buf = usbstoragep->cmd_data_buf;
    size_t xferdatalen = usbstoragep->cbw_buf.DataTransferLength;

    chDbgAssert(usbstoragep&&cmd&&buf,
                "inquiry_cmd()");

    
    if(xferdatalen>BUF_SIZ_DATA_R)
    {
        return ERROR_1_LEN;
    }    
    usb_memset(buf, 0, BUF_SIZ_CMDDATA);  //xferdatalen);
	buf[0] = TYPE_DISK;
    buf[1] = 0x80;
	buf[2] = 2;		    /* ANSI SCSI level 2 */
	buf[3] = 2;		    /* SCSI-2 INQUIRY data format */
	buf[4] = 31;		/* Additional length */
				        /* No special options */
    usb_memset(&buf[8], ' '/*0x20*/, 28);
    usb_memcpy(&buf[8], "HS6601", 6);     /* Vendor, max 8-byte */
    usb_memcpy(&buf[16],"USB Storage", 11); /* Product */

    return min(xferdatalen,cmd_data[cmd_idx].data_len);
}

static size_t test_unit_ready(USBStorageDriver *usbstoragep, uint8_t* cmd, uint8_t cmd_idx)
{
    (void)cmd_idx;
    
    chDbgAssert(usbstoragep&&cmd,
                "test_unit_ready()");

    chSysUnlock();
    usbstoragep->pfnChkSdInsert();
    chSysLock();
    return 0;
}

static size_t allow_removal(USBStorageDriver *usbstoragep, uint8_t* cmd, uint8_t cmd_idx)
{
    (void)cmd_idx;
    
    chDbgAssert(usbstoragep&&cmd,
                "allow_removal()");
    return 0;
}

static size_t request_sense(USBStorageDriver *usbstoragep, uint8_t* cmd, uint8_t cmd_idx)
{
    size_t xferdatalen = usbstoragep->cbw_buf.DataTransferLength;
    request_sense_data *buf = (request_sense_data *)usbstoragep->cmd_data_buf;

    chDbgAssert(usbstoragep&&buf&&cmd,
                "request_sense()");
    
    usb_memset(buf, 0, 18);
    buf->ErrorCode = 0x70;
    buf->Valid = 0;
    buf->SenseKey = 2;
    buf->Information[0] = 0;
    buf->Information[1] = 0;
    buf->Information[2] = 0;
    buf->Information[3] = 0;
    buf->AdditionalSenseLength = 0x0a;
    buf->AdditionalSenseCode   = 0x3a;
    buf->AdditionalSenseCodeQualifier = 0;

    return min(xferdatalen,cmd_data[cmd_idx].data_len);
}

static size_t mode_sense_6(USBStorageDriver *usbstoragep, uint8_t* cmd, uint8_t cmd_idx)
{
    uint8_t	*buf = usbstoragep->cmd_data_buf;
    size_t xferdatalen = usbstoragep->cbw_buf.DataTransferLength;

    chDbgAssert(usbstoragep&&buf&&cmd,
                "mode_sense_6()");
    
    buf[0] = 3;
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = 0;
    return min(xferdatalen,cmd_data[cmd_idx].data_len);
}

static size_t read_capacities(USBStorageDriver *usbstoragep, uint8_t* cmd, uint8_t cmd_idx)
{
    uint8_t	*buf = usbstoragep->cmd_data_buf;
    uint32_t sector_count = 0;
    uint32_t sector_size = 0;
    BlockDeviceInfo bdi;
    size_t xferdatalen = usbstoragep->cbw_buf.DataTransferLength;

    chDbgAssert(usbstoragep&&buf&&cmd,
                "read_capacities()");
    
    if(usbstoragep->pfnChkSdInsert())
    {
        if(HAL_SUCCESS == sdcGetInfo(&SDCD0, &bdi))
        {
            sector_count = bdi.blk_num-1;
            sector_size = bdi.blk_size;
        }
        
        *(uint32_t*)&buf[0] = 0x08000000;
        buf[4] = sector_count >> 24;
        buf[5] = 0xff & (sector_count >> 16);
        buf[6] = 0xff & (sector_count >> 8);
        buf[7] = 0xff & (sector_count);
        buf[8] = 0x02;
        buf[9] = 0xff & (sector_size >> 16);
        buf[10] = 0xff & (sector_size >> 8);
        buf[11] = 0xff & sector_size;
    }
    else
    {
        return ERROR_1_LEN;
        #if 0
        usb_stall_in(g_bulk_epnum);
        while(FALSE == usb_stall_in_sent())
        {  
        }
        usb_clear_in(g_bulk_epnum);
        usb_clear_datatoggle(g_bulk_epnum, TRUE);
        return 0;
        #endif
    }


    return min(xferdatalen,cmd_data[cmd_idx].data_len);
    
}

static size_t read_capacity(USBStorageDriver *usbstoragep, uint8_t* cmd, uint8_t cmd_idx)
{
    uint8_t	*buf = usbstoragep->cmd_data_buf;
    uint32_t sector_count = 0;
    uint32_t sector_size = 0;
    BlockDeviceInfo bdi;
    size_t xferdatalen = usbstoragep->cbw_buf.DataTransferLength;

    chDbgAssert(usbstoragep&&buf&&cmd,
                "read_capacity()");

    if(usbstoragep->pfnChkSdInsert())
    {
        chSysUnlock();
        if(HAL_SUCCESS == sdcGetInfo(&SDCD0, &bdi))
        {
            sector_count = bdi.blk_num-1;
            sector_size = bdi.blk_size;
        }
        chSysLock();
        
        buf[0] = sector_count >> 24;
        buf[1] = 0xff & (sector_count >> 16);
        buf[2] = 0xff & (sector_count >> 8);
        buf[3] = 0xff & (sector_count);
        buf[4] = 0x00;
        buf[5] = 0xff & (sector_size >> 16);
        buf[6] = 0xff & (sector_size >> 8);
        buf[7] = 0xff & sector_size;
    }
    else
    {
        return ERROR_1_LEN;
        #if 0
        usb_stall_in(g_bulk_epnum);
        while(FALSE == usb_stall_in_sent())
        {  
        }
        usb_clear_in(g_bulk_epnum);
        usb_clear_datatoggle(g_bulk_epnum, TRUE);
        return 0;
        #endif
    }

    return min(xferdatalen,cmd_data[cmd_idx].data_len);
}

static size_t read_10(USBStorageDriver *usbstoragep, uint8_t* cmd, uint8_t cmd_idx)
{
	(void)cmd_idx;
    uint32_t block = 0;
    uint32_t count = 0;
    size_t xferdatalen = usbstoragep->cbw_buf.DataTransferLength;

    chDbgAssert(usbstoragep&&cmd,
                "read_10()");

    if(!(usbstoragep->cbw_buf.Flags&0x80))
    {
        return ERROR_2_LEN;
    }
    
    if(!usbstoragep->pfnChkSdInsert())
    {
        return ERROR_1_LEN;
        #if 0
        usb_stall_in(g_bulk_epnum);
        while(FALSE == usb_stall_in_sent(g_bulk_epnum))
        {  
        }
        //usb_clear_in(usbstoragep->usbp, g_bulk_epnum);
        usb_clear_datatoggle(g_bulk_epnum, TRUE);
        return 0;
        #endif
    }
        
    block = (cmd[2]<<24)|(cmd[3]<<16)|(cmd[4]<<8)|(cmd[5]<<0);
    count = (cmd[7]<<8)|(cmd[8]<<0);
    usbCleanReadBuf(&usbstoragep->r_buf);
    clear_data_xfer_para(&g_read_xfer);
    set_data_xfer_para(&g_read_xfer, block, count, 0);

    return min(xferdatalen,count*MMCSD_BLOCK_SIZE);
}

static size_t write_10(USBStorageDriver *usbstoragep, uint8_t* cmd, uint8_t cmd_idx)
{
	(void)cmd_idx;
    uint32_t block = 0;
    uint32_t count = 0;
    size_t xferdatalen = usbstoragep->cbw_buf.DataTransferLength;

    chDbgAssert(usbstoragep&&cmd,
                "write_10()");
        
    block = (cmd[2]<<24)|(cmd[3]<<16)|(cmd[4]<<8)|(cmd[5]<<0);
    count = (cmd[7]<<8)|(cmd[8]<<0);
    usbCleanWriteBuf(&usbstoragep->w_buf);
    clear_data_xfer_para(&g_write_xfer);
    set_data_xfer_para(&g_write_xfer, 
                        block, 
                        count, 
                        0);

    return (xferdatalen==count*MMCSD_BLOCK_SIZE)?xferdatalen:ERROR_2_LEN;
}

static size_t verify_10(USBStorageDriver *usbstoragep, uint8_t* cmd, uint8_t cmd_idx)
{
	(void)cmd;
    (void)cmd_idx;
	(void)usbstoragep;
    return 0;
}

static size_t start_stop(USBStorageDriver *usbstoragep, uint8_t* cmd, uint8_t cmd_idx)
{      
    //pj, 20150623
    chDbgAssert(usbstoragep,
                "start_stop()"); 

    #if 1
    (void)cmd;
    (void)cmd_idx;
    #else
    uint8_t status = usbstoragep->cbw_buf.CDB[4];
    //flag for eject action in PC, g_UsbCfgInfo.Usb_PwSess, bit31
    if((status&0x03) == 0x02)
    {
        g_UsbCfgInfo.Usb_PwSess |= 1UL<<31;
    }
    else if((status&0x03) == 0x01)
    {
        g_UsbCfgInfo.Usb_PwSess &= ~(1UL<<31);
    }
    //end
    #endif
    
    return 0;
}

const scsi_cmd cmd_data[] =
{
    {SCSI_WRITE_10,        DIR_OUT,  write_10,       10, LEN_NOT_FIXED},
    {SCSI_READ_10,         DIR_IN,   read_10,        10, LEN_NOT_FIXED},        
    {SCSI_TEST_UNIT_READY, DIR_NONE, test_unit_ready, 6,  0},
    {SCSI_REQUEST_SENSE,   DIR_IN,   request_sense,   6, 18},
    {SCSI_INQUIRY,         DIR_IN,   inquiry_cmd,     8, 36},
    {SCSI_ALLOW_REMOVAL,   DIR_NONE, allow_removal,   6,  0},
    {SCSI_MODE_SENSE_6,    DIR_IN,   mode_sense_6,    6,  4},             
    {SCSI_START_STOP_UNIT, DIR_NONE, start_stop,      6,  0},            
    {SCSI_READ_CAPACITIES, DIR_IN,   read_capacities,10, 12},            
    {SCSI_READ_CAPACITY,   DIR_IN,   read_capacity,  10,  8},
    {SCSI_VERIFY_10,       DIR_NONE, verify_10,      10, 0},
};

static uint32_t usbstorage_analyse_scsi_cmd(USBDriver *usbp, uint8_t* cmd, uint8_t len)
{
	(void)len;
    USBStorageDriver *usbstoragep = usbp->in_params[0];
    uint32_t dataxferlen = 0;
    uint32_t i = 0;

    chDbgAssert(usbp&&usbstoragep&&cmd,
                "usbstorage_analyse_scsi_cmd()");
    
    for(i=0; i<sizeof(cmd_data)/sizeof(scsi_cmd); i++)
    {
        if(cmd_data[i].cmd == cmd[0])
        {
            if(cmd_data[i].handler)
            {
                //chSysUnlock();
                dataxferlen = cmd_data[i].handler(usbstoragep, cmd, i);
                //chSysLock();
                return dataxferlen;
            }
            else
            {
                break;
            }
            
        }
    }    

    if((usbstoragep->cbw_buf.Flags==0)&&(usbstoragep->cbw_buf.DataTransferLength))
    {
        return ERROR_2_LEN;
    }
    else
    {
        return ERROR_1_LEN;
    }
}

static uint32_t usbstorage_analyse_cbw(USBDriver *usbp, uint16_t cbwlen)
{
    USBStorageDriver *usbstoragep = usbp->in_params[0];
    uint32_t datalen = 0;

    chDbgAssert(usbp&&usbstoragep,
                "usbstorage_analyse_cbw()");


    //printf("sig=%08x,lun=%x,len=%d,cbwlen=%d\n\r",usbstoragep->cbw_buf.Signature,usbstoragep->cbw_buf.Lun,usbstoragep->cbw_buf.Length,cbwlen);

    
    if((usbstoragep->cbw_buf.Signature!= BULK_CBW_SIGNITURE)
        ||usbstoragep->cbw_buf.Lun
        ||usbstoragep->cbw_buf.Length>16
        ||cbwlen!=31)
    {
        datalen=ERROR_1_LEN;
        goto error_to_stall;
    }
    usbstoragep->csw_buf.Signature = BULK_CSW_SIGNITURE;
    usbstoragep->csw_buf.Tag = usbstoragep->cbw_buf.Tag;
    datalen = usbstorage_analyse_scsi_cmd(usbp, usbstoragep->cbw_buf.CDB, usbstoragep->cbw_buf.Length);
    if(datalen==ERROR_1_LEN||datalen==ERROR_2_LEN)
    {
        goto error_to_stall;
    }
    
    if(datalen==0)
    {
        usbstoragep->stage = STAGE_CSW;
        usbstoragep->csw_buf.Status = 0;
    }
    else
    {
        
        usbstoragep->stage = STAGE_DATA;
    }
    //datalen = usbstorage_analyse_scsi_cmd(usbp, usbstoragep->cbw_buf.CDB, usbstoragep->cbw_buf.Length);
    usbstoragep->csw_buf.Residue = datalen;
    return datalen;
error_to_stall:
    stall_inep_request(usbstoragep, (datalen==ERROR_2_LEN)?FALSE:TRUE);
    usbstoragep->stage = STAGE_CSW;
    usbstoragep->csw_buf.Status = 1;
    return datalen;
    
}

void usbstorage_handle_cbw(USBDriver *usbp, usbep_t ep)
{
    USBStorageDriver *usbstoragep = usbp->in_params[0];
    USBOutEndpointState *osp = usbp->epc[ep]->out_state; 
    uint32_t datalen = usbstorage_analyse_cbw(usbp, osp->rxcnt);

    chDbgAssert(usbp&&usbstoragep&&osp,
                "usbstorage_handle_cbw()");
    
    if(datalen == ERROR_1_LEN || datalen == ERROR_2_LEN)
    {
        usbstoragep->csw_buf.Status = 1;
        #if 0
        usb_stall_in(ep);
        setEpStatus(ep, TRUE, TRUE);
        usbstoragep->stage = STAGE_CSW;
        usb_clear_datatoggle(g_bulk_epnum, TRUE);
        return;
        #endif
    }
    else
    {
        usbstoragep->csw_buf.Status = 0; 
    }
    
    
    if(usbstoragep->stage == STAGE_DATA)
    {
        if(usbstoragep->cbw_buf.Flags&DIR_IN)
        {
            //send data to host
            if(usbstoragep->cbw_buf.CDB[0] != SCSI_READ_10)
            {
                usbPrepareTransmit(usbp, ep, usbstoragep->cmd_data_buf, datalen, DMA_MODE_NONE);
                usbStartTransmitI(usbp, ep);                
            }
            else
            {
                chMBPostS(&g_read_mbox, TRUE, TIME_INFINITE);
                //USB_PRINTF("E");
            }
        }
        else
        {
            //get data from host
            osp->rxcnt = 0;
            osp->rxsize = 0; 
            
        }
    }
    else if(usbstoragep->stage == STAGE_CSW)
    {   
        if(datalen == ERROR_2_LEN)
        {
            return;
        }
    
        usbstoragep->csw_buf.Residue = 0;

        #if 1
        if((usbstoragep->cbw_buf.CDB[0] == SCSI_TEST_UNIT_READY||SCSI_READ_CAPACITY ==usbstoragep->cbw_buf.CDB[0])
           &&!usbstoragep->pfnChkSdInsert())
        #else
        //pj, 20150623, fix eject problem, add 'g_os_eject' judgement condition
        if((usbstoragep->cbw_buf.CDB[0] == SCSI_TEST_UNIT_READY||SCSI_READ_CAPACITY ==usbstoragep->cbw_buf.CDB[0])
           &&(!g_card_plugged||(g_UsbCfgInfo.Usb_PwSess&(1UL<<31))))
        #endif
        {
            usbstoragep->csw_buf.Status = 1;
        }
        else
        {
            //usbstoragep->csw_buf.Status = 0;//pj, 20150618            
        }
        usbPrepareTransmit(usbp, ep, (uint8_t*)&usbstoragep->csw_buf, sizeof(bulk_csw), DMA_MODE_NONE);
        usbStartTransmitI(usbp, ep);
        //usbstoragep->stage == STAGE_CBW;
    }
}

void usbstorageInit(void) 
{
    USBSTORAGE1.state = USB_STORAGE_STATE_STOP;
}

void usbstorage_dma_outep_cb(USBDriver *usbp, usbep_t ep)
{
    (void)ep;
    //USBStorageDriver *usbstoragep = usbp->in_params[0];
    //USBOutEndpointState *osp = usbp->epc[ep]->out_state; 
    //g_testcnt_3 = 9;
    chDbgAssert(usbp,
                "usbstorage_handle_cbw()");


    //if(osp->dma_mode == DMA_MODE_1)
    //{
    //    _usb_isr_invoke_out_cb(usbp,ep); 
   // }
    //else
    //{
       wakeup_w_thread(); 
    //}   

    //return ;   
}

void usbstorage_dma_inep_cb(USBDriver *usbp, usbep_t ep)
{
    USBInEndpointState *isp = usbp->epc[ep]->in_state;

    chDbgAssert(usbp&&isp,
                "usbstorage_dma_inep_cb()");
    
    #if 1
    //for dma 1
    if(isp->dma_mode == DMA_MODE_1)
    {
        isp->dma_mode = DMA_MODE_NONE;
        _usb_isr_invoke_in_cb(usbp, ep);
        
    }
    #endif
          
       
}

uint8_t g_isPaused = FALSE;
void usbstorageDataTransmitted(USBDriver *usbp, usbep_t ep)
{
    USBStorageDriver *usbstoragep = usbp->in_params[0];
    USBInEndpointState *isp = usbp->epc[ep]->in_state; 

    chDbgAssert(usbp&&usbstoragep&&isp,
                "usbstorageDataTransmitted()");


    g_bulk_epnum = ep;

    if(usbp->epc[ep]->in_state->txsize > usbp->epc[ep]->in_state->txcnt)
    {
        isp->dma_mode = usbp->epc[ep]->dma_info.dma_mode[IDX_EP_IN];
        usbPrepareTransmit_2(usbp, ep);
        usbStartTransmitI(usbp, ep);
    }
    else
    {
        
        if(usbstoragep->stage == STAGE_DATA)
        {
            //chMtxLockS(&g_mp_r);
            usbstoragep->csw_buf.Residue -= usbp->epc[ep]->in_state->txcnt;          
            usbstoragep->r_buf.status[usbstoragep->r_buf.tail] = BUF_EMPTY;
            
            if(usbstoragep->csw_buf.Residue==0)
            {        
                usbstoragep->stage = STAGE_CBW;
                usbstoragep->csw_buf.Status = 0;            
                usbPrepareTransmit(usbp, ep, (uint8_t*)&usbstoragep->csw_buf, sizeof(bulk_csw), DMA_MODE_NONE);
                usbStartTransmitI(usbp, ep); 
            }
            else
            {
                uint32_t status = 0;
                usbstoragep->r_buf.tail++;
                usbstoragep->r_buf.tail%=BUF_NUM_DATA_R;
                chMBPostS(&g_read_mbox, TRUE, TIME_INFINITE);
                status = usbstoragep->r_buf.status[usbstoragep->r_buf.tail];          
                if(status == BUF_FULL)
                {
                    usbstoragep->r_buf.status[usbstoragep->r_buf.tail] = BUF_SENDING;          
                
                    
                    usbPrepareTransmit(usbp, 
                                        ep, 
                                        usbstoragep->r_buf.pbuf[usbstoragep->r_buf.tail],
                                        MMCSD_BLOCK_SIZE, 
                                        usbp->epc[ep]->dma_info.dma_mode[IDX_EP_IN]);
                    usbStartTransmitI(usbp, ep);
                    g_isPaused = FALSE;
                }
                else
                {
                    g_isPaused = TRUE;
                }
                
                
            }
            //chMtxUnlockS();
        }
        else if(usbstoragep->stage == STAGE_CSW)
        {
            usbstoragep->stage = STAGE_CBW;
        }
    }
}

static uint32_t error_datalen = 0;//for usb cv msc test
void usbstorageDataReceived(USBDriver *usbp, usbep_t ep)
{
    USBStorageDriver *usbstoragep = usbp->in_params[0];
    uint8_t recvbuf[64];
    USBOutEndpointState *osp = usbp->epc[ep]->out_state;

    chDbgAssert(usbp&&usbstoragep&&osp,
                "usbstorageDataReceived()");

    
    g_bulk_epnum = ep;

    if (usbstoragep == NULL)
        return;

    
    if(usbstoragep->stage == STAGE_CBW)
    {
        uint8_t dma_mode = DMA_MODE_NONE;//usbp->epc[ep]->dma_info.dma_mode[IDX_EP_OUT];
        //if(dma_mode == DMA_MODE_1)
        ///{
        //    dma_mode = DMA_MODE_0;
        //}
        //chMtxLockS(&g_mp_w);
        usbPrepareReceive(usbp, 
                          ep,
                          recvbuf,
                          64,
                          dma_mode);
        usbStartReceiveI(usbp, ep);
        usb_memcpy(&usbstoragep->cbw_buf, recvbuf, sizeof(bulk_cbw));
        //chMtxUnlockS();
        if(dma_mode==DMA_MODE_NONE)
        {
            usbstorage_handle_cbw(usbp, ep);              
        }    
        else
        {
        }
        
    }
    else if(usbstoragep->stage == STAGE_DATA)
    {
        if(usbp->epc[ep]->dma_info.dma_mode[IDX_EP_OUT]==DMA_MODE_1)
        {
        #if 0
            if(osp->rxcnt == 0)
            {        
                msg_t msg;
                uint8_t w_patch = TRUE;
                if(usbstoragep->w_buf.rev_num>=BUF_NUM_DATA_W)
                {
                    chMBFetchS(&g_write_sync_mbox, &msg, TIME_INFINITE);
                }         

                chMtxLockS(&g_mp_w);
                usbstoragep->w_buf.status[usbstoragep->w_buf.head] = BUF_FILLING;
                
                usbClearReceiveStatusI(usbp, ep);
                usbPrepareReceive(usbp, 
                                  ep, 
                                  usbstoragep->w_buf.buf[usbstoragep->w_buf.head], 
                                  64, 
                                  DMA_MODE_NONE);
                usbStartReceiveI(usbp, ep);
                usbClearReceiveStatusI(usbp, ep);
                if(BUF_SIZ_DATA_W<usbstoragep->csw_buf.Residue)
                {
                    usbstoragep->w_buf.len[usbstoragep->w_buf.head] = BUF_SIZ_DATA_W;
                }
                else
                {
                    usbstoragep->w_buf.len[usbstoragep->w_buf.head] = usbstoragep->csw_buf.Residue;
                }  

                if(w_patch)   
                {
                    usbPrepareReceive(usbp, 
                                      ep, 
                                      usbstoragep->w_buf.buf[usbstoragep->w_buf.head], 
                                      usbstoragep->w_buf.len[usbstoragep->w_buf.head]-64, 
                                      DMA_MODE_1); 
                    osp->rxcnt = 64;  
                    usbStartReceiveI(usbp, ep);                   
                }
                else
                {
                    usbPrepareReceive(usbp, 
                                      ep, 
                                      usbstoragep->w_buf.buf[usbstoragep->w_buf.head], 
                                      usbstoragep->w_buf.len[usbstoragep->w_buf.head], 
                                      DMA_MODE_1); 
                    osp->rxcnt = 64;  
                    usbStartReceiveI(usbp, ep);                      
                }     
                
                chMtxUnlockS();
                return;
                #if 1
                //g_test1 = 1;
                //chSysLock();
                g_thread_w = chThdSelf();
                chSchGoSleepS(THD_STATE_SUSPENDED);
                //chSysUnlock();
                //g_test1 = 2;
                if(w_patch) 
                {
                    return;
                }
                #endif
                
            }  

            if(osp->rxcnt == usbstoragep->w_buf.len[usbstoragep->w_buf.head]-64)
            {   
                #if 1
                //g_test1 = 3;
                
                usbPrepareReceive(usbp, 
                                  ep, 
                                  usbstoragep->w_buf.buf[usbstoragep->w_buf.head], 
                                  64, 
                                  DMA_MODE_NONE);
                osp->rxcnt = usbstoragep->w_buf.len[usbstoragep->w_buf.head]-64;
                usbStartReceiveI(usbp, ep);
                osp->rxsize = usbstoragep->w_buf.len[usbstoragep->w_buf.head];
                //g_test1 = 4;
                #endif
            }

            if(osp->rxcnt==usbstoragep->w_buf.len[usbstoragep->w_buf.head])
            {
                
                    msg_t msg;
                    //g_test1 = 5;
                    chMtxLockS(&g_mp_w);                    
                    usbstoragep->w_buf.status[usbstoragep->w_buf.head] = BUF_FULL;
                    usbstoragep->csw_buf.Residue -= osp->rxcnt;  
                    usbstoragep->w_buf.head++;
                    usbstoragep->w_buf.head%=BUF_NUM_DATA_W; 
                    usbstoragep->w_buf.rev_num++;
                    osp->rxcnt = 0;
                    usbClearReceiveStatusI(usbp, ep);
                    chMtxUnlockS();
                    chMBPostS(&g_write_mbox, TRUE, TIME_INFINITE);
                    //USB_PRINTF("i");
                    if(!usbstoragep->csw_buf.Residue)
                    {   
                        uint8_t i = 255;
                        //g_test1 = 6;
                        //printf("w");
                        while(i--)
                        {
                            chMBFetchS(&g_write_sync_mbox, &msg, TIME_INFINITE);
                            if(msg==FALSE)
                            {
                                break;
                            }
                        }
                        //g_test1 = 7;
                        usbstoragep->stage = STAGE_CBW;
                        usbstoragep->csw_buf.Status = 0;
                        usbClearReceiveStatusI(usbp, ep);
                        usbPrepareTransmit(usbp, ep, (uint8_t*)&usbstoragep->csw_buf, sizeof(bulk_csw), DMA_MODE_NONE);
                        usbStartTransmitI(usbp, ep);
                        //g_test1 = 8;
                        //usbClearTransmitStatusI(usbp, ep);  
                        chMBPostS(&g_write_mbox, FALSE, TIME_INFINITE);
                        //g_test1 = 9;
                    }
                    else
                    {
                    }

            }
           #endif 
        }
        else if(usbp->epc[ep]->dma_info.dma_mode[IDX_EP_OUT]==DMA_MODE_0)
        {
            if(osp->rxcnt == 0)
            {
                msg_t msg;
            

                if(usbstoragep->w_buf.rev_num>=BUF_NUM_DATA_W)
                {
                    chMBFetchS(&g_write_sync_mbox, &msg, TIME_INFINITE);
                }                     


                chMtxLockS(&g_mp_w);
                usbstoragep->w_buf.status[usbstoragep->w_buf.head] = BUF_FILLING;
                
                if(BUF_SIZ_DATA_W<usbstoragep->csw_buf.Residue)
                {
                    usbstoragep->w_buf.len[usbstoragep->w_buf.head] = BUF_SIZ_DATA_W;
                }
                else
                {
                    usbstoragep->w_buf.len[usbstoragep->w_buf.head] = usbstoragep->csw_buf.Residue;
                }  

                nds32_dcache_flush();
                usbPrepareReceive(usbp, 
                                  ep, 
                                  usbstoragep->w_buf.pbuf[usbstoragep->w_buf.head],
                                  usbstoragep->w_buf.len[usbstoragep->w_buf.head], 
                                  DMA_MODE_0);
                usbStartReceiveI(usbp, ep);
                chMtxUnlockS(&g_mp_w);
                g_thread_w = currp;
                chSchGoSleepS(CH_STATE_SUSPENDED);                 
            }
            else if(osp->rxcnt<osp->rxsize)
            {
                //chMtxLockS(&g_mp_w);
                osp->dma_mode = usbp->epc[ep]->dma_info.dma_mode[IDX_EP_OUT];
                nds32_dcache_flush();
                usbPrepareReceive_2(usbp, ep);
                usbStartReceiveI(usbp, ep); 
                //chMtxUnlockS();
                g_thread_w = currp;
                chSchGoSleepS(CH_STATE_SUSPENDED);
            }

            if(osp->rxcnt==osp->rxsize)
            {
                msg_t msg;
                chMtxLockS(&g_mp_w);                    
                usbstoragep->w_buf.status[usbstoragep->w_buf.head] = BUF_FULL;
                usbstoragep->csw_buf.Residue -= osp->rxcnt;  
                usbstoragep->w_buf.head++;
                usbstoragep->w_buf.head%=BUF_NUM_DATA_W; 
                usbstoragep->w_buf.rev_num++;
                osp->rxcnt = 0;
                usbClearReceiveStatusI(usbp, ep);
                chMtxUnlockS(&g_mp_w);
                chMBPostS(&g_write_mbox, TRUE, TIME_INFINITE);
                if(!usbstoragep->csw_buf.Residue)
                {   
                    uint8_t i = 255;
                    while(i--)
                    {
                        chMBFetchS(&g_write_sync_mbox, &msg, TIME_INFINITE);
                        if(msg==FALSE)
                        {
                            break;
                        }
                    }
                    usbstoragep->stage = STAGE_CBW;
                    usbstoragep->csw_buf.Status = 0;
                    usbClearReceiveStatusI(usbp, ep);
                    usbPrepareTransmit(usbp, ep, (uint8_t*)&usbstoragep->csw_buf, sizeof(bulk_csw), DMA_MODE_NONE);
                    usbStartTransmitI(usbp, ep);
                    
                    //chMBPostS(&g_write_mbox, FALSE, TIME_INFINITE);
                }
            }
        }
    }
    else
    {
        //error handling code
        //to pass usbcv msc test

        //receive wrong packet
        usbPrepareReceive(usbp, 
                          ep,
                          recvbuf,
                          64,
                          DMA_MODE_NONE);
        usbStartReceiveI(usbp, ep);  
        error_datalen += osp->rxcnt;
        if(error_datalen == usbstoragep->cbw_buf.DataTransferLength)
        {        
            //wait stall finished            
            while(TRUE == usb_stall_in_sent(ep))
            {  
                chThdSleepS(MS2ST(10)); 
            }
            usb_clear_datatoggle(ep, TRUE);
            error_datalen = 0;

            //send csw with status = 1
            usbClearTransmitStatusI(usbp, ep);
            usbstoragep->csw_buf.Residue = 0;
            usbstoragep->csw_buf.Status = 1;
            usbstoragep->csw_buf.Signature = BULK_CSW_SIGNITURE;
            usbstoragep->csw_buf.Tag = usbstoragep->cbw_buf.Tag;            
            usbPrepareTransmit(usbp, ep, (uint8_t*)&usbstoragep->csw_buf, sizeof(bulk_csw), DMA_MODE_NONE);
            if(usbStartTransmitI(usbp, ep))
            {
                //printf("TX fail\n\r");
            }
            else
            {
                //printf("TX OK\n\r");
            }
            usbstoragep->stage = STAGE_CBW;
        }
    }
}

void usbstorageObjectInit(USBStorageDriver *usbstoragep,USBDriver *usbp) 
{
    chDbgAssert(usbp&&usbstoragep,
                "usbstorageObjectInit()");


    usbstoragep->usbp = usbp;
    usbp->in_params[0]  = usbstoragep;
}

void usbstorageSetSdcCfg(USBStorageDriver *usbstoragep, const SDCConfig *sdcconfig)
{
    usbstoragep->sdcconfig = sdcconfig;
}

bool usbStorageChksdInsert(void)
{
  return false;
}

void usbstorageStart(USBStorageDriver *usbstoragep, hs_usbstorage_chksd_t pfnSdInsert)
{
    //const SDCConfig *sdcconfig = usbstoragep->sdcconfig;
    chDbgAssert(usbstoragep,
                "usbstorageStart()");   

    if(usbstoragep->state == USB_STORAGE_STATE_START)
      return ;
    
    usbstoragep->stage = STAGE_CBW;
    usb_memset(&usbstoragep->cbw_buf,    0, sizeof(bulk_cbw));
    usb_memset(&usbstoragep->csw_buf,    0, sizeof(bulk_csw));
    usb_memset(&usbstoragep->w_buf,      0, sizeof(w_data_buf));    
    usb_memset(&usbstoragep->r_buf,      0, sizeof(r_data_buf));   

    usbstoragep->w_buf.pbuf[0] = osBmemAlloc(BUF_SIZ_DATA_W);
    usbstoragep->w_buf.pbuf[1] = osBmemAlloc(BUF_SIZ_DATA_W);
    
    usbstoragep->r_buf.pbuf[0] = osBmemAlloc(BUF_SIZ_DATA_R);
    usbstoragep->r_buf.pbuf[1] = osBmemAlloc(BUF_SIZ_DATA_R);
    usbstoragep->r_buf.pbuf[2] = osBmemAlloc(BUF_SIZ_DATA_R);
    
    clear_data_xfer_para(&g_read_xfer);
    clear_data_xfer_para(&g_write_xfer);
    chMBObjectInit(&g_read_sync_mbox, g_message[0], US_MAILBOX_SIZE);
    chMBObjectInit(&g_read_mbox, g_thread_message[0], US_MAILBOX_SIZE);
    chMBObjectInit(&g_write_sync_mbox, g_message[1], US_MAILBOX_SIZE);
    chMBObjectInit(&g_write_mbox, g_thread_message[1], US_MAILBOX_SIZE);
    chMtxObjectInit(&g_mp_r);
    chMtxObjectInit(&g_mp_w);
    
    usbstoragep->rd_thread = chThdCreateFromHeap(NULL, 1024,
                    NORMALPRIO,
                    UsbStorageReadThread, 
                    (void*)usbstoragep);
    usbstoragep->wr_thread = chThdCreateFromHeap(NULL, 1024,
                    NORMALPRIO,
                    UsbStorageWriteThread, 
                    (void*)usbstoragep); 

    usb_lld_dmaServiceStart(usbstoragep->usbp);
    usbstoragep->state = USB_STORAGE_STATE_START;
    usbstoragep->pfnChkSdInsert = pfnSdInsert == NULL ? usbStorageChksdInsert : pfnSdInsert;
}

void usbstorageStop(void)
{
	if(USBSTORAGE1.state == USB_STORAGE_STATE_STOP)
		return ;

    USBSTORAGE1.state = USB_STORAGE_STATE_STOP;    

    if(USBSTORAGE1.rd_thread){
      chMBPost(&g_read_mbox, USB_STORAGE_THREAD_EXIT, TIME_INFINITE);
      osThreadTerminate(USBSTORAGE1.rd_thread);
      USBSTORAGE1.rd_thread = NULL;
    }

    if(USBSTORAGE1.wr_thread){
      chMBPost(&g_write_mbox, USB_STORAGE_THREAD_EXIT, TIME_INFINITE);
      osThreadTerminate(USBSTORAGE1.wr_thread);
      USBSTORAGE1.wr_thread = NULL;
    }

    usb_lld_dmaServiceStop(USBSTORAGE1.usbp);

    osBmemFree(USBSTORAGE1.w_buf.pbuf[0]);
    osBmemFree(USBSTORAGE1.w_buf.pbuf[1]);

    osBmemFree(USBSTORAGE1.r_buf.pbuf[0]);
    osBmemFree(USBSTORAGE1.r_buf.pbuf[1]);
    osBmemFree(USBSTORAGE1.r_buf.pbuf[2]);

    usb_memset(&USBSTORAGE1.w_buf, 0, sizeof(w_data_buf));    
    usb_memset(&USBSTORAGE1.r_buf, 0, sizeof(r_data_buf));
}

void usb_storage_reset(USBDriver *usbp) 
{
    USBStorageDriver* usbstoragep = usbp->in_params[0];

    chDbgAssert(usbp&&usbstoragep,
                "usb_storage_reset()");

    usb_memset(&usbstoragep->cbw_buf,    0, sizeof(bulk_cbw));
    usb_memset(&usbstoragep->csw_buf,    0, sizeof(bulk_csw));
    usbCleanReadBuf(&usbstoragep->r_buf);
    usbCleanWriteBuf(&usbstoragep->w_buf);

    clear_data_xfer_para(&g_read_xfer);
    clear_data_xfer_para(&g_write_xfer);
    usb_ldd_clear_datatoggle(2, TRUE);
    usb_ldd_clear_datatoggle(2, FALSE);
    //usbstoragep->stage = STAGE_CBW;
    
    //g_UsbCfgInfo.Usb_PwSess &= ~(1UL<<31);//pj, 20150623
    
}

bool_t usbstorage_class_req_handler_if(USBDriver *usbp) 
{
    uint16_t xferlen = usbFetchWord(&usbp->setup[6]);
    bool_t ret = FALSE;

    chDbgAssert(usbp,
                "usbstorage_class_req_handler_if()");
    
    if((usbp->setup[0] & USB_RTYPE_DIR_MASK) == USB_RTYPE_DIR_DEV2HOST)
    {
        if(usbp->setup[1] == GET_MAX_LUN)
        {
            if(usbp->setup[2]==0&&usbp->setup[3]==0&&usbp->setup[4]==IF_NUM_BULK&&usbp->setup[5]==0)
            {
                xferlen = min(1,xferlen);
                t_buf.u8vals[0] = 0;
                usbSetupTransfer(usbp, (uint8_t *)&t_buf.u8vals[0], xferlen, NULL);
                ret = TRUE;
            }
            else
            {
                ret = FALSE;
            }
        }
    }
    else
    {
        if(usbp->setup[1] == SET_BULK_RESET)
        {
            USBStorageDriver* usbstoragep = usbp->in_params[0];
            usb_clear_datatoggle(g_bulk_epnum?g_bulk_epnum:2, TRUE);
            usb_clear_datatoggle(g_bulk_epnum?g_bulk_epnum:2, FALSE);
            usb_clear_in(g_bulk_epnum);
            usb_clear_out(g_bulk_epnum);
            usbstoragep->stage = STAGE_CBW;
            xferlen = min(0,xferlen);
            usbSetupTransfer(usbp, NULL, xferlen, NULL);

            ret = TRUE;
        }
    }    
    usb_finish_ep0setup(usbp, 0);
    return ret;
}

#if 0
uint8_t get_buf_status_cnt(w_data_buf* buf, uint32_t status)
{
    uint8_t cnt = 0;
    uint8_t i = 0;
    for(i=0;i<BUF_NUM_DATA_W;i++)
    {
        if(buf->status[i] == status)
        {
            cnt++;
        }
    }
    return cnt;
}
#endif

static void UsbStorageReadThread(void *arg) 
{
    USBStorageDriver *usbstoragep;// = (USBStorageDriver *)arg;
    USBDriver *usbp;// = usbstoragep->usbp;
    uint8_t cnt = 0;
    msg_t msg;

    chRegSetThreadName("usbStorageRead");
    while(1)
    {          
        chMBFetch(&g_read_mbox, &msg, TIME_INFINITE);

        if(USB_STORAGE_THREAD_EXIT == msg)
          break;

        chSysLock();
        //chMtxLockS(&g_mp_r);    
        usbstoragep = (USBStorageDriver *)arg;
        usbp = usbstoragep->usbp;
        //chMtxUnlockS(); 

        chDbgAssert(usbp&&usbstoragep,
                "UsbStorageReadThread()");
        
        if(g_read_xfer.count_done<g_read_xfer.count)
        {
            uint32_t status = 0;
            
            chMtxLockS(&g_mp_r);
            status = usbstoragep->r_buf.status[usbstoragep->r_buf.head];
            chMtxUnlockS(&g_mp_r);
            while(status == BUF_EMPTY)
            {
                chSysUnlock();
                //chSysLock();
                #if 1
                cnt = REPEAT_READ_TIMES;
                while(cnt>0)
                {

                    if(HAL_SUCCESS==sdcRead(&SDCD0, 
                                           g_read_xfer.block+g_read_xfer.count_done, 
                                           usbstoragep->r_buf.pbuf[usbstoragep->r_buf.head],
                                           1)
                                           )
                    {
                        break;
                    }
                    cnt--;
                }
                #endif
                //chSysUnlock();
                chSysLock();
                chMtxLockS(&g_mp_r);
                usbstoragep->r_buf.status[usbstoragep->r_buf.head] = BUF_FULL;
                g_read_xfer.count_done++;          
                usbstoragep->r_buf.head++;
                usbstoragep->r_buf.head%=BUF_NUM_DATA_R;
                if(g_read_xfer.count_done==1||g_isPaused)
                {
                    usbstoragep->r_buf.status[usbstoragep->r_buf.tail] = BUF_SENDING;
                    usbPrepareTransmit( usbp, 
                                        g_bulk_epnum, 
                                        usbstoragep->r_buf.pbuf[usbstoragep->r_buf.tail],
                                        MMCSD_BLOCK_SIZE, 
                                        usbp->epc[g_bulk_epnum]->dma_info.dma_mode[IDX_EP_IN]);
                    usbStartTransmitI(usbp, g_bulk_epnum); 
                    g_isPaused = FALSE;
                }      
                else
                {             
                }
                if(g_read_xfer.count_done==g_read_xfer.count)
                {
                    chMtxUnlockS(&g_mp_r);
                    break;
                }   
                status = usbstoragep->r_buf.status[usbstoragep->r_buf.head];
                chMtxUnlockS(&g_mp_r);
            }
        }
        chSysUnlock();
    }

}


//static uint8_t w_thread_cnt = 0;

static void UsbStorageWriteThread(void *arg) 
{
    USBStorageDriver *usbstoragep = (USBStorageDriver *)arg;
    uint8_t cnt = 0;
    msg_t msg;
    //uint8_t flag = 0;

    chRegSetThreadName("usbStorageWrite");
    while(1)
    {      
        chMBFetch(&g_write_mbox, &msg, TIME_INFINITE);
        if(USB_STORAGE_THREAD_EXIT == msg)
          break;
        
        chDbgAssert(usbstoragep,
                "UsbStorageWriteThread()");
        
        chSysLock();
        usbstoragep = (USBStorageDriver *)arg;       
        if(g_write_xfer.count_done<g_write_xfer.count)
        {
            uint32_t status = 0;
            chMtxLockS(&g_mp_w);
            status = usbstoragep->w_buf.status[usbstoragep->w_buf.tail];
            chMtxUnlockS(&g_mp_w);
            while(status == BUF_FULL)
            {
                //USB_PRINTF("x");
                chSysUnlock();
                #if 1
                cnt = REPEAT_WRITE_TIMES;
                while(cnt>0)
                {
                    if(HAL_SUCCESS==sdcWrite(&SDCD0, 
                                            g_write_xfer.block+g_write_xfer.count_done, 
                                            usbstoragep->w_buf.pbuf[usbstoragep->w_buf.tail],
                                            usbstoragep->w_buf.len[usbstoragep->w_buf.tail]>>9)
                                            )
                    {
                        break;
                    }
                    cnt--;
                }    
                #endif
                chSysLock();
                chMtxLockS(&g_mp_w);
                usbstoragep->w_buf.status[usbstoragep->w_buf.tail] = BUF_EMPTY;
                g_write_xfer.count_done += usbstoragep->w_buf.len[usbstoragep->w_buf.tail]>>9;   
                usbstoragep->w_buf.tail++;
                usbstoragep->w_buf.tail%=BUF_NUM_DATA_W;   
                chMtxUnlockS(&g_mp_w);
                if(g_write_xfer.count_done==g_write_xfer.count)
                {
                    chMBPostS(&g_write_sync_mbox, FALSE, TIME_INFINITE);
                    break;
                }
                else
                {   
                    chMBPostS(&g_write_sync_mbox, TRUE, TIME_INFINITE);
                }
                chMBFetchS(&g_write_mbox, &msg, TIME_INFINITE);
                if(USB_STORAGE_THREAD_EXIT == msg)
                  break;
                //if(msg == FALSE)
                //{
                //    break;
                //}
                chMtxLockS(&g_mp_w);
                status = usbstoragep->w_buf.status[usbstoragep->w_buf.tail]; 
                chMtxUnlockS(&g_mp_w);
            }
        }
        chSysUnlock();
    }

}


#endif
