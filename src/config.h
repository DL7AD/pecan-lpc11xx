/* trackuino copyright (C) 2010  EA5HAV Javi
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "defines.h"
#include "LPC11xx.h"


// --------------------------------------------------------------------------
// THIS IS THE TRACKUINO FIRMWARE CONFIGURATION FILE. YOUR CALLSIGN AND
// OTHER SETTINGS GO HERE.
//
// NOTE: all pins are Arduino based, not the Atmega chip. Mapping:
// http://www.arduino.cc/en/Hacking/PinMapping
// --------------------------------------------------------------------------


// --------------------------------------------------------------------------
// APRS config (aprs.c)
// --------------------------------------------------------------------------

// Set your callsign and SSID here. Common values for the SSID are
// (from http://zlhams.wikidot.com/aprs-ssidguide):
//
// - Balloons:  11
// - Cars:       9
// - Buoy:       8
// - Home:       0
// - IGate:      5
//
#define S_CALLSIGN				"DL7AD"
#define S_CALLSIGN_ID			11

// Destination callsign: APRS (with SSID=0) is usually okay.
#define D_CALLSIGN				"APECAN"  // APExxx = Telemetry devices (that's what Pecan actually is)

#define D_CALLSIGN_ID			0


// APRS Symbol
// Consists of Symbol table (/ or \ or a dew overlay symbols) and the symbol ID
#define APRS_SYMBOL_TABLE		'/' // Default table
#define APRS_SYMBOL_ID			'O' // /O = Balloon

// Digipeating paths:
// (read more about digipeating paths here: http://wa8lmf.net/DigiPaths/ )
// The recommended digi path for a balloon is WIDE2-1 or pathless. The default
// is to use WIDE2-1. Comment out the following two lines for pathless:
#define DIGI_PATH1				"WIDE1"
#define DIGI_PATH1_TTL			1
#define DIGI_PATH2				"WIDE1"
#define DIGI_PATH2_TTL			1

// If we want to pass selected packets through the International Space Station
#define DIGI_PATH1_SAT "ARISS"
#define DIGI_PATH1_TTL_SAT		0
// #define DIGI_PATH2_SAT		"SGATE"
// #define DIGI_PATH2_TTL_SAT	0


// APRS comment: this goes in the comment portion of the APRS message. You
// might want to keep this short. The longer the packet, the more vulnerable
// it is to noise. 
#define APRS_COMMENT    ""




// Pressure in Pascal at sea level. 
// Can be adjusted at launch date to get exact altitude from BMP180
#define P0				101325.0


// --------------------------------------------------------------------------
// AX.25 config (ax25.cpp)
// --------------------------------------------------------------------------

// TX delay in milliseconds
#define TX_DELAY		300   // default was 300

// --------------------------------------------------------------------------
// Tracker config (trackuino.cpp)
// --------------------------------------------------------------------------

// APRS_PERIOD is the period between transmissions. Since we're not listening
// before transmitting, it may be wise to choose a "random" value here JUST
// in case someone else is transmitting at fixed intervals like us. 61000 ms
// is the default (1 minute and 1 second).
//
// Low-power transmissions on occasional events (such as a balloon launch)
// might be okay at lower-than-standard APRS periods (< 10m). Check with/ask
// permision to local digipeaters beforehand.
// #define APRS_PERIOD   25000UL

// APRS_PERIOD is here replaced with APRS_PERIOD_SECONDS because we use the 
// watchdog timer to save more power.
#define APRS_PERIOD_SECONDS 120

// Set any value here (in ms) if you want to delay the first transmission
// after resetting the device.
#define APRS_DELAY    0UL

// GPS baud rate (in bits per second)
#define GPS_BAUDRATE  9600   //4800 for Argentdata High Altitude GPS, 9600 for Venus or uBlox MAX-6Q

// Radio: I've tested trackuino with two different radios:
// Radiometrix HX1 and SRB MX146. The interface with these devices
// is implemented in their respective radio_*.cpp files, and here
// you can choose which one will be hooked up to the tracker.
// The tracker will behave differently depending on the radio used:
//
// RadioHx1 (Radiometrix HX1):
// - Time from PTT-on to transmit: 5ms (per datasheet)
// - PTT is TTL-level driven (on=high) and audio input is 5v pkpk
//   filtered and internally DC-coupled by the HX1, so both PTT
//   and audio can be wired directly. Very few external components
//   are needed for this radio, indeed.
//
// RadioMx146 (SRB MX146):
// - Time from PTT-on to transmit: signaled by MX146 (pin RDY)
// - Uses I2C to set freq (144.8 MHz) on start
// - I2C requires wiring analog pins 4/5 (SDA/SCL) via two level
//   converters (one for each, SDA and SCL). DO NOT WIRE A 5V ARDUINO
//   DIRECTLY TO THE 3.3V MX146, YOU WILL DESTROY IT!!!
//
//   I2C 5-3.3v LEVEL TRANSLATOR:
//
//    +3.3v o--------+-----+      +---------o +5v
//                   /     |      /
//                R  \     |      \ R
//                   /    G|      /
//              3K3  \   _ L _    \ 4K7
//                   |   T T T    |
//   3.3v device o---+--+|_| |+---+---o 5v device
//                     S|     |D
//                      +-|>|-+
//                             N-channel MOSFET
//                           (BS170, for instance)
//
//   (Explanation of the lever translator:
//   http://www.neoteo.com/adaptador-de-niveles-para-bus-i2c-3-3v-5v.neo)
//
// - Audio needs a low-pass filter (R=8k2 C=0.1u) plus DC coupling
//   (Cc=10u). This also lowers audio to 500mV peak-peak required by
//   the MX146.
//
//                   8k2        10uF
//   Arduino out o--/\/\/\---+---||---o
//                     R     |     Cc
//                          ===
//                     0.1uF | C
//                           v
//
// - PTT is pulled internally to 3.3v (off) or shorted (on). Use
//   an open-collector BJT to drive it:
//        
//                             o MX146 PTT
//                             |
//                    4k7    b |c
//   Arduino PTT o--/\/\/\----K  (Any NPN will do)
//                     R       |e
//                             |
//                             v GND
// 
// - Beware of keying the MX146 for too long, you will BURN it.
//
// So, summing up. Options are:
//
// - RadioMx146
// - RadioHx1
// - RadioAdf7012
// - RadioSi446x
//#define RADIO_CLASS   RadioSi446x

// --------------------------------------------------------------------------
// Radio config (radio_*.cpp)
// --------------------------------------------------------------------------

// This is the PTT pin to enable the transmitter and the VCXO; HIGH = on                

// The ADL5531 PA is switched on separately with the TPS77650 Enable pin; LOW = on
//#define TX_PA_PIN         6 // Not available at PecanPico

// This is the pin used by the MX146 radio to signal full RF
//#define MX146_READY_PIN   2 // Not available at PecanPico

#define VCXO_FREQ					27000000

// Some radios are frequency agile. Therefore we can set the (default) frequency here:
#define RADIO_FREQUENCY				144800000

// In other regions the APRS frequencies are different. Our balloon may travel
// from one region to another, so we may QSY according to GPS defined geographical regions
// Here we set some regional frequencies:

#define RADIO_FREQUENCY_REGION1		144800000 // Europe & Africa
#define RADIO_FREQUENCY_REGION2		144800000 // North and south America (Brazil is different)

#define RADIO_FREQUENCY_JAPAN		144800000
#define RADIO_FREQUENCY_CHINA		144800000
#define RADIO_FREQUENCY_BRAZIL		144800000
#define RADIO_FREQUENCY_AUSTRALIA	144800000
#define RADIO_FREQUENCY_NEWZEALAND	144800000
#define RADIO_FREQUENCY_THAILAND	144800000


/*
    144.390 MHz - Chile, Indonesia, North America
    144.575 MHz - New Zealand 
    144.660 MHz - Japan
    144.640 MHz - China
    144.800 MHz - South Africa, Europe, Russia
    144.930 MHz - Argentina, Uruguay (no confirmation found. Hope this is not a typo?)
    145.175 MHz - Australia
    145.570 MHz - Brazil
    145.525 MHz - Thailand

*/

