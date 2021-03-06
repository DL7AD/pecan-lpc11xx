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

#include "LPC11xx.h"
#include "aprs.h"
#include "ax25.h"
#include "config.h"
#include "gps.h"
#include "modem.h"
#include "Si446x.h"
#include "uart.h"
#include "global.h"
#include "spi.h"
#include "i2c.h"
#include "sleep.h"
#include "debug.h"
#include "adc.h"
#include "bmp180.h"
#include "ssd1306.h"

/**
 * Enter power save mode for 8 seconds. Power save is disabled and replaced by
 * delay function in debug mode to avoid stopping SWD interface.
 */
void power_save()
{
	//#undef DEBUG
	#ifdef DEBUG
	delay(8000);
	#else
	SetLowCurrentOnGPIO();
	InitDeepSleep(8000);
	EnterDeepSleep();
	// Reinitializing is done by wakeup interrupt routine in sleep.c => On_Wakeup
	#endif
}

int main(void)
{
	TargetResetInit(); // Configures clock and SystemTick timer

	// Display first heartbeat
	Init_SSD1306();
	display_configuration();

	// Configure ADC (for discharging capacitor at VCC1V8)
	ADC_Init();

	// This delay is necessary to get access again after module fell into a deep sleep state in which the reset pin is disabled !!!
	// To get access again, its necessary to access the chip in active mode. If chip is almost every time in sleep mode, it can be
	// only waked up by the reset pin which is (as mentioned before) disabled.
	delay(10000); // !!! IMPORTANT IMPORTANT IMPORTANT !!! DO NOT REMOVE THIS DELAY UNDER ANY CIRCUMSTANCES !!!

	// Clear terminal and display cycle start
	terminal_clear();
	terminal_addLine("> Start cycle");
	terminal_flush();
	delay(1000);

	trackingstate_t trackingstate = LOG;
	gpsstate_t gpsstate = GPS_LOSS;

	track_t trackPoint;

	uint64_t timestampPointer = 0;
	uint64_t lastLogPoint = 0;
	uint32_t id = 0;

	while(true) {

		// Measure battery voltage
		ADC_Init();
		uint32_t batt_voltage = getBatteryMV();
		ADC_DeInit();

		// Freeze tracker when battery below specific voltage
		if(batt_voltage < VOLTAGE_NOTRANSMIT)
		{
			GPS_PowerOff();
			timestampPointer = getUnixTimestamp(); // Mark timestamp for sleep routine
			trackingstate = SLEEP;
		}

		// Switch states
		switch(trackingstate)
		{
			case SLEEP:
				if(batt_voltage < VOLTAGE_NOGPS && gpsIsOn()) // Tracker has low battery, so switch off GPS
					GPS_PowerOff();

				if(getUnixTimestamp()-timestampPointer >= TIME_SLEEP_CYCLE*1000) {
					trackingstate = SWITCH_ON_GPS;
					continue;
				}
				power_save();
				break;

			case SWITCH_ON_GPS:
				if(batt_voltage < VOLTAGE_NOGPS) { // Tracker has low battery, so switch off GPS
					if(gpsIsOn())
						GPS_PowerOff();
					trackingstate = LOG;
					gpsstate = GPS_LOW_BATT;
					continue;
				}

				// Switch on GPS if switched off
				GPS_Init();

				timestampPointer = getUnixTimestamp(); // Mark timestamp for search_gps routine
				trackingstate = SEARCH_GPS;
				break;

			case SEARCH_GPS:
				// Decide to switch off GPS due to low battery
				if(batt_voltage < VOLTAGE_NOGPS-VOLTAGE_GPS_MAXDROP) { //Battery voltage dropped below specific value while acquisitioning
					GPS_PowerOff(); // Stop consuming power
					trackingstate = LOG;
					gpsstate = GPS_LOW_BATT;
				}

				// Parse NMEA
				if(gpsIsOn()) {
					uint8_t c;
					while(UART_ReceiveChar(&c)) {
						if(gps_decode(c)) { // Lock and 5 sats are used
							#ifdef USE_GPS_POWER_SAVE
							gps_activate_power_save(); // Activate power save mode
							#else
							GPS_PowerOff(); // Switch off GPS
							#endif

							// We have received and decoded our location
							gpsSetTime2lock((getUnixTimestamp() - timestampPointer) / 1000);
							trackingstate = LOG;
							gpsstate = GPS_LOCK;
						}
					}
				}

				if(getUnixTimestamp()-timestampPointer >= TIME_MAX_GPS_SEARCH*1000) { // Searching for GPS took too long
					gpsSetTime2lock(TIME_MAX_GPS_SEARCH);
					trackingstate = LOG;
					gpsstate = GPS_LOSS;
					continue;
				}

				break;

			case LOG:
				trackPoint.id = ++id;
				trackPoint.time = getUnixTimestamp()/1000;
				trackPoint.latitude = lastFix.latitude;
				trackPoint.longitude = lastFix.longitude;
				trackPoint.altitude = lastFix.altitude;
				trackPoint.satellites = lastFix.satellites;
				trackPoint.ttff = lastFix.ttff;

				ADC_Init();
				trackPoint.vbat = VBAT_TO_EIGHTBIT(getBatteryMV());
				#ifdef SOLAR_AVAIL
				trackPoint.vsol = VSOL_TO_EIGHTBIT(getSolarMV());
				#else
				trackPoint.vsol = 0;
				#endif
				ADC_DeInit();

				#ifdef BMP180_AVAIL
				BMP180_Init();
				trackPoint.temp = (getTemperature() / 10);
				trackPoint.pressure = getPressure();
				BMP180_DeInit();
				#else
				Si406x_Init();
				trackPoint.temp = Si406x_getTemperature();
				radioShutdown();
				#endif

				if(getUnixTimestamp()-lastLogPoint >= LOG_CYCLE_TIME*1000) { // New log point necessary
					logTrackPoint(trackPoint);
					lastLogPoint = getUnixTimestamp();
				}

				trackingstate = TRANSMIT;
				break;

			case TRANSMIT:
				// Mark timestamp for sleep routine (which will probably follow after this state)
				timestampPointer = getUnixTimestamp();

				// Transmit APRS telemetry
				transmit_telemetry(&trackPoint);

				// Wait a few seconds (Else aprs.fi reports "[Rate limited (< 5 sec)]")
				power_save(6000);

				// Transmit APRS position
				transmit_position(&trackPoint, gpsstate, lastFix.course, lastFix.speed);

				// Wait a few seconds (Else aprs.fi reports "[Rate limited (< 5 sec)]")
				power_save(6000);

				// Transmit log packet
				transmit_log(&trackPoint);

				// Change state depending on GPS status
				if(gpsstate == GPS_LOCK || gpsstate == GPS_LOW_BATT) {
					trackingstate = SLEEP;
				} else { // GPS_LOSS
					trackingstate = SWITCH_ON_GPS;
				}

				break;

			default: // It should actually never reach this state
				trackingstate = LOG;
				break;
		}
	}
}
