#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "types.h"

typedef struct {
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint16_t millisecond;
} date_t;

void SysTick_Handler(void);
void delay(uint32_t ms);
uint64_t date2UnixTimestamp(date_t time);
void setUnixTimestamp(uint64_t time);
uint64_t getUnixTimestamp(void);
void incrementUnixTimestamp(uint32_t ms);
date_t getUnixTimestampDecoded(void);

#endif
