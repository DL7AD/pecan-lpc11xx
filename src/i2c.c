#ifdef USE_I2C

#include "global.h"
#include "target.h"

#include "i2c.h"
#include "config.h"

/****** exportierte Objekte **************************************************/

/****** lokale Funktionen ****************************************************/
#define I2C_CONFLAG_AA				(1 << 2)			// Assert acknowledge flag
#define I2C_CONFLAG_SI				(1 << 3)			// I2C interrupt flag
#define I2C_CONFLAG_STO				(1 << 4)			// STOP flag
#define I2C_CONFLAG_STA				(1 << 5)			// START flag
#define I2C_CONFLAG_I2EN			(1 << 6)			// I2C interface enable


#define I2C_CONSET					LPC_I2C->CONSET
#define I2C_STAT					LPC_I2C->STAT
#define I2C_DAT						LPC_I2C->DAT
#define I2C_SCLL					LPC_I2C->SCLL
#define I2C_SCLH					LPC_I2C->SCLH
#define I2C_CONCLR					LPC_I2C->CONCLR
#define I2C_IRQ						I2C_IRQn


#define I2C_PINCONFIG()		{ \
	LPC_IOCON->PIO0_4 = (0 <<  8) | (1 <<  0); 	/* Standard mode/ Fast-mode I2C, SCL */ \
	LPC_IOCON->PIO0_5 = (0 <<  8) | (1 <<  0); 	/* Standard mode/ Fast-mode I2C, SDA */ \
	LPC_IOCON->I2C_PIO_PULL_VCC = 0x30; \
	I2C_GPIO_PULL_VCC->DIR	|= I2C_PIN_PULL_VCC; \
	I2C_GPIO_PULL_VCC->DATA	|= I2C_PIN_PULL_VCC; \
}

#define I2C_CONFIG()				{ \
                NVIC_DisableIRQ(I2C_IRQ); \
                NVIC_ClearPendingIRQ(I2C_IRQ); \
                LPC_SYSCON->PRESETCTRL	  |= (1 <<  1);	/* I²C Reset */ \
                LPC_SYSCON->SYSAHBCLKCTRL |= (1 <<  5);	/* Takt EIN */ \
                I2C_CONCLR = I2C_CONFLAG_AA | I2C_CONFLAG_SI | I2C_CONFLAG_STA | I2C_CONFLAG_I2EN;	/* Clear Flags */ \
				I2C_SCLL = getFcclk() / I2C_FREQUENCY / 2;	/* Low Duty Cycle */ \
				I2C_SCLH = I2C_SCLL;					/* 50% */ \
                NVIC_SetPriority(I2C_IRQ, INT_PRIORITY_I2C);\
                NVIC_EnableIRQ(I2C_IRQ); \
                I2C_CONSET = I2C_CONFLAG_I2EN;			/* Enable */ \
        }
#define I2C_SET_FREQUENCY(f)	{ \
				NVIC_DisableIRQ(I2C_IRQ); \
				NVIC_ClearPendingIRQ(I2C_IRQ); \
				I2C_SCLL = getFcclk() / (f) / 2;	/* Low Duty Cycle */ \
				I2C_SCLH = I2C_SCLL;					/* 50% */ \
				NVIC_EnableIRQ(I2C_IRQ); \
		}

static void	I2C_SetFrequency(uint32_t Frequency);
/****** lokale Variablen *****************************************************/
volatile struct {
	uint8_t	DeviceID;									// Geräte-ID (Bit 0) wird entsprechend von IsReadFromSlave behandelt
	bool	IsReadFromSlave;							// Lesen vom Slave?
	uint8_t	DataAddress;								// Adresse, ab der gelesen / geschrieben werden soll
	uint8_t	DataCount;									// Anzahl Bytes die gelesen / geschrieben werden sollen
	uint8_t*	pData;										// Zeiger auf den Datenbereich
	int32_t*	pError;										// Zeiger auf Rückgabewert (siehe I2C_ERROR_xxx)
	uint16_t	I2CStat;									// Status der I²C-State-Machine beim Beenden

	bool	IsProcessCall;								// Process Call durchführen
	uint16_t	CmdWord;									// Command Word (CommandCode wird in DataAddress geführt)
	uint16_t	AnswerWord;									// Antwort Word
}I2CInfo;

