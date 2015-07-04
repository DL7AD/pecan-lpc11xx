Pecan Tracker code for NXP LPC11xx microcontrollers
===================================================

made for
  * Pecan Femto 2.1 (based on LPC1114FHN33/303)
  * Pecan Pico 6 (based on LPC1114FHN33/333)

Known bugs
  * ADC reading does not work correctly in burst mode (OVERRUN error)

Todo:
  * Watchdog
  * Accurate geofencing for frequency switching
  * Evaluation whether ublox MAX8 works at >12,000m altitude or not
  * GPS jump filter
  * Full tracker reset when GPS lost

Versions
  * v0.1 has been used for one Balloon (Qualatex Floater at 7000m), sometimes it didn't transmit for several minutes (up to one hour) but started again (could be caused by inaccurate frequency [shifting] or software bug)
  * v0.2 stopped transmitting (~~probably software bug, e.g. hard fault~~ Antenna has broken off)
