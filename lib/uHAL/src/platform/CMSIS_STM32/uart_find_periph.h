//
// Generated by tools/cmsis/uart_find_periph.sh on Sun Sep  1 05:37:24 PM EDT 2024
//

//
// USART1
//
#if defined(USART1)
# define HAVE_UART1 1
#else
# define HAVE_UART1 0
#endif

#if HAVE_UART1
# if defined(PINID_UART1_RX) && PINID_UART1_RX > 0
#  define IS_UART1_RX_DEF(_p_) (PINID(_p_) == PINID_UART1_RX)
# else
#  define IS_UART1_RX_DEF(_p_) (0)
# endif
#if defined(PINID_UART1_RX_ALT2) && PINID_UART1_RX_ALT2 > 0
#  define IS_UART1_RX_ALT2(_p_) (PINID(_p_) == PINID_UART1_RX_ALT2)
# else
#  define IS_UART1_RX_ALT2(_p_) (0)
# endif
#if defined(PINID_UART1_RX_ALT) && PINID_UART1_RX_ALT > 0
#  define IS_UART1_RX_ALT1(_p_) (PINID(_p_) == PINID_UART1_RX_ALT)
# else
#  define IS_UART1_RX_ALT1(_p_) (0)
# endif
# define IS_UART1_RX(_p_) (IS_UART1_RX_DEF(_p_) || IS_UART1_RX_ALT1(_p_) || IS_UART1_RX_ALT2(_p_))

#if defined(PINID_UART1_TX) && PINID_UART1_TX > 0
#  define IS_UART1_TX_DEF(_p_) (PINID(_p_) == PINID_UART1_TX)
# else
#  define IS_UART1_TX_DEF(_p_) (0)
# endif
#if defined(PINID_UART1_TX_ALT2) && PINID_UART1_TX_ALT2 > 0
#  define IS_UART1_TX_ALT2(_p_) (PINID(_p_) == PINID_UART1_TX_ALT2)
# else
#  define IS_UART1_TX_ALT2(_p_) (0)
# endif
#if defined(PINID_UART1_TX_ALT) && PINID_UART1_TX_ALT > 0
#  define IS_UART1_TX_ALT1(_p_) (PINID(_p_) == PINID_UART1_TX_ALT)
# else
#  define IS_UART1_TX_ALT1(_p_) (0)
# endif
# define IS_UART1_TX(_p_) (IS_UART1_TX_DEF(_p_) || IS_UART1_TX_ALT1(_p_) || IS_UART1_TX_ALT2(_p_))

# define IS_UART1(_rxp_, _txp_) (IS_UART1_RX(_rxp_) && IS_UART1_TX(_txp_))
# define IS_UART1_STRUCT(_p_) (IS_UART1((_p_)->rx_pin, (_p_)->tx_pin))

# if uHAL_USE_UART_COMM && IS_UART1(UART_COMM_RX_PIN, UART_COMM_TX_PIN)
#  define UART_COMM_IRQn       USART1_IRQn
#  define UART_COMM_IRQHandler USART1_IRQHandler
#  define UART_COMM_CLOCKEN    RCC_PERIPH_UART1
   DEBUG_CPP_MSG("Comms on USART1")
# endif

#else // HAVE_UART1
# define IS_UART1(_rx_pin_, _tx_pin_) (0)
# define IS_UART1_STRUCT(_p_) (0)
#endif // HAVE_UART1

//
// USART2
//
#if defined(USART2)
# define HAVE_UART2 1
#else
# define HAVE_UART2 0
#endif

#if HAVE_UART2
# if defined(PINID_UART2_RX) && PINID_UART2_RX > 0
#  define IS_UART2_RX_DEF(_p_) (PINID(_p_) == PINID_UART2_RX)
# else
#  define IS_UART2_RX_DEF(_p_) (0)
# endif
#if defined(PINID_UART2_RX_ALT2) && PINID_UART2_RX_ALT2 > 0
#  define IS_UART2_RX_ALT2(_p_) (PINID(_p_) == PINID_UART2_RX_ALT2)
# else
#  define IS_UART2_RX_ALT2(_p_) (0)
# endif
#if defined(PINID_UART2_RX_ALT) && PINID_UART2_RX_ALT > 0
#  define IS_UART2_RX_ALT1(_p_) (PINID(_p_) == PINID_UART2_RX_ALT)
# else
#  define IS_UART2_RX_ALT1(_p_) (0)
# endif
# define IS_UART2_RX(_p_) (IS_UART2_RX_DEF(_p_) || IS_UART2_RX_ALT1(_p_) || IS_UART2_RX_ALT2(_p_))

