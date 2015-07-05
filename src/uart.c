#include "fifo.h"
#include "target.h"
#include "uart.h"
#include "config.h"

#define UART_CLEAR_IR() 			{(LPC_UART->IIR >> 1) & 0x07;}
#define UART_CLR_RXFIFO()			{LPC_UART->FCR |= (1 << 1);}
#define UART_TX()					{NVIC_SetPendingIRQ(UART_IRQn);}

#define UART_TX_LENGTH				16

#define UART_ISNOT_TXFIFOFULL()		((LPC_UART->LSR & 0x20) ? true : false)	// Transmitter holding register empty
#define UART_START_TX()				{NVIC_SetPendingIRQ(UART_IRQn);}

static uint8_t UART_Tx_FIFOBuffer[UART_DATA_MAXLENGTH];	// Buffer für Transmit-FIFO
static uint8_t UART_Rx_FIFOBuffer[UART_DATA_MAXLENGTH];	// Buffer für Receive-FIFO
static T_ByteFIFO UART_TxFIFO;							// Transmit-FIFO
static T_ByteFIFO UART_RxFIFO;							// Receive-FIFO
static uint32_t crit = 0;

/**
 * Handler für UART Interrupt
 */
void On_UART(void) {
	uint8_t Data;
	uint8_t IIRValue;
	uint8_t LSRValue;

	IIRValue = LPC_UART->IIR;
	IIRValue = (IIRValue >> 1) & 0x07;

	switch (IIRValue)
	{
	// Receive Line Status ///////////////////////////////////////////////////
	case 0x03:
		LSRValue = LPC_UART->LSR;							// LSR lesen
		Data = LPC_UART->RBR;								// Dummy Read
		UART_CLR_RXFIFO();								// Rx-FIFO löschen
		break;

	// Character Timeout /////////////////////////////////////////////////////
	case 0x06:
	// Receive Data Available ////////////////////////////////////////////////
	case 0x02:
		do
		{
			LSRValue = LPC_UART->LSR;						// Line Status Register
			Data = LPC_UART->RBR & 0xFF;						// Zeichen lesen
			if (	((LSRValue & 0x8F) == 0x01)	)		// Kein Fehler
			{
				FIFO_Put(&UART_RxFIFO, Data);
			}
		}
		while (LPC_UART->LSR & 0x01); 						// ganzen FIFO lesen
		break;

	// FIFO leer //////////////////////////////////////////////////////////////
	case 0x01: // THRE
		{
			uint32_t Count = UART_TX_LENGTH;
			while ((Count--) && FIFO_Get(&UART_TxFIFO, &Data))
				LPC_UART->THR = Data;
		}
		break;
	//////////////////////////////////////////////////////////////////////////
	default:
		{
			while (	(UART_ISNOT_TXFIFOFULL()) && (FIFO_Get(&UART_TxFIFO, &Data))	)
				LPC_UART->THR = Data;
		}
		break;
	}
}

