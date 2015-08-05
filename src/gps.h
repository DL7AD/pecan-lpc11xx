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
#include "global.h"

typedef struct {
	date_t time;
	double latitude;
	double longitude;
	uint16_t altitude;
	uint8_t satellites;
	uint16_t speed;
	uint16_t course;
	bool active;
	uint8_t time2lock;
} gps_t;

extern uint32_t gps_region_frequency;
extern uint8_t time2lock;
extern gps_t lastFix;

void GPS_Init();
void GPS_hibernate_uart(void);
void GPS_wake_uart(void);
void GPS_PowerOff(void);
void GPS_PowerOn(void);
void gps_set_nmeaCompatibility();
void gps_set_gps_only();
void gps_set_airborne_model();
void gps_configure_power_save();
void gps_activate_power_save();
void gps_disable_power_save();
void gps_configureActiveNMEASentences();
bool gps_decode(char c);
uint32_t gps_get_region_frequency();
void gpsSetTime2lock(uint32_t ms);
void gps_reset(void);
void gps_hw_switch(bool pos);
bool gpsIsOn(void);

#endif
