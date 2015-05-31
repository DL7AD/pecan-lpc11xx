#include "LPC11xx.h"
#include "global.h"
#include "types.h"

uint64_t unixTimeStamp = 0;

/**
 * @brief Interrupt routine for SystemTick
 * This method should be called every 1ms.
 */
void SysTick_Handler(void) {
	unixTimeStamp++;
}

/**
 * @brief Set unix timestamp
 * @param time UNIX time in ms
 */
void setUnixTimestamp(uint64_t time) {
	unixTimeStamp = time;
}

/**
 * @brief Returns unix timestamp
 * @return time UNIX time in ms
 */
uint64_t getUnixTimestamp(void) {
	return unixTimeStamp;
}

/**
 * Executes a delay
 */
void delay(uint32_t ms) {
	uint32_t i = 0;
	while(i++ < ms)
		__WFI();
}
