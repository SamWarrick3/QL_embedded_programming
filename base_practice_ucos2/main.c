/* BlinkyButton/Button with uC/OS-II RTOS */
#include "uc_ao.h"  /* uC/OS-II API, port and compile-time configuration */
#include "qassert.h"  /* embedded-system-friendly assertions */
#include "bsp.h"      /* Board Support Package */
#include <stdbool.h>

Q_DEFINE_THIS_MODULE("main") /* this module name for Q_ASSERT() */

/* The BlinkyButton thread =========================================================*/
enum { INITIAL_BLINK_TIME = (OS_TICKS_PER_SEC / 4) };

/* The BlinkyButton active object ================================================== */
typedef struct {
	Active super; /* inherit active base class */
	/* private data for the AO */
	TimeEvent te;
	bool isLedOn;
	uint32_t blink_time;
} BlinkyButton;

static void BlinkyButton_dispatch(BlinkyButton * const me, Event const * const e) {
	switch(e->sig){
		case INIT_SIG:
			BSP_ledGreenOff();
			/* intentionally fall through */
		case TIMEOUT_SIG: {
			if (me->isLedOn) { /* Led is on */
				BSP_ledGreenOff();
				me->isLedOn = false;
				TimeEvent_arm(&me->te, me->blink_time * 3U, 0U);
			}
			else { /* Led is off */
				BSP_ledGreenOn();
				me->isLedOn = true;
				TimeEvent_arm(&me->te, me->blink_time, 0U);
			}
			break;
		}
		case BUTTON_PRESSED_SIG: {
			BSP_ledBlueOn();
			
			/* update the blink time for the 'blink' thread */
			me->blink_time >>= 1; /* shorten the blink time by factor of 2 */
			if (me->blink_time == 0U) {
					me->blink_time = INITIAL_BLINK_TIME;
			}	
			break;
		}
		case BUTTON_RELEASED_SIG: {
			BSP_ledBlueOff();
			break;
		}
		default: {
			break;
		}
	}
}

void BlinkyButton_ctor(BlinkyButton * const me) {
	Active_ctor(&me->super, (DispatchHandler)&BlinkyButton_dispatch);
	TimeEvent_ctor(&me->te, TIMEOUT_SIG, &me->super);
	me->isLedOn = false;
	me->blink_time = INITIAL_BLINK_TIME;
}

OS_STK blinkyButton_stack[100]; /* task stack */
static Event *blinkyButton_queue[10];    /* create the message queue */
static BlinkyButton blinkyButton;              /* create the blinkyButton AO */
Active *AO_BlinkyButton = &blinkyButton.super; /* explicit pointer to blinkyButton AO super class? */

/* the main function =========================================================*/
int main() {
    INT8U err;

    BSP_init(); /* initialize the BSP */
    OSInit();   /* initialize uC/OS-II */

    /* create AO and start it */
		BlinkyButton_ctor(&blinkyButton);
		Active_start(AO_BlinkyButton,
								 2U,
								 blinkyButton_queue,
								 sizeof(blinkyButton_queue)/sizeof(blinkyButton_queue[0]),
								 blinkyButton_stack,
								 sizeof(blinkyButton_stack),
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
