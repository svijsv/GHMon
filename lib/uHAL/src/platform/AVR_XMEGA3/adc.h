// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021, 2024 svijsv                                          *
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
#ifndef _uHAL_PLATFORM_XMEGA3_ADC_H
#define _uHAL_PLATFORM_XMEGA3_ADC_H

#include "common.h"
#if uHAL_USE_ADC


//
// Initialize the ADC peripheral
void adc_init(void);


#endif // uHAL_USE_ADC
#endif // _uHAL_PLATFORM_XMEGA3_ADC_H
