#include "LPC11xx.h"

#define DEBUG_INIT() { \
	LPC_IOCON->EXT_PIO_OUT1	&= ~(0x10); \
	EXT_GPIO_OUT1->DIR		|= EXT_PIN_OUT1; \
	LPC_IOCON->EXT_PIO_OUT2	&= ~(0x10); \
	EXT_GPIO_OUT2->DIR		|= EXT_PIN_OUT2; \
	LPC_IOCON->EXT_PIO_OUT3	&= ~(0x10); \
	EXT_GPIO_OUT3->DIR		|= EXT_PIN_OUT3; \
	LPC_IOCON->EXT_PIO_OUT4	&= ~(0x10); \
	EXT_GPIO_OUT4->DIR		|= EXT_PIN_OUT4; \
}

#define OUT1_SET(Select)	{ \
	if (Select) \
		EXT_GPIO_OUT1->DATA |= EXT_PIN_OUT1; \
	else \
		EXT_GPIO_OUT1->DATA &= ~EXT_PIN_OUT1; \
}

#define OUT2_SET(Select)	{ \
	if (Select) \
		EXT_GPIO_OUT2->DATA |= EXT_PIN_OUT2; \
	else \
		EXT_GPIO_OUT2->DATA &= ~EXT_PIN_OUT2; \
}

#define OUT3_SET(Select)	{ \
	if (Select) \
		EXT_GPIO_OUT3->DATA |= EXT_PIN_OUT3; \
	else \
		EXT_GPIO_OUT3->DATA &= ~EXT_PIN_OUT3; \
}

#define OUT4_SET(Select)	{ \
	if (Select) \
		EXT_GPIO_OUT4->DATA |= EXT_PIN_OUT4; \
	else \
		EXT_GPIO_OUT4->DATA &= ~EXT_PIN_OUT4; \
}


