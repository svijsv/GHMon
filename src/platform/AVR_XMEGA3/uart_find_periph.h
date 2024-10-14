#ifdef USART0
# define HAVE_UART0 1
# define IS_UART0(rx_pin, tx_pin) ((PINID(rx_pin) == PINID_UART0_RX) && (PINID(tx_pin) == PINID_UART0_TX))
#else
# define HAVE_UART0 0
# define IS_UART0(rx_pin, tx_pin) (0)
#endif

#ifdef USART1
# define HAVE_UART1 1
# define IS_UART1(rx_pin, tx_pin) ((PINID(rx_pin) == PINID_UART1_RX) && (PINID(tx_pin) == PINID_UART1_TX))
#else
# define HAVE_UART1 0
# define IS_UART1(rx_pin, tx_pin) (0)
#endif

#ifdef USART2
# define HAVE_UART2 1
# define IS_UART2(rx_pin, tx_pin) ((PINID(rx_pin) == PINID_UART2_RX) && (PINID(tx_pin) == PINID_UART2_TX))
#else
# define HAVE_UART2 0
# define IS_UART2(rx_pin, tx_pin) (0)
#endif

#ifdef USART3
# define HAVE_UART3 1
# define IS_UART3(rx_pin, tx_pin) ((PINID(rx_pin) == PINID_UART3_RX) && (PINID(tx_pin) == PINID_UART3_TX))
#else
# define HAVE_UART3 0
# define IS_UART3(rx_pin, tx_pin) (0)
#endif
