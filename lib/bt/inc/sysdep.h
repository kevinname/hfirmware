/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/
 
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
    sysdep.h
Abstract:
	This file includes the definition of system dependent functions.
	Definition for different Operating system:
		CONFIG_OS_WIN32: For Windows 98/2000
		CONFIG_OS_LINUX: For Linux
		CONFIG_OS_WINCE: For Windows CE 2.12
		CONFIG_OS_WINKERNEL: For Windows Kernel module which is developed using VTOOLSD
		CONFIG_OS_REX:   For Rex
		CONFIG_OS_PSOS:	 For PSOS+
		CONFIG_OS_VXWORK:  For Vxwork 5.3 and above
		CONFIG_OS_NUCLEUS:	 For Nucleus
		CONFIG_OS_UC:	 For uC/OS-II
		CONFIG_OS_NONE:	 For no opeating system.

	Definition for other options:
		CONFIG_MEMORY_STATIC: use static memory allocation.
		CONFIG_ALIGN_ONEBYTE: one byte alignment for data structure.
		CONFIG_ALIGN_TWOBYTES: two byte alignment for data structure.
		CONFIG_LITTLE_ENDIAN: Using little endian encoding for integer.
---------------------------------------------------------------------------*/
#ifndef SYSTEMDEP_H
#define SYSTEMDEP_H

#include "autoconf.h"
#include "stdio.h"
#include "string.h"
#include "stdarg.h"

#if !defined __STATIC_INLINE
  #if   defined ( __CC_ARM )
    #define __INLINE         __inline                                   /*!< inline keyword for ARM Compiler       */
    #define __STATIC_INLINE  static __inline
  #elif defined ( __ICCARM__ )
    #define __INLINE         inline                                     /*!< inline keyword for IAR Compiler. Only available in High optimization mode! */
    #define __STATIC_INLINE  static inline
  #elif defined ( __GNUC__ )
    #define __INLINE         inline                                     /*!< inline keyword for GNU Compiler       */
    #define __STATIC_INLINE  static inline
  #endif
#endif

#ifdef CONFIG_OS_UC
#include "ucos_ii.h"
#define SYSTEM_LOCK	OS_EVENT *
//#define HANDLE		OS_EVENT *
#define TIMER_ID	OS_TMR *
#define GetCurrTime()   OSTimeGet()
#endif

#if defined(CONFIG_OS_CH) || defined(CONFIG_OS_CH2)
#include "ch.h"
#if CH_USE_MUTEXES
#define SYSTEM_LOCK	Mutex *
#else
#define SYSTEM_LOCK	Semaphore *
#endif
//#define HANDLE		Semaphore *
#define TIMER_ID	TaskTimer *
#define GetCurrTime()   chTimeNow()
#endif /* CONFIG_OS_CH */

#ifdef CONFIG_OS_CH3
#include "ch.h"
#if CH_CFG_USE_MUTEXES
#define SYSTEM_LOCK	mutex_t *
#else
#define SYSTEM_LOCK	semaphore_t *
#endif
#define TIMER_ID	void *
#define GetCurrTime()   chVTGetSystemTime()
#endif /* CONFIG_OS_CH3 */

#ifdef CONFIG_OS_RTX
#include "cmsis_os.h"
#define SYSTEM_LOCK	osMutexId
#define HANDLE		osSemaphoreId
#define TIMER_ID	osTimerId
#define GetCurrTime()   rt_time_get()
#endif /* CONFIG_OS_RTX */

#include "datatype.h"
#include "bterrno.h"
#ifdef  CONFIG_MEMORY_STATIC
#include "btmem.h"
#endif

typedef void(THREAD_FUNC)(void);
typedef void(THREAD_FUNC2)(void *);

/* System dependent functions */
UINT8 BTOSDepInit(void);
void BTOSDepDone(void);

SYSTEM_LOCK CREATE_CRITICAL(LPCSTR name);
void ENTER_CRITICAL(SYSTEM_LOCK mylock);
void LEAVE_CRITICAL(SYSTEM_LOCK mylock);
void FREE_CRITICAL(SYSTEM_LOCK mylock);
void WAITFOR_SINGLEOBJECT(HANDLE myobj);
void WAITFOR_SINGLEOBJECT_TIME(HANDLE myobj, DWORD milliseconds);
void BTCLOSE_HANDLE(HANDLE myobj);
void SET_EVENT(HANDLE myobj);
HANDLE CREATE_EVENT(LPCSTR name);
void CLOSE_EVENT(HANDLE myev);
void CREATE_THREAD(THREAD_FUNC * p);
void CREATE_THREAD2(THREAD_FUNC2 * p, void* param);
void CREATE_THREADEX(
	UCHAR *name,
	THREAD_FUNC2 * p, 		/* Start address */
	WORD		stksize,	/* Stack size */
	UCHAR*		stk,		/* Stack */
	WORD		pri,		/* Priority */
	void* param				/* Parameter */
);

