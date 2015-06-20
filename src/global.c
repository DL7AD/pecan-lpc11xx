#include "LPC11xx.h"
#include "global.h"
#include "types.h"

uint64_t unixTimeStamp = 0; // UNIX timestamp in milliseconds
const uint16_t nonLeapYear[] = {0,31,59,89,120,151,181,212,243,273,304,334};
const uint16_t leapYear[] = {0,31,58,88,119,150,180,211,242,272,303,333};

/**
 * @brief Interrupt routine for SystemTick
 * This method should be called every 1ms.
 */
void SysTick_Handler(void) {
	unixTimeStamp++;
}

/**
 * Calculates UNIX timestamp and writes it to internal interrupt controlled RTC.
 * Calculation valid until 2100 due to missing leapyear in 2100.
 * @param year Current Year
 * @param month Current Month
 * @param day Current Day
 * @param hour Current Hour
 * @param minute Current Minute
 * @param second Current Second
 * @param millisecond Current Millisecond
 */
void setUnixTimestamp(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t millisecond) {
	uint64_t time;
	time  = second;
	time += minute * 60;
	time += hour * 3600;
	time += day * 86400;

	if(year % 4 == 0) { // is leapyear?
		time += leapYear[month-1] * 86400;
	} else {
		time += leapYear[month-1] * 86400;
	}

	uint16_t i;
	for(i=1970; i<year; i++) {
		if(i % 4 == 0) { // is leapyear?
			time += 31622400;
		} else {
			time += 31536000;
		}
	}

	unixTimeStamp = time * 1000 + millisecond;
}

/**
 * @brief Returns unix timestamp
 * @return time UNIX time in ms
 */
uint64_t getUnixTimestamp(void) {
	return unixTimeStamp;
}

/**
 * Increments unix timestamp
 * @param ms Time to be incremented in milliseconds
 */
void incrementUnixTimestamp(uint32_t ms) {
	unixTimeStamp += ms;
}

/**
 * Executes a delay
 */
void delay(uint32_t ms) {
	uint32_t i = 0;
	while(i++ < ms)
		__WFI();
}
