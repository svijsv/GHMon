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
//

//
// The program name and version.
// These aren't mandatory and are only used when printing the system information
// to the serial terminal. They must be strings if defined.
//#define PROGNAME
//#define PROGVERS

//
// If non-zero, subsystems are enabled by default
#ifndef uHAL_USE_SUBSYSTEM_DEFAULT
# define uHAL_USE_SUBSYSTEM_DEFAULT 0
#endif
//
// If non-zero, device drivers are built by default
#ifndef uHAL_USE_DRIVER_DEFAULT
# define uHAL_USE_DRIVER_DEFAULT 0
#endif
//
// If non-zero, display drivers are built by default
// Overrides uHAL_USE_DRIVER_DEFAULT
#ifndef uHAL_USE_DISPLAY_DRIVER_DEFAULT
# define uHAL_USE_DISPLAY_DRIVER_DEFAULT uHAL_USE_DRIVER_DEFAULT
#endif

//
// Code size/performance options
//
// Cut down the size of internal components at the expense of functionality
// If 1, use smaller structures for data in RAM
// If 2, also use smaller structures for data in ROM
#ifndef uHAL_USE_SMALL_CODE
# define uHAL_USE_SMALL_CODE 0
#endif
//
// Skip checks for invalid arguments passed to functions
#ifndef uHAL_SKIP_INVALID_ARG_CHECKS
# define uHAL_SKIP_INVALID_ARG_CHECKS 0
#endif
//
// Skip checks for un-initialized data passed to functions
#ifndef uHAL_SKIP_INIT_CHECKS
# define uHAL_SKIP_INIT_CHECKS 0
#endif
//
// Skip other validity or safety checks
#ifndef uHAL_SKIP_OTHER_CHECKS
# define uHAL_SKIP_OTHER_CHECKS 0
#endif

//
// System sleep options
//
// Enable system sleep modes
// This controls sleep_ms() and hibernate_s(), but hibernate() is unaffected
#ifndef uHAL_USE_HIBERNATE
# define uHAL_USE_HIBERNATE uHAL_USE_SUBSYSTEM_DEFAULT
#endif
//
// Don't allow sleeping more deeply than this level
// Possible values are HIBERNATE_LIGHT, HIBERNATE_DEEP, and HIBERNATE_MAX
// Ignore if 0
#ifndef uHAL_HIBERNATE_LIMIT
# define uHAL_HIBERNATE_LIMIT 0
#endif
//
// When hibernating, print the expected duration and mode of sleep to the
// UART terminal
#ifndef uHAL_ANNOUNCE_HIBERNATE
# define uHAL_ANNOUNCE_HIBERNATE 1
#endif

//
// ADC configuration options
//
// Enable the Analog-to-Digital Converter
#ifndef uHAL_USE_ADC
# define uHAL_USE_ADC uHAL_USE_SUBSYSTEM_DEFAULT
#endif
//
// Number of samples to take and then average for every ADC reading
#ifndef ADC_SAMPLE_COUNT
# define ADC_SAMPLE_COUNT 1U
#endif
//
// The minimum number of microseconds to spend on each ADC sample
// If 0, use the smallest allowable period
#ifndef ADC_SAMPLE_uS
# define ADC_SAMPLE_uS 0U
#endif
//
// If a conversion takes longer than this many milliseconds, abort because
// *something* is wrong
// This is the time for all samples to be taken
// Ignored if 0
#ifndef ADC_TIMEOUT_MS
# define ADC_TIMEOUT_MS 100U
#endif

