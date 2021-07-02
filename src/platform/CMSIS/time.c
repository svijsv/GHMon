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
// time.c
// Manage the time-keeping peripherals
// NOTES:
//    The timer numbers are defined in time.h
//
//    Only timers 1-4 are set up because those are the ones present on the
//    STM32F103C8
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
/*
* Includes
*/
#include "time.h"
#include "system.h"

/*
* Static values
*/
// Divide the number of cycles per ms by this in a dumb delay to account for
// overhead
#define DUMB_DELAY_DIV 7

/*
* Types
*/


/*
* Variables
*/
// System tick count, milliseconds
volatile utime_t G_sys_msticks;


/*
* Local function prototypes
*/
static uint16_t calculate_APB1_TIM_prescaler(uint32_t hz);
static uint16_t calculate_APB2_TIM_prescaler(uint32_t hz);
static void systick_init(void);
static void RTC_init(void);
static void timers_init(void);


/*
* Interrupt handlers
*/
OPTIMIZE_FUNCTION \
void SysTick_Handler(void) {
	++G_sys_msticks;
	return;
}
OPTIMIZE_FUNCTION \
void RTC_Alarm_IRQHandler(void) {
	NVIC_DisableIRQ(RTC_Alarm_IRQn);
	NVIC_ClearPendingIRQ(RTC_Alarm_IRQn);

	CLEAR_BIT(RTC->CRL,
		RTC_CRL_ALRF  | // Clear the alarm interrupt flag
		0);

	// Clear the EXTI line interrupt flag
	// This is set to 1 to clear
	SET_BIT(EXTI->PR, RTC_ALARM_EXTI_LINE);
	// Disable the alarm EXTI interrupt
	CLEAR_BIT(EXTI->IMR, RTC_ALARM_EXTI_LINE);

	return;
}
OPTIMIZE_FUNCTION \
void SleepAlarm_IRQHandler(void) {
	NVIC_DisableIRQ(SLEEP_ALARM_IRQn);
	NVIC_ClearPendingIRQ(SLEEP_ALARM_IRQn);

	//CLEAR_BIT(SLEEP_ALARM_TIM->SR, TIM_SR_UIF);
	SLEEP_ALARM_TIM->SR = 0;
	// Configured to disable itself
	//CLEAR_BIT(SLEEP_ALARM_TIM->CR1, TIM_CR1_CEN);
	clock_disable(&RCC->APB1ENR, SLEEP_ALARM_CLOCKEN);

	return;
}


/*
* Functions
*/
void time_init(void) {
	systick_init();
	RTC_init();
	timers_init();

	return;
}
//
// Set up systick timer
static void systick_init(void) {
	uint32_t period;

	G_sys_msticks = 0;
	// G_sys_sticks = 0;

	// Re-implement SysTick_Config() from CMSIS/ARM/core_cm3.h so we can use
	// HCLK/8 as our clock source (but why?...)
	period = (G_freq_HCLK/8) / 1000;
	assert(period > 0);
	assert(SysTick_LOAD_RELOAD_Msk >= (period-1));

	SysTick->LOAD = (uint32_t )(period - 1);
	NVIC_SetPriority(SysTick_IRQn, SYSTICK_IRQp);
	SysTick->VAL = 0;
	MODIFY_BITS(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Msk|SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_ENABLE_Msk,
		(0b0 << SysTick_CTRL_CLKSOURCE_Pos) | // Keep 0 for HCLCK/8; set to 1 for HCLK
		(0b1 << SysTick_CTRL_TICKINT_Pos  ) | // Enable the interrupt
		(0b1 << SysTick_CTRL_ENABLE_Pos   ) | // Enable the counter
		0);

	return;
}
//
// Set up RTC
// The RTC clock is configured in the backup domain so the power interface
// clock and the backup domain interface clock need to be enabled before this
// is called.
// TODO: Load date from backup registers? AN2821 may shed light on that.
// TODO: Reset BDRST to allow changing RTC settings if they differ from current
static void RTC_init(void) {
	BD_write_enable();
	MODIFY_BITS(RCC->BDCR, RCC_BDCR_RTCSEL|RCC_BDCR_RTCEN,
		RCC_BDCR_RTCSEL_LSE |
		RCC_BDCR_RTCEN      | // Enable the RTC
		0);
	BD_write_disable();

	// Wait until the RTC registries are synchronized with the RTC core
	CLEAR_BIT(RTC->CRL, RTC_CRL_RSF);
	while (!BIT_IS_SET(RTC->CRL, RTC_CRL_RSF)) {
		// Nothing to do here
	}

	RTC_cfg_enable(1000);
	// Set the prescaler
	WRITE_SPLITREG(G_freq_LSE-1, RTC->PRLH, RTC->PRLL);
	RTC_cfg_disable(1000);

	NVIC_SetPriority(RTC_Alarm_IRQn, RTC_ALARM_IRQp);

	return;
}
utime_t get_RTC_seconds(void) {
	uint32_t rtcs;

	READ_SPLITREG(rtcs, RTC->CNTH, RTC->CNTL);

	return rtcs;
}
err_t set_RTC_seconds(utime_t s) {
	err_t res;

	if ((res = RTC_cfg_enable(1000)) == EOK) {
		WRITE_SPLITREG(s, RTC->CNTH, RTC->CNTL);
		res = RTC_cfg_disable(1000);
	}

	return res;
}

