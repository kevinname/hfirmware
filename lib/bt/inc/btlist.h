/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/
 
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Module Name:
    btlist.h
Abstract:
	This file includes the definition for thread safe list.
---------------------------------------------------------------------------*/

#ifndef BT_LIST_H
#define BT_LIST_H

struct BtList {
	UCHAR *			head;		/* head of the list*/
	UCHAR *			tail;		/* tail of the list*/
	WORD			number;		/* number of members in the list*/
	WORD			len;		/* member data length*/
	SYSTEM_LOCK		lock;		/* lock for the list*/
};
struct NewBtList {
	UCHAR *			head;		/* head of the list*/
	UCHAR *			tail;		/* tail of the list*/
	WORD			number;		/* number of members in the list*/
	WORD			len;		/* member data length*/
};

/*internal functions*/
#ifdef CONFIG_MEMORY_LEAK_DETECT
struct BtList * DBG_BtListNew(WORD len, UCHAR type,char* filename, int line);
UCHAR *	DBG_BtListMemberNew(WORD len, char* filename, int line);
#define BtListNew2(A,T) DBG_BtListNew(A,T, __FILE__, __LINE__)
#else
struct BtList * BtListNew2(WORD len,UCHAR type);
#endif
UCHAR BtListFree2(struct BtList * list,UCHAR type);
void AddHead2(struct BtList * list, UCHAR * data,UCHAR type);
void AddTail2(struct BtList * list, UCHAR * data,UCHAR type);
void InsertAt2(struct BtList * list, UCHAR * pos, UCHAR * data ,UCHAR type);
UCHAR * Find2(struct BtList * list,UCHAR * data, UCHAR len, UCHAR type);
UCHAR * GetHead2(struct BtList * list,UCHAR type);
UCHAR * GetTail2(struct BtList * list,UCHAR type);
UCHAR * RemoveHead2(struct BtList * list,UCHAR type);
UCHAR * RemoveTail2(struct BtList * list,UCHAR type);
UCHAR * RemoveAt2(struct BtList * list, UCHAR * pos,UCHAR type);
void RemoveAll2(struct BtList * list ,UCHAR type);

/*Export functions */
#ifdef CONFIG_MEMORY_LEAK_DETECT
#define BtListMemberNew(A) DBG_BtListMemberNew(A, __FILE__, __LINE__)
#else
UCHAR *	BtListMemberNew(WORD len);
#endif

#define BtListMemberFree FREE
#define Count(list) list->number
#define NEXT(s) (*((ULONG *)s+((list->len+sizeof(ULONG *)-1)>>PTR_BITS)))
				/*				~~~~~~~~~~~~~~~~~~~~~~~~~~~~ end of data, align to 4 bytes*/
#define PREV(s) (*((ULONG *)s+((list->len+sizeof(UCHAR *)+sizeof(ULONG *)-1)>>PTR_BITS)))
#define NEXT_MEM(l,m) (*((ULONG *)m+((l->len+sizeof(ULONG *)-1)>>PTR_BITS)))
#define PREV_MEM(l,m) (*((ULONG *)m+((l->len+sizeof(UCHAR *)+sizeof(ULONG *)-1)>>PTR_BITS)))

UCHAR BtInlist(struct BtList * list, UCHAR * pos);

#define OLD_LIST		0
#define NEW_LIST		1

/*for old list, (with lock)*/
#define BtListNew(l)	BtListNew2(l,OLD_LIST)
#define BtListFree(l)	BtListFree2(l,OLD_LIST)
#define AddHead(l,d)	AddHead2(l,d,OLD_LIST)
#define AddTail(l,d)	AddTail2(l,d,OLD_LIST)
#define InsertAt(l,p,d)	InsertAt2(l,p,d,OLD_LIST)
#define Find(l,d,le)	Find2(l,d,le,OLD_LIST)
#define GetHead(l)		GetHead2(l,OLD_LIST)
#define GetTail(l)		GetTail2(l,OLD_LIST)
#define RemoveHead(l)	RemoveHead2(l,OLD_LIST)
#define RemoveTail(l)	RemoveTail2(l,OLD_LIST)
#define RemoveAt(l,d)	RemoveAt2(l,d,OLD_LIST)
#define RemoveAll(l)	RemoveAll2(l,OLD_LIST)

