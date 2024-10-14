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
// fifo_uint8.h
// Manage unsigned byte buffers
// NOTES:
//    Unless otherwise noted, functions returning fifo_uint8_t* return the same
//    object passed to them.
//
//
#ifndef _ULIB_FIFO_UINT8_H
#define _ULIB_FIFO_UINT8_H

#include "src/configify.h"
#if ULIB_ENABLE_FIFO_UINT8

#include "types.h"
#include "util.h"


// To simplify things, use an int that allows adding 1 to the maximum size
// without overflowing.
#if FIFO_UINT8_MAX_SIZE < 0xFFU
 typedef uint8_t fifo_uint8_len_t;
#elif FIFO_UINT8_MAX_SIZE < 0xFFFFU
 typedef uint16_t fifo_uint8_len_t;
#elif FIFO_UINT8_MAX_SIZE < 0xFFFFFFFFU
 typedef uint32_t fifo_uint8_len_t;
#elif FIFO_UINT8_MAX_SIZE < 0xFFFFFFFFFFFFFFFFU
 typedef uint64_t fifo_uint8_len_t;
#else
# error "FIFO_UINT8_MAX_SIZE is too big"
#endif

typedef struct {
#if ULIB_USE_STRUCT_ID
	struct_id_t id;
#endif
	// The bank of all bytes in the buffer.
	uint8_t *bank;
	// The size of the buffer.
	fifo_uint8_len_t size;
	// The number of slots used.
	fifo_uint8_len_t used;
	// The first slot used.
	fifo_uint8_len_t head;
	// The last slot used.
	fifo_uint8_len_t tail;
} fifo_uint8_t;

#define FIFO_IS_CIRCULAR_BUFFER 1
#define FIFO_SIZE 0

#define FIFO_LEN_T fifo_uint8_len_t
#define FIFO_BANK_T uint8_t
#define FIFO_OVERFLOW ((uint8_t )-1)
#define FIFO_UNDERFLOW ((uint8_t )-1)

#define FIFO_STRUCT_T fifo_uint8_t

#define FIFO_INIT    fifo_uint8_init
#define FIFO_PEEK    fifo_uint8_peek
#define FIFO_ENQUEUE fifo_uint8_enqueue
#define FIFO_DEQUEUE fifo_uint8_dequeue

#define DO_FIFO_SAFETY_CHECKS DO_FIFO_UINT8_SAFETY_CHECKS
#define ID_FIFO ID_FIFO_UINT8

#include "templates/fifo_buffer.h"

#if ! HAVE_FIFO_UINT8_C
# undef FIFO_IS_CIRCULAR_BUFFER
# undef FIFO_BANK_T
# undef FIFO_LEN_T
# undef FIFO_OVERFLOW
# undef FIFO_UNDERFLOW
# undef FIFO_STRUCT_T
# undef FIFO_SIZE
# undef FIFO_INIT
# undef FIFO_PEEK
# undef FIFO_ENQUEUE
# undef FIFO_DEQUEUE
# undef FIFO_SAFETY_CHECKS
# undef ID_FIFO
#endif

#endif // ULIB_ENABLE_FIFO_UINT8
#endif // _ULIB_FIFO_UINT8_H
