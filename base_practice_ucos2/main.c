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

static void TimeBomb_init(TimeBomb * const me, Event const * const e) {
	BSP_ledGreenOff();
	me->state = WAITFORBUTTON_STATE;
	me->blink_time = INITIAL_BLINK_TIME * 3U;
}

static void TimeBomb_waitforbutton_PRESSED(TimeBomb * const me, Event const * const e) {
	BSP_ledGreenOn();
	/* start the countdown */
	me->blink_ctr = 3U;
	TimeEvent_arm(&me->te, me->blink_time, 0U);
	me->state = BLINK_STATE;
}
	
static void TimeBomb_blink_TIMEOUT(TimeBomb * const me, Event const * const e) {
	BSP_ledGreenOff();
	TimeEvent_arm(&me->te, me->blink_time, 0U);
	me->state = PAUSE_STATE;
}
	
static void TimeBomb_pause_TIMEOUT(TimeBomb * const me, Event const * const e) {
	BSP_ledGreenOn();
	me->blink_ctr--;
	if (me->blink_ctr > 0U) {
		TimeEvent_arm(&me->te, me->blink_time, 0U);
		me->state = BLINK_STATE;
	}
	else {
		me->state = BOOM_STATE;
	}
}
	
static void TimeBomb_ignore(TimeBomb * const me, Event const * const e) {
	/* pass */
}

typedef void (*TimeBombAction)(TimeBomb * const me, Event const * const e);

TimeBombAction const TimeBomb_table[MAX_STATE][MAX_SIG] = {
                 /* INIT               | BUTTON_PRESSED                 | BUTTON_RELEASED | TIMEOUT             | */
/* waitforbutton */ {&TimeBomb_init,   &TimeBomb_waitforbutton_PRESSED, &TimeBomb_ignore, &TimeBomb_ignore},
/* blink         */ {&TimeBomb_ignore, &TimeBomb_ignore,                &TimeBomb_ignore, &TimeBomb_blink_TIMEOUT},
/* pause         */ {&TimeBomb_ignore, &TimeBomb_ignore,                &TimeBomb_ignore, &TimeBomb_pause_TIMEOUT},
/* boom          */ {&TimeBomb_ignore, &TimeBomb_ignore,                &TimeBomb_ignore, &TimeBomb_ignore}
};

static void TimeBomb_dispatch(TimeBomb * const me, Event const * const e) {
	Q_ASSERT((me->state < MAX_STATE) && (e->sig < MAX_SIG));
	(*TimeBomb_table[me->state][e->sig])(me, e);
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
