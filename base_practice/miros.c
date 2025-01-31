/****************************************************************************
* MInimal Real-time Operating System (MIROS)
* version 0.26 (matching lesson 26)
*
* This software is a teaching aid to illustrate the concepts underlying
* a Real-Time Operating System (RTOS). The main goal of the software is
* simplicity and clear presentation of the concepts, but without dealing
* with various corner cases, portability, or error handling. For these
* reasons, the software is generally NOT intended or recommended for use
* in commercial applications. This software is heavily based on the software
* provided in the course Modern Embedded Systems Programming provided
* at https://www.state-machine.com.
*
* Copyright (C) 2024 Samuel Warrick. All Rights Reserved.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <https://www.gnu.org/licenses/>.
*
* Contact Information:
* swarrick6@gmail.com
****************************************************************************/

#include "miros.h"
#include "stm32c0xx.h"
#include "qassert.h"
#include "bsp.h"

Q_DEFINE_THIS_FILE

#define LOG2(x) (32U - __clz(x))

OSThread * volatile OS_curr; /* pointer to current thread executing */
OSThread * volatile OS_next; /* pointer to next thread to execute */

OSThread *OS_threads[32 + 1]; /* array of threads being used */
uint32_t OS_readySet; /* bit mask for threads that are ready, does not include the idle thread*/
uint32_t OS_blockedSet; /* bit mask for delayed threads */

OSThread idleThread; /* Declare the idle thread */
void main_idleThread(void) {
  while(1) {
		OS_onIdle();
  }
}

void OS_init(void *stkSto, uint32_t stkSize) {
	/* set the PendSV interrupt priority to the lowest level, same address for all cortex - M*/
	*(uint32_t volatile *)0xE000ED20 |= (0xFFU << 16);
	
	OSThread_start(&idleThread,
								 0U, /* set idle thread priority */
								 &main_idleThread,
								 stkSto, stkSize);
}

void OS_sched(void) {
	//uint8_t bit;
	/* check if all threads are blocked */
	if (OS_readySet == 0U) {
		OS_next = OS_threads[0]; /* idle thread */
	} else {
		OS_next = OS_threads[LOG2(OS_readySet)];
		Q_ASSERT(OS_next != (OSThread *)0);
	}
	
	if (OS_next != OS_curr) {
		*(uint32_t volatile *)0xE000ED04 = (1U << 28); /* Set the PendSV interrupt pending bit in the ICSR register */
	}
}

void OS_run(void) {
	/* callback to configure and start interrupts */
	OS_onStartup();
	
		__disable_irq();
	OS_sched();
	__enable_irq();
	
	/* following code should never execute */
	Q_ERROR();
}

void OS_delay(uint32_t ticks) {
	uint32_t bit;
	__disable_irq();
	
	/* check we are not delaying the idle thread */
	Q_REQUIRE(OS_curr != OS_threads[0]);
	
	OS_curr->timeout = ticks;
	bit = (1U << (OS_curr->prio - 1U));
	OS_readySet &= ~bit;
	OS_blockedSet |= bit;
	OS_sched();
	
	__enable_irq();
}

void OS_tick(void) {
	uint32_t workingSet = OS_blockedSet;
	while (workingSet != 0U) {
		OSThread *t = OS_threads[LOG2(workingSet)];
		uint32_t bit;
		Q_ASSERT((t != (OSThread *)0) && (t->timeout != 0U)); /* sanity check */
		
		bit = (1U << (t->prio - 1U));
		--t->timeout;
		if (t->timeout == 0U) {
			OS_blockedSet &= ~bit; /* remove thread from blocked */
			OS_readySet |= bit; /* set thread as ready */
		}
		workingSet &= ~bit;
	}
}

