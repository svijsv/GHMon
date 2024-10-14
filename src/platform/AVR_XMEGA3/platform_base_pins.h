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
// platform_base_pins.h
// Platform-specific shim for the frontend
// NOTES:
//   This file is for defining the base pin IDs without cluttering platform.h
//   and should only be included by platform.h
//

//
// IO bit masks
//
// 3 bits (8 possible pins)
#define GPIO_PIN_OFFSET (0U)
#define GPIO_PIN_MASK  (0x07U)
//
// Save a bit for the GPIO control bits when we don't need it for ports so we
// don't have to use 16-bit ints for pins or sacrifice high-level GPIO functionality
#if defined(PORTD)
 //
 // 3 bits, can't be 0 (7 possible ports)
# define GPIO_PORT_OFFSET (3U)
# define GPIO_PORT_MASK (0b111U << GPIO_PORT_OFFSET)
 //
 // 3 bits
 // If we're going to have to use 2 bytes, we might as well put the whole
 // control mask in the high one.
# define GPIO_CTRL_OFFSET (8U)
# define GPIO_CTRL_MASK (0b111U << GPIO_CTRL_OFFSET)
#else
 //
 // 2 bits, can't be 0 (3 possible ports)
# define GPIO_PORT_OFFSET (3U)
# define GPIO_PORT_MASK (0b11U << GPIO_PORT_OFFSET)
 //
 // 3 bits
# define GPIO_CTRL_OFFSET (5U)
# define GPIO_CTRL_MASK (0b111U << GPIO_CTRL_OFFSET)
#endif

#if GPIO_CTRL_OFFSET <= 5
typedef uint_fast8_t gpio_pin_t;
#else
typedef uint_fast16_t gpio_pin_t;
#endif

//#define GPIO_PIN_IS_VALID(_pin_) (PINID(_pin_) != 0 && GPIO_GET_PINNO(_pin_) <= 7)
//#define GPIO_PIN_IS_VALID(_pin_) (PINID(_pin_) != 0)
#define GPIO_PIN_IS_VALID(_pin_) (((_pin_) & GPIO_PORT_MASK) != 0)

#ifndef HAVE_GPIO_PORT_DEFAULT
# define HAVE_GPIO_PORT_DEFAULT 1
#endif

//
// Port A
#ifndef HAVE_GPIO_PORTA
# if HAVE_GPIO_PORT_DEFAULT && defined(PORTA)
#  define HAVE_GPIO_PORTA 1
# else
#  define HAVE_GPIO_PORTA 0
# endif
#endif
#if HAVE_GPIO_PORTA
# define GPIO_PORTA 1U
# define GPIO_PORTA_MASK (GPIO_PORTA << GPIO_PORT_OFFSET)
# define PINID_A0  (GPIO_PORTA_MASK | 0x00U)
# define PINID_A1  (GPIO_PORTA_MASK | 0x01U)
# define PINID_A2  (GPIO_PORTA_MASK | 0x02U)
# define PINID_A3  (GPIO_PORTA_MASK | 0x03U)
# define PINID_A4  (GPIO_PORTA_MASK | 0x04U)
# define PINID_A5  (GPIO_PORTA_MASK | 0x05U)
# define PINID_A6  (GPIO_PORTA_MASK | 0x06U)
# define PINID_A7  (GPIO_PORTA_MASK | 0x07U)
#endif

//
// Port B
#ifndef HAVE_GPIO_PORTB
# if HAVE_GPIO_PORT_DEFAULT && defined(PORTB)
#  define HAVE_GPIO_PORTB 1
# else
#  define HAVE_GPIO_PORTB 0
# endif
#endif
#if HAVE_GPIO_PORTB
# define GPIO_PORTB 1U
# define GPIO_PORTB_MASK (GPIO_PORTB << GPIO_PORT_OFFSET)
# define PINID_B0  (GPIO_PORTB_MASK | 0x00U)
# define PINID_B1  (GPIO_PORTB_MASK | 0x01U)
# define PINID_B2  (GPIO_PORTB_MASK | 0x02U)
# define PINID_B3  (GPIO_PORTB_MASK | 0x03U)
# define PINID_B4  (GPIO_PORTB_MASK | 0x04U)
# define PINID_B5  (GPIO_PORTB_MASK | 0x05U)
# define PINID_B6  (GPIO_PORTB_MASK | 0x06U)
# define PINID_B7  (GPIO_PORTB_MASK | 0x07U)
#endif

