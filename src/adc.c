#include "config.h"
#include "adc.h"
#include "global.h"
#include "ldo.h"

void ADC_Init(void) {
	// Enable LDO for reference voltage (only needed if VCC voltage unknown)
	#if ADC_REF == REF_VCC1V8_LDO
	LDO_INIT();
	LDO_EN(true);
	#endif

	LPC_SYSCON->PDRUNCFG		&= ~(1<<4);		// Power up ADC
	LPC_SYSCON->SYSAHBCLKCTRL	|= (1<<13);		// Enable ADC clock

	// ADC VCC1V8 (for reference voltage & for calculating battery voltage)
	#if ADC_REF == REF_VCC1V8_LDO
	LPC_IOCON->ADC_PIO_REF		 = ADC_AD_REF < AD5 ? 0x02 : 0x01;
	#elif ADC_REF == REF_VCC // Battery voltage
	LPC_IOCON->ADC_PIO_BATT		 = ADC_AD_BATT < AD5 ? 0x02 : 0x01;
	#endif

	// ADC Solar
	#ifdef SOLAR_AVAIL
	LPC_IOCON->ADC_PIO_SOLAR	 = ADC_AD_SOLAR < AD5 ? 0x02 : 0x01;
	#endif

	LPC_ADC->CR = 0xFF00; // Configure ADC block to max. accuracy

	#if ADC_REF == REF_VCC1V8_LDO
	delay(10); // Wait for LDO to establish LDO output voltage
	#endif
}

void ADC_DeInit(void) {
	#if ADC_REF == REF_VCC1V8_LDO
	LDO_EN(false);								// Power down LDO
	#endif
	LPC_SYSCON->PDRUNCFG        |= 1<<4;		// Power down ADC
	LPC_SYSCON->SYSAHBCLKCTRL   &= ~(1<<13);	// Disable ADC clock
}

/**
 * Measures battery voltage in millivolts
 * @return battery voltage
 */
uint32_t getBatteryMV(void)
{
	#if ADC_REF == REF_VCC1V8_LDO
	uint32_t ref = getADC(ADC_AD_REF);
	return 1777 * 1024 / ref;	// Calculate reference voltage (VCC which is also battery voltage)
	#elif ADC_REF == REF_VCC
	return getADC(ADC_AD_BATT) / REF_MV;		// Return battery voltage
	#endif
}

/**
 * Returns battery voltage in a 8bit value
 * 255 = 4080mV
 * @return 8bit battery voltage
 */
uint32_t getBattery8bit(void)
{
	return getBatteryMV() >> 4;
}

#ifdef SOLAR_AVAIL

/**
 * Measures solar voltage in millivolts
 * @return solar voltage
 */
uint32_t getSolarMV(void)
{
	#if ADC_REF == REF_VCC1V8_LDO
	return getADC(ADC_AD_SOLAR) / getBatteryMV();
	#elif ADC_REF == REF_VCC
	return getADC(ADC_AD_SOLAR) / REF_MV;
	#endif
}

/**
 * Returns solar voltage in a 8bit value
 * 255 = 2040mV
 * @return 8bit solar voltage
 */
uint32_t getSolar8bit(void)
{
	return getSolarMV() >> 3;
}

#endif

/**
 * Measures voltage at specific ADx and returns 10bit value (2^10-1 equals LPC reference voltage)
 * @param ad ADx pin
 */
uint16_t getADC(uint8_t ad)
{
	LPC_ADC->CR  = (LPC_ADC->CR & 0xFFF0) | (1 << ad);	// Configure active adc port
	LPC_ADC->CR |= (1 << 24);							// Start conversion
	while((LPC_ADC->DR[ad] < 0x7FFFFFFF));				// Wait for done bit to be toggled
	return ((LPC_ADC->DR[ad] & 0xFFC0) >> 6);			// Cut out 10bit value
}