void OSThread_start(OSThread *me,
										uint8_t prio, /* priority of this thread */
										OSThreadHandler threadHandler, 
										void *stkSto, uint32_t stkSize) 
{
	/* initialize stack pointer to top of stack + 1 address and ensure the stack is
	* 8 byte aligned (by rounding down) so the user is agnostic to the architecture */
	uint32_t *sp = (uint32_t *)((((uint32_t)stkSto + stkSize) / 8) * 8);
	uint32_t *stk_limit;
	
	/* check the priority is in range and has not been taken */
	Q_REQUIRE((prio < Q_DIM(OS_threads)) 
						 && (OS_threads[prio] == (OSThread *)0)); /* will not return if false */
											
	*(--sp) = (1U << 24); /* xPSR - instruction set is Thumb */
	*(--sp) = (uint32_t)threadHandler; /* PC */
	*(--sp) = 0x0000000EU; /* LR */
	*(--sp) = 0x0000000CU; /* R12 */
	*(--sp) = 0x00000003U; /* R3 */
	*(--sp) = 0x00000002U; /* R2 */
	*(--sp) = 0x00000001U; /* R1 */
	*(--sp) = 0x00000000U; /* R0 */
	/* additionally, fake registers R4 - R11*/
	*(--sp) = 0x0000000BU; /* R11 */ 
	*(--sp) = 0x0000000AU; /* R10 */
	*(--sp) = 0x00000009U; /* R9 */
	*(--sp) = 0x00000008U; /* R8 */
	*(--sp) = 0x00000007U; /* R7 */
	*(--sp) = 0x00000006U; /* R6 */
	*(--sp) = 0x00000005U; /* R5 */
	*(--sp) = 0x00000004U; /* R4 */
	
	/* set the thread's member sp to the current top of the stack */
	me->sp = sp;
	
	/* round the bottom of the stack up to the 8-byte boundary*/
	stk_limit = (uint32_t *)(((((uint32_t)stkSto - 1) / 8) + 1) * 8);
	
	/* pre-fill the unused part of the stack */
	for (sp = sp - 1U; sp >= stk_limit; --sp) {
		*sp = 0xDEADBEEFU;
	}
	
	/* register the thread with the OS_threads array */
	OS_threads[prio] = me;
	
	/* set the priority level */
	me->prio = prio;
	
	/* set the thread as ready to run */
	if (prio > 0U) {
		OS_readySet |= (1U << (prio -1U));
	}
}

__attribute__ ((naked))
void PendSV_Handler(void) {
__ASM volatile (
	/* __disable_irq(); */
	/* (OSThread *) 0 is the same as NULL but NULL is undeclared because we are not including <stdio.h> */ 
	/* 0x080005B0 B081      SUB      sp,sp,#0x04 */
	/* __ASM volatile ("cpsid i" : : : "memory"); */
	"	CPSID    I							\n"
	
	/* if (OS_curr != (OSThread *) 0) { //for the first case when there is no thread yet */
	"	LDR      r0,=OS_curr		\n"
	"	LDR      r0,[r0,#0x00]	\n"
	"	CMP      r0,#0x00				\n"
	"	BEQ      PendSV_restore	\n"
	
	/* Save the R4-R11 registers to the top of the stack and adjust the pointer */
	" SUB      sp,sp,#(8*4)		\n"
	"	MOV      r0,sp					\n"
	"	STM      r0!,{r4-r7}		\n"
	" MOV      r4,r8					\n"
	" MOV      r5,r9					\n"
	" MOV      r6,r10					\n"
	" MOV      r7,r11					\n"
	"	STM      r0!,{r4-r7}		\n"
	
	/* OS_curr->sp = sp; */
	"	LDR      r1,=OS_curr		\n"
	"	LDR      r1,[r1,#0x00]	\n"
	"	MOV      r0,sp					\n"
	"	STR      r0,[r1,#0x00]	\n"

	/* sp = OS_next->sp; */
	"	PendSV_restore:					\n"
	"	LDR      r1,=OS_next		\n"
	"	LDR      r1,[r1,#0x00]	\n"
	" LDR      r0,[r1,#0x00]	\n"
	"	MOV      sp,r0					\n"
	
	/* OS_curr = OS_next; */
	"	LDR      r0,=OS_next		\n"
	"	LDR      r0,[r0,#0x00]	\n"
	"	LDR      r1,=OS_curr		\n"
	"	STR      r0,[r1,#0x00]	\n"
	
	/* pop registers R4-R11 and set the sp */
	"	MOV      r0,sp					\n"
	"	MOV      r1,r0					\n"
	" ADDS     r1,r1,#(4*4)		\n"
	" LDM      r1!,{r4-r7}			\n"
	" MOV      r8,r4					\n"
	" MOV      r9,r5					\n"
	" MOV      r10,r6					\n"
	" MOV      r11,r7					\n"
	" LDM      r0!,{r4-r7}			\n"
	" ADD      sp,sp,#(8*4)		\n"
	
	/* __enable_irq */
	/* __ASM volatile ("cpsie i" : : : "memory"); */
	"	CPSIE    I							\n"

	/* return to the next thread */
	"	BX       lr							\n"
	);
}