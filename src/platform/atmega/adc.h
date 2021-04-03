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
#ifndef _PLATFORM_ATMEGA_ADC_H
#define _PLATFORM_ATMEGA_ADC_H

/*
* Includes
*/
#include "common.h"


#if USE_ADC

/*
* Static values
*/


/*
* Types
*/


/*
* Variable declarations
*/
// Frequency of the ADC clock
extern uint32_t G_freq_ADC;


/*
* Function prototypes
*/
// Initialize the ADC peripheral
void adc_init(void);


/*
* Macros
*/


#endif // USE_ADC

#endif // _PLATFORM_ATMEGA_ADC_H
#ifdef __cplusplus
 }
#endif
