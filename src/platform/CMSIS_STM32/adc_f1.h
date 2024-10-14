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
// Manage the ADC peripheral, F1 device setup
// NOTES:
//
#ifndef _uHAL_PLATFORM_CMSIS_ADC_F1_H
#define _uHAL_PLATFORM_CMSIS_ADC_F1_H


#define ADC_PRESCALER_Pos (RCC_CFGR_ADCPRE_Pos)
#define ADC_PRESCALER_REG (RCC->CFGR)

#define ADC_TSVREFE     (ADC_CR2_TSVREFE)
#define ADC_TSVREFE_REG (ADCx->CR2)


#define VREF_CHANNEL 17U
#define TEMP_CHANNEL 16U
// Datasheet value "Avg_Slope" converted from mV to uV
#define TEMP_INT_SLOPE_uV 4300U
// Datasheet value "V25" converted from V to mV
#define TEMP_INT_T25_mV   1430U
// Temperature sensor startup time specified as Tstart in the datasheet
#define TEMP_START_TIME_uS 10U
// The recommended sample time for the internal temperature
// This is used for the internal voltage reference too, as they have the
// same minimum sampling time on all the devices I've checked
#define TEMP_SAMPLE_uS     17U

// ADC stabilization time is specified under Tstab in the datasheet
#define ADC_STAB_TIME_uS 1U

#define ADC_SAMPLE_TIME_1_5   0b000U //  1.5 cycles
#define ADC_SAMPLE_TIME_7_5   0b001U //  7.5 cycles
#define ADC_SAMPLE_TIME_13_5  0b010U // 13.5 cycles
#define ADC_SAMPLE_TIME_28_5  0b011U // 28.5 cycles
#define ADC_SAMPLE_TIME_41_5  0b100U // 41.5 cycles
#define ADC_SAMPLE_TIME_55_5  0b101U // 55.5 cycles
#define ADC_SAMPLE_TIME_71_5  0b110U // 71.5 cycles
#define ADC_SAMPLE_TIME_239_5 0b111U // 239.5 cycles

#if ADC_MAX != 0x0FFF
# error "Unsupported ADC_MAX, must be 0xFFF"
#endif
# define ADC_BIT_DEPTH 12U

#define ADC_CYCLES_PER_uS (G_freq_ADCCLK / 1000000U)
#if ! ADC_SAMPLE_uS
# undef ADC_SAMPLE_uS
# define ADC_SAMPLE_uS (0)
#endif

#if (ADC_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 1
# define ADC_SAMPLE_TIME ADC_SAMPLE_TIME_1_5
# define ADC_SAMPLE_CYCLES 2U
#elif (ADC_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 7
# define ADC_SAMPLE_TIME ADC_SAMPLE_TIME_7_5
# define ADC_SAMPLE_CYCLES 8U
#elif (ADC_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 13
# define ADC_SAMPLE_TIME ADC_SAMPLE_TIME_13_5
# define ADC_SAMPLE_CYCLES 14U
#elif (ADC_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 28
# define ADC_SAMPLE_TIME ADC_SAMPLE_TIME_28_5
# define ADC_SAMPLE_CYCLES 29U
#elif (ADC_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 41
# define ADC_SAMPLE_TIME ADC_SAMPLE_TIME_41_5
# define ADC_SAMPLE_CYCLES 42U
#elif (ADC_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 55
# define ADC_SAMPLE_TIME ADC_SAMPLE_TIME_55_5
# define ADC_SAMPLE_CYCLES 56U
#elif (ADC_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 71
# define ADC_SAMPLE_TIME ADC_SAMPLE_TIME_71_5
# define ADC_SAMPLE_CYCLES 72U
//#elif (ADC_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 239
#else
# define ADC_SAMPLE_TIME ADC_SAMPLE_TIME_239_5
# define ADC_SAMPLE_CYCLES 240U
#endif
// Per the reference manual, the time taken for each conversion is the sample
// time + 12.5 ADC clock cycles
// We're working with integers so we round the number of cycles up and the
// constant 12.5 down
#define ADC_SAMPLES_PER_S (G_freq_ADCCLK / (12U + ADC_SAMPLE_CYCLES))
//#define ADC_SAMPLES_PER_S (G_freq_ADCCLK / (ADC_BIT_DEPTH + ADC_SAMPLE_CYCLES))

#if (TEMP_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 1
# define TEMP_SAMPLE_TIME ADC_SAMPLE_TIME_1_5
#elif (TEMP_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 7
# define TEMP_SAMPLE_TIME ADC_SAMPLE_TIME_7_5
#elif (TEMP_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 13
# define TEMP_SAMPLE_TIME ADC_SAMPLE_TIME_13_5
#elif (TEMP_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 28
# define TEMP_SAMPLE_TIME ADC_SAMPLE_TIME_28_5
#elif (TEMP_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 41
# define TEMP_SAMPLE_TIME ADC_SAMPLE_TIME_41_5
#elif (TEMP_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 55
# define TEMP_SAMPLE_TIME ADC_SAMPLE_TIME_55_5
#elif (TEMP_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 71
# define TEMP_SAMPLE_TIME ADC_SAMPLE_TIME_71_5
//#elif (TEMP_SAMPLE_uS * ADC_CYCLES_PER_uS) <= 239
#else
# define TEMP_SAMPLE_TIME ADC_SAMPLE_TIME_239_5
#endif

#define SMPR1_MASK 0x00FFFFFFU
#define SMPR2_MASK 0x3FFFFFFFU


#endif // _uHAL_PLATFORM_CMSIS_ADC_F1_H
