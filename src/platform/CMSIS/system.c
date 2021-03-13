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


#if LIGHT_SLEEP_PERIOD > MAX_SLEEP_PERIOD
# error "LIGHT_SLEEP_PERIOD must be <= MAX_SLEEP_PERIOD"
#endif // LIGHT_SLEEP_PERIOD > MAX_SLEEP_PERIOD


/*
* Static values
*/


/*
* Types
*/


/*
* Variables
*/
uint32_t G_freq_HCLK;
uint32_t G_freq_PCLK1;
uint32_t G_freq_PCLK2;


/*
* Local function prototypes
*/
static void light_sleep(utime_t ms, uint8_t flags);
static void deep_sleep(utime_t s, uint8_t flags);
static void clocks_init(void);
static void update_system_clock_vars(void);

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
	// Can't use led_flash(), we may not have delay() if we're coming out of
	// hibernate() or sleep().
	// led_flash(1, DELAY_ACK);
	sysflash();

	// The EXTI pending bit is cleared by writing 1
	SET_BIT(EXTI->PR, GPIO_GET_PINMASK(BUTTON_PIN));
	NVIC_ClearPendingIRQ(BUTTON_IRQn);

	return;
}
#endif // BUTTON_PIN

/*
* Functions
*/
void platform_init(void) {
	uint32_t remaps;

	// Needed before anything else to allow *_delay() to work
	update_system_clock_vars();

	clocks_init();

	// The AFIO clock is needed to fix a few problems, remap the inputs, and
	// to set up the button interrupt below, so enable it here and disable at
	// the end of setup when it's no longer needed
	clock_init(&RCC->APB2ENR, &RCC->APB2RSTR, RCC_APB2ENR_AFIOEN);
	remaps = 0;
	// Remap CAN to PD[01] (per the errata sheet) so it doesn't interfere with
	// USART1 (even though we don't use RTS...)
	remaps |= AFIO_MAPR_CAN_REMAP_REMAP3;
#if SPI1_DO_REMAP
	remaps |= AFIO_MAPR_SPI1_REMAP;
#endif
#if UART1_DO_REMAP
	remaps |= AFIO_MAPR_USART1_REMAP;
#endif
	// Unlike everything else, JTAG is enabled on reset and needs to be explicitly
	// disabled
	// Unlike everything else, reading the JTAG part of AFIO_MAPR will always
	// return '0' (fully-enabled) so it needs to be set everytime.
	remaps     |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
	MODIFY_BITS(AFIO->MAPR, AFIO_MAPR_SWJ_CFG|AFIO_MAPR_USART1_REMAP|AFIO_MAPR_SPI1_REMAP|AFIO_MAPR_CAN_REMAP,
		remaps);

	gpio_init();
	// Violoate our separation of concerns in order to provide early user
	// feedback
	gpio_set_mode(LED_PIN, GPIO_MODE_PP, GPIO_LOW);
	// Can't led_flash() until time_init() has been called.
	// led_flash(1, DELAY_SHORT);

	time_init();
	led_flash(1, DELAY_SHORT);

	uart_init();
	led_flash(1, DELAY_SHORT);

	spi_init();
	led_flash(1, DELAY_SHORT);

	adc_init();
	led_flash(1, DELAY_SHORT);

	// Set interrupt priority grouping
	// Use 4 bits for group priority and 0 bits for subpriority
	// See 4.4.5 of the Cortex M3 programming manual for how to determine the
	// value passed, it looks to be '3 + number of bits for subpriority'.
	NVIC_SetPriorityGrouping(0b011 + 0);

#if BUTTON_PIN
	uint32_t pmask, pinno;

	// Make sure the correct port is connected to the EXTI line
#if (BUTTON_PIN & GPIO_PORT_MASK) == GPIO_PORTA
	pmask = 0b0000;
#else
	pmask = 0b0001;
#endif
	pinno = GPIO_GET_PINNO(BUTTON_PIN);
#if GPIO_GET_PINNO(BUTTON_PIN) <= 3
	pinno = pinno*4;
	pmask <<= (pinno);
	MODIFY_BITS(AFIO->EXTICR[0], 0b1111 << pinno, pmask);
#elif GPIO_GET_PINNO(BUTTON_PIN) <= 7
	pinno = (pinno-4)*4;
	pmask <<= (pinno);
	MODIFY_BITS(AFIO->EXTICR[1], 0b1111 << pinno, pmask);
#elif GPIO_GET_PINNO(BUTTON_PIN) <= 11
	pinno = (pinno-8)*4;
	pmask <<= (pinno);
	MODIFY_BITS(AFIO->EXTICR[2], 0b1111 << pinno, pmask);
#elif GPIO_GET_PINNO(BUTTON_PIN) <= 15
	pinno = (pinno-12)*4;
	pmask <<= (pinno);
	MODIFY_BITS(AFIO->EXTICR[3], 0b1111 << pinno, pmask);
#else
# error "Unhandled BUTTON_PIN"
#endif // GPIO_GET_PINNO(BUTTON_PIN)

	// Configure the EXTI interrupt on the rising edge for the wakeup button
	gpio_set_mode(BUTTON_PIN, GPIO_MODE_IN, GPIO_LOW);
	SET_BIT(EXTI->IMR,  GPIO_GET_PINMASK(BUTTON_PIN));
	SET_BIT(EXTI->RTSR, GPIO_GET_PINMASK(BUTTON_PIN));
	NVIC_SetPriority(BUTTON_IRQn, BUTTON_IRQp);
	NVIC_EnableIRQ(BUTTON_IRQn);
#endif // BUTTON_PIN

	// Only need the clock for AFIO->EXTICR[x] and AFIO->MAPR access
	clock_disable(&RCC->APB2ENR, RCC_APB2ENR_AFIOEN);

	return;
}
static inline __attribute__((always_inline)) \
void sysflash(void) {
	SET_GPIO_OUTPUT_HIGH(LED_PIN);
	// Hopefully ~100 ms counting overhead time @4GHz
	for (uint32_t i = 0; i < 100000; ++i) {
		// Count some clock cycles
		__asm__ volatile("" : "+g" (i) : :);
	}

	SET_GPIO_OUTPUT_LOW(LED_PIN);
	// Short delay to keep flashes from bleeding into each other
	// Hopefully ~20ms counting overhead time @4GHz
	for (uint32_t i = 0; i < 20000; ++i) {
		// Count some clock cycles
		__asm__ volatile("" : "+g" (i) : :);
	}
}
static void clocks_init(void) {
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
	CLEAR_BIT(RCC->CR, RCC_CR_HSEON);

#else // !USE_INTERNAL_CLOCK
	SET_BIT(RCC->CR, RCC_CR_HSEON);
	while (!BIT_IS_SET(RCC->CR, RCC_CR_HSERDY)) {
		// Nothing to do here
		// TODO: Timeout?
	}

	MODIFY_BITS(RCC->CFGR, RCC_CFGR_SW,
		(0b01 << RCC_CFGR_SW_Pos) | // Use HSE as SYSCLK source
		0);
	// Wait for HSE to become SYSCLK
	while (GATHER_BITS(RCC->CFGR, 0b11, RCC_CFGR_SWS_Pos) != 0b01) {
		// Nothing to do here
		// TODO: Timeout?
	}
	CLEAR_BIT(RCC->CR, RCC_CR_HSION);
#endif // USE_INTERNAL_CLOCK

	MODIFY_BITS(RCC->CFGR, RCC_CFGR_PPRE2|RCC_CFGR_PPRE1|RCC_CFGR_HPRE,
		(0b000  << RCC_CFGR_PPRE2_Pos)  | // Use HCLK as APB2 clock
		(0b100  << RCC_CFGR_PPRE1_Pos)  | // Use HCLK/2 as APB1 clock
		(0b1000 << RCC_CFGR_HPRE_Pos )  | // Use SYSCLK/2 as HCLK
		0);

	update_system_clock_vars();

	// We don't use the LSI for anything
	CLEAR_BIT(RCC->CSR, RCC_CSR_LSION);

	// The LSE is configured in the backup domain so enable the power interface
	// clock and the backup domain interface clock; keep it on afterwards
	// because RTC access requires them too
	clock_enable(&RCC->APB1ENR, RCC_APB1ENR_PWREN|RCC_APB1ENR_BKPEN);

	BD_write_enable();
	SET_BIT(RCC->BDCR, RCC_BDCR_LSEON);
	while (!BIT_IS_SET(RCC->BDCR, RCC_BDCR_LSERDY)) {
		// Nothing to do here
		// TODO: Timeout?
	}
	BD_write_disable();

	// Set flash latency and prefetch buffer
	// According to the reference manual, you should use a flash latency of 0
	// for speeds <= 24MHz, 1 for 24-48MHz, and 2 for anything higher.
	// The prefetch buffer must be on when using a scaler other than 1 for HCLK
	MODIFY_BITS(FLASH->ACR, FLASH_ACR_PRFTBE|FLASH_ACR_LATENCY,
		(0b1   << FLASH_ACR_PRFTBE_Pos  ) | // Enable the prefetch buffer
		(0b000 << FLASH_ACR_LATENCY_Pos ) | // Use a latency of 0
		0);

	return;
}
static void update_system_clock_vars(void) {
	uint32_t div;

	// This function is provided by system_stm32f1xx.c from CMSIS
	SystemCoreClockUpdate();
	// SystemCoreClock is provided by system_stm32f1xx.c from CMSIS
	G_freq_HCLK = SystemCoreClock;

	div = GATHER_BITS(RCC->CFGR, 0b111, RCC_CFGR_PPRE1_Pos);
	// APBPrescTable[] is provided by system_stm32f1xx.c from CMSIS
	G_freq_PCLK1 = G_freq_HCLK >> APBPrescTable[div];

	div = GATHER_BITS(RCC->CFGR, 0b111, RCC_CFGR_PPRE2_Pos);
	// APBPrescTable[] is provided by system_stm32f1xx.c from CMSIS
	G_freq_PCLK2 = G_freq_HCLK >> APBPrescTable[div];

	return;
}

