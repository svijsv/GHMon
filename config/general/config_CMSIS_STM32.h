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
// config_CMSIS.h
// uHAL configuration file
// NOTES:
// 2024.10.16: This file was copied from uHAL commit f1ce4f6c099fb7e9cb3f032dd27f2a26556c1f24
//             and modified for use with GHMon.
//
#ifndef _uHAL_CONFIG_CMSIS_H
#define _uHAL_CONFIG_CMSIS_H

#include GHMON_INCLUDE_CONFIG_HEADER(lib/config_CMSIS_STM32.h)

#if defined(STM32F103x6) || defined(STM32F103xB) || defined(STM32F103xE) || defined(STM32F103xG)
# include GHMON_INCLUDE_CONFIG_HEADER(pindefs/stm32f103.h)
#elif defined(STM32F401xC) || defined(STM32F401xE)
# include GHMON_INCLUDE_CONFIG_HEADER(pindefs/stm32f401.h)
#endif
#if USE_UART_TERMINAL
  // The UART clock is disabled in deep sleep but we need to be able to wake up
# define uHAL_HIBERNATE_LIMIT HIBERNATE_LIGHT
#else
# define uHAL_HIBERNATE_LIMIT 0
#endif


// If non-zero, the SWD pins are enabled when DEBUG is non-zero
#define uHAL_SWD_DEBUG 1
// If non-zero, the JTAG pins are enabled when DEBUG is non-zero
// Takes precedence over uHAL_SWD_DEBUG
#define uHAL_JTAG_DEBUG 0

// If non-zero, reset the backup domain on system initialization
#define uHAL_BACKUP_DOMAIN_RESET 0

// Use the less-accurate but lower-power internal oscillator for the system
// clock where supported instead of an external crystal
#define uHAL_USE_INTERNAL_OSC 1
//
// Adjust the internal oscillator trim by this value
// This corresponds to the HSITRIM value in the RCC_CR register
// If 0, leave it at the default which varies between device lines
#define uHAL_HSI_TRIM 0
//
// Use the less-accurate but lower-power internal oscillator for the low-
// speed clock where supported instead of an external crystal
#define uHAL_USE_INTERNAL_LS_OSC 1
//
// This is the desired frequency of the main system clock
// Normally F_CPU will be set when invoking the compiler
#define F_CORE F_CPU
//
// This is the desired frequency of the AHB peripheral clock
#define F_HCLK (F_CORE)
//
// This is the desired frequency of the PCLK1 peripheral clock
// It's automatically determined if 0 or undefined
#define F_PCLK1 0
//
// This is the desired frequency of the PCLK2 peripheral clock
// It's automatically determined if 0 or undefined
#define F_PCLK2 0
//
// This is the frequency of the main oscillator
// If uHAL_USE_INTERNAL_OSC is not set, this must be defined either here
// or on the command line
// If uHAL_USE_INTERNAL_OSC is set, this overrides the device-specific default
//#define F_OSC
//
// This is the frequency of the low-speed oscillator
// If both this and uHAL_USE_INTERNAL_LS_OSC are set, this overrides the
// device-specific default
//#define F_LS_OSC 32768UL

// The offset of the RCC bus designation bits in the type used to identify
// peripheral clock buses
// This is here because the highest two bits in the associated registers
// are normally unused and so the whole structure can fit in a single 32-bit
// integer if those (probably non-existent) peripherals are left out, but
// someone may want them in for some reason at some point
// If >30, 64-bit values are used to track the peripheral
//#define RCC_BUS_OFFSET 30U

// This is the voltage of the internal voltage-reference
// The default value is device-dependent
//#define INTERNAL_VREF_mV

// This is the regulated voltage applied to the MCUs power pin
#define REGULATED_VOLTAGE_mV 3300

// The speed of the GPIO bus
// Options are OUTPUT_{SLOW, MEDIUM, FAST, VERY_FAST}
#define uHAL_GPIO_SPEED OUTPUT_MEDIUM

// The preferred frequency of the ADC clock
// If 0 it's set as high as possible without exceeding the device's maximum
// value
// This is *NOT* the sampling rate, just the base clock
#define F_ADC 0UL
//
// The maximum value returned by the ADC
// On some devices this can be set lower to decrease conversion time
#define ADC_MAX 0x0FFF

// The timer used to count micro-second periods
// The available timers vary by device, but anything from 1 to 14 should
// work if available
// This can auto-selected by setting it to '0' or 'TIMER_NONE'
// This is disabled if uHAL_USE_USCOUNTER is '0'
#define USCOUNTER_TIMER 0
//
// The timer used to wake from sleep
// This can be the same as the microsecond timer, but they can't both be
// used at the same time
// This can be auto-selected by setting it to '0' or 'TIMER_NONE'
#define SLEEP_ALARM_TIMER 0


#endif //_uHAL_CONFIG_CMSIS_H
