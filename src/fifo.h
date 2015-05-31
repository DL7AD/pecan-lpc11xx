#ifndef __FIFO_H__
#define __FIFO_H__
#include "types.h"

typedef void * pT_ByteFIFO;

/**************************************************************************//**
 * @brief		FIFO
 */
typedef struct {
	uint32_t	Size;										//!< (Bytes) Größe des FIFO-Buffers
	uint32_t	Head;										//!< Leseposition
	uint32_t	Tail;										//!< Schreibposition
	uint8_t*	Buffer;										//!< Buffer
}T_ByteFIFO;

bool	FIFO_Init		(T_ByteFIFO *pFIFO, uint32_t Size, uint8_t *pBuffer);
void	FIFO_Clear		(pT_ByteFIFO pFIFO);
bool	FIFO_Put		(pT_ByteFIFO pFIFO, uint8_t b);
bool	FIFO_Get		(pT_ByteFIFO pFIFO, uint8_t *pb);
uint32_t	FIFO_Available	(pT_ByteFIFO pFIFO);
uint32_t	FIFO_Free		(pT_ByteFIFO pFIFO);

#ifdef ISFREERTOSRUNNING
uint32_t	FIFO_PutLen		(pT_ByteFIFO pFIFO, const uint8_t *pData, uint32_t Len);
bool	FIFO_PutFromISR	(pT_ByteFIFO pFIFO, uint8_t b);
bool	FIFO_GetFromISR	(pT_ByteFIFO pFIFO, uint8_t *pb);
#endif

#endif
