#ifndef __TARGET_H__
#define __TARGET_H__

#include "types.h"
#include "LPC11xx.h"

#undef	SYSAHBCLKDIV_Val
#undef	AHBCLKCTRL_Val
#define AHBCLKCTRL_Val				0x0001305F			//!< 0000 0000 0000 0001 0000 0000 0101 1111
														//                   ||| | || |||| |||| |||+-    0 SYS
														//                   ||| | || |||| |||| ||+--    1 ROM
														//                   ||| | || |||| |||| |+---    2 RAM
														//                   ||| | || |||| |||| +----    3 FLASHREG
														//                   ||| | || |||| |||+------    4 FLASHARRAY
														//                   ||| | || |||| ||+-------    5 I2C
														//                   ||| | || |||| |+--------    6 GPIO
														//                   ||| | || |||| +---------	 7 CT16B0
														//                   ||| | || |||+-----------    8 CT16B1
														//                   ||| | || ||+------------    9 CT32B0
														//                   ||| | || |+-------------   10 CT32B1
														//                   ||| | || +--------------   11 SSP0
														//                   ||| | |+----------------   12 UART
														//                   ||| | +-----------------   13 ADC
														//                   ||| |                      14 reserved
														//                   ||| +-------------------   15 WDT
														//                   ||+---------------------   16 IOCON
														//                   |+----------------------   17 CAN
														//                   +-----------------------   18 SSP1

#define Fosc	 					12000000			//!< (Hz) IRC-Frequenz
#define Fext						27000000

#define UART_BAUDRATE				9600
#define UART_DATA_MAXLENGTH			128

// Interrupt priority
#define INT_PRIORITY_TMR16B0		5
#define INT_PRIORITY_TMR16B1		6
#define INT_PRIORITY_I2C			7
#define INT_PRIORITY_UART			8
#define INT_PRIORITY_SSP			9

void TargetResetInit(void);
void Target_SetClock_IRC(void);
void Target_SetClock_PLL(uint32_t crystal, uint32_t frequency);
void Set_Flash_Access_Time(uint32_t frequency);
uint32_t getFcclk(void);

#endif
