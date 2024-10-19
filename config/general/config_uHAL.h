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
// config_uHAL.h
// uHAL configuration file
// NOTES:
// 2024.10.16: This file was copied from uHAL commit f1ce4f6c099fb7e9cb3f032dd27f2a26556c1f24
//             and modified for use with GHMon.
//
#ifndef _uHAL_CONFIG_H
#define _uHAL_CONFIG_H

//
// Include the GHMon configuration before anything else, it may control some
// features.
#include "config/general/config.h"

//
// The program name and version.
// These aren't mandatory and are only used when printing the system information
// to the serial terminal. They must be strings if defined.
//#define PROGNAME
//#define PROGVERS

//
// If non-zero, subsystems are enabled by default
#define uHAL_USE_SUBSYSTEM_DEFAULT 0
//
// If non-zero, device drivers are built by default
#define uHAL_USE_DRIVER_DEFAULT 0
//
// If non-zero, display drivers are built by default
// Overrides uHAL_USE_DRIVER_DEFAULT
#define uHAL_USE_DISPLAY_DRIVER_DEFAULT uHAL_USE_DRIVER_DEFAULT

//
// Code size/performance options
//
// Cut down the size of internal components at the expense of functionality
#define uHAL_USE_SMALL_CODE USE_SMALL_CODE
//
// Skip checks for invalid arguments passed to functions
#define uHAL_SKIP_INVALID_ARG_CHECKS SKIP_LIB_SAFETY_CHECKS
//
// Skip checks for un-initialized data passed to functions
#define uHAL_SKIP_INIT_CHECKS SKIP_LIB_SAFETY_CHECKS
//
// Skip other validity or safety checks
#define uHAL_SKIP_OTHER_CHECKS SKIP_LIB_SAFETY_CHECKS

//
// System sleep options
//
// Enable system sleep modes
// This controls sleep_ms() and hibernate_s(), but hibernate() is unaffected
#define uHAL_USE_HIBERNATE 1
//
// Don't allow sleeping more deeply than this level
// Possible values are HIBERNATE_LIGHT, HIBERNATE_DEEP, and HIBERNATE_MAX
// Ignore if 0
//#define uHAL_HIBERNATE_LIMIT 0
//
// When hibernating, print the expected duration and mode of sleep to the
// UART terminal
#define uHAL_ANNOUNCE_HIBERNATE 1

//
// ADC configuration options
//
// Enable the Analog-to-Digital Converter
#define uHAL_USE_ADC 0
//
// Number of samples to take and then average for every ADC reading
#define ADC_SAMPLE_COUNT 1U
//
// The minimum number of microseconds to spend on each ADC sample
// If 0, use the smallest allowable period
#define ADC_SAMPLE_uS 0U
//
// If a conversion takes longer than this many milliseconds, abort because
// *something* is wrong
// This is the time for all samples to be taken
// Ignored if 0
#define ADC_TIMEOUT_MS 100U

//
// UART configuration options
//
// Enable UART peripherals
//#define uHAL_USE_UART uHAL_USE_SUBSYSTEM_DEFAULT
//
// If non-zero, enable listening on UART interfaces
// Disabling listening frees some space by not defining the relevant IRQs
// and support functions
//#define ENABLE_UART_LISTENING 1
//
// The number of buffered input bytes per interface
// Any bytes received on the interface while uart_listen_on() has been called
// will be discarded if this is 0
#define UART_INPUT_BUFFER_BYTES 0U
//
// Enable output to a UART serial console
//#define uHAL_USE_UART_COMM uHAL_USE_SUBSYSTEM_DEFAULT
//
// Timeout in ms for the serial console; this is the timeout for a whole
// message
#define UART_COMM_TIMEOUT_MS 10000UL // 10s
//
// Size of the serial console output buffer
// Set to 0 to disable
#define UART_COMM_BUFFER_BYTES 16U
//
// Size of the logger() history buffer
// Set to 0 to disable
#define LOGGER_HISTORY_BUFFER_BYTES 0U
//
// The serial console baud rate
#define UART_COMM_BAUDRATE 9600UL

