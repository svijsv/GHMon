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
#ifndef _PLATFORM_LIBOPENCM3_SYSTEM_H
#define _PLATFORM_LIBOPENCM3_SYSTEM_H

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
# define BUTTON_IRQn       NVIC_EXTI0_IRQ
# define Button_IRQHandler exti0_isr

#elif GPIO_GET_PINNO(BUTTON_PIN) == 1
# define BUTTON_IRQn       NVIC_EXTI1_IRQ
# define Button_IRQHandler exti1_isr

#elif GPIO_GET_PINNO(BUTTON_PIN) == 2
# define BUTTON_IRQn       NVIC_EXTI2_IRQ
# define Button_IRQHandler exti2_isr

#elif GPIO_GET_PINNO(BUTTON_PIN) == 3
# define BUTTON_IRQn       NVIC_EXTI3_IRQ
# define Button_IRQHandler exti3_isr

#elif GPIO_GET_PINNO(BUTTON_PIN) == 4
# define BUTTON_IRQn       NVIC_EXTI4_IRQ
# define Button_IRQHandler exti4_isr

#elif (GPIO_GET_PINNO(BUTTON_PIN) >= 5) && (GPIO_GET_PINNO(BUTTON_PIN) <= 9)
# define BUTTON_IRQn       NVIC_EXTI9_5_IRQ
# define Button_IRQHandler exti9_5_isr

#elif (GPIO_GET_PINNO(BUTTON_PIN) >= 10) && (GPIO_GET_PINNO(BUTTON_PIN) <= 15)
# define BUTTON_IRQn       NVIC_EXTI15_10_IRQ
# define Button_IRQHandler exti15_10_isr

#else
# error "Can't determine user button"
#endif // User button


/*
* Types
*/


/*
* Variable declarations
*/
// Bus frequencies; defined in lib/stm32/f1/rcc.c in the libopencm3 source
extern uint32_t rcc_apb1_frequency;
extern uint32_t rcc_apb2_frequency;
extern uint32_t rcc_ahb_frequency;


/*
* Function prototypes (defined in system.c)
*/


/*
* Macros
*/


#endif // _PLATFORM_LIBOPENCM3_SYSTEM_H
#ifdef __cplusplus
 }
#endif
