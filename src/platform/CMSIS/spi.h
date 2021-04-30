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
//
// Handle SPIx
#if USE_SPI
// SPI1 remapped
#if (PINID(SPI_SCK_PIN) == PINID_B3) && (PINID(SPI_MISO_PIN) == PINID_B4) && (PINID(SPI_MOSI_PIN) == PINID_B5)
# define SPI1_DO_REMAP 1
#endif

// SPI1
#if ((PINID(SPI_SCK_PIN) == PINID_A5) && (PINID(SPI_MISO_PIN) == PINID_A6) && (PINID(SPI_MOSI_PIN) == PINID_A7)) || SPI1_DO_REMAP
# define SPIx SPI1
# define SPIx_APBxENR  RCC->APB2ENR
# define SPIx_APBxRSTR RCC->APB2RSTR
# define SPIx_CLOCKEN  RCC_APB2ENR_SPI1EN
# define SPIx_BUSFREQ  G_freq_PCLK2

// SPI2
#elif (PINID(SPI_SCK_PIN) == PINID_B13) && (PINID(SPI_MISO_PIN) == PINID_B14) && (PINID(SPI_MOSI_PIN) == PINID_B15)
# define SPIx SPI2
# define SPIx_APBxENR  RCC->APB1ENR
# define SPIx_APBxRSTR RCC->APB1RSTR
# define SPIx_CLOCKEN  RCC_APB1ENR_SPI2EN
# define SPIx_BUSFREQ  G_freq_PCLK1

#elif !defined(SPI_SCK_PIN) && !defined(SPI_MISO_PIN) && !defined(SPI_MOSI_PIN)
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
