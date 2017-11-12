/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 2014-2019 HUNTERSUN Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Module Name:
	gatt_ui.h
Abstract:
	This file defines GATT ui macros, functions and defainations.
-----------------------------------------------------------------------------*/

#ifndef GATT_UIH
#define GATT_UIH

#include "global.h"
#include "l2cap_ui.h"

/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*-----------------------------------------------------------------------------*/
#define GATT_MAX_DATA_LEN      23


// opcode
#define    ATT_OPCO_ERROR_RES                   0x01
#define    ATT_OPCO_EXCHANGE_MTU_REQ            0x02
#define    ATT_OPCO_EXCHANGE_MTU_RES            0x03
#define    ATT_OPCO_FIND_INFO_REQ               0x04
#define    ATT_OPCO_FIND_INFO_RES               0x05
#define    ATT_OPCO_FIND_BY_TYPE_VAL_REQ        0x06
#define    ATT_OPCO_FIND_BY_TYPE_VAL_RES        0x07
#define    ATT_OPCO_READ_BY_TYPE_REQ            0x08
#define    ATT_OPCO_READ_BY_TYPE_RES            0x09
#define    ATT_OPCO_READ_REQ                    0x0A
#define    ATT_OPCO_READ_RES                    0x0B
#define    ATT_OPCO_READ_BLOB_REQ               0x0C
#define    ATT_OPCO_READ_BLOB_RES               0x0D
#define    ATT_OPCO_READ_MULTIPLE_REQ           0x0E
#define    ATT_OPCO_READ_MULTIPLE_RES           0x0F
#define    ATT_OPCO_READ_BY_GRP_TYPE_REQ        0x10
#define    ATT_OPCO_READ_BY_GRP_TYPE_RES        0x11
#define    ATT_OPCO_WRITE_REQ                   0x12
#define    ATT_OPCO_WRITE_RES                   0x13
#define    ATT_OPCO_WRITE_COMMAND               0x52
#define    ATT_OPCO_PREPARE_WRITE_REQ           0x16
#define    ATT_OPCO_PREPARE_WRITE_RES           0x17
#define    ATT_OPCO_EXCUTE_WRITE_REQ            0x18
#define    ATT_OPCO_EXCUTE_WRITE_RES            0x19
#define    ATT_OPCO_HDL_VAL_NOTIFY              0x1B
#define    ATT_OPCO_HDL_VAL_INDICATE            0x1D
#define    ATT_OPCO_HDL_VAL_CONFIRM             0x1E
#define    ATT_OPCO_SIGNED_WRITE_CMD            0xD2 

#define GATT_TRANSACTION_MODE_NONE      0x0
#define GATT_TRANSACTION_MODE_ACTIVE    0x1
#define GATT_TRANSACTION_MODE_EXECUTE   0x2
#define GATT_TRANSACTION_MODE_CANCEL    0x3

