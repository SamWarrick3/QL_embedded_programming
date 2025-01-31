#include <stdint.h>
#include <stdio.h>
#include "stm32c031xx.h"
#include "bsp.h"
#include "miros.h"
#include "qassert.h"

Q_DEFINE_THIS_FILE

#define OSTHREAD_STACK_SIZE 40U

uint32_t stack_blinky1[OSTHREAD_STACK_SIZE]; /* Stack for blinky1 thread */
OSThread blinky1; /* Declare the thread */
void main_blinky1(void) {
	BSP_blueLedOn();
	BSP_blueLedOff();
  while(1) {
		uint32_t volatile i;
		for (i = 1500U; i != 0U; --i) {
			BSP_greenLedOn();
			BSP_greenLedOff();
		}
		OS_delay(1U);
  }
}

uint32_t stack_blinky2[OSTHREAD_STACK_SIZE]; /* Stack for blinky2 thread */
OSThread blinky2; /* Declare the thread */
void main_blinky2(void) {
	while (1) {
		BSP_blueLedOn();
		OS_delay(BSP_TICKS_PER_SEC / 2U);
		BSP_blueLedOff();
		OS_delay(BSP_TICKS_PER_SEC / 3U);
	}
}

uint32_t stack_blinky3[OSTHREAD_STACK_SIZE]; /* stack for blinky3 thread */
OSThread blinky3; /* Declare the thread */
void main_blinky3(void) {
	BSP_blueLedOn();
	BSP_blueLedOff();
	while (1) {
		uint32_t volatile i;
		for (i = 3*1500U; i != 0U; --i) {
			BSP_greenLedOn();
			BSP_greenLedOff();
		}
		OS_delay(50U);	
	}
}

uint32_t stack_idleThread[OSTHREAD_STACK_SIZE]; /* stack for idle thread */
int main(void)
{
	BSP_init();
	OS_init(stack_idleThread, sizeof(stack_idleThread));
	
	/* create the stack frame for the blinky1 thread */
	OSThread_start(&blinky1,
								 5U, /* thread priority */
								 &main_blinky1, 
								 stack_blinky1, sizeof(stack_blinky1));
	
	/* create the stack frame for the blinky2 thread */
	//OSThread_start(&blinky2,
								 //1U, /* thread priority */
								 //&main_blinky2, 
								 //stack_blinky2, sizeof(stack_blinky2));
	
	/* create the stack frame for the blinky3 thread */
	OSThread_start (&blinky3,
									2U, /* thread priority */
									&main_blinky3,
									stack_blinky3, sizeof(stack_blinky3));
	
	OS_run(); /* should never return */
	
	Q_ERROR();
  return 0;
}