#if defined(PINID_UART2_TX) && PINID_UART2_TX > 0
#  define IS_UART2_TX_DEF(_p_) (PINID(_p_) == PINID_UART2_TX)
# else
#  define IS_UART2_TX_DEF(_p_) (0)
# endif
#if defined(PINID_UART2_TX_ALT2) && PINID_UART2_TX_ALT2 > 0
#  define IS_UART2_TX_ALT2(_p_) (PINID(_p_) == PINID_UART2_TX_ALT2)
# else
#  define IS_UART2_TX_ALT2(_p_) (0)
# endif
#if defined(PINID_UART2_TX_ALT) && PINID_UART2_TX_ALT > 0
#  define IS_UART2_TX_ALT1(_p_) (PINID(_p_) == PINID_UART2_TX_ALT)
# else
#  define IS_UART2_TX_ALT1(_p_) (0)
# endif
# define IS_UART2_TX(_p_) (IS_UART2_TX_DEF(_p_) || IS_UART2_TX_ALT1(_p_) || IS_UART2_TX_ALT2(_p_))

# define IS_UART2(_rxp_, _txp_) (IS_UART2_RX(_rxp_) && IS_UART2_TX(_txp_))
# define IS_UART2_STRUCT(_p_) (IS_UART2((_p_)->rx_pin, (_p_)->tx_pin))

# if uHAL_USE_UART_COMM && IS_UART2(UART_COMM_RX_PIN, UART_COMM_TX_PIN)
#  define UART_COMM_IRQn       USART2_IRQn
#  define UART_COMM_IRQHandler USART2_IRQHandler
#  define UART_COMM_CLOCKEN    RCC_PERIPH_UART2
   DEBUG_CPP_MSG("Comms on USART2")
# endif

#else // HAVE_UART2
# define IS_UART2(_rx_pin_, _tx_pin_) (0)
# define IS_UART2_STRUCT(_p_) (0)
#endif // HAVE_UART2

//
// USART3
//
#if defined(USART3)
# define HAVE_UART3 1
#else
# define HAVE_UART3 0
#endif

#if HAVE_UART3
# if defined(PINID_UART3_RX) && PINID_UART3_RX > 0
#  define IS_UART3_RX_DEF(_p_) (PINID(_p_) == PINID_UART3_RX)
# else
#  define IS_UART3_RX_DEF(_p_) (0)
# endif
#if defined(PINID_UART3_RX_ALT2) && PINID_UART3_RX_ALT2 > 0
#  define IS_UART3_RX_ALT2(_p_) (PINID(_p_) == PINID_UART3_RX_ALT2)
# else
#  define IS_UART3_RX_ALT2(_p_) (0)
# endif
#if defined(PINID_UART3_RX_ALT) && PINID_UART3_RX_ALT > 0
#  define IS_UART3_RX_ALT1(_p_) (PINID(_p_) == PINID_UART3_RX_ALT)
# else
#  define IS_UART3_RX_ALT1(_p_) (0)
# endif
# define IS_UART3_RX(_p_) (IS_UART3_RX_DEF(_p_) || IS_UART3_RX_ALT1(_p_) || IS_UART3_RX_ALT2(_p_))

#if defined(PINID_UART3_TX) && PINID_UART3_TX > 0
#  define IS_UART3_TX_DEF(_p_) (PINID(_p_) == PINID_UART3_TX)
# else
#  define IS_UART3_TX_DEF(_p_) (0)
# endif
#if defined(PINID_UART3_TX_ALT2) && PINID_UART3_TX_ALT2 > 0
#  define IS_UART3_TX_ALT2(_p_) (PINID(_p_) == PINID_UART3_TX_ALT2)
# else
#  define IS_UART3_TX_ALT2(_p_) (0)
# endif
#if defined(PINID_UART3_TX_ALT) && PINID_UART3_TX_ALT > 0
#  define IS_UART3_TX_ALT1(_p_) (PINID(_p_) == PINID_UART3_TX_ALT)
# else
#  define IS_UART3_TX_ALT1(_p_) (0)
# endif
# define IS_UART3_TX(_p_) (IS_UART3_TX_DEF(_p_) || IS_UART3_TX_ALT1(_p_) || IS_UART3_TX_ALT2(_p_))

