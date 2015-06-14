#ifndef __TYPES_H__
#define __TYPES_H__

#include "defines.h"
#include "LPC11xx.h"

#define PACK_STRUCT_END 	__attribute__((packed))

//#define __LONG	long
//#define __INT64	long long

typedef unsigned char			bool;
#define	BOOL					bool

typedef char					INT8;

typedef struct GPS {
	uint32_t	time;		// Sekunden seit Mitternacht
	int32_t		lat;		// Latitude in 10µMinuten, >=0 N, <0 S
	int32_t		lon;		// Longitude in 10µMinuten, >=0 E, <0 W
	uint16_t	altitude;	// Positive Meter
	uint8_t		satellites;	// Anzahl der Satelliten
	bool		isValid;	// Gültigkeit
} GPS_t;

typedef enum {
	SLEEP,					// General sleep mode
	SWITCH_ON_GPS,			// Switch on GPS
	SEARCH_GPS,				// Search for GPS
	TRANSMIT				// Transmit state
} trackingstate_t;

typedef enum {
	GPS_LOCK,
	GPS_LOSS
} gpsstate_t;

#endif
