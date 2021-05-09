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

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/pwr.h>

// Work around name conflict...
#undef gpio_set_mode
#include <libopencm3/stm32/gpio.h>
#define gpio_set_mode gpio_set_mode_OVERRIDE


/*
* Static values
*/


/*
* Types
*/


/*
* Variables
*/
// Bus frequencies; defined in lib/stm32/f1/rcc.c in the libopencm3 source
// If not using the rcc_clock_setup_*() functions, these need to be set
// when the clock is initialized because some libopencm3 code uses them
extern uint32_t rcc_apb1_frequency;
extern uint32_t rcc_apb2_frequency;
extern uint32_t rcc_ahb_frequency;

#if USE_TERMINAL
// Treat the first sleep as user-initiated to make entering the terminal
// more convenient
static bool button_wakeup = true;
#endif

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
	//nvic_disable_irq(BUTTON_IRQn);

	SET_BIT(G_IRQs, BUTTON_IRQf);

	// Let the user know we got the interrupt even if we're not going to do
	// anything for a while
	// Can't use led_flash(), we may not have delay_ms() if we're coming out of
	// hibernate_s() or sleep_ms().
	//led_flash(1, DELAY_ACK);
	sysflash();

	exti_reset_request(GPIO_GET_PINMASK(BUTTON_PIN));
	nvic_clear_pending_irq(BUTTON_IRQn);

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
	uint32_t remaps;

	clocks_init();

	// The AFIO clock is needed to fix a few problems, remap the inputs, and
	// to set up the button interrupt below, so enable it here and disable at
	// the end of setup when it's no longer needed
	rcc_periph_clock_enable(RCC_AFIO);
	// Remap CAN to PD[01] (per the errata sheet) so it doesn't interfere with
	// USART1 (even though we don't use RTS...)
	remaps = AFIO_MAPR_CAN1_REMAP_PORTD;
#if SPI1_DO_REMAP
	remaps |= AFIO_MAPR_SPI1_REMAP;
#endif
#if UART1_DO_REMAP
	remaps |= AFIO_MAPR_USART1_REMAP;
#endif
#if I2C1_DO_REMAP
	remaps |= AFIO_MAPR_I2C1_REMAP;
#endif
	gpio_primary_remap(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON, remaps);

	gpio_init();
	// Violoate our separation of concerns in order to provide early user
	// feedback
	gpio_set_mode(LED_PIN, GPIO_MODE_PP, GPIO_LOW);
	// Can't led_flash() until time_init() has been called.
	//led_flash(1, DELAY_SHORT);

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
	scb_set_priority_grouping(SCB_AIRCR_PRIGROUP_GROUP16_NOSUB);

#if BUTTON_PIN
	// Configure the EXTI interrupt on the rising edge for the wakeup button
	gpio_set_mode(BUTTON_PIN, GPIO_MODE_IN, GPIO_BIAS_TO_STATE(BUTTON_PIN));
	exti_select_source(GPIO_GET_PINMASK(BUTTON_PIN), GPIO_GET_PORT(BUTTON_PIN));
	exti_set_trigger(GPIO_GET_PINMASK(BUTTON_PIN), EXTI_TRIGGER_RISING);
	exti_enable_request(GPIO_GET_PINMASK(BUTTON_PIN));
	exti_reset_request(GPIO_GET_PINMASK(BUTTON_PIN));
	nvic_set_priority(BUTTON_IRQn, BUTTON_IRQp);
	nvic_enable_irq(BUTTON_IRQn);