/** @addtogroup ISRHandler **************************************************/
/*@{*/
/**************************************************************************//**
 * @brief		Handler für für I²C
 */
void	On_I2C(void)
{
	switch(I2C_STAT)									// je nach Zustand der I²C-State-Machine
	{
	///////////////////////////////////////////////////////////////////////////
	default:											// im Allgemeinen ist ein Fehler aufgetreten
		*I2CInfo.pError = I2C_ERROR_UNKNOWN;			// Fehlercode ablegen
		break;
	// Slave-Error ////////////////////////////////////////////////////////////
	case 0x20:											// SLA+W has been transmitted; NOT ACK has been received
	case 0x30:											// Data byte in I2DAT has been transmitted; NOT ACK has been received
	case 0x48:											// SLA+R has been transmitted; NOT-ACK has been received
		*I2CInfo.pError = I2C_ERROR_SLAVE;				// Fehlercode ablegen
		break;
	// Master-Error ///////////////////////////////////////////////////////////
	case 0x38:											// Arbitration lost in SLA+R/W or Data bytes
		*I2CInfo.pError = I2C_ERROR_MASTER;				// Fehlercode ablegen
		break;
	// A START condition has been transmitted /////////////////////////////////
	case 0x08:
		I2C_CONCLR = I2C_CONFLAG_STA;					// Start-Flag löschen
		I2C_DAT = I2CInfo.DeviceID & 0xFE;				// Write-ID Adresse senden
		break;
	// A repeated START condition has been transmitted ////////////////////////
	case 0x10:
		I2C_CONCLR = I2C_CONFLAG_STA;					// Start-Flag löschen
		// ID-Senden, RW-Bit setzen/löschen
		I2C_DAT = (I2CInfo.IsReadFromSlave) ? (I2CInfo.DeviceID | 0x01) : (I2CInfo.DeviceID & 0xFE);
		break;
	// SLA+W has been transmitted; ACK has been received //////////////////////
	case 0x18:
		I2C_DAT = I2CInfo.DataAddress;					// Data-Adresse senden
		if (I2CInfo.IsProcessCall)
		{	// SMBus Process Call
			I2CInfo.IsReadFromSlave = false;
			I2CInfo.pData = (uint8_t*)&I2CInfo.CmdWord;
			I2CInfo.DataCount = 2;
		}
		break;
	// Data byte in I2DAT has been transmitted; ACK has been received /////////
	case 0x28:
		if (I2CInfo.IsReadFromSlave)
		{
			I2C_CONSET = I2C_CONFLAG_STA;				// Repeated-Start senden
		}
		else
		{
			if (I2CInfo.DataCount)						// Sind noch Daten zum Senden da?
			{
				I2C_DAT = *I2CInfo.pData++;				// Datenbyte senden
				I2CInfo.DataCount--;
			}
			else
			{
				if (I2CInfo.IsProcessCall)
				{	// SMBus Process Call
					I2CInfo.IsReadFromSlave = true;
					I2CInfo.pData = (uint8_t*)&I2CInfo.AnswerWord;
					I2CInfo.DataCount = 2;
					I2C_CONSET = I2C_CONFLAG_STA;		// Repeated-Start senden
				}
				else
				{
					*I2CInfo.pError = I2C_ERROR_NONE;	// Fertigmeldung ablegen
				}
			}
		}
		break;
	// Data byte has been received; ACK has been returned /////////////////////
	case 0x50:
		*I2CInfo.pData++ = I2C_DAT;						// Datenbyte merken
		// !!! Durchfallen
	// SLA+R has been transmitted; ACK has been received //////////////////////
	case 0x40:
		if (I2CInfo.DataCount)							// Sind noch Daten zu Empfangen?
		{
			I2C_CONSET = I2C_CONFLAG_AA;				// ACK senden
			I2CInfo.DataCount--;
		}
		if (!I2CInfo.DataCount)							// Sind danach keine Daten mehr zu Empfangen?
		{
			I2C_CONCLR = I2C_CONFLAG_AA;				// NACK senden
		}
		break;
	// Data byte has been received; NOT ACK has been returned /////////////////
	case 0x58:
		*I2CInfo.pData = I2C_DAT;						// Datenbyte merken
		*I2CInfo.pError = I2C_ERROR_NONE;				// Fertigmeldung ablegen
		break;
	} /* Ende switch(I2C_STAT) */

	if (*I2CInfo.pError != I2C_ERROR_WAITING)			// Ende ?
	{
		I2CInfo.I2CStat = I2C_STAT;						// Status merken
		I2C_CONSET = I2C_CONFLAG_STO;					// Stop senden
	}

	I2C_CONCLR = I2C_CONFLAG_SI;						// clear SI starts action
}
/*@}*/ /* end of group ISRHandler */
/**************************************************************************//**
 * @brief		diverse Initialisierungen
 *
 * @return      !=0 Alles OK
 */
