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
// Platform-specific common header
// NOTES:
//
#ifndef _uHAL_PLATFORM_XMEGA3_COMMON_H
#define _uHAL_PLATFORM_XMEGA3_COMMON_H

#include "include/interface.h"

#include "ulib/include/debug.h"
#include "ulib/include/types.h"
#include "ulib/include/bits.h"
#include "ulib/include/time.h"
#include "ulib/include/util.h"


#define IRQ_IS_REQUESTED ((uHAL_CHECK_STATUS(uHAL_FLAG_IRQ)))
#define IRQ_IS_WAITING(_flags_) (BIT_IS_SET(_flags_, uHAL_CFG_ALLOW_INTERRUPTS) && IRQ_IS_REQUESTED)


#endif // _uHAL_PLATFORM_XMEGA3_COMMON_H
