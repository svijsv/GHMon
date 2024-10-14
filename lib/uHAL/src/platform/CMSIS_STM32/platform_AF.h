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
// platform_AF.h
// Platform-specific shim for the frontend
// NOTES:
//   This file is for defining alternate function pins without cluttering
//   platform.h and should only be included by platform.h
//
//   Listings of mappings can be found in the 'Pinout and pin description'
//   section of the datasheet and in the 'Alternate function IO' section
//   of the reference manual, with the former being more comprehensive
//
//   This isn't all the mappings
//
//   If the PWM timer-related mappings change, the tables used in
//   time_pwm_find_ch_fx_table.h need to be updated
//

#define GPIO_REMAP_DEFAULT 0
#define GPIO_REMAP_FULL    1
#define GPIO_REMAP_PARTIAL 2
#define GPIO_REMAP_PARTIAL_1 GPIO_REMAP_PARTIAL
#define GPIO_REMAP_PARTIAL_2 3

//
// Determine which peripherals to remap
#if HAVE_STM32F1_GPIO
# ifndef GPIO_REMAP_TIM1
#  define GPIO_REMAP_TIM1 0
# endif
//
// The default pins for timers 2 and 5 are the same, so we can get more PWM
// outputs by automatically remapping them
/*
# ifndef GPIO_REMAP_TIM2
#  if defined(TIM5) && defined(TIM2)
#   define GPIO_REMAP_TIM2 1
#  else
#   define GPIO_REMAP_TIM2 0
#  endif
# endif
*/
# ifndef GPIO_REMAP_TIM2
#  define GPIO_REMAP_TIM2 0
# endif
# ifndef GPIO_REMAP_TIM3
#  define GPIO_REMAP_TIM3 0
# endif
# ifndef GPIO_REMAP_TIM4
#  define GPIO_REMAP_TIM4 0
# endif
# ifndef GPIO_REMAP_TIM9
#  define GPIO_REMAP_TIM9 0
# endif
# ifndef GPIO_REMAP_TIM10
#  define GPIO_REMAP_TIM10 0
# endif
# ifndef GPIO_REMAP_TIM11
#  define GPIO_REMAP_TIM11 0
# endif
# ifndef GPIO_REMAP_TIM13
#  define GPIO_REMAP_TIM13 0
# endif
# ifndef GPIO_REMAP_TIM14
#  define GPIO_REMAP_TIM14 0
# endif
# ifndef GPIO_REMAP_UART1
#  define GPIO_REMAP_UART1 0
# endif
# ifndef GPIO_REMAP_UART2
#  define GPIO_REMAP_UART2 0
# endif
# ifndef GPIO_REMAP_UART3
#  define GPIO_REMAP_UART3 0
# endif
# ifndef GPIO_REMAP_SPI1
#  define GPIO_REMAP_SPI1 0
# endif
# ifndef GPIO_REMAP_I2C1
#  define GPIO_REMAP_I2C1 0
# endif
# ifndef GPIO_REMAP_PD01
#  if HAVE_GPIO_PORTD && uHAL_USE_INTERNAL_OSC
#   define GPIO_REMAP_PD01 1
#  else
#   define GPIO_REMAP_PD01 0
#  endif
# endif
# define DO_TIM1_GPIO_REMAP GPIO_REMAP_TIM1
# define DO_TIM2_GPIO_REMAP GPIO_REMAP_TIM2
# define DO_TIM3_GPIO_REMAP GPIO_REMAP_TIM3
# define DO_TIM4_GPIO_REMAP GPIO_REMAP_TIM4
# define DO_TIM9_GPIO_REMAP GPIO_REMAP_TIM9
# define DO_TIM10_GPIO_REMAP GPIO_REMAP_TIM10
# define DO_TIM11_GPIO_REMAP GPIO_REMAP_TIM11
# define DO_TIM13_GPIO_REMAP GPIO_REMAP_TIM13
# define DO_TIM14_GPIO_REMAP GPIO_REMAP_TIM14
# define DO_UART1_GPIO_REMAP GPIO_REMAP_UART1
# define DO_UART2_GPIO_REMAP GPIO_REMAP_UART2
# define DO_UART3_GPIO_REMAP GPIO_REMAP_UART3
# define DO_SPI1_GPIO_REMAP GPIO_REMAP_SPI1
# define DO_I2C1_GPIO_REMAP GPIO_REMAP_I2C1
# define DO_PD01_GPIO_REMAP GPIO_REMAP_PD01

