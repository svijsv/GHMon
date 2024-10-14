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
/// @attention
///    This file should only be included by interface.h
/// @attention
///    This file is only included when using the CMSIS platform
///

///
/// @name Clock Calibration
///
/// @attention
/// These are only available when Timer 5 is present.
/// @{
//
///
/// The type used to specify a low-speed oscillator source.
typedef enum {
	CALIB_LSI, ///< The internal low-speed oscillator.
	CALIB_LSE  ///< The external low-speed oscillator.
} osc_calib_t;
///
/// Measure a low-speed oscillator against a 1MHz signal derived from the
/// high-speed oscillator.
///
/// @attention
/// It can take several seconds for the LSE to stabilize after being enabled,
/// check the datasheet.
/// @attention
/// STM32F1 devices don't support measuring the low-speed external oscillator.
///
/// @param mode The low-speed oscillator to measure.
/// @param LS_cycles If non-NULL, set to the number of elapsed low-speed cycles.
/// @param HS_cycles If non-NULL, set to the number of elapsed high-speed cycles.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t tim5_oscillator_calibration(osc_calib_t mode, uint32_t *LS_cycles, uint32_t *HS_cycles);
///
/// Calibrate the RTC clock against the high-speed oscillator.
///
/// @attention
/// This only does anything when @c uHAL_USE_INTERNAL_LS_OSC is set.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t calibrate_RTC_clock(void);
/// @}
