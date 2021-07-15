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
// SPI1 remapped
#if (PINID(SPI_SCK_PIN) == PIN_SPI1_REMAP_SCK) && (PINID(SPI_MISO_PIN) == PIN_SPI1_REMAP_MISO) && (PINID(SPI_MOSI_PIN) == PIN_SPI1_REMAP_MOSI)
# define SPI1_DO_REMAP 1
#endif

#if USE_STM32F1_GPIO
# define IS_SPI1_MISO ((PINID(SPI_MISO_PIN) == PIN_SPI1_MISO) || SPI1_DO_REMAP)
# define IS_SPI1_MOSI ((PINID(SPI_MOSI_PIN) == PIN_SPI1_MOSI) || SPI1_DO_REMAP)
# define IS_SPI1_SCK  ((PINID(SPI_SCK_PIN ) == PIN_SPI1_SCK ) || SPI1_DO_REMAP)
#else
# define IS_SPI1_MISO ((PINID(SPI_MISO_PIN) == PIN_SPI1_MISO) || (PINID(SPI_MISO_PIN) == PIN_SPI1_REMAP_MISO))
# define IS_SPI1_MOSI ((PINID(SPI_MOSI_PIN) == PIN_SPI1_MOSI) || (PINID(SPI_MOSI_PIN) == PIN_SPI1_REMAP_MOSI))
# define IS_SPI1_SCK  ((PINID(SPI_SCK_PIN ) == PIN_SPI1_SCK ) || (PINID(SPI_SCK_PIN) == PIN_SPI1_REMAP_SCK))
#endif
// SPI2 and SPI3 can be remapped, but the remapped pins aren't accessible
// on the bluepill
#define IS_SPI2_MISO ((PINID(SPI_MISO_PIN) == PIN_SPI2_MISO))
#define IS_SPI2_MOSI ((PINID(SPI_MOSI_PIN) == PIN_SPI2_MOSI))
#define IS_SPI2_SCK  ((PINID(SPI_SCK_PIN ) == PIN_SPI2_SCK ))
#define IS_SPI3_MISO ((PINID(SPI_MISO_PIN) == PIN_SPI3_MISO))
#define IS_SPI3_MOSI ((PINID(SPI_MOSI_PIN) == PIN_SPI3_MOSI))
#define IS_SPI3_SCK  ((PINID(SPI_SCK_PIN ) == PIN_SPI3_SCK ))

// SPI1
#if IS_SPI1_MISO && IS_SPI1_MOSI && IS_SPI1_SCK
# define SPIx SPI1
# define SPIx_CLOCKEN  RCC_PERIPH_SPI1
# define SPIx_AF       AF_SPI1

// SPI2
#elif IS_SPI2_MISO && IS_SPI2_MOSI && IS_SPI2_SCK
# define SPIx SPI2
# define SPIx_CLOCKEN  RCC_PERIPH_SPI2
# define SPIx_AF       AF_SPI2

// SPI3
// The SPI3 pins are actually the same as the SPI1 remap pins so those will
// take precedence
#elif IS_SPI3_MISO && IS_SPI3_MOSI && IS_SPI3_SCK
# define SPIx SPI3
# define SPIx_CLOCKEN  RCC_PERIPH_SPI3
# define SPIx_AF       AF_SPI3

#else
# error "Can't determine SPI peripheral"
#endif // SPIx

#if (SPIx_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_APB1
# define SPIx_BUSFREQ G_freq_PCLK1
#elif (SPIx_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_APB2
# define SPIx_BUSFREQ G_freq_PCLK2
#elif (SPIx_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_AHB1
# define SPIx_BUSFREQ G_freq_HCLK
#else
# error "Can't determine SPI bus clock"
#endif

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