bool	I2C_Init(void)
{
bool result = true;										// Optimist

	I2C_PINCONFIG();
	I2C_CONFIG();

    return result;
}
/**************************************************************************//**
 * @brief		Stellt I²C-Frequenz ein
 * @param		Frequency in Hz
 */
void	I2C_SetFrequency(uint32_t Frequency)
{
	I2C_SET_FREQUENCY(Frequency);
}
/**************************************************************************//**
 * @brief		Liest eine Anzahl Bytes von einem Slave
 * 				(komplette Kommunikation, zu Beginn Übertragung der Leseadresse)
 * @param		DeviceID	- Lese-ID des Slaves (Schreib- Lesebit wird in der ISR verwaltet, hier egal)
 * @param		Address		- ab dieser Adresse im Slave wird gelesen
 * @param		Count		- und zwar soviele Bytes
 * @param[out]	pData		- nach hier
 * @param[out]	pError		- Speicher für den Fehlercode (I2C_ERROR_xxx), darf nicht NULL sein
 * @param		Retries		- Wiederholungen im Falle von Fehlern
 */
void	I2C_Read(uint8_t DeviceID, uint8_t Address, uint8_t Count, uint8_t* pData, int32_t* pError, uint32_t Retries)
{
	do
	{
		I2C_SetFrequency(I2C_FREQUENCY);
		// Struktur füllen
		I2CInfo.DeviceID		= DeviceID;				// Device-ID
		I2CInfo.IsProcessCall	= false;				// kein ProcessCall
		I2CInfo.IsReadFromSlave	= true;					// Lesen vom Slave?
		I2CInfo.DataAddress		= Address;				// Adresse, ab der gelesen werden soll
		I2CInfo.DataCount		= Count;				// Anzahl Bytes die gelesen werden sollen
		I2CInfo.pData			= pData;				// Zeiger auf den Datenbereich
		I2CInfo.pError			= pError;				// Zeiger auf Rückgabewert (siehe I2C_ERROR_xxx)
		*I2CInfo.pError 		= I2C_ERROR_WAITING;
		// Kommunikation auslösen
		I2C_CONSET = I2C_CONFLAG_I2EN;					// Enable I2C
		I2C_CONSET = I2C_CONFLAG_STO;					// STOP
		I2C_CONCLR = I2C_CONFLAG_AA;					// ACK löschen
		*I2CInfo.pError = I2C_ERROR_WAITING;
		I2C_CONSET = I2C_CONFLAG_STA;					// START setzen
		I2C_CONCLR = I2C_CONFLAG_SI;					// clear SI starts action

		uint8_t t = 0;
		while (		(*I2CInfo.pError != I2C_ERROR_NONE)
			  &&	(t++ < I2C_TIMEOUT)	)
		{
			__WFI();
		}
		if ((*I2CInfo.pError != I2C_ERROR_NONE))
		{
			delay(I2C_RETRYDELAY);
		}
	}
	while (	((*I2CInfo.pError) != I2C_ERROR_NONE) && (Retries--));
}
/**************************************************************************//**
 * @brief		Schreibt eine Anzahl Bytes in einen Slave
 * 				(komplette Kommunikation, zu Beginn Übertragung der Screibadresse)
 * @param		DeviceID	- Schreib-ID des Slaves (Schreib- Lesebit wird in der ISR verwaltet, hier egal)
 * @param		Address		- ab dieser Adresse im Slave wird geschrieben
 * @param		Count		- und zwar soviele Bytes
 * @param		pData		- von hier
 * @param[out]	pError		- Speicher für den Fehlercode (I2C_ERROR_xxx), darf nicht NULL sein
 * @param		Retries		- Wiederholungen im Falle von Fehlern
 */
