#ifndef __DEBUG__H__
#define __DEBUG__H__

#include "LPC11xx.h"
#include "config.h"

#ifdef EXT_PIN_AVAIL

#define OUT1_CONF_WRITE() { \
	LPC_IOCON->EXT_PIO_OUT1	&= ~(0x10); \
	EXT_GPIO_OUT1->DIR		|= EXT_PIN_OUT1; \
}
#define OUT2_CONF_WRITE() { \
	LPC_IOCON->EXT_PIO_OUT2	&= ~(0x10); \
	EXT_GPIO_OUT2->DIR		|= EXT_PIN_OUT2; \
}
#define OUT3_CONF_WRITE() { \
	LPC_IOCON->EXT_PIO_OUT3	&= ~(0x10); \
	EXT_GPIO_OUT3->DIR		|= EXT_PIN_OUT3; \
}
#define OUT4_CONF_WRITE() { \
	LPC_IOCON->EXT_PIO_OUT4	&= ~(0x10); \
	EXT_GPIO_OUT4->DIR		|= EXT_PIN_OUT4; \
}

#define OUT1_CONF_READ() { \
	LPC_IOCON->EXT_PIO_OUT1	&= ~(0x10); \
	EXT_GPIO_OUT1->DIR		&= ~EXT_PIN_OUT1; \
}
#define OUT2_CONF_READ() { \
	LPC_IOCON->EXT_PIO_OUT2	&= ~(0x10); \
	EXT_GPIO_OUT2->DIR		&= ~EXT_PIN_OUT2; \
}
#define OUT3_CONF_READ() { \
	LPC_IOCON->EXT_PIO_OUT3	&= ~(0x10); \
	EXT_GPIO_OUT3->DIR		&= ~EXT_PIN_OUT3; \
}
#define OUT4_CONF_READ() { \
	LPC_IOCON->EXT_PIO_OUT4	&= ~(0x10); \
	EXT_GPIO_OUT4->DIR		&= ~EXT_PIN_OUT4; \
}

#define OUT1_READ	EXT_GPIO_OUT1->DATA & (1<<EXT_PIN_OUT1)
#define OUT2_READ	EXT_GPIO_OUT2->DATA & (1<<EXT_PIN_OUT2)
#define OUT3_READ	EXT_GPIO_OUT3->DATA & (1<<EXT_PIN_OUT3)
#define OUT4_READ	EXT_GPIO_OUT4->DATA & (1<<EXT_PIN_OUT4)

#define DEBUG_INIT_WRITE() { \
	OUT1_CONF_WRITE(); \
	OUT2_CONF_WRITE(); \
	OUT3_CONF_WRITE(); \
	OUT4_CONF_WRITE(); \
}

#define DEBUG_INIT_READ() { \
	OUT1_CONF_READ(); \
	OUT2_CONF_READ(); \
	OUT3_CONF_READ(); \
	OUT4_CONF_READ(); \
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

#endif
#endif
