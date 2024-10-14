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
// platform_XXXX.h
// Platform-specific shim for the frontend
// NOTES:
//   This file is for individual MCU-specific configuration to avoid cluttering
//   up platform.h and should only be included from there
//
#ifndef _PLATFORM_T402_XMEGA3_H
#define _PLATFORM_T402_XMEGA3_H

//
// Hardware bug work-around flags
//
// There's a bug where the CNT and CMP registers need to be accessed as 16-bit
// registers even in 8-bit PWM mode, this enables handling that scenario.
#define TCB_PWM_16BIT_REG_WORKAROUND 1

//
// ADC characteristics
//
#ifndef ADC_MAX
# define ADC_MAX 0x3FFU
#endif
// Under normal conditions the frequency needs to be between 50KHz and 1.5MHz
// to get the maximum resolutioni but with the internal 0.55V reference the
// maximum is 260KHz and with 8 bit values it's 2MHz
#define F_ADC_MIN   50000U
#if INTERNAL_VREF_mV <= 600
# define F_ADC_MAX 260000U
#elif ADC_MAX <= 0xFF
# define F_ADC_MAX 2000000U
#else
# define F_ADC_MAX 1500000U
#endif
//
// The number of ADCCLK cycles a normal ADC conversion requires
#define ADC_CONVERSION_CYCLES (13U)
//
// A default ADC sampling takes 2 ADC clock cycles
#define ADC_SAMPLEN_BASE 2U
//
// 5 bits for SAMPLEN in the SAMPCTRL register
#define ADC_SAMPLEN_MAX 0x1FU
//
// The start-up time when using the internal voltage reference is 22uS,
// it's listed in the Electrical Characteristics of the reference manual.
// Using a standard delay of 32 cycles should be enough for any frequency
// up to 1.45MHz and 64 cycles should be good up to 1.9MHz.
#define ADC_INITDLY ADC_INITDLY_DLY64_gc

//
// Alternate function pins
//
// Communication
//
// UART0
#if GPIO_REMAP_UART0
# define PINID_UART0_RX PINID_A2
# define PINID_UART0_TX PINID_A1
#else
# define PINID_UART0_RX PINID_A7
# define PINID_UART0_TX PINID_A6
#endif
//
// SPI0
#define PINID_SPI0_SCK  PINID_A3
#define PINID_SPI0_SS   PINID_A0
#if GPIO_REMAP_SPI0
# define PINID_SPI0_MISO PINID_A7
# define PINID_SPI0_MOSI PINID_A6
#else
# define PINID_SPI0_MISO PINID_A2
# define PINID_SPI0_MOSI PINID_A1
#endif
//
// I2C0
#define PINID_I2C0_SCL PINID_A2
#define PINID_I2C0_SDA PINID_A1
//
// Timer output/PWM
//
// TCA0
#if GPIO_REMAP_TCA0W0
# define PINID_TCA0_WO0 PINID_A7
// There's a bug listed in the errata where remapping WO0-WO2 also remaps the
// corresponding WO3-WO5 output and the WO3-WO5 remap bits are non-functional.
// We don't have an alternative output pin for WO3 but I suspect that it still
// gets 'remapped' to something internally - I haven't actually tested this
// though.
# warning "Due to a hardware bug, TCA0W3 may be unavailable if TCA0W0 is remapped on this device."
#else
# define PINID_TCA0_WO0 PINID_A3
#endif
#define PINID_TCA0_WO1 PINID_A1
#define PINID_TCA0_WO2 PINID_A2
#define PINID_TCA0_WO3 PINID_A3
#define PINID_TCA0_WO4 0
#define PINID_TCA0_WO5 0
//
// TCBx
#define PINID_TCB0_WO PINID_A6
//#define PINID_TCB1_WO PINID_
//#define PINID_TCB2_WO PINID_
//#define PINID_TCB3_WO PINID_
//
// ADC input
// Unused inputs need to be defined to allow the ADC pin lookup to work
#define PINID_ADC0_AIN0  PINID_A0
#define PINID_ADC0_AIN1  PINID_A1
#define PINID_ADC0_AIN2  PINID_A2
#define PINID_ADC0_AIN3  PINID_A3
#define PINID_ADC0_AIN4  0
#define PINID_ADC0_AIN5  0
#define PINID_ADC0_AIN6  PINID_A6
#define PINID_ADC0_AIN7  PINID_A7
#define PINID_ADC0_AIN8  0
#define PINID_ADC0_AIN9  0
#define PINID_ADC0_AIN10 0
#define PINID_ADC0_AIN11 0
#define PINID_ADC0_AIN12 0
#define PINID_ADC0_AIN13 0
#define PINID_ADC0_AIN14 0
#define PINID_ADC0_AIN15 0


#endif // _PLATFORM_T402_XMEGA3_H
