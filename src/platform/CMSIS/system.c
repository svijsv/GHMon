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
// system.c
// General platform initialization
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
/*
* Includes
*/
#include "system.h"
#include "adc.h"
#include "gpio.h"
#include "spi.h"
#include "time.h"
#include "uart.h"
#include "i2c.h"


/*
* Static values
*/
#if defined STM32F1
# define EXTI_PREG AFIO
# define EXTI_PREG_CLOCKEN RCC_PERIPH_AFIO
# define FLASH_ACR_PRFTEN_Pos FLASH_ACR_PRFTBE_Pos
# define FLASH_ACR_PRFTEN     FLASH_ACR_PRFTBE
#else
# define EXTI_PREG SYSCFG
# define EXTI_PREG_CLOCKEN RCC_PERIPH_SYSCFG
# define RCC_BDCR_RTCSEL_NOCLOCK 0
# define RCC_BDCR_RTCSEL_LSE (0b01 << RCC_BDCR_RTCSEL_Pos)
# define RCC_BDCR_RTCSEL_LSI (0b10 << RCC_BDCR_RTCSEL_Pos)
# define RCC_BDCR_RTCSEL_HSE (0b11 << RCC_BDCR_RTCSEL_Pos)
#endif

// For STM32F1 use a flash latency of 0 for speeds <= 24MHz, 1 for
// 24-48MHz, and 2 for anything higher.
//
// For other STM32s, assuming 3.3V Vcc, the number of wait states varies
// by line but on the conservative end would be 0 for <= 25MHz and +1 for
// each additional 25MHz above that, or more commonly 0 for <= 30MHz and
// +1 for each additional 30MHz above that up to a line-dependendent maximum.
//
#if defined(STM32F1)
# define FLASH_WS_MAX 2
# define FLASH_WS_STEP 24000000
#elif defined(STM32F413xx) || defined(STM32F423xx)
# define FLASH_WS_MAX 6
# define FLASH_WS_STEP 25000000
#elif defined(STM32F401xC) || defined(STM32F401xE)
# define FLASH_WS_MAX 5
# define FLASH_WS_STEP 30000000
#elif defined(STM32F4)
# define FLASH_WS_MAX 6
# define FLASH_WS_STEP 30000000
#else
# error "Unsupported STM32 architecture"
#endif


/*
* Types
*/


/*
* Variables
*/
#if USE_TERMINAL
// Treat the first sleep as user-initiated to make entering the terminal
// more convenient
static bool button_wakeup = true;
#endif
static uint8_t bd_write_enabled = 0;


/*
* Local function prototypes
*/
static void light_sleep_ms(utime_t ms, uint8_t flags);
static void deep_sleep_s(utime_t s, uint8_t flags);
static void clocks_init(void);

static inline __attribute__((always_inline)) \
void sysflash(void);

/*
* Interrupt handlers
*/
#if BUTTON_PIN
OPTIMIZE_FUNCTION \
void Button_IRQHandler(void) {
	// The button shouldn't disable itself; there's no time when it shouldn't
	// at least let the user know the system is still running.
	// NVIC_DisableIRQ(BUTTON_IRQn);

	SET_BIT(G_IRQs, BUTTON_IRQf);

	// Let the user know we got the interrupt even if we're not going to do
	// anything for a while
	// Can't use led_flash(), we may not have delay_ms() if we're coming out of
	// hibernate_s() or sleep_ms().
	// led_flash(1, DELAY_ACK);
	sysflash();

	// The EXTI pending bit is cleared by writing 1
	SET_BIT(EXTI->PR, GPIO_GET_PINMASK(BUTTON_PIN));
	NVIC_ClearPendingIRQ(BUTTON_IRQn);

#if USE_TERMINAL
	button_wakeup = true;
#endif

	return;
}
#endif // BUTTON_PIN

