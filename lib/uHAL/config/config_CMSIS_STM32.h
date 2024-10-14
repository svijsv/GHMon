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
//    The commented-out values are the defaults
//    Any of these settings can also be set on the command line
//
#ifndef _uHAL_CONFIG_CMSIS_H
#define _uHAL_CONFIG_CMSIS_H


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
#define uHAL_GPIO_SPEED OUTPUT_SLOW

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


//
// Pin configuration
// Platform-specific
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
// On STM32 devices which expose pins on ports C or H, there are limitations
// to the use of those pins which can be found in the GPIO section of the
// reference manual. The short version is don't use PC13 if the RTC is used (it
// probably is), PC14 or PC15 when using  the LSE oscillator, or PH0 or PH1 when
// using the HSE oscillator, and don't source current or sink more than 3mA on
// PC13-PC15 or set them to a speed > 2MHz.
//
// On bluepill-like boards:
// Pins A15, B3, and B4 are JTAG and will be pulled up or down when flashing
// and during power on, and so shouldn't be used to power anything that doesn't
// like that (like an SD card) if it will be present at those times
//
// Some pins (like A12) may have external pullups or pulldowns
//
// F1-line STM32s only allow remapping peripheral pins in batches. In order
// to use the remapped pins, set the corresponding flag. Check the datasheet
// for pin mappings.
// Possible values are GPIO_REMAP_{DEFAULT,FULL,PARTIAL,PARTIAL_1,PARTIAL_2}
// Other STM32 lines don't use these flags.
#define GPIO_REMAP_SPI1  0
#define GPIO_REMAP_I2C1  0
#define GPIO_REMAP_UART1 0
#define GPIO_REMAP_UART2 0
#define GPIO_REMAP_UART3 0
#define GPIO_REMAP_TIM1  0
#define GPIO_REMAP_TIM2  0
#define GPIO_REMAP_TIM3  0
#define GPIO_REMAP_TIM4  0
#define GPIO_REMAP_TIM9  0
#define GPIO_REMAP_TIM10 0
#define GPIO_REMAP_TIM11 0
#define GPIO_REMAP_TIM13 0
#define GPIO_REMAP_TIM14 0
//
// If non-zero, any GPIO port which the device header files expose will be
// enabled. Otherwise, any port used needs to be explicitly enabled by setting
// HAVE_GPIO_PORTx to non-zero either on the command line or in this configuration
// file
#define HAVE_GPIO_PORT_DEFAULT 0
#define HAVE_GPIO_PORTA 1
#define HAVE_GPIO_PORTB 1
//
// When non-zero, disable PWM outputs on a given port
// This has the effect of reducing flash memory usage by 48 bytes per disabled
// port on non-F1 devices
// It's not necessary to explicitly disable PWM on a port if the port wasn't
// enabled above for general GPIO usage
#define DISABLE_PORTA_PWM 0
#define DISABLE_PORTB_PWM 0
//
// UART serial console pins
#define UART_COMM_TX_PIN PINID_UART1_TX
#define UART_COMM_RX_PIN PINID_UART1_RX
//
// SPI pins
#define SPI_SS_PIN    PINID_SPI1_NSS
#define SPI_SCK_PIN   PINID_SPI1_SCK
#define SPI_MISO_PIN  PINID_SPI1_MISO
#define SPI_MOSI_PIN  PINID_SPI1_MOSI
#define SPI_CS_SD_PIN SPI_SS_PIN
//
// I2C pins
#define I2C_SDA_PIN PINID_I2C1_SDA
#define I2C_SCL_PIN PINID_I2C1_SCL


#endif //_uHAL_CONFIG_CMSIS_H