#define TARGET					TARGET_PECAN_PICO6
#define RADIO_POWER				10


/* ---------------------------- Target definitions ---------------------------- */
/* ----------------------- Please don't touch anything ------------------------ */

#if TARGET == TARGET_PECAN_FEMTO2_1

	#define USE_GPS_SW_SW						// GPS software switch activated
	#define GPS_BUS				BUS_UART		// Use UART bus for GPS communication
	#define ADC_REF				REF_VCC			// ADC reference VCC input
	#define REF_MV				3300			// Reference voltage in mv

	#define ADC_PIO_BATT		PIO1_2
	#define ADC_AD_BATT			AD3

	#define ADC_REF				REF_VCC			// ADC reference is 1.8V LDO
	#define ADC_PIO_BATT		PIO1_2
	#define ADC_AD_BATT			AD3

	#define UART_GPIO_RXD		LPC_GPIO3
	#define UART_PIO_RXD		PIO4_5
	#define UART_PIN_RXD		(1 << 5)

	#define UART_GPIO_TXD		LPC_GPIO3
	#define UART_PIO_TXD		PIO3_4
	#define UART_PIN_TXD		(1 << 4)

	#define GPS_GPIO_RESET		LPC_GPIO0
	#define GPS_PIO_RESET		PIO0_7
	#define GPS_PIN_RESET		(1 << 7)

	#define SSP_PORT			LPC_SSP1
	#define SSP_IRQn			SSP1_IRQn
	#define SSP_PIO_MOSI		PIO1_9
	#define SSP_PIO_MISO		PIO1_10
	#define SSP_PIO_SCK			PIO3_2

	#define SSP_GPIO_RADIO_SS	LPC_GPIO2
	#define SSP_PIO_RADIO_SS	PIO2_0
	#define SSP_PIN_RADIO_SS	(1 << 0)

	#define RADIO_GPIO_SDN		LPC_GPIO1
	#define RADIO_PIO_SDN		PIO1_4
	#define RADIO_PIN_SDN		(1 << 4)

	#define VCXO_GPIO_EN		LPC_GPIO1
	#define VCXO_PIO_EN			PIO1_11
	#define VCXO_PIN_EN			(1 << 11)

	#define RADIO_GPIO_GPIO0	LPC_GPIO1
	#define RADIO_PIO_GPIO0		PIO1_7
	#define RADIO_PIN_GPIO0		(1 << 7)

	#define VCXO_GPIO_CTRL		LPC_GPIO1
	#define VCXO_PIO_CTRL		PIO1_6
	#define VCXO_MR_CTRL		MR0