enum
{
    /*Name						       Error Code        Description*/
    ERCO_ATT_SUCCESS               =    0x00,           /*Customized definition for att operation success*/
	ERCO_ATT_INVALID_HANDLE		   =    0x01, 			/*The attribute handle given was not valid on this server.*/
    ERCO_ATT_READ_NOT_PERMITTED    =    0x02, 			/*The attribute cannot be read.*/
    ERCO_ATT_WRITE_NOT_PERMITTED   =    0x03, 			/*The attribute cannot be written.*/
    ERCO_ATT_INVALID_PDU 		   =    0x04, 			/*The attribute PDU was invalid.*/
    ERCO_ATT_INSUFF_AUTHEN         =    0x05, 			/*The attribute requires authentication before it can be read or written.*/
    ERCO_ATT_REQ_NOT_SUPPORTED 	   =    0x06, 			/*Attribute server does not support the request received from the client.*/
    ERCO_ATT_INVALID_OFFSET        =    0x07, 			/*Offset specified was past the end of the attribute.*/
    ERCO_ATT_INSUFF_AUTHOR         =    0x08, 			/*The attribute requires authorization before it can be read or written.*/
    ERCO_ATT_PREPARE_QUEUE_FULL    =    0x09, 			/*Too many prepare writes have been queued.*/
    ERCO_ATT_ATTRIBUTE_NOT_FOUND   =    0x0A, 			/*No attribute found within the given attribute handle range.*/
    ERCO_ATT_ATTRIBUTE_NOT_LONG    =    0x0B, 			/*The attribute cannot be read or written using the Read Blob Request*/
    ERCO_ATT_INSUFF_ENC_KEY_SIZE   =    0x0C, 			/*The Encryption Key Size used for encrypting this link is insufficient.*/
    ERCO_ATT_INVALID_ATTR_VAL_LEN  =	0x0D, 			/*The attribute value length is invalid for the operation.*/
    ERCO_ATT_UNLIKELY_ERROR 	   =    0x0E, 			/*The attribute request that was requested has encountered an error that was unlikely, and thefore could not be completed as requested.*/
    ERCO_ATT_INSUFF_ENCRYP 		   =    0x0F, 			/*The attribute requires encryption before it can be read or written.*/
    ERCO_ATT_UNSUPPORTED_GROUP_TYPE=	0x10, 			/*The attribute type is not a supported grouping attribute as defined by a higher layer specication.*/
    ERCO_ATT_INSUFF_RESOURCES 	   =	0x11, 			/*Insufficient Resources to complete the request.*/
    ERCO_ATT_RESERVED_START 	   =    0x12, 			/*Reserved for future use.*/
    ERCO_ATT_RESERVED_END          =    0x7F, 			/*Reserved for future use.*/

	ERCO_ATT_APP_ERR_START         =    0x80, 			/*Application error code defined by a higher layer specification.*/
	ERCO_ATT_APP_MEM_ERROR         =    0x81, 			/*Upper layer memory error*/
    ERCO_ATT_APP_ACCESS_PEND       =    0x82,           /*DB need upper layer to specify */
	ERCO_ATT_APP_ERR_END           =    0xff, 			/*Application error code defined by a higher layer specification.*/

};

typedef enum {/* Events for upper layer */
   GATT_EV_CONNECT_COMPLETE     = 0x01,
   GATT_EV_DISCONNECT_COMPLETE  = 0x02,
}GATT_EVENT_TYPE;

struct GATT_RegCbkStru {
   void *cbk;
};

struct GATT_ConnectCompleteStru {
   struct HCI_AddressStru addr;
   UINT8    result;
   UINT8    side;
   UINT16   mtu;
};

typedef void (GATT_CbkFuc)(UINT16 msg, void *arg);
typedef UINT8 GATT_SignatureStru[12];
typedef UINT16 (*GATT_ReadCbk)(UINT16 handle, UINT16 offset, UINT8 * buffer, UINT16 buffer_size);
typedef UINT16 (*GATT_WriteCbk)(UINT16 handle, UINT16 transaction_mode, UINT16 offset, UINT8 *buffer, UINT16 buffer_size, GATT_SignatureStru * signature);

/*-----------------------------------------------------------------------------
Description:
	Input Function.
-----------------------------------------------------------------------------*/

FUNC_EXPORT void GATT_Init(void);
FUNC_EXPORT void GATT_Done(void);
FUNC_EXPORT void GATT_RegCbk(struct GATT_RegCbkStru *in);
FUNC_EXPORT void GATT_SetDatabase(UINT8* buf, UINT16 size);
FUNC_EXPORT void GATT_SetReadCallback(GATT_ReadCbk callback);
FUNC_EXPORT void GATT_SetWriteCallback(GATT_WriteCbk callback);
FUNC_EXPORT void GATT_SetDataValue(UINT16 handle, UINT8* data, UINT16 len);
FUNC_EXPORT void GATT_Notify(struct HCI_AddressStru addr, UINT16 handle, UINT8* data, UINT16 len);
FUNC_EXPORT void GATT_Indicate(struct HCI_AddressStru addr, UINT16 handle, UINT8* data, UINT16 len);
FUNC_EXPORT UINT8* GATT_FindDataByHandle(UINT16 handle);
FUNC_EXPORT UINT16 GATT_FindHandleByUuid(UINT16 uuid);
#endif