#else // HAVE_STM32F1_GPIO
# define DO_TIM1_GPIO_REMAP 0
# define DO_TIM2_GPIO_REMAP 0
# define DO_TIM3_GPIO_REMAP 0
# define DO_TIM4_GPIO_REMAP 0
# define DO_TIM9_GPIO_REMAP 0
# define DO_TIM10_GPIO_REMAP 0
# define DO_TIM11_GPIO_REMAP 0
# define DO_TIM13_GPIO_REMAP 0
# define DO_TIM14_GPIO_REMAP 0
# define DO_UART1_GPIO_REMAP 0
# define DO_UART2_GPIO_REMAP 0
# define DO_UART3_GPIO_REMAP 0
# define DO_SPI1_GPIO_REMAP 0
# define DO_I2C1_GPIO_REMAP 0
# define DO_PD01_GPIO_REMAP 0
#endif

//
// For non-STM32F1 devices this is the value written to GPIOx_AFRH or GPIOx_AFRL
// registers to select the alternate function
#define GPIOAF0  0x00U
#define GPIOAF1  0x01U
#define GPIOAF2  0x02U
#define GPIOAF3  0x03U
#define GPIOAF4  0x04U
#define GPIOAF5  0x05U
#define GPIOAF6  0x06U
#define GPIOAF7  0x07U
#define GPIOAF8  0x08U
#define GPIOAF9  0x09U
#define GPIOAF10 0x0AU
#define GPIOAF11 0x0BU
#define GPIOAF12 0x0CU
#define GPIOAF13 0x0DU
#define GPIOAF14 0x0EU
#define GPIOAF15 0x0FU

#define GPIOAF_DEFAULT  GPIOAF0
#define GPIOAF_TIM1     GPIOAF1
#define GPIOAF_TIM2     GPIOAF1
#define GPIOAF_TIM3     GPIOAF2
#define GPIOAF_TIM4     GPIOAF2
#define GPIOAF_TIM5     GPIOAF2
#define GPIOAF_TIM8     GPIOAF3
#define GPIOAF_TIM9     GPIOAF3
#define GPIOAF_TIM10    GPIOAF3
#define GPIOAF_TIM11    GPIOAF3
#define GPIOAF_TIM12    GPIOAF9
#define GPIOAF_TIM13    GPIOAF9
#define GPIOAF_TIM14    GPIOAF9
#define GPIOAF_I2C1     GPIOAF4
#define GPIOAF_I2C2     GPIOAF4
// FIXME: enable this on devices that support it
//#define GPIOAF_I2C2_ALT GPIOAF9
#define GPIOAF_I2C3     GPIOAF4
// FIXME: enable this on devices that support it
//#define GPIOAF_I2C3_ALT GPIOAF9
#define GPIOAF_SPI1  GPIOAF5
#define GPIOAF_SPI2  GPIOAF5
#define GPIOAF_SPI3  GPIOAF6
// FIXME: enable this on devices that support it
//#define GPIOAF_SPI3_ALT GPIOAF5
#define GPIOAF_UART1 GPIOAF7
#define GPIOAF_UART2 GPIOAF7
#define GPIOAF_UART3 GPIOAF7
#define GPIOAF_UART4 GPIOAF8
#define GPIOAF_UART5 GPIOAF8
#define GPIOAF_UART6 GPIOAF8
#define GPIOAF_UART7 GPIOAF8
#define GPIOAF_UART8 GPIOAF8

