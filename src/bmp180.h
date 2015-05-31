/**
 * BMP180 driver
 * @author Sven Steudte
 */

#ifndef __BMP180__H__
#define __BMP180__H__

#ifdef BMP180_AVAIL

#include "config.h"
#include "bmp180.h"
#include "i2c.h"
#include "global.h"

void BMP180_Init(void);
void BMP180_DeInit();
uint16_t readUncompensatedTemperature(void);
uint16_t readUncompensatedPressure(void);
int32_t getTemperature(void);
int32_t getPressure(void);

#endif
#endif
