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
// types.h
// Common type definitions
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _ULIB_TYPES_H
#define _ULIB_TYPES_H

/*
* Includes
*/
#include "common.h"

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

/*
* Static values
*/

/*
* Types
*/
#ifndef __cplusplus
/*
typedef enum {
	false = 0,
	true = 1
} bool;
*/
// enums are int-sized, this may be smaller
typedef uint_fast8_t bool;
# define true  1
# define false 0
#endif // __cplusplus

typedef unsigned int uint;

typedef enum {
	EOK = 0,
	EUNKNOWN,
	EABORT,
	ETIMEOUT,
	EUSAGE,
	EINVALID,
	ENOTREADY,
	EPERM,
	EMEM,
	EDISK,
} err_t;

// Types (optionally) used for bitfields
typedef uint32_t bitop_t;
typedef bitop_t  flag_t;

// A smaller, unsigned time-tracking type for embedded systems
typedef uint32_t utime_t;

// Types used for generic iterators
typedef int_fast8_t  iter_t;
typedef uint_fast8_t uiter_t;

/*
* Variable declarations (defined in types.c)
*/

/*
* Function prototypes (defined in types.c)
*/

/*
* Macros
*/

#endif // _LIB_TYPES_H
#ifdef __cplusplus
 }
#endif
