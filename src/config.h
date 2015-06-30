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
#define DIGI_PATH1				"WIDE2"
#define DIGI_PATH1_TTL			1
//#define DIGI_PATH2				"WIDE1"
//#define DIGI_PATH2_TTL			1

// APRS comment: this goes in the comment portion of the APRS message. You
// might want to keep this short. The longer the packet, the more vulnerable
// it is to noise. 
#define APRS_COMMENT    ""

// TX delay in milliseconds
#define TX_DELAY					300


#define TIME_SLEEP_CYCLE			120000
#define TIME_MAX_GPS_SEARCH			120000

#define TARGET						TARGET_PECAN_FEMTO2_1

// Battery type
// PRIMARY		LiFeSe2 Power save modes disabled, battery will be used until completely empty
// SECONDARY	LiFePO4 GPS will be kept off below 2700mV, no transmission is made below 2500mV to keep the accumulator healthy
#define BATTERY_TYPE				PRIMARY

// Frequency (which is used after reset state)
#define DEFAULT_FREQUENCY			144800000

// Radio power
// min. 1, max. 127
// Radio output power depends on VCC voltage
// 127 @ VCC=3400mV ~ 100mW
// 20  @ VCC=3400mV ~ 10mW
#define RADIO_POWER					10


/* ---------------------------- Target definitions ---------------------------- */
/* ----------------------- Please don't touch anything ------------------------ */

#if TARGET == TARGET_PECAN_FEMTO2_1

	#define USE_GPS_SW_SW						// GPS software switch activated
	#define GPS_BUS				BUS_UART		// Use UART bus for GPS communication
	#define GPS_BAUDRATE		9600			// Baudrate for ublox MAX7 or MAX8
	#define ADC_REF				REF_VCC			// ADC reference VCC input
	#define REF_MV				3300			// Reference voltage in mv

	#define ADC_PIO_BATT		R_PIO1_2
	#define ADC_AD_BATT			AD3

	#define UART_PIO_RXD		PIO3_5
	#define UART_PIO_TXD		PIO3_4

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

	#define VCXO_PIO_CTRL		PIO1_6
	#define VCXO_MR_CTRL		MR0

	#define RADIO_GPIO_GPIO0	LPC_GPIO1
	#define RADIO_PIO_GPIO0		PIO1_7
	#define RADIO_PIN_GPIO0		(1 << 7)

	#define OSC_FREQ(u)			26000000		// Oscillator frequency

	#if BATTERY_TYPE == SECONDARY
	#error Pecan Femto v2.1 can be only used whith primary batteries
	#endif

#elif TARGET == TARGET_PECAN_PICO6

	#define USE_GPS_HW_SW						// GPS transistor switch available
	#define EXT_PIN_AVAIL						// External pins available
	#define SOLAR_AVAIL							// Solar feed available
	#define BMP180_AVAIL						// Pressure sensor BMP180 is available
	#define I2C_PULLUPS_AVAIL					// Controlable I2C Pullups available
	#define VCXO_POWERED_BY_LDO					// VCXO powered by LDO (VCC1V8)
	#define GPS_BUS				BUS_UART		// Use UART bus for GPS communication
	#define GPS_BAUDRATE		9600			// Baudrate for ublox MAX7 or MAX8

	#define ADC_REF				REF_VCC1V8_LDO	// ADC reference is 1.8V LDO
	#define ADC_PIO_REF			R_PIO1_1
	#define ADC_AD_REF			AD2
	#define ADC_PIO_SOLAR		R_PIO1_0
	#define ADC_AD_SOLAR		AD1

	#define LDO_GPIO_EN			LPC_GPIO1
	#define LDO_PIO_EN			R_PIO1_2
	#define LDO_PIN_EN			(1 << 2)

	#define I2C_GPIO_PULL_VCC	LPC_GPIO1
	#define I2C_PIO_PULL_VCC	PIO1_9
	#define I2C_PIN_PULL_VCC	(1 << 9)

	#define UART_PIO_RXD		PIO1_6
	#define UART_PIO_TXD		PIO1_7

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

	#define VCXO_PIO_CTRL		PIO0_11
	#define VCXO_MR_CTRL		MR3

	#define RADIO_GPIO_GPIO0	LPC_GPIO3
	#define RADIO_PIO_GPIO0		PIO3_4
	#define RADIO_PIN_GPIO0		(1 << 4)

	#define RADIO_GPIO_GPIO1	LPC_GPIO3
	#define RADIO_PIO_GPIO1		PIO3_5
	#define RADIO_PIN_GPIO1		(1 << 5)

	#define EXT_GPIO_OUT1		LPC_GPIO1
	#define EXT_PIO_OUT1		PIO1_4
	#define EXT_PIN_OUT1		(1 << 4)

	#define EXT_GPIO_OUT2		LPC_GPIO1
	#define EXT_PIO_OUT2		PIO1_11
	#define EXT_PIN_OUT2		(1 << 11)

	#define EXT_GPIO_OUT3		LPC_GPIO3
	#define EXT_PIO_OUT3		PIO3_2
	#define EXT_PIN_OUT3		(1 << 2)

	#define EXT_GPIO_OUT4		LPC_GPIO1
	#define EXT_PIO_OUT4		PIO1_5
	#define EXT_PIN_OUT4		(1 << 5)

	#define I2C_GPIO_PULL_VCC	LPC_GPIO1
	#define I2C_PIO_PULL_VCC	PIO1_9
	#define I2C_PIN_PULL_VCC	(1 << 9)

	#define GPS_GPIO_EN			LPC_GPIO1
	#define GPS_PIO_EN			PIO1_8
	#define GPS_PIN_EN			(1 << 8)

	// Thomas DL4MDW and me DL7AD are using two different oscillators
	//#define OSC_FREQ(u)			((u*623/1024)+19997384)	// Oscillator frequency 20MHz !R10=3k3k!
	#define OSC_FREQ(u)			((u*3024/1024)+26990164)	// Oscillator frequency 27MHz !R10=10k!
