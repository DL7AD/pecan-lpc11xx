/**************************************************************************//**
 * @file		../shared/fifo.c
 * @brief		ByteFIFO		für Nicht-FreeRTOS: !!! Nicht Threadsafe !!!
 *
 * @authors		A. Quade
 *
 * @par
 * Projekt:		BAT60 Firmware
 *
 * @copyright
 * navtec GmbH, Berlin, Germany, www.navtec.de			\n
 * Tel.: +49 (0)3375 / 2465078							\n
 * FAX : +49 (0)3375 / 2465079							\n
 *
 * @since		V1.00
 *****************************************************************************/
/** @addtogroup FIFO *********************************************************/
/*@{*/
#include "fifo.h"



#ifndef ISFREERTOSRUNNING
/**************************************************************************//**
 * @brief		Richtet FIFO ein
 *
 * @param[in, out]	pFIFO								FIFO
 * @param[in]	Size									Größe des Buffers in Bytes (der FIFO kann dann ein Byte weniger aufnehmen)
 * @param[in]	pBuffer									Anfang des Speichers
 *
 * @return      true
 */
bool	FIFO_Init(T_ByteFIFO *FIFO, uint32_t Size, uint8_t *pBuffer)
{
	FIFO->Size = Size;
	FIFO->Head = 0;
	FIFO->Tail = 0;
	FIFO->Buffer = pBuffer;

	return true;
}
/**************************************************************************//**
 * @brief		Löscht FIFO
 *
 * @param[in, out]	pFIFO								FIFO
 */
void	FIFO_Clear(pT_ByteFIFO pFIFO)
{
	((T_ByteFIFO*)pFIFO)->Head = 0;
	((T_ByteFIFO*)pFIFO)->Tail = 0;
}
/**************************************************************************//**
 * @brief		Schreibt ein Byte in FIFO
 *
 * @param[in, out]	pFIFO								FIFO
 * @param[in]	b										Datum
 *
 * @return      true - Alles OK, false - kein Platz im FIFO
 */
bool	FIFO_Put(pT_ByteFIFO pFIFO, uint8_t b)
{
uint32_t Next = (((T_ByteFIFO*)pFIFO)->Head + 1) % ((T_ByteFIFO*)pFIFO)->Size;

	if (Next == ((T_ByteFIFO*)pFIFO)->Tail)							// Voll ?
		return false;									// Ende!

	((T_ByteFIFO*)pFIFO)->Buffer[((T_ByteFIFO*)pFIFO)->Head] = b;						// Byte rein
	((T_ByteFIFO*)pFIFO)->Head = Next;

	return true;
}
/**************************************************************************//**
 * @brief		Liest ein Byte aus FIFO
 *
 * @param[in, out]	pFIFO								FIFO
 * @param[out]	pb										Datum
 *
 * @return      true - Alles OK in pB steht Datum, false - FIFO ist leer
 */
bool	FIFO_Get(pT_ByteFIFO pFIFO, uint8_t *pb)
{
	if (((T_ByteFIFO*)pFIFO)->Head == ((T_ByteFIFO*)pFIFO)->Tail)						// Leer
		return false;									// Ende!

	uint32_t Next = (((T_ByteFIFO*)pFIFO)->Tail + 1) % ((T_ByteFIFO*)pFIFO)->Size;

	*pb = ((T_ByteFIFO*)pFIFO)->Buffer[((T_ByteFIFO*)pFIFO)->Tail];					// Byte raus
	((T_ByteFIFO*)pFIFO)->Tail = Next;

	return true;
}
/**************************************************************************//**
 * @brief		Ermittelt belegten Speicherplatz im FIFO
 *
 * @param[in, out]	pFIFO								FIFO
 *
 * @return      belegten Platz in Bytes
 */
uint32_t	FIFO_Available(pT_ByteFIFO pFIFO)
{
	return (((T_ByteFIFO*)pFIFO)->Size + ((T_ByteFIFO*)pFIFO)->Head - ((T_ByteFIFO*)pFIFO)->Tail) % ((T_ByteFIFO*)pFIFO)->Size;
}
/**************************************************************************//**
 * @brief		Ermittelt freien Speicherplatz im FIFO
 *
 * @param[in, out]	pFIFO								FIFO
 *
 * @return      freien Platz in Bytes
 */