/*for new list, (no lock)*/
#define BtNewListNew(l)			BtListNew2(l,NEW_LIST)
#define BtNewListFree(l)		BtListFree2(l,NEW_LIST)
#define NewListAddHead(l,d)		AddHead2(l,d,NEW_LIST)
#define NewListAddTail(l,d)		AddTail2(l,d,NEW_LIST)
#define NewListInsertAt(l,p,d)	InsertAt2(l,p,d,NEW_LIST)
#define NewListFind(l,d,le)		Find2(l,d,le,NEW_LIST)
#define NewListGetHead(l)		GetHead2(l,NEW_LIST)
#define NewListGetTail(l)		GetTail2(l,NEW_LIST)
#define NewListRemoveHead(l)	RemoveHead2(l,NEW_LIST)
#define NewListRemoveTail(l)	RemoveTail2(l,NEW_LIST)
#define NewListRemoveAt(l,d)	RemoveAt2(l,d,NEW_LIST)
#define NewListRemoveAll(l)		RemoveAll2(l,NEW_LIST)

#define NewRemoveHead(l)		NewListRemoveHead(l)
#define NewRemoveTail(l)		NewListRemoveTail(l)
#define NewRemoveAt(l,d)		NewListRemoveAt(l,d)
#define NewAddHead(l,d)			NewListAddHead(l,d)

/*---------------------------------------------------------------------------
Module Name:
	Pre-Pointer, Variable Preload List management
Abstract:
	This module defines the Bi-Direction and Single-Direction List manage functions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* 								Pre 2 PTR * List 										     */
/*---------------------------------------------------------------------------*/
struct ListStru {/* New List Structure by hiw */
	void *head;
	void *tail;
};

struct ListPriStru {/* 3 Priorities List, support add[Pri]/remove[Head] only */
	void *head;
	void *p[3];/* 0 is HIGH, 1 is Normal, 2 is Low(Tail) */
};

enum {/* For struct ListPriStru p[Pri] */
	LIST_PRI_LOW = 0,
	LIST_PRI_NORMAL = 1,
	LIST_PRI_HIGH = 2
};

/*---------------------------------------------------------------------------
Description:
	L2CAP/HCI QoS Data send list.
Pri Format:
	31 	30 		29 			28 					14 					0
    Super Signal 	Real/Low  	-Reserved & <-Timer Score Init->
---------------------------------------------------------------------------*/
struct ListQosCIDNodeStru {/* SiDirection Node */
	struct ListStru list;/* SiDirection Node */
	UINT32 pri;/* High 3 bits are type, low 30 bits are timer count */
	UINT32 cid_hdl;
	UINT16 signalnum;
};

enum {/* flag in List_QoSReRanking */
	LIST_QOSFLAG_ADD,
	LIST_QOSFLAG_GET,
	LIST_QOSFLAG_PRISUPER = 0xFF
};

enum {/* Pri type bits(High 2bits) in struct ListQosCIDNodeStru */
	LIST_QOSPRI_SUPER = 		(1 << 31),/* Highest bit */
	LIST_QOSPRI_SIGNAL = 		(1 << 30),/* High 2nd bit */
	LIST_QOSPRI_REALTIME = 		(1 << 29),
	LIST_QOSPRI_LOWDATA = 		(0 << 29),
	LIST_QOSPRI_TYPE = 			(LIST_QOSPRI_SUPER | LIST_QOSPRI_SIGNAL | LIST_QOSPRI_REALTIME),
	LIST_QOSPRI_INITBIT = 		(1 << 14)
};

