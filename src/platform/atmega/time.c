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
//
// NOTES:
//    The systick timer is timer 2 (8-bit)
//
//    The micro-second timer is timer 0 (8-bit)
//
//    The PWM timer is either timer 0 (8-bit) or timer 1 (16-bit) depending
//    on the output pin. Timer 0 will conflict with anything using the micro-
//    second timer at the same time. Timer 2 supports PWM but there's no way
//    I can think of to use it at the same time as the systick timer.
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

#include "ulib/math.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <util/delay_basic.h>


// To keep the code smaller, only a few CPU frequencies are supported; there's
// commented code to support arbitrary frequencies in/near calc_timer2_1ms()
#if G_freq_TIM2CLK != 16000000 && G_freq_TIM2CLK != 8000000 && G_freq_TIM2CLK != 4000000 && G_freq_TIM2CLK != 2000000 && G_freq_TIM2CLK != 1000000
# error "CPU frequency must be 16MHz, 8MHz, 4MHz, 2MHz, or 1MHz"
#endif

/*
* Static values
*/
#define WDT_WAKEUP    0
#define WDT_CALIBRATE 1
// Using more cycles for calibration *should* reduce the overall error
// Since wdt_calibration is only 8 bits and needs to store the number of ms
// elapsed in this period, this shouldn't be more than WDTO_120MS to be on
// the safe side
#define WDT_CALIBRATE_CYCLES WDTO_60MS
// This is the number of times the WDT can be called before recalibration;
// right now it's only used in set_wakeup_alarm() and should fit in a uint16_t
// It's hard to associate this with a period of time because the duration of
// each sleep is different; 1000 works out to ~2 hours if every sleep is 8s
// and nothing else much happens
#define WDT_CALIBRATE_INTERVAL 200

#define TIM01_PRESCALER_1    0b001
#define TIM01_PRESCALER_8    0b010
#define TIM01_PRESCALER_64   0b011
#define TIM01_PRESCALER_256  0b100
#define TIM01_PRESCALER_1024 0b101

#define TIM2_PRESCALER_1    0b001
#define TIM2_PRESCALER_8    0b010
#define TIM2_PRESCALER_32   0b011
#define TIM2_PRESCALER_64   0b100
#define TIM2_PRESCALER_128  0b101
#define TIM2_PRESCALER_256  0b110
#define TIM2_PRESCALER_1024 0b111

// Timer 1 could have a wider range of frequencies if the 9 and 10 bit MAX
// values were checked but that would complicate things
#if ((G_freq_TIM01CLK/0xFF)/1) < PWM_MAX_FREQUENCY
# define TIM01_PWM_PRESCALER TIM01_PRESCALER_1
#elif ((G_freq_TIM01CLK/0xFF)/8) < PWM_MAX_FREQUENCY
# define TIM01_PWM_PRESCALER TIM01_PRESCALER_8
#elif ((G_freq_TIM01CLK/0xFF)/64) < PWM_MAX_FREQUENCY
# define TIM01_PWM_PRESCALER TIM01_PRESCALER_64
#elif ((G_freq_TIM01CLK/0xFF)/256) < PWM_MAX_FREQUENCY
# define TIM01_PWM_PRESCALER TIM01_PRESCALER_256
#elif ((G_freq_TIM01CLK/0xFF)/1024) < PWM_MAX_FREQUENCY
# define TIM01_PWM_PRESCALER TIM01_PRESCALER_1024
#else
# error "PWM_MAX_FREQUENCY is too low for this core clock frequency"
#endif


/*
* Types
*/


/*
* Variables
*/
volatile bool wakeup_alarm_is_set = false;

// System ticks, milliseconds
volatile utime_t G_sys_msticks = 0;
// 'RTC' ticks, seconds
static utime_t RTC_ticks = 0;
static utime_t RTC_prev_msticks = 0;
// It would be easier in theory to add a millisecond to the RTC by subtracting
// one from RTC_prev_msticks but the time spent sleeping is so much greater
// than the time spent awake that doing so would roll the counter over rather
// quickly, so a separate counter is needed for that
static utime_t RTC_millis = 0;

