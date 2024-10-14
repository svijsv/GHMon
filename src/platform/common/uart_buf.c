//
// This file is meant for direct inclusion by uart.c (or the platform equivalent)
// and should not be compiled directly
//

#if UART_INPUT_BUFFER_BYTES > 0 && ENABLE_UART_LISTENING
//
// Fill the output buffer with as much of the RX buffer as we can, then return
// the index of the next byte
INLINE txsize_t eat_uart_buffer(uart_port_t *p, uint8_t *buffer, txsize_t size) {
	txsize_t i = 0;

	// Don't bother checking the inputs, this is an internal function and that
	// should all be handled by the caller
	//assert(p != NULL);
	//assert(buffer != NULL);
	//assert(size != NULL);

	if (size == 0) {
		return 0;
	}

	// We disabled the interrupt (you remembered to do that, right?) so we can
	// ignore volatile
	uart_buffer_t *rx_buf = (uart_buffer_t *)&p->rx_buf;

	switch (rx_buf->bytes) {
	case 1:
		buffer[0] = rx_buf->buffer[0];
		i = 1;
		rx_buf->bytes = 0;
		break;

# if UART_INPUT_BUFFER_BYTES > 1
	case 0:
		break;
	default:
		if (rx_buf->bytes < size) {
			memcpy(buffer, rx_buf->buffer, rx_buf->bytes);
			i = rx_buf->bytes;
			rx_buf->bytes = 0;
		} else {
			memcpy(buffer, rx_buf->buffer, size);
			rx_buf->bytes -= size;
			memmove(rx_buf->buffer, &rx_buf->buffer[size], rx_buf->bytes);
			i = size;
		}
# endif // UART_INPUT_BUFFER_BYTES > 1
	}

	return i;
}

#else // UART_INPUT_BUFFER_BYTES > 0 && ENABLE_UART_LISTENING
INLINE txsize_t eat_uart_buffer(uart_port_t *p, uint8_t *buffer, txsize_t size) {
	UNUSED(p);
	UNUSED(buffer);
	UNUSED(size);
	return 0;
}
#endif // UART_INPUT_BUFFER_BYTES > 0 && ENABLE_UART_LISTENING
