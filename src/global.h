#ifndef __FIFO_H__
#define __FIFO_H__

#include "types.h"

void SysTick_Handler(void);
void delay(uint32_t ms);
void setUnixTimestamp(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t millisecond);
uint64_t getUnixTimestamp(void);
void incrementUnixTimestamp(uint32_t ms);

#endif
