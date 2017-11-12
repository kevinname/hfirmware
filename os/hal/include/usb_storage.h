/*
 *  pengjiang, 20140730
 */

#ifndef _USB_STORAGE_H_
#define _USB_STORAGE_H_

#define USB_STORAGE_STATE_STOP    0
#define USB_STORAGE_STATE_START   1

#define USB_STORAGE_THREAD_EXIT   0x6601

__PACKED struct bulk_cbw_t {
	uint32_t    Signature;		/* Contains 'USBC' */
	uint32_t	Tag;			/* Unique per command id */
	uint32_t	DataTransferLength;	/* Size of the data */
	uint8_t	    Flags;			/* Direction in bit 7 */
	uint8_t	    Lun;			/* LUN (normally 0) */
	uint8_t	    Length;			/* Of the CDB, <= MAX_COMMAND_SIZE */
	uint8_t	    CDB[16];		/* Command Data Block */
} __PACKED_GCC;
typedef struct bulk_cbw_t bulk_cbw;

__PACKED struct bulk_csw_t {
	uint32_t	Signature;		/* Should = 'USBS' */
	uint32_t	Tag;			/* Same as original command */
	uint32_t	Residue;		/* Amount not transferred */
	uint8_t	    Status;			
} __PACKED_GCC;
typedef struct bulk_csw_t bulk_csw;

#define BULK_CBW_SIGNITURE      0x43425355
#define BULK_CSW_SIGNITURE      0x53425355

#if HAL_USE_USB_STORAGE

#define STAGE_CBW           0
#define STAGE_DATA          1
#define STAGE_CSW           2

#define IDX_EP_OUT          0
#define IDX_EP_IN           1

#define BUF_NUM_DATA_R      3
#define BUF_NUM_DATA_W      2
#define BUF_SIZ_DATA_R      (1*MMCSD_BLOCK_SIZE)
#define BUF_SIZ_DATA_W      (4*MMCSD_BLOCK_SIZE)
#define BUF_SIZ_CMDDATA     64



#define BUF_EMPTY           0
#define BUF_FULL            1
#define BUF_FILLING         2
#define BUF_SENDING         3

#define REPEAT_READ_TIMES   5
#define REPEAT_WRITE_TIMES  5

#define ERROR_1_LEN         0xffffffff
#define ERROR_2_LEN         0xfffffffe


__PACKED struct request_sense_data_t
{
    uint8_t ErrorCode:7;
    uint8_t Valid:1;
    uint8_t Reserved1;
    uint8_t SenseKey:4;
    uint8_t Reserved2:4;
    uint8_t Information[4];
    uint8_t AdditionalSenseLength;
    uint8_t Reserved3[4];
    uint8_t AdditionalSenseCode;
    uint8_t AdditionalSenseCodeQualifier;
    uint8_t Reserved4[4];
} __PACKED_GCC;
typedef struct request_sense_data_t request_sense_data;

__PACKED struct bulk_data_xfer_t {
    uint32_t block;//start block
    uint32_t count;//number of blocks to xfer
    uint32_t count_done;//number of blocks that have been transferred
} __PACKED_GCC;
typedef struct bulk_data_xfer_t bulk_data_xfer;

__PACKED struct r_data_buf_t {
    uint8_t  *pbuf[BUF_NUM_DATA_R];         //[BUF_SIZ_DATA_R];
    uint8_t  head;
    uint8_t  tail;
    uint8_t  reserved[2];
    uint32_t status[BUF_NUM_DATA_R];    
    uint32_t len[BUF_NUM_DATA_R];
} __PACKED_GCC;
typedef struct r_data_buf_t r_data_buf;

__PACKED struct w_data_buf_t {
    uint8_t  *pbuf[BUF_NUM_DATA_W];         //[BUF_SIZ_DATA_W];
    uint8_t  head;
    uint8_t  tail;
    uint16_t rev_num;
    uint32_t status[BUF_NUM_DATA_W]; 
    uint32_t len[BUF_NUM_DATA_W];
} __PACKED_GCC;
typedef struct w_data_buf_t w_data_buf;

typedef bool (*hs_usbstorage_chksd_t)(void);

/**
 * @brief   USB storage Driver configuration structure.
 */
