/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/
 
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
    buff.h
Abstract:
	This file includes the definition of handle management functions.
---------------------------------------------------------------------------*/
#ifndef HDLMGNT_H
#define HDLMGNT_H

struct HandleStruct {
	HANDLE		handle_id;		/* Handle ID of the handle */
	UCHAR *		field_id;		/* field attached to the handle */
	UCHAR *		param;			/* Parameter attached to the handle */
	WORD		class_id;		/* Class of the handle */
	UCHAR		field_len;		/* Length of field_id */
	CHAR 		active;			/* status of the handle */
} ;

#define HDL_TIMEOUT	2

void InitHandle(void);
void DoneHandle(void);

#ifdef CONFIG_MEMORY_LEAK_DETECT
struct HandleStruct * DBG_CreateHandle(WORD class_id, UCHAR field_len, UCHAR* field_id, UCHAR* param, char* filename, int line);
#define CreateHandle(A,B,C,D) DBG_CreateHandle(A,B,C,D, __FILE__, __LINE__)
#else
struct HandleStruct * CreateHandle(WORD class_id, UCHAR field_len, UCHAR* field_id, UCHAR* param);
#endif

struct HandleStruct * FindHandle(WORD class_id, UCHAR field_len, unsigned char * field_id);
UCHAR FindHdl(struct HandleStruct * hdl);
char ActivateHandle(WORD class_id, UCHAR field_len, UCHAR* field_id, UCHAR par_len, UCHAR* param);
char FastActHandle(struct HandleStruct * hdl, UCHAR f, UCHAR par_len, UCHAR *param);
char ActHandle3(struct HandleStruct * hdl, UCHAR f);
UCHAR WaitHandle(struct HandleStruct * hdl);
UCHAR  DeleteHandle(struct HandleStruct * hdl);
UCHAR WaitHandle2(struct HandleStruct * hdl, DWORD msecond);
UCHAR WaitHandleEx(struct HandleStruct *hdl);
void ToExpire(DWORD arg);

#define ActHandle2(h)  ActHandle3(h, 0);

extern struct BtList * handle_list;

#endif
