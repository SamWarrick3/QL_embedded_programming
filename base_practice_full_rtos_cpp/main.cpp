#include <stdint.h>
#include <stdio.h>
#include "stm32c031xx.h"
#include "qpc.h"
#include "bsp.h"
#include "shape.h"
#include "rectangle.h"
#include "circle.h"
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

Shape s1(1, 2); /* static allocation */
Rectangle r1(5, 6, 7, 8);
Circle c1(9, 2, 4);
uint32_t a;

int main(void)
{
	
	Shape s2(3, 4); /* dynamic allocation */
	Shape *ps3 = new Shape(5, 6); /* dynamic and automatic allocation */
	//Shape const *ps1 = &s1; /* const pointer to s1 */
	
	Shape const *graph[] = {
		&r1,
		&c1,
		ps3,
		(Shape *)0
	};
	
	s1.moveBy(7, 8);
	s2.moveBy(9, 10);
	ps3->moveBy(-1, -1);
	//ps1->moveBy(-3, -4);
	
	drawGraph(graph);
	Shape *ps = &r1; /* upcasting */
	r1.draw();
	ps->draw();
	a = r1.area();
	r1.moveBy(3, 8);
	Q_ASSERT(r1.distanceTo(&r1) == 0U);
	
	Q_ASSERT(s1.distanceTo(&s1) == 0U);
	Q_ASSERT(s1.distanceTo(&s2) == 
					 s2.distanceTo(&s1));
	Q_ASSERT(s1.distanceTo(&s2) <= 
					 s1.distanceTo(&s2) 
				 + ps3->distanceTo(&s2));
	
	delete ps3;
	
	BSP_init();
	QF_init();
	
	// initialize the B1_sema semaphore as binary, signaling semaphore//
  QXSemaphore_init(&B1_sema, // pointer to semaphore to initialize//
                     0U,  // initial semaphore count (signaling semaphore)//
                     1U); // maximum semaphore count (binary semaphore)//
	
	/* initialize and start blinky1 thread */
	QXThread_ctor(&blinky1, &main_blinky1, 0);
	QEvt const *ptr0 = 0;
	QXTHREAD_START(&blinky1,
								 5U, /* thread priority */
								 &ptr0, 0, /* message queue, not used */ 
								 stack_blinky1, sizeof(stack_blinky1),
								 &ptr0); /* extra parameter, not used */
	
	/* initialize and start blinky2 thread */
	QXThread_ctor(&blinky2, &main_blinky2, 0);
	QXTHREAD_START(&blinky2,
								 2U, /* thread priority */
								 &ptr0, 0, /* message queue, not used */ 
								 stack_blinky2, sizeof(stack_blinky2),
								 &ptr0); /* extra parameter, not used */
	
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

