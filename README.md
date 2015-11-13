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
- Log transmission (please contact me for activating log server which igates the log to HABHUB)

made for
  * Pecan Femto 2.1 (based on LPC1114FHN33/303)
  * Pecan Pico 6 (based on LPC1114FHN33/333) https://github.com/DL7AD/pecanpico6

Known bugs
  * none

Todo:
  * Watchdog
  * Accurate geofencing for frequency switching

Author: sven.steudte@gmail.com
