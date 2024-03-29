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
//    Backend-specific interface components and overrides are defined in
//    platform.h in the individual backend directories
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _PLATFORM_INTERFACE_H
#define _PLATFORM_INTERFACE_H

#define SYSTICKS NOW_MS() // Used in util.h
#define RTCTICKS NOW()    // Used in util.h
/*
* Includes
*/
#include "ulib/types.h"
#include "ulib/util.h"

#include "platform.h"
#include "config.h"

// Because platform.h is (and needs to be) included before config.h, anything
// that might be set in platform.h and overridden in config.h needs to be
// handled specially
#if ! INTERNAL_VREF_mV && _INTERNAL_VREF_mV
# define INTERNAL_VREF_mV _INTERNAL_VREF_mV
#endif
#if ! REGULATED_VOLTAGE_mV && _REGULATED_VOLTAGE_mV
# define REGULATED_VOLTAGE_mV _REGULATED_VOLTAGE_mV
#endif
#if ! REGULATED_VOLTAGE_LOW_mV && _REGULATED_VOLTAGE_LOW_mV
# define REGULATED_VOLTAGE_LOW_mV _REGULATED_VOLTAGE_LOW_mV
#endif

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
// Reserve 0xF0 for local uses
#define CFG_NONE             0x00
#define CFG_ALLOW_INTERRUPTS 0x01

//
// IO masks
// The port number mask for pin_t
// 2 bits, can't be 0 (3 possible ports)
#ifndef GPIO_PORT_MASK
# define GPIO_PORT_MASK (0x30)
#endif
#ifndef GPIO_PORT_OFFSET
# define GPIO_PORT_OFFSET 4
#endif
// The pin number mask for pin_t
// 4 bits (16 possible pins)
#ifndef GPIO_PIN_MASK
# define GPIO_PIN_MASK  (0x0F)
#endif
#ifndef GPIO_PIN_OFFSET
# define GPIO_PIN_OFFSET 0
#endif
// The pin bias mask for pin_t
// 2 bits, 0 is float/undefined
#ifndef GPIO_BIAS_MASK
# define GPIO_BIAS_MASK (0xC0)
#endif
#ifndef GPIO_BIAS_OFFSET
# define GPIO_BIAS_OFFSET 6
#endif
// BIAS_LOW and BIAS_HIGH don't use the *_MASK/*NO nomenclature because they're
// intended to be used in configuration, where keeping track of mask/number
// is a hassle
#ifndef BIAS_LOW
# define BIAS_LOW (0b01 << GPIO_BIAS_OFFSET)
#endif
#ifndef BIAS_HIGH
# define BIAS_HIGH (0b10 << GPIO_BIAS_OFFSET)
#endif
// HiZ when off push when on, the default
#ifndef BIAS_TRIHI
# define BIAS_TRIHI (0b00 << GPIO_BIAS_OFFSET)
#endif
// HiZ when off pull when on
// Treated the same as TRIHI for inputs and PWM outputs
#ifndef BIAS_TRILO
# define BIAS_TRILO (0b11 << GPIO_BIAS_OFFSET)
#endif
#ifndef BIAS_DEFAULT
# define BIAS_DEFAULT BIAS_TRIHI
#endif

// Maximum value returned by the ADC
#if ! ADC_MAX
# define ADC_MAX 0xFFFF
#endif

// Ideal voltage output by the on-board voltage regulator in millivolts
#if ! REGULATED_VOLTAGE_mV
# define REGULATED_VOLTAGE_mV 3300
#endif
// Consider the regulated voltage low if it falls below this
#if ! REGULATED_VOLTAGE_LOW_mV
# define REGULATED_VOLTAGE_LOW_mV (REGULATED_VOLTAGE_mV-(REGULATED_VOLTAGE_mV/20))
#endif