#elif TARGET == TARGET_PECAN_PICO6

	#define USE_GPS_HW_SW						// GPS transistor switch available
	#define EXT_PIN_AVAIL						// External pins available
	#define SOLAR_AVAIL							// Solar feed available
	#define BMP180_AVAIL						// Pressure sensor BMP180 is available
	#define GPS_BUS				BUS_UART		// Use UART bus for GPS communication

	#define ADC_REF				REF_VCC1V8_LDO	// ADC reference is 1.8V LDO
	#define ADC_PIO_REF			R_PIO1_1
	#define ADC_AD_REF			AD2
	#define ADC_PIO_SOLAR		R_PIO1_0
	#define ADC_AD_SOLAR		AD1

	#define LDO_GPIO_EN			LPC_GPIO1
	#define LDO_PIO_EN			R_PIO1_2
	#define LDO_PIN_EN			(1 << 2)

	#define SOLAR_AVAIL							// Solar input available
	#define ADC_PIO_VSOL		PIO1_2
	#define ADC_PIN_VSOL		(1 << 2)

	#define I2C_PULLUPS_AVAIL					// Controlable I2C Pullups available
	#define I2C_GPIO_PULL_VCC	LPC_GPIO1
	#define I2C_PIO_PULL_VCC	PIO1_9
	#define I2C_PIN_PULL_VCC	(1 << 9)

	#define UART_GPIO_RXD		LPC_GPIO1
	#define UART_PIO_RXD		PIO1_6
	#define UART_PIN_RXD		(1 << 6)

	#define UART_GPIO_TXD		LPC_GPIO1
	#define UART_PIO_TXD		PIO1_7
	#define UART_PIN_TXD		(1 << 7)

	#define GPS_GPIO_RESET		LPC_GPIO2
	#define GPS_PIO_RESET		PIO2_0
	#define GPS_PIN_RESET		(1 << 0)

	#define SSP_PORT			LPC_SSP0
	#define SSP_IRQn			SSP0_IRQn
	#define SSP_PIO_MOSI		PIO0_9
	#define SSP_PIO_MISO		PIO0_8
	#define SSP_PIO_SCK			PIO0_6

	#define SSP_GPIO_RADIO_SS	LPC_GPIO0
	#define SSP_PIO_RADIO_SS	PIO0_7
	#define SSP_PIN_RADIO_SS	(1 << 7)

	#define RADIO_GPIO_SDN		LPC_GPIO1
	#define RADIO_PIO_SDN		PIO1_10
	#define RADIO_PIN_SDN		(1 << 10)

	#define VCXO_GPIO_EN		LPC_GPIO0
	#define VCXO_PIO_EN			PIO0_3
	#define VCXO_PIN_EN			(1 << 3)

	#define RADIO_GPIO_GPIO0	LPC_GPIO3
	#define RADIO_PIO_GPIO0		PIO3_4
	#define RADIO_PIN_GPIO0		(1 << 4)

	#define RADIO_GPIO_GPIO1	LPC_GPIO3
	#define RADIO_PIO_GPIO1		PIO3_5
	#define RADIO_PIN_GPIO1		(1 << 5)

	#define VCXO_GPIO_CTRL		LPC_GPIO0
	#define VCXO_PIO_CTRL		PIO0_11
	#define VCXO_MR_CTRL		MR3

	#define EXT_GPIO_OUT1		GPIO1
	#define EXT_PIO_OUT1		PIO1_4
	#define EXT_PIN_OUT1		(1 << 4)

	#define EXT_GPIO_OUT2		GPIO1
	#define EXT_PIO_OUT2		PIO1_11
	#define EXT_PIN_OUT2		(1 << 11)

	#define EXT_GPIO_OUT3		GPIO3
	#define EXT_PIO_OUT3		PIO3_2
	#define EXT_PIN_OUT3		(1 << 2)

	#define EXT_GPIO_OUT4		GPIO1
	#define EXT_PIO_OUT4		PIO1_5
	#define EXT_PIN_OUT4		(1 << 5)

	#define I2C_GPIO_SDA		GPIO0
	#define I2C_PIO_SDA			PIO0_5
	#define I2C_PIN_SDA			(1 << 5)
	#define I2C_GPIO_SCL		GPIO0
	#define I2C_PIO_SCL			PIO0_4
	#define I2C_PIN_SCL			(1 << 4)

	#define I2C_GPIO_PULL_VCC	LPC_GPIO1
	#define I2C_PIO_PULL_VCC	PIO1_9
	#define I2C_PIN_PULL_VCC	(1 << 9)

	#define GPS_GPIO_EN			LPC_GPIO1
	#define GPS_PIO_EN			PIO1_8
	#define GPS_PIN_EN			(1 << 8)

	#define BMP_AVAIL

