// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2022, 2023 svijsv                                          *
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
// cstrings.c
// Tools for dealing with C-style strings
// NOTES:
//
//
#include "cstrings.h"
#if ULIB_ENABLE_CSTRINGS

#include "ascii.h"
#include "debug.h"

#include <string.h>

bool cstring_eq(const char *s1, const char *s2) {
	ulib_assert(s1 != NULL);
	ulib_assert(s2 != NULL);

#if DO_CSTRING_SAFETY_CHECKS
	if ((s1 == NULL) || (s2 == NULL)) {
		return false;
	}
#endif
	return (strcmp(s1, s2) == 0);
}
bool cstring_eqn(const char *s1, const char *s2, uint_t n) {
	ulib_assert(s1 != NULL);
	ulib_assert(s2 != NULL);

#if DO_CSTRING_SAFETY_CHECKS
	if ((s1 == NULL) || (s2 == NULL)) {
		return false;
	}
#endif
	return (strncmp(s1, s2, n) == 0);
}
bool cstring_eqz(const char *s1, const char *s2) {
	ulib_assert(s1 != NULL);
	ulib_assert(s2 != NULL);

#if DO_CSTRING_SAFETY_CHECKS
	if ((s1 == NULL) || (s2 == NULL)) {
		return false;
	}
#endif

	const unsigned char *a = (const unsigned char *)s1;
	const unsigned char *b = (const unsigned char *)s2;

	while ((*a != 0) && (*b != 0)) {
		if (*a != *b) {
			return false;
		}
		++a, ++b;
	}
	return (*a == 0);
}
const char* cstring_eat_whitespace(const char *s) {
	ulib_assert(s != NULL);

#if DO_CSTRING_SAFETY_CHECKS
	if (s == NULL) {
		return NULL;
	}
#endif

	for (;;) {
		switch (*s) {
			case '\t':
			case '\r':
			case '\n':
			case ' ':
				++s;
				break;
			default:
				return (char *)s;
		}
	}

	return (char *)s;
}
const char* cstring_next_token(const char *cs, char sep) {
	ulib_assert(cs != NULL);
	ulib_assert(sep != 0);

#if DO_CSTRING_SAFETY_CHECKS
	if (sep == 0) {
		while (*cs != 0) {
			++cs;
		}
		return cs;
	}
	if (cs == NULL) {
		return NULL;
	}
#endif

	// Eat current token
	for (; ((*cs != sep) && (*cs != 0)); ++cs) {
		// Nothing to do in here
	}
	// Eat separators
	for (; (*cs == sep); ++cs) {
		// Nothing to do in here
	}

	return cs;
}
char *cstring_pop_token(char *input, char sep, uint_t *len) {
	uint_t l = 0;
	char *cs = input;

	ulib_assert(input != NULL);
	ulib_assert(sep != 0);

#if DO_CSTRING_SAFETY_CHECKS
	if (sep == 0) {
		while (*cs != 0) {
			++cs;
		}
		goto END;
	}
	if (input == NULL) {
		if (len != NULL) {
			*len = 0;
		}
		return NULL;
	}
#endif

	// Eat current token
	for (; ((*cs != sep) && (*cs != 0)); ++cs, ++l) {
		// Nothing to do in here
	}
	if (*cs == sep) {
		*cs = 0;
		++cs;
	}

END:
	if (len != NULL) {
		*len = l;
	}
	return cs;
}

const char* cstring_basename(const char *s) {
	const char *bn;
	static const char *dot = ".";

	// basename() accepts null pointers, and we're trying to be like basename().
	/*
	ulib_assert(s != NULL);

#if DO_CSTRING_SAFETY_CHECKS
	if (s == NULL) {
		return dot;
	}
#endif
	*/

	if ((s == NULL) || (s[0] == 0)) {
		return dot;
	}

	bn = strrchr(s, '/');
	if (bn == NULL) {
		bn = s;
	} else {
		++bn;
	}
	if (bn[0] == 0) {
		bn = dot;
	}

	return bn;
}
char *cstring_to_upper(char *s) {
	uint8_t *c;

	ulib_assert(s != NULL);

#if DO_CSTRING_SAFETY_CHECKS
	if (s == NULL) {
		return NULL;
	}
#endif

	for (c = (uint8_t *)s; *c != 0; ++c) {
		*c = ASCII_TO_UPPER(*c);
	}

	return s;
}
char *cstring_to_lower(char *s) {
	uint8_t *c;

	ulib_assert(s != NULL);

#if DO_CSTRING_SAFETY_CHECKS
	if (s == NULL) {
		return NULL;
	}
#endif

	for (c = (uint8_t *)s; *c != 0; ++c) {
		*c = ASCII_TO_LOWER(*c);
	}

	return s;
}
char *cstring_tr(char *s, char old, char new) {
	ulib_assert(s != NULL);
	ulib_assert(new != 0);

#if DO_CSTRING_SAFETY_CHECKS
	if ((s == NULL) || (new == 0)) {
		return s;
	}
#endif

	CSTRING_TR(s, old, new);
	return s;
}

uint_t cstring_from_uint(char *dest, uint_t size, uint_t src, uint_t base) {
	uint_t i = 0, w = 0;

	ulib_assert(dest != NULL);
	ulib_assert(size > 0);
	ulib_assert((base > 1) && (base <= 16));

#if DO_CSTRING_SAFETY_CHECKS
	if ((dest == NULL) || (size == 0) || (base <= 1) || (base > 16)) {
		return 0;
	}
#endif

	if (src == 0) {
		w = 1;
	} else {
		for (uint_t x = 1; x <= src; x *= base, ++w) {
			// Nothing to do here
		}
	}

	if (w < size) {
		i = w;
		dest[i] = 0;

		do {
			--i;
			uint_t d = src % base;
			dest[i] = (char )ASCII_FROM_XDIGIT(d);
			src /= base;
		} while ((src != 0));
	}

	return w;
}

#else
	// ISO C forbids empty translation units, this makes it happy.
	typedef int make_iso_compilers_happy;
#endif // ULIB_ENABLE_CSTRINGS