//
// Terminal configuraiton options
//
// Enable the serial terminal (requires UART)
//#define uHAL_USE_TERMINAL uHAL_USE_SUBSYSTEM_DEFAULT
//
// If non-zero, the global symbols 'terminal_extra_cmds[]' and 'terminal_extra_help[]'
// declared in interface.h are used to define additional terminal commands
// and must be defined *somewhere*
#define TERMINAL_HAVE_EXTRA_CMDS 1
//
// Enable SD card formatting from the terminal
#define uHAL_USE_FDISK 0
//
// Timeout in seconds for the command terminal (that is, when it gives up
// listening)
#define TERMINAL_TIMEROUT_S 600U // 10 minutes
//
// Size of the input buffer
#define TERMINAL_BUFFER_BYTES 64U
//
// Maximum size of a terminal command name excluding final NUL byte
#define TERMINAL_CMD_NAME_LEN 11U


//
// SPI configuration options
//
// Enable SPI peripherals
#define uHAL_USE_SPI uHAL_USE_SUBSYSTEM_DEFAULT
//
// Target speed of the SPI bus; it will generally be somewhat higher due to
// hardware limitations
#define SPI_FREQUENCY_HZ 100000UL

//
// I2C configuration options
//
// Enable I2C peripherals
#define uHAL_USE_I2C uHAL_USE_SUBSYSTEM_DEFAULT
//
// Target speed of the I2C bus
#define I2C_FREQUENCY_HZ 50000UL

//
// PWM configuration options
//
// Enable PWM output
#define uHAL_USE_PWM uHAL_USE_SUBSYSTEM_DEFAULT
//
// Target frequency for PWM output
#define PWM_FREQUENCY_HZ 1000UL
//
// PWM duty cycle is defined as a number from 0 to PWM_DUTY_CYCLE_SCALE
// The practical effect of increasing the scale is hardware-dependent and may
// reduce the maximum possible frequency, have a limited effect on the actual
// resolution of a duty-cycle setting, increase code size, and/or slow down
// response time
#define PWM_DUTY_CYCLE_SCALE 100U

//
// FAT file system configuration
// This requires the FatFS library (http://elm-chan.org/fsw/ff/)
//
// Enable FatFS support
#define uHAL_USE_FATFS uHAL_USE_SUBSYSTEM_DEFAULT
//
// The path to the FatFS header 'ff.h'
#define FATFS_FF_H_PATH "FatFS/ff.h"
//
// The path to the FatFS header 'diskio.h'
#define FATFS_DISKIO_H_PATH "FatFS/diskio.h"
//
// Enable FAT on SD cards
// Only a generic SPI access is implemented
// SPI_CS_SD_PIN must be defined, it's used as the CS pin for the SD card
#define uHAL_USE_FATFS_SD uHAL_USE_FATFS

//
// Real-time clock configuration
//
// Enable the real-time clock
#define uHAL_USE_RTC 1

//
// Micro-second counter configuration
//
// Enable the micro-second counter
#define uHAL_USE_USCOUNTER uHAL_USE_SUBSYSTEM_DEFAULT

//
// GPIO configuration
//
// Enable the high-level GPIO functions (those that use the GPIO_CTRL bits)
#define uHAL_USE_HIGH_LEVEL_GPIO 1
//
// Enable an experimental alternative high-level GPIO interface
// This can co-exist with the old interface.
#define uHAL_USE_EXPERIMENTAL_GPIO_INTERFACE uHAL_USE_SUBSYSTEM_DEFAULT
//
// If non-zero, setting a GPIO configured as an output pin to GPIO_FLOAT will
// toggle it instead of doing nothing.
#define uHAL_TOGGLE_GPIO_OUTPUT_WITH_FLOAT 0

//
// Device drivers
//
// SSD1306 OLED display
#define uHAL_USE_DISPLAY_SSD1306 uHAL_USE_DISPLAY_DRIVER_DEFAULT
//
// How many caller-defined commands to send to the device during initialization
// These are defined in the configuration structure
#define SSD1306_INIT_COMMANDS_COUNT 0
//
// Support font scaling by way of the scale_x and scale_y members of ssd1306_font_t
#define SSD1306_FONT_AUTOSCALE 1
//
// Width of fonts
// If <= 0, use a per-font value set in ssd1306_font_t
#define SSD1306_FONT_WIDTH 8U
//
// If set, include a basic ASCII font to be used when the passed font is NULL
#define SSD1306_INCLUDE_DEFAULT_FONT 1


#endif // _uHAL_CONFIG_H
