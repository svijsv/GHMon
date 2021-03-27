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
// from_fstr.c
// Convert a flash string into a normal string
//
// NOTES:
//    These functions are only used for MCUs with separate flash and RAM
//    address spaces
//
//    Hopefully the compiler is smart enough to remove any unused FROM_FSTR()
//    variants
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


#if USE_AVR
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
char* FROM_FSTR(_FLASH_STR_T *fs) {
	static char _FLASH_TMP[_FLASH_TMP_SIZE];
	uint8_t i;

	assert(fs != NULL);

	for (i = 0; ((_FLASH_TMP[i] = fs[i]) != 0) && (i < (_FLASH_TMP_SIZE-1)); ++i) {
		// Nothing to do here
	}
	_FLASH_TMP[i] = 0;

	return _FLASH_TMP;
}
char* FROM_FSTR1(_FLASH_STR_T *fs) {
	static char _FLASH_TMP[_FLASH_TMP_SIZE];
	uint8_t i;

	assert(fs != NULL);

	for (i = 0; ((_FLASH_TMP[i] = fs[i]) != 0) && (i < (_FLASH_TMP_SIZE-1)); ++i) {
		// Nothing to do here
	}
	_FLASH_TMP[i] = 0;

	return _FLASH_TMP;
}
char* FROM_FSTR2(_FLASH_STR_T *fs) {
	static char _FLASH_TMP[_FLASH_TMP_SIZE];
	uint8_t i;

	assert(fs != NULL);

	for (i = 0; ((_FLASH_TMP[i] = fs[i]) != 0) && (i < (_FLASH_TMP_SIZE-1)); ++i) {
		// Nothing to do here
	}
	_FLASH_TMP[i] = 0;

	return _FLASH_TMP;
}
char* FROM_FSTR3(_FLASH_STR_T *fs) {
	static char _FLASH_TMP[_FLASH_TMP_SIZE];
	uint8_t i;

	assert(fs != NULL);

	for (i = 0; ((_FLASH_TMP[i] = fs[i]) != 0) && (i < (_FLASH_TMP_SIZE-1)); ++i) {
		// Nothing to do here
	}
	_FLASH_TMP[i] = 0;

	return _FLASH_TMP;
}


#endif // USE_AVR

#endif // ! USE_MONOLITHIC_ULIB
#ifdef __cplusplus
 }
#endif
