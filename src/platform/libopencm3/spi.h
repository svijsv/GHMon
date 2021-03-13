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
#ifndef _PLATFORM_LIBOPENCM3_SPI_H
#define _PLATFORM_LIBOPENCM3_SPI_H

/*
* Includes
*/
#include "common.h"


#if USE_SPI

/*
* Static values
*/
// Target speed of the SPI bus; it will generally be somewhat higher due to
// limitations on choices available.
// Try to hit ~100KHz; speed's not overly important to us and the wiring isn't
// necessarily reliable.
#define SPI_SPEED 100000


//
// Handle SPIx
// SPI1 remapped
#if (SPIx_SCK_PIN == PB3) && (SPIx_MISO_PIN == PB4) && (SPIx_MOSI_PIN == PB5)
# define SPI1_DO_REMAP 1
#endif
//
// SPI1
#if ((SPIx_SCK_PIN == PA5) && (SPIx_MISO_PIN == PA6) && (SPIx_MOSI_PIN == PA7)) || SPI1_DO_REMAP
# define SPIx SPI1
# define SPIx_RCC RCC_SPI1
# define SPIx_RST RST_SPI1
# define SPIx_BUSFREQ rcc_apb2_frequency
//
// SPI2
#elif (SPIx_SCK_PIN == PB13) && (SPIx_MISO_PIN == PB14) && (SPIx_MOSI_PIN == PB15)
# define SPIx SPI2
# define SPIx_RCC RCC_SPI2
# define SPIx_RST RST_SPI2
# define SPIx_BUSFREQ rcc_apb1_frequency
//
#else
# error "Can't determine SPI peripheral"
#endif // SPIx


/*
* Types
*/


/*
* Variable declarations (defined in spi.c)
*/


/*
* Function prototypes (defined in spi.c)
*/
// Initialize the SPI peripheral
void spi_init(void);


/*
* Macros
*/


#endif // USE_SPI

#endif // _PLATFORM_LIBOPENCM3_SPI_H
#ifdef __cplusplus
 }
#endif
