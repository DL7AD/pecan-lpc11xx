/** This is a library for our Monochrome OLEDs based on SSD1306 drivers
  * Pick one up today in the adafruit shop!
  * ------> http://www.adafruit.com/category/63_98
  * These displays use SPI to communicate, 4 or 5 pins are required to
  * interface
  * Adafruit invests time and resources providing this open source code,
  * please support Adafruit and open-source hardware by purchasing
  * products from Adafruit!
  * Written by Limor Fried/Ladyada  for Adafruit Industries.
  * BSD license, check license.txt for more information
  * All text above, and the splash screen below must be included in any redistribution
  */

#include "ssd1306.h"
#include "debug.h"
#include "types.h"
#include "global.h"
#include "target.h"
#include "glcdfont.h"
#include <string.h>
#include <stdlib.h>

uint8_t poledbuff[1024];
char textbuffer[7][22];

void Init_SSD1306(void) {
	// Power up SSD1306
	SSD1306_PIN_INIT();

	// Configure SSD1306
	i2c_send_command_1byte(SSD_Display_Off);
	i2c_send_command_2bytes(SSD1306_SETDISPLAYCLOCKDIV, 0x80);
	i2c_send_command_2bytes(SSD1306_SETMULTIPLEX, 0x3F);
	i2c_send_command_2bytes(SSD1306_SETDISPLAYOFFSET, 0x00);
	i2c_send_command_1byte(SSD1306_SETSTARTLINE | 0x0);
	i2c_send_command_2bytes(SSD1306_CHARGEPUMP, 0x14);
	i2c_send_command_2bytes(SSD1306_MEMORYMODE, 0x00);
	i2c_send_command_1byte(SSD1306_SEGREMAP | 0x1);
	i2c_send_command_1byte(SSD1306_COMSCANDEC);
	i2c_send_command_2bytes(SSD1306_SETCOMPINS, 0x12);
	i2c_send_command_2bytes(SSD_Set_ContrastLevel, 0xCF);
	i2c_send_command_2bytes(SSD1306_SETPRECHARGE, 0xF1);
	i2c_send_command_2bytes(SSD1306_SETVCOMDETECT, 0x40);
	i2c_send_command_1byte(SSD1306_DISPLAYALLON_RESUME);
	i2c_send_command_1byte(SSD1306_Normal_Display);

	// Reset to default value in case of
	// no reset pin available on OLED
	i2c_send_command_3bytes( 0x21, 0, 127 );
	i2c_send_command_3bytes( 0x22, 0,   7 );
	i2c_send_command_1byte(SSD_Deactivate_Scroll);

	// Empty uninitialized buffer
	memset(poledbuff, 0, (OLED_WIDTH*OLED_HEIGHT)/8);
	i2c_send_command_1byte(SSD_Display_On);
}

void i2c_write_byte(bool send_start, bool send_stop, uint8_t byte) {
	if(send_start) {
		SDA(LOW);
		SCL(LOW);
	}

	uint8_t bit;
	for(bit = 0; bit < 8; bit++) {
		// Write bit
		SDA((byte & 0x80) != 0);
		SCL(HIGH);
		SCL(LOW);

		byte <<= 1;
	}

	// ACK (will be ignored)
	SDA(false);
	SCL(HIGH);
	SCL(LOW);

	if(send_stop) {
		SCL(HIGH);
		SDA(HIGH);
	}
}
void i2c_send_command_1byte(uint8_t b1) {
	i2c_write_byte(true, false, SSD1306_I2C_ADDRESS);
	i2c_write_byte(false, false, SSD_COMMAND_MODE);
	i2c_write_byte(false, true, b1);
}
void i2c_send_command_2bytes(uint8_t b1, uint8_t b2) {
	i2c_write_byte(true, false, SSD1306_I2C_ADDRESS);
	i2c_write_byte(false, false, SSD_COMMAND_MODE);
	i2c_write_byte(false, false, b1);
	i2c_write_byte(false, true, b2);
}
void i2c_send_command_3bytes(uint8_t b1, uint8_t b2, uint8_t b3) {
	i2c_write_byte(true, false, SSD1306_I2C_ADDRESS);
	i2c_write_byte(false, false, SSD_COMMAND_MODE);
	i2c_write_byte(false, false, b1);
	i2c_write_byte(false, false, b2);
	i2c_write_byte(false, true, b3);
}
void i2c_send_bytes_list(uint8_t len, uint8_t* bytes) {
	i2c_write_byte(true, false, SSD1306_I2C_ADDRESS);
	uint8_t i;
	for(i=0; i<len-1; i++)
		i2c_write_byte(false, false, bytes[i]);
	i2c_write_byte(false, true, bytes[len-1]);
}

void ssd1306_drawPixel(uint8_t x, uint8_t y, bool color) {
	uint8_t* p = poledbuff;

	if (x >= OLED_WIDTH || y >= OLED_HEIGHT)
		return;

	// Get where to do the change in the buffer
	p = poledbuff + (x + (y/8)*OLED_WIDTH );

	// x is which column
	if(color)
		*p |=  (1 << (y%8));
	else
		*p &= ~(1 << (y%8));
}

void ssd1306_display(void)
{
	i2c_send_command_1byte(SSD1306_SETLOWCOLUMN  | 0x0); // low col = 0
	i2c_send_command_1byte(SSD1306_SETHIGHCOLUMN | 0x0); // hi col = 0
	i2c_send_command_1byte(SSD1306_SETSTARTLINE  | 0x0); // line #0

	// pointer to OLED data buffer
	uint8_t* p = poledbuff;

	uint8_t buff[65];
	uint8_t x;

	// Setup D/C to switch to data mode
	buff[0] = SSD_Data_Mode;

	// loop trough all OLED buffer and
	// send a bunch of 16 data byte in one xmission
	uint16_t i;
	for(i=0; i<(OLED_WIDTH*OLED_HEIGHT/8); i+=64)
	{
		for(x=1; x<=64; x++)
			buff[x] = *p++;

		i2c_send_bytes_list(65, buff);
	}
}

void drawChar(int16_t x, int16_t y, char c, uint16_t color, uint16_t bg) {

	if((x >= OLED_WIDTH) || (y >= OLED_HEIGHT) || ((x + 6 - 1) < 0) || ((y + 8 - 1) < 0))
		return;

	bool _cp437 = true;
	if(!_cp437 && (c >= 176)) c++; // Handle 'classic' charset behavior

	int8_t i;
	for(i=0; i<6; i++ ) {
		uint8_t line;
		if (i == 5)
			line = 0x0;
		else
			line = font[(c*5)+i];
		int8_t j;
		for(j = 0; j<8; j++) {
			if (line & 0x1) {
				ssd1306_drawPixel(x+i, y+j, color);
			} else if (bg != color) {
				ssd1306_drawPixel(x+i, y+j, bg);
			}
			line >>= 1;
		}
	}
}

void drawLines(void) {
	uint8_t line, charN;
	for(line=0; line<7; line++) {
		bool eol = false;
		for(charN=0; charN<21; charN++) {
			if(textbuffer[line][charN] == '\0')
				eol = true;
			drawChar(charN*6, line*9+2, (eol ? ' ' : textbuffer[line][charN]), OLED_WHITE, OLED_BLACK);
		}
	}
	ssd1306_display();
}

void addLine(char* line) {
	uint8_t i;
	for(i=1; i<7; i++)
		memcpy(textbuffer[i-1], textbuffer[i], 22);
	memcpy(textbuffer[6], line, 22);
}