# define IS_UART3(_rxp_, _txp_) (IS_UART3_RX(_rxp_) && IS_UART3_TX(_txp_))
# define IS_UART3_STRUCT(_p_) (IS_UART3((_p_)->rx_pin, (_p_)->tx_pin))

# if uHAL_USE_UART_COMM && IS_UART3(UART_COMM_RX_PIN, UART_COMM_TX_PIN)
#  define UART_COMM_IRQn       USART3_IRQn
#  define UART_COMM_IRQHandler USART3_IRQHandler
#  define UART_COMM_CLOCKEN    RCC_PERIPH_UART3
   DEBUG_CPP_MSG("Comms on USART3")
# endif

#else // HAVE_UART3
# define IS_UART3(_rx_pin_, _tx_pin_) (0)
# define IS_UART3_STRUCT(_p_) (0)
#endif // HAVE_UART3

//
// USART6
//
#if defined(USART6)
# define HAVE_UART6 1
#else
# define HAVE_UART6 0
#endif

#if HAVE_UART6
# if defined(PINID_UART6_RX) && PINID_UART6_RX > 0
#  define IS_UART6_RX_DEF(_p_) (PINID(_p_) == PINID_UART6_RX)
# else
#  define IS_UART6_RX_DEF(_p_) (0)
# endif
#if defined(PINID_UART6_RX_ALT2) && PINID_UART6_RX_ALT2 > 0
#  define IS_UART6_RX_ALT2(_p_) (PINID(_p_) == PINID_UART6_RX_ALT2)
# else
#  define IS_UART6_RX_ALT2(_p_) (0)
# endif
#if defined(PINID_UART6_RX_ALT) && PINID_UART6_RX_ALT > 0
#  define IS_UART6_RX_ALT1(_p_) (PINID(_p_) == PINID_UART6_RX_ALT)
# else
#  define IS_UART6_RX_ALT1(_p_) (0)
# endif
# define IS_UART6_RX(_p_) (IS_UART6_RX_DEF(_p_) || IS_UART6_RX_ALT1(_p_) || IS_UART6_RX_ALT2(_p_))

#if defined(PINID_UART6_TX) && PINID_UART6_TX > 0
#  define IS_UART6_TX_DEF(_p_) (PINID(_p_) == PINID_UART6_TX)
# else
#  define IS_UART6_TX_DEF(_p_) (0)
# endif
#if defined(PINID_UART6_TX_ALT2) && PINID_UART6_TX_ALT2 > 0
#  define IS_UART6_TX_ALT2(_p_) (PINID(_p_) == PINID_UART6_TX_ALT2)
# else
#  define IS_UART6_TX_ALT2(_p_) (0)
# endif
#if defined(PINID_UART6_TX_ALT) && PINID_UART6_TX_ALT > 0
#  define IS_UART6_TX_ALT1(_p_) (PINID(_p_) == PINID_UART6_TX_ALT)
# else
#  define IS_UART6_TX_ALT1(_p_) (0)
# endif
# define IS_UART6_TX(_p_) (IS_UART6_TX_DEF(_p_) || IS_UART6_TX_ALT1(_p_) || IS_UART6_TX_ALT2(_p_))

# define IS_UART6(_rxp_, _txp_) (IS_UART6_RX(_rxp_) && IS_UART6_TX(_txp_))
# define IS_UART6_STRUCT(_p_) (IS_UART6((_p_)->rx_pin, (_p_)->tx_pin))

# if uHAL_USE_UART_COMM && IS_UART6(UART_COMM_RX_PIN, UART_COMM_TX_PIN)
#  define UART_COMM_IRQn       USART6_IRQn
#  define UART_COMM_IRQHandler USART6_IRQHandler
#  define UART_COMM_CLOCKEN    RCC_PERIPH_UART6
   DEBUG_CPP_MSG("Comms on USART6")
# endif

