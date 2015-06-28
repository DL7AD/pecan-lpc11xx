#include "LPC11xx.h"
#include "global.h"
#include "types.h"

int64_t unixTimeStamp = 0; // UNIX timestamp in milliseconds
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
uint64_t date2UnixTimestamp(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t millisecond) {
	uint64_t time = 0;
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

	return time * 1000 + millisecond;
}

date_t getUnixTimestampDecoded(void) {
	date_t date;
	uint64_t dateRaw = unixTimeStamp / 1000;

	date.millisecond = unixTimeStamp % 1000;
	date.year = 1970;
	while(true)
	{
		uint32_t secondsInThisYear = date.year % 4 ? 31536000 : 31622400;
		if(dateRaw >= secondsInThisYear) {
			dateRaw -= secondsInThisYear;
			date.year++;
		} else {
			break;
		}
	}

	for(date.month=1; (date.year%4 ? nonLeapYear[date.month] : leapYear[date.month])*86400<dateRaw; date.month++);
	dateRaw -= (date.year%4 ? nonLeapYear[date.month-1] : leapYear[date.month-1])*86400;

	date.day    = (dateRaw / 86400) + 1;
	date.hour   = (dateRaw % 86400) / 3600;
	date.minute = (dateRaw % 3600) / 60;
	date.second = dateRaw % 60;

	return date;
}

/**
 * @brief Returns unix timestamp
 * @return time UNIX time in ms
 */
uint64_t getUnixTimestamp(void) {
	return unixTimeStamp;
}

/**
 * @brief Set unix timestamp
 * @param time UNIX time in ms
 */
void setUnixTimeStamp(uint64_t time) {
	unixTimeStamp = time;
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
