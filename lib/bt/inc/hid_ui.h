/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 2014-2019 HUNTERSUN Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Module Name:
	hid_ui.h
Abstract:
	This file defines HID ui macros, functions and defainations.
-----------------------------------------------------------------------------*/

#ifndef HID_UIH
#define HID_UIH

#include "global.h"
#include "l2cap_ui.h"

/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*-----------------------------------------------------------------------------*/
#define HID_MAX_DATA_LEN      46

typedef enum {
   HID_ROLE_DEVICE = 0x00,
   HID_ROLE_HOST   = 0x01			
}HID_ROLE_TYPE;

typedef enum {
   HID_REPORT_ID_UNKNOWN   = 0x00,
   HID_REPORT_ID_KEYBOARD  = 0x01,
   HID_REPORT_ID_MOUSE     = 0x02,
   HID_REPORT_ID_RESERVED  = 0x03,			
}HID_REPORT_ID;

typedef enum {/* Events for upper layer */
   HID_EV_CONNECT_COMPLETE     = 0x01,
   HID_EV_DISCONNECT_COMPLETE  = 0x02,
   HID_EV_DATA_IND             = 0x03,
   HID_EV_DATA_CFM             = 0x04,
}HID_EVENT_TYPE;

typedef enum {
   HID_ERROR_SUCCESS     = 0x00,
   HID_ERROR_MAX_LEN           ,
   HID_ERROR_PARAM             ,
   HID_ERROR_NO_INIT           ,
}HID_ERROR_CODE;

typedef enum
{
   HID_REPORT_TYPE_UNKNOWN = 0,
   HID_REPORT_TYPE_INPUT,
   HID_REPORT_TYPE_OUTPUT,
   HID_REPORT_TYPE_FEATURE,  
} HID_REPORT_TYPE;

typedef void (HID_CbkFuc)(UINT16 msg, void *arg);/* callback to upper protocol */


/*-----------------------------------------------------------------------------
Description:
	Input structures.
-----------------------------------------------------------------------------*/
struct HID_AttrStru {
    UINT16 release_num; // Deprecated
    UINT16 parser_version;
    UINT8  sub_class;
    UINT8  country_code;
    UINT16 profile_version;
    UINT16 supervision_timeout; // option
    UINT16 max_latency;  // option
    UINT16 min_timeout;  // option
    //UINT16 language;
    //UINT16 encoding;
    //UINT16 lang_id;
    UINT16 desc_length;
    UINT8 *descriptor;
    UINT16 name_length;
    UINT8 *service_name; // option
    UINT16 name_desc_length;
    UINT8 *name_desc;  // option
    UINT16 pro_length;
    UINT8 *pro_name; // option
};

struct HID_RegCbkStru {
   void *cbk;
};

struct HID_ConnectReqStru {
   UINT8 bd[BD_ADDR_LEN];
};

struct HID_ConnectCompleteStru {
   UINT8    bd_addr[BD_ADDR_LEN];
   UINT8    channel;
   UINT8    result;
   UINT8    side;
   UINT16   mtu;
};

struct HID_DataStru {
   UINT8    report_type;
   UINT8    len;
   UINT8    data[HID_MAX_DATA_LEN];
   UINT8    bd_addr[BD_ADDR_LEN];
};

/*-----------------------------------------------------------------------------
Description:
	Input Function.
-----------------------------------------------------------------------------*/

FUNC_EXPORT void HID_Init(void);
FUNC_EXPORT void HID_Done(void);
FUNC_EXPORT void HID_RegCbk(struct HID_RegCbkStru *in);
FUNC_EXPORT UINT32 HID_RegisterService(UINT8 svc_type, UINT16 features, struct HID_AttrStru* attr);
FUNC_EXPORT void HID_UnregisterService(UINT32 svr_hdl);

FUNC_EXPORT void HID_Connect_Req(struct HID_ConnectReqStru *in);
FUNC_EXPORT void HID_Disconnect_Req(void);

FUNC_EXPORT HID_ERROR_CODE HID_SendDataByControl(struct HID_DataStru* data);
FUNC_EXPORT HID_ERROR_CODE HID_SendDataByInterrupt(struct HID_DataStru* data);
#endif