#else // HAVE_UART6
# define IS_UART6(_rx_pin_, _tx_pin_) (0)
# define IS_UART6_STRUCT(_p_) (0)
#endif // HAVE_UART6

//
// UART4
//
#if defined(UART4)
# define HAVE_UART4 1
#else
# define HAVE_UART4 0
#endif

#if HAVE_UART4
# if defined(PINID_UART4_RX) && PINID_UART4_RX > 0
#  define IS_UART4_RX_DEF(_p_) (PINID(_p_) == PINID_UART4_RX)
# else
#  define IS_UART4_RX_DEF(_p_) (0)
# endif
#if defined(PINID_UART4_RX_ALT2) && PINID_UART4_RX_ALT2 > 0
#  define IS_UART4_RX_ALT2(_p_) (PINID(_p_) == PINID_UART4_RX_ALT2)
# else
#  define IS_UART4_RX_ALT2(_p_) (0)
# endif
#if defined(PINID_UART4_RX_ALT) && PINID_UART4_RX_ALT > 0
#  define IS_UART4_RX_ALT1(_p_) (PINID(_p_) == PINID_UART4_RX_ALT)
# else
#  define IS_UART4_RX_ALT1(_p_) (0)
# endif
# define IS_UART4_RX(_p_) (IS_UART4_RX_DEF(_p_) || IS_UART4_RX_ALT1(_p_) || IS_UART4_RX_ALT2(_p_))

#if defined(PINID_UART4_TX) && PINID_UART4_TX > 0
#  define IS_UART4_TX_DEF(_p_) (PINID(_p_) == PINID_UART4_TX)
# else
#  define IS_UART4_TX_DEF(_p_) (0)
# endif
#if defined(PINID_UART4_TX_ALT2) && PINID_UART4_TX_ALT2 > 0
#  define IS_UART4_TX_ALT2(_p_) (PINID(_p_) == PINID_UART4_TX_ALT2)
# else
#  define IS_UART4_TX_ALT2(_p_) (0)
# endif
#if defined(PINID_UART4_TX_ALT) && PINID_UART4_TX_ALT > 0
#  define IS_UART4_TX_ALT1(_p_) (PINID(_p_) == PINID_UART4_TX_ALT)
# else
#  define IS_UART4_TX_ALT1(_p_) (0)
# endif
# define IS_UART4_TX(_p_) (IS_UART4_TX_DEF(_p_) || IS_UART4_TX_ALT1(_p_) || IS_UART4_TX_ALT2(_p_))

# define IS_UART4(_rxp_, _txp_) (IS_UART4_RX(_rxp_) && IS_UART4_TX(_txp_))
# define IS_UART4_STRUCT(_p_) (IS_UART4((_p_)->rx_pin, (_p_)->tx_pin))

# if uHAL_USE_UART_COMM && IS_UART4(UART_COMM_RX_PIN, UART_COMM_TX_PIN)
#  define UART_COMM_IRQn       UART4_IRQn
#  define UART_COMM_IRQHandler UART4_IRQHandler
#  define UART_COMM_CLOCKEN    RCC_PERIPH_UART4
   DEBUG_CPP_MSG("Comms on UART4")
# endif

#else // HAVE_UART4
# define IS_UART4(_rx_pin_, _tx_pin_) (0)
# define IS_UART4_STRUCT(_p_) (0)
#endif // HAVE_UART4

//
// UART5
//
#if defined(UART5)
# define HAVE_UART5 1
#else
# define HAVE_UART5 0
#endif

#if HAVE_UART5
# if defined(PINID_UART5_RX) && PINID_UART5_RX > 0
#  define IS_UART5_RX_DEF(_p_) (PINID(_p_) == PINID_UART5_RX)
# else
#  define IS_UART5_RX_DEF(_p_) (0)
# endif
#if defined(PINID_UART5_RX_ALT2) && PINID_UART5_RX_ALT2 > 0
#  define IS_UART5_RX_ALT2(_p_) (PINID(_p_) == PINID_UART5_RX_ALT2)
# else
#  define IS_UART5_RX_ALT2(_p_) (0)
# endif
#if defined(PINID_UART5_RX_ALT) && PINID_UART5_RX_ALT > 0
#  define IS_UART5_RX_ALT1(_p_) (PINID(_p_) == PINID_UART5_RX_ALT)
# else
#  define IS_UART5_RX_ALT1(_p_) (0)
# endif
# define IS_UART5_RX(_p_) (IS_UART5_RX_DEF(_p_) || IS_UART5_RX_ALT1(_p_) || IS_UART5_RX_ALT2(_p_))

