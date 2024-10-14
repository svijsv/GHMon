#include "common.h"

#if TEST_UART_LISTEN

#include "ulib/include/ascii.h"

//
// Globals initialization


//
// Misc functions
void uart_rx_irq_hook(uart_port_t *p) {
	UNUSED(p);
	uHAL_SET_STATUS(uHAL_FLAG_IRQ);

	return;
}


//
// main() initialization
void init_UART_LISTEN(void) {
	uart_listen_on(UART_COMM_PORT);

	return;
}


//
// Main loop
void loop_UART_LISTEN(void) {
	if (uart_rx_is_available(UART_COMM_PORT)) {
		uint8_t ibuf;

		serial_printf("RX echo (timeout: 1000ms):\r\n");
		while (uart_receive_block(UART_COMM_PORT, &ibuf, 1, 1000) == ERR_OK) {
			if (ascii_is_graph(ibuf)) {
				serial_printf("'%c'\r\n", (int_t )ibuf);
			} else {
				serial_printf("0x%02X\r\n", (uint_t )ibuf);
			}
		}
	}

	return;
}


#endif // TEST_UART_LISTEN
