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

#include "config.h"
#include "ax25.h"
#include "gps.h"
#include "aprs.h"
#include "adc.h"
#include "bmp180.h"
#include "modem.h"
#include "small_printf_code.h"
#include "global.h"
#include <string.h>
#include <stdlib.h>
#include "i2c.h"
#include "ssd1306.h"

#define METER_TO_FEET(m) (((m)*26876) / 8192)


static uint16_t telemetry_counter = 0;
static uint16_t loss_of_gps_counter = 0;

const s_address_t addresses[] =
{ 
	{D_CALLSIGN, D_CALLSIGN_ID},  // Destination callsign
	{S_CALLSIGN, S_CALLSIGN_ID},  // Source callsign (-11 = balloon, -9 = car)
	#ifdef DIGI_PATH1
	{DIGI_PATH1, DIGI_PATH1_TTL}, // Digi1 (first digi in the chain)
	#endif
	#ifdef DIGI_PATH2
	{DIGI_PATH2, DIGI_PATH2_TTL}, // Digi2 (second digi in the chain)
	#endif
};

/**
 * Transmit APRS telemetry packet. The packet contain following values:
 * - Battery voltage in mV (has to be multiplied by 16, val=185 => 2960mV)
 * - Temperature in celcius (100 has to be subtracted, val=104 => 4 celcius)
 * - Altitude in feet (has to be multiplied by 1000, val=26 => 26000ft)
 * - Solar voltage in mV (has to be multiplied by 8, val=123 => 984mV)
 * - TTFF in seconds
 * Therafter is bitwise encoding:
 * - [7:4] Number of cycles where GPS has been lost
 * - [3:0] unused
 */
void transmit_telemetry(void)
{
	char temp[12];
	int16_t value;

	// Encode telemetry header
	ax25_send_header(addresses, sizeof(addresses)/sizeof(s_address_t));
	ax25_send_string("T#");
	telemetry_counter++;
	if (telemetry_counter > 255) {
		telemetry_counter = 0;
	}
	nsprintf(temp, 4, "%03d", telemetry_counter);
	ax25_send_string(temp);
	ax25_send_byte(',');

	ADC_Init();	// Initialize ADCs

	// Encode battery voltage
	nsprintf(temp, 4, "%03d", getBattery8bit());
	ax25_send_string(temp);               // write 8 bit ADC value
	ax25_send_byte(',');

	// Encode temperature
	#ifdef BMP180_AVAIL
	BMP180_Init();
	int8_t bmp180temp = getTemperature() / 10;	// Read temperature in degree celcius
	BMP180_DeInit();
	#else
	int8_t bmp180temp = 0;
	#endif
	nsprintf(temp, 4, "%03d", (int)(bmp180temp + 100));
	ax25_send_string(temp);
	ax25_send_byte(',');

	// Encode altitude
	nsprintf(temp, 4, "%03d", METER_TO_FEET(lastFix.altitude)/1000); // Altitude in kfeet; Must be > 0, therefore abs()
	ax25_send_string(temp);               // write 8 bit value
	ax25_send_byte(',');

	// Encode solar voltage
	#ifdef SOLAR_AVAIL
	value = getSolar8bit();
	#else
	value = 0;
	#endif
	ADC_DeInit();
	nsprintf(temp, 4, "%03d", value);
	ax25_send_string(temp);
	ax25_send_byte(',');

	// Encode TTFF (time to first fix)
	nsprintf(temp, 4, "%03d", lastFix.ttff); // TTFF in seconds
	ax25_send_string(temp);
	ax25_send_byte(',');

	// Encode bitwise
	// [7:4] Number of cycles where GPS has been lost
	// [3:0] unused

	// Encode count of GPS losses
	int z;
	for (z = 8; z > 0; z >>= 1) {
		if ((loss_of_gps_counter & z) == z) {
			ax25_send_byte('1');
		} else {
			ax25_send_byte('0');
		}
	}

	// Filling up unused bits
	ax25_send_byte('0');
	ax25_send_byte('0');
	ax25_send_byte('0');
	ax25_send_byte('0');

	ax25_send_footer();

	// Print debug message
	if(!S_CALLSIGN_ID) {
		nsprintf(temp, 22, "> Send Tele.%9s", S_CALLSIGN);
	} else {
		nsprintf(temp, 22, "> Send Tele.%6s-%d", S_CALLSIGN, S_CALLSIGN_ID);
	}
	terminal_clear();
	terminal_addLine(temp);
	terminal_flush();

	// Transmit
	ax25_flush_frame();
}

