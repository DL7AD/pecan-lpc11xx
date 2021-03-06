#ifndef __DEFINES_H__
#define __DEFINES_H__

#define FALSE	(uint8_t)0
#define TRUE	(uint8_t)1
#define true	TRUE
#define false	FALSE

#define LOW		(uint8_t)0
#define HIGH	(uint8_t)1
#define high	HIGH
#define low		LOW

#define NULL	((void *)0)

// Configuration defines
#define TARGET_PECAN_FEMTO2_1	1
#define TARGET_PECAN_PICO6		2

#define REF_VCC1V8_LDO			4
#define REF_VCC					5

#define BUS_UART				6
#define BUS_I2C					7

#define PRIMARY					8
#define SECONDARY				9

#define _20MHZ					10
#define _26MHZ					11
#define _27MHZ					12

#define AD0						0
#define AD1						1
#define AD2						2
#define AD3						3
#define AD4						4
#define AD5						5
#define AD6						6
#define AD7						7

#endif
