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
// system.h
// General platform initialization
// NOTES:
//   Prototypes for some of the related functions are in interface.h
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _PLATFORM_STSTM32_SYSTEM_H
#define _PLATFORM_STSTM32_SYSTEM_H

/*
* Includes
*/
#include "config.h"
#include "stm32f103.h"


/*
* Static values
*/
// Oscillator frequencies, named to mesh with the bus frequency variables
// The reference manual gives a range of 30KHz-60KHz for the LSI
#define G_freq_LSI 40000
#define G_freq_HSI 8000000
#define G_freq_HSE HSE_VALUE
#define G_freq_LSE LSE_VALUE


/*
* Types
*/


/*
* Variable declarations (defined in system.c)
*/
// Bus frequencies
extern uint32_t G_freq_HCLK;
extern uint32_t G_freq_PCLK1;
extern uint32_t G_freq_PCLK2;


/*
* Function prototypes (defined in system.c)
*/
// Initialize/Enable/Disable one or more peripheral clocks
void clock_init(__IO uint32_t *en_reg, __IO uint32_t *rst_reg, uint32_t enable_mask);
void clock_enable(__IO uint32_t *reg, uint32_t enable_mask);
void clock_disable(__IO uint32_t *reg, uint32_t enable_mask);

// Enable/Disable writes to backup-domain registers
void BD_write_enable(void);
void BD_write_disable(void);

// Enable/Disable writes to RTC configuration registers
err_t RTC_cfg_enable(utime_t timeout);
err_t RTC_cfg_disable(utime_t timeout);


/*
* Macros
*/


#endif // _PLATFORM_STSTM32_SYSTEM_H
#ifdef __cplusplus
 }
#endif