#else
	#error No/incorrect target selected
#endif

#if BATTERY_TYPE == SECONDARY
	#define VOLTAGE_NOGPS		2700			// Don't switch on GPS below this voltage (Telemetry transmission only)
	#define VOLTAGE_NOTRANSMIT	2500			// Don't transmit below this voltage
	#define VOLTAGE_GPS_MAXDROP 100				// Max. Battery drop voltage until GPS is switched off while acquisition
												// Example: VOLTAGE_NOGPS = 2700 & VOLTAGE_GPS_MAXDROP = 100 => GPS will be switched
												// off at 2600mV, GPS will not be switched on if battery voltage already below 2700mV
#elif BATTERY_TYPE == PRIMARY
	#define VOLTAGE_NOGPS		0				// All battery saving options are switched off, so battery will be used until completely empty
	#define VOLTAGE_NOTRANSMIT	0
	#define VOLTAGE_GPS_MAXDROP 0
#endif



/* ------------------------------ Error messages ------------------------------ */
/* ----------------------- Please don't touch anything ------------------------ */


// TODO: Rewrite configuration validation again and check which checks are missing


/* ---------------------------- Misc definitions ------------------------------ */
/* ----------------------- Please don't touch anything ------------------------ */

#ifdef BMP180_AVAIL
	#define USE_I2C
#endif
#if GPS_BUS == I2C
	#define USE_I2C
#endif


// Assigning default values if not set
#ifndef VOLTAGE_NOGPS
#define VOLTAGE_NOGPS 0
#endif

#ifndef VOLTAGE_NOTRANSMIT
#define VOLTAGE_NOTRANSMIT 0
#endif

#ifndef VOLTAGE_GPS_MAXDROP
#define VOLTAGE_GPS_MAXDROP 100
#endif


/* ---------- Constant definitions (which will never change in life) ---------- */
/* ----------------------- Please don't touch anything ------------------------ */

// In other regions the APRS frequencies are different. Our balloon may travel
// from one region to another, so we may QSY according to GPS defined geographical regions
// Here we set some regional frequencies:

#define RADIO_FREQUENCY_REGION1		144800000 // Europe & Africa
#define RADIO_FREQUENCY_REGION2		144390000 // North and south America (Brazil is different)

#define RADIO_FREQUENCY_JAPAN		144660000
#define RADIO_FREQUENCY_CHINA		144640000
#define RADIO_FREQUENCY_BRAZIL		145570000
#define RADIO_FREQUENCY_AUSTRALIA	145175000
#define RADIO_FREQUENCY_NEWZEALAND	144575000
#define RADIO_FREQUENCY_THAILAND	145525000

#endif
