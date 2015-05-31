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
#include "Si446x.h"
#include "i2c.h"

// Module globals
static uint16_t telemetry_counter = 0;
static uint16_t loss_of_gps_counter = 0;

char gps_aprs_lat_old[]	= "0000.00N";
char gps_aprs_lon_old[]	= "00000.00W";
char gps_time_old[]		= "000000";

// changed from const  
s_address_t addresses[] =
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


// Module functions
float meters_to_feet(float m)
{
	// 10000 ft = 3048 m
	return m / 0.3048;
}

uint32_t addtime(uint32_t original, uint32_t seconds2add)
{
	uint32_t hours =   original / 10000;
	uint32_t minutes = (original % 10000) / 100;
	uint32_t seconds = (original % 10000) % 100;

	seconds += seconds2add;

	minutes += seconds / 60;
	seconds = seconds % 60;

	hours += minutes / 60;
	minutes = minutes % 60;
	hours = hours % 24;

	return 10000 * hours + 100 * minutes + seconds;
}  

// Exported functions
void aprs_send()
{
	char temp[12];
	uint32_t frequency;
	float altitude = 0;
	int16_t value;
	int8_t bmp180temp = 0;
	int32_t bmp180pressure = 0;

	#ifdef BMP_AVAIL
	BMP180_Init();
	bmp180temp = getTemperature() / 10;	// Read temperature in degree celcius
	bmp180pressure = getPressure();		// Read pressure in pascal
	BMP180_DeInit();
	#endif

	// Set radio power and frequency
	frequency = gps_get_region_frequency();
	modem_set_tx_freq(frequency);
	modem_set_tx_power(RADIO_POWER);

	// Send a telemetry package
	ax25_send_header(addresses, sizeof(addresses)/sizeof(s_address_t));
	ax25_send_string("T#");                // Telemetry Report
	telemetry_counter++;
	if (telemetry_counter > 999) {
		telemetry_counter = 0;
	}
	nsprintf(temp, 4, "%03d", telemetry_counter);
	ax25_send_string(temp);              // sequential id
	ax25_send_byte(',');

	// adc0 = Battery Voltage ; Write to T# pos 1
	ADC_Init();	// Initialize ADCs
	nsprintf(temp, 4, "%03d", getBattery8bit());
	ax25_send_string(temp);               // write 8 bit ADC value
	ax25_send_byte(',');


	// Temperature (+100 deg C for all positive values); Write to T# pos 2
	nsprintf(temp, 4, "%03d", (int)(bmp180temp + 100));
	ax25_send_string(temp);
	ax25_send_byte(',');


	// fill the 3rd telemetry value with a number that's proportional to the altitude:
	nsprintf(temp, 4, "%03d", (int32_t)abs((meters_to_feet(gps_altitude) + 0.5)/1000L)); // Altitude in kfeet; Must be > 0, therefore abs()
	ax25_send_string(temp);               // write 8 bit value
	ax25_send_byte(',');




	// adc6 = Usolar; Write to T# pos 4
	#ifdef SOLAR_AVAIL
	value = getSolar8bit();          // read ADC
	#else
	value = 0;
	#endif
	ADC_DeInit();
	nsprintf(temp, 4, "%03d", value);
	ax25_send_string(temp);               // write 8 bit ADC value
	ax25_send_byte(',');



	// Time to lock; Write to T# pos 5
	nsprintf(temp, 4, "%03d", time2lock); // TTL in cycle periods
	ax25_send_string(temp);               // write 8 bit value
	ax25_send_byte(',');

	// Here the APRS telemetry specification requires a 8 character long "binary" string
	// consisting of "1" or "0" characters

	// We'll encode the GPS loss counter in the next 4 bits
	// loss_of_gps_counter: 0 .. 14, (15 or more)
	int z;
	for (z = 8; z > 0; z >>= 1) {
		if ((loss_of_gps_counter & z) == z) {
			ax25_send_byte('1');
		} else {
			ax25_send_byte('0');
		}
	}
  
	// Next two bits are the signum of latitude/longitude
	// East = 1, West = 0
	// North = 1, South = 0
	if(gps_lat > 0) {
		ax25_send_byte('1');
	} else {
		ax25_send_byte('0');
	}

	if(gps_lon > 0) {
		ax25_send_byte('1');
	} else {
		ax25_send_byte('0');
	}


	// Check if the GPS is on or off at the time of transmission
	value = 1;          // read ADC

	if(value > 512) {
		ax25_send_byte('1');
	} else {
		ax25_send_byte('0');
	}
  

	ax25_send_byte('0');
 
	ax25_send_footer();
	ax25_flush_frame();                 // Tell the modem to go



	// Wait a few seconds (Else aprs.fi reports "[Rate limited (< 5 sec)]")
	delay(6000);


	ax25_send_header(addresses, sizeof(addresses)/sizeof(s_address_t));
	ax25_send_byte('/');                // Report w/ timestamp, no APRS messaging. $ = NMEA raw data

	altitude = gps_get_altitude();
  
	if(newPositionStillUnknown)
	{
		//use the old position
		strcpy(gps_aprs_lat, gps_aprs_lat_old);
		strcpy(gps_aprs_lon, gps_aprs_lon_old);

		nsprintf(gps_time, 7, "%06d", addtime(atol(gps_time_old), APRS_PERIOD_SECONDS));

		//altitude = 0; // Is this necessary? SSE

	}

	ax25_send_string(gps_time);         // 170915 = 17h:09m:15s zulu (not allowed in Status Reports)
	ax25_send_byte('h');
	ax25_send_string(gps_aprs_lat);     // Lat: 38deg and 22.20 min (.20 are NOT seconds, but 1/100th of minutes)
	ax25_send_byte(APRS_SYMBOL_TABLE);                // Symbol table
	ax25_send_string(gps_aprs_lon);     // Lon: 000deg and 25.80 min
	ax25_send_byte(APRS_SYMBOL_ID);                // Symbol: /O=balloon, /-=QTH, \N=buoy
	nsprintf(temp, 4, "%03d", (int16_t)(gps_course + 0.5));
	ax25_send_string(temp);             // Course (degrees)
	ax25_send_byte('/');                // and
	nsprintf(temp, 4, "%03d", (int16_t)(gps_speed + 0.5));
	ax25_send_string(temp);             // speed (knots)
	ax25_send_string("/A=");            // Altitude (feet). Goes anywhere in the comment area
	if (altitude < 0) { altitude = 0.0; }; // Negative altitudes are not displayed correctly at aprs.fi :(
	nsprintf(temp, 7, "%06ld", (int32_t)abs((meters_to_feet(altitude) + 0.5)));
	ax25_send_string(temp);
	ax25_send_string(" ");
    
	ADC_Init();
	itoa(getBatteryMV(), temp, 10);
	ax25_send_string(temp);
	ax25_send_string("Vb ");

	#ifdef SOLAR_AVAIL
	itoa(getSolarMV(), temp, 10);
	ax25_send_string(temp);
	ax25_send_string("Vs ");
	#endif
	ADC_DeInit();

	if (bmp180pressure > 0) {
		ax25_send_string(itoa(bmp180temp, temp, 10));
		ax25_send_string("C ");

		ax25_send_string(itoa(bmp180pressure, temp, 10));
		ax25_send_string("Pa ");
	}
	ax25_send_string("SATS");
	nsprintf(temp, 3, "%02d", gps_sats);
	ax25_send_string(temp);

	if(strcmp(APRS_COMMENT, "")) {
		ax25_send_string(" ");
		ax25_send_string(APRS_COMMENT);     // Comment
	}

	if(newPositionStillUnknown) {
		loss_of_gps_counter++;
		if(loss_of_gps_counter >= 15) { // make sure we don't get above 15
			loss_of_gps_counter = 15; // will stay at 15 at permanent gps loss. 15 is maximum due to telemetry overflow
		}
		ax25_send_string(" GPS loss ");
		nsprintf(temp, 3, "%02d", loss_of_gps_counter);
		ax25_send_string(temp);               // write 8 bit value
		gps_sats = 0;
	} else {
		loss_of_gps_counter = 0;
	}

	ax25_send_footer();
	ax25_flush_frame();                 // Tell the modem to go
	strcpy(gps_aprs_lat_old, gps_aprs_lat);
	strcpy(gps_aprs_lon_old, gps_aprs_lon);
	strcpy(gps_time_old, gps_time);
}
