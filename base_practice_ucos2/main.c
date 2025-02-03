/* TimeBomb/Button with uC/OS-II RTOS */
#include "uc_ao.h"  /* uC/OS-II API, port and compile-time configuration */
#include "qassert.h"  /* embedded-system-friendly assertions */
#include "bsp.h"      /* Board Support Package */
#include <stdbool.h>

Q_DEFINE_THIS_MODULE("main") /* this module name for Q_ASSERT() */

/* The TimeBomb thread =========================================================*/
enum { INITIAL_BLINK_TIME = (OS_TICKS_PER_SEC / 4) };

/* The TimeBomb active object ================================================== */
typedef struct {
	Active super; /* inherit active base class */
	/* private data for the AO */
	TimeEvent te;
	/* note enumerated constants are automatically assigned numerical values based on positioning in the enum struct */
	enum {
		WAITFORBUTTON_STATE, /* 0 */
		BLINK_STATE,         /* 1 */
		PAUSE_STATE,         /* 2 */
		BOOM_STATE,          /* 3 */
		/* ... */
		MAX_STATE            /* 4 */
	} state; 
	uint32_t blink_time;
	uint8_t blink_ctr;
} TimeBomb;

typedef enum { TRAN_STATUS, HANDLED_STATUS, IGNORED_STATUS, INIT_STATUS } Status;

/* TimeBomb waitforbutton state ================================================== */
static Status TimeBomb_init(TimeBomb * const me, Event const * const e) {
	BSP_ledGreenOff();
	me->state = WAITFORBUTTON_STATE;
	me->blink_time = INITIAL_BLINK_TIME * 3U;
	return INIT_STATUS;
}
static Status TimeBomb_waitforbutton_ENTRY(TimeBomb * const me, Event const * const e) {
	BSP_ledGreenOff();
	return HANDLED_STATUS;
}

static Status TimeBomb_waitforbutton_PRESSED(TimeBomb * const me, Event const * const e) {
	BSP_ledGreenOn();
	/* start the countdown */
	me->blink_ctr = 3U;
	me->state = BLINK_STATE;
	return TRAN_STATUS;
}

/* TimeBomb blink state ================================================== */
static Status TimeBomb_blink_ENTRY(TimeBomb * const me, Event const * const e) {
	BSP_ledGreenOn();
	TimeEvent_arm(&me->te, me->blink_time, 0U);
	return HANDLED_STATUS;
}
	
static Status TimeBomb_blink_EXIT(TimeBomb * const me, Event const * const e) {
	BSP_ledGreenOff();
	return HANDLED_STATUS;
}

static Status TimeBomb_blink_TIMEOUT(TimeBomb * const me, Event const * const e) {
	me->state = PAUSE_STATE;
	return TRAN_STATUS;
}

/* TimeBomb pause state ================================================== */
static Status TimeBomb_pause_ENTRY(TimeBomb * const me, Event const * const e) {
	BSP_ledGreenOff();
	TimeEvent_arm(&me->te, me->blink_time, 0U);
	return HANDLED_STATUS;
}	

static Status TimeBomb_pause_TIMEOUT(TimeBomb * const me, Event const * const e) {
	me->blink_ctr--;
	if (me->blink_ctr > 0U) {
		me->state = BLINK_STATE;
	}
	else {
		me->state = BOOM_STATE;
	}
	return TRAN_STATUS;
}
	
/* TimeBomb boom state ================================================== */
static Status TimeBomb_boom_ENTRY(TimeBomb * const me, Event const * const e) {
	BSP_ledGreenOn();
	return HANDLED_STATUS;
}
	
/* TimeBomb ignore base case ================================================== */
static Status TimeBomb_ignore(TimeBomb * const me, Event const * const e) {
	/* pass */
	return IGNORED_STATUS;
}

typedef Status (*TimeBombAction)(TimeBomb * const me, Event const * const e);

TimeBombAction const TimeBomb_table[MAX_STATE][MAX_SIG] = {
                 /* INIT               | ENTRY                        | EXIT                | BUTTON_PRESSED                 | BUTTON_RELEASED | TIMEOUT                | */
/* waitforbutton */ {&TimeBomb_init,   &TimeBomb_waitforbutton_ENTRY, &TimeBomb_ignore,     &TimeBomb_waitforbutton_PRESSED, &TimeBomb_ignore, &TimeBomb_ignore},
/* blink         */ {&TimeBomb_ignore, &TimeBomb_blink_ENTRY,         &TimeBomb_blink_EXIT, &TimeBomb_ignore,                &TimeBomb_ignore, &TimeBomb_blink_TIMEOUT},
/* pause         */ {&TimeBomb_ignore, &TimeBomb_pause_ENTRY,         &TimeBomb_ignore,     &TimeBomb_ignore,                &TimeBomb_ignore, &TimeBomb_pause_TIMEOUT},
/* boom          */ {&TimeBomb_ignore, &TimeBomb_boom_ENTRY,          &TimeBomb_ignore,     &TimeBomb_ignore,                &TimeBomb_ignore, &TimeBomb_ignore}
};

static void TimeBomb_dispatch(TimeBomb * const me, Event const * const e) {
	Status stat;
	int prev_state = me->state;
	
	Q_ASSERT((me->state < MAX_STATE) && (e->sig < MAX_SIG));
	stat = (*TimeBomb_table[me->state][e->sig])(me, e);
	
	if (stat == TRAN_STATUS) { /* just executed a transition state? */
		Q_ASSERT(me->state < MAX_STATE);
		(*TimeBomb_table[prev_state][EXIT_SIG])(me, (Event *)0); /* exit action of prev state */
		(*TimeBomb_table[me->state][ENTRY_SIG])(me, (Event *)0); /* entry action of new state */
	}
	else if (stat == INIT_STATUS) { /* init entry case */
		(*TimeBomb_table[me->state][ENTRY_SIG])(me, (Event *)0);
	}
}

void TimeBomb_ctor(TimeBomb * const me) {
	Active_ctor(&me->super, (DispatchHandler)&TimeBomb_dispatch);
	TimeEvent_ctor(&me->te, TIMEOUT_SIG, &me->super);
	me->state = WAITFORBUTTON_STATE;
}

OS_STK timeBomb_stack[100]; /* task stack */
static Event *timeBomb_queue[10];    /* create the message queue */
static TimeBomb timeBomb;              /* create the timeBomb AO */
Active *AO_TimeBomb = &timeBomb.super; /* explicit pointer to timeBomb AO super class? */

/* the main function =========================================================*/
int main() {
    INT8U err;

    BSP_init(); /* initialize the BSP */
    OSInit();   /* initialize uC/OS-II */

    /* create AO and start it */
		TimeBomb_ctor(&timeBomb);
		Active_start(AO_TimeBomb,
								 2U,
								 timeBomb_queue,
								 sizeof(timeBomb_queue)/sizeof(timeBomb_queue[0]),
								 timeBomb_stack,
								 sizeof(timeBomb_stack),
								 0U);

    BSP_start(); /* configure and start the interrupts */

    OSStart(); /* start the uC/OS-II scheduler... */
    return 0; /* NOTE: the scheduler does NOT return */
}

/*******************************************************************************
* NOTE1:
* The call to uC/OS-II API OSTaskCreateExt() assumes that the pointer to the
* top-of-stack (ptos) is at the end of the provided stack memory. This is
* correct only for CPUs with downward-growing stack, but must be changed for
* CPUs with upward-growing stack.
*/
