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

#ifndef __MIROS_H__
#define __MIROS_H__

#include <stdint.h>

/* Thread Control Block (TCB) */
typedef struct {
	void *sp; /* stack pointer */
	uint32_t timeout; /* delay timeout in ticks */
	uint8_t prio; /* thread priority level */
} OSThread;

/* callback to define the idle thread, implemented in BSP.c */
void OS_init(void *stkSto, uint32_t stkSize);

/* This function should always be called with interrupts DISABLED */
void OS_sched(void);

/* transfer control to the running threads */
void OS_run(void);

/* set the current thread to delay state */
void OS_delay(uint32_t ticks);

/* check threads that are delayed */
void OS_tick(void);

/* callback to configure and start interrupts, implemented in BSP.c */
void OS_onStartup(void);

/* idle thread */
void OS_onIdle(void);

typedef void (*OSThreadHandler)();

void OSThread_start(OSThread *me,
										uint8_t prio, /* priority of this thread */
										OSThreadHandler threadHandler, 
										void *stkSto, uint32_t stkSize);

#endif //__MIROS_H__