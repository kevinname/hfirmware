/*---------------------------------------------------------------------------
*
* Copyright (c) 1999-2014 IVT Corporation
*
* All rights reserved.
*
---------------------------------------------------------------------------*/
 
/*---------------------------------------------------------------------------
Module Name:
	fsm.h
Abstract:
	This file includes the definition of finite state machine functions.
---------------------------------------------------------------------------*/
#ifndef FSM_H
#define FSM_H

#define MAX_INST_NAME_LEN		7
#define MAX_TIMER_NAME_LEN		7

/* Finite state machine instance */
struct FsmInst {				/* Fsm instance */
	void *user_data;			/* User specific data for the fsm */
	struct Fsm *fsm;			/* The Fsm entity for this Fsm instance. */
	UINT8 state;				/* The Current state for the fsm. */
#ifdef CONFIG_DEBUG	
	char fsm_inst_name[MAX_INST_NAME_LEN];/* The name of the fsm instance */
#endif
};

typedef void (FsmFunc) (struct FsmInst *, UINT8, void *);

/* Entry to fill the jump matrix for FSM */
struct FsmNode {		/* Fsm node*/
	UINT8 state;
	UINT8 event;		/* state and event */
	FsmFunc *routine;
};

/* Finite state machine */
struct Fsm {					/* Fsm entity */
	UINT8 *st;					/* St offest table by hiw */
	UINT8 *it;					/* Ev->Index Table by hiw */
	FsmFunc **func_array;		/* Function array by hiw */
	FsmFunc *default_func;		/* The entity of the default function */
	UINT8 ref_count;			/* Number of Fsm instances that use this Fsm. */
};

struct Event {					/* Event for the FSM */
	struct FsmInst *fi; 		/* Correspond Fsm Instance */
	void *arg;					/* Event Argument. */
	FsmFunc *func;				/* "no status need to be checked" eg: If this is a control fsm func */
	FsmFunc *freefunc;			/* Use to free arg */
	UINT8 event;				/* Event ID. */
};

enum {/* mask value of s_block_ev */
	SCH_READY = 0x01,			/* quit block following event/timer */
	SCH_IDLE = 0x02				/* SCH is idle now */
};

enum {/*  */
	BT_PRI_HIGHEST = 			2,//LIST_PRI_HIGH
	BT_PRI_HIGH = 				1,//LIST_PRI_NORMAL
	BT_PRI_NORMAL = 			0//LIST_PRI_LOW
};

/* FSM functions */
#define FsmChangeState(fi_, newstate_) 					((fi_)->state = (newstate_))

struct Fsm *FsmNewFunc(const struct FsmNode *fnlist, FsmFunc *def_func, UINT8 fncount, UINT8 state_count);/* Internal */
#define FsmNew(list, fn, st, ev, func)	FsmNewFunc(list, func, fn, st)
struct Fsm *FsmDup(struct Fsm *fsm);
struct FsmInst *FsmInstNew_(void *user_data);/* Internal */

#ifdef CONFIG_DEBUG
struct FsmInst *FsmInstNewDBG(struct FsmInst *fi, const char *name);
#define FsmInstNew(ud, name) FsmInstNewDBG(FsmInstNew_(ud), name)
#else
#define FsmInstNew(ud, name) FsmInstNew_(ud)
#endif

void FsmEventFunc_Internal(struct FsmInst *fi, FsmFunc *func, FsmFunc *freefunc, void *arg, UINT32 pri_event);
void FsmEventFunc_External(struct FsmInst *fi, FsmFunc *func, FsmFunc *freefunc, void *arg, UINT32 pri_event);

/* FSM functions UI interfaces */
/* Internal of SCH thread */
void FSMFree(struct FsmInst *fi, UINT8 event, void *arg);
void FsmInstFree(struct FsmInst *fsminst);
void FsmDirectEvent(struct FsmInst *fi, UINT8 event, void *arg);

