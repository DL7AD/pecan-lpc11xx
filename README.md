Pecan Tracker code for NXP LPC11xx microcontrollers
===================================================

made for
  * Pecan Femto 2.1 (based on LPC1114FHN33/303)
  * Pecan Pico 6 (based on LPC1114FHN33/333)

! Modulation only works perfect when compiler optimization is switch on (-O1 at least)

Known bugs
  * ~~ADC reading does not work correctly in burst mode (OVERRUN error)~~ solved, ADC conversion has been started too early, LDO_Vout capacitors have been charged by internal GPIO pull up to VCC and haven't been discharged

Todo:
  * Watchdog
  * Accurate geofencing for frequency switching
  * ~~Evaluation whether ublox MAX8 works at >12,000m altitude or not~~ Has been checked with GPS Simulator, bug fixed
  * ~~Evaluation whether APRS frequencies changes at different location~~ works perfectly
  * GPS jump filter
  * ~~Full tracker reset when GPS lost~~ Discarded, GPS and UART Interface will be reset
  * In-System-AFSK-Modulation (Branch mod_next, work in progress)

Versions
  * v0.1 has been used for one Balloon (Qualatex Floater at 7000m), sometimes it didn't transmit for several minutes (up to one hour) but started again (caused by faulty NMEA-parsing)
  * v0.2 stopped transmitting (~~probably software bug, e.g. hard fault~~ Antenna has broken off)
