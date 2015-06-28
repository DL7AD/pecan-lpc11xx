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

#include "gps.h"
#include "uart.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "global.h"
#include "LPC11xx.h"
#include "config.h"

#define PI	3.1415926536

#define GPS_RESET(Select) { \
	if (Select) \
		GPS_GPIO_RESET->DATA &= ~GPS_PIN_RESET; \
	else \
		GPS_GPIO_RESET->DATA |= GPS_PIN_RESET; \
}

#define GPS_EN_SET(Select) { \
	if (Select) \
		GPS_GPIO_EN->DATA |= GPS_PIN_EN; \
	else \
		GPS_GPIO_EN->DATA &= ~GPS_PIN_EN; \
}

#define UBX_SETNAV { \
	0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06, 0x03, \
	0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, \
	0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, \
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	0x00, 0x00, 0x16, 0xDC \
}

#define UBX_SETGLLOFF { \
	0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x01, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x01, 0x01, 0x2B \
}
#define UBX_SETGSAOFF { \
	0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x02, 0x00, 0x00, \
	0x00, 0x00, 0x00, 0x01, 0x02, 0x32 \
}
#define UBX_SETGSVOFF { \
	0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x03, 0x00, 0x00, \
	0x00, 0x00, 0x00, 0x01, 0x03, 0x39 \
}
#define UBX_SETVTGOFF { \
	0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x05, 0x00, 0x00, \
	0x00, 0x00, 0x00, 0x01, 0x05, 0x47 \
}
#define UBX_POWEROFF { \
	0xB5, 0x62, 0x02, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, \
	0x02, 0x00, 0x00, 0x00, 0x4D, 0x3B \
}
#define UBX_POWERON { \
	0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x00, 0x00, 0x09, 0x00, \
	0x17, 0x76 \
}

//Max Performance Mode (default)
#define UBX_SETMAXPERFORMANCE { \
		0xB5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x08, 0x00, 0x21, 0x91 \
}

//Power Save Mode
#define UBX_SETPOWERSAVEMODE { \
		0xB5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x08, 0x01, 0x22, 0x92 \
}

#define UBX_SETCYCLICTRACKING { \
	0xB5, 0x62, 0x06, 0x3B, 0x2C, 0x00, 0x01, 0x06, 0x00, 0x00, \
	0x00, 0x90, 0x02, 0x00, 0xE8, 0x03, 0x00, 0x00, 0x10, 0x27, \
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	0x2C, 0x01, 0x00, 0x00, 0x4F, 0xC1, 0x03, 0x00, 0x86, 0x02, \
	0x00, 0x00, 0xFE, 0x00, 0x00, 0x00, 0x64, 0x40, 0x01, 0x00, \
	0x93, 0x95 \
}

#define UBX_SETCYCLICTRACKINGLONG { \
	0xB5, 0x62, 0x06, 0x3B, 0x2C, 0x00, 0x01, 0x06, 0x00, 0x00, \
	0x00, 0x90, 0x02, 0x00, 0x10, 0x27, 0x00, 0x00, 0xE0, 0x93, \
	0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	0x2C, 0x01, 0x00, 0x00, 0x4F, 0xC1, 0x03, 0x00, 0x86, 0x02, \
	0x00, 0x00, 0xFE, 0x00, 0x00, 0x00, 0x64, 0x40, 0x01, 0x00, \
	0x1F, 0xAD \
}

// Module declarations
static void parse_sentence_type(const char * token);
static void parse_time(const char *token);
static void parse_sats(const char *token);
static void parse_date(const char *token);
static void parse_status(const char *token);
static void parse_lat(const char *token);
static void parse_lat_hemi(const char *token);
static void parse_lon(const char *token);
static void parse_lon_hemi(const char *token);
static void parse_speed(const char *token);
static void parse_course(const char *token);
static void parse_altitude(const char *token);



// Module types
typedef void (*t_nmea_parser)(const char *token);

typedef enum {
	SENTENCE_UNK, // unknown packet
	SENTENCE_GGA, // GGA packet
	SENTENCE_RMC  // RMC packet
} sentence_t;


// Module constants
static const t_nmea_parser unk_parsers[] = {
	parse_sentence_type,    // $GPxxx
};

static const t_nmea_parser gga_parsers[] = {
	NULL,             // $GPGGA
	parse_time,       // Time
	NULL,             // Latitude
	NULL,             // N/S
	NULL,             // Longitude
	NULL,             // E/W
	NULL,             // Fix quality
	parse_sats,       // Number of satellites
	NULL,             // Horizontal dilution of position
	parse_altitude,   // Altitude
	NULL,             // "M" (mean sea level)
	NULL,             // Height of GEOID (MSL) above WGS84 ellipsoid
	NULL,             // "M" (mean sea level)
	NULL,             // Time in seconds since the last DGPS update
	NULL              // DGPS station ID number
};