#if defined(PINID_UART5_TX) && PINID_UART5_TX > 0
#  define IS_UART5_TX_DEF(_p_) (PINID(_p_) == PINID_UART5_TX)
# else
#  define IS_UART5_TX_DEF(_p_) (0)
# endif
#if defined(PINID_UART5_TX_ALT2) && PINID_UART5_TX_ALT2 > 0
#  define IS_UART5_TX_ALT2(_p_) (PINID(_p_) == PINID_UART5_TX_ALT2)
# else
#  define IS_UART5_TX_ALT2(_p_) (0)
# endif
#if defined(PINID_UART5_TX_ALT) && PINID_UART5_TX_ALT > 0
#  define IS_UART5_TX_ALT1(_p_) (PINID(_p_) == PINID_UART5_TX_ALT)
# else
#  define IS_UART5_TX_ALT1(_p_) (0)
# endif
# define IS_UART5_TX(_p_) (IS_UART5_TX_DEF(_p_) || IS_UART5_TX_ALT1(_p_) || IS_UART5_TX_ALT2(_p_))

# define IS_UART5(_rxp_, _txp_) (IS_UART5_RX(_rxp_) && IS_UART5_TX(_txp_))
# define IS_UART5_STRUCT(_p_) (IS_UART5((_p_)->rx_pin, (_p_)->tx_pin))

# if uHAL_USE_UART_COMM && IS_UART5(UART_COMM_RX_PIN, UART_COMM_TX_PIN)
#  define UART_COMM_IRQn       UART5_IRQn
#  define UART_COMM_IRQHandler UART5_IRQHandler
#  define UART_COMM_CLOCKEN    RCC_PERIPH_UART5
   DEBUG_CPP_MSG("Comms on UART5")
# endif

#else // HAVE_UART5
# define IS_UART5(_rx_pin_, _tx_pin_) (0)
# define IS_UART5_STRUCT(_p_) (0)
#endif // HAVE_UART5

//
// UART7
//
#if defined(UART7)
# define HAVE_UART7 1
#else
# define HAVE_UART7 0
#endif

#if HAVE_UART7
# if defined(PINID_UART7_RX) && PINID_UART7_RX > 0
#  define IS_UART7_RX_DEF(_p_) (PINID(_p_) == PINID_UART7_RX)
# else
#  define IS_UART7_RX_DEF(_p_) (0)
# endif
#if defined(PINID_UART7_RX_ALT2) && PINID_UART7_RX_ALT2 > 0
#  define IS_UART7_RX_ALT2(_p_) (PINID(_p_) == PINID_UART7_RX_ALT2)
# else
#  define IS_UART7_RX_ALT2(_p_) (0)
# endif
#if defined(PINID_UART7_RX_ALT) && PINID_UART7_RX_ALT > 0
#  define IS_UART7_RX_ALT1(_p_) (PINID(_p_) == PINID_UART7_RX_ALT)
# else
#  define IS_UART7_RX_ALT1(_p_) (0)
# endif
# define IS_UART7_RX(_p_) (IS_UART7_RX_DEF(_p_) || IS_UART7_RX_ALT1(_p_) || IS_UART7_RX_ALT2(_p_))

#if defined(PINID_UART7_TX) && PINID_UART7_TX > 0
#  define IS_UART7_TX_DEF(_p_) (PINID(_p_) == PINID_UART7_TX)
# else
#  define IS_UART7_TX_DEF(_p_) (0)
# endif
#if defined(PINID_UART7_TX_ALT2) && PINID_UART7_TX_ALT2 > 0
#  define IS_UART7_TX_ALT2(_p_) (PINID(_p_) == PINID_UART7_TX_ALT2)
# else
#  define IS_UART7_TX_ALT2(_p_) (0)
# endif
#if defined(PINID_UART7_TX_ALT) && PINID_UART7_TX_ALT > 0
#  define IS_UART7_TX_ALT1(_p_) (PINID(_p_) == PINID_UART7_TX_ALT)
# else
#  define IS_UART7_TX_ALT1(_p_) (0)
# endif
# define IS_UART7_TX(_p_) (IS_UART7_TX_DEF(_p_) || IS_UART7_TX_ALT1(_p_) || IS_UART7_TX_ALT2(_p_))

