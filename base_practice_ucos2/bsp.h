/*****************************************************************************
* BSP for NUCLEO-C031C6 with uC/OS-II RTOS
*****************************************************************************/
#ifndef BSP_H
#define BSP_H

void BSP_init(void);
void BSP_start(void);

void BSP_ledRedOn(void);
void BSP_ledRedOff(void);
void BSP_ledBlueOn(void);
void BSP_ledBlueOff(void);
void BSP_ledGreenOn(void);
void BSP_ledGreenOff(void);

/* note enumerated constants are automatically assigned numerical values based on positioning in the enum struct */
enum EventSignals {
	BUTTON_PRESSED_SIG = USER_SIG, /* 0 */
	BUTTON_RELEASED_SIG,           /* 1 */
	TIMEOUT_SIG,                   /* 2 */
	/* ... */
	MAX_SIG                        /* 3 */
};

/* active objects */
extern Active *AO_TimeBomb;

#endif /* BSP_H */
