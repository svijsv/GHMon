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
// private.h
// Module private members
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _LIB_TIME_PRIVATE_H
#define _LIB_TIME_PRIVATE_H

/*
* Includes
*/
#include "../time.h"
#include "../assert.h"


/*
* Static values
*/


/*
* Types
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
// Calculate the number of leaping years up to some year
// This calculation excludes YEAR_0 when it's a multiple of the factor
// This calculation includes year y when it's a multiple of the factor and not 0
#define GET_LEAPS(y, f) (((YEAR_0 + (int16_t )(y)) - (YEAR_0 - (YEAR_0 % (f)))) / (f))
// Determine if a year relative to YEAR_0 is a leap year
#define IS_LEAP_YEAR(y) ((((YEAR_0 + (y)) % 4) == 0) && ((((YEAR_0 + (y)) % 100) != 0) || (((YEAR_0 + (y)) % 400) == 0)))


#endif // _LIB_TIME_PRIVATE_H
#ifdef __cplusplus
 }
#endif
