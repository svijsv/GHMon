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
// platform_AF.h
// Platform-specific shim for the frontend
// NOTES:
//   This file is for defining alternate function pins without cluttering
//   platform.h and should only be included by platform.h
//
//   Listings of mappings can be found in the 'Pinout and pin description'
//   section of the datasheet and in the 'Alternate function IO' section
//   of the reference manual
//
//   This isn't all the mappings
//
// ADC
#define PIN_ADC0 PINID_A0
#define PIN_ADC1 PINID_A1
#define PIN_ADC2 PINID_A2
#define PIN_ADC3 PINID_A3
#define PIN_ADC4 PINID_A4
#define PIN_ADC5 PINID_A5
#define PIN_ADC6 PINID_A6
#define PIN_ADC7 PINID_A7
#define PIN_ADC8 PINID_B0
#define PIN_ADC9 PINID_B1
// SWDIO
#define PIN_SWDIO PINID_A13
#define PIN_SWCLK PINID_A14
// JTAG
#define PIN_JTMS   PINID_A13
#define PIN_JTCK   PINID_A14
#define PIN_JTDI   PINID_A15
#define PIN_JTDO   PINID_B3
#define PIN_JNTRST PINID_B4
// Timer 1
#define PINID_TIM1_CH1 PINID_A8
#define PINID_TIM1_CH2 PINID_A9
#define PINID_TIM1_CH3 PINID_A10
#define PINID_TIM1_CH4 PINID_A11
// Timer 2
#define PINID_TIM2_CH1 PINID_A0
#define PINID_TIM2_CH2 PINID_A1
#define PINID_TIM2_CH3 PINID_A2
#define PINID_TIM2_CH4 PINID_A3
// Timer 3
#define PINID_TIM3_CH1 PINID_A6
#define PINID_TIM3_CH2 PINID_A7
#define PINID_TIM3_CH3 PINID_B0
#define PINID_TIM3_CH4 PINID_B1
// Timer 4
#define PINID_TIM4_CH1 PINID_B6
#define PINID_TIM4_CH2 PINID_B7
#define PINID_TIM4_CH3 PINID_B8
#define PINID_TIM4_CH4 PINID_B9
// Timer 5
#define PINID_TIM5_CH1 PINID_A0
#define PINID_TIM5_CH2 PINID_A1
#define PINID_TIM5_CH3 PINID_A2
#define PINID_TIM5_CH4 PINID_A3
// USART1
#define PIN_UART1_CTS PINID_A11
#define PIN_UART1_RTS PINID_A12
#define PIN_UART1_TX  PINID_A9
#define PIN_UART1_RX  PINID_A10
#define PIN_UART1_CK  PINID_A8
#define PIN_UART1_REMAP_CTS PIN_UART1_CTS
#define PIN_UART1_REMAP_RTS PIN_UART1_RTS
#define PIN_UART1_REMAP_TX  PINID_B6
#define PIN_UART1_REMAP_RX  PINID_B7
#define PIN_UART1_REMAP_CK  PIN_UART1_CK
// USART2
#define PIN_UART2_CTS PINID_A0
#define PIN_UART2_RTS PINID_A1
#define PIN_UART2_TX  PINID_A2
#define PIN_UART2_RX  PINID_A3
#define PIN_UART2_CK  PINID_A4
// USART3
#define PIN_UART3_CTS PINID_B13
#define PIN_UART3_RTS PINID_B14
#define PIN_UART3_TX  PINID_B10
#define PIN_UART3_RX  PINID_B11
#define PIN_UART3_CK  PINID_B12
// SPI1
#define PIN_SPI1_NSS  PINID_A4
#define PIN_SPI1_SCK  PINID_A5
#define PIN_SPI1_MISO PINID_A6
#define PIN_SPI1_MOSI PINID_A7
#define PIN_SPI1_REMAP_NSS  PINID_A15
#define PIN_SPI1_REMAP_SCK  PINID_B3
#define PIN_SPI1_REMAP_MISO PINID_B4
#define PIN_SPI1_REMAP_MOSI PINID_B5
// SPI2
#define PIN_SPI2_NSS  PINID_B12
#define PIN_SPI2_SCK  PINID_B13
#define PIN_SPI2_MISO PINID_B14
#define PIN_SPI2_MOSI PINID_B15
// SPI3
#define PIN_SPI3_NSS  PINID_A15
#define PIN_SPI3_SCK  PINID_B3
#define PIN_SPI3_MISO PINID_B4
#define PIN_SPI3_MOSI PINID_B5
// I2C1
#define PIN_I2C1_SCL   PINID_B6
#define PIN_I2C1_SDA   PINID_B7
#define PIN_I2C1_SMBAI PINID_B5
#define PIN_I2C1_REMAP_SCL   PINID_B8
#define PIN_I2C1_REMAP_SDA   PINID_B9
#define PIN_I2C1_REMAP_SMBAI PIN_I2C1_SMBAI
// I2C2
#define PIN_I2C2_SCL   PINID_B10
#define PIN_I2C2_SDA   PINID_B11
#define PIN_I2C2_SMBAI PINID_B12
#define PIN_I2C2_SDA_REMAP PINID_B3
// I2C3
#define PIN_I2C3_SCL   PINID_A8
#define PIN_I2C3_SDA   PINID_B4
#define PIN_I2C3_SMBAI PINID_A9

