#include "config.h"
#include "adc.h"
#include "global.h"

void ADC_Init(void) {
	// Enable LDO for reference voltage (only needed if VCC voltage unknown)
	#if ADC_REF == REF_VCC1V8_LDO
	LPC_IOCON->LDO_PIO_EN = 0x30;
	LDO_GPIO_EN->DATA &= ~LDO_PIN_EN;
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

	LPC_ADC->CR = 0x0B01; // Configure ADC block to max. accuracy
}

void ADC_DeInit(void) {
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
	return 1811 * 1024 / ref;			// Calculate reference voltage (VCC which is also battery voltage)
	#elif ADC_REF == REF_VCC
	uint32_t adc = getADC(ADC_AD_BATT);
	return (adc * REF_MV) >> 10;		// Return battery voltage
	#endif
}

#ifdef SOLAR_AVAIL

/**
 * Measures solar voltage in millivolts
 * @return solar voltage
 */
uint32_t getSolarMV(void)
{
	#if ADC_REF == REF_VCC1V8_LDO
	uint32_t adc = getADC(ADC_AD_SOLAR);
	uint32_t batt = getBatteryMV();
	return (adc * batt) / 1024;
	#elif ADC_REF == REF_VCC
	return getADC(ADC_AD_SOLAR) / REF_MV;
	#endif
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
