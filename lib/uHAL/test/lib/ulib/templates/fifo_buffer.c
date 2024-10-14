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
// fifo_buffer.c
// Manage FIFO buffers
// NOTES:
//   This should only be used by way of a master C file which defines the
//   necessary types. See the associated header for details.
//
//
#include "debug.h"

#if FIFO_SIZE
# define FIFO_GET_SIZE(_q_) FIFO_SIZE
#else
# define FIFO_GET_SIZE(_q_) ((_q_)->size)
#endif

//#define ADV_POS(_ptr_) ((_ptr_) %= FIFO_GET_SIZE(q))
#define ADV_POS(_ptr_) do { \
	++(_ptr_); \
	if ((_ptr_) == FIFO_GET_SIZE(q)) { \
		(_ptr_) = 0; \
	} \
} while (0)

FIFO_STRUCT_T* FIFO_INIT(FIFO_STRUCT_T *q, FIFO_LEN_T size) {
	ulib_assert(POINTER_IS_VALID(q));

#if DO_FIFO_SAFETY_CHECKS
	if (!POINTER_IS_VALID(q)) {
		return NULL;
	}
#endif

#if ULIB_USE_STRUCT_ID && defined(ID_FIFO)
	q->id.value = ID_FIFO;
#endif

	q->head = 0;
	q->tail = 0;
	q->used = 0;
#ifndef FIFO_GET_SIZE
	q->size = size;
#else
	UNUSED(size);
#endif

	return q;
}

FIFO_BANK_T FIFO_DEQUEUE(FIFO_STRUCT_T *q) {
	FIFO_BANK_T value;

	if (q->used == 0) {
		return FIFO_UNDERFLOW;
	}

	value = q->bank[q->head];
	ADV_POS(q->head);
	--q->used;

	return value;
}

FIFO_BANK_T FIFO_ENQUEUE(FIFO_STRUCT_T *q, FIFO_BANK_T value) {
#if FIFO_IS_CIRCULAR_BUFFER
	if (q->used == FIFO_GET_SIZE(q)) {
		return FIFO_OVERFLOW;
	}
#endif // FIFO_IS_CIRCULAR_BUFFER

	q->bank[q->tail] = value;
	ADV_POS(q->tail);

#if FIFO_IS_CIRCULAR_BUFFER
	++q->used;

#else // FIFO_IS_CIRCULAR_BUFFER
	if (q->used < FIFO_GET_SIZE(q)) {
		++q->used;
	} else {
		ADV_POS(q->head);
	}
#endif // FIFO_IS_CIRCULAR_BUFFER

	return value;
}

FIFO_BANK_T FIFO_PEEK(FIFO_STRUCT_T *q) {
	if (q->used == 0) {
		return FIFO_UNDERFLOW;
	}

	return q->bank[q->head];
}
