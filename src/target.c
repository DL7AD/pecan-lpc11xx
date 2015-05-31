/**
 * Clocking helper. Parts were taken from Github.
 * @see https://github.com/Zuph/lpc1114-blink/blob/master/main.c
 */

#include "target.h"

uint32_t fcclk;

void TargetResetInit(void)
{
	Target_SetClock_IRC(); // Set cock to IRC (12MHz)

	LPC_SYSCON->SYSAHBCLKDIV  = 1;
	LPC_SYSCON->SYSAHBCLKCTRL = AHBCLKCTRL_Val;
}

/**
 * brief Sets the main clock to IRC (12MHz). Disables PLL.
 * Reconfigures System Tick Timer.
 */
void Target_SetClock_IRC(void)
{
	LPC_SYSCON->MAINCLKSEL = 0x00;				// select IRC
	LPC_SYSCON->MAINCLKUEN = 0x01;				// Toggle Update Register
	LPC_SYSCON->MAINCLKUEN = 0x00;
	LPC_SYSCON->MAINCLKUEN = 0x01;
	while (!(LPC_SYSCON->MAINCLKUEN & 1));

	LPC_SYSCON->PDRUNCFG	|=  (1 << 7);		// Power-Down SYSPLL
	SysTick_Config(Fosc / 1000);				// Configure tick timer
	fcclk = Fosc;
}

/**
 * @brief Configures flash access time.
 * @details Configures flash access time which allows the chip to run at higher
 * speeds.
 *
 * @param frequency defines the target frequency of the core
 */
void Set_Flash_Access_Time(uint32_t frequency)
{
	uint32_t access_time, flashcfg_register;

	if (frequency < 20000000ul)				// 1 system clock for core speed below 20MHz
		access_time = 0x0;
	else if (frequency < 40000000ul)		// 2 system clocks for core speed between 20MHz and 40MHz
		access_time = 0x1;
	else									// 3 system clocks for core speed over 40MHz
		access_time = 0x2;

	// do not modify reserved bits in FLASHCFG register
	flashcfg_register = LPC_FLASHCTRL->FLASHCFG;			// read register
	flashcfg_register &= ~(0x3 << 0x0);	// mask the FLASHTIM field
	flashcfg_register |= access_time << 0x0;	// use new FLASHTIM value
	LPC_FLASHCTRL->FLASHCFG = flashcfg_register;			// save the new value back to the register
}

/**
 * @brief Starts the PLL.
 * @details Configure and enable PLL to achieve some frequency with some
 * crystal. Before the speed change flash access time is configured via
 * flash_access_time(). Main oscillator is configured and started. PLL
 * parameters m and p are based on function parameters. The PLL is configured,
 * started and selected as the main clock. AHB clock divider is set to 1.
 * The real frequency that was set is saved in global variable fcclk.
 * Reconfigures System Tick Timer to current clock.
 *
 * @param crystal is the frequency of the crystal resonator connected to
 * the LPC1114 chip.
 * @param frequency is the desired target frequency after enabling the PLL
 */

void Target_SetClock_PLL(uint32_t crystal, uint32_t frequency)
{
	uint32_t m, p = 0, fcco;

	Set_Flash_Access_Time(frequency);			// configure flash access time first

	// SYSOSCCTRL_FREQRANGE should be 0 for crystals in range 1 - 20MHz
	// SYSOSCCTRL_FREQRANGE should be 1 for crystals in range 15 - 25MHz
	if (crystal < 17500000)					// divide the ranges on 17.5MHz then
		LPC_SYSCON->SYSOSCCTRL = 0;			// "lower speed" crystals
	else
		LPC_SYSCON->SYSOSCCTRL = (1 << 1);	// "higher speed" crystals

	LPC_SYSCON->PDRUNCFG &= ~(1 << 5);	// power-up main oscillator

	LPC_SYSCON->SYSPLLCLKSEL = (0 << 0);	// select main oscillator as the input clock for PLL
	LPC_SYSCON->SYSPLLCLKUEN = 0;			// confirm the change of PLL input clock by toggling the...
	LPC_SYSCON->SYSPLLCLKUEN = (1 << 0);	// ...ENA bit in LPC_SYSCON->SYSPLLCLKUEN register

	// calculate PLL parameters
	m = frequency / crystal;				// M is the PLL multiplier
	fcco = m * crystal * 2;					// FCCO is the internal PLL frequency

	fcclk = crystal * m;

	while (fcco < 156000000)
	{
		fcco *= 2;
		p++;								// find P which gives FCCO in the allowed range (over 156MHz)
	}

	LPC_SYSCON->SYSPLLCTRL = ((m - 1) << 0) | (p << 5);	// configure PLL
	LPC_SYSCON->PDRUNCFG &= ~(1 << 7); // power-up PLL

	while (!(LPC_SYSCON->SYSPLLSTAT & (1 << 0)));	// wait for PLL lock

	LPC_SYSCON->MAINCLKSEL = (0x3 << 0);	// select PLL output as the main clock
	LPC_SYSCON->MAINCLKUEN = 0;				// confirm the change of main clock by toggling the...
	LPC_SYSCON->MAINCLKUEN = (1 << 0);	// ...ENA bit in LPC_SYSCON->MAINCLKUEN register

	LPC_SYSCON->SYSAHBCLKDIV = 1;			// set AHB clock divider to 1

	SysTick_Config(fcclk / 1000);			// Configure tick timer
}

/**
 * Returns the current clock speed.
 * @return Fcclk frequency
 */
uint32_t getFcclk(void)
{
	return fcclk;
}
