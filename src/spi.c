/**
 * SPI library. The original author does not want to be named at this place.
 * @author Sven Steudte
 */

#include "spi.h"
#include "target.h"
#include "LPC11xx.h"
#include "config.h"

SSP_Info_t volatile SSPStruct;

/**
 * Handler for SSP interrupt
 */
void On_SSP(void) {
	uint16_t Data;

	SSP_CLRPENDING_IRQ();

	// Lesen bis Ende Rx-FIFO
	while (!SSP_ISRXFIFOEMPTY()) {
		Data = SSP_PORT->DR;
		if (SSPStruct.RxCount) {
			SSPStruct.RxCount--;
			if (SSPStruct.pRxData) {
				*SSPStruct.pRxData++ = Data;
			}
		}
	}

	if (!SSPStruct.RxCount) { // Everything finished?
		SSP_CS_SET(false);
		SSP_DISABLE_IRQ();
	}


	if ((!SSPStruct.TxCount) && (!SSP_ISBUSY())) { // Finished writing?
		SSP_DISABLE_IRQ_TXFIFO(); // Disable IRQ due to empty tx fifo
	}

	// Writing until tx fifo is full; max. 8 bytes, so rx fifo does not overflow
	uint32_t i = 8;
	while ((!SSP_ISTXFIFOFULL()) && (SSPStruct.TxCount) && (i--)) {
		if (SSPStruct.pTxData)
			SSP_PORT->DR = *SSPStruct.pTxData++;
		else
			SSP_PORT->DR = 0x00;
		SSPStruct.TxCount--;
	}
	SSP_CLEAR_IRQFLAG();
}

