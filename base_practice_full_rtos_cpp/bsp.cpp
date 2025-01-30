/* Board Support Package */

#include "qpc.h"
#include "bsp.h"
#include "stm32c0xx.h"

#define MODER_5 (0b01U << 10)
#define MODER_5_RST (0b11U << 10)
#define OTYPER_5 (1U << 5)
#define OSPEEDR_5_RST (0b11U << 10)
#define OSPEEDR_5 (0b11U << 10)
#define BSRR_BS_5 (1U << 5)
#define BSRR_BR_5 (1U << 21)
#define LD4_PIN  5U
#define BTN_B1 13U

static uint32_t volatile l_tickCtr;

#ifdef __cplusplus
extern "C" {
#endif
void assert_failed(const char *type) //const char *type makes sure the original string is not modified
{
	(void)type;
  //printf("Assert failed with type: %s\n", type);
	while(1) {
	}
	NVIC_SystemReset();
}

#ifdef __cplusplus
}
#endif

void SysTick_Handler(void) {
	QXK_ISR_ENTRY(); /* inform QXK about entering an ISR */
	
	QF_TICK_X(0U, (void *)0);
	
	//__disable_irq();
	QXK_ISR_EXIT(); /* inform QXK about exiting an ISR */
	//__enable_irq();
}

void EXTI4_15_IRQHandler(void) {
	QXK_ISR_ENTRY(); /* inform QXK about entering an ISR */
	
	if (EXTI->FPR1 == (1U << BTN_B1)) { /* interrupt triggered by BTN_B1 */
		EXTI->FPR1 = (1U << BTN_B1); /* clear interrupt */
		QXSemaphore_signal(&B1_sema);
	}
	//__disable_irq();
	QXK_ISR_EXIT(); /* inform QXK about exiting an ISR */
	//__enable_irq();
}

void BSP_init(void) {
  RCC->IOPENR |= (1U << 0U); //ungate clocks for GPIOA
  
  GPIOA->OTYPER &= ~OTYPER_5;
  GPIOA->OSPEEDR &= ~OSPEEDR_5_RST;
  GPIOA->OSPEEDR |= OSPEEDR_5;
  GPIOA->MODER &= ~MODER_5_RST;
  GPIOA->MODER |= MODER_5;
	
	RCC->IOPENR |= (1U << 2U); /* ungate clocks for GPIOC */
	
	/* configure button B1 */
	GPIOC->MODER &= ~(3U << 2*BTN_B1); /* set input mode */
	GPIOC->PUPDR &= ~(3U << 2*BTN_B1); /*no pull up pull down */
	
	/* configure button B1 as an interrupt */
	EXTI->EMR1 &= ~(1U << BTN_B1); /* disable event wakeups */
	EXTI->IMR1 |= (1U << BTN_B1); /* enable interrupts on PC13*/
	EXTI->RTSR1 &= ~(1U << BTN_B1); /* clear rising trigger */
	EXTI->FTSR1 |= (1U << BTN_B1); /* enable falling trigger */
	EXTI->EXTICR[3] |= (2U << 8); /* enable BTN_B1 as an interrupt */
	
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

/* --------Callbacks-------------------------------------------------------- */
void QF_onStartup(void) {
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);
	
	/* set the interrupt priorities of "kernel aware" interrupts */
	NVIC_SetPriority(SysTick_IRQn, QF_AWARE_ISR_CMSIS_PRI);
	NVIC_SetPriority(EXTI4_15_IRQn, QF_AWARE_ISR_CMSIS_PRI);

	/* enable interrupts in NVIC */
	NVIC_EnableIRQ(EXTI4_15_IRQn);
}

void QF_onCleanup(void) {

}

void QXK_onIdle(void) {
	//__WFI(); /* stop the CPU and wait for interrupt, low power */
}

void Q_onAssert(char const * module, int loc) {
	/* TBD, damage control */
	(void)module; /* avoiding the unused parameter compiler warning */
	(void)loc; /* avoiding the unused parameter compiler warning */
	NVIC_SystemReset();
}