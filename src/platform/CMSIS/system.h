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
#ifndef _PLATFORM_CMSIS_SYSTEM_H
#define _PLATFORM_CMSIS_SYSTEM_H

/*
* Includes
*/
#include "common.h"


/*
* Static values
*/
//
// Handle user button
#if ! BUTTON_PIN
	// Nothing to do here

#elif GPIO_GET_PINNO(BUTTON_PIN) == 0
# define BUTTON_IRQn       EXTI0_IRQn
# define Button_IRQHandler EXTI0_IRQHandler

#elif GPIO_GET_PINNO(BUTTON_PIN) == 1
# define BUTTON_IRQn       EXTI1_IRQn
# define Button_IRQHandler EXTI1_IRQHandler

#elif GPIO_GET_PINNO(BUTTON_PIN) == 2
# define BUTTON_IRQn       EXTI2_IRQn
# define Button_IRQHandler EXTI2_IRQHandler

#elif GPIO_GET_PINNO(BUTTON_PIN) == 3
# define BUTTON_IRQn       EXTI3_IRQn
# define Button_IRQHandler EXTI3_IRQHandler

#elif GPIO_GET_PINNO(BUTTON_PIN) == 4
# define BUTTON_IRQn       EXTI4_IRQn
# define Button_IRQHandler EXTI4_IRQHandler

#elif (GPIO_GET_PINNO(BUTTON_PIN) >= 5) && (GPIO_GET_PINNO(BUTTON_PIN) <= 9)
# define BUTTON_IRQn       EXTI9_5_IRQn
# define Button_IRQHandler EXTI9_5_IRQHandler

#elif (GPIO_GET_PINNO(BUTTON_PIN) >= 10) && (GPIO_GET_PINNO(BUTTON_PIN) <= 15)
# define BUTTON_IRQn       EXTI15_10_IRQn
# define Button_IRQHandler EXTI15_10_IRQHandler

#else
# error "Can't determine user button"
#endif // User button


/*
* Types
*/


/*
* Variable declarations (defined in system.c)
*/


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


#endif // _PLATFORM_CMSIS_SYSTEM_H
#ifdef __cplusplus
 }
#endif
