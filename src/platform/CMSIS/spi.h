// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021 svijsv                                                *
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
// spi.h
// Manage the SPI peripheral
// NOTES:
//   Prototypes for most of the related functions are in interface.h
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _PLATFORM_CMSIS_SPI_H
#define _PLATFORM_CMSIS_SPI_H

/*
* Includes
*/
#include "config.h"
#include "stm32f103.h"


/*
* Static values
*/
// Target speed of the SPI bus; it will generally be somewhat higher due to
// limitations on choices available.
// Try to hit ~100KHz; speed's not overly important to us and the wiring isn't
// necessarily reliable.
#define SPI_SPEED 100000


/*
* Types
*/


/*
* Variable declarations (defined in spi.c)
*/


/*
* Function prototypes (defined in spi.c)
*/
#if USE_SPI
// Initialize the SPI peripheral
void spi_init(void);
#else
#define spi_init() ((void )0U)
#endif


/*
* Macros
*/


#endif // _PLATFORM_CMSIS_SPI_H
#ifdef __cplusplus
 }
#endif