static const t_nmea_parser rmc_parsers[] = {
	NULL,             // $GPRMC
	parse_time,       // Time
	parse_status,     // A=active, V=void
	parse_lat,        // Latitude,
	parse_lat_hemi,   // N/S
	parse_lon,        // Longitude
	parse_lon_hemi,   // E/W
	parse_speed,      // Speed over ground in knots
	parse_course,     // Track angle in degrees (true)
	parse_date,       // Date (DDMMYY)
	NULL,             // Magnetic variation
	NULL              // E/W
};

static const int16_t NUM_OF_UNK_PARSERS = (sizeof(unk_parsers) / sizeof(t_nmea_parser));
static const int16_t NUM_OF_GGA_PARSERS = (sizeof(gga_parsers) / sizeof(t_nmea_parser));
static const int16_t NUM_OF_RMC_PARSERS = (sizeof(rmc_parsers) / sizeof(t_nmea_parser));

gps_t newFix;
gps_t lastFix;
uint64_t time_lastRMCpacket;
uint64_t time_lastGGApacket;

// Module variables
static sentence_t sentence_type = SENTENCE_UNK;
static bool at_checksum = false;
static unsigned char our_checksum = '$';
static unsigned char their_checksum = 0;
static char token[16];
static int16_t num_tokens = 0;
static uint16_t offset = 0;
//static bool active = false;
//static char gga_time[7], rmc_time[7], rmc_date[7];
//static char new_time[7], new_date[7];
//static int16_t new_sats, gga_sats;
//static float new_lat;
//static float new_lon;
//static char new_aprs_lat[9];
//static char new_aprs_lon[10];
//static float new_course;
//static float new_speed;
//static float new_altitude;

// Public (extern) variables, readable from other modules
//char gps_time[8];       // HHMMSS
//char gps_date[7];       // DDMMYY
//int16_t gps_sats = 0;
//float gps_lat = 0;
//float gps_lon = 0;
//char gps_aprs_lat[10];
//char gps_aprs_lon[11];
//float gps_course = 0;
//float gps_speed = 0;
//float gps_altitude = -1000.0; //The dead sea at -420 m is theoretically the deepest usable spot on earth where you could use a GPS
//                              //Here we define -1000 m as our invalid gps altitude
//uint8_t time2lock;

bool isOn = false;

// Module functions
unsigned char from_hex(char a)
{
	if (a >= 'A' && a <= 'F')
		return a - 'A' + 10;
	else if (a >= 'a' && a <= 'f')
		return a - 'a' + 10;
	else if (a >= '0' && a <= '9')
		return a - '0';
	else
		return 0;
}

void parse_sentence_type(const char *token)
{
	if (strcmp(token, "$GPGGA") == 0 || strcmp(token, "$GNGGA") == 0) {
		sentence_type = SENTENCE_GGA;
	} else if(strcmp(token, "$GPRMC") == 0 || strcmp(token, "$GNRMC") == 0) {
		sentence_type = SENTENCE_RMC;
	} else if(strcmp(token, "$GPUNK") == 0 || strcmp(token, "$GNUNK") == 0) {
		sentence_type = SENTENCE_UNK;
	}
}

void parse_time(const char *token)
{
	// Time can have decimals (fractions of a second), but we only take HHMMSS
	char timestr[7];
	strncpy(timestr, token, 6);
	uint32_t timeint = atoi(timestr);
	newFix.time.hour = timeint / 10000;
	newFix.time.minute = (timeint % 10000) / 100;
	newFix.time.second = timeint % 100;
}

void parse_date(const char *token)
{
	// Date in DDMMYY
	char datestr[7];
	strncpy(datestr, token, 6);
	uint32_t dateint = atoi(datestr);
	newFix.time.day = dateint / 10000;
	newFix.time.month = (dateint % 10000) / 100;
	newFix.time.year = (dateint % 100) + 2000;
}

void parse_sats(const char *token)
{
	// Date in DDMMYY
	newFix.satellites = atoi(token);
}


void parse_status(const char *token)
{
	// "A" = active, "V" = void. We shoud disregard void sentences
	newFix.active = strcmp(token, "A") == 0;
}

