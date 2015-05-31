/**
 * Sleep library for Pecan Pico 6 / Pecan Femto 2.1. This library has been
 * modified from NXP's timedwakeup sleep library.
 * @see https://github.com/tacowars/LPC1114-sandbox/blob/master/drivers/timedwakeup/src/main.c
 *
 * For counting TMR32B0 and Watchdog timer is used. Watchdogtimer is calibrated
 * before entering deep sleep so sleep time is roughly accurate.
 *
 * @author NXP
 * @author Sven Steudte
 */

#include "sleep.h"
#include "global.h"
#include "target.h"

// Configuration for normal operation
// If other peripherals are used, they need to be added to the _RUN macro
#define BF_PDRUNCFG_RUN    (BF_PDCFG_RESERVEDMSK   \
                            & ~(  BF_PDCFG_IRC     \
                                | BF_PDCFG_FLASH   \
                                | BF_PDCFG_WDTOSC))

// If other peripherals are used, they need to be added to the _RUN macro
#define BF_SYSAHBCLKCTRL_RUN       (BF_SYSAHBCLKCTRL_SLEEP  \
                                  | BF_SYSAHBCLKCTRL_GPIO   \
								  | BF_SYSAHBCLKCTRL_FLASHREG \
                                  | BF_SYSAHBCLKCTRL_ROM    \
                                  | BF_SYSAHBCLKCTRL_IOCON)

#define WUTIME_CLOCKS           60
#define WDOMEASUREDURATION_MS	100 // Timelength in which watchdog frequency is probed (in ms)

uint32_t lastTimeSetup = 0;

void EnterDeepSleep(void)
{
    LPC_TMR32B0->EMR = BF_TIMER_EMR_SETOUT2; // set timer to drive P0_1 high at match
    LPC_SYSCON->SYSAHBCLKCTRL = BF_SYSAHBCLKCTRL_SLEEP; // Shut down clocks to almost everything
    SCB->SCR |= BF_SCR_SLEEPDEEP; // Set SLEEPDEEP bit so MCU will enter DeepSleep mode on __WFI();

    // Switch main clock to low-speed WDO
    LPC_SYSCON->MAINCLKSEL = MAINCLKSEL_SEL_WDOSC;
    LPC_SYSCON->MAINCLKUEN = 0;
    LPC_SYSCON->MAINCLKUEN = 1; // toggle to enable
    LPC_SYSCON->MAINCLKUEN = 0;

    // Preload clock selection for quick switch back to IRC on wakeup
    LPC_SYSCON->MAINCLKSEL = MAINCLKSEL_SEL_IRCOSC;

    setUnixTimestamp(getUnixTimestamp() + lastTimeSetup); // Correct unix timestamp for next wakeup
    LPC_TMR32B0->TCR = BF_TIMER_TCR_RUN; // start sleep timer

    __WFI();  // Enter deep sleep mode
}

void On_Wakeup(void)
{
    LPC_SYSCON->MAINCLKUEN = 1;							// Restore main clock to IRC 12 MHz
    LPC_TMR32B0->EMR = 0;								// Clear match bit on timer
    LPC_SYSCON->STARTRSRP0CLR = BF_STARTLOGIC_P0_1;		// Clear pending bit on start logic
    SCB->SCR &= ~BF_SCR_SLEEPDEEP;						// Clear SLEEPDEEP bit so MCU will enter Sleep mode on __WFI();
    LPC_SYSCON->SYSAHBCLKCTRL = BF_SYSAHBCLKCTRL_RUN;	// Restore clocks to chip modules
}

