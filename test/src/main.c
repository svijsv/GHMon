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
// main.c
// Main program logic
// NOTES:
//
#include "common.h"
#include "gather_tests.h"


/*
#include <stdarg.h>
void xvfprintf(void(*func)(int), const char*	fmt, va_list arp);
void printf_va(void (*pputc)(uint_fast8_t c), const char *restrict fmt, va_list arp) {
	xvfprintf(pputc, fmt, arp);
	return;
}
*/

// UART interrupts can't wake us up from deep sleep so sleep lightly for a
// few seconds before entering deep sleep in order to detect them
// The user will have to use the button to enter the serial terminal.
#if HAVE_STM32 && uHAL_USE_TERMINAL
# define LIGHT_SLEEP_SECONDS 2
void pre_hibernate_hook(utime_t *s, sleep_mode_t *sleep_mode, uHAL_flags_t flags) {
	assert(s != NULL);
	assert(sleep_mode != NULL);

	if (BIT_IS_SET(flags, uHAL_CFG_ALLOW_INTERRUPTS) && (*sleep_mode > HIBERNATE_LIGHT)) {
		uint32_t w;
		utime_t ss = *s;

		w = (ss > (LIGHT_SLEEP_SECONDS)) ? LIGHT_SLEEP_SECONDS : ss;
		hibernate_s(w, HIBERNATE_LIGHT, flags);
		*s = ss - w;
	}

	return;
}
#endif


#if GPIO_PIN_IS_VALID(LED_PIN)
void error_state_hook(void) {
	gpio_toggle_output_state(LED_PIN);
}
#endif

int main(void) {
	platform_init();

	init_RESET();
	init_LED();
	init_LED_PINCTRL();
	init_LED_PINCTRL2();
	init_PWM();
	init_BUTTON();
	init_USCOUNTER();
	init_DATE();
	init_ADC();
	init_SD();
	init_UART_LISTEN();
	init_TERMINAL();
	init_SSD1306();

	while (true) {
		loop_LED();
		loop_LED_PINCTRL();
		loop_LED_PINCTRL2();
		loop_PWM();
		loop_USCOUNTER();
		loop_DATE();
		loop_ADC();
		loop_SD();
		loop_SSD1306();

		uHAL_CLEAR_STATUS(uHAL_FLAG_IRQ);
#if TEST_SLEEP
		DEBUG_CPP_MSG("Loop is sleeping")
		sleep_ms(PAUSE_MS);

#elif TEST_HIBERNATE
		DEBUG_CPP_MSG("Loop is hibernating")
		hibernate_s(PAUSE_MS/1000U, TEST_HIBERNATE_MODE, uHAL_CFG_ALLOW_INTERRUPTS);

#elif TEST_HIBERNATE_FOREVER
		DEBUG_CPP_MSG("Loop is hibernating... FOREVER")
		hibernate(TEST_HIBERNATE_FOREVER_MODE, uHAL_CFG_ALLOW_INTERRUPTS);

#else
		DEBUG_CPP_MSG("Loop is delaying")
		delay_ms(PAUSE_MS);
#endif

		loop_BUTTON();
		loop_UART_LISTEN();
		loop_TERMINAL();
		loop_RESET();
		//PRINTF("\r\n");
	}

	return 0;
}
