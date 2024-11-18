#if WRITE_LOG_TO_UART

static bool print_to_uart = false;
static uart_port_t *uart_log_port = NULL;

//#define LOG_UART_IS_COMM ((UART_LOG_TX_PIN == UART_COMM_TX_PIN && UART_LOG_RX_PIN == UART_COMM_RX_PIN) || (UART_LOG_TX_PIN == 0 && UART_LOG_RX_PIN == 0))
// Only checking the TX pin simplifies things a bit becauase we can just not care
// if the RX pin is mistmatched - we don't use it anyway.
#define LOG_UART_IS_COMM (uHAL_USE_UART_COMM && ((UART_LOG_TX_PIN == UART_COMM_TX_PIN) || (UART_LOG_TX_PIN == 0)))

static void log_init_UART(void) {
	if (LOG_UART_IS_COMM) {
		uart_log_port = UART_COMM_PORT;
	} else {
		err_t res;
		static uart_port_t _uart_log_port;
		uart_port_cfg_t cfg = {
			.baud_rate = UART_LOG_BAUDRATE,
			.tx_pin = UART_LOG_TX_PIN,
			.rx_pin = UART_LOG_RX_PIN
		};

		if ((res = uart_init_port(&_uart_log_port, &cfg)) != ERR_OK) {
			LOGGER("UART log initialization failed: %u", res);
		} else {
			uart_log_port = &_uart_log_port;
		}
	}

	return;
}

#if LOG_PRINT_BUFFER_SIZE > 0
static err_t write_buffer_to_UART(void) {
	if (uart_log_port == NULL) {
		return ERR_INIT;
	}
	return (print_to_uart) ? uart_transmit_block(uart_log_port, print_buffer.buffer, print_buffer.size, UART_LOG_TIMEOUT_MS) : ERR_OK;
}
#else
static err_t write_char_to_UART(uint8_t c) {
	if (uart_log_port == NULL) {
		return ERR_INIT;
	}
	return (print_to_uart) ? uart_transmit_block(uart_log_port, &c, 1, UART_LOG_TIMEOUT_MS) : ERR_OK;
}
#endif

static err_t prepare_UART(void) {
	err_t res;

	if (uart_log_port == NULL) {
		return ERR_INIT;
	}
	res = LOG_UART_IS_COMM ? ERR_OK : uart_on(uart_log_port);
	if (res == ERR_OK) {
		print_to_uart = true;
	}
	return res;
}
static err_t close_UART(void) {
	print_to_uart = false;

	if (uart_log_port == NULL) {
		return ERR_INIT;
	}

#if LOG_PRINT_BUFFER_SIZE > 0
	write_buffer_to_UART();
#endif

	return LOG_UART_IS_COMM ? ERR_OK : uart_off(uart_log_port);
}

#endif // WRITE_LOG_TO_UART
