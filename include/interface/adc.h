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
/// @brief Analog to Digital Converter Interface
/// @attention
///    This file should only be included by interface.h.
///

//
// This is defined in the device platform.h, it's included here for
// documentation purposes.
#if __HAVE_DOXYGEN__
///
/// Type used to hold the value read by an ADC.
///
/// @attention
/// The actual definition will vary based on the value of ADC_MAX.
typedef uint_t adc_t;

///
/// Type used to perform mathematical functions with ADC readings.
///
/// This is needed for operations which may overflow adc_t such as multiplication
/// or those which may involve negative values.
///
/// @attention
/// The actual definition will vary based on the value of ADC_MAX.
typedef long int adcm_t;

///
/// The maximum value returned by the ADC.
#define ADC_MAX
#else// __HAVE_DOXYGEN__

# if ADC_MAX <= 0xFFU
typedef uint_fast8_t adc_t;
typedef int_fast16_t adcm_t;
# elif ADC_MAX <= 0xFFFFU
typedef uint_fast16_t adc_t;
typedef int_fast32_t adcm_t;
# elif ADC_MAX <= 0xFFFFFFFFU
typedef uint_fast32_t adc_t;
typedef int_fast64_t adcm_t;
# else
typedef uint_fast64_t adc_t;
typedef int_fast64_t adcm_t;
# endif
#endif // __HAVE_DOXYGEN__

///
/// Enable the ADC peripheral.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t adc_on(void);

///
/// Disable the ADC peripheral.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t adc_off(void);

///
/// Check if the ADC is enabled.
///
/// @retval true if turned on.
/// @retval false if turned off.
bool adc_is_on(void);

///
/// Measure the ADC voltage reference.
///
/// @attention
/// Depending on the platform and configuration this may always return
/// @c REGULATED_VOLTAGE_mV
///
/// @returns The ADC reference voltage in millivolts on success or @c 0
///  on failure.
uint_fast16_t adc_read_vref_mV(void);

///
/// Read the value on an analog pin
///
/// @param pin The pin to examine.
///
/// @returns The level relative to @c ADC_MAX on the pin on success or
///  @c ERR_ADC on failure.
adc_t adc_read_pin(gpio_pin_t pin);

///
/// Try to find the amplitude of an AC voltage.
///
/// @attention
/// This function does not respect @c ADC_SAMPLE_COUNT.
///
/// @param pin The pin to examine.
/// @param period_ms The time to spend monitoring in milliseconds.
/// @param min If non-NULL, set to the lowest ADC reading encountered.
/// @param max If non-NULL, set to the highest ADC reading encountered.
///
/// @returns Half the difference between the high and low peaks, or ERR_ADC if
///  there's an error.
adc_t adc_read_ac_amplitude(gpio_pin_t pin, uint_fast32_t period_ms, adc_t *min, adc_t *max);
