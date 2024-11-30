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
//
#ifndef _uHAL_CONFIG_AVR_XMEGA3_H
#define _uHAL_CONFIG_AVR_XMEGA3_H


// Use the less-accurate but lower-power internal oscillator for the system
// instead of the external crystal
#ifndef uHAL_USE_INTERNAL_OSC
# define uHAL_USE_INTERNAL_OSC 1
#endif
//
// This is the desired frequency of the main system clock
// Normally F_CPU will be set when invoking the compiler
#ifndef F_CORE
# define F_CORE F_CPU
#endif
//
// This is the frequency of the main oscillator
// It must be defined either here or on the command line
// The internal oscillator may be either 16MHz or 20MHz depending on how
// the fuses are set and an external oscillator can be almost anything so
// no guess is made as to it's value
//#define F_OSC 16000000UL
//#define F_OSC 20000000UL

// Use the internal low-speed oscillator rather than an external crystal
#ifndef uHAL_USE_INTERNAL_LS_OSC
# define uHAL_USE_INTERNAL_LS_OSC 1
#endif
//
// This is the frequency of the low-speed clock
#ifndef F_LS_OSC
# define F_LS_OSC 32768UL
#endif

// This is the voltage of the internal voltage-reference
// Each device has several internal reference sources and one is selected
// for use based on this value
#ifndef INTERNAL_VREF_mV
# define INTERNAL_VREF_mV 1100U
#endif

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
#ifndef ADC_MAX
# define ADC_MAX 0x3FF
#endif

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
#ifndef SYSTICK_TIMER
# define SYSTICK_TIMER TIMER_RTT
#endif
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
#ifndef USE_STDBY_TCB
# define USE_STDBY_TCB 0
#endif
#ifndef USE_STDBY_TCA
# define USE_STDBY_TCA 0
#endif

// The timer used to count micro-second periods
// The options are the same as for SYSTICK_TIMER (except for TIMER_RTT) and have
// the same limitations with regard to PWM outputs which may additionally
// experience overshoots while using the counter
// This can auto-selected by setting it to '0' or 'TIMER_NONE'
// This is disabled if uHAL_USE_USCOUNTER is '0'
#ifndef USCOUNTER_TIMER
# define USCOUNTER_TIMER 0
#endif

// The Real-Time Timer used to wake from sleep uses a not-very-accurate
// internal clock and can calibrate itself against the main system clock
// periodically after this many seconds
// Set to '0' to disable
#ifndef RTT_RECALIBRATE_INTERVAL_S
# define RTT_RECALIBRATE_INTERVAL_S (60U * 30U) // Every 30 Minutes
#endif
//
// The RTT calibration period is this many RTT clock cycles
#ifndef RTT_CALIBRATE_CYCLES
# define RTT_CALIBRATE_CYCLES (512UL)
#endif
//
// Use the high-speed external oscillator as the Real-Time Counter source instead
// of the low-speed oscillator
// This is ignored if the high-speed oscillator is not external
// This is entirely untested
#ifndef RTT_USE_EXTCLK_SRC
# define RTT_USE_EXTCLK_SRC 0
#endif

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
#ifndef uHAL_USE_RTC_EMULATION
# define uHAL_USE_RTC_EMULATION uHAL_USE_RTC
#endif


//
// Pin configuration
//
// The full list of defined pins is in each platform's platform.h file or
// (usually) a file included by platform.h.
//
// Identifiers beginning with 'PINID_' refer to the internal pin/port combinations.
// Those beginning with just 'PIN_' (which may not be present) refer to labels
// on common development boards using the associated MCU or to package pinouts.
//
// Pins defined for a peripheral (e.g. I2C or SPI) are only actually used if
// the peripheral is used.
//
// There are internal pullups but no internal pulldowns on AVR devices
//
// Some peripheral pins can be remapped. In order to use the remapped pins,
// set the corresponding flag. Check the reference manual for pin mappings.
#define GPIO_REMAP_I2C0   0
#define GPIO_REMAP_SPI0   0
#define GPIO_REMAP_UART0  0
#define GPIO_REMAP_TCA0W0 0
#define GPIO_REMAP_TCA0W1 0
#define GPIO_REMAP_TCA0W2 0
#define GPIO_REMAP_TCA0W3 0
#define GPIO_REMAP_TCB0   0
//
// If set to non-zero, any GPIO port which the device header files expose
// will be enabled. When zero, any port used needs to be explicitly enabled
// by setting HAVE_GPIO_PORTx to non-zero either on the command line or in
// this configuration file
#define HAVE_GPIO_PORT_DEFAULT 1
//#define HAVE_GPIO_PORTA 1
//
// UART serial console pins
#define UART_COMM_TX_PIN PINID_UART0_TX
#define UART_COMM_RX_PIN PINID_UART0_RX
//
// SPI pins
#define SPI_SS_PIN    PINID_SPI0_SS
#define SPI_SCK_PIN   PINID_SPI0_SCK
#define SPI_MISO_PIN  PINID_SPI0_MISO
#define SPI_MOSI_PIN  PINID_SPI0_MOSI
#define SPI_CS_SD_PIN SPI_SS_PIN
//
// I2C pins
#define I2C_SDA_PIN PINID_I2C0_SDA
#define I2C_SCL_PIN PINID_I2C0_SCL

#endif //_uHAL_CONFIG_AVR_XMEGA3_H