void set_RTC_alarm(utime_t time) {
	assert(time > 0);
	time = get_RTC_seconds() + time;

	RTC_cfg_enable(100);
	WRITE_SPLITREG(time, RTC->ALRH, RTC->ALRL);

	CLEAR_BIT(RTC->CRL, RTC_CRL_ALRF); // Clear the alarm interrupt flag
	RTC_cfg_disable(100);

	// Clear the EXTI line interrupt flag
	// This is set to 1 to clear
	SET_BIT(EXTI->PR, RTC_ALARM_EXTI_LINE);
	// Enable the alarm EXTI rising-edge trigger
	// This is mandatory to wake from stop mode
	SET_BIT(EXTI->RTSR, RTC_ALARM_EXTI_LINE);
	// Enable the alarm EXTI interrupt
	SET_BIT(EXTI->IMR, RTC_ALARM_EXTI_LINE);

	NVIC_ClearPendingIRQ(RTC_Alarm_IRQn);
	NVIC_EnableIRQ(RTC_Alarm_IRQn);

	return;
}
void stop_RTC_alarm(void) {
	NVIC_DisableIRQ(RTC_Alarm_IRQn);
	NVIC_ClearPendingIRQ(RTC_Alarm_IRQn);

	// Clear the EXTI line interrupt flag
	// This is set to 1 to clear
	SET_BIT(EXTI->PR, RTC_ALARM_EXTI_LINE);
	// Disable the alarm EXTI rising-edge trigger
	CLEAR_BIT(EXTI->RTSR, RTC_ALARM_EXTI_LINE);
	// Disable the alarm EXTI interrupt
	CLEAR_BIT(EXTI->IMR, RTC_ALARM_EXTI_LINE);

	// Clear the alarm interrupt flag
	CLEAR_BIT(RTC->CRL, RTC_CRL_ALRF);

	return;
}
//
// Configure the remaining timers
#define CONFIGURE_PWM_TIMER(TIMx) do { \
	(TIMx)->CR1 = cr1; \
	(TIMx)->ARR = 100; \
	(TIMx)->CCMR1 = ccmr; \
	(TIMx)->CCMR2 = ccmr; \
	/* Generate an update event to load the shadow registers */ \
	SET_BIT((TIMx)->EGR, TIM_EGR_UG); \
	/* Clear all event flags */ \
	(TIMx)->SR = 0; \
	} while (0);

