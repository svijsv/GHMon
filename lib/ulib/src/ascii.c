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
// ascii.c
// Tools for dealing with 7-bit ASCII characters
//
// NOTES:
//    Wish I knew all this earlier: https://danq.me/2024/07/21/ascii/
//
//    In experimentation, optimized builds produce smaller code when using
//    if {...} than switch {...} to check multiple ranges but they produce
//    larger code for a single continuous range (e.g. '0-9 && a-f' vs '0-9')
//    but the difference isn't always substantial
//
//
#include "ascii.h"
#if ULIB_ENABLE_ASCII

#include "debug.h"

#if DO_ASCII_SAFETY_CHECKS
# define ASCII_VALIDATE_INPUT(_c_) \
	do { \
		if (((_c_) & 0x80U) != 0) { \
			return false; \
		} \
	} while (0)
#else
# define ASCII_VALIDATE_INPUT(_c_) ((void )0U)
#endif

//
// These functions test codes
//
bool ascii_is_valid(uint_t c) {
	return (c < 0x80U);
}

bool ascii_is_cntrl(uint8_t c) {
	ASCII_VALIDATE_INPUT(c);

	return (c < 0x20U || c == 0x7FU);
}

bool ascii_is_print(uint8_t c) {
	ASCII_VALIDATE_INPUT(c);

	return (c >= 0x20U && c != 0x7FU);
}

bool ascii_is_graph(uint8_t c) {
	ASCII_VALIDATE_INPUT(c);

	return (c > 0x20U && c != 0x7FU);
}

bool ascii_is_space(uint8_t c) {
	ASCII_VALIDATE_INPUT(c);

	switch (c) {
	case ' ':
	case '\f':
	case '\n':
	case '\r':
	case '\t':
	case '\v':
		return true;
	}
	return false;
}

bool ascii_is_blank(uint8_t c) {
	ASCII_VALIDATE_INPUT(c);

	switch (c) {
	case ' ':
	case '\t':
		return true;
	}
	return false;
}

bool ascii_is_digit(uint8_t c) {
	ASCII_VALIDATE_INPUT(c);

	switch (c) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		return true;
	}
	return false;

	/*
	return ((c >= 'a' && c <= 'f') || (c >= '0' && c <= '9'));
	*/
}
bool ascii_is_xdigit(uint8_t c) {
	ASCII_VALIDATE_INPUT(c);

	/*
	switch (c | 0x20U) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
		return true;
	}
	return false;
	*/

	c |= 0x20U;
	return ((c >= 'a' && c <= 'f') || (c >= '0' && c <= '9'));
}

bool ascii_is_alpha(uint8_t c) {
	ASCII_VALIDATE_INPUT(c);

	c |= 0x20U;
	return (c >= 'a' && c <= 'z');
}

bool ascii_is_alnum(uint8_t c) {
	ASCII_VALIDATE_INPUT(c);

	c |= 0x20U;
	return ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'));
}

bool ascii_is_punct(uint8_t c) {
	ASCII_VALIDATE_INPUT(c);

	/*
	switch (c) {
	case 0x21U ... 0x2FU:
	case 0x3AU ... 0x40U:
	case 0x5BU ... 0x60U:
	case 0x7BU ... 0x7EU:
		return true;
	}
	return false;
	*/

	return (
		(c >= 0x21U && c <= 0x2FU) ||
		(c >= 0x3AU && c <= 0x40U) ||
		(c >= 0x5BU && c <= 0x60U) ||
		(c >= 0x7BU && c <= 0x7EU)
	);
}

bool ascii_is_lower(uint8_t c) {
	ASCII_VALIDATE_INPUT(c);

	return (c >= 'a' && c <= 'z');
}

bool ascii_is_upper(uint8_t c) {
	ASCII_VALIDATE_INPUT(c);

	return (c >= 'A' && c <= 'Z');
}

//
// These functions modify codes
//
uint8_t ascii_to_upper(uint8_t c) {
#if DO_ASCII_SAFETY_CHECKS
	if (c < 'a' || c > 'z') {
		return c;
	}
#endif

	return c ^ 0x20U;
}
uint8_t ascii_to_lower(uint8_t c) {
#if DO_ASCII_SAFETY_CHECKS
	if (c < 'A' || c > 'Z') {
		return c;
	}
#endif

	return c | 0x20U;
}

uint8_t ascii_to_digit(uint8_t c) {
#if DO_ASCII_SAFETY_CHECKS
	if (c < '0' || c > '9') {
		return 0;
	}
#endif

	return (c ^ 0x30U);
}
uint8_t ascii_from_digit(uint8_t c) {
#if DO_ASCII_SAFETY_CHECKS
	if (c > 9) {
		return '0';
	}
#endif

	return (c | 0x30U);
}

uint8_t ascii_to_xdigit(uint8_t c) {
#if DO_ASCII_SAFETY_CHECKS
	if ((c < '0' || c > '9') && (c < 'a' || c > 'f') && (c < 'A' || c > 'F'))  {
		return 0;
	}
#endif
	if (c >= '0' && c <= '9') {
		return (c ^ 0x30U);
	}

	c |= 0x20U;
	return (uint8_t )(0x0AU + (c - 'a'));

	/*
	c |= 0x20U;
	switch (c) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		return (c ^ 0x30U);

	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
		return (uint8_t )(0x0AU + (c - 'a'));
	}

	return c;
	*/
}

uint8_t ascii_from_xdigit(uint8_t c) {
#if DO_ASCII_SAFETY_CHECKS
	if (c > 0x0FU) {
		return '0';
	}
#endif
	if (c <= 9) {
		return (c | 0x30U);
	}

	return ('A' - 0x0AU) + c;
}

#else
	// ISO C forbids empty translation units, this makes it happy.
	typedef int make_iso_compilers_happy;
#endif // ULIB_ENABLE_ASCII