typedef struct {
  /**
   * @brief   USB driver to use.
   */
  USBDriver                 *usbp;  
  bulk_cbw                  cbw_buf;
  bulk_csw                  csw_buf;
  uint8_t                   cmd_data_buf[BUF_SIZ_CMDDATA];
  r_data_buf                r_buf; //from device to host  
  w_data_buf                w_buf; //from host to device
  uint8_t                   stage;
  uint8_t                   state;
  const SDCConfig           *sdcconfig;

  hs_usbstorage_chksd_t     pfnChkSdInsert;

  thread_t *rd_thread;
  thread_t *wr_thread;
} USBStorageDriver;

typedef  size_t (*cbw_handler)(USBStorageDriver *usbstoragep, uint8_t* cmd, uint8_t cmd_idx);

typedef struct 
{
    uint8_t     cmd;
    uint8_t     dir;
    cbw_handler handler;    
    size_t      cmd_len;
    size_t      data_len;
}scsi_cmd;


extern USBStorageDriver  USBSTORAGE1;
#ifdef __cplusplus
extern "C" {
#endif
void usbstorageInit(void);
void usbstorageDataTransmitted(USBDriver *usbp, usbep_t ep);
void usbstorageDataReceived(USBDriver *usbp, usbep_t ep);
void usbstorageObjectInit(USBStorageDriver *usbstoragep,USBDriver *usbp);
void usbstorageStart(USBStorageDriver *usbstoragep, hs_usbstorage_chksd_t pfnSdInsert);
void usbstorageStop(void);
bool_t usbstorage_class_req_handler_if(USBDriver *usbp);
void   usbstorage_dma_outep_cb(USBDriver *usbp, usbep_t ep);
void   usbstorage_dma_inep_cb(USBDriver *usbp, usbep_t ep);
void usb_storage_reset(USBDriver *usbp) ;
void usbstorageSetSdcCfg(USBStorageDriver *usbstoragep, const SDCConfig *sdcconfig);

#ifdef __cplusplus
}
#endif

#endif

//usb bulk only spec definitions
#define MASS_STORAGE_CLASS_CODE         0x08
#define MASS_STORAGE_SUBCLASS_SCSI	    0x06
#define MASS_STORAGE_IFPROTOCAL_BULK	0x50/* Bulk-only */
#define MASS_STORAGE_ENDPOINT_BULK      0x02

//SCSI device types
#define TYPE_DISK	0x00
#define TYPE_CDROM	0x05

//scsi cmd
#define SCSI_FORMAT_UNIT			0x04
#define SCSI_INQUIRY			    0x12
#define SCSI_MODE_SELECT_6		    0x15
#define SCSI_MODE_SELECT_10		    0x55
#define SCSI_MODE_SENSE_6			0x1a
#define SCSI_MODE_SENSE_10		    0x5a
#define SCSI_ALLOW_REMOVAL	        0x1e
#define SCSI_READ_6			        0x08
#define SCSI_READ_10			    0x28
#define SCSI_READ_12			    0xa8
#define SCSI_READ_CAPACITY		    0x25
#define SCSI_READ_CAPACITIES	    0x23
#define SCSI_READ_HEADER			0x44
#define SCSI_READ_TOC			    0x43
#define SCSI_RELEASE			    0x17
#define SCSI_REQUEST_SENSE		    0x03
#define SCSI_RESERVE			    0x16
#define SCSI_SEND_DIAGNOSTIC		0x1d
#define SCSI_START_STOP_UNIT		0x1b
#define SCSI_SYNCHRONIZE_CACHE      0x35
#define SCSI_TEST_UNIT_READY		0x00
#define SCSI_VERIFY_10			    0x2f
#define SCSI_WRITE_6			    0x0a
#define SCSI_WRITE_10			    0x2a
#define SCSI_WRITE_12			    0xaa

#define GET_MAX_LUN                 (0xFE)
#define SET_BULK_RESET              (0xFF)

#define LEN_NOT_FIXED               (-1) 

#define IF_NUM_BULK             0x03

//data direction
#define DIR_IN                  0x80 //from dev to host
#define DIR_OUT                 0x00 //from host to dev
#define DIR_NONE                0xff

#endif
