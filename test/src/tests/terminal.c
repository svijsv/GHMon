#include "common.h"

#if TEST_TERMINAL

#include "ulib/include/cstrings.h"
#include <stdlib.h>


#if ! uHAL_USE_HIBERNATE
# define sleep_ms(_ms_) delay_ms(_ms_)
#endif


//
// Misc functions
void uart_rx_irq_hook(uart_port_t *p) {
	UNUSED(p);
	uHAL_SET_STATUS(uHAL_FLAG_IRQ);

	return;
}

#define NEXT_TOK(_cs, _sep) (cstring_next_token((_cs), (_sep)))

void led_on(void) {
	output_pin_on(TEST_TERMINAL_LED_PIN);

	return;
}
void led_off(void) {
	output_pin_off(TEST_TERMINAL_LED_PIN);

	return;
}
void led_toggle(void) {
	output_pin_toggle(TEST_TERMINAL_LED_PIN);

	return;
}
void led_flash(uint_fast8_t count, uint_fast16_t ms) {
	for (uiter_t i = 0; i < count; ++i) {
		led_toggle();
		sleep_ms(ms);
		led_toggle();
		// Slight delay to keep separate flashes distinct
		sleep_ms(200);
	}

	return;
}

// Format: 'led_flash <N>'
static int terminalcmd_led_flash(const char *line_in) {
	int n;

	n = atoi(NEXT_TOK(line_in, ' '));

	PRINTF("Flashing LED %u times.\r\n", (uint )n);

	led_flash(n, 300);

	return 0;
}
static int terminalcmd_led_on(const char *line_in) {
	UNUSED(line_in);
	PUTS("Turning LED on\r\n", 0);
	led_on();
	return 0;
}
static int terminalcmd_led_off(const char *line_in) {
	UNUSED(line_in);
	PUTS("Turning LED off\r\n", 0);
	led_off();
	return 0;
}
static int terminalcmd_led_toggle(const char *line_in) {
	UNUSED(line_in);
	PUTS("Toggling LED\r\n", 0);
	led_toggle();
	return 0;
}
static int terminalcmd_reset(const char *line_in) {
	UNUSED(line_in);
	PUTS("Resetting device\r\n", 0);
	platform_reset();
	return 0;
}

/*
// If I understand the header right, 'peek 4 0x40023804' should give the
// value of the PLLCFGR register on STM32F1 but it hard faults instead and
// I don't really care enough to sort it out right now
// hard fault handler is defined in cmsis/interrupts.c if i decide to try again
static int terminalcmd_peek(const char *line_in) {
	uint_fast8_t size;
	uint_t i = 0;

	line_in = NEXT_TOK(line_in, ' ');
	size = strtol(line_in, NULL, 0);

	while (*line_in != 0) {
		line_in = NEXT_TOK(line_in, ' ');
		void *ptr;
		// 8 hex digits + '0x' + \0
		unsigned long value = 0;

		ptr = (void *)strtol(line_in, NULL, 0);

		switch (size) {
		case 1:
			value = *((uint8_t *)ptr);
			break;
		case 2:
			value = *((uint16_t *)ptr);
			break;
		case 4:
			value = *((uint32_t *)ptr);
			break;
		}

		PRINTF("ADDR %u:\t0x%08lX\r\n", i, value);
		++i;
	}

	return 0;
}
*/

FMEM_STORAGE const terminal_cmd_t terminal_extra_cmds[] = {
	{ terminalcmd_led_flash,   "led_flash",   9 },
	{ terminalcmd_led_on,      "led_on",      6 },
	{ terminalcmd_led_off,     "led_off",     7 },
	{ terminalcmd_led_toggle,  "led_toggle", 10 },
	{ terminalcmd_reset,       "reset",       5 },
//	{ terminalcmd_peek,        "peek",        4 },
	{ NULL, {0}, 0 },
};
FMEM_STORAGE const char terminal_extra_help[] =
"   led_flash <N>              - Flash the LED <N> times\r\n"
"   led_on\r\n"
"   led_off\r\n"
"   led_toggle\r\n"
"   reset                      - Reset the device\r\n"
//"   peek <size> <ADDR1 ADDRn>  - Display the <size> bytes of memory at location(s) <ADDR>\r\n"
;


//
// main() initialization
void init_TERMINAL(void) {
	uart_listen_on(UART_COMM_PORT);

	return;
}


//
// Main loop
void loop_TERMINAL(void) {
	if (uart_rx_is_available(UART_COMM_PORT)) {
		terminal();
	}

	return;
}


#endif // TEST_TERMINAL
