#if UART_INPUT_BUFFER_BYTES <= 0xFFU
typedef uint_fast8_t uart_buffer_size_t;
#elif UART_INPUT_BUFFER_BYTES <= 0xFFFFU
typedef uint_fast16_t uart_buffer_size_t;
#elif UART_INPUT_BUFFER_BYTES <= 0xFFFFFFFFU
typedef uint_fast32_t uart_buffer_size_t;
#else
# error "Unsupported UART_INPUT_BUFFER_BYTES size"
#endif

#if ENABLE_UART_LISTENING
typedef struct {
	uart_buffer_size_t buffer[UART_INPUT_BUFFER_BYTES];
	uart_buffer_size_t bytes;
} uart_buffer_t;
#endif
