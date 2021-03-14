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
#if USE_SPI
// SPI1 remapped
#if (SPIx_SCK_PIN == PB3) && (SPIx_MISO_PIN == PB4) && (SPIx_MOSI_PIN == PB5)
# define SPI1_DO_REMAP 1
#endif

// SPI1
#if ((SPIx_SCK_PIN == PA5) && (SPIx_MISO_PIN == PA6) && (SPIx_MOSI_PIN == PA7)) || SPI1_DO_REMAP
# define SPIx SPI1
# define SPIx_APBxENR  RCC->APB2ENR
# define SPIx_APBxRSTR RCC->APB2RSTR
# define SPIx_CLOCKEN  RCC_APB2ENR_SPI1EN
# define SPIx_BUSFREQ  G_freq_PCLK2

// SPI2
#elif (SPIx_SCK_PIN == PB13) && (SPIx_MISO_PIN == PB14) && (SPIx_MOSI_PIN == PB15)
# define SPIx SPI2
# define SPIx_APBxENR  RCC->APB1ENR
# define SPIx_APBxRSTR RCC->APB1RSTR
# define SPIx_CLOCKEN  RCC_APB1ENR_SPI2EN
# define SPIx_BUSFREQ  G_freq_PCLK1

#elif !defined(SPIx_SCK_PIN) && !defined(SPIx_MISO_PIN) && !defined(SPIx_MOSI_PIN)
# error "No SPI peripheral set"
#else
# error "Can't determine SPI peripheral"
#endif // SPIx
#endif // USE_SPI


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

#endif // _PLATFORM_CMSIS_SPI_H
#ifdef __cplusplus
 }
#endif
