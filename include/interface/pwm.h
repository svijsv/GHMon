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
/// @brief Pulse-Width Modulation Interface
/// @attention
///    This file should only be included by interface.h.
///

///
/// Enable hardware PWM output on an IO pin.
///
/// @attention
/// The pin mode may be changed by the backend when PWM is enabled.
///
/// @param output The handle used to manage the pin. Must not be NULL.
/// @param pin The pin to enable PWM on. If @c 0, attempt to use the last pin associated with the handle.
/// @param duty_cycle The initial duty cycle of @c output.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t pwm_on(pwm_output_t *output, gpio_pin_t pin, uint_fast16_t duty_cycle);
///
/// Disable hardware PWM output on an IO pin.
///
/// @attention
/// The pin may be set to HiZ mode when turned off.
///
/// @param output The handle used to manage the pin. Must not be NULL.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t pwm_off(pwm_output_t *output);
///
/// Adjust hardware PWM output on an IO pin.
///
/// @param output The handle used to manage the pin. Must not be NULL.
/// @param duty_cycle The duty cycle to set @c output to.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t pwm_set(pwm_output_t *output, uint_fast16_t duty_cycle);
