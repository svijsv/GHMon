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
//    This differs from malloc() in the rather important respect that the
//    allocated memory can not be conveniently free()d.
//
//    This will not get along well with malloc(), don't use both at the same
//    time.
//
#ifndef _ULIB_HALLOC_H
#define _ULIB_HALLOC_H

#include "src/configify.h"
#if ULIB_ENABLE_HALLOC

#include "types.h"

//
// Allocate a block of memory size bytes long
void* halloc(size_t size);
//
// Truncate the allocated memory to this address
void halloc_truncate(void *trunc_addr);
//
// Report how much memory is allocated in total
size_t halloc_total_allocated(void);
//
// Report the address of the next allocation
size_t halloc_next_addr(void);
//
// Report the address of the first allocation
size_t halloc_base_addr(void);


#endif // ULIB_ENABLE_HALLOC
#endif // _ULIB_HALLOC_H