# define IS_UART7(_rxp_, _txp_) (IS_UART7_RX(_rxp_) && IS_UART7_TX(_txp_))
# define IS_UART7_STRUCT(_p_) (IS_UART7((_p_)->rx_pin, (_p_)->tx_pin))

# if uHAL_USE_UART_COMM && IS_UART7(UART_COMM_RX_PIN, UART_COMM_TX_PIN)
#  define UART_COMM_IRQn       UART7_IRQn
#  define UART_COMM_IRQHandler UART7_IRQHandler
#  define UART_COMM_CLOCKEN    RCC_PERIPH_UART7
   DEBUG_CPP_MSG("Comms on UART7")
# endif

#else // HAVE_UART7
# define IS_UART7(_rx_pin_, _tx_pin_) (0)
# define IS_UART7_STRUCT(_p_) (0)
#endif // HAVE_UART7

//
// UART8
//
#if defined(UART8)
# define HAVE_UART8 1
#else
# define HAVE_UART8 0
#endif

#if HAVE_UART8
# if defined(PINID_UART8_RX) && PINID_UART8_RX > 0
#  define IS_UART8_RX_DEF(_p_) (PINID(_p_) == PINID_UART8_RX)
# else
#  define IS_UART8_RX_DEF(_p_) (0)
# endif
#if defined(PINID_UART8_RX_ALT2) && PINID_UART8_RX_ALT2 > 0
#  define IS_UART8_RX_ALT2(_p_) (PINID(_p_) == PINID_UART8_RX_ALT2)
# else
#  define IS_UART8_RX_ALT2(_p_) (0)
# endif
#if defined(PINID_UART8_RX_ALT) && PINID_UART8_RX_ALT > 0
#  define IS_UART8_RX_ALT1(_p_) (PINID(_p_) == PINID_UART8_RX_ALT)
# else
#  define IS_UART8_RX_ALT1(_p_) (0)
# endif
# define IS_UART8_RX(_p_) (IS_UART8_RX_DEF(_p_) || IS_UART8_RX_ALT1(_p_) || IS_UART8_RX_ALT2(_p_))

#if defined(PINID_UART8_TX) && PINID_UART8_TX > 0
#  define IS_UART8_TX_DEF(_p_) (PINID(_p_) == PINID_UART8_TX)
# else
#  define IS_UART8_TX_DEF(_p_) (0)
# endif
#if defined(PINID_UART8_TX_ALT2) && PINID_UART8_TX_ALT2 > 0
#  define IS_UART8_TX_ALT2(_p_) (PINID(_p_) == PINID_UART8_TX_ALT2)
# else
#  define IS_UART8_TX_ALT2(_p_) (0)
# endif
#if defined(PINID_UART8_TX_ALT) && PINID_UART8_TX_ALT > 0
#  define IS_UART8_TX_ALT1(_p_) (PINID(_p_) == PINID_UART8_TX_ALT)
# else
#  define IS_UART8_TX_ALT1(_p_) (0)
# endif
# define IS_UART8_TX(_p_) (IS_UART8_TX_DEF(_p_) || IS_UART8_TX_ALT1(_p_) || IS_UART8_TX_ALT2(_p_))

# define IS_UART8(_rxp_, _txp_) (IS_UART8_RX(_rxp_) && IS_UART8_TX(_txp_))
# define IS_UART8_STRUCT(_p_) (IS_UART8((_p_)->rx_pin, (_p_)->tx_pin))

# if uHAL_USE_UART_COMM && IS_UART8(UART_COMM_RX_PIN, UART_COMM_TX_PIN)
#  define UART_COMM_IRQn       UART8_IRQn
#  define UART_COMM_IRQHandler UART8_IRQHandler
#  define UART_COMM_CLOCKEN    RCC_PERIPH_UART8
   DEBUG_CPP_MSG("Comms on UART8")
# endif

#else // HAVE_UART8
# define IS_UART8(_rx_pin_, _tx_pin_) (0)
# define IS_UART8_STRUCT(_p_) (0)
#endif // HAVE_UART8