//
// UART configuration options
//
// Enable UART peripherals
#ifndef uHAL_USE_UART
# define uHAL_USE_UART uHAL_USE_SUBSYSTEM_DEFAULT
#endif
//
// If non-zero, enable listening on UART interfaces
// Disabling listening frees some space by not defining the relevant IRQs
// and support functions
#ifndef ENABLE_UART_LISTENING
# define ENABLE_UART_LISTENING 1
#endif
//
// The number of buffered input bytes per interface
// Any bytes received on the interface while uart_listen_on() has been called
// will be discarded if this is 0
#ifndef UART_INPUT_BUFFER_BYTES
# define UART_INPUT_BUFFER_BYTES 1U
#endif
//
// Enable output to a UART serial console
#ifndef uHAL_USE_UART_COMM
# define uHAL_USE_UART_COMM uHAL_USE_SUBSYSTEM_DEFAULT
#endif
//
// Timeout in ms for the serial console; this is the timeout for a whole
// message
#ifndef UART_COMM_TIMEOUT_MS
# define UART_COMM_TIMEOUT_MS 10000UL // 10s
#endif
//
// Size of the serial console output buffer
// Set to 0 to disable
#ifndef UART_COMM_BUFFER_BYTES
# define UART_COMM_BUFFER_BYTES 64U
#endif
//
// Size of the logger() history buffer
// Set to 0 to disable
#ifndef LOGGER_HISTORY_BUFFER_BYTES
# define LOGGER_HISTORY_BUFFER_BYTES 0U
#endif
//
// The serial console baud rate
#ifndef UART_COMM_BAUDRATE
# define UART_COMM_BAUDRATE 9600UL
#endif

//
// Terminal configuraiton options
//
// Enable the serial terminal (requires UART)
#ifndef uHAL_USE_TERMINAL
# define uHAL_USE_TERMINAL uHAL_USE_SUBSYSTEM_DEFAULT
#endif
//
// If non-zero, the global symbols 'terminal_extra_cmds[]' and 'terminal_extra_help[]'
// declared in interface.h are used to define additional terminal commands
// and must be defined *somewhere*
#ifndef TERMINAL_HAVE_EXTRA_CMDS
# define TERMINAL_HAVE_EXTRA_CMDS 0
#endif
//
// Enable SD card formatting from the terminal
#ifndef uHAL_USE_FDISK
# define uHAL_USE_FDISK 0
#endif
//
// Timeout in seconds for the command terminal (that is, when it gives up
// listening)
#ifndef TERMINAL_TIMEROUT_S
# define TERMINAL_TIMEROUT_S 3600U // 1 Hour
#endif
//
// Size of the input buffer
#ifndef TERMINAL_BUFFER_BYTES
# define TERMINAL_BUFFER_BYTES 64U
#endif
//
// Maximum size of a terminal command name excluding final NUL byte
#ifndef TERMINAL_CMD_NAME_LEN
# define TERMINAL_CMD_NAME_LEN 11U
#endif


//
// SPI configuration options
//
// Enable SPI peripherals
#ifndef uHAL_USE_SPI
# define uHAL_USE_SPI uHAL_USE_SUBSYSTEM_DEFAULT
#endif
//
// Target speed of the SPI bus; it will generally be somewhat higher due to
// hardware limitations
#ifndef SPI_FREQUENCY_HZ
# define SPI_FREQUENCY_HZ 100000UL
#endif

//
// I2C configuration options
//
// Enable I2C peripherals
#ifndef uHAL_USE_I2C
# define uHAL_USE_I2C uHAL_USE_SUBSYSTEM_DEFAULT
#endif
//
// Target speed of the I2C bus
#ifndef I2C_FREQUENCY_HZ
# define I2C_FREQUENCY_HZ 50000UL
#endif

//
// PWM configuration options
//
// Enable PWM output
#ifndef uHAL_USE_PWM
# define uHAL_USE_PWM uHAL_USE_SUBSYSTEM_DEFAULT
#endif
//
// Target frequency for PWM output
#ifndef PWM_FREQUENCY_HZ
# define PWM_FREQUENCY_HZ 1000UL
#endif
//
// PWM duty cycle is defined as a number from 0 to PWM_DUTY_CYCLE_SCALE
// The practical effect of increasing the scale is hardware-dependent and may
// reduce the maximum possible frequency, have a limited effect on the actual
// resolution of a duty-cycle setting, increase code size, and/or slow down
// response time
#ifndef PWM_DUTY_CYCLE_SCALE
# define PWM_DUTY_CYCLE_SCALE 100U
#endif