/* Bidirection List Node */
#define LGET(n)			((void **)(n) + (1 * 2))/* From memory start ptr go to userdata */
#define LSTART(n)		((void **)(n) - (1 * 2))/* From userdata get back to memory start ptr */
#define LNEXT(n)		(*((void **)(n) - (1 * 2)))
#define LPREV(n)		(*((void **)(n) - 1))
#define LFREE(n)		(FREE(LSTART(n)))/* Input ListNode userdata Ptr, Free it */

/* Single Direction List Node */
#define SGET(n)			((void **)(n) + 1)/* From memory start ptr go to userdata */
#define SSTART(n)		((void **)(n) - 1)/* From userdata get back to memory start ptr */
#define SNEXT(n)		(*((void **)(n) - 1))
#define SFREE(n)		(FREE(SSTART(n)))/* Input ListNode userdata Ptr, Free it */

#define LIST_STRUOFF(stru_, mem_)				((UINT32)(&(((stru_)(NULL))->mem_)))
#define LIST_STRUMEMLEN(stru_, mem_)			sizeof((((stru_)(NULL))->mem_))
void *List_Find_(struct ListStru *list, UINT32 off, UINT32 len, UINT8 *val);/* Shall not be used by UI */

#define VARLEN(stru_, mem_, len_)				(((UINT32)(&(((stru_)(NULL))->mem_))) + (len_))
#define NEWVARLEN(stru_, mem_, len_)			NEW(VARLEN(stru_, mem_, len_))

/*---------------------------------------------------------------------------
Example:
	LIST_FIND(&L2CAP_CID_LIST, struct L2CAP_CIDListStru *, cid, val);
---------------------------------------------------------------------------*/
#define LIST_FIND(list_, stru_, mem_, val_)				List_Find_(list_, LIST_STRUOFF(stru_, mem_), LIST_STRUMEMLEN(stru_, mem_), (UINT8 *)(&(val_)))/* This is UI */
#define LIST_FINDLEN(list_, stru_, mem_, len_, pval_)	List_Find_(list_, LIST_STRUOFF(stru_, mem_), (len_), (UINT8 *)(pval_))
#define LIST_FINDLENOFF0(list_, len_, pval_)			List_Find_(list_, 0, (len_), (UINT8 *)(pval_))

UINT32 List_Count(struct ListStru *list);
void *List_NodeExist(struct ListStru *list, void *p);
#ifdef CONFIG_MEMORY_LEAK_DETECT
void *DBG_List_NodeNew(UINT32 len, char* filename, int line);
#define List_NodeNew(l) DBG_List_NodeNew(l, __FILE__, __LINE__)
#else
void *List_NodeNew(UINT32 len);
#endif
void List_AddTail(struct ListStru *list, void *node);
void List_AddHead(struct ListStru *list, void *node);
void List_InsertAt(struct ListStru *list, void *pos, void *node);
void *List_RemoveHead(struct ListStru *list);
void *List_RemoveTail(struct ListStru *list);
void List_RemoveAt(struct ListStru *list, void *node);
void List_RemoveAll(struct ListStru *list);

struct BuffStru *List_QoSGet(struct ListStru *list);
void List_QoSGetCfm(struct ListStru *list, struct BuffStru *buf, UINT8 pri);
UINT32 List_QoSAdd(struct ListStru *list, struct BuffStru *buf, UINT32 cid_hdl);
UINT32 List_QoSRemoveHdl(struct ListStru *list, UINT16 hdl);
void List_QoSRemoveAll(struct ListStru *list);

UINT32 List_SCount(struct ListStru *list);
void *List_SNodeNew(UINT32 len);
void List_SAddTail(struct ListStru *list, void *node);
void List_SAddHead(struct ListStru *list, void *node);
void *List_SInsertAt(struct ListStru *list, void *pos, void *node, UINT32 len);
void *List_SRemoveHead(struct ListStru *list);
void List_SRemoveAll(struct ListStru *list);

#endif

