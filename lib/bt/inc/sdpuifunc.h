/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
    sdpuifunc.h
Abstract:
	This files exports SDP API functions.
---------------------------------------------------------------------------*/
#ifndef _IVTSDP_UI_FUNC_H_
#define _IVTSDP_UI_FUNC_H_

#include "sdpuimacro.h"

typedef void (SDPUICfm)(struct SDP_UICfmParamStru *cfm);

UCHAR SDPInit(void);
void SDPDone(void);

FUNC_EXPORT WORD sdRegisterService(struct ServiceRegStru *service);
FUNC_EXPORT WORD sdUnregisterService(DWORD handle);
FUNC_EXPORT WORD sdAllocateSvcHdl(DWORD *handle);

FUNC_EXPORT WORD sdOpenSearchReq(struct SDP_UIReqParamStru *req);
FUNC_EXPORT WORD sdCloseSearchReq(struct SDP_UIReqParamStru *req);
FUNC_EXPORT WORD sdxSearchReq(struct SDP_UIReqParamStru *req);

FUNC_EXPORT void  sdRegisterCB(SDPUICfm *cb);

FUNC_EXPORT UCHAR* sdMallocMemory(int size);
FUNC_EXPORT void  sdFreeMemory(UCHAR *buf);


#endif