/**
 * Transmit APRS position packet. The comments are filled with:
 * - Static comment (can be set in config.h)
 * - Battery voltage in mV
 * - Solar voltage in mW (if tracker is solar-enabled)
 * - Temperature in Celcius
 * - Air pressure in Pascal
 * - Number of satellites being used
 * - Number of cycles where GPS has been lost (if applicable in cycle)
 */
void transmit_position(gpsstate_t gpsstate)
{
	char temp[22];
	int8_t bmp180temp = 0;
	int32_t bmp180pressure = 0;

	#ifdef BMP180_AVAIL
	BMP180_Init();
	bmp180temp = getTemperature() / 10;	// Read temperature in degree celcius
	bmp180pressure = getPressure();		// Read pressure in pascal
	BMP180_DeInit();
	#endif

	ADC_Init();
	uint16_t battery = getBatteryMV();
	#ifdef SOLAR_AVAIL
	uint16_t solar = getSolarMV();
	#endif
	ADC_DeInit();

	ax25_send_header(addresses, sizeof(addresses)/sizeof(s_address_t));
	ax25_send_byte('/');                // Report w/ timestamp, no APRS messaging. $ = NMEA raw data

	if(gpsstate != GPS_LOCK)
	{
		lastFix.time = unixTimestamp2Date(getUnixTimestamp()); // Replace old GPS timestamp with current time
	}

	nsprintf(temp, 7, "%02d%02d%02d", lastFix.time.hour, lastFix.time.minute, lastFix.time.second);
	ax25_send_string(temp);         // 170915 = 17h:09m:15s zulu (not allowed in Status Reports)
	ax25_send_byte('h');
	uint16_t lat_degree = abs((int16_t)lastFix.latitude);
	uint32_t lat_decimal = abs((int32_t)(lastFix.latitude*100000))%100000;
	uint8_t lat_minute = lat_decimal * 6 / 10000;
	uint8_t lat_minute_dec = (lat_decimal * 6 / 100) % 100;
	nsprintf(temp, 9, "%02d%02d.%02d%c", lat_degree, lat_minute, lat_minute_dec, lastFix.latitude>0 ? 'N' : 'S');
	ax25_send_string(temp);     // Lat: 38deg and 22.20 min (.20 are NOT seconds, but 1/100th of minutes)
	ax25_send_byte(APRS_SYMBOL_TABLE);                // Symbol table
	uint16_t lon_degree = abs((int16_t)lastFix.longitude);
	uint32_t lon_decimal = abs((int32_t)(lastFix.longitude*100000))%100000;
	uint8_t lon_minute = lon_decimal * 6 / 10000;
	uint8_t lon_minute_dec = (lon_decimal * 6 / 100) % 100;
	nsprintf(temp, 10, "%03d%02d.%02d%c", lon_degree, lon_minute, lon_minute_dec, lastFix.longitude>0 ? 'E' : 'W');

	ax25_send_string(temp);     // Lon: 000deg and 25.80 min
	ax25_send_byte(APRS_SYMBOL_ID);                // Symbol: /O=balloon, /-=QTH, \N=buoy
	nsprintf(temp, 4, "%03d", lastFix.course);
	ax25_send_string(temp);             // Course (degrees)
	ax25_send_byte('/');                // and
	nsprintf(temp, 4, "%03d", lastFix.speed);
	ax25_send_string(temp);             // speed (knots)
	ax25_send_string("/A=");            // Altitude (feet). Goes anywhere in the comment area
	nsprintf(temp, 7, "%06ld", METER_TO_FEET(lastFix.altitude));
	ax25_send_string(temp);
	ax25_send_string(" ");
    
	itoa(battery, temp, 10);
	ax25_send_string(temp);
	ax25_send_string("mVb ");

	#ifdef SOLAR_AVAIL
	itoa(solar, temp, 10);
	ax25_send_string(temp);
	ax25_send_string("mVs ");
	#endif

	if (bmp180pressure > 0) {
		ax25_send_string(itoa(bmp180temp, temp, 10));
		ax25_send_string("C ");

		ax25_send_string(itoa(bmp180pressure, temp, 10));
		ax25_send_string("Pa ");
	}
	ax25_send_string("SATS");
	nsprintf(temp, 3, "%02d", lastFix.satellites);
	ax25_send_string(temp);

	#ifdef APRS_COMMENT
	ax25_send_string(" ");
	ax25_send_string(APRS_COMMENT); // Comment
	#endif

	if(gpsstate != GPS_LOCK) {
		if(loss_of_gps_counter >= 5) { // GPS lost 3 times (6min if cycle = 2min) TODO: This is actually not a task of APRS encoding
			loss_of_gps_counter = 0;

			GPS_PowerOff();	// Reset UART interface
			gps_reset();	// Reset GPS
			GPS_Init();		// Reinit GPS
		}
		loss_of_gps_counter++;
		ax25_send_string(" GPS loss ");
		nsprintf(temp, 3, "%02d", loss_of_gps_counter);
		ax25_send_string(temp);               // write 8 bit value
		lastFix.satellites = 0;
	} else {
		loss_of_gps_counter = 0;
	}

	ax25_send_footer();

	// Print debug message
	if(!S_CALLSIGN_ID) {
		nsprintf(temp, 22, "> Send Pos.%10s", S_CALLSIGN);
	} else {
		nsprintf(temp, 22, "> Send Pos.%7s-%d", S_CALLSIGN, S_CALLSIGN_ID);
	}
	terminal_addLine(temp);

	if(gpsstate == GPS_LOSS) {
		terminal_addLine("GPS loss");
	} else if(gpsstate == GPS_LOW_BATT) {
		terminal_addLine("GPS lowbatt power off");
	} else if(gpsstate == GPS_LOCK) {
		nsprintf(temp, 22, "GPS lock (in %d sec)", lastFix.ttff);
		terminal_addLine(temp);
	}

	nsprintf(
		temp, 22, "%c%02d%c%02d.%02d'%c%03d%c%02d.%02d'",
		(lastFix.longitude < 0 ? 'S' : 'N'),
		lat_degree, 0xF8, lat_minute, lat_minute_dec,
		(lastFix.latitude < 0 ? 'W' : 'E'),
		lon_degree, 0xF8, lon_minute, lon_minute_dec
	);
	terminal_addLine(temp);

	nsprintf(temp, 22, "TIM %02d-%02d-%02d %02d:%02d:%02d", lastFix.time.year%100, lastFix.time.month, lastFix.time.day, lastFix.time.hour, lastFix.time.minute, lastFix.time.second);
	terminal_addLine(temp);

	nsprintf(temp, 22, "ALT%6d m   SATS %d", lastFix.altitude, lastFix.satellites);
	terminal_addLine(temp);

	nsprintf(temp, 22, "BAT%5d mV%7d %cC", battery, bmp180temp, (char)0xF8);
	terminal_addLine(temp);

	#ifdef SOLAR_AVAIL
	nsprintf(temp, 22, "SOL%5d mV%7d Pa", solar, bmp180pressure);
	terminal_addLine(temp);
	#endif

	terminal_flush();

	// Transmit
	ax25_flush_frame();
}

