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
// adc_f1.h
// Manage the ADC peripheral, Fx device setup
// NOTES:
//
#ifndef _uHAL_PLATFORM_CMSIS_ADC_Fx_H
#define _uHAL_PLATFORM_CMSIS_ADC_Fx_H


#define ADC_PRESCALER_Pos (ADC_CCR_ADCPRE_Pos)
#define ADC_PRESCALER_REG (ADC->CCR)

#define ADC_TSVREFE     (ADC_CCR_TSVREFE)
#define ADC_TSVREFE_REG (ADC->CCR)

#define VREF_CHANNEL 17U
#define TEMP_CHANNEL 18U
// Datasheet value "Avg_Slope" converted from mV to uV
#define TEMP_INT_SLOPE_uV  2500U
// Datasheet value "V25" converted from V to mV
#define TEMP_INT_T25_mV     760U
// Temperature sensor startup time specified as Tstart in the datasheet
#define TEMP_START_TIME_uS   10U
// The recommended sample time for the internal temperature
// Datasheet gives a minimum sample time of 10uS but no max
// This is used for the internal voltage reference too, as they have the
// same minimum sampling time on all the devices I've checked
#define TEMP_SAMPLE_uS       10U

// ADC stabilization time is specified under Tstab in the datasheet
#define ADC_STAB_TIME_uS 3U

#define ADC_SAMPLE_TIME_3   0b000U
#define ADC_SAMPLE_TIME_15  0b001U
#define ADC_SAMPLE_TIME_28  0b010U
#define ADC_SAMPLE_TIME_56  0b011U
#define ADC_SAMPLE_TIME_84  0b100U
#define ADC_SAMPLE_TIME_112 0b101U
#define ADC_SAMPLE_TIME_144 0b110U
#define ADC_SAMPLE_TIME_480 0b111U

#define ADC_CR1_RES_6  (0b11U << ADC_CR1_RES_Pos)
#define ADC_CR1_RES_8  (0b10U << ADC_CR1_RES_Pos)
#define ADC_CR1_RES_10 (0b01U << ADC_CR1_RES_Pos)
#define ADC_CR1_RES_12 (0b00U << ADC_CR1_RES_Pos)

#if ADC_MAX == 0x003F
# define ADC_RES ADC_CR1_RES_6
# define ADC_BIT_DEPTH 6U
#elif ADC_MAX == 0x00FF
# define ADC_RES ADC_CR1_RES_8
# define ADC_BIT_DEPTH 8U
#elif ADC_MAX == 0x3FF
# define ADC_RES ADC_CR1_RES_10
# define ADC_BIT_DEPTH 10U
#elif ADC_MAX == 0x0FFF
# define ADC_RES ADC_CR1_RES_12
# define ADC_BIT_DEPTH 12U
#else
# error "Unsupported ADC_MAX, must be 0x03F, 0x0FF, 0x3FF, or 0xFFF"
#endif

#define ADC_CYCLES_PER_uS (G_freq_ADCCLK / 1000000U)
#if ! ADC_SAMPLE_uS
# undef ADC_SAMPLE_uS
# define ADC_SAMPLE_uS (0)
#endif

#if (ADC_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 3
# define ADC_SAMPLE_TIME ADC_SAMPLE_TIME_3
# define ADC_SAMPLE_CYCLES 3U
#elif (ADC_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 15
# define ADC_SAMPLE_TIME ADC_SAMPLE_TIME_15
# define ADC_SAMPLE_CYCLES 15U
#elif (ADC_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 28
# define ADC_SAMPLE_TIME ADC_SAMPLE_TIME_28
# define ADC_SAMPLE_CYCLES 28U
#elif (ADC_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 56
# define ADC_SAMPLE_TIME ADC_SAMPLE_TIME_56
# define ADC_SAMPLE_CYCLES 56U
#elif (ADC_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 84
# define ADC_SAMPLE_TIME ADC_SAMPLE_TIME_84
# define ADC_SAMPLE_CYCLES 84U
#elif (ADC_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 112
# define ADC_SAMPLE_TIME ADC_SAMPLE_TIME_112
# define ADC_SAMPLE_CYCLES 112U
#elif (ADC_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 144
# define ADC_SAMPLE_TIME ADC_SAMPLE_TIME_144
# define ADC_SAMPLE_CYCLES 144U
//#elif (ADC_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 480
#else
# define ADC_SAMPLE_TIME ADC_SAMPLE_TIME_480
# define ADC_SAMPLE_CYCLES 480U
#endif
// Per the reference manual, the time taken for each conversion is the sample
// time + ADC_resolution_bits clock cycles
#define ADC_SAMPLES_PER_S (G_freq_ADCCLK / (ADC_BIT_DEPTH + ADC_SAMPLE_CYCLES))

#if (TEMP_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 3
# define TEMP_SAMPLE_TIME ADC_SAMPLE_TIME_3
#elif (TEMP_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 15
# define TEMP_SAMPLE_TIME ADC_SAMPLE_TIME_15
#elif (TEMP_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 28
# define TEMP_SAMPLE_TIME ADC_SAMPLE_TIME_28
#elif (TEMP_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 56
# define TEMP_SAMPLE_TIME ADC_SAMPLE_TIME_56
#elif (TEMP_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 84
# define TEMP_SAMPLE_TIME ADC_SAMPLE_TIME_84
#elif (TEMP_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 112
# define TEMP_SAMPLE_TIME ADC_SAMPLE_TIME_112
#elif (TEMP_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 144
# define TEMP_SAMPLE_TIME ADC_SAMPLE_TIME_144
//#elif (TEMP_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 480
#else
# define TEMP_SAMPLE_TIME ADC_SAMPLE_TIME_480
#endif

#define SMPR1_MASK 0x07FFFFFFU
#define SMPR2_MASK 0x3FFFFFFFU


#endif // _uHAL_PLATFORM_CMSIS_ADC_Fx_H
