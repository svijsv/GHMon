// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2024 svijsv                                                *
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
* along with this program. If not, see <http:// www.gnu.org/licenses/>.*
*                                                                      *
*                                                                      *
***********************************************************************/
// fifo_buffer.h
// Manage FIFO buffers
// NOTES:
//    This is a template. See src/fifo_uint8.c and include/fifo_uint8.h for
//    examples. To use it, first define the following macros:
//      FIFO_STRUCT_T: The structure containing the FIFO data.
//
//      FIFO_SIZE: If non-zero, use this as the size of all FIFO_STRUCT_T
//      banks instead of a .size field.
//
//      FIFO_BANK_T: The type contained in the FIFO_STRUCT_T bank.
//
//      FIFO_OVERFLOW: The value returned when attempting to write to a full
//      buffer.
//
//      FIFO_UNDERFLOW: The value returned when attempting to read from an empty
//      buffer.
//
//      FIFO_IS_CIRCULAR_BUFFER: If non-zero, adding to a full buffer will
//      overwrite the current head instead of returning FIFO_OVERFLOW.
//
//      FIFO_INIT: Initialize a FIFO_STRUCT_T structure
//
//      FIFO_DEQUEUE: The name of the function used to read a value and advance
//      the head.
//
//      FIFO_PEEK: The name of the function used to read a value without
//      advancing the head.
//
//      FIFO_ENQUEUE: The name of the function used to add a value and advance
//      the tail.
//
//      FIFO_SAFETY_CHECK: If non-zero, perform additional safety checks
//
//      FIFO_ID: If non-zero, initialize a struct_id_value_t member in FIFO_STRUCT_T
//    Then include the header and C files in the master header and C files.
//
//    Unless otherwise noted, functions returning FIFO_STRUCT_T* return the same
//    object passed to them.
//
//

//
// Initialize the known members of a FIFO_STRUCT_T structure
// 'size' is ignored if FIFO_SIZE is non-zero.
FIFO_STRUCT_T* FIFO_INIT(FIFO_STRUCT_T *q, FIFO_LEN_T size);
//
// Read a value without advancing the head.
FIFO_BANK_T FIFO_PEEK(FIFO_STRUCT_T *q);
//
// Read a value and advance the head.
FIFO_BANK_T FIFO_DEQUEUE(FIFO_STRUCT_T *q);
//
// Append a value and advance the tail.
FIFO_BANK_T FIFO_ENQUEUE(FIFO_STRUCT_T *q, FIFO_BANK_T value);