#if 1/* For New */
#define FsmEventKx(fi_, ev_, func_, arg_) 				FsmEventFunc_Internal(fi_, func_, func_, arg_, (ev_))
#define FsmEventFx(fi_, func_, arg_) 					FsmEventFunc_Internal(fi_, func_, func_, arg_, 1)/* Event not care */
#define FsmEventTx(fi_, func_, arg_) 					FsmEventFunc_Internal(fi_, func_, FSMFree, arg_, 1)/* Event not care */
#define FsmEventCx(fi_, ev_, func_, arg_) 				FsmEventFunc_Internal(fi_, func_, FSMFree, arg_, (ev_))
#define FsmEvent(fi_, ev_, arg_) 						FsmEventFunc_Internal(fi_, NULL, FSMFree, arg_, (ev_))
#else/* For old applications, called outside of SCH */
#define FsmEventKx(fi_, ev_, func_, arg_) 				FsmEventFunc_External(fi_, func_, func_, arg_, (ev_))
#define FsmEventFx(fi_, func_, arg_) 					FsmEventFunc_External(fi_, func_, func_, arg_, 1)/* Event not care */
#define FsmEventTx(fi_, func_, arg_) 					FsmEventFunc_External(fi_, func_, FSMFree, arg_, 1)/* Event not care */
#define FsmEventCx(fi_, ev_, func_, arg_) 				FsmEventFunc_External(fi_, func_, FSMFree, arg_, (ev_))
#define FsmEvent(fi_, ev_, arg_) 						FsmEventFunc_External(fi_, NULL, FSMFree, arg_, (ev_))
#endif

/* External of SCH thread */
#define DoneLoop() 										FsmEventFunc_External(NULL, NULL, NULL, NULL, 0)/* Schloop is going done */
#define FsmEvent_ExternalKx(fi_, ev_, func_, arg_) 		FsmEventFunc_External(fi_, func_, func_, arg_, (ev_))
#define FsmEvent_ExternalFx(fi_, func_, arg_) 			FsmEventFunc_External(fi_, func_, func_, arg_, 1)/* Event not care */
#define FsmEvent_ExternalTx(fi_, func_, arg_)			FsmEventFunc_External(fi_, func_, FSMFree, arg_, 1)
#define FsmEvent_ExternalCx(fi_, ev_, func_, arg_) 		FsmEventFunc_External(fi_, func_, FSMFree, arg_, (ev_))
#define FsmEvent_External(fi_, ev_, arg_) 				FsmEventFunc_External(fi_, NULL, FSMFree, arg_, (ev_))

/* Timer structure */
struct FsmTimer {			/* Timer in the fsm instance */
	UINT32 u_id; 			/* User defined id to index the timer */
	UINT32 millisec; 		/* Timer value */
	struct FsmInst *fi; 	/* Correspond Fsm Instance */
	FsmFunc *func;			/* Default Timer expire function */
	void *arg;				/* Argument for the event. */
	TIMER_ID timer_id;		/* System timer id */
	UINT8 event;			/* Event when time out. */
#ifdef CONFIG_DEBUG	
	char name[MAX_TIMER_NAME_LEN];/* Name for the timer */
#endif
}; 

/* Timer functions */
typedef void (UserTimerFunc)(UINT32 param);
struct FsmTimer *FSM_RestartTimer(struct FsmTimer *ft, struct FsmInst *fi, UINT32 millisec, UINT8 event, FsmFunc *func, void *arg, void *name/* DBG */, UINT32 u_id);
struct FsmTimer *FsmAddTimerFunc(struct FsmInst *fi, UINT32 millisec, UINT8 event, FsmFunc *func, void *arg, const char *name, UINT32 u_id);
void FsmExpireTimer(struct FsmTimer *ft);
UINT8 *FsmDelTimer(struct FsmInst *fi, UINT32 u_id);
UINT8 *FsmDelTimer2(struct FsmTimer *ft);
void FSM_DelTimer(struct FsmTimer **ft);
void *FsmTimerActive(struct FsmInst *fi, UINT32 u_id);
FsmFunc *GetFsmFunc(struct FsmInst *fi, UINT8 ev);

/* Timer UI interface functions */
#define FsmAddTimer(fi_, millisec_, ev_, arg_, name_, u_id_) 			FsmAddTimerFunc(fi_, millisec_, ev_, NULL, arg_, name_, u_id_)
#define FsmAddTimerCx(fi_, millisec_, ev_, func_, arg_, name_, u_id_) 	FsmAddTimerFunc(fi_, millisec_, ev_, func_, arg_, name_, u_id_)

void InitSchedule(void);
void ScheduleLoop(void);

#endif/* FSM_H */

