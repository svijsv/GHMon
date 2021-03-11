// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021 svijsv
* This program is free software: you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation, version 3.                             *
*                                                                      *
* This program is distributed in the hope that it will be useful, but  *
* WITHOUT ANY WARRANTY; without even the implied warranty of           *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU    *
* General Public License for more details.                             *
*                                                                      *
* You should have received a copy of the GNU General Public License    *
* along with this program.  If not, see <http:// www.gnu.org/licenses/>.*
*                                                                      *
*                                                                      *
***********************************************************************/
// vvprintf.c
//
// NOTES:
//
// If there are problems with the output of vvprintf() or vaprintf(), make sure
// you used the right one - the compiler can't check that for you.
//

#ifdef __cplusplus
 extern "C" {
#endif
#if ! USE_MONOLITHIC_ULIB

//#define DEBUG 1 1
/*
* Includes
*/
#include "private.h"

#if USE_XPRINTF_PRINTF
# include "xprintf/xprintf.h"
// xv[f]printf() is normally declared static; that needs to be changed in
// xfprintf.c
// void xvprintf (const char* fmt, va_list arp);
void xvfprintf (void(*func)(int), const char* fmt, va_list arp);
#endif // USE_XPRINTF_PRINTF

/*
* Static values
*/


/*
* Types
*/


/*
* Variables
*/


/*
* Local function prototypes
*/


/*
* Interrupt handlers
*/


/*
* Functions
*/
#if USE_XPRINTF_PRINTF
void vvprintf(printf_putc_t printf_putc, const char *fmt, ...) {
	va_list arp;

	assert(printf_putc != NULL);
	assert(fmt         != NULL);

	va_start(arp, fmt);
	xvfprintf(printf_putc, fmt, arp);
	va_end(arp);

	return;
}
#endif // USE_XPRINTF_PRINTF


#endif // ! USE_MONOLITHIC_ULIB
#ifdef __cplusplus
 }
#endif