#endif // BUTTON_PIN

	// Only need this clock for EXTI interrupt configuration and GPIO remapping
	rcc_periph_clock_disable(RCC_AFIO);

	return;
}
static inline __attribute__((always_inline)) \
void sysflash(void) {
	SET_GPIO_OUTPUT_HIGH(LED_PIN);
	// Hopefully ~100 ms counting overhead time @4MHz
	for (uint32_t i = 100000; i > 0; --i) {
		// Count some clock cycles
		__asm__ volatile("" : "+g" (i) : :);
	}

	SET_GPIO_OUTPUT_LOW(LED_PIN);
	// Short delay to keep flashes from bleeding into each other
	// Hopefully ~20ms counting overhead time @4MHz
	for (uint32_t i = 20000; i > 0; --i) {
		// Count some clock cycles
		__asm__ volatile("" : "+g" (i) : :);
	}
}
static void clocks_init(void) {
	// Don't use clock source protection
	rcc_css_disable();

#if USE_INTERNAL_CLOCK
	rcc_osc_on(RCC_HSI);
	rcc_wait_for_osc_ready(RCC_HSI);
	rcc_set_sysclk_source(RCC_CFGR_SW_SYSCLKSEL_HSICLK);
	rcc_osc_off(RCC_HSE);

#else // !USE_INTERNAL_CLOCK
	rcc_osc_on(RCC_HSE);
	rcc_wait_for_osc_ready(RCC_HSE);
	rcc_set_sysclk_source(RCC_CFGR_SW_SYSCLKSEL_HSECLK);
	rcc_osc_off(RCC_HSI);
#endif // USE_INTERNAL_CLOCK

	// The names of these flags are different in the online docs than they are
	// in the source used by platformio, which I take to mean they're going to
	// change at some point.
#if G_freq_HCLK == G_freq_OSC
	//rcc_set_hpre(RCC_CFGR_HPRE_NODIV);
	rcc_set_hpre(RCC_CFGR_HPRE_SYSCLK_NODIV);
#elif G_freq_HCLK == (G_freq_OSC / 2)
	rcc_set_hpre(RCC_CFGR_HPRE_SYSCLK_DIV2);
#elif G_freq_HCLK == (G_freq_OSC / 4)
	rcc_set_hpre(RCC_CFGR_HPRE_SYSCLK_DIV4);
#elif G_freq_HCLK == (G_freq_OSC / 8)
	rcc_set_hpre(RCC_CFGR_HPRE_SYSCLK_DIV8);
#elif G_freq_HCLK == (G_freq_OSC / 16)
	rcc_set_hpre(RCC_CFGR_HPRE_SYSCLK_DIV16);
#elif G_freq_HCLK == (G_freq_OSC / 64)
	rcc_set_hpre(RCC_CFGR_HPRE_SYSCLK_DIV64);
#elif G_freq_HCLK == (G_freq_OSC / 128)
	rcc_set_hpre(RCC_CFGR_HPRE_SYSCLK_DIV128);
#elif G_freq_HCLK == (G_freq_OSC / 256)
	rcc_set_hpre(RCC_CFGR_HPRE_SYSCLK_DIV256);
#elif G_freq_HCLK == (G_freq_OSC / 512)
	rcc_set_hpre(RCC_CFGR_HPRE_SYSCLK_DIV512);
#else
# error "G_freq_HCLK must be a G_freq_OSC / (1|2|4|8|16|64|128|256|512)"
#endif

#if G_freq_PCLK1 == G_freq_HCLK
	//rcc_set_ppre1(RCC_CFGR_PPRE_NODIV);
	rcc_set_ppre1(RCC_CFGR_PPRE1_HCLK_NODIV);
#elif G_freq_PCLK1 == (G_freq_HCLK / 2)
	rcc_set_ppre1(RCC_CFGR_PPRE1_HCLK_DIV2);
#elif G_freq_PCLK1 == (G_freq_HCLK / 4)
	rcc_set_ppre1(RCC_CFGR_PPRE1_HCLK_DIV4);
#elif G_freq_PCLK1 == (G_freq_HCLK / 8)
	rcc_set_ppre1(RCC_CFGR_PPRE1_HCLK_DIV8);
#elif G_freq_PCLK1 == (G_freq_HCLK / 16)
	rcc_set_ppre1(RCC_CFGR_PPRE1_HCLK_DIV16);
#else
# error "G_freq_PCLK1 must be a G_freq_HCLK / (1|2|4|8|16)"
#endif

#if G_freq_PCLK2 == G_freq_HCLK
	//rcc_set_ppre2(RCC_CFGR_PPRE_NODIV);
	rcc_set_ppre2(RCC_CFGR_PPRE2_HCLK_NODIV);
#elif G_freq_PCLK2 == (G_freq_HCLK / 2)
	rcc_set_ppre2(RCC_CFGR_PPRE2_HCLK_DIV2);
#elif G_freq_PCLK2 == (G_freq_HCLK / 4)
	rcc_set_ppre2(RCC_CFGR_PPRE2_HCLK_DIV4);
#elif G_freq_PCLK2 == (G_freq_HCLK / 8)
	rcc_set_ppre2(RCC_CFGR_PPRE2_HCLK_DIV8);
#elif G_freq_PCLK2 == (G_freq_HCLK / 16)
	rcc_set_ppre2(RCC_CFGR_PPRE2_HCLK_DIV16);
#else
# error "G_freq_PCLK2 must be a G_freq_HCLK / (1|2|4|8|16)"
#endif

#if   G_freq_ADC == (G_freq_PCLK2 / 2)
	rcc_set_adcpre(RCC_CFGR_ADCPRE_PCLK2_DIV2);
#elif G_freq_ADC == (G_freq_PCLK2 / 4)
	rcc_set_adcpre(RCC_CFGR_ADCPRE_PCLK2_DIV4);
#elif G_freq_ADC == (G_freq_PCLK2 / 6)
	rcc_set_adcpre(RCC_CFGR_ADCPRE_PCLK2_DIV6);
#elif G_freq_ADC == (G_freq_PCLK2 / 8)
	rcc_set_adcpre(RCC_CFGR_ADCPRE_PCLK2_DIV8);
#else
# error "G_freq_ADC must be a G_freq_PCLK2 / (2|4|6|8)"
#endif

	// Set flash latency and prefetch buffer
	// According to the reference manual, you should use a flash latency of 0
	// for speeds <= 24MHz, 1 for 24-48MHz, and 2 for anything higher.
	flash_set_ws(FLASH_ACR_LATENCY_0WS);

	// We don't use the LSI for anything
	rcc_osc_off(RCC_LSI);

	// The LSE is configured in the backup domain so enable the power interface
	// clock and the backup domain interface clock
	rcc_periph_clock_enable(RCC_BKP);
	rcc_periph_clock_enable(RCC_PWR);

	pwr_disable_backup_domain_write_protect();
	rcc_osc_on(RCC_LSE);
	rcc_wait_for_osc_ready(RCC_LSE);
	pwr_enable_backup_domain_write_protect();

	// If not using the rcc_clock_setup_*() functions, these need to be set
	// here because some libopencm3 code uses them
	rcc_ahb_frequency = G_freq_HCLK;
	rcc_apb1_frequency = G_freq_PCLK1;
	rcc_apb2_frequency = G_freq_PCLK2;

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
		nvic_clear_pending_irq(UARTx_IRQn);
		nvic_enable_irq(UARTx_IRQn);
	}