//
// FAT file system configuration
// This requires the FatFS library (http://elm-chan.org/fsw/ff/)
//
// Enable FatFS support
#ifndef uHAL_USE_FATFS
# define uHAL_USE_FATFS uHAL_USE_SUBSYSTEM_DEFAULT
#endif
//
// The path to the FatFS header 'ff.h'
#ifndef FATFS_FF_H_PATH
# define FATFS_FF_H_PATH "FatFS/ff.h"
#endif
//
// The path to the FatFS header 'diskio.h'
#ifndef FATFS_DISKIO_H_PATH
# define FATFS_DISKIO_H_PATH "FatFS/diskio.h"
#endif
//
// Enable FAT on SD cards
// Only a generic SPI access is implemented
// SPI_CS_SD_PIN must be defined, it's used as the CS pin for the SD card
#ifndef uHAL_USE_FATFS_SD
# define uHAL_USE_FATFS_SD uHAL_USE_FATFS
#endif

//
// Real-time clock configuration
//
// Enable the real-time clock
#ifndef uHAL_USE_RTC
# define uHAL_USE_RTC uHAL_USE_SUBSYSTEM_DEFAULT
#endif

//
// Micro-second counter configuration
//
// Enable the micro-second counter
#ifndef uHAL_USE_USCOUNTER
# define uHAL_USE_USCOUNTER uHAL_USE_SUBSYSTEM_DEFAULT
#endif

//
// GPIO configuration
//
// Enable the high-level GPIO functions (those that use the GPIO_CTRL bits)
#ifndef uHAL_USE_HIGH_LEVEL_GPIO
# define uHAL_USE_HIGH_LEVEL_GPIO uHAL_USE_SUBSYSTEM_DEFAULT
#endif
//
// Enable an experimental alternative high-level GPIO interface
// This can co-exist with the old interface.
#ifndef uHAL_USE_EXPERIMENTAL_GPIO_INTERFACE
# define uHAL_USE_EXPERIMENTAL_GPIO_INTERFACE uHAL_USE_SUBSYSTEM_DEFAULT
#endif
//
// If non-zero, setting a GPIO configured as an output pin to GPIO_FLOAT will
// toggle it instead of doing nothing.
#ifndef uHAL_TOGGLE_GPIO_OUTPUT_WITH_FLOAT
# define uHAL_TOGGLE_GPIO_OUTPUT_WITH_FLOAT 0
#endif

//
// Device drivers
//
// SSD1306 OLED display
#ifndef uHAL_USE_DISPLAY_SSD1306
# define uHAL_USE_DISPLAY_SSD1306 uHAL_USE_DISPLAY_DRIVER_DEFAULT
#endif
//
// How many caller-defined commands to send to the device during initialization
// These are defined in the configuration structure
#ifndef SSD1306_INIT_COMMANDS_COUNT
# define SSD1306_INIT_COMMANDS_COUNT 0
#endif
//
// Support font scaling by way of the scale_x and scale_y members of ssd1306_font_t
#ifndef SSD1306_FONT_AUTOSCALE
# define SSD1306_FONT_AUTOSCALE 1
#endif
//
// Width of fonts
// If <= 0, use a per-font value set in ssd1306_font_t
#ifndef SSD1306_FONT_WIDTH
# define SSD1306_FONT_WIDTH 8U
#endif
//
// If set, include a basic ASCII font to be used when the passed font is NULL
#ifndef SSD1306_INCLUDE_DEFAULT_FONT
# define SSD1306_INCLUDE_DEFAULT_FONT 1
#endif