//
// ADC
#define PINID_ADC0 PINID_A0
#define PINID_ADC1 PINID_A1
#define PINID_ADC2 PINID_A2
#define PINID_ADC3 PINID_A3
#define PINID_ADC4 PINID_A4
#define PINID_ADC5 PINID_A5
#define PINID_ADC6 PINID_A6
#define PINID_ADC7 PINID_A7
#define PINID_ADC8 PINID_B0
#define PINID_ADC9 PINID_B1
//
// SWDIO
#define PINID_SWDIO PINID_A13
#define PINID_SWCLK PINID_A14
//
// JTAG
#define PINID_JTMS   PINID_A13
#define PINID_JTCK   PINID_A14
#define PINID_JTDI   PINID_A15
#define PINID_JTDO   PINID_B3
#define PINID_JNTRST PINID_B4
//
// Timer 1
#if DO_TIM1_GPIO_REMAP == GPIO_REMAP_FULL
# define PINID_TIM1_CH1 PINID_E9
# define PINID_TIM1_CH2 PINID_E11
# define PINID_TIM1_CH3 PINID_E13
# define PINID_TIM1_CH4 PINID_E14
// The channel pins remain the same for partial remap
//#elif DO_TIM1_GPIO_REMAP == GPIO_REMAP_PARTIAL
#else
# define PINID_TIM1_CH1 PINID_A8
# define PINID_TIM1_CH2 PINID_A9
# define PINID_TIM1_CH3 PINID_A10
# define PINID_TIM1_CH4 PINID_A11
# if ! HAVE_STM32F1_GPIO
#  define PINID_TIM1_CH1_ALT PINID_E9
#  define PINID_TIM1_CH2_ALT PINID_E11
#  define PINID_TIM1_CH3_ALT PINID_E13
#  define PINID_TIM1_CH4_ALT PINID_E14
# endif
#endif
//
// Timer 2
#if DO_TIM2_GPIO_REMAP == GPIO_REMAP_FULL
# define PINID_TIM2_CH1 PINID_A15
# define PINID_TIM2_CH2 PINID_B3
# define PINID_TIM2_CH3 PINID_B10
# define PINID_TIM2_CH4 PINID_B11
#elif DO_TIM2_GPIO_REMAP == GPIO_REMAP_PARTIAL_1
# define PINID_TIM2_CH1 PINID_A15
# define PINID_TIM2_CH2 PINID_B3
# define PINID_TIM2_CH3 PINID_A2
# define PINID_TIM2_CH4 PINID_A3
#elif DO_TIM2_GPIO_REMAP == GPIO_REMAP_PARTIAL_2
# define PINID_TIM2_CH1 PINID_A0
# define PINID_TIM2_CH2 PINID_A1
# define PINID_TIM2_CH3 PINID_B10
# define PINID_TIM2_CH4 PINID_B11
#else
# define PINID_TIM2_CH1 PINID_A0
# define PINID_TIM2_CH2 PINID_A1
# define PINID_TIM2_CH3 PINID_A2
# define PINID_TIM2_CH4 PINID_A3
# if ! HAVE_STM32F1_GPIO
#  define PINID_TIM2_CH1_ALT  PINID_A15
#  define PINID_TIM2_CH1_ALT2 PINID_A5
#  define PINID_TIM2_CH2_ALT  PINID_B3
#  define PINID_TIM2_CH3_ALT  PINID_B10
#  define PINID_TIM2_CH4_ALT  PINID_B11
# endif
#endif
//
// Timer 3
#if DO_TIM3_GPIO_REMAP == GPIO_REMAP_FULL
# define PINID_TIM3_CH1 PINID_C6
# define PINID_TIM3_CH2 PINID_C7
# define PINID_TIM3_CH3 PINID_C8
# define PINID_TIM3_CH4 PINID_C9
#elif DO_TIM3_GPIO_REMAP == GPIO_REMAP_PARTIAL
# define PINID_TIM3_CH1 PINID_B4
# define PINID_TIM3_CH2 PINID_B5
# define PINID_TIM3_CH3 PINID_B0
# define PINID_TIM3_CH4 PINID_B1
#else
# define PINID_TIM3_CH1 PINID_A6
# define PINID_TIM3_CH2 PINID_A7
# define PINID_TIM3_CH3 PINID_B0
# define PINID_TIM3_CH4 PINID_B1
# if ! HAVE_STM32F1_GPIO
#  define PINID_TIM3_CH1_ALT  PINID_C6
#  define PINID_TIM3_CH1_ALT2 PINID_B4
#  define PINID_TIM3_CH2_ALT  PINID_C7
#  define PINID_TIM3_CH2_ALT2 PINID_B5
#  define PINID_TIM3_CH3_ALT  PINID_C8
#  define PINID_TIM3_CH4_ALT  PINID_C9
# endif
#endif
//
// Timer 4
#if DO_TIM4_GPIO_REMAP
# define PINID_TIM4_CH1 PINID_D12
# define PINID_TIM4_CH2 PINID_D13
# define PINID_TIM4_CH3 PINID_D14
# define PINID_TIM4_CH4 PINID_D15
#else
# define PINID_TIM4_CH1 PINID_B6
# define PINID_TIM4_CH2 PINID_B7
# define PINID_TIM4_CH3 PINID_B8
# define PINID_TIM4_CH4 PINID_B9
# if ! HAVE_STM32F1_GPIO
#  define PINID_TIM4_CH1_ALT PINID_D12
#  define PINID_TIM4_CH2_ALT PINID_D13
#  define PINID_TIM4_CH3_ALT PINID_D14
#  define PINID_TIM4_CH4_ALT PINID_D15
# endif
#endif
//
// Timer 5
#define PINID_TIM5_CH1 PINID_A0
#define PINID_TIM5_CH2 PINID_A1
#define PINID_TIM5_CH3 PINID_A2
#define PINID_TIM5_CH4 PINID_A3
#if ! HAVE_STM32F1_GPIO
# define PINID_TIM5_CH1_ALT PINID_H10
# define PINID_TIM5_CH2_ALT PINID_H11
# define PINID_TIM5_CH3_ALT PINID_H12
# define PINID_TIM5_CH4_ALT PINID_I0
#endif
//
// Timer 8
#define PINID_TIM8_CH1 PINID_C6
#define PINID_TIM8_CH2 PINID_C7
#define PINID_TIM8_CH3 PINID_C8
#define PINID_TIM8_CH4 PINID_C9
#if ! HAVE_STM32F1_GPIO
# define PINID_TIM8_CH1_ALT PINID_I5
# define PINID_TIM8_CH2_ALT PINID_I6
# define PINID_TIM8_CH3_ALT PINID_I7
# define PINID_TIM8_CH4_ALT PINID_I2
#endif
//
// Timer 9
#if DO_TIM9_GPIO_REMAP
# define PINID_TIM9_CH1 PINID_E5
# define PINID_TIM9_CH2 PINID_E6
#else
# define PINID_TIM9_CH1 PINID_A2
# define PINID_TIM9_CH2 PINID_A3
# if ! HAVE_STM32F1_GPIO
#  define PINID_TIM9_CH1_ALT PINID_E5
#  define PINID_TIM9_CH2_ALT PINID_E6
# endif
#endif
//
// Timer 10
#if DO_TIM10_GPIO_REMAP
# define PINID_TIM10_CH1 PINID_F6
#else
# define PINID_TIM10_CH1 PINID_B8
# if ! HAVE_STM32F1_GPIO
#  define PINID_TIM10_CH1_ALT PINID_F6
# endif
#endif
//
// Timer 11
#if DO_TIM11_GPIO_REMAP
# define PINID_TIM11_CH1 PINID_F7
#else
# define PINID_TIM11_CH1 PINID_B9
# if ! HAVE_STM32F1_GPIO
#  define PINID_TIM11_CH1_ALT PINID_F7
# endif
#endif
//
// Timer 12
#define PINID_TIM12_CH1 PINID_B14
#define PINID_TIM12_CH2 PINID_B15
#if ! HAVE_STM32F1_GPIO
# define PINID_TIM12_CH1_ALT PINID_H6
# define PINID_TIM12_CH2_ALT PINID_H9
#endif
//
// Timer 13
#if DO_TIM13_GPIO_REMAP
# define PINID_TIM13_CH1 PINID_F8
#else
# define PINID_TIM13_CH1 PINID_A6
# if ! HAVE_STM32F1_GPIO
#  define PINID_TIM13_CH1_ALT PINID_F8
# endif
#endif
//
// Timer 14
#if DO_TIM14_GPIO_REMAP
# define PINID_TIM14_CH1 PINID_F9
#else
# define PINID_TIM14_CH1 PINID_A7
# if ! HAVE_STM32F1_GPIO
#  define PINID_TIM14_CH1_ALT PINID_F9
# endif
#endif
//
// UART1
#if DO_UART1_GPIO_REMAP
# define PINID_UART1_CTS PINID_A11
# define PINID_UART1_RTS PINID_A12
# define PINID_UART1_TX  PINID_B6
# define PINID_UART1_RX  PINID_B7
# define PINID_UART1_CK  PINID_A8
#else
# define PINID_UART1_CTS PINID_A11
# define PINID_UART1_RTS PINID_A12
# define PINID_UART1_TX  PINID_A9
# define PINID_UART1_RX  PINID_A10
# define PINID_UART1_CK  PINID_A8
# if ! HAVE_STM32F1_GPIO
#  define PINID_UART1_TX_ALT PINID_B6
#  define PINID_UART1_RX_ALT PINID_B7
# endif
#endif
//
// UART2
#if DO_UART2_GPIO_REMAP
# define PINID_UART2_CTS PINID_D3
# define PINID_UART2_RTS PINID_D4
# define PINID_UART2_TX  PINID_D5
# define PINID_UART2_RX  PINID_D6
# define PINID_UART2_CK  PINID_D7
#else
# define PINID_UART2_CTS PINID_A0
# define PINID_UART2_RTS PINID_A1
# define PINID_UART2_TX  PINID_A2
# define PINID_UART2_RX  PINID_A3
# define PINID_UART2_CK  PINID_A4
# if HAVE_STM32F1_GPIO
#  define PINID_UART2_CTS_ALT PINID_D3
#  define PINID_UART2_RTS_ALT PINID_D4
#  define PINID_UART2_TX_ALT  PINID_D5
#  define PINID_UART2_RX_ALT  PINID_D6
#  define PINID_UART2_CK_ALT  PINID_D7
# endif
#endif
//
// UART3
#if DO_UART3_GPIO_REMAP == GPIO_REMAP_FULL
# define PINID_UART3_CTS PINID_D11
# define PINID_UART3_RTS PINID_D12
# define PINID_UART3_TX  PINID_D8
# define PINID_UART3_RX  PINID_D9
# define PINID_UART3_CK  PINID_D10
#elif DO_UART3_GPIO_REMAP == GPIO_REMAP_PARTIAL
# define PINID_UART3_CTS PINID_B13
# define PINID_UART3_RTS PINID_B14
# define PINID_UART3_TX  PINID_C10
# define PINID_UART3_RX  PINID_C11
# define PINID_UART3_CK  PINID_C12
#else
# define PINID_UART3_CTS PINID_B13
# define PINID_UART3_RTS PINID_B14
# define PINID_UART3_TX  PINID_B10
# define PINID_UART3_RX  PINID_B11
# define PINID_UART3_CK  PINID_B12
# if HAVE_STM32F1_GPIO
#  define PINID_UART3_TX_ALT  PINID_C10
#  define PINID_UART3_TX_ALT2 PINID_D8
#  define PINID_UART3_RX_ALT  PINID_C11
#  define PINID_UART3_RX_ALT2 PINID_D9
#  define PINID_UART3_CK_ALT  PINID_C12
#  define PINID_UART3_CK_ALT2 PINID_D10
#  define PINID_UART3_CTS_ALT PINID_D11
#  define PINID_UART3_RTS_ALT PINID_D12
# endif
#endif
//
// SPI1
#if DO_SPI1_GPIO_REMAP
# define PINID_SPI1_NSS  PINID_A15
# define PINID_SPI1_SCK  PINID_B3
# define PINID_SPI1_MISO PINID_B4
# define PINID_SPI1_MOSI PINID_B5
#else
# define PINID_SPI1_NSS  PINID_A4
# define PINID_SPI1_SCK  PINID_A5
# define PINID_SPI1_MISO PINID_A6
# define PINID_SPI1_MOSI PINID_A7
# if ! HAVE_STM32F1_GPIO
#  define PINID_SPI1_NSS_ALT  PINID_A15
#  define PINID_SPI1_SCK_ALT  PINID_B3
#  define PINID_SPI1_MISO_ALT PINID_B4
#  define PINID_SPI1_MOSI_ALT PINID_B5
# endif
#endif
//
// SPI2
#define PINID_SPI2_NSS  PINID_B12
#define PINID_SPI2_SCK  PINID_B13
#define PINID_SPI2_MISO PINID_B14
#define PINID_SPI2_MOSI PINID_B15
#if ! HAVE_STM32F1_GPIO
# define PINID_SPI2_NSS_ALT   PINID_B9
# define PINID_SPI2_NSS_ALT2  PINID_I0
# define PINID_SPI2_NSS_ALT3  PINID_A9
# define PINID_SPI2_NSS_ALT4  PINID_D3

