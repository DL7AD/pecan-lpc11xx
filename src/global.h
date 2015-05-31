#ifndef __FIFO_H__
#define __FIFO_H__

#include "types.h"

void SysTick_Handler(void);
void delay(uint32_t ms);
void setUnixTimestamp(uint64_t time);
uint64_t getUnixTimestamp(void);

#endif