void	I2C_Write(uint8_t DeviceID, uint8_t Address, uint8_t Count, uint8_t* pData, int32_t* pError, uint32_t Retries)
{
	do
	{
		I2C_SetFrequency(I2C_FREQUENCY);
		// Struktur füllen
		I2CInfo.DeviceID		= DeviceID;				// Device-ID
		I2CInfo.IsProcessCall	= false;				// kein ProcessCall
		I2CInfo.IsReadFromSlave	= false;				// Lesen vom Slave?
		I2CInfo.DataAddress		= Address;				// Adresse, ab der geschrieben werden soll
		I2CInfo.DataCount		= Count;				// Anzahl Bytes die geschrieben werden sollen
		I2CInfo.pData			= pData;				// Zeiger auf den Datenbereich
		I2CInfo.pError			= pError;				// Zeiger auf Rückgabewert (siehe I2C_ERROR_xxx)
		*I2CInfo.pError=I2C_ERROR_WAITING;
		// Kommunikation auslösen
		I2C_CONSET = I2C_CONFLAG_I2EN;					// Enable I2C
		I2C_CONSET = I2C_CONFLAG_STO;					// STOP
		I2C_CONCLR = I2C_CONFLAG_AA;					// ACK löschen
		*I2CInfo.pError = I2C_ERROR_WAITING;
		I2C_CONSET = I2C_CONFLAG_STA;					// START setzen
		I2C_CONCLR = I2C_CONFLAG_SI;					// clear SI starts action

		uint8_t t = 0;
		while (		(*I2CInfo.pError != I2C_ERROR_NONE)
			  &&	(t++ < I2C_TIMEOUT)	)
		{
			__WFI();
		}
		if ((*I2CInfo.pError != I2C_ERROR_NONE))
		{
			delay(I2C_RETRYDELAY);
		}
	}
	while (	((*I2CInfo.pError) != I2C_ERROR_NONE) && (Retries--));
}
/**************************************************************************//**
 * @brief		Führt einen SMBus Process Call durch
 *
 * @param		DeviceID	- Schreib-ID des Slaves (Schreib- Lesebit wird in der ISR verwaltet, hier egal)
 * @param		CmdCode		- Command Code
 * @param		CmdWord		- Command Word
 * @param[out]	AnswerWord	- Antwort Word
 * @param[out]	pError		- Speicher für den Fehlercode (I2C_ERROR_xxx), darf nicht NULL sein
 * @param		Retries		- Wiederholungen im Falle von Fehlern
 */
void	I2C_Process(uint8_t DeviceID, uint8_t CmdCode, uint16_t CmdWord, uint16_t *AnswerWord, int32_t* pError, uint32_t Retries)
{
	do
	{
		I2C_SetFrequency(I2C_FREQUENCY);
		// Struktur füllen
		I2CInfo.DeviceID		= DeviceID;				// Device-ID
		I2CInfo.IsProcessCall	= true;					// ProcessCall
		I2CInfo.DataAddress		= CmdCode;				// Adresse, ab der geschrieben werden soll
		I2CInfo.CmdWord			= CmdWord;				// Command Word
		I2CInfo.pError			= pError;				// Zeiger auf Rückgabewert (siehe I2C_ERROR_xxx)
		*I2CInfo.pError=I2C_ERROR_WAITING;
		// Kommunikation auslösen
		I2C_CONSET = I2C_CONFLAG_I2EN;					// Enable I2C
		I2C_CONSET = I2C_CONFLAG_STO;					// STOP
		I2C_CONCLR = I2C_CONFLAG_AA;					// ACK löschen
		*I2CInfo.pError = I2C_ERROR_WAITING;
		I2C_CONSET = I2C_CONFLAG_STA;					// START setzen
		I2C_CONCLR = I2C_CONFLAG_SI;					// clear SI starts action

		uint8_t t = 0;
		while (		(*I2CInfo.pError != I2C_ERROR_NONE)
			  &&	(t++ < I2C_TIMEOUT)	)
		{
			__WFI();
		}
		if ((*I2CInfo.pError == I2C_ERROR_NONE))		// Alles OK ?
		{
			*AnswerWord = I2CInfo.AnswerWord;			// Anwort kopieren
		}
		else											// sonst
		{
			delay(I2C_RETRYDELAY);					// nächste Anfrage verzögern
		}
	}
	while (	((*I2CInfo.pError) != I2C_ERROR_NONE) && (Retries--));
}

void I2C_DeInit(void) {

}

#endif
