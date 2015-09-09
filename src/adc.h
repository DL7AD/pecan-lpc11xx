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

#ifndef __SENSORS_H__
#define __SENSORS_H__

#include "types.h"

// Conversion voltages to compressed 8bit-formats (Unit: mV)
#define VBAT_TO_EIGHTBIT(x) ((x)/10)
#define VSOL_TO_EIGHTBIT(x) ((x)/10)
#define EIGHTBIT_TO_VBAT(x) (10*(x))
#define EIGHTBIT_TO_VSOL(x) ((x)*10)

void ADC_Init(void);
void ADC_DeInit(void);
uint32_t getBatteryMV(void);
uint16_t getADC(uint8_t ad);

#ifdef SOLAR_AVAIL
uint32_t getSolarMV(void);
#endif

#endif
