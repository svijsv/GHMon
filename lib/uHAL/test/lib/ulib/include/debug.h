// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2022, 2023 svijsv                                          *
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
* along with this program. If not, see <http:// www.gnu.org/licenses/>.*
*                                                                      *
*                                                                      *
***********************************************************************/
// debug.h
// Tools to aid in debugging
// NOTES:
//
//
#ifndef _ULIB_DEBUG_H
#define _ULIB_DEBUG_H

#include "src/configify.h"
#include "fmem.h"
#include "msg.h"
#include "types.h"

#if !ULIB_ENABLE_FMEM
# define F(s) (s)
# define F1(s) (s)
#endif

#if USE_ULIB_ASSERT
# if USE_ULIB_LOCAL_ASSERT
#  ifndef NDEBUG
// Function called when an assertion fails
// ulib_assert_failed() is project-specific and must be defined somewhere if
// NDEBUG is undefined and USE_ULIB_ASSERT is set.
void ulib_assert_failed(const char *file_path, uint32_t lineno, const char *func_name, const char *expr);

//# define ulib_assert(_exp_) ((_exp_) ? (void)0 : ulib_assert_failed(__FILE__, __LINE__, __func__, #_exp_))
// Using F() with only #_exp_ saves more RAM and uses less flash than using it
// only with __FILE__, presumably because the multiple uses of each file name
// are deduplicated in RAM but not flash
#   define ulib_assert(_exp_) ((_exp_) ? (void )0 : ulib_assert_failed(F1(__FILE__), __LINE__, __func__, F(#_exp_)))
#  else // NDEBUG
#   define ulib_assert(_exp_) ((void )0)
#  endif // NDEBUG
#  if USE_ULIB_ASSERT_OVERRIDE
#   define assert(_exp_) ulib_assert(_exp_)
#  endif
# else // USE_ULIB_LOCAL_ASSERT
#  include <assert.h>
#  define ulib_assert(_exp_) assert(_exp_)
# endif // USE_ULIB_LOCAL_ASSERT
#else // USE_ULIB_ASSERT
# define ulib_assert(_exp_) ((void )0)
#endif

#if USE_ULIB_PANIC
# if USE_ULIB_LOCAL_PANIC
// Function called when an assertion fails
// ulib_panic_abort() is project-specific and must be defined somewhere if
// USE_ULIB_PANIC is set.
void ulib_panic_abort(const char *file_path, uint32_t lineno, const char *func_name, const char *msg);
// Using F() with only #_exp_ saves more RAM and uses less flash than using it
// only with __FILE__, presumably because the multiple uses of each file name
// are deduplicated in RAM but not flash
// Edit: That's true for assert(), but maybe messages will be repeated more
// often than expressions are? TODO: test this theory.
//#  define ulib_panic(_msg_) ulib_assert_failed(F1(__FILE__), __LINE__, __func__, F(_msg_))
#  define ulib_panic(_msg_) ulib_assert_failed(F1(__FILE__), __LINE__, __func__, _msg_)
# else // USE_ULIB_LOCAL_PANIC
#  include <stdlib.h>
// The message passed to ulib_panic is ignored here, but such is life.
#  define ulib_panic(_msg_) abort()
# endif // USE_ULIB_LOCAL_PANIC
#else // USE_ULIB_PANIC
# define ulib_panic(_msg_) ((void )0)
#endif

#define PRINT_HERE() msg_debug("-HERE- %s() %s:%d", __func__, F1(__FILE__), __LINE__)
#define PRINT_VALUE(_val_) (msg_debug("%s: 0x%02X", #_val_, (uint_t )_val_))

//
// Use these to show the value of a macro at compile-time (among other possible uses)
// https://stackoverflow.com/questions/1562074/how-do-i-show-the-value-of-a-define-at-compile-time
#define XTRINGIZE(_x_) STRINGIZE(_x_)
#define STRINGIZE(_x_) #_x_
#define DO_PRAGMA(_x_) _Pragma(#_x_)

#if !defined(DEBUG_CPP_MESSAGES)
# if DEBUG
#  define DEBUG_CPP_MESSAGES 1
# else
#  define DEBUG_CPP_MESSAGES 0
# endif
#endif
#if DEBUG_CPP_MESSAGES
// Display the value of a defined macro
# define DEBUG_CPP_MACRO(_x_) DO_PRAGMA(message #_x_ " == " XTRINGIZE(_x_))
// Display a textual message
# define DEBUG_CPP_MSG(_msg_) DO_PRAGMA(message _msg_)
#else
# define DEBUG_CPP_MACRO(_x_)
# define DEBUG_CPP_MSG(_msg_)
#endif
// Like the above, but display it always
#define PRINT_CPP_MACRO(_x_) DO_PRAGMA(message #_x_ " == " XTRINGIZE(_x_))

#endif // _ULIB_DEBUG_H
