#include <stdint.h>
#include <stdio.h>
#include "stm32c031xx.h"
#include "qpc.h"
#include "bsp.h"
#include "shape.h"
#include "rectangle.h"
#include <stdlib.h>

Q_DEFINE_THIS_FILE

#define QXThread_STACK_SIZE 40U

QXSemaphore B1_sema;

uint32_t stack_blinky1[QXThread_STACK_SIZE]; /* Stack for blinky1 thread */
QXThread blinky1; /* Declare the thread */
void main_blinky1(QXThread * const me) {
  while(1) {
		uint32_t volatile i;
		for (i = 1500U; i != 0U; --i) {
			BSP_greenLedOn();
			BSP_greenLedOff();
		}
		QXThread_delay(1U);
  }
}

uint32_t stack_blinky2[QXThread_STACK_SIZE]; /* Stack for blinky2 thread */
QXThread blinky2; /* Declare the thread */
void main_blinky2(QXThread * const me) {
	while (1) {
		uint32_t volatile i;
		QXSemaphore_wait(&B1_sema,  // <--- pointer to semaphore to wait on//
								 QXTHREAD_NO_TIMEOUT); // timeout for waiting//
		for (i = 3*1500U; i != 0U; --i) {
			BSP_greenLedOn();
			BSP_greenLedOff();
		}
	}
}

uint32_t stack_blinky3[QXThread_STACK_SIZE]; /* stack for blinky3 thread */
QXThread blinky3; /* Declare the thread */
void main_blinky3(QXThread * const me) {
	while (1) {
		uint32_t volatile i;
		for (i = 3*1500U; i != 0U; --i) {
			BSP_greenLedOn();
			BSP_greenLedOff();
		}
		QXThread_delay(50U);	
	}
}

uint32_t stack_idleThread[QXThread_STACK_SIZE]; /* stack for idle thread */

Rectangle r1; /* static allocation */
Shape s1;
uint32_t a;

int main(void)
{
	
	Shape s2; /* dynamic allocation */
	Shape *ps3 = malloc(sizeof(Rectangle)); /* dynamic and automatic allocation */
	//Shape const *ps1 = &s1; /* const pointer to s1 */
	Shape_ctor(&s1, 5, 11);
	Shape_ctor(&s2, 9, 4);
	Rectangle_ctor((Rectangle *)ps3, 2, 5, 3, 14);
	Rectangle_ctor(&r1, 7, 8, -9, 10);
	
	Shape const *graph[] = {
		&s1,
		(Shape *)&r1,
		ps3,
		(Shape *)0
	};
	drawGraph(graph);
	
	Shape_ctor(&s1, 1, 2);
	Shape_ctor(&s2, 3, 4);
	Shape_ctor(ps3, 5, 6);
	
	Shape_moveBy(&s1, 7, 8);
	Shape_moveBy(&s2, 9, 10);
	Shape_moveBy(ps3, -1, -2);
	//Shape_moveBy(ps1, 4, 5);
	
	Rectangle_draw(&r1);
	a = Rectangle_area(&r1);
	Shape_moveBy(&r1.super, 4, 5);
	
	Q_ASSERT(Shape_distanceTo(&s1, &s1) == 0U);
	Q_ASSERT(Shape_distanceTo(&s1, &s2) == 
					 Shape_distanceTo(&s2, &s1));
	Q_ASSERT(Shape_distanceTo(&s1, &s2) <= 
					 Shape_distanceTo(&s1, &s2) 
				 + Shape_distanceTo(ps3, &s2));
	
	free(ps3);
	
	BSP_init();
	QF_init();
	
	// initialize the B1_sema semaphore as binary, signaling semaphore//
  QXSemaphore_init(&B1_sema, // pointer to semaphore to initialize//
                     0U,  // initial semaphore count (signaling semaphore)//
                     1U); // maximum semaphore count (binary semaphore)//
	
	/* initialize and start blinky1 thread */
	QXThread_ctor(&blinky1, &main_blinky1, 0);
	QXTHREAD_START(&blinky1,
								 5U, /* thread priority */
								 (void *)0, 0, /* message queue, not used */ 
								 stack_blinky1, sizeof(stack_blinky1),
								 (void *)0); /* extra parameter, not used */
	
	/* initialize and start blinky2 thread */
	QXThread_ctor(&blinky2, &main_blinky2, 0);
	QXTHREAD_START(&blinky2,
								 2U, /* thread priority */
								 (void *)0, 0, /* message queue, not used */ 
								 stack_blinky2, sizeof(stack_blinky2),
								 (void *)0); /* extra parameter, not used */
	
	/* initialize and start blinky3 thread */
	//QXThread_ctor(&blinky3, &main_blinky3, 0);
	//QXTHREAD_START(&blinky3,
								 //1U, /* thread priority */
								 //(void *)0, 0, /* message queue, not used */ 
								 //stack_blinky3, sizeof(stack_blinky3),
								 //(void *)0); /* extra parameter, not used */
	
	QF_run(); /* should never return */
	
	Q_ERROR();
  return 0;
}