# define PINID_SPI2_SCK_ALT   PINID_B10
# define PINID_SPI2_SCK_ALT2  PINID_I1

# define PINID_SPI2_MISO_ALT  PINID_C2
# define PINID_SPI2_MISO_ALT2 PINID_I2

# define PINID_SPI2_MOSI_ALT  PINID_C1
# define PINID_SPI2_MOSI_ALT2 PINID_I3
#endif
//
// SPI3
#define PINID_SPI3_NSS  PINID_A15
#define PINID_SPI3_SCK  PINID_B3
#define PINID_SPI3_MISO PINID_B4
#define PINID_SPI3_MOSI PINID_B5
#if ! HAVE_STM32F1_GPIO
# define PINID_SPI3_NSS_ALT  PINID_A4
# define PINID_SPI3_SCK_ALT  PINID_C10
# define PINID_SPI3_MISO_ALT PINID_C11
# define PINID_SPI3_MOSI_ALT PINID_C12
// This is a different GPIOAF code so ignore it for simplicity
//# define PINID_SPI3_MOSI_ALT2 PINID_D6
#endif
//
// SPI4
#define PINID_SPI4_NSS  PINID_E4
#define PINID_SPI4_SCK  PINID_E2
#define PINID_SPI4_MISO PINID_E5
#define PINID_SPI4_MOSI PINID_E6
#if ! HAVE_STM32F1_GPIO
# define PINID_SPI4_NSS_ALT  PINID_E11
# define PINID_SPI4_SCK_ALT  PINID_E12
# define PINID_SPI4_MISO_ALT PINID_E13
# define PINID_SPI4_MOSI_ALT PINID_E14
#endif
//
// SPI5
#define PINID_SPI5_NSS  PINID_F6
#define PINID_SPI5_SCK  PINID_F7
#define PINID_SPI5_MISO PINID_F8
#define PINID_SPI5_MOSI PINID_F9
#if ! HAVE_STM32F1_GPIO
# define PINID_SPI5_NSS_ALT  PINID_H5
# define PINID_SPI5_SCK_ALT  PINID_H6
# define PINID_SPI5_MISO_ALT PINID_H7
# define PINID_SPI5_MOSI_ALT PINID_F11
#endif
//
// SPI6
#define PINID_SPI6_NSS  PINID_G8
#define PINID_SPI6_SCK  PINID_G13
#define PINID_SPI6_MISO PINID_G12
#define PINID_SPI6_MOSI PINID_G14
//
// I2C1
#if DO_I2C1_GPIO_REMAP
# define PINID_I2C1_SCL   PINID_B8
# define PINID_I2C1_SDA   PINID_B9
# define PINID_I2C1_SMBAI PINID_B5
#else
# define PINID_I2C1_SCL   PINID_B6
# define PINID_I2C1_SDA   PINID_B7
# define PINID_I2C1_SMBAI PINID_B5
# if ! HAVE_STM32F1_GPIO
#  define PINID_I2C1_SCL_ALT PINID_B8
#  define PINID_I2C1_SDA_ALT PINID_B9
# endif
#endif
//
// I2C2
#define PINID_I2C2_SCL   PINID_B10
#define PINID_I2C2_SDA   PINID_B11
#define PINID_I2C2_SMBAI PINID_B12
#define PINID_I2C2_SDA_REMAP PINID_B3
#if ! HAVE_STM32F1_GPIO
// This is a different GPIOAF code and not present on all devices, simplest
// to ignore it...
//# define PINID_I2C2_SCL_ALT   PINID_B3
# define PINID_I2C2_SCL_ALT   PINID_F1
# define PINID_I2C2_SDA_ALT   PINID_F0
# define PINID_I2C2_SMBAI_ALT PINID_F2
# define PINID_I2C2_SCL_ALT2   PINID_H4
# define PINID_I2C2_SDA_ALT2   PINID_H5
# define PINID_I2C2_SMBAI_ALT2 PINID_H6
#endif
//
// I2C3
#define PINID_I2C3_SCL   PINID_A8
#define PINID_I2C3_SDA   PINID_B4
#define PINID_I2C3_SMBAI PINID_A9
#if ! HAVE_STM32F1_GPIO
// This is a different GPIOAF code and not present on all devices, simplest
// to ignore it...
//# define PINID_I2C3_SDA_ALT   PINID_B4
// This isn't present on all devices, simplest to ignore it...
//# define PINID_I2C3_SDA_ALT   PINID_C9
# define PINID_I2C3_SCL_ALT   PINID_H7
# define PINID_I2C3_SDA_ALT   PINID_H8
# define PINID_I2C3_SMBAI_ALT PINID_H9
#endif
