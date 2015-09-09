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
#include "log.h"
#include "gps.h"
#include "aprs.h"
#include "adc.h"
#include "small_printf_code.h"
#include "global.h"
#include <string.h>
#include <stdlib.h>
#include "ssd1306.h"
#include "base64.h"

#define METER_TO_FEET(m) (((m)*26876) / 8192)


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
void transmit_telemetry(track_t *trackPoint)
{
	char temp[12];

	// Encode telemetry header
	ax25_send_header(addresses, sizeof(addresses)/sizeof(s_address_t));
	ax25_send_string("T#");
	nsprintf(temp, 4, "%03d", trackPoint->id % 255);
	ax25_send_string(temp);
	ax25_send_byte(',');

	// Encode battery voltage
	nsprintf(temp, 4, "%03d", trackPoint->vbat);
	ax25_send_string(temp);
	ax25_send_byte(',');

	// Encode temperature
	nsprintf(temp, 4, "%03d", trackPoint->temp + 100);
	ax25_send_string(temp);
	ax25_send_byte(',');

	// Encode altitude
	nsprintf(temp, 4, "%03d", METER_TO_FEET(trackPoint->altitude)/1000);
	ax25_send_string(temp);
	ax25_send_byte(',');

	// Encode solar voltage
	nsprintf(temp, 4, "%03d", trackPoint->vsol);
	ax25_send_string(temp);
	ax25_send_byte(',');

	// Encode TTFF (time to first fix in seconds)
	nsprintf(temp, 4, "%03d", trackPoint->ttff);
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
void transmit_position(track_t *trackPoint, gpsstate_t gpsstate, uint16_t course, uint16_t speed)
{
	char temp[22];
	date_t date = unixTimestamp2Date(trackPoint->time * 1000);

	ax25_send_header(addresses, sizeof(addresses)/sizeof(s_address_t));
	ax25_send_byte('/');                // Report w/ timestamp, no APRS messaging. $ = NMEA raw data

	nsprintf(temp, 7, "%02d%02d%02d", date.hour, date.minute, date.second);
	ax25_send_string(temp);         // 170915 = 17h:09m:15s zulu (not allowed in Status Reports)
	ax25_send_byte('h');
	uint16_t lat_degree = abs((int16_t)trackPoint->latitude);
	uint32_t lat_decimal = abs((int32_t)(trackPoint->latitude*100000))%100000;
	uint8_t lat_minute = lat_decimal * 6 / 10000;
	uint8_t lat_minute_dec = (lat_decimal * 6 / 100) % 100;
	nsprintf(temp, 9, "%02d%02d.%02d%c", lat_degree, lat_minute, lat_minute_dec, trackPoint->latitude>0 ? 'N' : 'S');
	ax25_send_string(temp);     // Lat: 38deg and 22.20 min (.20 are NOT seconds, but 1/100th of minutes)
	ax25_send_byte(APRS_SYMBOL_TABLE);                // Symbol table
	uint16_t lon_degree = abs((int16_t)trackPoint->longitude);
	uint32_t lon_decimal = abs((int32_t)(trackPoint->longitude*100000))%100000;
	uint8_t lon_minute = lon_decimal * 6 / 10000;
	uint8_t lon_minute_dec = (lon_decimal * 6 / 100) % 100;
	nsprintf(temp, 10, "%03d%02d.%02d%c", lon_degree, lon_minute, lon_minute_dec, trackPoint->longitude>0 ? 'E' : 'W');

	ax25_send_string(temp);     // Lon: 000deg and 25.80 min
	ax25_send_byte(APRS_SYMBOL_ID);                // Symbol: /O=balloon, /-=QTH, \N=buoy
	nsprintf(temp, 4, "%03d", course);
	ax25_send_string(temp);             // Course (degrees)
	ax25_send_byte('/');                // and
	nsprintf(temp, 4, "%03d", speed);
	ax25_send_string(temp);             // speed (knots)
	ax25_send_string("/A=");            // Altitude (feet). Goes anywhere in the comment area
	nsprintf(temp, 7, "%06ld", METER_TO_FEET(trackPoint->altitude));
	ax25_send_string(temp);
	ax25_send_string(" ");

	uint16_t vbat = EIGHTBIT_TO_VBAT(trackPoint->vbat);
	nsprintf(temp, 8, "%d.%02dVb ", vbat/1000, (vbat%1000)/10);
	ax25_send_string(temp);

	#ifdef SOLAR_AVAIL
	uint16_t vsol = EIGHTBIT_TO_VSOL(trackPoint->vsol);
	nsprintf(temp, 8, "%d.%02dVs ", vsol/1000, (vsol%1000)/10);
	ax25_send_string(temp);
	#endif

	ax25_send_string(itoa(trackPoint->temp, temp, 10));
	ax25_send_string("C ");

	#ifdef BMP180_AVAIL
	ax25_send_string(itoa(trackPoint->pressure, temp, 10));
	ax25_send_string("Pa ");
	#endif

	ax25_send_string("SATS");
	nsprintf(temp, 3, "%02d", trackPoint->satellites);
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
		ax25_send_string(temp);
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
		terminal_addLine("GPS lowbatt");
	} else if(gpsstate == GPS_LOCK) {
		nsprintf(temp, 22, "GPS lock (in %d sec)", trackPoint->ttff);
		terminal_addLine(temp);
	}

	nsprintf(
		temp, 22, "%c%02d%c%02d.%02d'%c%03d%c%02d.%02d'",
		(trackPoint->longitude < 0 ? 'S' : 'N'),
		lat_degree, 0xF8, lat_minute, lat_minute_dec,
		(trackPoint->latitude < 0 ? 'W' : 'E'),
		lon_degree, 0xF8, lon_minute, lon_minute_dec
	);
	terminal_addLine(temp);

	nsprintf(temp, 22, "TIM %02d-%02d-%02d %02d:%02d:%02d", date.year%100, date.month, date.day, date.hour, date.minute, date.second);
	terminal_addLine(temp);

	nsprintf(temp, 22, "ALT%6d m   SATS %d", trackPoint->altitude, trackPoint->satellites);
	terminal_addLine(temp);

	nsprintf(temp, 22, "BAT%5d mV%7d %cC", EIGHTBIT_TO_VBAT(trackPoint->vbat), trackPoint->temp, (char)0xF8);
	terminal_addLine(temp);

	#ifdef SOLAR_AVAIL
	nsprintf(temp, 22, "SOL%5d mV%7d Pa", EIGHTBIT_TO_VSOL(trackPoint->vsol), trackPoint->pressure);
	terminal_addLine(temp);
	#endif

	terminal_flush();

	// Transmit
	ax25_flush_frame();
}

/**
 * Transmit APRS log packet
 */
void transmit_log(track_t *trackPoint)
{
	// Encode telemetry header
	ax25_send_header(addresses, sizeof(addresses)/sizeof(s_address_t));
	ax25_send_string("{{L");

	// Encode log message
	uint8_t i;
	for(i=0; i<LOG_TRX_NUM; i++) {
		track_t *data = getNextLogPoint();
		uint8_t base64[BASE64LEN(sizeof(track_t))+1];
		base64_encode((uint8_t*)data, base64, sizeof(track_t));
		ax25_send_string((char*)base64);
	}

	// Send footer
	ax25_send_footer();

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

	nsprintf(temp, 22, "Sleep cycle:%6dsec", TIME_SLEEP_CYCLE);
	terminal_addLine(temp);
	nsprintf(temp, 22, "max GPS cycle:%4dsec", TIME_MAX_GPS_SEARCH);
	terminal_addLine(temp);
	terminal_flush();
}