void parse_lat(const char *token)
{
	// Parses latitude in the format "DD" + "MM" (+ ".M{...}M")
	char degs[3];
	char mins[3];
	if (strlen(token) >= 4) {
		degs[0] = token[0];
		degs[1] = token[1];
		degs[2] = '\0';
		mins[0] = token[2];
		mins[1] = token[3];
		mins[2] = '\0';
		newFix.latitude = atoi(degs) + atoi(mins) / 60.0 + atoi(token + 5) / 3600.0;
	}
}

void parse_lat_hemi(const char *token)
{
	if(token[0] == 'S')
		newFix.latitude = -newFix.latitude;
}

void parse_lon(const char *token)
{
	// Longitude is in the format "DDD" + "MM" (+ ".M{...}M")
	char degs[4];
	char mins[3];
	if(strlen(token) >= 5) {
		degs[0] = token[0];
		degs[1] = token[1];
		degs[2] = token[2];
		degs[3] = '\0';
		mins[0] = token[3];
		mins[1] = token[4];
		mins[2] = '\0';
		newFix.longitude = atoi(degs) + atoi(mins) / 60.0 + atoi(token + 6) / 3600.0;
	}
}

void parse_lon_hemi(const char *token)
{
	if(token[0] == 'W')
		newFix.longitude = -newFix.longitude;
}

void parse_speed(const char *token)
{
	newFix.speed = atoi(token);
}

void parse_course(const char *token)
{
	newFix.course = atoi(token);
}

void parse_altitude(const char *token)
{
	newFix.altitude = atoi(token);
}


// Exported functions

void GPS_Init() {
	// Configure pins
	LPC_IOCON->GPS_PIO_RESET = 0x30;		// GPS reset pin
	GPS_GPIO_RESET->DIR |= GPS_PIN_RESET;	// Set output

	#ifdef USE_GPS_HW_SW
	LPC_IOCON->GPS_PIO_EN = 0x30;			// GPS enable pin
	GPS_GPIO_EN->DIR |= GPS_PIN_EN;			// Set output
	#endif

	UART_Init(GPS_BAUDRATE);				// Init UART

	GPS_PowerOn();
	gps_setMaxPerformance();
}

void GPS_PowerOn(void) {
	gps_resetBuffer();					// Reset GPS buffer
	gps_hw_switch(true);				// Power up GPS
	gps_sw_switch(true);				// Switch on GPS

	gps_setNavigationMode();			// Set navigation mode
	gps_setNMEAstrings();				// Switch off unnecessary NMES strings
	//gps_setMaxPerformance();

	isOn = true;
}

void GPS_PowerOff(void) {
	gps_sw_switch(false);				// Switch off GPS
	gps_hw_switch(false);				// Power down GPS
	UART_DeInit();						// Power off UART

	isOn = false;
}

void gps_setMaxPerformance()
{
	uint8_t i;
	uint8_t message[] = UBX_SETMAXPERFORMANCE;
	for(i=0; i<sizeof(message); i++)
		UART_TransmitChar(message[i]);
}

void gps_setPowerSaveMode()
{
	uint8_t i;
	uint8_t message[] = UBX_SETPOWERSAVEMODE;
	for(i=0; i<sizeof(message); i++)
		UART_TransmitChar(message[i]);

	uint8_t message2[] = UBX_SETCYCLICTRACKINGLONG;
	for(i=0; i<sizeof(message2); i++)
		UART_TransmitChar(message2[i]);
}

void gps_setNavigationMode()
{
	uint8_t i;
	uint8_t message[] = UBX_SETNAV;
	for(i=0; i<sizeof(message); i++)
		UART_TransmitChar(message[i]);
}

void gps_setNMEAstrings() {
	uint8_t i;

	uint8_t message2[] = UBX_SETGLLOFF;
	for(i=0; i<sizeof(message2); i++)
		UART_TransmitChar(message2[i]);

	uint8_t message3[] = UBX_SETGSAOFF;
	for(i=0; i<sizeof(message3); i++)
		UART_TransmitChar(message3[i]);

	uint8_t message4[] = UBX_SETGSVOFF;
	for(i=0; i<sizeof(message4); i++)
		UART_TransmitChar(message4[i]);

	uint8_t message5[] = UBX_SETVTGOFF;
	for(i=0; i<sizeof(message5); i++)
		UART_TransmitChar(message5[i]);
}

void gps_resetBuffer()
{
	// TODO
	//strcpy(gps_time, "000000");
	//strcpy(gps_date, "000000");
	//strcpy(gps_aprs_lat, "0000.00N");
	//strcpy(gps_aprs_lon, "00000.00E");
	//time2lock = 0;
}