void InitDeepSleep(uint32_t ms)
{
    LPC_SYSCON->PDRUNCFG      = BF_PDRUNCFG_RUN; // Initialize power to chip modules
    LPC_SYSCON->SYSAHBCLKCTRL = BF_SYSAHBCLKCTRL_RUN; // Initialize clocks

    // Make sure clock is set to 12 MHz and setup System Tick Timer (1 Tick = 1ms)
    Target_SetClock_IRC();

    LPC_SYSCON->PDAWAKECFG = LPC_SYSCON->PDRUNCFG; // Configure PDAWAKECFG to restore PDRUNCFG on wake up

    LPC_SYSCON->PDSLEEPCFG = BF_PDSLEEPCFG_WDT; // Configure deep sleep with WDT oscillator

    LPC_TMR32B0->TCR = BF_TIMER_TCR_RESET; // reset timer

    // The following lines initializing PR and MR0
    LPC_TMR32B0->PR = 0;
    LPC_TMR32B0->MR2 = (ms*MeasureWDO()/WDOMEASUREDURATION_MS);

    LPC_TMR32B0->MCR = BF_TIMER_MCR_MATCHSTOP2 | BF_TIMER_MCR_MATCHRESET2;

    LPC_IOCON->PIO0_1 = (LPC_IOCON->PIO0_1 & ~0x3F) | 0x2; // Set IOCON register on P0.1 to match function

    /* Configure Wakeup I/O */
    /* Specify the start logic to allow the chip to be waken up using PIO0_1 */
    LPC_SYSCON->STARTAPRP0          |=  BF_STARTLOGIC_P0_1; // Rising edge
    LPC_SYSCON->STARTRSRP0CLR       =   BF_STARTLOGIC_P0_1; // Clear pending bit
    LPC_SYSCON->STARTERP0           |=  BF_STARTLOGIC_P0_1; // Enable Start Logic
    NVIC_EnableIRQ(WAKEUP1_IRQn);

    lastTimeSetup = ms; // Save time in local variable for EnterDeepSleep()
}

uint32_t MeasureWDO(void)
{
    uint32_t start,end;
    static uint32_t WDO_clocks;

    if(!WDO_clocks)
    {
        // Configure Watchdog Oscillator
        LPC_SYSCON->WDTOSCCTRL = (0x1<<5) | (0x1F<<0);

        LPC_SYSCON->SYSAHBCLKCTRL |= BF_SYSAHBCLKCTRL_WDT;  // Turn on clock to WDT register block

        // Now enable clock to WDT counter
        LPC_SYSCON->WDTCLKSEL = WDTCLKSEL_SEL_WDOSC;
        LPC_SYSCON->WDTCLKUEN = 0;  // Arm WDT clock selection update
        LPC_SYSCON->WDTCLKUEN = 1;  // Update WDT clock selection
        LPC_SYSCON->WDTCLKUEN = 0;  // Arm WDT clock selection update
        LPC_SYSCON->WDTCLKDIV = 1;  // WDT clock divide by 1

        LPC_WDT->TC = 0xFFFFFF;   // big value, WDT timer duration = don't care
        LPC_WDT->MOD = 1;           // enable WDT for counting/interrupts but not resets
        LPC_WDT->FEED = 0xAA;       // WDT start sequence step 1
        LPC_WDT->FEED = 0x55;       // WDT start sequence step 2

        delay(100);               // Need to delay before measuring because WDT looses 256 cycles
        start = LPC_WDT->TV;
        delay(WDOMEASUREDURATION_MS);
        end = LPC_WDT->TV;

        LPC_SYSCON->SYSAHBCLKCTRL &= ~BF_SYSAHBCLKCTRL_WDT;  // Turn off clock to WDT register block
        WDO_clocks = (start-end)*4;
    }

    return WDO_clocks;
}

/**
 * Configures GPIO pins that way, that the target is consuming less
 * power. In this state all peripheral hardware is not usable.
 */