OPTIMIZE_FUNCTION \
void sleep(utime_t ms) {
	light_sleep(ms, 0);

	return;
}
OPTIMIZE_FUNCTION \
void hibernate(utime_t s, uint8_t flags) {
	uint32_t sleep_s;

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
	// The user will have to use the button or wait for the RTC alarm to enter
	// the serial terminal.
	sleep_s = (s > (LIGHT_SLEEP_PERIOD + MIN_DEEP_SLEEP_PERIOD)) ? LIGHT_SLEEP_PERIOD : s;
	if ((G_IRQs == 0) && (s > 0)) {
		// LOGGER("Just a %u second nap...zZz...", (uint )sleep_s);
		LOGGER("Sleeping lightly %u seconds", (uint )sleep_s);
		light_sleep(sleep_s * 1000, flags);
	}

	// TODO: Handle the case where the RTC isn't clocked by the LSE and the RTC
	// alarm therefore can't be used
	s -= sleep_s;
	if ((G_IRQs == 0) && (s > 0)) {
		sleep_s = (s > MAX_SLEEP_PERIOD) ? MAX_SLEEP_PERIOD : s;
		// LOGGER("Wake me in %u seconds...ZZz...Zzz...zzZZZZZzzzZz", (uint )sleep_s);
		LOGGER("Sleeping deeply %u seconds", (uint )sleep_s);
		deep_sleep(sleep_s, flags);
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
static void light_sleep(utime_t ms, uint8_t flags) {
	uint16_t period;

	// The systick interrupt will wake us from sleep if left enabled
	CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_ENABLE_Msk);

	// Don't use deep sleep mode
	CLEAR_BIT(SCB->SCR, SCB_SCR_SLEEPDEEP_Msk);

	while (ms > 0) {
		if (ms < (0xFFFF/TIM_MS_PERIOD)) {
			period = ms;
			ms = 0;
		} else {
			period = (0xFFFF/TIM_MS_PERIOD);
			ms -= (0xFFFF/TIM_MS_PERIOD);
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
	SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_ENABLE_Msk);

	return;
}
OPTIMIZE_FUNCTION \
static void deep_sleep(utime_t s, uint8_t flags) {
	if (s == 0) {
		return;
	}

	// UART can't do anything during deep sleep and there's a pulldown when it's
	// on, so turn it off until wakeup
	uart_off();

	// The systick interrupt will wake us from sleep if left enabled
	CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_ENABLE_Msk);

	// Use deep sleep mode
	SET_BIT(SCB->SCR, SCB_SCR_SLEEPDEEP_Msk);

	MODIFY_BITS(PWR->CR, PWR_CR_PDDS|PWR_CR_LPDS,
		(0b0 << PWR_CR_PDDS_Pos) | // Enter stop mode
		(0b1 << PWR_CR_LPDS_Pos) | // Use the low-power voltage regulator
		0);

	set_RTC_alarm(s);
	while (BIT_IS_SET(EXTI->IMR, RTC_ALARM_EXTI_LINE)) {
		// Wait for an interrupt
		// The stop mode entry procedure will be ignored and program execution
		// continues if any of the EXTI interrupt pending flags, peripheral
		// interrupt pending flags, or RTC alarm flag are set.
		__WFI();

		// If desired keep sleeping until the wakeup alarm triggers
		if (BIT_IS_SET(flags, CFG_ALLOW_INTERRUPTS)) {
			break;
		} else if (BIT_IS_SET(EXTI->IMR, RTC_ALARM_EXTI_LINE)) {
			// Let the user know we can't do anything right now
			sysflash();
			sysflash();
		}
	}
	stop_RTC_alarm();

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
		// TODO: Timeout?
	}

	MODIFY_BITS(RCC->CFGR, RCC_CFGR_SW,
		(0b01 << RCC_CFGR_SW_Pos)  | // Use HSE as SYSCLK source
		0);
	// Wait for HSE to become SYSCLK
	while (GATHER_BITS(RCC->CFGR, 0b11, RCC_CFGR_SWS_Pos) != 0b01) {
		// Nothing to do here
		// TODO: Timeout?
	}
	CLEAR_BIT(RCC->CR, RCC_CR_HSION);
#endif // USE_INTERNAL_CLOCK

	// Resume systick
	SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_ENABLE_Msk);

	// Clear wakeup flag by writing 1 to CWUF
	SET_BIT(PWR->CR, PWR_CR_CWUF);

	uart_on();

	return;
}

