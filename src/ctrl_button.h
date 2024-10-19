// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2024 svijsv                                                *
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
// ctrl_button.h
// Configure the control button
// NOTES:
//
#ifndef _CTRL_BUTTON_H
#define _CTRL_BUTTON_H

static uint_fast8_t ctrl_button_pressed = 0;
static gpio_listen_t ctrl_button_listen_handle;

#if HAVE_STM32
# if GPIO_GET_PINNO(CTRL_BUTTON_PIN) == 0
#  define CTRL_BUTTON_ISR EXTI0_IRQHandler
# elif GPIO_GET_PINNO(CTRL_BUTTON_PIN) == 1
#  define CTRL_BUTTON_ISR EXTI1_IRQHandler
# elif GPIO_GET_PINNO(CTRL_BUTTON_PIN) == 2
#  define CTRL_BUTTON_ISR EXTI2_IRQHandler
# elif GPIO_GET_PINNO(CTRL_BUTTON_PIN) == 3
#  define CTRL_BUTTON_ISR EXTI3_IRQHandler
# elif GPIO_GET_PINNO(CTRL_BUTTON_PIN) == 4
#  define CTRL_BUTTON_ISR EXTI4_IRQHandler
# elif (GPIO_GET_PINNO(CTRL_BUTTON_PIN) >= 5) && (GPIO_GET_PINNO(CTRL_BUTTON_PIN) <= 9)
#  define CTRL_BUTTON_ISR EXTI9_5_IRQHandler
# elif (GPIO_GET_PINNO(CTRL_BUTTON_PIN) >= 10) && (GPIO_GET_PINNO(CTRL_BUTTON_PIN) <= 15)
#  define CTRL_BUTTON_ISR EXTI15_10_IRQHandler
# else
#  error "CTRL_BUTTON_PIN is unhandled"
# endif
# define ISR(_f_) void _f_(void)
# define CLEAR_CTRL_BUTTON_ISR() (void )0U

#elif HAVE_AVR_XMEGA3
# include <avr/interrupt.h> // For button interrupt handling
# if (GPIO_GET_PORTNO(CTRL_BUTTON_PIN) == GPIO_PORTA)
#  define CTRL_BUTTON_ISR PORTA_PORT_vect
#  define BUTTON_PORT PORTA
# elif (GPIO_GET_PORTNO(CTRL_BUTTON_PIN) == GPIO_PORTB)
#  define CTRL_BUTTON_ISR PORTB_PORT_vect
#  define BUTTON_PORT PORTB
# else
#  error "CTRL_BUTTON_PIN is unhandled"
# endif
// Write '1' to the flag to clear it
# define CLEAR_CTRL_BUTTON_ISR() do { BUTTON_PORT.INTFLAGS = GPIO_GET_PINMASK(CTRL_BUTTON_PIN); } while (0)

#else
# error "Unhandled device"
#endif

DEBUG_CPP_MACRO(CTRL_BUTTON_ISR);

ISR(CTRL_BUTTON_ISR) {
	CLEAR_CTRL_BUTTON_ISR();

	// Need to turn the interrupt off to clear interrupt and keep it off so button
	// bounce doesn't re-trigger
	input_pin_listen_off(&ctrl_button_listen_handle);

	SET_BIT(ghmon_IRQs, BUTTON_IRQ_FLAG);
	uHAL_SET_STATUS(uHAL_FLAG_IRQ);

	return;
}

//
// IRQ handler for the control button, called from the main loop
static void button_IRQHandler(void) {
	utime_t timeout;

	ctrl_button_pressed = 1;
	led_flash(1, STATUS_LED_ACK_DELAY_MS);
	timeout = SET_TIMEOUT_MS(CTRL_PRESS_MS);

	if (BUTTON_DEBOUNCE_MS) {
		sleep_ms(BUTTON_DEBOUNCE_MS);
	}

	// Use a delay of 10ms to limit the influence of sleep_ms() overhead
	do {
		if (TIMES_UP(timeout)) {
			++ctrl_button_pressed;
			led_flash(1, STATUS_LED_ACK_DELAY_MS);
			timeout = SET_TIMEOUT_MS(CTRL_PRESS_MS);
		}
		// This must be delay_ms(), sleep_ms() pauses the systick timer which
		// we need to detect timeouts.
		delay_ms(10);
	} while (input_pin_is_on(CTRL_BUTTON_PIN));

	if (BUTTON_DEBOUNCE_MS) {
		sleep_ms(BUTTON_DEBOUNCE_MS);
	}

	return;
}


#endif // _CTRL_BUTTON_H