#endif

	// UART interrupts can't wake us up from deep sleep so sleep lightly for a
	// few seconds before entering deep sleep in order to detect them
	// The user will have to use the button to enter the serial terminal.
#if USE_TERMINAL
	if (button_wakeup && (BIT_IS_SET(flags, CFG_ALLOW_INTERRUPTS))) {
		if ((G_IRQs == 0) && (s > 0)) {
			uint32_t w;

			button_wakeup = false;
			w = (s > (LIGHT_SLEEP_SECONDS + MIN_DEEP_SLEEP_SECONDS)) ? LIGHT_SLEEP_SECONDS : s;
			NOTIFY("Waiting %u seconds for UART input", (uint )w);
			light_sleep_ms(w * 1000, flags);
			s -= w;
		}
	}
#endif

	// TODO: Handle the case where the RTC isn't clocked by the LSE and the RTC
	// alarm therefore can't be used
	if ((G_IRQs == 0) && (s > 0)) {
		LOGGER("Sleeping deeply %u seconds", (uint )s);
		deep_sleep_s(s, flags);
	}

	if (G_IRQs != 0) {
		LOGGER("Hibernation ending with G_IRQs at 0x%02X", (uint )G_IRQs);
	}

#if USE_SERIAL
	nvic_disable_irq(UARTx_IRQn);
	nvic_clear_pending_irq(UARTx_IRQn);
