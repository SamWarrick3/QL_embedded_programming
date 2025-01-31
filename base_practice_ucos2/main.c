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
	enum {
		WAITFORBUTTON_STATE,
		BLINK_STATE,
		PAUSE_STATE,
		BOOM_STATE
	} state;
	uint32_t blink_time;
	uint8_t blink_ctr;
} TimeBomb;

static void TimeBomb_dispatch(TimeBomb * const me, Event const * const e) {
	if (e->sig == INIT_SIG) {
		BSP_ledGreenOff();
		me->state = WAITFORBUTTON_STATE;
		me->blink_time = INITIAL_BLINK_TIME * 3U;
	}
	
	switch(me->state) {
		case WAITFORBUTTON_STATE: {
			switch (e->sig) {
				case BUTTON_PRESSED_SIG: {
					BSP_ledGreenOn();
					/* start the countdown */
					me->blink_ctr = 3U;
					TimeEvent_arm(&me->te, me->blink_time, 0U);
					me->state = BLINK_STATE;
					break;
				}
			}
			break;
		}
		case BLINK_STATE: {
			switch (e->sig) {
				case TIMEOUT_SIG: {
					BSP_ledGreenOff();
					TimeEvent_arm(&me->te, me->blink_time, 0U);
					me->state = PAUSE_STATE;
					break;
				}
			}
			break;
		}
		case PAUSE_STATE: {
			switch (e->sig) {
				case TIMEOUT_SIG: {
					BSP_ledGreenOn();
					me->blink_ctr--;
					if (me->blink_ctr > 0U) {
						TimeEvent_arm(&me->te, me->blink_time, 0U);
						me->state = BLINK_STATE;
					}
					else {
						me->state = BOOM_STATE;
					}
					break;
				}
			}
		}
		case BOOM_STATE: {
			break;
		}
		default: {
			Q_ASSERT(0); /* should never be in this state */
			break;			
		}	
	}
}

void TimeBomb_ctor(TimeBomb * const me) {
	Active_ctor(&me->super, (DispatchHandler)&TimeBomb_dispatch);
	TimeEvent_ctor(&me->te, TIMEOUT_SIG, &me->super);
	me->blink_time = INITIAL_BLINK_TIME;
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
