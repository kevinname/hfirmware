/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

#ifndef BTTASK_H
#define BTTASK_H

typedef void (FuncIndMsg)(UCHAR msg_id,UCHAR* param);

#define RX_RESERVE_DONE		0
#define RX_RESERVE_INIT		1
#define RX_USER_MSG			10

struct IndMsgStru {
	UCHAR* param;
	FuncIndMsg* func;
	UCHAR msg_id;
};

void InitRxLoop(void);
void SendIndMsg(UCHAR* param,FuncIndMsg* func,UCHAR msg_id);
#define DoneRxLoop()	SendIndMsg(NULL,NULL,RX_RESERVE_DONE)

void RxLoop(void);

#endif
