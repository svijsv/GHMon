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
// NOTES:
//
#ifndef _COMMON_H
#define _COMMON_H


#include "uHAL/include/interface.h"
#include "ulib/include/debug.h"

#ifndef HAVE_AVR_XMEGA3
# define HAVE_AVR_XMEGA3 0
#endif
#ifndef HAVE_STM32
# define HAVE_STM32 0
#endif

#if HAVE_STM32
# define ISR(_isr_) void _isr_(void)
#endif

#define TEST_TEMPLATE 0


#endif // _COMMON_H