bool gps_decode(char c)
{
	int16_t ret = false;

	switch(c) {
		case '\r':
		case '\n':
			// End of sentence

			if (num_tokens && our_checksum == their_checksum) {
				// Return a valid position only when we've got two rmc and gga
				// messages with the same timestamp.

				uint64_t time = date2UnixTimestamp(
					newFix.time.year,	// Year
					newFix.time.month,	// Month
					newFix.time.day,	// Day
					newFix.time.hour,	// Hour
					newFix.time.minute,	// Minute
					newFix.time.second,	// Second
					0					// Millisecond
				);

				switch (sentence_type) {
					case SENTENCE_UNK: // Unkown packet type
						break; // Keeps gcc happy
					case SENTENCE_GGA:
						time_lastGGApacket = time; // Mark timestamp of last GGA packet
						lastFix.satellites = newFix.satellites;
						break;
					case SENTENCE_RMC:
						time_lastRMCpacket = time; // Mark timestamp of last RMC packet
						if(newFix.time.year != 0) { // gps has valid time when date != 0
							//setUnixTimestamp(time); TODO: TMP
						}
						break;
				}

				// Valid position scenario:
				//
				// 1. The timestamps of the two previous GGA/RMC sentences must match.
				//
				// 2. We just processed a known (GGA/RMC) sentence. Suppose the
				//    contrary: after starting up this module, gga_time and rmc_time
				//    are both equal (they're both initialized to ""), so (1) holds
				//    and we wrongly report a valid position.
				//
				// 3. The GPS has a valid fix. For some reason, the Venus 634FLPX
				//    reports 24 deg N, 121 deg E (the middle of Taiwan) until a valid
				//    fix is acquired:
				//
				//    $GPGGA,120003.000,2400.0000,N,12100.0000,E,0,00,0.0,0.0,M,0.0,M,,0000**69 (OK!)
				//    $GPGSA,A,1,,,,,,,,,,,,,0.0,0.0,0.0**30 (OK!)
				//    $GPRMC,120003.000,V,2400.0000,N,12100.0000,E,000.0,000.0,280606,,,N**78 (OK!)
				//    $GPVTG,000.0,T,,M,000.0,N,000.0,K,N**02 (OK!)
				//
				// 4.) We also want a clean altitude being reported.
				//     When is an altitude clean? Theoretically we could launch a balloon from the dead sea
				//     420 m below sea level. Therefore we define -1000 m as our invalid altitude and
				//     expect to find an altitude above -420 m when we've decoded a valid GGA statement.

				if (sentence_type != SENTENCE_UNK &&				// Known sentence?
						time_lastRMCpacket == time_lastGGApacket &&	// RMC/GGA times match?
						newFix.active &&							// Valid fix?
						newFix.satellites > 2 &&					// 3 sats or more?
						newFix.altitude > -1000.0) {				// Valid new altitude?

//					// Atomically merge data from the two sentences
//					//strcpy(gps_time, new_time);
//					//strcpy(gps_date, new_date);
//					gps_lat = new_lat;
//					gps_lon = new_lon;
//					strcpy(gps_aprs_lat, new_aprs_lat);
//					strcpy(gps_aprs_lon, new_aprs_lon);
//					gps_course = new_course;
//					gps_speed = new_speed;
//					gps_sats = gga_sats;
//					gps_altitude = new_altitude;
//					new_altitude = -1000.0; // Invalidate new_altitude so that we are sure to get a valid one next time
					lastFix = newFix;
					ret = true;
				}
			}

			at_checksum = false;        // CR/LF signals the end of the checksum
			our_checksum = '$';         // Reset checksums
			their_checksum = 0;
			offset = 0;                 // Prepare for the next incoming sentence
			num_tokens = 0;
			sentence_type = SENTENCE_UNK;
			break;

		case '*':
			// Begin of checksum and process token (ie. do not break)
			at_checksum = true;
			our_checksum ^= c;

		case ',':
			// Process token
			token[offset] = '\0';
			our_checksum ^= c;  // Checksum the ',', undo the '*'

			// Parse token
			switch (sentence_type) {
				case SENTENCE_UNK:
					if (num_tokens < NUM_OF_UNK_PARSERS && unk_parsers[num_tokens])
						unk_parsers[num_tokens](token);
					break;
				case SENTENCE_GGA:
					if (num_tokens < NUM_OF_GGA_PARSERS && gga_parsers[num_tokens])
						gga_parsers[num_tokens](token);
					break;
				case SENTENCE_RMC:
					if (num_tokens < NUM_OF_RMC_PARSERS && rmc_parsers[num_tokens])
						rmc_parsers[num_tokens](token);
					break;
			}

			// Prepare for next token
			num_tokens++;
			offset = 0;
			break;

		default:
			// Any other character
			if (at_checksum) {
				// Checksum value
				their_checksum = their_checksum * 16 + from_hex(c);
			} else {
				// Regular NMEA data
				if (offset < 15) {  // Avoid buffer overrun (tokens can't be > 15 chars)
					token[offset] = c;
					offset++;
					our_checksum ^= c;
				}
			}
	}
	return ret;
}


