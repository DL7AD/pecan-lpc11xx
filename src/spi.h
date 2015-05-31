/**
 * SPI library. The original author does not want to be named at this place.
 * @author Sven Steudte
 */

#ifndef __SSP__H__
#define __SSP__H__

#include "LPC11xx.h"
#include "config.h"
#include "defines.h"

#define SSP_CS_SET(Select)			{ \
									if (Select) \
											SSP_GPIO_RADIO_SS->DATA &= ~SSP_PIN_RADIO_SS; \
										else \
											SSP_GPIO_RADIO_SS->DATA |= SSP_PIN_RADIO_SS; \
									}

#define	SSP_DISABLE_IRQ()			NVIC_DisableIRQ(SSP_IRQn)
#define SSP_DISABLE_IRQ_TXFIFO()	{SSP_PORT->IMSC	= 0x07;}
#define	SSP_ENABLE_IRQ()			{SSP_PORT->IMSC	= 0x0F; NVIC_EnableIRQ(SSP_IRQn);}
#define SSP_SETPENDING_IRQ()		{NVIC_SetPendingIRQ(SSP_IRQn);}
#define SSP_START_IRQ()				{ \
										SSP_CS_SET(true); \
										SSP_SETPENDING_IRQ(); \
										SSP_ENABLE_IRQ(); \
									}
#define SSP_CLRPENDING_IRQ()		{NVIC_ClearPendingIRQ(SSP_IRQn);}
#define	SSP_CLEAR_IRQFLAG()			{SSP_PORT->ICR	= 0x03;}

#define SSP_ISTXFIFOFULL()			((SSP_PORT->SR & 0x02) ? false : true)
#define SSP_ISRXFIFOEMPTY()			((SSP_PORT->SR & 0x04) ? false : true)
#define SSP_ISBUSY()				((SSP_PORT->SR & 0x10) ? true : false)

typedef struct SSP_Info {
	uint32_t	TxCount;								// Gesamtanzahl Sendeworte
	uint8_t		*pTxData;								// SendeDaten
	uint32_t	RxCount;								// Gesamtanzahl Empfangsworte
	uint8_t		*pRxData;								// Empfangsdaten
} SSP_Info_t;

extern SSP_Info_t volatile SSPStruct;

void SSP_Init(void);
void SSP_DeInit();
void On_SSP(void);
void SSP_WaitTransferComplete(void);

#endif