bool UART_Init(uint32_t baud) {
	bool result = true;

	// Configure RXD pin
	if(&LPC_IOCON->UART_PIO_RXD == &LPC_IOCON->PIO1_6) {
		LPC_IOCON->RXD_LOC	= 0x0;
		LPC_IOCON->PIO1_6	= 0x31;		// PIO1_6 as UART RXD, Pull-Up, Hysterese ON
	} else if(&LPC_IOCON->UART_PIO_RXD == &LPC_IOCON->PIO2_7) {
		LPC_IOCON->RXD_LOC	= 0x1;
		LPC_IOCON->PIO2_7	= 0x32;		// PIO2_7 as UART RXD, Pull-Up, Hysterese ON
	} else if(&LPC_IOCON->UART_PIO_RXD == &LPC_IOCON->PIO3_1) {
		LPC_IOCON->RXD_LOC	= 0x2;
		LPC_IOCON->PIO3_1	= 0x33;		// PIO3_1 as UART RXD, Pull-Up, Hysterese ON
	} else { // PIO3_4
		LPC_IOCON->RXD_LOC	= 0x3;
		LPC_IOCON->PIO3_4	= 0x32;		// PIO3_4 as UART RXD, Pull-Up, Hysterese ON
	}

	// Configure TXD pin
	if(&LPC_IOCON->UART_PIO_TXD == &LPC_IOCON->PIO1_7) {
		LPC_IOCON->PIO1_7	= 0x31;		// PIO1_7 as UART TXD, Pull-Up, Hysterese ON
	} else if(&LPC_IOCON->UART_PIO_TXD == &LPC_IOCON->PIO3_0) {
		LPC_IOCON->PIO3_0	= 0x33;		// PIO3_0 as UART TXD, Pull-Up, Hysterese ON
	} else if(&LPC_IOCON->UART_PIO_TXD == &LPC_IOCON->PIO3_5) {
		LPC_IOCON->PIO3_5	= 0x32;		// PIO3_5 as UART TXD, Pull-Up, Hysterese ON
	} else { // PIO2_8
		LPC_IOCON->PIO2_8	= 0x32;		// PIO2_8 as UART TXD, Pull-Up, Hysterese ON
	}

	// Initialize fifo
	if (!FIFO_Init(&UART_TxFIFO, sizeof(UART_Tx_FIFOBuffer), UART_Tx_FIFOBuffer))
		result = false;

	if (!FIFO_Init(&UART_RxFIFO, sizeof(UART_Rx_FIFOBuffer), UART_Rx_FIFOBuffer))
		result = false;

	// Configure UART clocking
	if(result) {

		NVIC_DisableIRQ(UART_IRQn);
		LPC_SYSCON->SYSAHBCLKCTRL  |= (1 << 12);	// Enable clock for UART
		LPC_SYSCON->UARTCLKDIV		= 0x1;			// Set UART prescaler

		LPC_UART->IER = 0x00;						// Deactivate IRQs
		LPC_UART->LCR = 0x83;						// 8 bits, no Parity, 1 Stop bit

		if(baud == 50) {
			LPC_UART->FDR = ((UART_MULVAL_50 & 0x0F) << 4) | (UART_DIVADDVAL_50 & 0x0F);
			LPC_UART->DLM = UART_DIVISOR_50 / 0x100;
			LPC_UART->DLL = UART_DIVISOR_50 % 0x100;
		} else if(baud == 150) {
			LPC_UART->FDR = ((UART_MULVAL_150 & 0x0F) << 4) | (UART_DIVADDVAL_150 & 0x0F);
			LPC_UART->DLM = UART_DIVISOR_150 / 0x100;
			LPC_UART->DLL = UART_DIVISOR_150 % 0x100;
		} else if(baud == 300) {
			LPC_UART->FDR = ((UART_MULVAL_300 & 0x0F) << 4) | (UART_DIVADDVAL_300 & 0x0F);
			LPC_UART->DLM = UART_DIVISOR_300 / 0x100;
			LPC_UART->DLL = UART_DIVISOR_300 % 0x100;
		} else if(baud == 600) {
			LPC_UART->FDR = ((UART_MULVAL_600 & 0x0F) << 4) | (UART_DIVADDVAL_600 & 0x0F);
			LPC_UART->DLM = UART_DIVISOR_600 / 0x100;
			LPC_UART->DLL = UART_DIVISOR_600 % 0x100;
		} else if(baud == 1200) {
			LPC_UART->FDR = ((UART_MULVAL_1200 & 0x0F) << 4) | (UART_DIVADDVAL_1200 & 0x0F);
			LPC_UART->DLM = UART_DIVISOR_1200 / 0x100;
			LPC_UART->DLL = UART_DIVISOR_1200 % 0x100;
		} else if(baud == 9600) {
			LPC_UART->FDR = ((UART_MULVAL_9600 & 0x0F) << 4) | (UART_DIVADDVAL_9600 & 0x0F);
			LPC_UART->DLM = UART_DIVISOR_9600 / 0x100;
			LPC_UART->DLL = UART_DIVISOR_9600 % 0x100;
		}

		LPC_UART->LCR = 0x03;						// DLAB = 0
		LPC_UART->FCR = 0x07;						// Initialize FIFO
		NVIC_SetPriority(UART_IRQn, INT_PRIORITY_UART);
		NVIC_EnableIRQ(UART_IRQn);

		// Start
		LPC_UART->IER = 0x07;						// Activate IRQs
	}

	return result;
}

void UART_DeInit(void) {
	LPC_SYSCON->SYSAHBCLKCTRL &= ~(1 << 12);		// Disable clock for UART
}

/**
 * @brief		Legt Datenbyte in FIFO, löst Senden aus
 * @param[in]	Data	- Datenbyte
 * @return      true 	- OK, false - FIFO voll
 */
bool UART_TransmitChar(uint8_t Data) {
	bool result;

	EnterCritical();
	result = FIFO_Put(&UART_TxFIFO, Data);
	UART_START_TX();
	ExitCritical();

	__WFI(); // Todo: If this function is called quickly multiple times, the UART is not transmitted the right way, __WFI is just a quick workaround but no real solution

	return result;
}
/**
 * @brief		Holt ein evtl. vorhandenes Zeichen
 * @param[out]	pData
 * @return      false - kein Zeichen vorhanden
 */
bool UART_ReceiveChar(uint8_t* pData)
{
	bool result = false;

	if (FIFO_Available(&UART_RxFIFO))
	{
		EnterCritical();
		result = FIFO_Get(&UART_RxFIFO, pData);
		ExitCritical();
	}

	return result;
}
/**
 * @brief Writes string into FIFO and triggers transmit. The length of the transmitted string will be determined automatically.
 * @param pData String to be transmitted
 * @param Len String length
 * @return Actual length of transmitted bytes
 */
uint32_t UART_TxString(uint8_t* pData, uint32_t Len)
{
	bool result = true;
	uint32_t Count = Len;
	uint8_t	*pB = pData;

	EnterCritical();
		while (	result && (*pB) && (Len))
		{
			if ((result = FIFO_Put(&UART_TxFIFO, *pB++)))
				Len--;
		}
	ExitCritical();

	UART_START_TX();

	return Count - Len;
}

void EnterCritical(void) {
	while(crit);
	crit++;
}

void ExitCritical(void) {
	crit--;
}

