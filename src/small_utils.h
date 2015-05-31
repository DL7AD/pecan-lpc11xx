/***********************************************************************
 * $Id:: small_utils.h 4181 2010-08-03 23:14:09Z nxp28548              $
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

#ifndef SMALLUTILS_H_INCLUDED
#define SMALLUTILS_H_INCLUDED

int small_strlen(const char *str);
int small_strcmp(const char *str1, const char *str2);
int small_stricmp(const char *str, const char *str2);
void small_strim(char *str);

unsigned long gethex(const char *s);

double small_fmodf(double f, double div);

#endif