uint32_t	FIFO_Free(pT_ByteFIFO pFIFO)
{
	return (((T_ByteFIFO*)pFIFO)->Size - 1 - FIFO_Available(pFIFO));
}
#else
// FreeRTOS Version
#include "freertos.h"
#include "task.h"

/**************************************************************************//**
 * @brief		Richtet FIFO ein
 *
 * @param[in, out]	pFIFO								FIFO
 * @param[in]	Size									Größe des Buffers in Bytes (der FIFO kann dann ein Byte weniger aufnehmen)
 * @param[in]	pBuffer									Anfang des Speichers
 *
 * @return      true
 */
bool	FIFO_Init(T_ByteFIFO *pFIFO, uint32_t Size, uint8_t *pBuffer)
{
	taskENTER_CRITICAL();

		((T_ByteFIFO*)pFIFO)->Size = Size;
		((T_ByteFIFO*)pFIFO)->Head = 0;
		((T_ByteFIFO*)pFIFO)->Tail = 0;
		((T_ByteFIFO*)pFIFO)->Buffer = pBuffer;

	taskEXIT_CRITICAL();

	return true;
}
/**************************************************************************//**
 * @brief		Löscht FIFO
 *
 * @param[in, out]	pFIFO								FIFO
 */
void	FIFO_Clear(pT_ByteFIFO pFIFO)
{
	taskENTER_CRITICAL();

		((T_ByteFIFO*)pFIFO)->Head = 0;
		((T_ByteFIFO*)pFIFO)->Tail = 0;

	taskEXIT_CRITICAL();
}
/**************************************************************************//**
 * @brief		Schreibt ein Byte in FIFO
 *
 * @param[in, out]	pFIFO								FIFO
 * @param[in]	b										Datum
 *
 * @return      true - Alles OK, false - kein Platz im FIFO
 */
bool	FIFO_Put(pT_ByteFIFO pFIFO, uint8_t b)
{
uint32_t Next = (((T_ByteFIFO*)pFIFO)->Head + 1) % ((T_ByteFIFO*)pFIFO)->Size;

	if (Next == ((T_ByteFIFO*)pFIFO)->Tail)							// Voll ?
		return false;									// Ende!

	taskENTER_CRITICAL();

		((T_ByteFIFO*)pFIFO)->Buffer[((T_ByteFIFO*)pFIFO)->Head] = b;					// Byte rein
		((T_ByteFIFO*)pFIFO)->Head = Next;

	taskEXIT_CRITICAL();

	return true;
}
/**************************************************************************//**
 * @brief		Schreibt mehrere Bytes in FIFO
 *
 * @param[in, out]	pFIFO	- FIFO
 * @param[in]	pData		- Daten
 * @param		Len			- Anzahl zu schreibende Bytes
 *
 * @return      Tatsächlich geschriebene Bytes
 */
uint32_t	FIFO_PutLen(pT_ByteFIFO pFIFO, const uint8_t *pData, uint32_t Len)
{
uint32_t Next = (((T_ByteFIFO*)pFIFO)->Head + 1) % ((T_ByteFIFO*)pFIFO)->Size;
uint32_t BytesWritten = 0;
uint8_t* pb = (uint8_t*)pData;

	if (Next == ((T_ByteFIFO*)pFIFO)->Tail)							// Voll ?
		return false;									// Ende!

	taskENTER_CRITICAL();

		while (Len--)
		{
			BytesWritten++;

			((T_ByteFIFO*)pFIFO)->Buffer[((T_ByteFIFO*)pFIFO)->Head] = *pb++;					// Byte rein
			((T_ByteFIFO*)pFIFO)->Head = Next;

			Next = (((T_ByteFIFO*)pFIFO)->Head + 1) % ((T_ByteFIFO*)pFIFO)->Size;
			if (Next == ((T_ByteFIFO*)pFIFO)->Tail)							// Voll ?
				break;

			// mal andere arbeiten lassen
			if (!(BytesWritten % 0x40))
			{
				taskEXIT_CRITICAL();
				taskENTER_CRITICAL();
			}
		}

	taskEXIT_CRITICAL();

	return BytesWritten;
}
/**************************************************************************//**
 * @brief		Schreibt von ISR aus ein Byte in FIFO
 *
 * @param[in, out]	pFIFO								FIFO
 * @param[in]	b										Datum
 *
 * @return      true - Alles OK, false - kein Platz im FIFO
 */
