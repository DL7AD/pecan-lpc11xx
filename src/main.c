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

//unsigned long next_tx_millis;
uint32_t wd_counter;
uint32_t ms_counter;
bool newPositionStillUnknown;

void power_save()
{
	if(!modem_busy()) {  // Don't sleep if we're txing.

		if (newPositionStillUnknown == true) {
			gps_setMaxPerformance();
		} else {
			//Tell the GPS to go into Power save mode.
			GPS_PowerOff();
		}

		// Sleep mode is replaced with simple delay in debug mode because it will stop the SWD interface otherwise
		// #undef DEBUG
		#ifdef DEBUG
		delay(8000);
		#else
		SetLowCurrentOnGPIO();
		InitDeepSleep(8000);
		EnterDeepSleep();
		// Reinitializing is done by wakeup interrupt routine in sleep.c => On_Wakeup
		#endif

	}
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
		if (wd_counter >= APRS_PERIOD_SECONDS / 8)
		{
			if (!newPositionStillUnknown || (wd_counter >= APRS_PERIOD_SECONDS / 8)) // We have our GPS position or 2 minutes have passed without lease (giving up)
			{
				gpsSetTime2lock(wd_counter - APRS_PERIOD_SECONDS / 8);

				if (!newPositionStillUnknown)
				{
					GPS_PowerOff();
				}

				aprs_send();
				wd_counter = 0;
			}

			// Show modem ISR stats from the previous transmission
			if(!gpsIsOn())
			{
				GPS_Init();		// Init MAX7
				newPositionStillUnknown = true;
			}
		}

		if (newPositionStillUnknown) // Check for the available position first, without touching the serial. Else the GPS will wake up.
		{
			uint8_t c;
			while(UART_ReceiveChar(&c))
			{
				if (gps_decode(c))
				{
					// We have received and decoded our location
					newPositionStillUnknown = false;
				}
			}

			if(ms_counter++ == 8000) {
				wd_counter++;
				ms_counter = 0;
			}
			delay(1);
		}
		else
		{
			power_save();
			wd_counter++;
			ms_counter = 0;
		}
	}

}