/*
* Functions
*/
void platform_init(void) {
	clocks_init();

	// The AFIO clock is needed to fix a few problems, remap the inputs, and
	// to set up the button interrupt below, so enable it here and disable at
	// the end of setup when it's no longer needed
	clock_init(EXTI_PREG_CLOCKEN);

	gpio_init();
	// Violoate our separation of concerns in order to provide early user
	// feedback
	gpio_set_mode(LED_PIN, GPIO_MODE_PP, GPIO_LOW);
	// Can't led_flash() until time_init() has been called.
	// led_flash(1, DELAY_SHORT);

	time_init();
	led_flash(1, DELAY_SHORT);

#if USE_SERIAL
	uart_init();
	led_flash(1, DELAY_SHORT);
#endif

#if USE_SPI
	spi_init();
	led_flash(1, DELAY_SHORT);
#endif

#if USE_I2C
	i2c_init();
	led_flash(1, DELAY_SHORT);
#endif

#if USE_ADC
	adc_init();
	led_flash(1, DELAY_SHORT);
#endif

	// Set interrupt priority grouping
	// Use 4 bits for group priority and 0 bits for subpriority
	// See 4.4.5 (SCB_AIRCR register) of the Cortex M3 programming manual for
	// how to determine the value passed, it looks to be '3 + number of bits
	// for subpriority'. Cortex M4 is the same.
	NVIC_SetPriorityGrouping(0b011 + 0);

#if BUTTON_PIN
	uint32_t pmask, pinno;

	// Make sure the correct port is connected to the EXTI line
#if GPIO_GET_PORTNO(BUTTON_PIN) == GPIO_PORTA
	pmask = 0b0000;
#elif GPIO_GET_PORTNO(BUTTON_PIN) == GPIO_PORTB
	pmask = 0b0001;
#else
# error "Unsupported BUTTON_PIN port"
#endif
	pinno = GPIO_GET_PINNO(BUTTON_PIN);
#if GPIO_GET_PINNO(BUTTON_PIN) <= 3
	pinno = pinno*4;
	pmask <<= (pinno);
	MODIFY_BITS(EXTI_PREG->EXTICR[0], 0b1111 << pinno, pmask);
#elif GPIO_GET_PINNO(BUTTON_PIN) <= 7
	pinno = (pinno-4)*4;
	pmask <<= (pinno);
	MODIFY_BITS(EXTI_PREG->EXTICR[1], 0b1111 << pinno, pmask);
#elif GPIO_GET_PINNO(BUTTON_PIN) <= 11
	pinno = (pinno-8)*4;
	pmask <<= (pinno);
	MODIFY_BITS(EXTI_PREG->EXTICR[2], 0b1111 << pinno, pmask);
#elif GPIO_GET_PINNO(BUTTON_PIN) <= 15
	pinno = (pinno-12)*4;
	pmask <<= (pinno);
	MODIFY_BITS(EXTI_PREG->EXTICR[3], 0b1111 << pinno, pmask);
#else
# error "Unhandled BUTTON_PIN"
#endif // GPIO_GET_PINNO(BUTTON_PIN)

	// Configure the EXTI interrupt on the rising edge for the wakeup button
	gpio_set_mode(BUTTON_PIN, GPIO_MODE_IN, GPIO_BIAS_TO_STATE(BUTTON_PIN));
	SET_BIT(EXTI->IMR,  GPIO_GET_PINMASK(BUTTON_PIN));
	SET_BIT(EXTI->RTSR, GPIO_GET_PINMASK(BUTTON_PIN));
	NVIC_SetPriority(BUTTON_IRQn, BUTTON_IRQp);
	NVIC_EnableIRQ(BUTTON_IRQn);
#endif // BUTTON_PIN

	clock_disable(EXTI_PREG_CLOCKEN);

	return;
}
static inline __attribute__((always_inline)) \
void sysflash(void) {
	SET_GPIO_OUTPUT_HIGH(LED_PIN);
	// Hopefully ~100 ms counting overhead time @4MHz
	for (uint32_t i = 0; i < 100000; ++i) {
		// Count some clock cycles
		__asm__ volatile("" : "+g" (i) : :);
	}

	SET_GPIO_OUTPUT_LOW(LED_PIN);
	// Short delay to keep flashes from bleeding into each other
	// Hopefully ~20ms counting overhead time @4MHz
	for (uint32_t i = 0; i < 20000; ++i) {
		// Count some clock cycles
		__asm__ volatile("" : "+g" (i) : :);
	}
}
static void clocks_init(void) {
	uint32_t reg, latency;

	// Don't use clock source protection
	CLEAR_BIT(RCC->CR, RCC_CR_CSSON);

#if USE_INTERNAL_CLOCK
	SET_BIT(RCC->CR, RCC_CR_HSION);
	while (!BIT_IS_SET(RCC->CR, RCC_CR_HSIRDY)) {
		// Nothing to do here
	}

	MODIFY_BITS(RCC->CFGR, RCC_CFGR_SW,
		(0b00 << RCC_CFGR_SW_Pos )  | // Use HSI as SYSCLK source
		0);
	// Wait for HSI to become SYSCLK
	while (GATHER_BITS(RCC->CFGR, 0b11, RCC_CFGR_SWS_Pos) != 0b00) {
		// Nothing to do here
	}
	// Disable HSE
	CLEAR_BIT(RCC->CR, RCC_CR_HSEON);

#else // !USE_INTERNAL_CLOCK
	SET_BIT(RCC->CR, RCC_CR_HSEON);
	while (!BIT_IS_SET(RCC->CR, RCC_CR_HSERDY)) {
		// Nothing to do here
	}

	MODIFY_BITS(RCC->CFGR, RCC_CFGR_SW,
		(0b01 << RCC_CFGR_SW_Pos) | // Use HSE as SYSCLK source
		0);
	// Wait for HSE to become SYSCLK
	while (GATHER_BITS(RCC->CFGR, 0b11, RCC_CFGR_SWS_Pos) != 0b01) {
		// Nothing to do here
	}
	// Disable HSI
	CLEAR_BIT(RCC->CR, RCC_CR_HSION);
#endif // USE_INTERNAL_CLOCK

	reg = 0;
#if G_freq_HCLK == G_freq_OSC
	reg |= HCLK_PRESCALER_1;
#elif G_freq_HCLK == (G_freq_OSC / 2)
	reg |= HCLK_PRESCALER_2;
#elif G_freq_HCLK == (G_freq_OSC / 4)
	reg |= HCLK_PRESCALER_4;
#elif G_freq_HCLK == (G_freq_OSC / 8)
	reg |= HCLK_PRESCALER_8;
#elif G_freq_HCLK == (G_freq_OSC / 16)
	reg |= HCLK_PRESCALER_16;
#elif G_freq_HCLK == (G_freq_OSC / 64)
	reg |= HCLK_PRESCALER_64;
#elif G_freq_HCLK == (G_freq_OSC / 128)
	reg |= HCLK_PRESCALER_128;
#elif G_freq_HCLK == (G_freq_OSC / 256)
	reg |= HCLK_PRESCALER_256;
#elif G_freq_HCLK == (G_freq_OSC / 512)
	reg |= HCLK_PRESCALER_512;
#else
# error "G_freq_HCLK must be a G_freq_OSC / (1|2|4|8|16|64|128|256|512)"
#endif

#if G_freq_PCLK1 == G_freq_HCLK
	reg |= PCLK1_PRESCALER_1;
#elif G_freq_PCLK1 == (G_freq_HCLK / 2)
	reg |= PCLK1_PRESCALER_2;
#elif G_freq_PCLK1 == (G_freq_HCLK / 4)
	reg |= PCLK1_PRESCALER_4;
#elif G_freq_PCLK1 == (G_freq_HCLK / 8)
	reg |= PCLK1_PRESCALER_8;
#elif G_freq_PCLK1 == (G_freq_HCLK / 16)
	reg |= PCLK1_PRESCALER_16;
#else
# error "G_freq_PCLK1 must be a G_freq_HCLK / (1|2|4|8|16)"
#endif

#if G_freq_PCLK2 == G_freq_HCLK
	reg |= PCLK2_PRESCALER_1;
#elif G_freq_PCLK2 == (G_freq_HCLK / 2)
	reg |= PCLK2_PRESCALER_2;
#elif G_freq_PCLK2 == (G_freq_HCLK / 4)
	reg |= PCLK2_PRESCALER_4;
#elif G_freq_PCLK2 == (G_freq_HCLK / 8)
	reg |= PCLK2_PRESCALER_8;
#elif G_freq_PCLK2 == (G_freq_HCLK / 16)
	reg |= PCLK2_PRESCALER_16;
#else
# error "G_freq_PCLK2 must be a G_freq_HCLK / (1|2|4|8|16)"
#endif

	MODIFY_BITS(RCC->CFGR, RCC_CFGR_HPRE|RCC_CFGR_PPRE1|RCC_CFGR_PPRE2,
		reg
		);

	// Set flash latency and prefetch buffer
	//
	// The prefetch buffer must be on when using a scaler other than 1 for AHB
	//
	// The latency is 3 bits on the STM32F1 and 4 bits on the other lines
	latency = G_freq_HCLK / FLASH_WS_STEP;
	latency = (latency > FLASH_WS_MAX) ? FLASH_WS_MAX : latency;
	MODIFY_BITS(FLASH->ACR, FLASH_ACR_PRFTEN|FLASH_ACR_LATENCY,
		(0b1     << FLASH_ACR_PRFTEN_Pos  ) | // Enable the prefetch buffer
		(latency << FLASH_ACR_LATENCY_Pos ) |
		0);

	// We don't use the LSI for anything
	CLEAR_BIT(RCC->CSR, RCC_CSR_LSION);

	// The LSE is configured in the backup domain so enable the power interface
	// clock and the backup domain interface clock; keep it on afterwards
	// because RTC access requires them too
	clock_enable(RCC_PERIPH_PWR);
#if RCC_PERIPH_BKP
	clock_enable(RCC_PERIPH_BKP);
#endif

	BD_write_enable();
	SET_BIT(RCC->BDCR, RCC_BDCR_LSEON);
	while (!BIT_IS_SET(RCC->BDCR, RCC_BDCR_LSERDY)) {
		// Nothing to do here
	}
	BD_write_disable();

	return;
}

