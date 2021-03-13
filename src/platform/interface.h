// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021 svijsv                                                *
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
// interface.h
// Interface between the frontend and backend code
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _INTERFACE_H
#define _INTERFACE_H

#define SYSTICKS G_sys_uticks // Used in util.h
#define RTCTICKS NOW()        // Used in util.h
/*
* Includes
*/
#include "ulib/types.h"
#include "ulib/util.h"

#include "config.h"


/*
* Static values
*/
//
// Delay periods in milliseconds
#define DELAY_ERR   100 // Length of error flash
#define DELAY_LONG  500
#define DELAY_SHORT 250
#define DELAY_BUSY  1
#define DELAY_ACK   100 // Length of aknowledgement flash

//
// IRQ received flags
#define BUTTON_IRQf 0x01
#define UART_IRQf   0x02

//
// Configuration flags
#define CFG_NONE             0x00
#define CFG_ALLOW_INTERRUPTS 0x01


/*
* Types
*/
// Define the state of a GPIO pin
typedef enum {
	GPIO_LOW  = 0,
	GPIO_HIGH = 1,
	GPIO_FLOAT
} gpio_state_t;

// Define the operation mode of a GPIO pin
// Alternate function modes may be the same as normal modes depending on
// platform and wouldn't normally be used in the frontend anyway.
typedef enum {
	GPIO_MODE_RESET = 0, // Reset state of the pin
	GPIO_MODE_PP,    // Push-pull output
	GPIO_MODE_PP_AF, // Alternate-function push-pull output
	GPIO_MODE_OD,    // Open-drain output
	GPIO_MODE_OD_AF, // Alternate-function open-drain output
	GPIO_MODE_IN,    // Input
	GPIO_MODE_AIN,   // Analog input
	GPIO_MODE_HiZ    // High-impedence mode
} gpio_mode_t;

// An identifier for MCU pins
// Using an int is simpler than a struct because I don't need to define
// structures to use pins, can check them with the preprocessor, and individual
// backends have more flexibility choosing how to make use of the bits.
// '0' is reserved for unassigned pins.
typedef uint8_t pin_t;

// Type used to hold the value returned by an ADC
typedef uint16_t adc_t;
// Type used to perform mathematical functions with ADC readings which may
// overflow adc_t such as multiplication or those which may involve negative
// values
typedef int32_t adcm_t;


/*
* Variable declarations
*/
//
// Backend-provided
//
// System ticks, milliseconds
extern volatile utime_t G_sys_uticks;

//
// Frontend-provided
//
// IRQs that frontend needs to handle
extern volatile uint8_t G_IRQs;


/*
* Function prototypes
*/
//
// Backend-provided
//
//
// Platform core interface

// Initialize the platform hardware
void platform_init(void);

// Sleep (low-power mode) for ms milliseconds
void sleep(utime_t ms);
// Hibernate (lower-power mode) for s seconds
// If CFG_ALLOW_INTERRUPTS is set, exit when an interrupt is received
void hibernate(utime_t s, uint8_t flags);

// Print platform-specific information about the running system using printf_putc()
void print_platform_info(printf_putc_t printf_putc);

//
// GPIO interface

// Set the operating mode of a pin
// istate is either the bias of an input pin, the initial state of a normal
// output pin, or ignored
void gpio_set_mode(pin_t pin, gpio_mode_t mode, gpio_state_t istate);

// Get the operating mode of a pin
// Depending on the platform certain modes may never be returned because they're
// the same as other modes; see gpio.h.
gpio_mode_t gpio_get_mode(pin_t pin);

// Set the state of a pin
// For outputs this sets the pin high or low
// For non-floating inputs this sets the bias
// For other modes this shouldn't have an effect
void gpio_set_state(pin_t pin, gpio_state_t new_state);

// Toggle the state of a pin
// For outputs this toggles the pin high or low
// For non-floating inputs this toggles the bias
// For other modes this shouldn't have an effect
void gpio_toggle_state(pin_t pin);

// Get the state of a pin
// For inputs, get the value of the input data register
// For outputs, get the value of the output data register
gpio_state_t gpio_get_state(pin_t pin);

//
// UART interface
#if USE_SERIAL

// Turn the UART peripheral on or off
void uart_on(void);
void uart_off(void);

