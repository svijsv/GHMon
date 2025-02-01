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
// error.c
// Error code handling functions
// NOTES:
//
//
#include "error.h"
#if ULIB_ENABLE_ERROR && ULIB_ENABLE_ERROR_TRANSLATION

#include "debug.h"

#include <errno.h>

// There doesn't seem to be an actual 'unknown error' code for errno but we
// need something for when there's not valid conversion. This will *probably*
// work for most cases.
#define EUNKNOWN 0xFF

const char* err_to_cstring(err_t err) {
	switch (err) {
	case ERR_OK:
		return "OK";

	case ERR_ACCESS:
		return "Access denied";
	case ERR_PERM:
		return "Operation not permitted";
	case ERR_NODEV:
		return "No such device";
	case ERR_NOTSUP:
		return "Operation not supported";
	case ERR_IMPOSSIBLE:
		return "Operation not possible";

	case ERR_BADARG:
		return "Function passed a bad argument";
	case ERR_INIT:
		return "Resource not initialized";
	case ERR_BADFILE:
		return "Bad file identifier";
	case ERR_EXISTS:
		return "Resurce already exists";
	case ERR_NOENT:
		return "Resource does not exist";
	case ERR_NOTEMPTY:
		return "Resource not empty";
	case ERR_BADTYPE:
		return "Incorrect resource type";
	case ERR_STALE:
		return "Stale resource";
	case ERR_INUSE:
		return "Resource already in use";
	case ERR_NOTAVAIL:
		return "Resource not available";
	case ERR_LONGNAME:
		return "Name too long";

	case ERR_IO:
		return "Input/output error";
	case ERR_NOMEM:
		return "Not enough memory";
	case ERR_RETRY:
		return "Resource temporarily unavailable";
	case ERR_TIMEOUT:
		return "Operation timed out";
	case ERR_INTERRUPT:
		return "Operation interrupted";
	case ERR_DEADLOCK:
		return "Resource deadlock";
	case ERR_NETHOST:
		return "Network host error";
	case ERR_NET:
		return "Netowrk error";
	case ERR_EXHAUSTED:
		return "Resource exhausted";
	case ERR_NOMEDIUM:
		return "No medium found";

	case ERR_COMMTX:
		return "Transmission error";
	case ERR_COMMRX:
		return "Reception error";
	case ERR_PROTO:
		return "Protocol error";
	case ERR_CONNABORTED:
		return "Connection aborted";
	case ERR_CONNREFUSED:
		return "Connection refused";
	case ERR_CONNRESET:
		return "Connection reset";

	case ERR_RANGE:
		return "Result outside valid range";
	case ERR_LOOP:
		return "Too many levels of indirection";

	case ERR_UNKNOWN:
		return "Unknown error";
	}

	return "";
}

int err_to_errno(err_t err) {
	if (err < 0) {
		err = -err;
	}

	switch (err) {
	case ERR_OK:
		return 0;

	case ERR_ACCESS:
		return EACCES;
	case ERR_PERM:
		return EPERM;
	case ERR_NODEV:
		return ENODEV;
	case ERR_NOTSUP:
		return ENOTSUP;
	case ERR_IMPOSSIBLE:
		return EUNKNOWN;

	case ERR_BADARG:
		return EINVAL;
	case ERR_INIT:
		return EUNKNOWN;
	case ERR_BADFILE:
		return EBADF;
	case ERR_EXISTS:
		return EEXIST;
	case ERR_NOENT:
		return ENOENT;
	case ERR_NOTEMPTY:
		return ENOTEMPTY;
	case ERR_BADTYPE:
		return EUNKNOWN;
	case ERR_STALE:
		return ESTALE;
	case ERR_INUSE:
		return EADDRINUSE;
	case ERR_NOTAVAIL:
		return EADDRNOTAVAIL;
	case ERR_LONGNAME:
		return ENAMETOOLONG;

	case ERR_IO:
		return EIO;
	case ERR_NOMEM:
		return ENOMEM;
	case ERR_RETRY:
		return EAGAIN;
	case ERR_TIMEOUT:
		return ETIMEDOUT;
	case ERR_INTERRUPT:
		return EINTR;
	case ERR_DEADLOCK:
		return EDEADLOCK;
	case ERR_NETHOST:
		return EHOSTUNREACH;
	case ERR_NET:
		return ENETUNREACH;
	case ERR_EXHAUSTED:
		return EUNKNOWN;
	case ERR_NOMEDIUM:
		return ENOMEDIUM;

	case ERR_COMMTX:
		return ECOMM;
	case ERR_COMMRX:
		return EUNKNOWN;
	case ERR_PROTO:
		return EPROTO;
	case ERR_CONNABORTED:
		return ECONNABORTED;
	case ERR_CONNREFUSED:
		return ECONNREFUSED;
	case ERR_CONNRESET:
		return ECONNRESET;

	case ERR_RANGE:
		return ERANGE;
	case ERR_LOOP:
		return ELOOP;

	case ERR_UNKNOWN:
		return EUNKNOWN;
	}

	return EUNKNOWN;
}

