#if WRITE_LOG_TO_UART

//
// If we don't have a log-specific UART output, send it to the standard serial
// output
#if !defined(UART_LOG_TX_PIN) || UART_LOG_TX_PIN == 0
# undef UART_LOG_TX_PIN
# undef UART_LOG_RX_PIN
# define UART_LOG_TX_PIN UART_COMM_TX_PIN
# define UART_LOG_RX_PIN UART_COMM_RX_PIN
# if uHAL_USE_UART_COMM
#  define LOG_UART_IS_COMM 1
# else
#  define LOG_UART_IS_COMM 0
# endif
#else
# define LOG_UART_IS_COMM 0
#endif

static bool print_to_UART = false;
static uart_port_t *uart_log_port = NULL;

static void init_log_UART(void) {
#if LOG_UART_IS_COMM
	uart_log_port = UART_COMM_PORT;
#else
	err_t res;
	static uart_port_t _uart_log_port;
	uart_port_cfg_t cfg = {
		.baud_rate = UART_LOG_BAUDRATE,
		.tx_pin = UART_LOG_TX_PIN,
		.rx_pin = UART_LOG_RX_PIN
	};

	if ((res = uart_init_port(&_uart_log_port, &cfg)) != ERR_OK) {
		PRINTF("UART log initialization failed: error %d", (int )res);
	} else {
		uart_log_port = &_uart_log_port;
	}
#endif

	return;
}

static err_t write_buffer_to_UART(uint8_t *buf, print_buffer_size_t bytes) {
	if (!print_to_UART) {
		return ERR_OK;
	}
	if (!SKIP_SAFETY_CHECKS && uart_log_port == NULL) {
		return ERR_INIT;
	}

	return uart_transmit_block(uart_log_port, buf, bytes, UART_LOG_TIMEOUT_MS);
}

static err_t open_UART(void) {
	err_t res;

	if (!SKIP_SAFETY_CHECKS && uart_log_port == NULL) {
		return ERR_INIT;
	}

	res = LOG_UART_IS_COMM ? ERR_OK : uart_on(uart_log_port);
	if (res == ERR_OK) {
		print_to_UART = true;
	}

	return res;
}
static err_t close_UART(void) {
	print_to_UART = false;

	if (!SKIP_SAFETY_CHECKS && uart_log_port == NULL) {
		return ERR_INIT;
	}

	return LOG_UART_IS_COMM ? ERR_OK : uart_off(uart_log_port);
}

#else // WRITE_LOG_TO_UART
# define init_log_UART() (void )0U
# define write_buffer_to_UART(_b_, _s_) (ERR_OK)
# define open_UART() (ERR_OK)
# define close_UART() (ERR_OK)
#endif // WRITE_LOG_TO_UART