// Receive a block of data
err_t uart_receive_block(uint8_t *buffer, uint32_t size, utime_t timeout);

// Transmit a block of data
err_t uart_transmit_block(const uint8_t *buffer, uint32_t size, utime_t timeout);

#else // !USE_SERIAL
# define uart_on() ((void )0U)
# define uart_off() ((void )0U)
# define uart_receive_block(...) ((void )0U)
# define uart_transmit_block(...) ((void )0U)
#endif // USE_SERIAL

//
// SPI interface
#if USE_SPI

// Turn on and off the SPI peripheral
void spi_on(void);
void spi_off(void);

// Exchange a byte
err_t spi_exchange_byte(uint8_t tx, uint8_t *rx, utime_t timeout);

// Receive a data block
// tx is the value transmitted for each byte read and must be 0xFF when
// communicating with an SD card in order to keep MOSI high; other devices
// may have different (or no) requirements.
err_t spi_receive_block(uint8_t *rx_buffer, uint32_t rx_size, uint8_t tx, utime_t timeout);

// Transmit a data block
err_t spi_transmit_block(const uint8_t *tx_buffer, uint32_t tx_size, utime_t timeout);

#else // !USE_SPI
#define spi_on() ((void )0U)
#define spi_off() ((void )0U)
#define spi_exchange_byte(...) (EOK)
#define spi_receive_block(...) (EOK)
#define spi_transmit_block(...) (EOK)
#endif // USE_SPI

//
// Time-management interface

// Get the system time enumerated in seconds
utime_t get_RTC_seconds(void);

// Set system date and time
err_t set_time(uint8_t hour, uint8_t minute, uint8_t second);
err_t set_date(uint8_t year, uint8_t month, uint8_t day);

// Don't do anything for a bit
void delay(utime_t ms);
// A 'dumb' delay that doesn't know about ticks
// Don't expect this to be very accurate.
void dumb_delay(utime_t ms);
// A 'dumber' delay that doesn't even know about milliseconds
void dumber_delay(uint32_t cycles);

//
// ADC interface
#if USE_ADC
// Enable and disable the ADC peripheral
void adc_on(void);
void adc_off(void);

// Return the analog voltage reference value and the MCU's internal temperature
// sensor
// vref is measured in millivolts
// tempCx10 is measured in degrees celsius * 10
// Depending on the platform and configuration this may always return
// REGULATED_VOLTAGE and/or 0 respectively or may be very inaccurate; see
// adc.h.
// Both arguments are mandatory.
void adc_read_internals(int16_t *vref, int16_t *tempCx10);

// Read the value on an analog pin
// If the pin selected doesn't support analog reading, 0 is returned
adc_t adc_read_pin(pin_t pin, utime_t timeout);

#else // !USE_ADC
#define adc_on() ((void )0U)
#define adc_off() ((void )0U)
#define adc_read_internals(...) ((void )0U)
#define adc_read_pin(...) ((void )0U)
#endif // USE_ADC


//
// Frontend-provided
//
#if USE_SERIAL
// Print a debug message
void logger(const char *format, ...)
	__attribute__ ((format(printf, 1, 2)));
#else
# define logger(...) ((void )0U)
#endif

// Go to an endless error loop
void error_state_crude(void);
void error_state(const char *file_path, uint32_t lineno, const char *func_name, const char *msg);

// LED control functions
void led_on(void);
void led_off(void);
void led_toggle(void);
// Toggle the LED, then toggle it again; set to on/off first if you want to
// flash a certain way
void led_flash(uint8_t count, uint16_t ms);
// Flash a few times to get someone's attention
void issue_warning(void);


/*
* Macros
*/
#if (DEBUG || USE_SMALL_CODE < 2) && USE_SERIAL
#define ERROR_STATE(msg) error_state(__FILE__, __LINE__, __func__, msg)
#else
#define ERROR_STATE(msg) error_state_crude()
#endif

#if DEBUG
#define LOGGER(...)      logger(__VA_ARGS__)
#else
#define LOGGER(...)      ((void )0U)
#endif

// Get the system time enumerated in seconds
#define NOW() (get_RTC_seconds())

#endif // _INTERFACE_H
#ifdef __cplusplus
 }
#endif