//
// Port C
#ifndef HAVE_GPIO_PORTC
# if HAVE_GPIO_PORT_DEFAULT && defined(PORTC)
#  define HAVE_GPIO_PORTC 1
# else
#  define HAVE_GPIO_PORTC 0
# endif
#endif
#if HAVE_GPIO_PORTC
# define GPIO_PORTC 1U
# define GPIO_PORTC_MASK (GPIO_PORTC << GPIO_PORT_OFFSET)
# define PINID_C0  (GPIO_PORTC_MASK | 0x00U)
# define PINID_C1  (GPIO_PORTC_MASK | 0x01U)
# define PINID_C2  (GPIO_PORTC_MASK | 0x02U)
# define PINID_C3  (GPIO_PORTC_MASK | 0x03U)
# define PINID_C4  (GPIO_PORTC_MASK | 0x04U)
# define PINID_C5  (GPIO_PORTC_MASK | 0x05U)
# define PINID_C6  (GPIO_PORTC_MASK | 0x06U)
# define PINID_C7  (GPIO_PORTC_MASK | 0x07U)
#endif

//
// Port D
#ifndef HAVE_GPIO_PORTD
# if HAVE_GPIO_PORT_DEFAULT && defined(PORTD)
#  define HAVE_GPIO_PORTD 1
# else
#  define HAVE_GPIO_PORTD 0
# endif
#endif
#if HAVE_GPIO_PORTD
# define GPIO_PORTD 1U
# define GPIO_PORTD_MASK (GPIO_PORTD << GPIO_PORT_OFFSET)
# define PINID_D0  (GPIO_PORTD_MASK | 0x00U)
# define PINID_D1  (GPIO_PORTD_MASK | 0x01U)
# define PINID_D2  (GPIO_PORTD_MASK | 0x02U)
# define PINID_D3  (GPIO_PORTD_MASK | 0x03U)
# define PINID_D4  (GPIO_PORTD_MASK | 0x04U)
# define PINID_D5  (GPIO_PORTD_MASK | 0x05U)
# define PINID_D6  (GPIO_PORTD_MASK | 0x06U)
# define PINID_D7  (GPIO_PORTD_MASK | 0x07U)
#endif

//
// Port E
#ifndef HAVE_GPIO_PORTE
# if HAVE_GPIO_PORT_DEFAULT && defined(PORTE)
#  define HAVE_GPIO_PORTE 1
# else
#  define HAVE_GPIO_PORTE 0
# endif
#endif
#if HAVE_GPIO_PORTE
# define GPIO_PORTE 1U
# define GPIO_PORTE_MASK (GPIO_PORTE << GPIO_PORT_OFFSET)
# define PINID_E0  (GPIO_PORTE_MASK | 0x00U)
# define PINID_E1  (GPIO_PORTE_MASK | 0x01U)
# define PINID_E2  (GPIO_PORTE_MASK | 0x02U)
# define PINID_E3  (GPIO_PORTE_MASK | 0x03U)
# define PINID_E4  (GPIO_PORTE_MASK | 0x04U)
# define PINID_E5  (GPIO_PORTE_MASK | 0x05U)
# define PINID_E6  (GPIO_PORTE_MASK | 0x06U)
# define PINID_E7  (GPIO_PORTE_MASK | 0x07U)
#endif

//
// Port F
#ifndef HAVE_GPIO_PORTF
# if HAVE_GPIO_PORT_DEFAULT && defined(PORTF)
#  define HAVE_GPIO_PORTF 1
# else
#  define HAVE_GPIO_PORTF 0
# endif
#endif
#if HAVE_GPIO_PORTF
# define GPIO_PORTF 1U
# define GPIO_PORTF_MASK (GPIO_PORTF << GPIO_PORT_OFFSET)
# define PINID_F0  (GPIO_PORTF_MASK | 0x00U)
# define PINID_F1  (GPIO_PORTF_MASK | 0x01U)
# define PINID_F2  (GPIO_PORTF_MASK | 0x02U)
# define PINID_F3  (GPIO_PORTF_MASK | 0x03U)
# define PINID_F4  (GPIO_PORTF_MASK | 0x04U)
# define PINID_F5  (GPIO_PORTF_MASK | 0x05U)
# define PINID_F6  (GPIO_PORTF_MASK | 0x06U)
# define PINID_F7  (GPIO_PORTF_MASK | 0x07U)
#endif