/*
* Types
*/
// Define the state of a GPIO pin
// GPIO_LOW should always be '0' and GPIO_HIGH should always be '1'; any
// other values can't have assumptions made about them
typedef enum {
	GPIO_LOW  = 0,
	GPIO_HIGH = 1,
	GPIO_FLOAT
} gpio_state_t;
// Define the operation mode of a GPIO pin
// Some modes may be absent or aliased to other modes depending on platform
// Platforms that define their own gpio_mode_t must include these values
#if ! GPIO_MODE_T_IS_DEFINED
typedef enum {
	GPIO_MODE_RESET = 0, // Reset state of the pin
	GPIO_MODE_PP,    // Push-pull output
	GPIO_MODE_OD,    // Open-drain output
	GPIO_MODE_IN,    // Input
	GPIO_MODE_AIN,   // Analog input
	GPIO_MODE_HiZ    // High-impedence mode
} gpio_mode_t;
#endif
// An identifier for MCU pins
// Using an int is simpler than a struct because I don't need to define
// structures to use pins, can check them with the preprocessor, and individual
// backends have more flexibility choosing how to make use of the bits.
// '0' is reserved for unassigned pins.
#if ! PIN_T_IS_DEFINED
typedef uint8_t pin_t;
#endif
#if ! GPIO_QUICK_T_IS_DEFINED
typedef pin_t gpio_quick_t;
#endif

// Type used to hold the value returned by an ADC
#if ! ADC_T_IS_DEFINED
typedef uint16_t adc_t;
// Type used to perform mathematical functions with ADC readings which may
// overflow adc_t such as multiplication or those which may involve negative
// values
typedef int32_t adcm_t;
#endif

#if ! TXSIZE_T_IS_DEFINED
typedef uint16_t txsize_t;
#endif


/*
* Variable declarations
*/
//
// Frontend-provided
//
// IRQs the frontend needs to handle
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
void sleep_ms(utime_t ms);
// Hibernate (lower-power mode) for s seconds
// If CFG_ALLOW_INTERRUPTS is set in flags, exit when an interrupt is received
void hibernate_s(utime_t s, uint8_t flags);
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
// For inputs this sets the bias
// For other modes this shouldn't have an effect
void gpio_set_state(pin_t pin, gpio_state_t new_state);
// Toggle the state of a pin
// For outputs this toggles the pin high or low
// For non-floating inputs this toggles the bias
// For other modes this shouldn't have an effect
void gpio_toggle_state(pin_t pin);
// Get the state of a pin
// Get the value of the input data register (which should match the output
// register on output pins)
gpio_state_t gpio_get_state(pin_t pin);
// Prepare to read the state of a pin quickly
// qpin can be passed to GPIO_QUICK_READ() after being set up here
void gpio_quickread_prepare(volatile gpio_quick_t *qpin, pin_t pin);

#if USE_SERIAL
//
// UART interface
// Turn the UART peripheral on or off
void uart_on(void);
void uart_off(void);
// Receive a block of data
err_t uart_receive_block(uint8_t *buffer, txsize_t size, utime_t timeout);
// Transmit a block of data
err_t uart_transmit_block(const uint8_t *buffer, txsize_t size, utime_t timeout);
#else
# define uart_on()  ((void )0)
# define uart_off() ((void )0)
# define uart_receive_block(...)  ((void )0)
# define uart_transmit_block(...) ((void )0)
#endif

#if USE_SPI
//
// SPI interface
// Turn on and off the SPI peripheral
void spi_on(void);
void spi_off(void);
// Exchange a byte
err_t spi_exchange_byte(uint8_t tx, uint8_t *rx, utime_t timeout);
// Receive a data block
// tx is the value transmitted for each byte read and must be 0xFF when
// communicating with an SD card in order to keep MOSI high; other devices
// may have different (or no) requirements.
err_t spi_receive_block(uint8_t *rx_buffer, txsize_t rx_size, uint8_t tx, utime_t timeout);
// Transmit a data block
err_t spi_transmit_block(const uint8_t *tx_buffer, txsize_t tx_size, utime_t timeout);
#else
# define spi_on()  ((void )0)
# define spi_off() ((void )0)
# define spi_exchange_byte(...)  ((void )0)
# define spi_receive_block(...)  ((void )0)
# define spi_transmit_block(...) ((void )0)
#endif

