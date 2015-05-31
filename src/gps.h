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

#ifndef __GPS_H__
#define __GPS_H__

#include "types.h"

extern char gps_time[8];       // HHMMSS
extern char gps_date[7];       // DDMMYY
extern int16_t gps_sats;
extern float gps_lat;
extern float gps_lon;
extern char gps_aprs_lat[10];
extern char gps_aprs_lon[11];
extern uint32_t gps_region_frequency;
extern uint32_t doppler_frequency;
extern bool satInView;
extern float gps_course;
extern float gps_speed;
extern float gps_altitude;
extern float gps_get_lat();
extern float gps_get_lon();
extern float gps_get_altitude();
extern int32_t gps_get_time();
extern int32_t gps_get_date();
extern int16_t iss_lat;
extern int16_t iss_lon;
extern uint16_t iss_datapoint;
extern int16_t time2lock;
extern bool newPositionStillUnknown;

void GPS_Init();
void gps_reset(void);
void gps_setMaxPerformance();
void gps_setPowerSaveMode();
void gpsSetTime2lock(int16_t periods);

bool gps_decode(char c);
uint32_t gps_get_region_frequency();
bool gps_check_satellite();
void gps_resetBuffer(void);
void gps_setNavigationMode(void);

void gps_hw_switch(bool pos);
void gps_sw_switch(bool pos);
bool gpsIsOn(void);

void GPS_PowerOn(void);
void GPS_PowerOff(void);

#endif