volatile uint8_t wdt_state;
volatile uint8_t wdt_calibration;

/*
* Local function prototypes
*/
static void systick_init(void);
static void WDT_init(void);
static void calibrate_WDT(void);
static void calc_timer2_1ms(uint8_t *cnt, uint8_t *psc);
//static uint16_t calc_timer2_Xms(uint16_t ms, uint8_t *cnt, uint8_t *psc);
static void timer0_init(void);
static void timer1_init(void);


/*
* Macros
*/
// Whether there's a remainder of a division by a power of two can be
// determined by shifting left until only the bits that would be shifted out
// for the division remain
#define HAS_REMAINDER(x, shift) (((x) << ((sizeof(x)<<3)-(shift))) != 0)

#define ENABLE_WDT_ISR() \
	do { \
	SET_BIT(WDTCSR, _BV(WDCE) | _BV(WDE)); \
	SET_BIT(WDTCSR, _BV(WDIE)); \
	CLEAR_BIT(WDTCSR, _BV(WDCE) | _BV(WDE)); \
	} while (0);

// The only timer interrupt we care about is OCIExA
#define ENABLE_TIMER_ISR(timsk)  (MODIFY_BITS(timsk, 0b111, 0b010))
#define DISABLE_TIMER_ISR(timsk) (CLEAR_BIT(timsk, 0b111))

#define DISABLE_COUNTER(tccrb)     (CLEAR_BIT(tccrb, 0b111))
#define SET_TIMER_PRESCALER(tccrb, psc) (MODIFY_BITS(tccrb, 0b111, psc))
#define RESET_TIMER10_PRESCALER() (SET_BIT(GTCCR, _BV(PSRSYNC)))
#define RESET_TIMER2_PRESCALER() (SET_BIT(GTCCR, _BV(PSRASY)))

#define SYSTICK_ISR TIMER2_COMPA_vect
#define SYSTICK_POWER_ENABLE()    power_timer2_enable()
#define SYSTICK_POWER_DISABLE()   power_timer2_disable()
#define SYSTICK_POWER_CHECK()     (!BIT_IS_SET(PRR, _BV(PRTIM2)))

#define USCOUNTER_POWER_ENABLE()    power_timer0_enable()
#define USCOUNTER_POWER_DISABLE()   power_timer0_disable()
#define USCOUNTER_POWER_CHECK()     (!BIT_IS_SET(PRR, _BV(PRTIM0)))


/*
* Interrupt handlers
*/
ISR(SYSTICK_ISR) {
	++G_sys_msticks;
}
ISR(WDT_vect) {
	wdt_disable();

	switch (wdt_state) {
	case WDT_WAKEUP:
		wakeup_alarm_is_set = false;
		break;

	case WDT_CALIBRATE:
		wdt_state = WDT_WAKEUP;
		break;
	}
}


