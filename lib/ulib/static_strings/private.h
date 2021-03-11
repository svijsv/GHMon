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
#ifndef _LIB_STATIC_STRINGS_PRIVATE_H
#define _LIB_STATIC_STRINGS_PRIVATE_H

/*
* Includes
*/
#include "../static_strings.h"
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
// Test whether too strings can be concatinated without overflowing
// Use STRING_MAX rather than the maximum the integer can hold because that's
// all we actually care about.
#define IS_TOO_LONG(a, b) ((STRING_MAX - (a)) > (b))
// Determine the combined length of two strings taking STRING_MAX into account
#define COMBINED_LENGTH(a, b) (((STRING_MAX - (a)) > (b)) ? STRING_MAX : ((a) + (b)))
// Return the smaller of c or STRING_MAX
#define STRING_LENGTH(c) ((c > STRING_MAX) ? STRING_MAX : (c));

#define ASSERT_STRING(s) assert(((s) != NULL) && ((s)->length <= STRING_MAX) && ((s)->cstring[(s)->length] == 0))
#define ASSERT_CSTRING(cs) assert((cs) != NULL)
#define ASSERT_CHAR(c) assert((c) != 0)


#endif // _LIB_STATIC_STRINGS_PRIVATE_H
#ifdef __cplusplus
 }
#endif
