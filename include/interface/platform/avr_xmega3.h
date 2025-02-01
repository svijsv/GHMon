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
/// @file
/// @brief Platform-specific features.
/// @note
///    This file should only be included by interface.h
/// @note
///    This file is only included when using the AVR_XMEGA3 platform
///

#if uHAL_USE_HIBERNATE || __HAVE_DOXYGEN__
///
/// @name Real-Time Timer Calibration
///
/// The calibration value is the number of milliseconds expected to pass during
/// @c RTT_CALIBRATE_CYCLES RTT source clock cycles (set in the configuration file).
///
/// @note
/// These are only available when uHAL_USE_HIBERNATE is set.
/// @{
//
///
/// Run the auto-calibration routine.
///
/// @note
/// This is not available if the RTT is used for the system tick.
void calibrate_RTT(void);
///
/// Adjust the current calibration value.
///
/// @param adj The amount to adjust the calibration value by.
void adj_RTT_calibration(int_fast16_t adj);
///
/// Set a new calibration value.
///
/// @param cal The new calibration value. Ignored if @c 0.
void set_RTT_calibration(uint_fast16_t cal);
///
/// Get the current calibration value.
///
/// @returns The current calibration value.
uint_fast16_t get_RTT_calibration(void);
/// @}
#endif // uHAL_USE_HIBERNATE
