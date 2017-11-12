/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/
 
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
    btinit.h
Abstract:

---------------------------------------------------------------------------*/
#ifndef BTINIT_H
#define BTINIT_H

#include "datatype.h"

struct AppEvConn {
	WORD clsid;
	UCHAR bd[6];	/* Remote bd address */
	UCHAR* sub_hdl; /*this is for PAN, each PAN client connection's handle*/
};

struct AppEvConnEx {
	WORD    clsid;
	UCHAR   bd[6];	                    /* Remote bd address */
	UCHAR   *sub_hdl;                   /* this is for PAN, each PAN client connection's handle */
	DWORD   total_bandwidth;			/* Total radio bandwidth available. Kbits */
	DWORD   guaranteed_bandwidth;	    /* Max bandwidth assinged to this connection. Kbits */
	UINT8   radio_type;					/* Radio type e.g. BR/EDR, 802.11. It can be One of BT_RADIO_TYPE_BREDR, BT_RADIO_TYPE_80211, etc. */	
};

struct AppEvQoSIndParam {
	UCHAR *hdl;                         /* Connection handle */
	DWORD  total_bandwidth;				/* Total radio bandwidth available. Kbits */
	DWORD  guaranteed_bandwidth;	    /* Max bandwidth assinged to this connection. Kbits */
	UCHAR  radio_type;					/* Radio type e.g. BR/EDR, 802.11. It can be One of BT_RADIO_TYPE_BREDR, BT_RADIO_TYPE_80211, etc. */	
};

/* Possible radio type - AMP controller type, 0x03 - 0xEF Reserved, 0xF0 - 0xFF Vendor-Specific */
#define BT_RADIO_TYPE_BREDR         0x00	/* Bluetooth, HCI_DEVICE_TYPE_BREDR */
#define BT_RADIO_TYPE_80211         0x01	/* WIFI, AMP_CONTROLLER_TYPE_80211  */
#define BT_RADIO_TYPE_ECMA368       0x02	/* UWB, AMP_CONTROLLER_TYPE_ECMA368 */
#define BT_RADIO_TYPE_FAKEPAL_UDP   0xEF    /* UDP Fake PAL, AMP_CONTROLLER_TYPE_FAKEPAL_UDP */

#define IS_SERVEREV(a)	(a&0x8000)
#define APP_EVENT(a)	(a&0x7fff)
#define SERVER_EV(a)	(a|0x8000)
#define BTAPP_EV_CONN		0x01	/* Connection is setup. Param is a pointer to AppEvConn structure*/
#define BTAPP_EV_DISC		0x02	/* Connection is disconected. Param is not used */
#define BTAPP_EV_START		0x03	/* Start server.  Param is not used */
#define BTAPP_EV_STOP		0x04	/* Stop server. Param is not used */
#define BTAPP_EV_PREV_DISC	0x05
#define BTAPP_EV_SEND		0x05
#define BTAPP_EV_RECEIVE	0x06
#define BTAPP_EV_CONN_CFM	0x07	/* Confirmation to a connection request. Param is a pointer to AppEvConn structure */
#define BTAPP_EV_DISC_CFM   0x08	/* Confirmation to a disconnection request. Param is not used */
#define BTAPP_EV_EX_CONN	    0x09	/* Connection is setup. Param is a pointer to AppEvConnEx structure */
#define BTAPP_EV_EX_CONN_CFM	0x0A    /* Confirmation to a connection request. Param is a pointer to AppEvConnEx structure */
#define BTAPP_EV_QOS_CHANGED    0x0B    /* Qos setting for the connection is changed. Param is a pointer to AppEvQoSIndParam structure */

#define BTAPP_MAX_EVENT			10000

typedef UCHAR (StatusInd)(WORD event, UCHAR * hdl, UCHAR * param);

void BtInitProfile(void);
void BtDoneProfile(void);
void BtInitEnv(void);
void BtInitStack(void);
void BtDoneStack(void);

void BtInit(void);
void BtDone(void);

#ifdef CONFIG_GAP
#include "gap_ui.h"
#endif

#ifdef CONFIG_SDAP
#include "sdap_ui.h"
#endif

#ifdef CONFIG_AVCTP
#include "avctp_ui.h"
#endif

#ifdef CONFIG_AVRCP
#include "avrcp_ui.h"
#endif

#ifdef CONFIG_A2DP
#include "a2dp_ui.h"
#endif

#ifdef CONFIG_HFP
#include "hfp_ui.h"
#endif

#ifdef CONFIG_SPP
#include "spp_ui.h"
#endif

#ifdef CONFIG_HID
#include "hid_ui.h"
#endif

#ifdef CONFIG_GATT
#include "gatt_ui.h"
#endif
#endif
