// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021, 2024 svijsv                                          *
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
// config_unify.h
// Determine which subsystems to enable or disable based on user configuration
// NOTES:
//
#ifndef USE_SMALL_SENSORS
# define USE_SMALL_SENSORS USE_SMALL_BUILD
#endif
#ifndef USE_SMALL_CONTROLLERS
# define USE_SMALL_CONTROLLERS USE_SMALL_BUILD
#endif
#ifndef USE_SMALL_CODE
# define USE_SMALL_CODE USE_SMALL_BUILD
#endif
#ifndef SKIP_SAFETY_CHECKS
# define SKIP_SAFETY_CHECKS USE_SMALL_BUILD
#endif
#ifndef SKIP_LIB_SAFETY_CHECKS
# define SKIP_LIB_SAFETY_CHECKS SKIP_SAFETY_CHECKS
#endif

#define uHAL_USE_UART_COMM (USE_UART_OUTPUT || USE_UART_TERMINAL)
#define ENABLE_UART_LISTENING (USE_UART_TERMINAL)
#define uHAL_USE_TERMINAL (USE_UART_TERMINAL)
#define uHAL_USE_UART (uHAL_USE_UART_COMM || uHAL_USE_TERMINAL)
#define ULIB_ENABLE_CSTRINGS (uHAL_USE_TERMINAL)
