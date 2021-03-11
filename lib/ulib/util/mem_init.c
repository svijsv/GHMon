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
// mem_init.c
//
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#if ! USE_MONOLITHIC_ULIB

//#define DEBUG 1
/*
* Includes
*/
#include "private.h"


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
#if USE_CLIB_STRING
void mem_init(void *mem, uint8_t value, uint32_t size) {
	memset(mem, value, size);
	return;
}

#else // !USE_CLIB_STRING
void mem_init(void *mem, uint8_t value, uint32_t size) {
	uint32_t i;
	uint8_t *as_8;

	assert(mem != NULL);

	for (i = 0, as_8 = mem; i < size; as_8[i++] = value) {
		// Nothing to do in here
	}

	return;
}
#endif // USE_CLIB_STRING


#endif // ! USE_MONOLITHIC_ULIB
#ifdef __cplusplus
 }
#endif
