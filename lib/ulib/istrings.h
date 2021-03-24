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
// istrings.h
// A library for working with C-style strings using macros
// NOTES:
//   Avoid casting input pointers; this may be used with __flash or similar
//   address space work-arounds
//
//   These macros make use of the GCC extension ({...}) to 'return' values

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _ULIB_ISTRINGS_H
#define _ULIB_ISTRINGS_H

/*
* Includes
*/
#include "common.h"

#include "types.h"
#include "cstrings.h"

/*
* Types
*/

/*
* Static values
*/

/*
* Variable declarations
*/

/*
* Function prototypes
*/

/*
* Macros
*/
//#define ISTRING_LEN(s) do { cstrlen_t i = 0; for (; c[i]; ++i) {}; i; } while (0)
#define ISTRING_LEN(s) ({ cstrlen_t i = 0; for (; s[i]; ++i) {}; i; })

#define ISTRING_CMP(s1, s2) \
	({ \
		int r; \
		cstrlen_t i = 0; \
		for (; (s1[i] && (s1[i] == s2[i])); ++i) {}; \
		r = (s1[i] == s2[i]) ? 0 : ((uint8_t )s1[i] > (uint8_t )s2[i]) ? 1 : -1; \
		r; \
	})

#define ISTRING_NCMP(s1, s2, n) \
	({ \
		int r; \
		cstrlen_t i = 0; \
		for (; s1[i] && (s1[i] == s2[i]) && (i < n); ++i) {}; \
		if (i == n) --i; \
		r = (s1[i] == s2[i]) ? 0 : ((uint8_t )s1[i] > (uint8_t )s2[i]) ? 1 : -1; \
		r; \
	})

#endif // _LIB_ISTRINGS_H
#ifdef __cplusplus
 }
#endif
