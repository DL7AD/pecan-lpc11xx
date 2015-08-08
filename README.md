Pecan Tracker code for NXP LPC11xx microcontrollers
===================================================

This is the APRS tracking software made for Pecan Pico 6 and Pecan Femto 2.1. It transmits APRS-telemetry & -position packets in regular timed intervals. The software supports transmitting:
- GPS-Position
- GPS-Satellites being used
- GPS-Altitude
- Airpressure (if tracker has BMP180)
- Temperature (if tracker has BMP180)
- Battery voltage
- Solar voltage (if tracker is solar-equipped)
- Static comment

made for
  * Pecan Femto 2.1 (based on LPC1114FHN33/303)
  * Pecan Pico 6 (based on LPC1114FHN33/333)

Note: Modulation only works perfect when Compiler optimization is switch on (-O1 at least)

Known bugs
  * ~~ADC reading does not work correctly in burst mode (OVERRUN error)~~ solved, ADC conversion has been started too early, LDO_Vout capacitors have been charged by internal GPIO pull up to VCC and haven't been discharged

Todo:
  * Watchdog
  * Backlog transmission
  * Accurate geofencing for frequency switching
  * ~~Evaluation whether ublox MAX8 works at >12,000m altitude or not~~ Has been checked with GPS Simulator, bug fixed
  * ~~Evaluation whether APRS frequencies changes at different location~~ works perfectly
  * ~~GPS jump filter~~ Discarded, no major problem
  * ~~Full tracker reset when GPS lost~~ Discarded, GPS and UART Interface will be reset
  * ~~In-System-AFSK-Modulation~~ Done

Versions
  * v0.1 has been used for one Balloon (Qualatex Floater at 7000m), sometimes it didn't transmit for several minutes (up to one hour) but started again (caused by faulty NMEA-parsing)
  * v0.2 stopped transmitting (~~probably software bug, e.g. hard fault~~ Antenna has broken off)