OPTIMIZE_FUNCTION \
void sleep_ms(utime_t ms) {
	light_sleep_ms(ms, 0);

	return;
}
OPTIMIZE_FUNCTION \
void hibernate_s(utime_t s, uint8_t flags) {
	if (s == 0) {
		return;
	}

#if USE_SERIAL
	if (BIT_IS_SET(flags, CFG_ALLOW_INTERRUPTS)) {
		NVIC_ClearPendingIRQ(UARTx_IRQn);
		NVIC_EnableIRQ(UARTx_IRQn);
	}
#endif // USE_SERIAL

	// UART interrupts can't wake us up from deep sleep so sleep lightly for a
	// few seconds before entering deep sleep in order to detect them
	// The user will have to use the button to enter the serial terminal.
#if USE_TERMINAL
	if (button_wakeup && (BIT_IS_SET(flags, CFG_ALLOW_INTERRUPTS))) {
		if ((G_IRQs == 0) && (s > 0)) {
			uint32_t w;

			button_wakeup = false;
			w = (s > (LIGHT_SLEEP_SECONDS + MIN_DEEP_SLEEP_SECONDS)) ? LIGHT_SLEEP_SECONDS : s;
			// LOGGER("Just a %u second nap...zZz...", (uint )w);
			NOTIFY("Waiting %u seconds for UART input", (uint )w);
			light_sleep_ms(w * 1000, flags);
			s -= w;
		}
	}
#endif

	// TODO: Handle the case where the RTC isn't clocked by the LSE and the RTC
	// alarm therefore can't be used
	if ((G_IRQs == 0) && (s > 0)) {
		// LOGGER("Wake me in %u seconds...ZZz...Zzz...zzZZZZZzzzZz", (uint )sleep_s);
		LOGGER("Sleeping deeply %u seconds", (uint )s);
		deep_sleep_s(s, flags);
	}

	if (G_IRQs != 0) {
		LOGGER("Hibernation ending with G_IRQs at 0x%02X", (uint )G_IRQs);
	}

#if USE_SERIAL
	NVIC_DisableIRQ(UARTx_IRQn);
	NVIC_ClearPendingIRQ(UARTx_IRQn);
#endif // USE_SERIAL

	return;
}
OPTIMIZE_FUNCTION \
static void light_sleep_ms(utime_t ms, uint8_t flags) {
	uint32_t period;

	// The systick interrupt will wake us from sleep if left enabled
	suspend_systick();

	// Don't use deep sleep mode
	CLEAR_BIT(SCB->SCR, SCB_SCR_SLEEPDEEP_Msk);

	while (ms > 0) {
		if (ms < SLEEP_TIM_MAX_MS) {
			period = ms;
			ms = 0;
		} else {
			period = SLEEP_TIM_MAX_MS;
			ms -= SLEEP_TIM_MAX_MS;
		}

		set_sleep_alarm(period);
		while (BIT_IS_SET(RCC->APB1ENR, SLEEP_ALARM_CLOCKEN)) {
			// Wait for an interrupt
			__WFI();
			// If desired keep sleeping until the wakeup alarm triggers
			if (BIT_IS_SET(flags, CFG_ALLOW_INTERRUPTS)) {
				ms = 0;
				break;
			} else if (BIT_IS_SET(RCC->APB1ENR, SLEEP_ALARM_CLOCKEN)) {
				// Let the user know we can't do anything right now
				sysflash();
				sysflash();
			}
		}
		stop_sleep_alarm();
	}

	// Resume systick
	resume_systick();

	return;
}
OPTIMIZE_FUNCTION \
static void deep_sleep_s(utime_t s, uint8_t flags) {
	utime_t period;

	// UART can't do anything during deep sleep and there's a pulldown when it's
	// on, so turn it off until wakeup
	uart_off();

	// The systick interrupt will wake us from sleep if left enabled
	suspend_systick();

	// Use deep sleep mode
	SET_BIT(SCB->SCR, SCB_SCR_SLEEPDEEP_Msk);

	MODIFY_BITS(PWR->CR, PWR_CR_PDDS|PWR_CR_LPDS,
		(0b0 << PWR_CR_PDDS_Pos) | // Enter stop mode
		(0b1 << PWR_CR_LPDS_Pos) | // Use the low-power voltage regulator
		0);

	while (s > 0) {
		if (s < HIBERNATE_MAX_S) {
			period = s;
			s = 0;
		} else {
			period = HIBERNATE_MAX_S;
			s -= HIBERNATE_MAX_S;
		}

		set_RTC_alarm(period);
		while (BIT_IS_SET(EXTI->IMR, RTC_ALARM_EXTI_LINE)) {
			// Wait for an interrupt
			// The stop mode entry procedure will be ignored and program execution
			// continues if any of the EXTI interrupt pending flags, peripheral
			// interrupt pending flags, or RTC alarm flag are set.
			__WFI();

			// If desired keep sleeping until the wakeup alarm triggers
			if (BIT_IS_SET(flags, CFG_ALLOW_INTERRUPTS)) {
				s = 0;
				break;
			} else if (BIT_IS_SET(EXTI->IMR, RTC_ALARM_EXTI_LINE)) {
				// Let the user know we can't do anything right now
				sysflash();
				sysflash();
			}
		}
		stop_RTC_alarm();
	}

	// The SYSCLK is always HSI on wakeup
#if USE_INTERNAL_CLOCK
	SET_BIT(RCC->CR, RCC_CR_HSION);
	while (!BIT_IS_SET(RCC->CR, RCC_CR_HSIRDY)) {
		// Nothing to do here
	}
#else // !USE_INTERNAL_CLOCK
	SET_BIT(RCC->CR, RCC_CR_HSEON);
	while (!BIT_IS_SET(RCC->CR, RCC_CR_HSERDY)) {
		// Nothing to do here
	}

	MODIFY_BITS(RCC->CFGR, RCC_CFGR_SW,
		(0b01 << RCC_CFGR_SW_Pos)  | // Use HSE as SYSCLK source
		0);
	// Wait for HSE to become SYSCLK
	while (GATHER_BITS(RCC->CFGR, 0b11, RCC_CFGR_SWS_Pos) != 0b01) {
		// Nothing to do here
	}
	CLEAR_BIT(RCC->CR, RCC_CR_HSION);
#endif // USE_INTERNAL_CLOCK

	// Resume systick
	resume_systick();

	// Clear wakeup flag by writing 1 to CWUF
	SET_BIT(PWR->CR, PWR_CR_CWUF);

	uart_on();

	return;
}