void SetLowCurrentOnGPIO(void)
{
	// ISP_ENTRY (doesnt matter if high or low)
	LPC_IOCON->PIO0_1 = 0xC0;
	LPC_GPIO0->DIR |= (1 << 1);
	LPC_GPIO0->DATA &= ~(1 << 1);

	// IRQ (must be low)
	LPC_IOCON->PIO0_2 = 0xC0;
	LPC_GPIO0->DIR |= (1 << 2);
	LPC_GPIO0->DATA &= ~(1 << 2);

	// VCXO_EN (high impedance)
	LPC_IOCON->PIO0_3 = 0xC0;
	LPC_GPIO0->DIR |= ~(1 << 3);
	LPC_GPIO0->DATA &= ~(1 << 3);

	// I2C_PULL_VCC (high impedance)
	LPC_IOCON->PIO1_9 = 0xC0;
	LPC_GPIO1->DIR &= ~(1 << 9);
	LPC_GPIO1->DATA &= ~(1 << 9);

	// SCL (high impedance)
	LPC_IOCON->PIO0_4 = 0xC0; // I2C pin
	LPC_GPIO0->DIR &= ~(1 << 4);
	LPC_GPIO0->DATA &= ~(1 << 4);

	// SDA (high impedance)
	LPC_IOCON->PIO0_5 = 0xC0; // I2C pin
	LPC_GPIO0->DIR &= ~(1 << 5);
	LPC_GPIO0->DATA &= ~(1 << 5);

	// SCK (must be high)
	LPC_IOCON->PIO0_6 = 0xC0;
	LPC_GPIO0->DIR |= (1 << 6);
	LPC_GPIO0->DATA |= (1 << 6);

	// SS (doesnt matter if high or low)
	LPC_IOCON->PIO0_7 = 0xC0;
	LPC_GPIO0->DIR |= (1 << 7);
	LPC_GPIO0->DATA |= (1 << 7);

	// MISO (doesnt matter if high or low)
	LPC_IOCON->PIO0_8 = 0xC0;
	LPC_GPIO0->DIR |= (1 << 8);
	LPC_GPIO0->DATA |= (1 << 8);

	// MOSI (doesnt matter if high or low)
	LPC_IOCON->PIO0_9 = 0xC0;
	LPC_GPIO0->DIR |= (1 << 9);
	LPC_GPIO0->DATA |= (1 << 9);

	// VCXO_CTRL (doesnt matter if high or low)
	LPC_IOCON->R_PIO0_11 = 0xC1; // ADC pin
	LPC_GPIO0->DIR |= (1 << 11);
	LPC_GPIO0->DATA |= (1 << 11);

	// ADC_VSOL (high Impedance)
	LPC_IOCON->R_PIO1_0 = 0xC1; // ADC pin
	LPC_GPIO1->DIR &= ~(1 << 0);
	LPC_GPIO1->DATA |= (1 << 0);

	// VCC1V8 (high Impedance)
	LPC_IOCON->R_PIO1_1 = 0xD0; // ADC pin
	LPC_GPIO1->DIR &= ~(1 << 1);
	LPC_GPIO1->DATA |= (1 << 1);

	// LDO_EN (must be low)
	LPC_IOCON->R_PIO1_2 = 0xC1; // ADC pin
	LPC_GPIO1->DIR |= (1 << 2);
	LPC_GPIO1->DATA &= ~(1 << 2);

	// OUT1 (must be low)
	LPC_IOCON->PIO1_4 = 0xC0;
	LPC_GPIO1->DIR |= (1 << 4);
	LPC_GPIO1->DATA &= ~(1 << 4);

	// OUT4 (must be low)
	LPC_IOCON->PIO1_5 = 0xC0;
	LPC_GPIO1->DIR |= (1 << 5);
	LPC_GPIO1->DATA &= ~(1 << 5);

	// RXD (must be high)
	LPC_IOCON->PIO1_6 = 0xC0;
	LPC_GPIO1->DIR |= (1 << 6);
	LPC_GPIO1->DATA |= (1 << 6);

	// TXD
	LPC_IOCON->PIO1_7 = 0xC0;
	LPC_GPIO1->DIR |= (1 << 7);
	LPC_GPIO1->DATA |= (1 << 7);

	// GPS_OFF (doesnt matter if high or high impedance, DO NOT SET LOW!)
	// LPC_IOCON->PIO1_8 = 0xC0;
	// LPC_GPIO1->DIR |= (1 << 8);
	// LPC_GPIO1->DATA |= (1 << 8);

	// RADIO_SDN (must be high, DO NOT SET LOW OR HIGH IMPEDANCE!)
	LPC_IOCON->PIO1_10 = 0xC0;
	LPC_GPIO1->DIR |= (1 << 10);
	LPC_GPIO1->DATA |= (1 << 10);

	// OUT2 (must be low)
	LPC_IOCON->PIO1_11 = 0xC0;
	LPC_GPIO1->DIR |= (1 << 11);
	LPC_GPIO1->DATA &= ~(1 << 11);

	// OUT3 (must be low)
	LPC_IOCON->PIO3_2 = 0xC0;
	LPC_GPIO3->DIR |= (1 << 2);
	LPC_GPIO3->DATA &= ~(1 << 2);

	// GPIO0 (must be low)
	LPC_IOCON->PIO3_4 = 0xC0;
	LPC_GPIO3->DIR |= (1 << 4);
	LPC_GPIO3->DATA &= ~(1 << 4);

	// GPIO1 (must be low)
	LPC_IOCON->PIO3_5 = 0xC0;
	LPC_GPIO3->DIR |= (1 << 5);
	LPC_GPIO3->DATA &= ~(1 << 5);

	LPC_IOCON->RESET_PIO0_0 = 0xC1; // disables reset
	LPC_IOCON->SWCLK_PIO0_10 = 0xC1; // disables SWCLK
	LPC_IOCON->SWDIO_PIO1_3 = 0xC1; // ADC pin, disables SWDIO
}