#if USE_I2C
//
// I2C interface
// Turn on and off the I2C peripheral
void i2c_on(void);
void i2c_off(void);
// Receive a data block
err_t i2c_receive_block(uint8_t addr, uint8_t *rx_buffer, txsize_t rx_size, utime_t timeout);
// Transmit a data block
err_t i2c_transmit_block(uint8_t addr, const uint8_t *tx_buffer, txsize_t tx_size, utime_t timeout);
#else
# define i2c_on()  ((void )0)
# define i2c_off() ((void )0)
# define i2c_receive_block(...)  ((void )0)
# define i2c_transmit_block(...) ((void )0)
#endif

//
// Time-management interface
// Get and set the system time enumerated in seconds
err_t set_RTC_seconds(utime_t s);
utime_t get_RTC_seconds(void);
// Enable and disable the microsecond counter used for USCOUNTER_START()
// and USCOUNTER_STOP()
void uscounter_on(void);
void uscounter_off(void);
// Don't do anything for a bit
void delay_ms(utime_t ms);
// A 'dumb' delay that doesn't know about ticks
// Don't expect this to be very accurate.
void dumb_delay_ms(utime_t ms);
// A 'dumber' delay that doesn't even know about milliseconds
void dumb_delay_cycles(uint32_t cycles);
// Enable/disable hardware PWM output on an IO pin at the given duty cycle
// The pin mode will be changed by the backend when PWM is enabled but won't
// be switched back when it's disabled, the frontend needs to handle that
void pwm_on(pin_t pin, uint16_t duty_cycle);
void pwm_off(pin_t pin);
// Adjust an already-enabled PWM output
void pwm_set(pin_t pin, uint16_t duty_cycle);

#if USE_ADC
//
// ADC interface
// Enable and disable the ADC peripheral
void adc_on(void);
void adc_off(void);
// Return the analog voltage reference value in millivolts
// Depending on the platform and configuration this may always return
// REGULATED_VOLTAGE_mV
int16_t adc_read_vref_mV(void);
// Read the value on an analog pin
// If the pin selected doesn't support analog reading, 0 is returned
adc_t adc_read_pin(pin_t pin);
// Try to find the amplitude of an AC voltage
// The value returned is half the difference between the high and low peaks
// The period is the time in milliseconds to spend monitoring, typically a
// good value will be the time it takes for a full wave cycle
adc_t adc_read_ac_amplitude(pin_t pin, uint32_t period_ms);
#else
# define adc_on()  ((void )0)
# define adc_off() ((void )0)
# define adc_read_vref_mV() REGULATED_VOLTAGE_mV
# define adc_read_pin(p) 0
# define adc_read_ac_amplitude(p, t) 0
#endif


//
// Frontend-provided
//
#if USE_SERIAL
// Print a debug message
void logger(const char *fmt, ...)
	__attribute__ ((format(printf, 1, 2)));
// Print formatted string
void serial_printf(const char *fmt, ...)
	__attribute__ ((format(printf, 1, 2)));
// Print a string
void serial_print(const char *msg, txsize_t len);
#else
# define logger(...)        ((void )0)
# define serial_printf(...) ((void )0)
# define serial_print(...)  ((void )0)
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

// Power a specific pin on and off
// power_{on,off}_output() will handle PWM for any pin specified by PWM_PINS
// The backend should only use these after the clock and input subsystems are
// initialized.
void power_on_PWM_output(pin_t pin, uint16_t duty_cycle);
void power_off_PWM_output(pin_t pin);
void power_on_output(pin_t pin);
void power_off_output(pin_t pin);
void power_on_input(pin_t pin);
void power_off_input(pin_t pin);

