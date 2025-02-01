// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2025 svijsv                                                *
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
#ifndef _ULIB_ERROR_H
#define _ULIB_ERROR_H

#include "src/configify.h"
#if ULIB_ENABLE_ERROR

//
// The type used to indicate errors
// The names in '()' following the description are similar errno codes if available.
typedef enum {
	ERR_OK = 0, ///< No error

	//
	// Permission errors:
	ERR_ACCESS,     ///< Access denied (EACCES).
	ERR_PERM,       ///< Operation not permitted (EPERM).
	ERR_NOTSUP,     ///< Operation not supported (ENOTSUP, ENOPROTOOPT, EPROTONOSUPPORT, ENOSYS).
	ERR_IMPOSSIBLE, ///< Operation not possible.
	//
	// Argument errors:
	ERR_BADARG,   ///< Function passed a bad argument (EINVAL).
	ERR_INIT,     ///< Resource not initialized.
	ERR_BADFILE,  ///< Bad file identifier/handle (EBADF).
	ERR_EXISTS,   ///< Resource already exists (EEXIST).
	ERR_NOENT,    ///< Resource does not exist (EFAULT, ENOENT).
	ERR_NOTEMPTY, ///< Resource not empty (ENOTEMPTY).
	ERR_BADTYPE,  ///< Resource is incorrect type (EISDIR, ENOTDIR, EMEDIUMTYPE).
	ERR_NODEV,    ///< No such device (ENODEV, ENXIO)
	ERR_STALE,    ///< Stale resource (ESTALE)
	ERR_INUSE,    ///< Resource already in use (EADDRINUSE)
	ERR_NOTAVAIL, ///< Resource not available (EADDRNOTAVAIL)
	ERR_LONGNAME, ///< Name too long (ENAMETOOLONG).
	//
	// Transient errors
	ERR_IO,        ///< Input/output error (EIO).
	ERR_NOMEM,     ///< Not enough memory (ENOMEM, ENOBUFS).
	ERR_RETRY,     ///< Device or resource temporarily unavailable (EAGAIN, EWOULDBLOCK, EALREADY, EBUSY, EINPROGRESS).
	ERR_TIMEOUT,   ///< Operation timed out (ETIME, ETIMEDOUT).
	ERR_INTERRUPT, ///< Operation interrupted (EINTR, ECANCELED).
	ERR_DEADLOCK,  ///< Resource deadlock (EDEADLK, EDEADLOCK).
	ERR_NETHOST,   ///< Network host error (EHOSTDOWN, EHOSTUNREACH).
	ERR_NET,       ///< Network error (ENETDOWN, ENETUNREACH, ENONET).
	ERR_EXHAUSTED, ///< Resource exhausted (ENFILE, EMFILE, ENOLCK, ENOSPC).
	ERR_NOMEDIUM,  ///< No medium found (ENOMEDIUM).
	//
	// Communication errors
	ERR_COMMTX,  ///< Communication error on send (ECOMM).
	ERR_COMMRX,  ///< Communication error on receive.
	ERR_PROTO,   ///< Protocol error (EPROTO).
	ERR_CONNABORTED, ///< Connection aborted (ECONNABORTED).
	ERR_CONNREFUSED, ///< Connection refused (ECONNREFUSED).
	ERR_CONNRESET,   ///< Connection reset (ECONNRESET).
	//
	// Range errors
	ERR_RANGE,  ///< Result outside valid range (EDOM, ERANGE, EOVERFLOW).
	ERR_LOOP,   ///< Too many levels of indirrection (ELOOP).

	ERR_UNKNOWN = 127 ///< Unknown error.
} err_t;

#if ULIB_ENABLE_ERROR_TRANSLATION
//
// Return a string describing the error code.
const char* err_to_cstring(err_t err);
//
// Try to translate an err_t code into an errno code.
int err_to_errno(err_t err);
//
// Try to translate an errno code into an err_t code.
err_t errno_to_err(int en);

#endif // ULIB_ENABLE_ERROR_TRANSLATION
#endif // ULIB_ENABLE_ERROR
#endif // _ULIB_ERROR_H
