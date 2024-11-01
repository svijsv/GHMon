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
// halloc.c
// Allocate memory on the heap
//
// NOTES:
//    https://www.nongnu.org/avr-libc/user-manual/malloc.html
//    https://visualgdb.com/documentation/embedded/stackheap/
//
#include "halloc.h"
#if ULIB_ENABLE_HALLOC

#include "debug.h"
#include "util.h"


#if !HALLOC_HEAP_START_ADDR
  extern char HALLOC_HEAP_START_LINKER_VAR;
# undef HALLOC_HEAP_START_ADDR
# define HALLOC_HEAP_START_ADDR (&HALLOC_HEAP_START_LINKER_VAR)
#endif

#if HALLOC_HEAP_END_ADDR > 0 && HALLOC_HEAP_CANARY != 0
# define HALLOC_REAL_HEAP_END_ADDR (HALLOC_HEAP_END_ADDR - 1U)
#else
# define HALLOC_REAL_HEAP_END_ADDR (HALLOC_HEAP_END_ADDR)
#endif

#if ! HALLOC_ADDR_ALIGNMENT
# undef HALLOC_ADDR_ALIGNMENT
# define HALLOC_ADDR_ALIGNMENT (sizeof(uintptr_t))
#endif

static const uintptr_t base_addr = (uintptr_t )HALLOC_HEAP_START_ADDR;
static       uintptr_t next_addr = (uintptr_t )HALLOC_HEAP_START_ADDR;

static void write_canary(void) {
	if (HALLOC_HEAP_CANARY != 0) {
		uint8_t *canary_addr = (uint8_t *)next_addr;
		canary_addr[0] = HALLOC_HEAP_CANARY;
	}
	return;
}
static bool canary_is_valid(void) {
	if (HALLOC_HEAP_CANARY != 0 && next_addr != base_addr) {
		uint8_t *canary_addr = (uint8_t *)next_addr;
		return (canary_addr[0] == HALLOC_HEAP_CANARY);
	}
	return true;
}

__attribute__((weak))
size_t ulib_get_stack_pointer_addr(void) {
	// To simplify things and keep this platform-agnostic C99, we take the
	// address of the most recently-allocated stack variable to determine where
	// the bottom of the stack is. This is imperfect, but OK for our purposes.
	size_t stack_ptr = (size_t )&stack_ptr;

	return stack_ptr;
}

void* halloc(size_t size) {
	void *ret_ptr = (void *)next_addr;
	uintptr_t block_end = (size % HALLOC_ADDR_ALIGNMENT);
	if (block_end != 0) {
		block_end = HALLOC_ADDR_ALIGNMENT - block_end;
	}
	block_end += (next_addr + size);

	if (DO_HALLOC_SAFETY_CHECKS) {
		if (HALLOC_HEAP_END_ADDR <= 0) {
			uintptr_t stack_ptr = ulib_get_stack_pointer_addr();

			if ((block_end + HALLOC_STACK_MARGIN) >= stack_ptr) {
				return NULL;
			}
		} else if (block_end > HALLOC_REAL_HEAP_END_ADDR) {
			return NULL;
		}

		if (!canary_is_valid()) {
			ulib_panic("Heap canary has been violated!");
		}
	}
	if (HALLOC_MEM_INIT_VALUE >= 0) {
		mem_init((void *)next_addr, HALLOC_MEM_INIT_VALUE, size);
	}
	next_addr = block_end;
	write_canary();

	return ret_ptr;
}

void halloc_truncate(void *trunc_addr) {
	uintptr_t block_end = (uintptr_t )trunc_addr;

	if (DO_HALLOC_SAFETY_CHECKS) {
		if (block_end >= next_addr) {
			return;
		}
		if (block_end < (uintptr_t )HALLOC_HEAP_START_ADDR) {
			block_end = (uintptr_t )HALLOC_HEAP_START_ADDR;
		}
	}

	next_addr = block_end;
	write_canary();

	return;
}

size_t halloc_total_allocated(void) {
	return (size_t )(next_addr - base_addr);
}
size_t halloc_base_addr(void) {
	return (size_t )(base_addr);
}
size_t halloc_next_addr(void) {
	return (size_t )(next_addr);
}

#else
	// ISO C forbids empty translation units, this makes it happy.
	typedef int make_iso_compilers_happy;
#endif // ULIB_ENABLE_HALLOC
