// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2024 svijsv                                                *
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
// fifo_uint8.c
// Manage a FIFO uffer of unsigned bytes
// NOTES:
//
//
#define HAVE_FIFO_UINT8_C 1
#include "fifo_uint8.h"

#if ULIB_ENABLE_FIFO_UINT8

#include "templates/fifo_buffer.c"

#else
	// ISO C forbids empty translation units, this makes it happy.
	typedef int make_iso_compilers_happy;
#endif // ULIB_ENABLE_FIFO_UINT8