static void timers_init(void) {
	uint32_t apb1_enr = 0, apb2_enr = 0;
	uint16_t psc_apb1, psc_apb2, cr1, ccmr;

	// Timers 2-7 and 12-14 are on APB1
	// Timers 1 and 8-11 are on APB2
	// Not all timers are present on all hardware
#ifdef TIM1
	apb2_enr |= RCC_APB2ENR_TIM1EN;
#endif
#ifdef TIM2
	apb1_enr |= RCC_APB1ENR_TIM2EN;
#endif
#ifdef TIM3
	apb1_enr |= RCC_APB1ENR_TIM3EN;
#endif
#ifdef TIM4
	apb1_enr |= RCC_APB1ENR_TIM4EN;
#endif
	clock_init(&RCC->APB1ENR, &RCC->APB1RSTR, apb1_enr);
	clock_init(&RCC->APB2ENR, &RCC->APB2RSTR, apb2_enr);

	//
	// PWM timers
	//
	psc_apb1 = calculate_APB1_TIM_prescaler(PWM_MAX_FREQUENCY*PWM_DUTY_CYCLE_SCALE);
	psc_apb2 = calculate_APB2_TIM_prescaler(PWM_MAX_FREQUENCY*PWM_DUTY_CYCLE_SCALE);
	// Auto-reload register buffer is suggested by the reference manual for
	// reliability in PWM mode
	cr1 = TIM_CR1_ARPE;
	// PWM mode 1 is high while TIMx_CNT < TIMx_CCRx
	// PWM mode 2 is high while TIMx_CNT > TIMx_CCRx
	ccmr =
		(0b00  << TIM_CCMR1_CC1S_Pos)  | // Channel 1/3, output mode
		(0b110 << TIM_CCMR1_OC1M_Pos)  | // Channel 1/3, PWM mode 1
		(0b1   << TIM_CCMR1_OC1PE_Pos) | // Channel 1/3, preload register enabled
		(0b00  << TIM_CCMR1_CC2S_Pos)  | // Channel 2/4, output mode
		(0b110 << TIM_CCMR1_OC2M_Pos)  | // Channel 2/4, PWM mode 1
		(0b1   << TIM_CCMR1_OC2PE_Pos) | // Channel 2/4, preload register enabled
		0;
#if USE_TIMER1_PWM
	TIM1->PSC = psc_apb2;
	// Main output enable, only needed for advanced timers (1 and 8)
	TIM1->BDTR = TIM_BDTR_MOE;
	CONFIGURE_PWM_TIMER(TIM1);
#endif
#if USE_TIMER2_PWM
	TIM2->PSC = psc_apb1;
	CONFIGURE_PWM_TIMER(TIM2);
#endif
#if USE_TIMER3_PWM
	TIM3->PSC = psc_apb1;
	CONFIGURE_PWM_TIMER(TIM3);
#endif
#if USE_TIMER4_PWM
	TIM4->PSC = psc_apb1;
	CONFIGURE_PWM_TIMER(TIM4);
#endif

	//
	// Sleep timer
	//
	MODIFY_BITS(SLEEP_ALARM_TIM->CR1, TIM_CR1_ARPE|TIM_CR1_CMS|TIM_CR1_DIR|TIM_CR1_OPM,
		(0b0  << TIM_CR1_ARPE_Pos) | // 0 to disable reload register buffer
		(0b00 << TIM_CR1_CMS_Pos ) | // 0 to disable bidirectional counting
		(0b0 << TIM_CR1_DIR_Pos  ) | // 0 to use as an upcounter
		(0b1 << TIM_CR1_OPM_Pos  ) | // 1 to automatically disable on update events
		0);
	SLEEP_ALARM_TIM->PSC = calculate_APB1_TIM_prescaler(1000*TIM_MS_TICKS);
	NVIC_SetPriority(SLEEP_ALARM_IRQn, SLEEP_ALARM_IRQp);

	//
	// USCOUNTER timer
	//
	MODIFY_BITS(USCOUNTER_TIM->CR1, TIM_CR1_ARPE|TIM_CR1_CMS|TIM_CR1_DIR|TIM_CR1_OPM,
		(0b0  << TIM_CR1_ARPE_Pos) | // 0 to disable reload register buffer
		(0b00 << TIM_CR1_CMS_Pos ) | // 0 to disable bidirectional counting
		(0b0 << TIM_CR1_DIR_Pos  ) | // 0 to use as an upcounter
		(0b1 << TIM_CR1_OPM_Pos  ) | // 1 to automatically disable on update events
		0);
	USCOUNTER_TIM->PSC = calculate_APB1_TIM_prescaler(1000000);
	USCOUNTER_TIM->ARR = 0xFFFF;

	clock_disable(&RCC->APB1ENR, apb1_enr);
	clock_disable(&RCC->APB2ENR, apb2_enr);

	return;
}
// hz is the target Hz, e.g. 1000 for 1ms timing
// Clocks:
// Timers 2-7 and 12-14
//   PCLK1*1 if PCLK1 prescaler is 1
//   PCLK1*2 otherwise
// Timers 1 and 8-11
//   PCLK2*1 if PCLK2 prescaler is 1
//   PCLK2*2 otherwise
static uint16_t calculate_APB1_TIM_prescaler(uint32_t hz) {
	uint16_t prescaler;

	assert(((G_freq_PCLK1/hz) <= (0xFFFF/2)) || (SELECT_BITS(RCC->CFGR, RCC_CFGR_PPRE1) == 0));
	assert(hz != 0);

	prescaler = G_freq_PCLK1 / hz;
	if (SELECT_BITS(RCC->CFGR, RCC_CFGR_PPRE1_2) != 0) {
		prescaler *= 2;
	}

	assert(prescaler != 0);
	// A prescaler of 0 divides by 1 so we need to adjust.
	--prescaler;

	return prescaler;
}
static uint16_t calculate_APB2_TIM_prescaler(uint32_t hz) {
	uint16_t prescaler;

	assert(((G_freq_PCLK2/hz) <= (0xFFFF/2)) || (SELECT_BITS(RCC->CFGR, RCC_CFGR_PPRE2) == 0));
	assert(hz != 0);

	prescaler = G_freq_PCLK2 / hz;
	if (SELECT_BITS(RCC->CFGR, RCC_CFGR_PPRE2_2) != 0) {
		prescaler *= 2;
	}

	assert(prescaler != 0);
	// A prescaler of 0 divides by 1 so we need to adjust.
	--prescaler;

	return prescaler;
}
void set_sleep_alarm(uint16_t ms) {
	assert(ms != 0);
#if TIM_MS_TICKS > 1
	assert((0xFFFF/TIM_MS_TICKS) >= ms);
#endif // TIM_MS_TICKS > 1

	clock_enable(&RCC->APB1ENR, SLEEP_ALARM_CLOCKEN);

	// Set the reload value and generate an update event to load it
	SLEEP_ALARM_TIM->ARR = ms * TIM_MS_TICKS;
	SET_BIT(SLEEP_ALARM_TIM->EGR, TIM_EGR_UG);

	SLEEP_ALARM_TIM->SR = 0x0000;                 // Clear all event flags
	SET_BIT(SLEEP_ALARM_TIM->DIER, TIM_DIER_UIE); // Enable update interrupts
	NVIC_ClearPendingIRQ(SLEEP_ALARM_IRQn);
	NVIC_EnableIRQ(SLEEP_ALARM_IRQn);
	SET_BIT(SLEEP_ALARM_TIM->CR1, TIM_CR1_CEN);   // Enable the timer

	return;
}
void stop_sleep_alarm(void) {
	NVIC_DisableIRQ(SLEEP_ALARM_IRQn);
	NVIC_ClearPendingIRQ(SLEEP_ALARM_IRQn);

	//CLEAR_BIT(SLEEP_ALARM_TIM->SR, TIM_SR_UIF);
	SLEEP_ALARM_TIM->SR = 0;
	// Configured to disable itself
	//CLEAR_BIT(SLEEP_ALARM_TIM->CR1, TIM_CR1_CEN);

	clock_disable(&RCC->APB1ENR, SLEEP_ALARM_CLOCKEN);

	return;
}
void uscounter_on(void) {
	clock_enable(&RCC->APB1ENR, USCOUNTER_CLOCKEN);
	return;
}
void uscounter_off(void) {
	clock_disable(&RCC->APB1ENR, USCOUNTER_CLOCKEN);
	return;
}

