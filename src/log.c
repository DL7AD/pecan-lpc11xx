#include "log.h"
#include "config.h"

static track_t track[LOG_SIZE]; // Log data
static uint8_t trackPointer = 0; // Logging pointer for logging
static uint8_t trackPointerTRX = 0; // Logging pointer for transmission
static uint8_t lastTrack = 0; // Last log point

void logTrackPoint(track_t logPoint) {
	// Log point
	track[trackPointer] = logPoint;

	// Move tracker pointer one position foward
	trackPointer = (trackPointer+1) % LOG_SIZE;
	lastTrack = trackPointer;
}

track_t* getNextLogPoint(void) {
	track_t *logPoint = &track[trackPointerTRX];
	trackPointerTRX = (trackPointerTRX+1) % (!lastTrack ? 1 : lastTrack);
	return logPoint;
}
