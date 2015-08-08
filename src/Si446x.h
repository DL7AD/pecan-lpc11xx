#ifndef __SI406X__H__
#define __SI406X__H__

#include "types.h"
#include "target.h"
#include "Si446x.h"

#define POWER_0DBM		6
#define POWER_8DBM		16
#define POWER_10DBM		20
#define POWER_14DBM		32
#define POWER_17DBM		40
#define POWER_20DBM		127

bool Si406x_Init(void);
void SendCmdReceiveAnswer(uint8_t* txData, uint32_t byteCountTx, uint8_t* rxData, uint32_t byteCountRx);
void SendCmdReceiveAnswerSetDelay(uint8_t* txData, uint32_t byteCountTx, uint8_t* rxData, uint32_t byteCountRx, uint32_t delays);
void sendFrequencyToSi406x(uint32_t freq);
void setModem();
void setDeviation(uint32_t deviation);
void setPowerLevel(uint8_t level);
void startTx(void);
void stopTx(void);
void radioShutdown(void);
void radioTune(uint32_t frequency, uint8_t level);
inline void setGPIO(bool s);

#endif