void clock_enable(rcc_periph_t periph_clock) {
	uint32_t mask;
	__IO uint32_t *reg, tmpreg;

	switch (SELECT_BITS(periph_clock, RCC_BUS_MASK)) {
	case RCC_BUS_AHB1:
#if defined(STM32F1)
		reg = &RCC->AHBENR;
#else
		reg = &RCC->AHB1ENR;
#endif
		break;
	case RCC_BUS_APB1:
		reg = &RCC->APB1ENR;
		break;
	case RCC_BUS_APB2:
		reg = &RCC->APB2ENR;
		break;
	default:
		return;
	}

	mask = MASK_BITS(periph_clock, RCC_BUS_MASK);
	SET_BIT(*reg, mask);

	// Delay after enabling clock; method copied from ST HAL
	tmpreg = SELECT_BITS(*reg, mask);
	while (SELECT_BITS(*reg, mask) != mask) {
		// Nothing to do here
	}
	tmpreg = tmpreg; // Shut the compiler up

	return;
}
void clock_disable(rcc_periph_t periph_clock) {
	uint32_t mask;
	__IO uint32_t *reg;

	switch (SELECT_BITS(periph_clock, RCC_BUS_MASK)) {
	case RCC_BUS_AHB1:
#if defined(STM32F1)
		reg = &RCC->AHBENR;
#else
		reg = &RCC->AHB1ENR;
#endif
		break;
	case RCC_BUS_APB1:
		reg = &RCC->APB1ENR;
		break;
	case RCC_BUS_APB2:
		reg = &RCC->APB2ENR;
		break;
	default:
		return;
	}

	mask = MASK_BITS(periph_clock, RCC_BUS_MASK);
	CLEAR_BIT(*reg, mask);
	while (SELECT_BITS(*reg, mask) != 0) {
		// Nothing to do here
	}

	return;
}
void clock_init(rcc_periph_t periph_clock) {
	uint32_t mask;
	__IO uint32_t *reg;

	clock_enable(periph_clock);

	switch (SELECT_BITS(periph_clock, RCC_BUS_MASK)) {
	// The STM32F1 series doesn't have a reset register for the AHB
#if ! defined(STM32F1)
	case RCC_BUS_AHB1:
		reg = &RCC->AHB1RSTR;
		break;
#endif
	case RCC_BUS_APB1:
		reg = &RCC->APB1RSTR;
		break;
	case RCC_BUS_APB2:
		reg = &RCC->APB2RSTR;
		break;
	default:
		return;
	}

	mask = MASK_BITS(periph_clock, RCC_BUS_MASK);
	SET_BIT(*reg, mask);
	while (SELECT_BITS(*reg, mask) != mask) {
		// Nothing to do here
	}
	CLEAR_BIT(*reg, mask);
	while (SELECT_BITS(*reg, mask) != 0) {
		// Nothing to do here
	}

	return;
}

void BD_write_enable(void) {
	if (bd_write_enabled == 0) {
		SET_BIT(PWR->CR, PWR_CR_DBP);
		while (!BIT_IS_SET(PWR->CR, PWR_CR_DBP)) {
			// Nothing to do here
		}
	}
	++bd_write_enabled;

	return;
}
void BD_write_disable(void) {
	if (bd_write_enabled != 0) {
		--bd_write_enabled;
	}
	if (bd_write_enabled == 0) {
		// Per the reference manual, backup domain write protection must remain
		// disabled if using HSE/128 as the RTC clock
		if (SELECT_BITS(RCC->BDCR, RCC_BDCR_RTCSEL) != RCC_BDCR_RTCSEL_HSE) {
			CLEAR_BIT(PWR->CR, PWR_CR_DBP);
			while (BIT_IS_SET(PWR->CR, PWR_CR_DBP)) {
				// Nothing to do here
			}
		}
	}

	return;
}


#ifdef __cplusplus
 }
#endif
