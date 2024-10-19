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
// config_AVR_XMEGA3.h
// uHAL configuration file
// NOTES:
// 2024.10.16: This file was copied from uHAL commit f1ce4f6c099fb7e9cb3f032dd27f2a26556c1f24
//             and modified for use with GHMon.
//
#ifndef _uHAL_CONFIG_AVR_XMEGA3_H
#define _uHAL_CONFIG_AVR_XMEGA3_H

#if defined(__AVR_ATtiny402__)
# include GHMON_INCLUDE_CONFIG_HEADER(pindefs_attiny402.h)
# define uHAL_USE_INTERNAL_OSC 1
# define uHAL_USE_INTERNAL_LS_OSC 1
#endif
#define uHAL_HIBERNATE_LIMIT 0


// Use the less-accurate but lower-power internal oscillator for the system
// instead of the external crystal
//#define uHAL_USE_INTERNAL_OSC 1
//
// This is the desired frequency of the main system clock
// Normally F_CPU will be set when invoking the compiler
#define F_CORE F_CPU
//
// This is the frequency of the main oscillator
// It must be defined either here or on the command line
// The internal oscillator may be either 16MHz or 20MHz depending on how
// the fuses are set and an external oscillator can be almost anything so
// no guess is made as to it's value
//#define F_OSC 16000000UL
//#define F_OSC 20000000UL

// Use the internal low-speed oscillator rather than an external crystal
//#define uHAL_USE_INTERNAL_LS_OSC 1
//
// This is the frequency of the low-speed clock
#define F_LS_OSC 32768UL

// This is the voltage of the internal voltage-reference
// Each device has several internal reference sources and one is selected
// for use based on this value
#define INTERNAL_VREF_mV 1100U

// This is the regulated voltage applied to the MCUs power pin
#ifndef REGULATED_VOLTAGE_mV
# define REGULATED_VOLTAGE_mV 3300U
#endif

//
// The preferred frequency of the ADC clock
// Under normal conditions the frequency needs to be between 50KHz and 1.5MHz
// to get the maximum resolution, but with the internal 0.55V reference the
// maximum is 260KHz and with 8 bit values it's 2MHz
// If undefined it's set as high as possible without exceeding the device's
// maximum value
// This is *NOT* the sampling rate, just the base clock
//#define F_ADC
//
// The maximum value returned by the ADC
// On some devices this can be set lower to reliably use higher ADC clock
// frequencies
#define ADC_MAX 0x3FF

// The timer used to track millisecond system ticks
// Options are TIMER_RTT, TIMER_TCA0, TIMER_TCA0_HIGH, TIMER_TCA0_LOW, and
// (depending on hardware) one or more of TIMER_TCB0 - TIMER_TCB3
//
// When using the RTT, each tick will (nominally) be 1.024ms instead of 1ms
//
// When using TCA0, the PWM output will be limited to WO0 - WO2 (see the manual
// for which pins those correspond to)
//
// When using any TCA timer, the PWM outputs will have a more limited frequency
// and range selection
//
// When using any TCB timer, the PWM outputs associated with that timer will
// be disabled
#define SYSTICK_TIMER TIMER_RTT
//
// The number of system ticks per second, used by the RTC emulation code
// When undefined this will be set according to the system tick source
// The only reason to set this is if you've measured the frequency of the timer
// source and can provide a more accurate value than the default
//#define SYSTICKS_PER_S

// Force the use of split (8-bit) TCA0
// This will allow for more possible PWM outputs
// This is disabled when SYSTICK_TIMER or USCOUNTER_TIMER are TIMER_TCA0
//#define USE_SPLIT_TCA0 1

// Keep timers enabled when in standby sleep mode
// This is mostly just useful for when PWM outputs are expected to run even
// during deep sleep
// Depending on device this may or may not work for a particular timer - specifically
// TCA doesn't always support it
#define USE_STDBY_TCB 1
#define USE_STDBY_TCA 1

// The timer used to count micro-second periods
// The options are the same as for SYSTICK_TIMER (except for TIMER_RTT) and have
// the same limitations with regard to PWM outputs which may additionally
// experience overshoots while using the counter
// This can auto-selected by setting it to '0' or 'TIMER_NONE'
// This is disabled if uHAL_USE_USCOUNTER is '0'
#define USCOUNTER_TIMER 0

// The Real-Time Timer used to wake from sleep uses a not-very-accurate
// internal clock and can calibrate itself against the main system clock
// periodically after this many seconds
// Set to '0' to disable
#define RTT_RECALIBRATE_INTERVAL_S (60U * 30U) // Every 30 Minutes
//
// The RTT calibration period is this many RTT clock cycles
#define RTT_CALIBRATE_CYCLES (512UL)
//
// Use the high-speed external oscillator as the Real-Time Counter source instead
// of the low-speed oscillator
// This is ignored if the high-speed oscillator is not external
// This is entirely untested
#define RTT_USE_EXTCLK_SRC 0

// The scale to use internally for PWM duty cycles
// Normally this is automatically calculated based on PWM_DUTY_CYCLE_SCALE
//#define TCA0_DUTY_CYCLE_SCALE
//#define TCB_DUTY_CYCLE_SCALE
//
// The algorithm used to change a value given in reference to PWM_DUTY_CYCLE_SCALE
// into a value referenced to TCx_DUTY_CYCLE_SCALE
// Normally this is automatically calculated based on TCx_DUTY_CYCLE_SCALE
//#define TCA0_DUTY_CYCLE_ADJUST(_dc_) (_dc_)
//#define TCB_DUTY_CYCLE_ADJUST(_dc_) (_dc_)

// If non-zero, use RTC emulation code
// There's no other RTC option for this platform
#define uHAL_USE_RTC_EMULATION uHAL_USE_RTC


#endif //_uHAL_CONFIG_AVR_XMEGA3_H
