/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/
 
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
    global.h
Abstract:
	This file includes the definition of global data and definitions.
---------------------------------------------------------------------------*/
#ifndef GLOBAL_H
#define GLOBAL_H
 
#include "sysdep.h"
#include "debug.h"
#include "btlist.h"
#include "fsm.h"
#include "hdlmgnt.h"
#include "buff.h"
#include "bttask.h"

/* Use a value other than '1' set by "Make Config" for future extension. 
For example, identify features supported by an older version but 
changed or removed in the latest version. */
#ifdef CONFIG_HCI_30
#define CONFIG_HCI_21   4
#define CONFIG_HCI_12   4
#define CONFIG_HCI_11   4
#elif (defined(CONFIG_HCI_21))
#define CONFIG_HCI_12   3
#define CONFIG_HCI_11   3
#elif (defined(CONFIG_HCI_12))
#define CONFIG_HCI_11   2
#endif

#define PROT_HCI	1
#define PROT_L2CAP	2
#define PROT_SDP	3
#define PROT_RFCOMM	4
#define PROT_TCS	5
#define PROT_SPP	6
#define PROT_OBEX   7
#define PROT_BNEP   8
#define PROT_HCRP	9
#define PROT_PAN   10
#define PROT_SWRAP	 11
#define PROT_HID	 12
#define PROT_PHONE	 13	
#define PROT_AVCTP	 14
#define PROT_AVDTP	 15
#define PROT_BIP	 16
#define PROT_GAVDP 	 17
#define PROT_A2DP 	 18
#define PROT_AVCOMMON 	 18
#define PROT_VDP 	 19
#define PROT_GOEP	 20
#define PROT_FTP	 21
#define PROT_OPP	 22
#define PROT_SYNC	 23
#define PROT_GAP	 24
#define PROT_SDAP	 25
#define PROT_HSP	 26
#define PROT_HFP	 26
#define PROT_SDK	 9
#define PROT_DHCP	 10
#define PROT_AVRCP   14

#define MOD_FSM		31
#define MOD_SYSDEP	31
#define MOD_HANDLE	31
#define MOD_LIST	31
#define MOD_BUFF	31
#define MOD_GLOBAL  31


#define PSM_SDP		1
#define PSM_RFCOMM	3
#define PSM_TCS_BIN		5
#define PSM_TCS_BIN_CORDLESS		7
#define PSM_BNEP	0x000F
#define PSM_AVCTP	0x0017
#define PSM_AVDTP	0x0019


#define MAX_DEFINE_PER_PROTOCOL		100


#define PROT_HCI_BASE		((PROT_HCI-1)*MAX_DEFINE_PER_PROTOCOL)	
#define PROT_L2CAP_BASE	((PROT_L2CAP-1)*MAX_DEFINE_PER_PROTOCOL)	
#define PROT_SDP_BASE		((PROT_SDP-1)*MAX_DEFINE_PER_PROTOCOL)	
#define PROT_RFCOMM_BASE	((PROT_RFCOMM-1)*MAX_DEFINE_PER_PROTOCOL)	
#define PROT_TCS_BASE		((PROT_TCS-1)*MAX_DEFINE_PER_PROTOCOL)	
#define PROT_SPP_BASE		((PROT_SPP-1)*MAX_DEFINE_PER_PROTOCOL)
#define PROT_OBEX_BASE		((PROT_OBEX-1)*MAX_DEFINE_PER_PROTOCOL)	
#define PROT_BNEP_BASE		((PROT_BNEP-1)*MAX_DEFINE_PER_PROTOCOL)	
#define PROT_PAN_BASE		((PROT_PAN-1)*MAX_DEFINE_PER_PROTOCOL)	
#define PROT_HCRP_BASE		((PROT_HCRP-1)*MAX_DEFINE_PER_PROTOCOL)	
#define PROT_HID_BASE		((PROT_HID-1)*MAX_DEFINE_PER_PROTOCOL)	
#define PROT_AVCTP_BASE	((PROT_AVCTP-1)*MAX_DEFINE_PER_PROTOCOL)	
#define PROT_AVDTP_BASE	((PROT_AVDTP-1)*MAX_DEFINE_PER_PROTOCOL)	
#define PROT_BIP_BASE	((PROT_BIP-1)*MAX_DEFINE_PER_PROTOCOL)
#define PROT_GAVDP_BASE	((PROT_GAVDP-1)*MAX_DEFINE_PER_PROTOCOL)
#define PROT_A2DP_BASE		((PROT_A2DP-1)*MAX_DEFINE_PER_PROTOCOL)
#define PROT_VDP_BASE		((PROT_VDP-1)*MAX_DEFINE_PER_PROTOCOL)
#define PROT_HFP_BASE		((PROT_HFP-1)*MAX_DEFINE_PER_PROTOCOL)

#define L2CAP_CO_RESERVE	4
#define L2CAP_CLT_RESERVE	6
#define HCI_RESERVE			16

#define FSM_EV_BASE			1
/* 
 * Global variables 
*/
extern SYSTEM_LOCK msg_timer_lock;
extern SYSTEM_LOCK g_localinfo_lock;
extern volatile UCHAR g_finishing;

#ifdef CONFIG_MEM_USER_FUNC
extern FuncUserAlloc* user_alloc;
extern FuncUserDeAlloc* user_dealloc;
extern FuncUserMemAlert* user_alert;
#endif

#ifndef CONFIG_HCI_ZEEVO
#define FREE_ACL_DATA(d) FREE(d)
#else
extern void FreeAclData(UCHAR* d);
#define FREE_ACL_DATA(d)	FreeAclData((UCHAR*)d)
#endif

#ifdef CONFIG_MEMORY_STATIC
#ifdef  CONFIG_MEMORY_LEAK_DETECT
extern int   mem_max_used;
extern int   mem_cur_used;
#endif
extern SYSTEM_LOCK mem_lock;
extern UCHAR mem_map[BLK_IDXNUM];	/* map of allocated block */
extern UCHAR mem_map2[BLK_IDXNUM]; /* flags of allocated block */

#ifndef CONFIG_HCI_ZEEVO
extern DWORD HUGE g_pool[POOL_SIZE/sizeof(DWORD)];		/* memory pool for memory allocation */
extern UCHAR* pool;
#else
extern UCHAR* pool;
#endif

#endif /*CONFIG_MEMORY_STATIC*/

#ifdef CONFIG_MEMORY_LEAK_DETECT
extern SYSTEM_LOCK len_lock;
#endif

#endif