void delay_ms(utime_t ms) {
	utime_t timer;

#if DEBUG && USE_SERIAL
	uint32_t systick_mask = SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_ENABLE_Msk;
	if (SELECT_BITS(SysTick->CTRL, systick_mask) != systick_mask) {
		uart_on();
		LOGGER("Someone is using delay_ms() without systick...");
		dumb_delay_ms(ms);
		return;
	}
#endif

	timer = SET_TIMEOUT(ms);
	while (!TIMES_UP(timer)) {
		// Nothing to do here
	}

	return;
}
// https:// stackoverflow.com/questions/7083482/how-to-prevent-gcc-from-optimizing-out-a-busy-wait-loop
void dumb_delay_ms(utime_t ms) {
	uint32_t cycles;

	cycles = ms * (G_freq_HCLK/(1000*DUMB_DELAY_DIV));

	for (uint32_t i = 0; i < cycles; ++i) {
		// Count some clock cycles
		__asm__ volatile("" : "+g" (i) : :);
	}

	return;
}
void dumb_delay_cycles(uint32_t cycles) {
	for (uint32_t i = 0; i < cycles; ++i) {
		// Count some clock cycles
		__asm__ volatile("" : "+g" (i) : :);
	}

	return;
}
//
// PWM management
void pwm_on(pin_t pin, uint16_t duty_cycle) {
	uint8_t channel;
	TIM_TypeDef *TIMx;

	channel = 4;
	switch (PINID(pin)) {
#if USE_TIMER1_PWM
	case PINID_TIM1_CH1:
		--channel;
		// fall through
	case PINID_TIM1_CH2:
		--channel;
		// fall through
	case PINID_TIM1_CH3:
		--channel;
		// fall through
	case PINID_TIM1_CH4:
		clock_enable(&RCC->APB2ENR, RCC_APB2ENR_TIM1EN);
		TIMx = TIM1;
		break;
#endif
#if USE_TIMER2_PWM
	case PINID_TIM2_CH1:
		--channel;
		// fall through
	case PINID_TIM2_CH2:
		--channel;
		// fall through
	case PINID_TIM2_CH3:
		--channel;
		// fall through
	case PINID_TIM2_CH4:
		clock_enable(&RCC->APB1ENR, RCC_APB1ENR_TIM2EN);
		TIMx = TIM2;
		break;
#endif
#if USE_TIMER3_PWM
	case PINID_TIM3_CH1:
		--channel;
		// fall through
	case PINID_TIM3_CH2:
		--channel;
		// fall through
	case PINID_TIM3_CH3:
		--channel;
		// fall through
	case PINID_TIM3_CH4:
		clock_enable(&RCC->APB1ENR, RCC_APB1ENR_TIM3EN);
		TIMx = TIM3;
		break;
#endif
#if USE_TIMER4_PWM
	case PINID_TIM4_CH1:
		--channel;
		// fall through
	case PINID_TIM4_CH2:
		--channel;
		// fall through
	case PINID_TIM4_CH3:
		--channel;
		// fall through
	case PINID_TIM4_CH4:
		clock_enable(&RCC->APB1ENR, RCC_APB1ENR_TIM4EN);
		TIMx = TIM4;
		break;
#endif

	default:
		LOGGER("Attempted PWM with incapable pin 0x%02X", (uint )pin);
		goto END;
		break;
	}

	switch (channel) {
	case 1:
		TIMx->CCR1 = duty_cycle;
		SET_BIT(TIMx->CCER, TIM_CCER_CC1E);
		break;
	case 2:
		TIMx->CCR2 = duty_cycle;
		SET_BIT(TIMx->CCER, TIM_CCER_CC2E);
		break;
	case 3:
		TIMx->CCR3 = duty_cycle;
		SET_BIT(TIMx->CCER, TIM_CCER_CC3E);
		break;
	case 4:
		TIMx->CCR4 = duty_cycle;
		SET_BIT(TIMx->CCER, TIM_CCER_CC4E);
		break;
	}
	SET_BIT(TIMx->CR1, TIM_CR1_CEN);

	// Enable output by setting the pin to AF PP mode
	// See section 9.1.11 of the reference manual
	gpio_set_mode(pin, GPIO_MODE_PP_AF, GPIO_FLOAT);

END:
	return;
}
void pwm_off(pin_t pin) {
	uint8_t channel, apb;
	uint16_t TIMxEN;
	TIM_TypeDef *TIMx;

	gpio_set_mode(pin, GPIO_MODE_HiZ, GPIO_FLOAT);

	channel = 4;
	switch (PINID(pin)) {
#if USE_TIMER1_PWM
	case PINID_TIM1_CH1:
		--channel;
		// fall through
	case PINID_TIM1_CH2:
		--channel;
		// fall through
	case PINID_TIM1_CH3:
		--channel;
		// fall through
	case PINID_TIM1_CH4:
		clock_enable(&RCC->APB2ENR, RCC_APB2ENR_TIM1EN);
		TIMx = TIM1;
		TIMxEN = RCC_APB2ENR_TIM1EN;
		apb = 2;
		break;
#endif
#if USE_TIMER2_PWM
	case PINID_TIM2_CH1:
		--channel;
		// fall through
	case PINID_TIM2_CH2:
		--channel;
		// fall through
	case PINID_TIM2_CH3:
		--channel;
		// fall through
	case PINID_TIM2_CH4:
		clock_enable(&RCC->APB1ENR, RCC_APB1ENR_TIM2EN);
		TIMx = TIM2;
		TIMxEN = RCC_APB1ENR_TIM2EN;
		apb = 1;
		break;
#endif
#if USE_TIMER3_PWM
	case PINID_TIM3_CH1:
		--channel;
		// fall through
	case PINID_TIM3_CH2:
		--channel;
		// fall through
	case PINID_TIM3_CH3:
		--channel;
		// fall through
	case PINID_TIM3_CH4:
		clock_enable(&RCC->APB1ENR, RCC_APB1ENR_TIM3EN);
		TIMx = TIM3;
		TIMxEN = RCC_APB1ENR_TIM3EN;
		apb = 1;
		break;
#endif
#if USE_TIMER4_PWM
	case PINID_TIM4_CH1:
		--channel;
		// fall through
	case PINID_TIM4_CH2:
		--channel;
		// fall through
	case PINID_TIM4_CH3:
		--channel;
		// fall through
	case PINID_TIM4_CH4:
		clock_enable(&RCC->APB1ENR, RCC_APB1ENR_TIM4EN);
		TIMx = TIM4;
		TIMxEN = RCC_APB1ENR_TIM4EN;
		apb = 1;
		break;
#endif

	default:
		LOGGER("Attempted PWM with incapable pin 0x%02X", (uint )pin);
		goto END;
		break;
	}

	switch (channel) {
	case 1:
		CLEAR_BIT(TIMx->CCER, TIM_CCER_CC1E);
		break;
	case 2:
		CLEAR_BIT(TIMx->CCER, TIM_CCER_CC2E);
		break;
	case 3:
		CLEAR_BIT(TIMx->CCER, TIM_CCER_CC3E);
		break;
	case 4:
		CLEAR_BIT(TIMx->CCER, TIM_CCER_CC4E);
		break;
	}

	//if (SELECT_BITS(TIMx->CCER, TIM_CCER_CC1E|TIM_CCER_CC2E|TIM_CCER_CC3E|TIM_CCER_CC4E) == 0) {
	if (TIMx->CCER == 0) {
		CLEAR_BIT(TIMx->CR1, TIM_CR1_CEN);
		if (apb == 1) {
			clock_disable(&RCC->APB1ENR, TIMxEN);
		} else {
			clock_disable(&RCC->APB2ENR, TIMxEN);
		}
	}

END:
	return;
}

#ifdef __cplusplus
 }
#endif
