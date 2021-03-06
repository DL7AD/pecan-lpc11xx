#ifndef __I2C__H__
#define __I2C__H__

#include "types.h"
#include "config.h"

#ifdef USE_I2C

#define I2C_ERROR_WAITING	10							// Alles OK, die Kommunikation ist im Gange
#define I2C_ERROR_NONE		0							// Alles OK
#define I2C_ERROR_MASTER	-1							// Ein Fehler auf Masterseite, meist Fehlkonfiguration
#define I2C_ERROR_SLAVE		-2							// Ein Fehler auf Slaveseite, meist fehlendes ACK
#define I2C_ERROR_BUS		-3							// Der I²C-Bus verhält sich nicht korrekt, z.B. Fehler bei Start-Generierung
#define I2C_ERROR_UNKNOWN	-4							// Nichts Genaues weiß man nicht!

#define I2C_FREQUENCY		100000
#define I2C_RETRYDELAY		50
#define I2C_TIMEOUT			50

bool	I2C_Init(void);
void	I2C_Read(uint8_t DeviceID, uint8_t Address, uint8_t Count, uint8_t* pData, int32_t* pError, uint32_t Retries);
void	I2C_Write(uint8_t DeviceID, uint8_t Address, uint8_t Count, uint8_t* pData, int32_t* pError, uint32_t Retries);
void	I2C_Process(uint8_t DeviceID, uint8_t CmdCode, uint16_t CmdWord, uint16_t *AnswerWord, int32_t* pError, uint32_t Retries);
void	On_I2C(void);
void	I2C_DeInit(void);

#endif
#endif