bool	FIFO_PutFromISR(pT_ByteFIFO pFIFO, uint8_t b)
{
uint32_t Next = (((T_ByteFIFO*)pFIFO)->Head + 1) % ((T_ByteFIFO*)pFIFO)->Size;

	if (Next == ((T_ByteFIFO*)pFIFO)->Tail)							// Voll ?
		return false;									// Ende!

uint32_t SavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

		((T_ByteFIFO*)pFIFO)->Buffer[((T_ByteFIFO*)pFIFO)->Head] = b;					// Byte rein
		((T_ByteFIFO*)pFIFO)->Head = Next;

		portCLEAR_INTERRUPT_MASK_FROM_ISR(SavedInterruptStatus);

	return true;
}
/**************************************************************************//**
 * @brief		Liest ein Byte aus FIFO
 *
 * @param[in, out]	pFIFO								FIFO
 * @param[out]	pb										Datum
 *
 * @return      true - Alles OK in pB steht Datum, false - FIFO ist leer
 */
bool	FIFO_Get(pT_ByteFIFO pFIFO, uint8_t *pb)
{
	if (((T_ByteFIFO*)pFIFO)->Head == ((T_ByteFIFO*)pFIFO)->Tail)						// Leer
		return false;									// Ende!

uint32_t Next = (((T_ByteFIFO*)pFIFO)->Tail + 1) % ((T_ByteFIFO*)pFIFO)->Size;

	taskENTER_CRITICAL();

		*pb = ((T_ByteFIFO*)pFIFO)->Buffer[((T_ByteFIFO*)pFIFO)->Tail];					// Byte raus
		((T_ByteFIFO*)pFIFO)->Tail = Next;

	taskEXIT_CRITICAL();

	return true;
}
/**************************************************************************//**
 * @brief		Liest von ISR aus ein Byte aus FIFO
 *
 * @param[in, out]	pFIFO								FIFO
 * @param[out]	pb										Datum
 *
 * @return      true - Alles OK in pB steht Datum, false - FIFO ist leer
 */
bool	FIFO_GetFromISR(pT_ByteFIFO pFIFO, uint8_t *pb)
{
	if (((T_ByteFIFO*)pFIFO)->Head == ((T_ByteFIFO*)pFIFO)->Tail)						// Leer
		return false;									// Ende!

uint32_t Next = (((T_ByteFIFO*)pFIFO)->Tail + 1) % ((T_ByteFIFO*)pFIFO)->Size;

uint32_t SavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

		*pb = ((T_ByteFIFO*)pFIFO)->Buffer[((T_ByteFIFO*)pFIFO)->Tail];					// Byte raus
		((T_ByteFIFO*)pFIFO)->Tail = Next;

		portCLEAR_INTERRUPT_MASK_FROM_ISR(SavedInterruptStatus);

	return true;
}
/**************************************************************************//**
 * @brief		Ermittelt belegten Speicherplatz im FIFO
 *
 * @param[in, out]	pFIFO								FIFO
 *
 * @return      belegten Platz in Bytes
 */
uint32_t	FIFO_Available(pT_ByteFIFO pFIFO)
{
	return (((T_ByteFIFO*)pFIFO)->Size + ((T_ByteFIFO*)pFIFO)->Head - ((T_ByteFIFO*)pFIFO)->Tail) % ((T_ByteFIFO*)pFIFO)->Size;
}
/**************************************************************************//**
 * @brief		Ermittelt freien Speicherplatz im FIFO
 *
 * @param[in, out]	pFIFO								FIFO
 *
 * @return      freien Platz in Bytes
 */
uint32_t	FIFO_Free(pT_ByteFIFO pFIFO)
{
	return (((T_ByteFIFO*)pFIFO)->Size - 1 - FIFO_Available(pFIFO));
}

#endif

/*@}*/ /* end of group FIFO */