err_t errno_to_err(int en) {
	if (en < 0) {
		en = -en;
	}

	switch (en) {
	case 0:
		return ERR_OK;

	case EACCES:
		return ERR_ACCESS;
	case EPERM:
		return ERR_PERM;
	case ENOTSUP:
	case ENOPROTOOPT:
	case EPROTONOSUPPORT:
	case ENOSYS:
		return ERR_NOTSUP;

	case EINVAL:
		return ERR_BADARG;
	case EBADF:
		return ERR_BADFILE;
	case EEXIST:
		return ERR_EXISTS;
	case EFAULT:
	case ENOENT:
		return ERR_NOENT;
	case ENOTEMPTY:
		return ERR_NOTEMPTY;
	case EISDIR:
	case ENOTDIR:
	case EMEDIUMTYPE:
		return ERR_BADTYPE;
	case ENODEV:
	case ENXIO:
		return ERR_NODEV;
	case ESTALE:
		return ERR_STALE;
	case EADDRINUSE:
		return ERR_INUSE;
	case EADDRNOTAVAIL:
		return ERR_NOTAVAIL;
	case ENAMETOOLONG:
		return ERR_LONGNAME;

	case EIO:
		return ERR_IO;
	case ENOMEM:
	case ENOBUFS:
		return ERR_NOMEM;
	case EAGAIN:
#if EWOULDBLOCK != EAGAIN
	case EWOULDBLOCK:
#endif
	case EALREADY:
	case EBUSY:
	case EINPROGRESS:
		return ERR_RETRY;
	case ETIME:
	case ETIMEDOUT:
		return ERR_TIMEOUT;
	case EINTR:
	case ECANCELED:
		return ERR_INTERRUPT;
	case EDEADLOCK:
#if EDEADLOCK != EDEADLK
	case EDEADLK:
#endif
		return ERR_DEADLOCK;
	case EHOSTDOWN:
	case EHOSTUNREACH:
		return ERR_NETHOST;
	case ENETDOWN:
	case ENETUNREACH:
	case ENONET:
		return ERR_NET;
	case ENFILE:
	case EMFILE:
	case ENOLCK:
	case ENOSPC:
		return ERR_EXHAUSTED;
	case ENOMEDIUM:
		return ERR_NOMEDIUM;

	case ECOMM:
		return ERR_COMMTX;
	case EPROTO:
		return ERR_PROTO;
	case ECONNABORTED:
		return ERR_CONNABORTED;
	case ECONNREFUSED:
		return ERR_CONNREFUSED;
	case ECONNRESET:
		return ERR_CONNRESET;

	case ERANGE:
		return ERR_RANGE;
	case ELOOP:
		return ERR_LOOP;
	}

	return ERR_UNKNOWN;
}

#else
	// ISO C forbids empty translation units, this makes it happy.
	typedef int make_iso_compilers_happy;
#endif // ULIB_ENABLE_ERROR && ULIB_ENABLE_ERROR_TRANSLATION
