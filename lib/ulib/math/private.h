// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021 svijsv
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
// private.h
// Module private members
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _LIB_MATH_PRIVATE_H
#define _LIB_MATH_PRIVATE_H

/*
* Includes
*/
#include "../math.h"
#include "../assert.h"


/*
* Static values
*/
// ln(2)
#define FIXEDP_LOGE_2Q31 0x58B90BFB // ln(2) * (1 << 31)
#define FIXEDP_LOGE_2 (FIXEDP_LOGE_2Q31 >> (31 - FIXEDP_FRACT_BITS))
// log10(2)
#define FIXEDP_LOG10_2Q31 0x268826A1 // log10(2) * (1 << 31)
#define FIXEDP_LOG10_2 (FIXEDP_LOG10_2Q31 >> (31 - FIXEDP_FRACT_BITS))


/*
* Types
*/


/*
* Variable declarations
*/


/*
* Function prototypes
*/


/*
* Macros
*/


#endif // _LIB_MATH_PRIVATE_H
#ifdef __cplusplus
 }
#endif
