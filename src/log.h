#ifndef __LOG_H__
#define __LOG_H__

#include "global.h"

typedef struct {
	uint32_t id;
	uint32_t time;
	float latitude;
	float longitude;
	uint16_t altitude;
	uint8_t satellites;
	uint8_t ttff;
	uint8_t vbat;
	uint8_t vsol;
	int8_t temp;
	uint32_t pressure;
} track_t;

void logTrackPoint(track_t logPoint);
track_t* getNextLogPoint(void);

#endif
