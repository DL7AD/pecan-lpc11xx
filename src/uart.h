#ifndef __UART_H__
#define __UART_H__

#include "types.h"
#include "LPC11xx.h"

// Einstellungen Für 12 MHz Takt
#define UART_MULVAL_9600			9
#define UART_DIVADDVAL_9600			10
#define	UART_DIVISOR_9600			37

#define UART_MULVAL_4800			4
#define UART_DIVADDVAL_4800			1
#define	UART_DIVISOR_4800			125

#define UART_MULVAL_1200			1
#define UART_DIVADDVAL_1200			0
#define	UART_DIVISOR_1200			625

#define UART_MULVAL_600				1
#define UART_DIVADDVAL_600			0
#define	UART_DIVISOR_600			1250

#define UART_MULVAL_300				1
#define UART_DIVADDVAL_300			0
#define	UART_DIVISOR_300			2500

#define UART_MULVAL_150				1
#define UART_DIVADDVAL_150			0
#define	UART_DIVISOR_150			5000

#define UART_MULVAL_50				1
#define UART_DIVADDVAL_50			0
#define	UART_DIVISOR_50				15000

bool UART_Init(uint32_t baud);

bool UART_TransmitChar(uint8_t Data);
bool UART_ReceiveChar(uint8_t* pData);
uint32_t UART_TxString(uint8_t* pData, uint32_t Len);
void On_UART(void);
void UART_DeInit(void);

void EnterCritical(void);
void ExitCritical(void);

/*@}*/ /* end of group UART */
#endif /* __UART_H__ */
