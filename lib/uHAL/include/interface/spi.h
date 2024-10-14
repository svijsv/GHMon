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
/// @brief Serial Peripheral Interface... Interface
/// @attention
///    This file should only be included by interface.h.
///

///
/// Turn on the SPI peripheral.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t spi_on(void);

///
/// Turn off the SPI peripheral.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t spi_off(void);

///
/// Check if the SPI peripheral is turned on
///
/// @retval true if turned on.
/// @retval false if turned off.
bool spi_is_on(void);

///
/// Exchange a byte.
///
/// @param tx The byte to transmit.
/// @param rx The byte received.
///  Must not be NULL.
/// @param timeout Abort if the operation takes more than this many milliseconds.
///  Must be > 0.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t spi_exchange_byte(uint8_t tx, uint8_t *rx, utime_t timeout);

///
/// Receive a data block
///
/// @param rx_buffer The bytes received.
///  Must not be NULL.
/// @param rx_size The number of bytes to receive.
///  Must be > 0.
/// @param tx The byte to transmit in parallel with each byte received.
///  Generally safe to use 0xFF as that keeps the output line held high.
/// @param timeout Abort if the operation takes more than this many milliseconds.
///  Must be > 0.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t spi_receive_block(uint8_t *rx_buffer, txsize_t rx_size, uint8_t tx, utime_t timeout);

///
/// Transmit a data block
///
/// @param tx_buffer The bytes to send.
///  Must not be NULL.
/// @param tx_size The number of bytes in @c tx_buffer.
///  Must be > 0.
/// @param timeout Abort if the operation takes more than this many milliseconds.
///  Must be > 0.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t spi_transmit_block(const uint8_t *tx_buffer, txsize_t tx_size, utime_t timeout);
