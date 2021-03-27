// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021 svijsv                                                *
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
// fmem.h
// Helpers for working with data in a separate flash address space
//
// NOTES:
//    Unfortunately there's no real way to do typechecking between flash and
//    RAM strings (or other data for that matter) except for using dummy
//    container structs, and those aren't always practical in the current
//    use case
//
//    The F() macro seems to increase flash usage by double what it saves in
//    RAM, so probably isn't worth using in spite of it's ease
//
// TODO:
//
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _ULIB_FMEM_H
#define _ULIB_FMEM_H

/*
* Includes
*/
#include "common.h"
#include "types.h"
#include "util.h"

#if USE_AVR
/*
* Static values
*/
#define _FLASH __flash

// The size of the static array used to return strings from FROM_STR()
#if ! _FLASH_TMP_SIZE
# define _FLASH_TMP_SIZE 128
#endif

/*
* Variables
*/

/*
* Types
*/
/*
typedef struct {
	char *contents;
} fstring_t;
*/
//#define _FLASH_STR_T _FLASH const fstring_t
#define _FLASH_STR_T _FLASH const char

/*
* Function prototypes
*/
// Macro and inline versions of FROM_FSTR() are possible but take a LOT more
// program space per invocation
// Multiple versions are defined because they use internal static arrays and
// so can't be used more than once in a single expression
char* FROM_FSTR(_FLASH_STR_T *fs);
char* FROM_FSTR1(_FLASH_STR_T *fs);
char* FROM_FSTR2(_FLASH_STR_T *fs);
char* FROM_FSTR3(_FLASH_STR_T *fs);

/*
* Macros
*/
#define FSTR(s) ((_FLASH_STR_T *)({static _FLASH const char __c[] = (s); &__c[0];}))
// Use the name 'F()' (borrowed from arduino) rather than PSTR() (used by
// avr-gcc) because we're building with the latter and want to avoid clashes
#define F(s) FROM_FSTR(FSTR(s))
#define F1(s) FROM_FSTR1(FSTR(s))
#define F2(s) FROM_FSTR2(FSTR(s))
#define F3(s) FROM_FSTR3(FSTR(s))


#else // !USE_AVR
/*
* Static values
*/
#define _FLASH

/*
* Types
*/
/*
typedef struct {
	char *contents;
} fstring_t;
*/
//#define _FLASH_STR_T _FLASH const fstring_t
#define _FLASH_STR_T _FLASH const char

/*
* Macros
*/
#define FSTR(s) (s)
#define FROM_FSTR(s) (s)
#define FROM_FSTR1(s) (s)
#define FROM_FSTR2(s) (s)
#define FROM_FSTR3(s) (s)
#define F(s) (s)
#define F1(s) (s)
#define F2(s) (s)
#define F3(s) (s)


#endif // USE_AVR

#endif // _LIB_FMEM_H
#ifdef __cplusplus
 }
#endif
