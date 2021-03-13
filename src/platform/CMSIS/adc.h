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
#include "stm32f103.h"


#if USE_ADC

/*
* Static values
*/


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

// Return the analog voltage reference value and the MCU's internal temperature
// sensor
// vref is measured in millivolts
// tempCx10 is measured in degrees celsius * 10
// The internal temperature sensor isn't very accurate; the reference manual
// claims it varies by 45C between chips due to process differences.
// If ADCx isn't ADC1, this will always return 3300V and 0C.
// Both arguments are mandatory.
//void adc_read_internals(int16_t *vref, int16_t *tempCx10);


/*
* Macros
*/


#endif // USE_ADC

#endif // _PLATFORM_CMSIS_ADC_H
#ifdef __cplusplus
 }
#endif