void display_configuration(void)
{
	char temp[22];

	if(TARGET == TARGET_PECAN_PICO6) {
		terminal_addLine("> Start Pecan Pico 6");
	} else if(TARGET == TARGET_PECAN_FEMTO2_1) {
		terminal_addLine("> Start Pecan Femto 2");
	}
	if(!S_CALLSIGN_ID) {
		nsprintf(temp, 22, "Call:%16s", S_CALLSIGN);
	} else {
		nsprintf(temp, 22, "Call:%13s-%d", S_CALLSIGN, S_CALLSIGN_ID);
	}
	terminal_addLine(temp);
	#if !defined(DIGI_PATH1) && !defined(DIGI_PATH2)
	nsprintf(temp, 22, "Path:            none");
	#elif defined(DIGI_PATH1) && !defined(DIGI_PATH2)
	nsprintf(temp, 22, "Path:%14s-%d", DIGI_PATH1, DIGI_PATH1_TTL);
	#else
	nsprintf(temp, 22, "Path: %s-%d,%s-%d", DIGI_PATH1, DIGI_PATH1_TTL, DIGI_PATH2, DIGI_PATH2_TTL);
	#endif
	terminal_addLine(temp);

	nsprintf(temp, 22, "Radio power:%9d", RADIO_POWER);
	terminal_addLine(temp);

	if(BATTERY_TYPE == PRIMARY) {
		terminal_addLine("Battery type:     PRI");
	} else {
		terminal_addLine("Battery type:     SEC");
	}

	nsprintf(temp, 22, "Sleep cycle:%6dsec", TIME_SLEEP_CYCLE/1000);
	terminal_addLine(temp);
	nsprintf(temp, 22, "max GPS cycle:%4dsec", TIME_MAX_GPS_SEARCH/1000);
	terminal_addLine(temp);
	terminal_flush();
}