void clock_enable(__IO uint32_t *reg, uint32_t enable_mask) {
	__IO uint32_t tmpreg;

	SET_BIT(*reg, enable_mask);
	// Delay after enabling clock; method copied from ST HAL
	tmpreg = SELECT_BITS(*reg, enable_mask);
	while (SELECT_BITS(*reg, enable_mask) != enable_mask) {
		// Nothing to do here
		// TODO: Timeout?
	}
	tmpreg = tmpreg; // Shut the compiler up

	return;
}
void clock_disable(__IO uint32_t *reg, uint32_t enable_mask) {
	CLEAR_BIT(*reg, enable_mask);
	while (SELECT_BITS(*reg, enable_mask) != 0) {
		// Nothing to do here
		// TODO: Timeout?
	}

	return;
}
void clock_init(__IO uint32_t *en_reg, __IO uint32_t *rst_reg, uint32_t enable_mask) {
	clock_enable(en_reg, enable_mask);

	SET_BIT(*rst_reg, enable_mask);
	while (SELECT_BITS(*rst_reg, enable_mask) != enable_mask) {
		// Nothing to do here
		// TODO: Timeout?
	}

	CLEAR_BIT(*rst_reg, enable_mask);
	while (SELECT_BITS(*rst_reg, enable_mask) != 0) {
		// Nothing to do here
		// TODO: Timeout?
	}

	return;
}

