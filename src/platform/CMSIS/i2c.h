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
// i2c.h
// Manage the I2C peripheral
// NOTES:
//   Prototypes for most of the related functions are in interface.h
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _PLATFORM_CMSIS_I2C_H
#define _PLATFORM_CMSIS_I2C_H

/*
* Includes
*/
#include "common.h"


#if USE_I2C

/*
* Static values
*/
//
// Handle I2Cx
#if USE_I2C
//
// I2C1 remapped
#if (PINID(I2Cx_SCL_PIN) == PINID_B8) && (PINID(I2Cx_SDA_PIN) == PINID_B9)
# define I2C1_DO_REMAP 1
#endif
//
// I2C1
#if ((PINID(I2Cx_SCL_PIN) == PINID_B6) && (PINID(I2Cx_SDA_PIN) == PINID_B7)) || I2C1_DO_REMAP
# define I2Cx I2C1
# define I2Cx_APBxENR  (RCC->APB1ENR)
# define I2Cx_APBxRSTR (RCC->APB1RSTR)
# define I2Cx_CLOCKEN  RCC_APB1ENR_I2C1EN
# define I2Cx_BUSFREQ  G_freq_PCLK1
//
// I2C2
#elif (PINID(I2Cx_SCL_PIN) == PINID_B10) && (PINID(I2Cx_SDA_PIN) == PINID_B11)
# define I2Cx I2C2
# define I2Cx_APBxENR  (RCC->APB1ENR)
# define I2Cx_APBxRSTR (RCC->APB1RSTR)
# define I2Cx_CLOCKEN  RCC_APB1ENR_I2C2EN
# define I2Cx_BUSFREQ  G_freq_PCLK1

#else
# error "Can't determine I2C peripheral"
#endif // I2Cx
#endif // USE_I2C


/*
* Types
*/


/*
* Variable declarations (defined in i2c.c)
*/


/*
* Function prototypes (defined in i2c.c)
*/
// Initialize the I2C peripheral
void i2c_init(void);


/*
* Macros
*/


#endif // USE_I2C

#endif // _PLATFORM_CMSIS_I2C_H
#ifdef __cplusplus
 }
#endif