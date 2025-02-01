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
/// @brief Two-Wire / Inter-Integrated Circuit Interface
/// @note
///    This file should only be included by interface.h.
///

///
/// Turn on the I2C peripheral.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t i2c_on(void);

///
/// Turn off the I2C peripheral.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t i2c_off(void);

///
/// Check if the I2C peripheral is turned on.
///
/// @retval true if turned on.
/// @retval false if turned off.
bool i2c_is_on(void);

///
/// Receive a data block in one transaction.
///
/// @param addr The address of the device to request the data from.
/// @param rx_buffer The bytes received.
///  Must not be NULL.
/// @param rx_size The number of bytes to receive.
///  Must be > 0.
/// @param timeout Abort if the operation takes more than this many milliseconds.
///  Must be > 0.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t i2c_receive_block(uint8_t addr, uint8_t *rx_buffer, txsize_t rx_size, utime_t timeout);

///
/// Begin reception of a data block in multiple transactions.
///
/// @param addr The address of the device to request the data from.
/// @param timeout Abort if the operation takes more than this many milliseconds.
///  Must be > 0.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t i2c_receive_block_begin(uint8_t addr, utime_t timeout);

///
/// Continue reception of a data block in multiple transactions.
///
/// Must call @c i2c_receive_block_begin() before this.
///
/// @c i2c_receive_block_continue() can be called as many times as needed.
///
/// @param rx_buffer The bytes received.
///  Must not be NULL.
/// @param rx_size The number of bytes to receive.
///  Must be > 0.
/// @param timeout Abort if the operation takes more than this many milliseconds.
///  Must be > 0.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t i2c_receive_block_continue(uint8_t *rx_buffer, txsize_t rx_size, utime_t timeout);

///
/// End reception of a data block in multiple transactions.
///
/// @param rx_byte The final received.
///  Must not be NULL.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t i2c_receive_block_end(uint8_t *rx_byte);

///
/// Transmit a data block in one transaction.
///
/// @param addr The address of the device to send data to.
/// @param tx_buffer The bytes to send.
///  Must not be NULL.
/// @param tx_size The number of bytes in @c tx_buffer.
///  Must be > 0.
/// @param timeout Abort if the operation takes more than this many milliseconds.
///  Must be > 0.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t i2c_transmit_block(uint8_t addr, const uint8_t *tx_buffer, txsize_t tx_size, utime_t timeout);

///
/// Begin transmission of a data block in multiple transactions.
///
/// @param addr The address of the device to request the data from.
/// @param timeout Abort if the operation takes more than this many milliseconds.
///  Must be > 0.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t i2c_transmit_block_begin(uint8_t addr, utime_t timeout);

///
/// Continue transmission of a data block in multiple transactions.
///
/// Must call @c i2c_transmit_block_begin() before this.
///
/// @c i2c_transmit_block_continue() can be called as many times as needed.
///
/// @param tx_buffer The bytes to transmit.
///  Must not be NULL.
/// @param tx_size The number of bytes to transmit.
///  Must be > 0.
/// @param timeout Abort if the operation takes more than this many milliseconds.
///  Must be > 0.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t i2c_transmit_block_continue(const uint8_t *tx_buffer, txsize_t tx_size, utime_t timeout);

///
/// End transmission of a data block in multiple transactions.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t i2c_transmit_block_end(void);