/*
* Functions
*/
void time_init(void) {
	systick_init();
	WDT_init();
	timer0_init();
	timer1_init();

	return;
}
//
// Set up systick timer
static void systick_init(void) {
	uint8_t counter = 0, prescaler = 0;

	SYSTICK_POWER_ENABLE();
	DISABLE_COUNTER(TCCR2B);

	// Set the timer counter to reset when it reaches OCR2A
	MODIFY_BITS(TCCR2A, 0b11 << WGM20, 0b10 << WGM20);
	CLEAR_BIT(TCCR2B, 1 << WGM22);

	// Calculate the proper counter and prescaler to get 1ms
	calc_timer2_1ms(&counter, &prescaler);
	OCR2A = counter;
	// Setting the prescaler re-enables the counter
	SET_TIMER_PRESCALER(TCCR2B, prescaler);

	ENABLE_TIMER_ISR(TIMSK2);

	return;
}
//
// Calculating timer periods
//
// The available prescalers are limited to 1, 8, 64, 256, and 1024 for timers
// 0 and 1 and 1, 8, 32, 64, 128, 256, and 1024 for timer 2 so accurate timing
// can only happen for a subset of frequencies; the rest need to be
// approximated
//
// Starting at the highest prescaler and working down will normally give us
// the smallest error because that error exists only in the counter part of
// the calculation; the exception is when the clock frequency is divisible by
// the prescaler, which is more likely with smaller prescalers because each
// prescaler is a factor of the larger prescalers
//
// It's embarrasing how long this took to figure out, but in order to match
// timer cycles to time, where Ct is the number of clock cycles per timer
// timer cycle and Cs is the number of clock cycles per second:
//    Ct = psc * cnt
//    Cs = CLK_FREQ
//    1s = Ct/Cs
//    1s = (psc * cnt) / CLK_FREQ
// For 1 millisecond, Cs is divided by 1000:
//    1ms = (psc*cnt) / (CLK_FREQ/1000)
// For a period of x milliseconds, multiply the clock frequency:
//    1 period = (psc*cnt) / ((CLK_FREQ*x)/1000)
// The timer's TCNT will actually by cnt-1
//
static void calc_timer2_1ms(uint8_t *cnt, uint8_t *psc) {
	switch (G_freq_TIM2CLK) {
	case 16000000:
		*psc = TIM2_PRESCALER_64;
		*cnt = 249;
		break;
	case 8000000:
		*psc = TIM2_PRESCALER_64;
		*cnt = 124;
		break;
	case 4000000:
		*psc = TIM2_PRESCALER_32;
		*cnt = 124;
		// *psc = TIM01_PRESCALER_1024;
		// *cnt = 3; // Actually 2.90625
		break;
	case 2000000:
		*psc = TIM2_PRESCALER_8;
		*cnt = 249;
		break;
	case 1000000:
		*psc = TIM2_PRESCALER_8;
		*cnt = 124;
		break;
	/*
	default:
		calc_timer2_Xms(1, cnt, psc);
		return;
	*/
	}

	return;
}
/*
static uint16_t calc_timer2_Xms(uint16_t ms, uint8_t *cnt, uint8_t *psc) {
	uint16_t test;
	uint32_t clk_mHz;
	static _FLASH const uint16_t prescalers[] = { 1, 8, 32, 64, 128, 256, 1024 };
	static _FLASH const uint8_t  psc_bits[] = {
		TIM2_PRESCALER_1,
		TIM2_PRESCALER_8,
		TIM2_PRESCALER_32,
		TIM2_PRESCALER_64,
		TIM2_PRESCALER_128,
		TIM2_PRESCALER_256,
		TIM2_PRESCALER_1024
	};

	assert(ms  != 0);
	assert(cnt != NULL);
	assert(psc != NULL);

	clk_mHz = G_freq_TIM2CLK / 1000;

	//
	// Make sure we can even achieve the needed ms, return what we *can* do
	// otherwise
	//
	test = (((0xFF+1) * 1024UL) / clk_mHz);
	if (ms > test) {
		*psc = TIM2_PRESCALER_1024;
		*cnt = 0xFF;
		return test;
	}
	clk_mHz *= ms;

	//
	// See if we can get an exact match
	//
	// We can automatically exclude any higher prescalers if a smaller one
	// isn't a factor of the clock frequency because each prescaler is a
	// factor of all larger prescalers
	for (uiter_t i = 0; i < 7; ++i) {
		if ((clk_mHz % prescalers[i]) != 0) {
			break;
		}

		test = (clk_mHz / prescalers[i]);
		if ((test > 0) && (test <= (0xFF+1))) {
			*psc = psc_bits[i];
			*cnt = test-1;
			return ms;
		}
	}

	//
	// If there's no exact factor among the prescalers, just get as close as
	// we can starting at the highest prescaler to reduce compounding errors
	//
	for (iter_t i = 6; i != -1; --i) {
		// Add half the prescaler to the numerator to round to the nearest
		// integer instead of truncating
		test = ((clk_mHz+(prescalers[i]/2)) / prescalers[i]);
		if ((test > 0) && (test <= (0xFF+1))) {
			*psc = psc_bits[i];
			*cnt = test-1;
			return ms;
		}
	}

	return 0;
}
*/
void enable_systick(void) {
	SYSTICK_POWER_ENABLE();
	//RTC_POWER_ENABLE();

	return;
}
void disable_systick(void) {
	SYSTICK_POWER_DISABLE();
	//RTC_POWER_DISABLE();

	return;
}
//
// Set up the watchdog timer used to wake from sleep
static void WDT_init(void) {
	calibrate_WDT();

	return;
}
// If the accuracy of this time-keeping mattered more, I would set up an
// independent calibration timer and do more in the WDT ISR; that was adding
// unecessary complexity for something which shouldn't be *that* important
static void calibrate_WDT(void) {
	utime_t pre_calib, post_calib;
	uint8_t sreg;

	assert(SYSTICK_POWER_CHECK() && BIT_IS_SET(SREG, 0x80));

	wdt_state = WDT_CALIBRATE;

	DISABLE_INTERRUPTS(sreg);
	wdt_reset();
	wdt_enable(WDT_CALIBRATE_CYCLES);
	ENABLE_WDT_ISR();
	// Enable interrupts without concern for initial state, which is already
	// saved
	sei();
	READ_VOLATILE(pre_calib, G_sys_msticks);
	wdt_reset();

	do {
		// Nothing to do here
	} while (wdt_state != WDT_WAKEUP);

	// Because we don't know either the initial state of the systick timer
	// or where exactly it is now, we're going to end up overcounting on
	// one end and undercounting on the other; but because we could see
	// tick increments during this read and have to deal ISR overhead, we're
	// mostly going to overcount so subtract 1ms to (hopefully) keep things a
	// little closer to reality
	READ_VOLATILE(post_calib, G_sys_msticks);
	wdt_calibration = (uint8_t )(post_calib - pre_calib) - 1;

	RESTORE_INTERRUPTS(sreg);

	return;
}
//
// For convenience sake, set_wakeup_alarm() helps keep track of RTC_ticks
uint16_t set_wakeup_alarm(uint16_t ms) {
	static uint16_t calib_counter = WDT_CALIBRATE_INTERVAL;
	uint16_t period_ms;
	uint8_t shifts, adjust_ms = 0;

	if (ms == 0) {
		return 0;
	}

	wakeup_alarm_is_set = false;

	// Don't count for recalibration if we're not waiting long
	if (ms > 500) {
		--calib_counter;

		if (calib_counter == 0) {
			calib_counter = WDT_CALIBRATE_INTERVAL;
			calibrate_WDT();

			adjust_ms = wdt_calibration;
			if (ms > adjust_ms) {
				ms -= adjust_ms;
			} else {
				// Returning a period without having set wakeup_alarm_is_set should
				// cause the caller to skip a loop
				return ms;
			}
		}
	}

	// Each watchdog time period is double the one before it, so it's easy
	// to figure out which we should use given only the wdt_calibration value
	// Start by checking the highest duration and work down from there
	shifts = 0;
	period_ms = (uint16_t )wdt_calibration << (WDTO_8S - WDT_CALIBRATE_CYCLES);

	while ((period_ms > ms) && (shifts <= WDTO_8S)) {
		++shifts;
		period_ms >>= 1;
	}
	if (shifts > WDTO_8S) {
		period_ms = 0;
	}

	if (period_ms != 0) {
		uint8_t sreg;

		wakeup_alarm_is_set = true;

		DISABLE_INTERRUPTS(sreg);
		wdt_reset();
		wdt_enable(WDTO_8S-shifts);
		ENABLE_WDT_ISR();
		RESTORE_INTERRUPTS(sreg);

		// When sleep is interrupted (specifically by a button press), this will
		// result in the system time being ahead of real time
		add_RTC_millis(period_ms);
	}

	return period_ms + adjust_ms;
}
void stop_wakeup_alarm(void) {
	wdt_reset();
	wdt_disable();
	wakeup_alarm_is_set = false;

	return;
}
//
// Manage the fake RTC system
utime_t get_RTC_seconds(void) {
	utime_t msticks;

	READ_VOLATILE(msticks, G_sys_msticks);
	RTC_millis += (msticks - RTC_prev_msticks);
	RTC_prev_msticks = msticks;

	// This should happen close enough to every second that repeated subtraction
	// will be faster than division
	while (RTC_millis > 1000) {
		++RTC_ticks;
		RTC_millis -= 1000;
	}

	return RTC_ticks;
}
err_t set_RTC_seconds(utime_t s) {
	RTC_ticks = s;
	READ_VOLATILE(RTC_prev_msticks, G_sys_msticks);
	RTC_millis = 0;

	return EOK;
}
void add_RTC_millis(uint16_t ms) {
	RTC_millis += ms;

	return;
}
//
// Initialize the general-purpose timers
static void timer0_init(void) {
	power_timer0_enable();
	DISABLE_COUNTER(TCCR0B);

	// Normal mode; no PWM and the timer counter resets when it reaches OxFF
	CLEAR_BIT(TCCR0A, 0b11 << WGM00);
	CLEAR_BIT(TCCR0B, 0b1  << WGM02);

	OCR0A = 0;
	OCR0B = 0;

	power_timer0_disable();

	return;
}
static void timer1_init(void) {
	power_timer1_enable();
	DISABLE_COUNTER(TCCR1B);

	// Normal mode; no PWM and the timer counter resets when it reaches OxFFFF
	CLEAR_BIT(TCCR1A, 0b11 << WGM10);
	CLEAR_BIT(TCCR1B, 0b11 << WGM12);

	OCR1A = 0;
	OCR1B = 0;

	power_timer1_disable();

	return;
}
//
// Set up the micro-second timer; assume it's already set to the default of
// normal mode
void uscounter_on(void) {
	USCOUNTER_POWER_ENABLE();

	return;
}
void uscounter_off(void) {
	USCOUNTER_POWER_DISABLE();

	return;
}
//
// Set up a PWM timer
static void timer0_pwm_on(pin_t pin, uint8_t off_at) {
	power_timer0_enable();
	// Don't disable the counter in case the other output is already in
	// use; this may result in the first cycle being too long
	//DISABLE_COUNTER(TCCR0B);

	// Fast PWM mode; timer counter resets when it reaches OxFF
	MODIFY_BITS(TCCR0A, 0b11 << WGM00, 0b11 << WGM00);
	MODIFY_BITS(TCCR0B, 0b1  << WGM02, 0b0  << WGM02);

	if (PINID(pin) == PINID_OC0A) {
		OCR0A = off_at;
		MODIFY_BITS(TCCR0A, 0b11 << COM0A0, 0b10 << COM0A0);
	} else {
		OCR0B = off_at;
		MODIFY_BITS(TCCR0A, 0b11 << COM0B0, 0b10 << COM0B0);
	}
	// Enable the timer by setting the prescaler
	SET_TIMER_PRESCALER(TCCR0B, TIM01_PWM_PRESCALER);

	return;
}
static void timer1_pwm_on(pin_t pin, uint8_t off_at) {
	power_timer1_enable();
	// Don't disable the counter in case the other output is already in
	// use; this may result in the first cycle being too long
	//DISABLE_COUNTER(TCCR1B);

	// Fast PWM mode, 8 bit mode; timer counter resets when it reaches OxFF
	MODIFY_BITS(TCCR1A, 0b11 << WGM10, 0b01 << WGM10);
	MODIFY_BITS(TCCR1B, 0b11 << WGM12, 0b01 << WGM12);
	// Fast PWM mode, 10 bit mode; timer resets when it reaches Ox3FF
	//MODIFY_BITS(TCCR1A, 0b11 << WGM10, 0b11 << WGM10);
	//MODIFY_BITS(TCCR1B, 0b11 << WGM12, 0b01 << WGM12);

	if (PINID(pin) == PINID_OC1A) {
		OCR1A = off_at;
		MODIFY_BITS(TCCR1A, 0b11 << COM1A0, 0b10 << COM1A0);
	} else {
		OCR1B = off_at;
		MODIFY_BITS(TCCR1A, 0b11 << COM1B0, 0b10 << COM1B0);
	}
	// Enable the timer by setting the prescaler
	SET_TIMER_PRESCALER(TCCR1B, TIM01_PWM_PRESCALER);

	return;
}
void pwm_on(pin_t pin, uint16_t duty_cycle) {
	uint8_t off_at;

	assert(duty_cycle <= PWM_DUTY_CYCLE_SCALE);

	off_at = (duty_cycle * 0xFF) / PWM_DUTY_CYCLE_SCALE;

	switch (PINID(pin)) {
	case PINID_OC0A:
	case PINID_OC0B:
		timer0_pwm_on(pin, off_at);
		break;
	case PINID_OC1A:
	case PINID_OC1B:
		timer1_pwm_on(pin, off_at);
		break;
	default:
		LOGGER("Attempted PWM with incapable pin 0x%02X", (uint )pin);
		break;
	}

	// Enable output by setting DDR for the output pin
	gpio_set_mode(pin, GPIO_MODE_PP, GPIO_FLOAT);

	return;
}
static void timer0_pwm_off(pin_t pin) {
	if (PINID(pin) == (PINID_OC0A)) {
		CLEAR_BIT(TCCR0A, 0b11 << COM0A0);
		OCR0A = 0;
	} else {
		CLEAR_BIT(TCCR0A, 0b11 << COM0B0);
		OCR0B = 0;
	}
	if ((OCR0A == 0) && (OCR0B == 0)) {
		timer0_init();
	}

	return;
}
static void timer1_pwm_off(pin_t pin) {
	if (PINID(pin) == (PINID_OC1A)) {
		CLEAR_BIT(TCCR1A, 0b11 << COM1A0);
		OCR1A = 0;
	} else {
		CLEAR_BIT(TCCR1A, 0b11 << COM1B0);
		OCR1B = 0;
	}
	if ((OCR1A == 0) && (OCR1B == 0)) {
		timer1_init();
	}

	return;
}
void pwm_off(pin_t pin) {
	switch (PINID(pin)) {
	case PINID_OC0A:
	case PINID_OC0B:
		timer0_pwm_off(pin);
		break;
	case PINID_OC1A:
	case PINID_OC1B:
		timer1_pwm_off(pin);
		break;
	}

	return;
}

void delay_ms(utime_t ms) {
	utime_t timer;

	timer = SET_TIMEOUT(ms);
	while (!TIMES_UP(timer)) {
		// Nothing to do here
	}

	return;
}
void dumb_delay_ms(utime_t ms) {
	utime_t count;

	// 4 cycles per iteration in _delay_loop_2(), plus a little for the
	// overhead of dumb_delay_ms() itself
	count = (G_freq_CORECLK/5000) * ms;
	while (count > 0xFFFF) {
		_delay_loop_2(0xFFFF);
		count -= 0xFFFF;
	}
	_delay_loop_2(count);

	return;
}
void dumb_delay_cycles(uint32_t cycles) {
	uint32_t i;

	// Compensate a bit for how much longer than a clock cycle each iteration
	// takes by dividing the number of cycles by 8
	for (i = SHIFT_DIV_8(cycles); i != 0; --i) {
		// Count some clock cycles
		__asm__ volatile("" : "+g" (i) : :);
	}

	return;
}


#ifdef __cplusplus
 }
#endif