void SLEEP(DWORD milli_sec);

#ifdef  CONFIG_MEMORY_LEAK_DETECT
#define NEW(A)  DBG_NEW((int)A,__FILE__,__LINE__)
#define FREE(A)  DBG_FREE((void*)A,__FILE__,__LINE__)
void * DBG_NEW (int size, char* filename, int line);
void DBG_FREE (void * buf, char* filename, int line);
SYSTEM_LOCK DBG_CREATE_CRITICAL(char *a, char* filename, int line);
#else
#ifdef CONFIG_MEMORY_STATIC
#define NEW mem_malloc
#define FREE mem_free
void * mem_malloc (int size);
void mem_free(void * buf);
#else
void * NEW(int size);
void FREE(void * buf);
#endif
#endif

#ifdef CONFIG_DEBUG
int DBG_PRINT( const char *format, ...);
#endif


/*  System dependent timer functions */
struct FsmTimer;
struct BuffStru;

int InitTimer(struct FsmTimer *ft);
int AddTimer(struct FsmTimer *ft);
int DelTimer(struct FsmTimer *ft);

/*
 * Codeing and decoding functions
*/
#define STRU_NOTRANS	 (0x80000000)
#define STRU_TRANS_MULTI (0x40000000)

WORD EncodeNew(UCHAR * dest, UCHAR * src, DWORD mask,UCHAR reverse);
WORD DecodeNew(UCHAR * dest, UCHAR * src, DWORD mask,UCHAR reverse);
#define Encode(a,b,c) EncodeNew(a,b,c,0)
#define Decode(a,b,c) DecodeNew(a,b,c,0)
UCHAR StruLen( DWORD mask );

WORD MyEncode( UCHAR * buf, char * fmt, ...);

/*-----------------------------------------------------------------------------
Description:
	CPU Endian with Protocal Endian.
-----------------------------------------------------------------------------*/
#ifndef CONFIG_BIG_ENDIAN/* CPU Endian is Little */

#define DECODE2BYTE_LITTLE(ptr) 	(UINT16)(*(UINT8 *)(ptr) | (*(UINT8 *)((ptr) + 1) << 8))
#define DECODE4BYTE_LITTLE(ptr)\
	(UINT32)(*(UINT8 *)(ptr) | (*(UINT8 *)((ptr) + 1) << 8) \
	| (*(UINT8 *)((ptr) + 2) << 16) | (*(UINT8 *)((ptr) + 3) << 24))
#define ENCODE2BYTE_LITTLE(ptr, data)\
	*(UINT8 *)((ptr) + 1) = (UINT8)((data) >> 8);\
	*(UINT8 *)(ptr) = (UINT8)(data);
#define ENCODE4BYTE_LITTLE(ptr, data)\
	*(UINT8 *)((ptr) + 3) = (UINT8)((data) >> 24);\
	*(UINT8 *)((ptr) + 2) = (UINT8)((data) >> 16);\
	*(UINT8 *)((ptr) + 1) = (UINT8)((data) >> 8);\
	*(UINT8 *)(ptr) = (UINT8)(data);

#define DECODE2BYTE_BIG(ptr) 	(UINT16)(*(UINT8 *)((ptr) + 1) | (*(UINT8 *)(ptr) << 8))
#define DECODE4BYTE_BIG(ptr)\
	(UINT32)(*(UINT8 *)((ptr) + 3) | (*(UINT8 *)((ptr) + 2) << 8) \
	| (*(UINT8 *)((ptr) + 1) << 16) | (*(UINT8 *)(ptr) << 24))
#define ENCODE2BYTE_BIG(ptr, data)\
	*(UINT8 *)(ptr) = (UINT8)((data) >> 8);\
	*(UINT8 *)((ptr) + 1) = (UINT8)(data);
#define ENCODE4BYTE_BIG(ptr, data)\
	*(UINT8 *)(ptr) = (UINT8)((data) >> 24);\
	*(UINT8 *)((ptr) + 1) = (UINT8)((data) >> 16);\
	*(UINT8 *)((ptr) + 2) = (UINT8)((data) >> 8);\
	*(UINT8 *)((ptr) + 3) = (UINT8)(data);

#else/* CPU Endian is Big */

#define DECODE2BYTE_BIG(ptr) 	(UINT16)(*(UINT8 *)(ptr) | (*(UINT8 *)((ptr) + 1) << 8))
#define DECODE4BYTE_BIG(ptr)\
	(UINT32)(*(UINT8 *)(ptr) | (*(UINT8 *)((ptr) + 1) << 8) \
	| (*(UINT8 *)((ptr) + 2) << 16) | (*(UINT8 *)((ptr) + 3) << 24))
