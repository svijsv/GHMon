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
// common.h
//
// NOTES:
//
#ifndef _uHAL_COMMON_H
#define _uHAL_COMMON_H


#include "interface.h"

#include "ulib/include/debug.h"

#if ! uHAL_USE_HIBERNATE
ALWAYS_INLINE void sleep_ms(utime_t ms) {
	delay_ms(ms);
}
#endif

#endif // _uHAL_COMMON_H
