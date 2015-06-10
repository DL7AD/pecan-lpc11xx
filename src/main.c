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

//unsigned long next_tx_millis;
uint32_t wd_counter = 0;
uint32_t ms_counter = 0;
bool newPositionStillUnknown = true;

/**
 * Enter power save mode for 8 seconds. Power save is disabled and replaced by
 * delay function in debug mode to avoid stopping SWD interface.
 */
void power_save()
{
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

	// This delay is necessary to get access again after module fell into a deep sleep state in which the reset pin is disabled !!!
	// To get access again, its necessary to access the chip in active mode. If chip is almost everytime in sleep mode, it can be
	// only waked up by the reset pin which is (as mentioned before) disabled.
	delay(10000); // !!! IMPORTANT IMPORTANT IMPORTANT !!! DO NOT REMOVE THIS DELAY UNDER ANY CIRCUMSTANCES !!!

	newPositionStillUnknown = true;

	// Manipulate wd_counter that way, that its transmitting immediately after being switched on
	wd_counter = APRS_PERIOD_SECONDS / 8;
	ms_counter = 0;

	while(true)
	{
		// Measure battery voltage
		ADC_Init();
		uint32_t batt_voltage = getBatteryMV();
		ADC_DeInit();

		// Switch tracker to sleep when battery below specific voltage
		if(batt_voltage < VOLTAGE_NOTRANSMIT) {
			GPS_PowerOff();
			power_save();
			continue;
		}

		if (wd_counter >= APRS_PERIOD_SECONDS / 8)
		{
			if (!newPositionStillUnknown || (wd_counter >= APRS_PERIOD_SECONDS / 8)) // We have our GPS position or 2 minutes have passed without lease (giving up)
			{
				if (newPositionStillUnknown)
				{
					gpsSetTime2lock(wd_counter); // Set how many cycles (1cycle=8sec) it took to aquire GPS
				}

				transmit_telemetry();		// Transmit APRS telemetry packet

				// Transmit position packet only when battery voltage above specific value or when GPS position already known
				if(batt_voltage >= VOLTAGE_NOGPS || !newPositionStillUnknown) {
					delay(6000);			// Wait a few seconds (Else aprs.fi reports "[Rate limited (< 5 sec)]")
					transmit_position();	// Transmit APRS position packet
				}

				wd_counter = 0;
			}

			// Show modem ISR stats from the previous transmission
			if(!gpsIsOn())
			{
				if(batt_voltage >= VOLTAGE_NOGPS)
					GPS_Init(); // Switch on GPS if battery above specific voltage

				newPositionStillUnknown = true;
			}
		}

		if(gpsIsOn()) { // Check position if position unknown and gps switched on (switch off when battery runs low)
			if(newPositionStillUnknown) {
				uint8_t c;
				while(UART_ReceiveChar(&c)) {
					if (gps_decode(c)) {
						// We have received and decoded our location
						newPositionStillUnknown = false;
					}
				}

				if(batt_voltage < VOLTAGE_NOGPS-VOLTAGE_GPS_MAXDROP) //Battery voltage dropped below specific value while acquisitioning
					GPS_PowerOff(); // Stop consuming power

				if(ms_counter++ == 8000) {
					wd_counter++;
					ms_counter = 0;
				}
				delay(1);
			} else {
				gpsSetTime2lock(wd_counter); // Set how many cycles (1cycle=8sec) it took to aquire GPS
				GPS_PowerOff();

				wd_counter++;
				ms_counter = 0;
				power_save(); // Enter power save mode
			}
		} else { // GPS switched off
			wd_counter++;
			power_save(); // Enter power save mode
		}
	}

}
