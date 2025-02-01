// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021, 2024 svijsv                                          *
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
/// @file
/// @brief Universal Asynchronous Receiver and Transmitter Interface
/// @note
///    This file should only be included by interface.h.
///

//
// This is defined in the device platform.h, it's included here for
// documentation purposes.
#if __HAVE_DOXYGEN__
///
/// The handle used manage UART ports.
typedef struct uart_port_t uart_port_t;
#endif

///
/// The structure used to specify UART port configuration.
typedef struct {
	uint32_t baud_rate; ///< The desired BAUD rate.
	gpio_pin_t rx_pin;  ///< The RX pin.
	gpio_pin_t tx_pin;  ///< The TX pin.
} uart_port_cfg_t;

#if uHAL_USE_UART_COMM || __HAVE_DOXYGEN__
///
/// The port used for the serial interface when enabled.
extern uart_port_t uHAL_uart_comm_port;
///
/// The port used for the serial interface when enabled.
# define UART_COMM_PORT (&uHAL_uart_comm_port)
#endif

///
/// Initialize a UART peripheral.
///
/// @note
/// The interface is always configured as 8 data bits with 1 stop bit and no
/// parity bit.
///
/// @param port The handle used to manage the port.
///  If the serial interface is enabled, passing NULL as the port will act on that.
///  Otherwise this must not be NULL.
/// @param conf A @c uart_port_cfg_t structure describing the interface.
///  Must not be NULL.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t uart_init_port(uart_port_t *port, const uart_port_cfg_t *conf);

///
/// Turn a UART peripheral on.
///
/// @param port The handle used to manage the port.
///  If the serial interface is enabled, passing NULL as the port will act on that.
///  Otherwise this must not be NULL.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t uart_on(const uart_port_t *port);

///
/// Turn a UART peripheral off.
///
/// @param port The handle used to manage the port.
///  If the serial interface is enabled, passing NULL as the port will act on that.
///  Otherwise this must not be NULL.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t uart_off(const uart_port_t *port);

///
/// Check if a UART peripheral is turned on.
///
/// @param port The handle used to manage the port.
///  If the serial interface is enabled, passing NULL as the port will act on that.
///  Otherwise this must not be NULL.
///
/// @retval true if turned on.
/// @retval false if turned off.
bool uart_is_on(const uart_port_t *port);

///
/// Receive a block of data
///
/// @param port The handle used to manage the port.
///  If the serial interface is enabled, passing NULL as the port will act on that.
///  Otherwise this must not be NULL.
/// @param buffer The buffer used to return data.
///  Must not be NULL.
/// @param size The number of bytes to receive.
///  Must be > 0.
/// @param timeout Abort if the operation takes more than this many milliseconds.
///  Must be > 0.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t uart_receive_block(uart_port_t *port, uint8_t *buffer, txsize_t size, utime_t timeout);

///
/// Transmit a block of data
///
/// @param port The handle used to manage the port.
///  If the serial interface is enabled, passing NULL as the port will act on that.
///  Otherwise this must not be NULL.
/// @param buffer The buffer holding the data to send.
///  Must not be NULL.
/// @param size The number of bytes to send from @c buffer.
///  Must be > 0.
/// @param timeout Abort if the operation takes more than this many milliseconds.
///  Must be > 0.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t uart_transmit_block(uart_port_t *port, const uint8_t *buffer, txsize_t size, utime_t timeout);

#if ENABLE_UART_LISTENING || __HAVE_DOXYGEN__
///
/// Turn the UART receive interrupt on.
///
/// @note
/// The interrupt is defined internally so that the received input can be placed
/// in the rx buffer, but the hook @c uart_rx_irq_hook() is called afterward.
///
/// @param port The handle used to manage the port.
///  If the serial interface is enabled, passing NULL as the port will act on that.
///  Otherwise this must not be NULL.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t uart_listen_on(const uart_port_t *port);

///
/// Turn the UART receive interrupt off.
///
/// @param port The handle used to manage the port.
///  If the serial interface is enabled, passing NULL as the port will act on that.
///  Otherwise this must not be NULL.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t uart_listen_off(const uart_port_t *port);

///
/// Check if the UART receive interrupt is enabled.
///
/// @param port The handle used to manage the port.
///  If the serial interface is enabled, passing NULL as the port will act on that.
///  Otherwise this must not be NULL.
///
/// @retval true if the UART port is listening.
/// @retval false if the UART port is not listening.
bool uart_is_listening(const uart_port_t *port);

///
/// Check if the UART port has received any input.
///
/// @param port The handle used to manage the port.
///  If the serial interface is enabled, passing NULL as the port will act on that.
///  Otherwise this must not be NULL.
///
/// @retval true if the UART port has data waiting for processing.
/// @retval false if the UART port doesn't have data waiting for processing.
bool uart_rx_is_available(const uart_port_t *port);

///
/// Overrideable hook called by UART ISRs when receiving data.
/// The default function does nothing.
///
/// @note
/// This function is overrideable.
///
/// @param port The handle used to manage the port.
///  If the serial interface is enabled, passing NULL as the port will act on that.
///  Otherwise this must not be NULL.
void uart_rx_irq_hook(uart_port_t *port);
#endif // ENABLE_UART_LISTENING
