/***********************************************************************
 * $Id:: small_printf.h 4181 2010-08-03 23:14:09Z nxp28548             $
 *
 * Description:
 *     Small printf library based on LGPL code- see following header.
 *     Changelog:
 * 		Added * printf specifier to get field width from argument list
 *  	Printf functions accept function pointer to output routine
 *  	Duplicate float / non-float functions allow creation of
 *  	    static library (linker pulls in only referenced function)
 *
 ***********************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 **********************************************************************/

/*
Copyright 2001, 2002 Georges Menie (www.menie.org)
stdarg version contributed by Christian Ettinger

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
putchar is the only external dependency for this file,
if you have a working putchar, leave it commented out.
If not, uncomment the define below and
replace outbyte(c) by your own function call.

#define putchar(c) outbyte(c)
*/
#ifndef SMALLPRINTF_H_INCLUDED
#define SMALLPRINTF_H_INCLUDED
#include <stdarg.h>

typedef int (*fp_printf_write_func)(char c);


extern int func_printf_nofloat(const fp_printf_write_func printf_write, const char *format, ...)
	__attribute__ ((format (printf, 2, 3)));

extern int printf_format_nofloat(const fp_printf_write_func printf_write, const char *format, va_list varg);

// We don't implement sprintf because of the risk of buffer overruns
extern int snprintf_nofloat(char *buffer, int buffer_length, const char *format, ...)
    __attribute__ ((format (printf, 3, 4)));

extern int func_printf_float(const fp_printf_write_func printf_write, const char *format, ...)
	__attribute__ ((format (printf, 2, 3)));

extern int printf_format_float(const fp_printf_write_func printf_write, const char *format, va_list varg);

// We don't implement sprintf because of the risk of buffer overruns
extern int snprintf_float(char *buffer, int buffer_length, const char *format, ...)
    __attribute__ ((format (printf, 3, 4)));

// Internal functions defined in small_printf_support.c
// Used by small_printf.c and small_printf_float.c which should be identical except for
// #define enabling and disabling float at the top
int prints(const fp_printf_write_func printf_write, const char *string, int width, int pad);
int printi(const fp_printf_write_func printf_write, int i, int b, int sg, int width, int pad, int letbase);
void nsprintf_write_init();
int nsprintf_write(char c);

#ifdef LIB_FLOAT_PRINTF
#define func_printf(...)	( func_printf_float(__VA_ARGS__))
#define printf_format(...) 	( printf_format_float(__VA_ARGS__))
#define snprintf(...) 		( snprintf_float(__VA_ARGS__))
#else
#define func_printf(...) 	( func_printf_nofloat(__VA_ARGS__))
#define printf_format(...) 	( printf_format_nofloat(__VA_ARGS__))
#define snprintf(...) 		( snprintf_nofloat(__VA_ARGS__))
#endif

#endif