uint32_t gps_get_region_frequency()
{
	uint32_t frequency = DEFAULT_FREQUENCY;
	if(-168 < lastFix.longitude && lastFix.longitude < -34) frequency = RADIO_FREQUENCY_REGION2;
	if(-34 <  lastFix.longitude && lastFix.longitude <  71) frequency = RADIO_FREQUENCY_REGION1;
	if(-34.95f < lastFix.latitude && lastFix.latitude < 7.18f  && -73.13f < lastFix.longitude && lastFix.longitude < -26.46f) frequency = RADIO_FREQUENCY_BRAZIL;      // Brazil
	if( 29.38f < lastFix.latitude && lastFix.latitude < 47.10f && 127.16f < lastFix.longitude && lastFix.longitude < 153.61f) frequency = RADIO_FREQUENCY_JAPAN;       // Japan
	if( 19.06f < lastFix.latitude && lastFix.latitude < 53.74f &&  72.05f < lastFix.longitude && lastFix.longitude < 127.16f) frequency = RADIO_FREQUENCY_CHINA;       // China
	if( -0.30f < lastFix.latitude && lastFix.latitude < 20.42f &&  93.06f < lastFix.longitude && lastFix.longitude < 105.15f) frequency = RADIO_FREQUENCY_THAILAND;    // Thailand
	if(-54.54f < lastFix.latitude && lastFix.latitude <-32.43f && 161.62f < lastFix.longitude && lastFix.longitude < 179.99f) frequency = RADIO_FREQUENCY_NEWZEALAND;  // New Zealand
	if(-50.17f < lastFix.latitude && lastFix.latitude < -8.66f && 105.80f < lastFix.longitude && lastFix.longitude < 161.62f) frequency = RADIO_FREQUENCY_AUSTRALIA;   // Australia

	// Note: These regions are a super simplified approach to define rectangles on the world map, representing regions where we may consider at least some
	// chance that an APRS Digipeter or an Igate is listening. They have absolutely NO political relevance. I was just trying to identify
	// regions with APRS activity on APRS.fi and look up the used frequencies on the web. Any corrections or additions are welcome. Please email
	// your suggestions to thomas@tkrahn.com

	// use your own coordinates for testing; Comment out when testing is finished!
	//  if(29.7252 < gps_lat && gps_lat < 29.7261 && -95.5082 < gps_lon && gps_lon < -95.5074) frequency = MX146_FREQUENCY_TESTING; // KT5TK home
	//if(29.7353 < gps_lat && gps_lat < 29.7359 && -95.5397 < gps_lon && gps_lon < -95.5392) frequency = MX146_FREQUENCY_TESTING; // Gessner BBQ

	// Note: Never define a region that spans the date line! Use two regions instead.

	// switch to default when we don't have a GPS lease
	if(lastFix.latitude == 0 && lastFix.longitude == 0) frequency = DEFAULT_FREQUENCY; // TODO: This might not work because lastFix is not updated when GPS is lost

	return frequency;
}

void gpsSetTime2lock(uint32_t ms)
{
	lastFix.time2lock = ms/1000 < 255 ? ms/1000 : 255;
}

void gps_reset(void) {
	GPS_RESET(HIGH);
	delay(10);
	GPS_RESET(LOW);
	delay(1000);
}

/**
 * @brief Switches GPS on/off by transistor
 * @param pos true or false wether gps should be switch on or off
 */
void gps_hw_switch(bool pos) {
	#ifdef USE_GPS_HW_SW
	GPS_EN_SET(!pos);
	#endif
}

/**
 * @brief Switches GPS into sleep mode or wakes it up by software
 * @param pos true or false wether gps should be switch on or off
 */
void gps_sw_switch(bool pos) {
	#ifdef USE_GPS_SW_SW
	uint8_t i;
	if(pos) {
		uint8_t message[] = UBX_POWERON;
		for(i=0; i<sizeof(message); i++)
			UART_TransmitChar(message[i]);
		uint8_t message2[] = UBX_SETMAXPERFORMANCE;
		for(i=0; i<sizeof(message2); i++)
			UART_TransmitChar(message2[i]);
	} else {
		uint8_t message[] = UBX_POWEROFF;
		for(i=0; i<sizeof(message); i++)
			UART_TransmitChar(message[i]);
	}
	#endif
}

bool gpsIsOn(void) {
	return isOn;
}






