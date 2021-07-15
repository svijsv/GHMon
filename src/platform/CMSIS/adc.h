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
// adc.h
// Manage the ADC peripheral
// NOTES:
//   Prototypes for most of the related functions are in interface.h
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _PLATFORM_CMSIS_ADC_H
#define _PLATFORM_CMSIS_ADC_H

/*
* Includes
*/
#include "common.h"


#if USE_ADC

/*
* Static values
*/
//
// Handle ADCx
// The ADC is internal, there's no reason for anyone to change it
// Only ADC1 can read the internal voltage reference and temperature sensor
#define ADCx ADC1
#define ADCx_CLOCKEN RCC_PERIPH_ADC1
#if ! USE_STM32F1_ADC
# define ADCx_COMMON ADC1_COMMON
#endif

#if USE_STM32F1_ADC
# define ADC_PRESCALER_Pos RCC_CFGR_ADCPRE_Pos
# define ADC_PRESCALER_REG RCC->CFGR
#else
# define ADC_PRESCALER_Pos ADC_CCR_ADCPRE_Pos
# define ADC_PRESCALER_REG ADC->CCR
#endif
#define ADC_PRESCALER_Msk (0b11 << ADC_PRESCALER_Pos)
#define ADC_PRESCALER_2   (0b00 << ADC_PRESCALER_Pos)
#define ADC_PRESCALER_4   (0b01 << ADC_PRESCALER_Pos)
#define ADC_PRESCALER_6   (0b10 << ADC_PRESCALER_Pos)
#define ADC_PRESCALER_8   (0b11 << ADC_PRESCALER_Pos)

#if (ADCx_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_APB1
# define ADCx_BUSFREQ G_freq_PCLK1
#elif (ADCx_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_APB2
# define ADCx_BUSFREQ G_freq_PCLK2
#elif (ADCx_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_AHB1
# define ADCx_BUSFREQ G_freq_HCLK
#else
# error "Can't determine ADC bus clock"
#endif

/*
* Types
*/


/*
* Variable declarations (defined in adc.c)
*/


/*
* Function prototypes (defined in adc.c)
*/
// Initialize the ADC peripheral
void adc_init(void);


/*
* Macros
*/


#endif // USE_ADC

#endif // _PLATFORM_CMSIS_ADC_H
#ifdef __cplusplus
 }
#endif
