#ifndef __SSD1306__H__
#define __SSD1306__H__

#include "debug.h"
#include "defines.h"
#include "types.h"

#define SDA(Select) { \
	if(Select) { \
		OUT4_CONF_READ(); \
	} else { \
		OUT4_CONF_WRITE(); \
		OUT4_SET(false); \
	} \
}
#define SCL(Select) { \
	if(Select) { \
		OUT3_CONF_READ(); \
	} else { \
		OUT3_CONF_WRITE(); \
		OUT3_SET(false); \
	} \
}
#define SSD1306_PIN_INIT() { \
	OUT1_CONF_WRITE(); \
	OUT1_SET(HIGH); \
}

#define SSD_COMMAND_MODE			0x00
#define SSD_Data_Mode				0x40
#define SSD_Display_Off				0xAE
#define SSD_Display_On				0xAF
#define SSD_Set_ContrastLevel		0x81
#define SSD_Deactivate_Scroll		0x2E

#define SSD1306_DISPLAYALLON_RESUME	0xA4
#define SSD1306_DISPLAYALLON 		0xA5

#define SSD1306_Normal_Display		0xA6

#define SSD1306_SETDISPLAYOFFSET 	0xD3
#define SSD1306_SETCOMPINS 			0xDA
#define SSD1306_SETVCOMDETECT		0xDB
#define SSD1306_SETDISPLAYCLOCKDIV	0xD5
#define SSD1306_SETPRECHARGE 		0xD9
#define SSD1306_SETMULTIPLEX 		0xA8
#define SSD1306_SETLOWCOLUMN 		0x00
#define SSD1306_SETHIGHCOLUMN 		0x10
#define SSD1306_SETSTARTLINE 		0x40
#define SSD1306_MEMORYMODE 			0x20
#define SSD1306_COMSCANDEC 			0xC8
#define SSD1306_SEGREMAP 			0xA1 /* it has been A0 */
#define SSD1306_CHARGEPUMP 			0x8D

#define SSD1306_I2C_ADDRESS			0x78

#define OLED_WIDTH					128
#define OLED_HEIGHT					64

#define OLED_WHITE					true
#define OLED_BLACK					false

void Init_SSD1306(void);
void i2c_write_byte(bool send_start, bool send_stop, uint8_t byte);
void i2c_send_command_1byte(uint8_t b1);
void i2c_send_command_2bytes(uint8_t b1, uint8_t b2);
void i2c_send_command_3bytes(uint8_t b1, uint8_t b2, uint8_t b3);
void i2c_send_bytes_list(uint8_t len, uint8_t* bytes);
void ssd1306_drawPixel(uint8_t x, uint8_t y, bool color);
void ssd1306_display(void);
void drawChar(int16_t x, int16_t y, char c, uint16_t color, uint16_t bg);
void drawLines(void);
void addLine(char* line);

#endif