#define ENCODE2BYTE_BIG(ptr, data)\
	*(UINT8 *)((ptr) + 1) = (UINT8)((data) >> 8);\
	*(UINT8 *)(ptr) = (UINT8)(data);
#define ENCODE4BYTE_BIG(ptr, data)\
	*(UINT8 *)((ptr) + 3) = (UINT8)((data) >> 24);\
	*(UINT8 *)((ptr) + 2) = (UINT8)((data) >> 16);\
	*(UINT8 *)((ptr) + 1) = (UINT8)((data) >> 8);\
	*(UINT8 *)(ptr) = (UINT8)(data);

#define DECODE2BYTE_LITTLE(ptr) 	(UINT16)(*(UINT8 *)((ptr) + 1) | (*(UINT8 *)(ptr) << 8))
#define DECODE4BYTE_LITTLE(ptr)\
	(UINT32)(*(UINT8 *)((ptr) + 3) | (*(UINT8 *)((ptr) + 2) << 8) \
	| (*(UINT8 *)((ptr) + 1) << 16) | (*(UINT8 *)(ptr) << 24))
#define ENCODE2BYTE_LITTLE(ptr, data)\
	*(UINT8 *)(ptr) = (UINT8)((data) >> 8);\
	*(UINT8 *)((ptr) + 1) = (UINT8)(data);
#define ENCODE4BYTE_LITTLE(ptr, data)\
	*(UINT8 *)(ptr) = (UINT8)((data) >> 24);\
	*(UINT8 *)((ptr) + 1) = (UINT8)((data) >> 16);\
	*(UINT8 *)((ptr) + 2) = (UINT8)((data) >> 8);\
	*(UINT8 *)((ptr) + 3) = (UINT8)(data);

#endif

/*
 * Unicode and ASCII transaction
*/
/* using CP_ACP code page */
WORD Get_ACP(void);
void Set_ACP(WORD acp);
int WideToByte(char* lpw,char* lpa,int len);
int ByteToWide(char* lpa,char* lpw,int len);
/* using CP_UTF8 code page */
int WideToUTF8(char* lpw,char* lpa,int len);
int UTF8ToWide(char* lpa,char* lpw,int len);
/* using CP_UTF7 cod page */
int WideToUTF7(char* lpw,char* lpa,int len);
int UTF7ToWide(char* lpa,char* lpw,int len);
int ShortenString(char *lstr, int llen, char *sstr, int slen);

/* String Routine. */
UINT EnhancedAtoI(const char *string);
#if !defined(__GNUC__)
char *strcasestr(const char *str, const char *charset);
#endif
void ATOU64(const char *src_str, int src_len, UINT32 *phigh, UINT32 *plow);
int U64TOA(UINT32 high, UINT32 low, char *dest_str, int dest_size);

#ifndef FUNC_EXPORT 
#define FUNC_EXPORT 
#endif

#ifdef CONFIG_CHECK_STACK
#ifdef CONFIG_OS_WIN32
void STK_StartChk(UCHAR f,const char *func);
WORD STK_EnterFunc(const char *func);
void STK_LeaveFunc(const char *func);
void STK_StopChk(UCHAR f);
#else
#define STK_StartChk(f,d)
#define STK_EnterFunc(f)
#define STK_LeaveFunc(f)
#define STK_StopChk(f)
#endif
#else
#define STK_StartChk(f,d)
#define STK_EnterFunc(f)
#define STK_LeaveFunc(f)
#define STK_StopChk(f)
#endif

#ifdef  CONFIG_MEMORY_LEAK_DETECT
#ifdef CONFIG_OS_WIN32
void SetMemLeakFlag(void);
void MapFilenameToProt(char *fn,char *prot);
void RegProtForStat(char *affix);
void RegStackProtForStat(void);
void EndMemDetect();
#else
#define SetMemLeakFlag()
#define MapFilenameToProt(fn,prot) 
#define RegProtForStat(af)
#define RegStackProtForStat()
#define EndMemDetect()
#endif
#else
#define SetMemLeakFlag()
#define MapFilenameToProt(fn,prot) 
#define RegProtForStat(af)
#define RegStackProtForStat()
#define EndMemDetect()
#endif

#if defined(__GNUC__) && defined(CONFIG_DEBUG)
#define assert(condition) \
  do { if (!(condition)) { DBG_PRINT("assert failure at %s:%d\r\n", __FILE__, __LINE__); while (1); } } while(0)
#else
#define assert(condition)
#endif

#endif 

