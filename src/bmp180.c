#include "config.h"
#include "bmp180.h"
#include "i2c.h"
#include "global.h"

#define BMP180_ADDRESS_READ		0xEF	// I2C address of BMP180 for read access
#define BMP180_ADDRESS_WRITE	0xEE	// I2C address of BMP180 for write access
#define BMP180_OSS				0		// Oversampling setting
#define BMP180_I2C_RETRIES		0		// I2C retries

// Calibration data
typedef struct {
	int16_t ac1;
	int16_t ac2;
	int16_t ac3;
	uint16_t ac4;
	uint16_t ac5;
	uint16_t ac6;
	int16_t b1;
	int16_t b2;
	int16_t mb;
	int16_t mc;
	int16_t md;
} bmp180calibration_t;
bmp180calibration_t calib;

int32_t errorcode;

void BMP180_Init(void)
{
	I2C_Init();
	I2C_Read(BMP180_ADDRESS_READ, 0xAA, sizeof(calib), (uint8_t*)&calib, &errorcode, BMP180_I2C_RETRIES);
	__ASM volatile ("rev16 %0, %1" : "=r" (calib.ac1) : "r" (calib.ac1) );
	__ASM volatile ("rev16 %0, %1" : "=r" (calib.ac2) : "r" (calib.ac2) );
	__ASM volatile ("rev16 %0, %1" : "=r" (calib.ac3) : "r" (calib.ac3) );
	__ASM volatile ("rev16 %0, %1" : "=r" (calib.ac4) : "r" (calib.ac4) );
	__ASM volatile ("rev16 %0, %1" : "=r" (calib.ac5) : "r" (calib.ac5) );
	__ASM volatile ("rev16 %0, %1" : "=r" (calib.ac6) : "r" (calib.ac6) );
	__ASM volatile ("rev16 %0, %1" : "=r" (calib.b1) : "r" (calib.b1) );
	__ASM volatile ("rev16 %0, %1" : "=r" (calib.b2) : "r" (calib.b2) );
	__ASM volatile ("rev16 %0, %1" : "=r" (calib.mb) : "r" (calib.mb) );
	__ASM volatile ("rev16 %0, %1" : "=r" (calib.mc) : "r" (calib.mc) );
	__ASM volatile ("rev16 %0, %1" : "=r" (calib.md) : "r" (calib.md) );
}

void BMP180_DeInit()
{
	I2C_DeInit();
}

uint16_t readUncompensatedTemperature(void)
{
	  uint8_t buffer[2] = {0x2E};
	  I2C_Write(BMP180_ADDRESS_WRITE, 0xF4, 1, buffer, &errorcode, BMP180_I2C_RETRIES);

	  delay(5); // Wait at least 4.5ms

	  I2C_Read(BMP180_ADDRESS_READ, 0xF6, 2, buffer, &errorcode, BMP180_I2C_RETRIES);
	  __ASM volatile ("rev16 %0, %1" : "=r" (((uint16_t*)buffer)[0]) : "r" (((uint16_t*)buffer)[0]) ); // Converting 16bit int to little endian
	  return ((uint16_t*)buffer)[0];
}

uint16_t readUncompensatedPressure(void)
{
	uint8_t buffer[3] = {0x34 + (BMP180_OSS<<6)};
	I2C_Write(BMP180_ADDRESS_WRITE, 0xF4, 1, buffer, &errorcode, BMP180_I2C_RETRIES);

	delay(2 + (3<<BMP180_OSS)); // Wait for conversion, delay time dependent on OSS

	// Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
	I2C_Read(BMP180_ADDRESS_READ, 0xF6, 3, buffer, &errorcode, BMP180_I2C_RETRIES);

	return ((buffer[0] << 16) | (buffer[1] << 8) | buffer[2]) >> (8-BMP180_OSS);}

/**
 * Read temperature from BMP180
 * @return Temperature in 0.1 deg C per unit
 */
int32_t getTemperature(void)
{
	int32_t x1 = ((readUncompensatedTemperature() - calib.ac6) * calib.ac5) >> 15;
	int32_t x2 = (calib.mc << 11) / (x1 + calib.md);
	return (x1 + x2 + 8) >> 4;
}

/**
 * Read pressure from BMP180
 * @return Pressure in pascal
 */
int32_t getPressure(void)
{
	int32_t x1, x2, x3, b3, b4, b6, p;
	int64_t b7;

	x1 = ((readUncompensatedTemperature() - calib.ac6) * calib.ac5) >> 15;
	x2 = (calib.mc << 11) / (x1 + calib.md);
	b6 = x1 + x2 - 4000;

	x1 = (calib.b2 * ((b6 * b6) >> 12)) >> 11;
	x2 = (calib.ac2 * b6) >> 11;
	x3 = x1 + x2;
	b3 = (((calib.ac1*4 + x3) << BMP180_OSS) + 2) / 4;

	x1 = calib.ac3 * b6 >> 13;
	x2 = (calib.b1 * ((b6*b6) >> 12)) >> 16;
	x3 = ((x1+2) + 2) >> 2;
	b4 = (calib.ac4 * (uint64_t)(x3 + 32768)) >> 15;

	b7 = ((uint64_t)(readUncompensatedPressure() - b3) * (50000>>BMP180_OSS));
	if (b7 < 0x80000000)
		p = (b7*2)/b4;
	else
		p = (b7/b4)*2;

	x1 = (p>>8) * (p>>8);
	x1 = (x1 * 3038) >> 16;
	x2 = (-7357 * p) >> 16;
	p += (x1 + x2 + 3791) >> 4;

	return p;
}