/*
* Macros
*/
#if (DEBUG || USE_SMALL_CODE < 2) && USE_SERIAL
# define ERROR_STATE(msg)     error_state(F1(__FILE__), __LINE__, __func__, F(msg))
# define ERROR_STATE_NOF(msg) error_state(F1(__FILE__), __LINE__, __func__, msg)
#else
# define ERROR_STATE(msg)     error_state_crude()
# define ERROR_STATE_NOF(msg) error_state_crude()
#endif

#if DEBUG && USE_SERIAL
# define LOGGER(fmt, ...)      logger(F1(fmt), ## __VA_ARGS__)
# define LOGGER_NOF(fmt, ...)  logger(fmt, ## __VA_ARGS__)
#else
# define LOGGER(...)      ((void )0U)
# define LOGGER_NOF(...)  ((void )0U)
#endif
#if USE_SERIAL
# define NOTIFY(fmt, ...)      logger(F1(fmt), ## __VA_ARGS__)
# define NOTIFY_NOF(fmt, ...)  logger(fmt, ## __VA_ARGS__)
#else
# define NOTIFY(...)      ((void )0U)
# define NOTIFY_NOF(...)  ((void )0U)
#endif

#if USE_SERIAL
# define PRINTF(fmt, ...)     serial_printf(F1(fmt), ## __VA_ARGS__)
# define PRINTF_NOF(fmt, ...) serial_printf(fmt, ## __VA_ARGS__)
# define PUTS(msg, len)       serial_print(F1(msg), len)
# define PUTS_NOF(msg, len)   serial_print(msg, len)
#else
# define PRINTF(...)     ((void )0U)
# define PRINTF_NOF(...) ((void )0U)
# define PUTS(...)       ((void )0U)
# define PUTS_NOF(...)   ((void )0U)
#endif

// Get the system time enumerated in seconds; this may be lower for a later
// read if the system time was set in the meantime, but it updates even
// during sleep
#ifndef NOW
# define NOW() (get_RTC_seconds())
#endif
// Read the systick counter; this always increases or stays the same from one
// read to the next but is stopped during sleep
#ifndef NOW_MS
# define NOW_MS() (G_sys_msticks)
#endif

// Get the pin number of a given pin
#ifndef GPIO_GET_PINNO
# define GPIO_GET_PINNO(pin) (((pin) & GPIO_PIN_MASK) >> GPIO_PIN_OFFSET)
#endif
// Get the bit mask corresponding to a given pin's number
#ifndef GPIO_GET_PINMASK
# define GPIO_GET_PINMASK(pin) _BV(GPIO_GET_PINNO((pin)))
#endif

// Get the port number of a given pin
#ifndef GPIO_GET_PORTNO
# define GPIO_GET_PORTNO(pin) (((pin) & GPIO_PORT_MASK) >> GPIO_PORT_OFFSET)
#endif
// Get the bit mask corresponding to a given pin's port
#ifndef GPIO_GET_PORTMASK
# define GPIO_GET_PORTMASK(pin) ((pin) & GPIO_PORT_MASK)
#endif
// Get the port of a given pin
#ifndef GPIO_GET_PORT
# define GPIO_GET_PORT(pin) GPIO_GET_PORTNO(pin)
#endif

// Get the bias of a given pin
#ifndef GPIO_GET_BIAS
# define GPIO_GET_BIAS(pin) ((pin) & GPIO_BIAS_MASK)
#endif
#define GPIO_BIAS_TO_STATE(pin) ((((pin) & GPIO_BIAS_MASK) == BIAS_HIGH) ? GPIO_HIGH : (((pin) & GPIO_BIAS_MASK) == BIAS_LOW) ? GPIO_LOW : GPIO_FLOAT)

// Get the identifier of a given pin
#ifndef PINID
# define PINID(pin) ((pin) & (GPIO_PIN_MASK | GPIO_PORT_MASK))
#endif

// Read a pin quickly; qpin must be set by gpio_quickread_prepare() beforehand
#ifndef GPIO_QUICK_READ
# define GPIO_QUICK_READ(qpin) (gpio_get_state(qpin))
#endif

#endif // _PLATFORM_INTERFACE_H
#ifdef __cplusplus
 }
#endif
