#ifndef __BSP_H__
#define __BSP_H__

#include <stdint.h>

#define GPIOA_MODER (*((unsigned int *)(GPIOA_BASE + 0x00U)))
#define GPIOA_BSRR (*((unsigned int *)(GPIOA_BASE + 0x18U)))
	
#define BSP_TICKS_PER_SEC 1000U

void BSP_init(void);

void BSP_delay(uint32_t delay);

void BSP_greenLedOn(void);
void BSP_greenLedOff(void);
void BSP_blueLedOn(void);
void BSP_blueLedOff(void);
#endif // __BSP_H__