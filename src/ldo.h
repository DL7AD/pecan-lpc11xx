#include "LPC11xx.h"
#include "config.h"

#ifdef VCXO_POWERED_BY_LDO

#define LDO_INIT()	{ \
						LPC_IOCON->LDO_PIO_EN = 0x30; \
					}
#define LDO_EN(Select)	{ \
							if (Select) \
								LDO_GPIO_EN->DATA &= ~LDO_PIN_EN; \
							else \
								LDO_GPIO_EN->DATA |= LDO_PIN_EN; \
						}

#else

#define LDO_INIT		{}
#define LDO_EN(Select)	{}

#endif
