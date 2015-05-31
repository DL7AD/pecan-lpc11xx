/*
 * GENERATED FILE - DO NOT EDIT
 * (c) Code Red Technologies Ltd, 2008-2015
 * (c) NXP Semiconductors 2013-2015
 * Linker script memory definitions
 * Created from LinkMemoryTemplate
 * By LPCXpresso v7.7.2 [Build 379] [2015-03-31]  on Sun Apr 26 10:19:10 UTC 2015)
*/

MEMORY
{
  /* Define each memory region */
  MFlash56 (rx) : ORIGIN = 0x0, LENGTH = 0xe000 /* 56K bytes */
  RamLoc8 (rwx) : ORIGIN = 0x10000000, LENGTH = 0x2000 /* 8K bytes */


}
  /* Define a symbol for the top of each memory region */
  __top_MFlash56 = 0x0 + 0xe000;
  __top_RamLoc8 = 0x10000000 + 0x2000;