void SSP_Init(void) {
	// Initialize SSP
	LPC_IOCON->SSP_PIO_RADIO_SS		= 0x30;						// Slave select pin as GPIO, Pull-Up, Hysterese ON
	SSP_GPIO_RADIO_SS->DIR		   |= SSP_PIN_RADIO_SS;			// Slave select pin as output
	SSP_GPIO_RADIO_SS->DATA		   |= SSP_PIN_RADIO_SS;			// Slave select pin deselect

	// Configure SPP specific pins
	if(SSP_PORT == LPC_SSP0) {

		if(&LPC_IOCON->SSP_PIO_SCK == &LPC_IOCON->SWCLK_PIO0_10) {
			LPC_IOCON->SCK_LOC			= 0x0;					// Selects SCK0 location (0x0 for PIO0_10)
			LPC_IOCON->SWCLK_PIO0_10	= 0x32;					// PIO0_10 as SCK SSP, Pull-Up, Hysterese ON
		} else if(&LPC_IOCON->SSP_PIO_SCK == &LPC_IOCON->PIO2_11) {
			LPC_IOCON->SCK_LOC			= 0x1;					// Selects SCK0 location (0x1 for PIO2_11)
			LPC_IOCON->PIO2_11			= 0x31;					// PIO2_11 as SCK SSP, Pull-Up, Hysterese ON
		} else { // PIO0_6
			LPC_IOCON->SCK_LOC			= 0x2;					// Selects SCK0 location (0x2 for PIO0_6)
			LPC_IOCON->PIO0_6			= 0x32;					// PIO0_6 as SCK SSP, Pull-Up, Hysterese ON
		}

		LPC_IOCON->PIO0_8				= 0x31;					// PIO0_8 as MISO SSP, Pull-Up, Hysterese ON
		LPC_IOCON->PIO0_9				= 0x31;					// PIO0_9 as MOSI SSP, Pull-Up, Hysterese ON

	} else { // SSP1

		if(&LPC_IOCON->SSP_PIO_MOSI == &LPC_IOCON->PIO2_3) {
			LPC_IOCON->MOSI1_LOC		= 0x0;					// Selects MOSI1 location (0x0 for PIO2_3)
			LPC_IOCON->PIO2_3			= 0x32;					// PIO2_3 as MOSI SSP, Pull-Up, Hysterese ON
		} else { // PIO1_9
			LPC_IOCON->MOSI1_LOC		= 0x1;					// Selects MOSI1 location (0x1 for PIO1_9)
			LPC_IOCON->PIO1_9			= 0x32;					// PIO1_9 as MOSI SSP, Pull-Up, Hysterese ON
		}

		if(&LPC_IOCON->SSP_PIO_MISO == &LPC_IOCON->PIO2_2) {
			LPC_IOCON->MISO1_LOC		= 0x0;					// Selects MISO1 location (0x0 for PIO2_2)
			LPC_IOCON->PIO2_2			= 0x32;					// PIO2_2 as MISO SSP, Pull-Up, Hysterese ON
		} else { // PIO1_10
			LPC_IOCON->MISO1_LOC		= 0x1;					// Selects MISO1 location (0x1 for PIO1_10)
			LPC_IOCON->PIO1_10			= 0x33;					// PIO1_10 as MISO SSP, Pull-Up, Hysterese ON
		}

		if(&LPC_IOCON->SSP_PIO_SCK == &LPC_IOCON->PIO2_1) {
			LPC_IOCON->SCK1_LOC			= 0x0;					// Selects SCK1 location (0x0 for PIO2_1)
			LPC_IOCON->PIO2_1			= 0x31;					// PIO2_1 as SCK SSP, Pull-Up, Hysterese ON
		} else { // PIO3_2
			LPC_IOCON->SCK1_LOC			= 0x1;					// Selects SCK1 location (0x1 for PIO3_2)
			LPC_IOCON->PIO3_2			= 0x33;					// PIO3_2 as SCK SSP, Pull-Up, Hysterese ON
		}

	}

	NVIC_DisableIRQ(SSP_IRQn);
	if(SSP_PORT == LPC_SSP0) {
		LPC_SYSCON->SYSAHBCLKCTRL	|= (1 << 11);				// Enables clock for SSP0
		LPC_SYSCON->SSP0CLKDIV		 = 0x5;					// Clocking divider (6 -> 8MHz @ Fcclk 48MHz)
		LPC_SYSCON->PRESETCTRL		&= ~(1 << 0);				// SSP_RST_N assert
		LPC_SYSCON->PRESETCTRL		|= (1 <<  0);				// SSP_RST_N de-assert
	} else { // SSP1
		LPC_SYSCON->SYSAHBCLKCTRL	|= (1 << 18);				// Enables clock for SSP1
		LPC_SYSCON->SSP1CLKDIV		 = 0x5;					// Clocking divider (6 -> 8MHz @ Fcclk 48MHz)
		LPC_SYSCON->PRESETCTRL		&= ~(1 <<  2);				// SSP_RST_N assert
		LPC_SYSCON->PRESETCTRL		|= (1 <<  2);				// SSP_RST_N de-assert
	}

	SSP_PORT->CR1					= 0x00;						// Disable SSP,	0000 SOD: 0, MS: 0, SSE: 0, LBM: 0
	SSP_PORT->CPSR					= 24;						// Prescaler = /24 = 500kHz
	SSP_PORT->CR0					= 0x0007;					// 00000000 0000 0111	Serial Clock Rate: 0, CPHA: 0, CPOL: 0, FRF: 00, DSS: 0111
	SSP_PORT->IMSC					= 0x0F;						// alle Interrupts
	NVIC_SetPriority(SSP_IRQn, INT_PRIORITY_SSP);
	NVIC_EnableIRQ(SSP_IRQn);
	SSP_PORT->CR1					= 0x02;						// Enable SSP,	0010 SOD: 0, MS: 0, SSE: 1, LBM: 0
}

void SSP_DeInit() {
	// TODO
}

/**
 * Blocks transfer until transmitting is completed or reached timeout
 */
void SSP_WaitTransferComplete(void) {
	while(SSPStruct.RxCount);
		__WFI();
	SSP_CS_SET(false);
	SSP_DISABLE_IRQ();
}
