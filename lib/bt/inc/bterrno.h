/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/
 
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
    bterrno.h
Abstract:
	This file includes the definition of global error numbers.
---------------------------------------------------------------------------*/
#ifndef BTERRNO_H
#define BTERRNO_H

enum {
	ER_TYPE_HCI = 0x01,/* 0x00 is reserved */
	ER_TYPE_L2CAP = 0x02,
	ER_TYPE_OBEX = 0x03
};

#define ER_SET(t, v)							((UINT16)((v) ? (((t) << 8) | (v)) : 0))/* only when value != 0, then set type */
#define ER_GETTYPE(s)							((UINT8)((s) >> 8))
#define ER_GETVALUE(s)							((UINT8)((s) & 0xFF))
#define ER_SETCBK(v, t)							(((v) & 0xFFFFFF00) ? (((v) & 0xFFFFFF00) >> 8) : ER_SET((t), (v)))
#define ER_L2CAP_SETCBK(v)						(((v) & 0xFFFFFF00) ? ER_SET((ER_TYPE_HCI), (((v) & 0xFFFFFF00) >> 8)) : ER_SET((ER_TYPE_L2CAP), (v)))

#endif
