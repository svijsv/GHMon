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
// assert.h
// A sort-of-replacement for assert.h
// NOTES:
//   Normally assert() is present unless NDEBUG is defined, with this header
//   assert() is absent unless DEBUG is defined.

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _ULIB_ASSERT_H
#define _ULIB_ASSERT_H

/*
* Includes
*/
#include "common.h"
#include "fmem.h"

/*
* Static values
*/

/*
* Types
*/

/*
* Variables (defined in assert.c)
*/

/*
* Functions (defined in assert.c)
*/
// Function called when an assertion fails
// _assert_failed() is project-specific and must be defined somewhere if
// debugging is enabled.
void _assert_failed(const char *file_path, uint32_t lineno, const char *func_name, const char *expr);

/*
* Macros
*/
#if DEBUG && ! NO_ASSERT
//# define assert(exp) ((exp) ? (void)0 : _assert_failed(__FILE__, __LINE__, __func__, #exp))
// Using F() with only #exp saves more RAM and uses less flash than using it
// only with __FILE__, presumably because the multiple uses of each file name
// are deduplicated in RAM but not flash
# define assert(exp) ((exp) ? (void)0 : _assert_failed(F1(__FILE__), __LINE__, __func__, F(#exp)))
#else
# define assert(exp) ((void)0)
#endif

#endif // _LIB_ASSERT_H
#ifdef __cplusplus
 }
#endif
