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
#define ADCx_IS_ADC1 1
#define ADCx ADC1
#define RCC_APB2ENR_ADCxEN_Pos RCC_APB2ENR_ADC1EN_Pos
#define RCC_APB2ENR_ADCxEN     (1 << RCC_APB2ENR_ADC1EN_Pos)


/*
* Types
*/


/*
* Variable declarations (defined in adc.c)
*/
// Frequency of the ADC bus
extern uint32_t G_freq_ADC;


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
