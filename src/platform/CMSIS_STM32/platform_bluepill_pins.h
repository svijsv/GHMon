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
// platform_*_pins.h
// Platform-specific shim for the frontend
// NOTES:
//   This file is for defining board-specific pin names without cluttering
//   platform.h and should only be included by platform.h
//
//   These are the Bluepill pin mappings
//   They should be accurate for pretty much all other boards too, though there
//   are some that use a more Arduino-like scheme or have additional pins
//   Not all pins will be broken out on all boards
//

#define PIN_A0  PINID_A0
#define PIN_A1  PINID_A1
#define PIN_A2  PINID_A2
#define PIN_A3  PINID_A3
#define PIN_A4  PINID_A4
#define PIN_A5  PINID_A5
#define PIN_A6  PINID_A6
#define PIN_A7  PINID_A7
#define PIN_A8  PINID_A8
#define PIN_A9  PINID_A9
#define PIN_A10 PINID_A10
#define PIN_A11 PINID_A11
#define PIN_A12 PINID_A12
// PINID_A13 and PINID_A14 are used for SWDIO and SWCLK when DEBUG is enabled
#if ! DEBUG
# define PIN_A13 PINID_A13
# define PIN_A14 PINID_A14
#endif
#define PIN_A15 PINID_A15

#define PIN_B0  PINID_B0
#define PIN_B1  PINID_B1
#define PIN_B3  PINID_B3
#define PIN_B4  PINID_B4
#define PIN_B5  PINID_B5
#define PIN_B6  PINID_B6
#define PIN_B7  PINID_B7
#define PIN_B8  PINID_B8
#define PIN_B9  PINID_B9
#define PIN_B10 PINID_B10
#define PIN_B11 PINID_B11
#define PIN_B12 PINID_B12
#define PIN_B13 PINID_B13
#define PIN_B14 PINID_B14
#define PIN_B15 PINID_B15