#else
	#error No/incorrect target selected
#endif



/* ------------------------------ Error messages ------------------------------ */
/* ----------------------- Please don't touch anything ------------------------ */


// TODO: Rewrite configuration validation again and check which checks are missing

//#if !defined(ADC_PIN_REF) || !defined(ADC_PIO_REF)
//#error No ADC Reference Pin defined
//#endif
//
//#if !defined(ADC_REF)
//#error No ADC reference type defined
//#endif
//
//#if ADC_REF == REF_VCC1V8_LDO && (!defined(LDO_GPIO_EN) || !defined(LDO_PIO_EN) || !defined(LDO_PIN_EN))
//#error No LDO enable pin defined
//#endif
//
//#if defined(SOLAR_AVAIL) && (!defined(ADC_PIO_VSOL) || !defined(ADC_PIN_VSOL))
//#error No Solar ADC pin defined
//#endif
//
//#if GPS_BUS == BUS_UART && UART_TXD != UART_TXD_PIO1_7 && UART_TXD != UART_TXD_PIO3_5
//#error No UART TXD pin or incorrect pin defined
//#endif
//
//#if GPS_BUS == BUS_UART && UART_RXD != UART_RXD_PIO1_6 && UART_RXD != UART_RXD_PIO3_4
//#error No UART RXD pin or incorrect pin defined
//#endif


//#if !defined(SSP_GPIO_RADIO_SS) || !defined(SSP_PIO_RADIO_SS) || !defined(SSP_PIN_RADIO_SS)
//#error No SSP Slave Select pin defined
//#endif
//
//#if !defined(RADIO_GPIO_SDN) || !defined(RADIO_PIO_SDN) || !defined(RADIO_PIN_SDN)
//#error No Radio shutdown pin defined
//#endif
//
//#if !defined(VCXO_GPIO_EN) || !defined(VCXO_PIO_EN) || !defined(VCXO_PIN_EN)
//#error No VCXO enable pin defined
//#endif
//
//#if !defined(VCXO_GPIO_CTRL) || !defined(VCXO_PIO_CTRL)
//#error No VCXO PWM pin defined
//#endif
//
//#if !defined(VCXO_MR_CTRL)
//#error No VCXO PWM Match Register defined
//#endif
//
//#if !defined(GPS_GPIO_RESET) || !defined(GPS_PIO_RESET) || !defined(GPS_PIN_RESET)
//#error No GPS reset pin defined
//#endif



/* ------------------------------ Pin definitions ------------------------------ */
/* ----------------------- Please don't touch anything ------------------------ */

//#if ADC_REF == REF_VCC1V8_LDO
//	#define ADC_PIO_VCC1V8		ADC_PIO_REF
//	#define ADC_PIN_VCC1V8		ADC_PIN_REF
//#elif ADC_REF == REF_VCC
//	#define ADC_PIO_VCC			ADC_PIO_REF
//	#define ADC_PIN_VCC			ADC_PIN_REF
//#endif

#ifdef BMP180_AVAIL
	#define USE_I2C
#endif
#if GPS_BUS == I2C
	#define USE_I2C
#endif



#endif
