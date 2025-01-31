/* Board Support Package */

#include "stm32c0xx.h"
#include "bsp.h"
#include "miros.h"

#define MODER_5 (0b01U << 10)
#define MODER_5_RST (0b11U << 10)
#define OTYPER_5 (1U << 5)
#define OSPEEDR_5_RST (0b11U << 10)
#define OSPEEDR_5 (0b11U << 10)
#define BSRR_BS_5 (1U << 5)
#define BSRR_BR_5 (1U << 21)
#define LD4_PIN  5U

static uint32_t volatile l_tickCtr;

void assert_failed(const char *type) //const char *type makes sure the original string is not modified
{
	(void)type;
  //printf("Assert failed with type: %s\n", type);
	while(1) {
	}
	NVIC_SystemReset();
}

void SysTick_Handler(void) {
	OS_tick();
	
	__disable_irq();
	OS_sched();
	__enable_irq();
}

void BSP_init(void) {
  RCC->IOPENR |= (1U << 0U); //ungate clocks for GPIOs
  
  GPIOA->OTYPER &= ~OTYPER_5;
  GPIOA->OSPEEDR &= ~OSPEEDR_5_RST;
  GPIOA->OSPEEDR |= OSPEEDR_5;
  GPIOA->MODER &= ~MODER_5_RST;
  GPIOA->MODER |= MODER_5;
	
}

uint32_t BSP_tickCtr(void) {
	uint32_t curr_val;
	
	__disable_irq(); //ensure mutual exclusion (critical section)
	curr_val = l_tickCtr;
	__enable_irq();
	
	return curr_val;
}

void BSP_delay(uint32_t delay) {
	uint32_t start = BSP_tickCtr();
	
	while (BSP_tickCtr() < (delay + start)) {
		
	}
}

void BSP_greenLedOn(void) {
	GPIOA->BSRR |= BSRR_BS_5; //set to 1
}

void BSP_greenLedOff(void) {
	GPIOA->BSRR |= BSRR_BR_5; //reset to 0
}

void BSP_blueLedOn(void) {
	//no blue led on STM32 NUCLEO Board
}

void BSP_blueLedOff(void) {
	//no blue led on STM32 NUCLEO Board
}

void OS_onStartup(void) {
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);
	
	/* Set SysTick priority to the highest */
	NVIC_SetPriority(SysTick_IRQn, 0U);

}

void OS_onIdle(void) {
	//__WFI(); /* stop the CPU and wait for interrupt, low power */
}

void Q_onAssert(char const * module, int loc) {
	/* TBD, damage control */
	(void)module; /* avoiding the unused parameter compiler warning */
	(void)loc; /* avoiding the unused parameter compiler warning */
	NVIC_SystemReset();
}