#endif

	return;
}
OPTIMIZE_FUNCTION \
static void light_sleep_ms(utime_t ms, uint8_t flags) {
	uint16_t period;

	// The systick interrupt will wake us from sleep if left enabled
	systick_interrupt_disable();

	// I don't see any functions for light sleep mode, so...
	CLEAR_BIT(SCB_SCR, SCB_SCR_SLEEPDEEP);

	while (ms > 0) {
		if (ms < (0xFFFF/TIM_MS_TICKS)) {
			period = ms;
			ms = 0;
		} else {
			period = (0xFFFF/TIM_MS_TICKS);
			ms -= (0xFFFF/TIM_MS_TICKS);
		}

		set_sleep_alarm(period);

		while (sleep_alarm_is_set) {
			// Wait for an interrupt
			//__WFI();
			__asm volatile("wfi");
			// If desired keep sleeping until the wakeup alarm triggers
			if (!sleep_alarm_is_set || BIT_IS_SET(flags, CFG_ALLOW_INTERRUPTS)) {
				ms = 0;
				break;
			} else {
				// Let the user know we can't do anything right now
				sysflash();
				sysflash();
			}
		}
		stop_sleep_alarm();
	}

	// Resume systick
	systick_interrupt_enable();

	return;
}
OPTIMIZE_FUNCTION \
static void deep_sleep_s(utime_t s, uint8_t flags) {
	if (s == 0) {
		return;
	}

	// UART can't do anything during deep sleep and there's a pulldown when it's
	// on, so turn it off until wakeup
	uart_off();

	// The systick interrupt will wake us from sleep if left enabled
	systick_interrupt_disable();

	// Use deep sleep mode
	SET_BIT(SCB_SCR, SCB_SCR_SLEEPDEEP);
	// Enter stop mode
	pwr_set_stop_mode();
	// Use the low-power voltage regulator
	pwr_voltage_regulator_low_power_in_stop();

	set_RTC_alarm(s);
	while (RTC_alarm_is_set) {
		// Wait for an interrupt
		// The stop mode entry procedure will be ignored and program execution
		// continues if any of the EXTI interrupt pending flags, peripheral
		// interrupt pending flags, or RTC alarm flag are set.
		__asm volatile("wfi");
		//__WFI();

		// If desired keep sleeping until the wakeup alarm triggers
		if (!RTC_alarm_is_set || BIT_IS_SET(flags, CFG_ALLOW_INTERRUPTS)) {
			break;
		} else {
			// Let the user know we can't do anything right now
			sysflash();
			sysflash();
		}
	}
	stop_RTC_alarm();

	// The SYSCLK is always HSI on wakeup
#if USE_INTERNAL_CLOCK
	rcc_osc_on(RCC_HSI);
	rcc_wait_for_osc_ready(RCC_HSI);
	rcc_set_sysclk_source(RCC_CFGR_SW_SYSCLKSEL_HSICLK);
	rcc_osc_off(RCC_HSE);

#else // !USE_INTERNAL_CLOCK
	rcc_osc_on(RCC_HSE);
	rcc_wait_for_osc_ready(RCC_HSE);
	rcc_set_sysclk_source(RCC_CFGR_SW_SYSCLKSEL_HSECLK);
	rcc_osc_off(RCC_HSI);
#endif // USE_INTERNAL_CLOCK

	// Resume systick
	systick_interrupt_enable();

	// Clear wakeup flag
	pwr_clear_wakeup_flag();

	uart_on();

	return;
}

#ifdef __cplusplus
 }
#endif
