#include "LPC11xx.h"
#include "config.h"

#ifdef VCXO_POWERED_BY_LDO

#define LDO_INIT() { \
	LPC_IOCON->LDO_PIO_EN = 0x30; \
}
#define LDO_EN(Select) { \
	if (Select) { \
		LPC_IOCON->ADC_PIO_REF = 0x30; \
		ADC_GPIO_REF->DIR |= ADC_PIN_REF; \
		ADC_GPIO_REF->DATA &= ~ADC_PIN_REF; \
		delay(50); \
		LDO_GPIO_EN->DATA &= ~LDO_PIN_EN; \
	} else { \
		LDO_GPIO_EN->DATA |= LDO_PIN_EN; \
	} \
}

#else

#define LDO_INIT		{}
#define LDO_EN(Select)	{}

#endif
