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
// common_periph.h
// Platform-specific common header
// NOTES:
//    This is the STM32 peripheral bus definitions, broken out from the main
//    common.h file to avoid making a mess. It should only ever be included
//    from common.h.
//
//    A single value is used instead of separate values for the clock and enable
//    bits in order to avoid having to define and track both for each peripheral,
//    but this means having to choose between 64 bit values or losing access to
//    the high bits. At present the highest two bits are used to indicate the bus
//    which means both that any peripheral that's controlled by those bits can't
//    be enabled and that AHB2 and AHB3 aren't handled because there aren't
//    enough possible values. Neither of these are problems on STM32F1 devices
//    which have no AHB2 or AHB3 and nothing in the high two bits, and there
//    isn't anything important lost on other devices from what I can see but it
//    should be easy enough to convert to a 64-bit integer if required.
//
//    Not all possible values are defined, only those that I anticipate using.
//

//
// Registers
//
// The offset of the RCC bus designation bits
// If >30, common.h uses 64-bit values to track the peripheral
#if ! defined(RCC_BUS_OFFSET) || RCC_BUS_OFFSET <= 0
# undef RCC_BUS_OFFSET
# define RCC_BUS_OFFSET 30U
#endif
// Avoid using 0x00 as a value here, it's good for catching errors
#define RCC_BUS_MASK (0b011LLU << RCC_BUS_OFFSET)
#define RCC_BUS_APB1 (0x01LLU << RCC_BUS_OFFSET)
#define RCC_BUS_APB2 (0x02LLU << RCC_BUS_OFFSET)
#define RCC_BUS_AHB1 (0x03LLU << RCC_BUS_OFFSET)
// There's nothing interesting on these buses and there aren't enough bits
// to track them without bumping up to 64 either
#if RCC_BUS_OFFSET > 30
# undef RCC_BUS_MASK
# define RCC_BUS_MASK (0b111LLU << RCC_BUS_OFFSET)
# define RCC_BUS_AHB2 (0x04LLU << RCC_BUS_OFFSET)
# define RCC_BUS_AHB3 (0x05LLU << RCC_BUS_OFFSET)
#endif
//
// Peripheral enable/disable bits
//
// AHB1
#if ! HAVE_STM32F1_GPIO
# define RCC_PERIPH_GPIOA (RCC_BUS_AHB1 | RCC_AHB1ENR_GPIOAEN)
# define RCC_PERIPH_GPIOB (RCC_BUS_AHB1 | RCC_AHB1ENR_GPIOBEN)
# define RCC_PERIPH_GPIOC (RCC_BUS_AHB1 | RCC_AHB1ENR_GPIOCEN)
# define RCC_PERIPH_GPIOD (RCC_BUS_AHB1 | RCC_AHB1ENR_GPIODEN)
# define RCC_PERIPH_GPIOE (RCC_BUS_AHB1 | RCC_AHB1ENR_GPIOEEN)
# define RCC_PERIPH_GPIOF (RCC_BUS_AHB1 | RCC_AHB1ENR_GPIOFEN)
# define RCC_PERIPH_GPIOG (RCC_BUS_AHB1 | RCC_AHB1ENR_GPIOGEN)
# define RCC_PERIPH_GPIOH (RCC_BUS_AHB1 | RCC_AHB1ENR_GPIOHEN)
# define RCC_PERIPH_GPIOI (RCC_BUS_AHB1 | RCC_AHB1ENR_GPIOIEN)
# define RCC_PERIPH_GPIOJ (RCC_BUS_AHB1 | RCC_AHB1ENR_GPIOJEN)
# define RCC_PERIPH_GPIOK (RCC_BUS_AHB1 | RCC_AHB1ENR_GPIOKEN)
#endif
//
// APB1
#define RCC_PERIPH_TIM2  (RCC_BUS_APB1 | RCC_APB1ENR_TIM2EN)
#define RCC_PERIPH_TIM3  (RCC_BUS_APB1 | RCC_APB1ENR_TIM3EN)
#define RCC_PERIPH_TIM4  (RCC_BUS_APB1 | RCC_APB1ENR_TIM4EN)
#define RCC_PERIPH_TIM5  (RCC_BUS_APB1 | RCC_APB1ENR_TIM5EN)
#define RCC_PERIPH_TIM6  (RCC_BUS_APB1 | RCC_APB1ENR_TIM6EN)
#define RCC_PERIPH_TIM7  (RCC_BUS_APB1 | RCC_APB1ENR_TIM7EN)
#define RCC_PERIPH_TIM12 (RCC_BUS_APB1 | RCC_APB1ENR_TIM12EN)
#define RCC_PERIPH_TIM13 (RCC_BUS_APB1 | RCC_APB1ENR_TIM13EN)
#define RCC_PERIPH_TIM14 (RCC_BUS_APB1 | RCC_APB1ENR_TIM14EN)
#define RCC_PERIPH_SPI2  (RCC_BUS_APB1 | RCC_APB1ENR_SPI2EN)
#define RCC_PERIPH_SPI3  (RCC_BUS_APB1 | RCC_APB1ENR_SPI3EN)
#define RCC_PERIPH_UART2 (RCC_BUS_APB1 | RCC_APB1ENR_USART2EN)
#define RCC_PERIPH_UART3 (RCC_BUS_APB1 | RCC_APB1ENR_USART3EN)
#define RCC_PERIPH_UART4 (RCC_BUS_APB1 | RCC_APB1ENR_UART4EN)
#define RCC_PERIPH_UART5 (RCC_BUS_APB1 | RCC_APB1ENR_UART5EN)
// These are the high 2 bits and so unusable when RCC_BUS_OFFSET < 30
#if RCC_BUS_OFFSET > 30
# define RCC_PERIPH_UART7 (RCC_BUS_APB1 | RCC_APB1ENR_UART7EN)
# define RCC_PERIPH_UART8 (RCC_BUS_APB1 | RCC_APB1ENR_UART8EN)
#endif
#define RCC_PERIPH_I2C1  (RCC_BUS_APB1 | RCC_APB1ENR_I2C1EN)
#define RCC_PERIPH_I2C2  (RCC_BUS_APB1 | RCC_APB1ENR_I2C2EN)
#define RCC_PERIPH_I2C3  (RCC_BUS_APB1 | RCC_APB1ENR_I2C3EN)
#define RCC_PERIPH_PWR   (RCC_BUS_APB1 | RCC_APB1ENR_PWREN)
#define RCC_PERIPH_DAC   (RCC_BUS_APB1 | RCC_APB1ENR_DACEN)
#if defined(RCC_APB1ENR_BKPEN)
# define RCC_PERIPH_BKP  (RCC_BUS_APB1 | RCC_APB1ENR_BKPEN)
#endif
//
// APB2
#define RCC_PERIPH_ADC1  (RCC_BUS_APB2 | RCC_APB2ENR_ADC1EN)
#define RCC_PERIPH_ADC2  (RCC_BUS_APB2 | RCC_APB2ENR_ADC2EN)
#define RCC_PERIPH_ADC3  (RCC_BUS_APB2 | RCC_APB2ENR_ADC3EN)
#define RCC_PERIPH_TIM1  (RCC_BUS_APB2 | RCC_APB2ENR_TIM1EN)
#define RCC_PERIPH_TIM8  (RCC_BUS_APB2 | RCC_APB2ENR_TIM8EN)
#define RCC_PERIPH_TIM9  (RCC_BUS_APB2 | RCC_APB2ENR_TIM9EN)
#define RCC_PERIPH_TIM10 (RCC_BUS_APB2 | RCC_APB2ENR_TIM10EN)
#define RCC_PERIPH_TIM11 (RCC_BUS_APB2 | RCC_APB2ENR_TIM11EN)
#define RCC_PERIPH_SPI1  (RCC_BUS_APB2 | RCC_APB2ENR_SPI1EN)
#define RCC_PERIPH_SPI4  (RCC_BUS_APB2 | RCC_APB2ENR_SPI4EN)
#define RCC_PERIPH_SPI5  (RCC_BUS_APB2 | RCC_APB2ENR_SPI5EN)
#define RCC_PERIPH_SPI6  (RCC_BUS_APB2 | RCC_APB2ENR_SPI6EN)
#define RCC_PERIPH_UART1 (RCC_BUS_APB2 | RCC_APB2ENR_USART1EN)
#define RCC_PERIPH_UART6 (RCC_BUS_APB2 | RCC_APB2ENR_USART6EN)
#if HAVE_STM32F1_GPIO
# define RCC_PERIPH_AFIO  (RCC_BUS_APB2 | RCC_APB2ENR_AFIOEN)
# define RCC_PERIPH_GPIOA (RCC_BUS_APB2 | RCC_APB2ENR_IOPAEN)
# define RCC_PERIPH_GPIOB (RCC_BUS_APB2 | RCC_APB2ENR_IOPBEN)
# define RCC_PERIPH_GPIOC (RCC_BUS_APB2 | RCC_APB2ENR_IOPCEN)
# define RCC_PERIPH_GPIOD (RCC_BUS_APB2 | RCC_APB2ENR_IOPDEN)
# define RCC_PERIPH_GPIOE (RCC_BUS_APB2 | RCC_APB2ENR_IOPEEN)
# define RCC_PERIPH_GPIOF (RCC_BUS_APB2 | RCC_APB2ENR_IOPFEN)
# define RCC_PERIPH_GPIOG (RCC_BUS_APB2 | RCC_APB2ENR_IOPGEN)
#else
# define RCC_PERIPH_SYSCFG (RCC_BUS_APB2 | RCC_APB2ENR_SYSCFGEN)
#endif
