#include "config.h"
#include "modem.h"
#include "target.h"
#include "global.h"

// only include the selected radio
#include "Si446x.h"

uint32_t radio_frequency = 0;
uint8_t radio_power = 0;

// Sine table
const uint8_t sine_table[32] = {
	16,19,21,24,26,28,30,31,
	31,31,30,28,26,24,21,19,
	16,12,10,7, 5, 3, 1, 0,
	0, 0, 1, 3, 5, 7, 10,12,
};

/* The sine_table is the carrier signal. To achieve phase continuity, each tone
 * starts at the index where the previous one left off. By changing the stride of
 * the index (phase_delta) we get 1200 or 2200 Hz. The PHASE_DELTA_XXXX values
 * can be calculated as:
 * 
 * Fg = frequency of the output tone (1200 or 2200)
 * Fm = sampling rate (PLAYBACK_RATE_HZ)
 * Tt = sine table size (TABLE_SIZE)
 * 
 * PHASE_DELTA_Fg = Tt*(Fg/Fm)
 */

#define TABLE_SIZE			sizeof(sine_table)
#define PLAYBACK_RATE		(Fosc / 256) // When transmitting CPU is switched to 48 MHz -> 187.5 kHz
#define BAUD_RATE			1200
#define SAMPLES_PER_BAUD	(PLAYBACK_RATE / BAUD_RATE) // 52.083333333 / 26.041666667
#define PHASE_DELTA_1200	(((TABLE_SIZE * 1200) << 7) / PLAYBACK_RATE) // Fixed point 9.7 // 1258 / 2516
#define PHASE_DELTA_2200	(((TABLE_SIZE * 2200) << 7) / PLAYBACK_RATE) // 2306 / 4613


// Module globals
static uint16_t current_byte;
static uint16_t current_sample_in_baud;    // 1 bit = SAMPLES_PER_BAUD samples
static uint32_t phase_delta;                // 1200/2200 for standard AX.25
static uint32_t phase;                      // Fixed point 9.7 (2PI = TABLE_SIZE)
static uint32_t packet_pos;                 // Next bit to be sent out

// Exported globals
uint16_t modem_packet_size = 0;
uint8_t modem_packet[MODEM_MAX_PACKET];

/**
 * Initializes two timer
 * Timer 1: One Tick per 1/playback_rate	CT16B0
 * Timer 2: PWM								CT32B0
 */
void Modem_Init(void)
{
	// Initialize radio
	Si406x_Init();

	// Key the radio
	radioTune(radio_frequency, 1, radio_power);

	// Setup bit timer
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<7);	// Enable TIMER16_B0 clock
	LPC_TMR16B0->PR = 10000;				// Timer prescaler (1200 ticks per second)
	LPC_TMR16B0->MR3 = 1;					// MR3 = Period
	LPC_TMR16B0->MCR = 0x600;				// MR3 resets timer & generates interrupt
	LPC_TMR16B0->TCR = 0b1;					// Enable Timer

	// Setup out timer
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<8);	// Enable TIMER16_B1 clock
	LPC_TMR16B1->MR3 = 5000;				// MR3 = Period		5000 @ 1200Hz
											//					2727 @ 2200Hz
	LPC_TMR16B1->MCR = 0x600;				// MR3 resets timer & generates interrupt
	LPC_TMR16B1->TCR = 0b1;					// Enable Timer

	NVIC_SetPriority(TIMER_16_0_IRQn, INT_PRIORITY_TMR16B0);
	NVIC_SetPriority(TIMER_16_1_IRQn, INT_PRIORITY_TMR16B1);
	NVIC_EnableIRQ(TIMER_16_0_IRQn);
	NVIC_EnableIRQ(TIMER_16_1_IRQn);
}

bool modem_busy() {
	return LPC_TMR16B0->TCR & 0b1;			// Is sample timer activated?
}

void modem_flush_frame(void) {
	phase_delta = PHASE_DELTA_1200;
	phase = 0;
	packet_pos = 0;
	current_sample_in_baud = 0;

	Modem_Init();							// Initialize timers and radio

	while(modem_busy())						// Wait for radio getting finished
		__WFI();

	radioShutdown();						// Shutdown radio
}

/**
 * Interrupt routine which is called <PLAYBACK_RATE> times per second.
 * This method is supposed to load the next sample into the PWM timer.
 */
void On_Sample_Handler(void) {
	// If done sending packet
	if(packet_pos == modem_packet_size) {
		LPC_TMR16B0->TCR = 0b10;	// Disable bit interrupt
		//LPC_TMR16B0->IR = 0x08;		// Clear interrupt
		LPC_TMR16B1->TCR = 0b10;	// Disable tone interrupt
		//LPC_TMR16B1->IR = 0x08;		// Clear interrupt
		return;						// Done
	}

	if ((packet_pos & 7) == 0) {          // Load up next byte
		current_byte = modem_packet[packet_pos >> 3];
	} else {
		current_byte = current_byte / 2;  // ">>1" forces int conversion
	}

	if ((current_byte & 1) == 0) {
		// Toggle tone (1200 <> 2200)
		phase_delta ^= (PHASE_DELTA_1200 ^ PHASE_DELTA_2200);
		LPC_TMR16B1->MR3 = phase_delta == PHASE_DELTA_1200 ? 5000 : 2727;
	}

	packet_pos++;

	LPC_TMR16B0->IR = 0x08; // Clear interrupt
}

void On_Tone_Handler(void) {
	static uint8_t tone;

	if(tone) {
		setLowTone();
	} else {
		setHighTone();
	}
	tone = !tone;

	LPC_TMR16B1->IR = 0x08; // Clear interrupt
}

void modem_set_tx_freq(uint32_t frequency) {
	radio_frequency = frequency;
}

void modem_set_tx_power(uint8_t power) {
	radio_power = power;
}