void BD_write_enable(void) {
	SET_BIT(PWR->CR, PWR_CR_DBP);
	while (!BIT_IS_SET(PWR->CR, PWR_CR_DBP)) {
		// Nothing to do here
		// TODO: Timeout?
	}

	return;
}
void BD_write_disable(void) {
	// Per the reference manual, backup domain write protection must remain
	// disabled if using HSE/128 as the RTC clock
	if (SELECT_BITS(RCC->BDCR, RCC_BDCR_RTCSEL) != RCC_BDCR_RTCSEL_HSE) {
		CLEAR_BIT(PWR->CR, PWR_CR_DBP);
		while (BIT_IS_SET(PWR->CR, PWR_CR_DBP)) {
			// Nothing to do here
			// TODO: Timeout?
		}
	}

	return;
}
err_t RTC_cfg_enable(utime_t timeout) {
	timeout = SET_TIMEOUT(timeout);

	BD_write_enable();

	while (!BIT_IS_SET(RTC->CRL, RTC_CRL_RTOFF)) {
		if (TIMES_UP(timeout)) {
			return ETIMEOUT;
		}
	}

	SET_BIT(RTC->CRL, RTC_CRL_CNF);
	while (!BIT_IS_SET(RTC->CRL, RTC_CRL_CNF)) {
		if (TIMES_UP(timeout)) {
			return ETIMEOUT;
		}
	}

	return EOK;
}
err_t RTC_cfg_disable(utime_t timeout) {
	timeout = SET_TIMEOUT(timeout);

	CLEAR_BIT(RTC->CRL, RTC_CRL_CNF);
	while (BIT_IS_SET(RTC->CRL, RTC_CRL_CNF)) {
		if (TIMES_UP(timeout)) {
			return ETIMEOUT;
		}
	}

	while (!BIT_IS_SET(RTC->CRL, RTC_CRL_RTOFF)) {
		if (TIMES_UP(timeout)) {
			return ETIMEOUT;
		}
	}

	BD_write_disable();

	return EOK;
}

#ifdef __cplusplus
 }
#endif
