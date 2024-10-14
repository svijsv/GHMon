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
// adc.c
// Manage the ADC peripheral
// NOTES:
//

#include "adc.h"
#include "system.h"
#include "gpio.h"

#include <avr/io.h>
#include <avr/power.h>

#if uHAL_USE_ADC

// There's only one possible ADC device, but to make future expansion easier
// let's do this
#define ADCx ADC0

#if ADC_MAX == 0x0FFU
# define ADC_RESSEL ADC_RESSEL_8BIT_gc
#elif ADC_MAX == 0x3FFU
# define ADC_RESSEL ADC_RESSEL_10BIT_gc
#else
# error "ADC_MAX must be 0x0FF or 0x3FF"
#endif

// There's hardware support for multiple samples but only in powers of 2
#define SAMPLE_BATCH 1
#if (ADC_SAMPLE_COUNT == 1)
# define ADC_SAMPNUM_ACC ADC_SAMPNUM_ACC1_gc
#elif (ADC_SAMPLE_COUNT == 2)
# define ADC_SAMPNUM_ACC ADC_SAMPNUM_ACC2_gc
#elif (ADC_SAMPLE_COUNT == 4)
# define ADC_SAMPNUM_ACC ADC_SAMPNUM_ACC4_gc
#elif (ADC_SAMPLE_COUNT == 8)
# define ADC_SAMPNUM_ACC ADC_SAMPNUM_ACC8_gc
#elif (ADC_SAMPLE_COUNT == 16)
# define ADC_SAMPNUM_ACC ADC_SAMPNUM_ACC16_gc
#elif (ADC_SAMPLE_COUNT == 32)
# define ADC_SAMPNUM_ACC ADC_SAMPNUM_ACC32_gc
#elif (ADC_SAMPLE_COUNT == 64)
# define ADC_SAMPNUM_ACC ADC_SAMPNUM_ACC64_gc
#else
# define ADC_SAMPNUM_ACC ADC_SAMPNUM_ACC1_gc
# undef SAMPLE_BATCH
# define SAMPLE_BATCH 0
#endif

#ifndef F_ADC
# if F_ADC_MAX >= (G_freq_ADCCLK / 2)
#  define F_ADC (G_freq_ADCCLK / 2U)
# elif F_ADC_MAX >= (G_freq_ADCCLK / 4)
#  define F_ADC (G_freq_ADCCLK / 4U)
# elif F_ADC_MAX >= (G_freq_ADCCLK / 8)
#  define F_ADC (G_freq_ADCCLK / 8U)
# elif F_ADC_MAX >= (G_freq_ADCCLK / 16)
#  define F_ADC (G_freq_ADCCLK / 16U)
# elif F_ADC_MAX >= (G_freq_ADCCLK / 32)
#  define F_ADC (G_freq_ADCCLK / 32U)
# elif F_ADC_MAX >= (G_freq_ADCCLK / 64)
#  define F_ADC (G_freq_ADCCLK / 64U)
# elif F_ADC_MAX >= (G_freq_ADCCLK / 128)
#  define F_ADC (G_freq_ADCCLK / 128U)
//# elif F_ADC_MAX >= (G_freq_ADCCLK / 256)
# else
#  define F_ADC (G_freq_ADCCLK / 256U)
# endif
#endif

#if F_ADC == (G_freq_ADCCLK / 2)
# define ADC_PRESC (0x00U)
#elif F_ADC == (G_freq_ADCCLK / 4)
# define ADC_PRESC (0x01U)
#elif F_ADC == (G_freq_ADCCLK / 8)
# define ADC_PRESC (0x02U)
#elif F_ADC == (G_freq_ADCCLK / 16)
# define ADC_PRESC (0x03U)
#elif F_ADC == (G_freq_ADCCLK / 32)
# define ADC_PRESC (0x04U)
#elif F_ADC == (G_freq_ADCCLK / 64)
# define ADC_PRESC (0x05U)
#elif F_ADC == (G_freq_ADCCLK / 128)
# define ADC_PRESC (0x06U)
#elif F_ADC == (G_freq_ADCCLK / 256)
# define ADC_PRESC (0x07U)
#else
# error "F_ADC must be F_CORE / (2|4|8|16|32|64|128|256)"
#endif

#define ADC_CYCLES_PER_uS (F_ADC / 1000000U)
//
// By default, the sample length is 2 ADC cycles. For each increment of
// SAMPLEN, that increases by 1 ADC cycle.
#define ADC_SAMPLEN_PRE ((ADC_CYCLES_PER_uS * ADC_SAMPLE_uS))
#if ADC_SAMPLEN_PRE < ADC_SAMPLEN_BASE
# undef ADC_SAMPLEN_PRE
# define ADC_SAMPLEN_PRE ADC_SAMPLEN_BASE
#endif
#define ADC_SAMPLEN (ADC_SAMPLEN_PRE - ADC_SAMPLEN_BASE)
#if ADC_SAMPLEN < 0
# undef ADC_SAMPLEN
# define ADC_SAMPLEN 0U
#elif ADC_SAMPLEN > ADC_SAMPLEN_MAX
# undef ADC_SAMPLEN
# define ADC_SAMPLEN ADC_SAMPLEN_MAX
#endif

#define ADC_SAMPLE_CYCLES (ADC_SAMPLEN + ADC_SAMPLEN_BASE)
#define ADC_SAMPLES_PER_S (F_ADC / (ADC_CONVERSION_CYCLES + ADC_SAMPLEN))

DEBUG_CPP_MACRO(F_ADC)
DEBUG_CPP_MACRO(ADC_PRESC)
DEBUG_CPP_MACRO(ADC_SAMPLE_CYCLES)
DEBUG_CPP_MACRO(ADC_SAMPLES_PER_S)


//
// Mapping of the various GPIO pins to their respective analog input channels
static const gpio_pin_t ain_table[] = {
	PINID_ADC0_AIN0,  PINID_ADC0_AIN1,  PINID_ADC0_AIN2,  PINID_ADC0_AIN3,
	PINID_ADC0_AIN4,  PINID_ADC0_AIN5,  PINID_ADC0_AIN6,  PINID_ADC0_AIN7,
	PINID_ADC0_AIN8,  PINID_ADC0_AIN9,  PINID_ADC0_AIN10, PINID_ADC0_AIN11,
	PINID_ADC0_AIN12, PINID_ADC0_AIN13, PINID_ADC0_AIN14, PINID_ADC0_AIN15,
};
#define NO_AIN_CHANNEL 16U

static adc_t adc_read_channel(uint8_t channel);

void adc_init(void) {
	uint8_t reg_tmp;

	ADCx.EVCTRL = 0;
	ADCx.INTCTRL = 0;

	ADCx.CTRLA = ADC_RESSEL;
	ADCx.CTRLB = ADC_SAMPNUM_ACC;

	reg_tmp = 0;
	// Use SAMPCAP for reference voltages >= 1V
	// Keep the voltage reference set to Vdd normally, it will be changed if
	// needed
	SET_BIT(reg_tmp, ADC_SAMPCAP_bm|ADC_REFSEL_VDDREF_gc);
	SET_BIT(reg_tmp, (ADC_PRESC << ADC_PRESC_gp));
	ADCx.CTRLC = reg_tmp;

	reg_tmp = 0;
	MODIFY_BITS(reg_tmp, ADC_INITDLY_gm, ADC_INITDLY);
	// The sampling delay variation is *probably* more of a problem (by slowing
	// down the sampling frequency) than a benefit...
	//SET_BIT(reg_tmp, ASDV_ENABLED);
	ADCx.CTRLD = reg_tmp;

	ADCx.CTRLE = 0;
	ADCx.SAMPCTRL = (ADC_SAMPLEN << ADC_SAMPLEN_gp);

	// 50% if ADC_CLK > 1.5MHz
	ADCx.CALIB = (F_ADC > 1500000U) ? ADC_DUTYCYC_DUTY50_gc : ADC_DUTYCYC_DUTY25_gc;

	// Reset all the interrupt flags
	ADCx.INTFLAGS = 0xFFU;

	adc_off();

	return;
}
err_t adc_on(void) {
	SET_BIT(ADCx.CTRLA, ADC_ENABLE_bm);

	return ERR_OK;
}
err_t adc_off(void) {
	CLEAR_BIT(ADCx.CTRLA, ADC_ENABLE_bm);

	return ERR_OK;
}

static uint8_t adc_find_pin_ain(gpio_pin_t pin) {
	uint8_t channel = 0;
	gpio_pin_t pinid;

	pinid = PINID(pin);
	for (channel = 0; channel < NO_AIN_CHANNEL; ++channel) {
		if (ain_table[channel] == pinid) {
			break;
		}
	}

	return channel;
}
adc_t adc_read_pin(gpio_pin_t pin) {
	uint8_t channel = 0;

	assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return ERR_ADC;
	}
#endif

	channel = adc_find_pin_ain(pin);
	if (channel == NO_AIN_CHANNEL) {
		return ERR_ADC;
	}

	return adc_read_channel(channel);
}
static adc_t adc_read_channel(uint8_t channel) {
	adcm_t adc;
#if ADC_TIMEOUT_MS
	utime_t timeout;
#endif

	// Five bits of channel selection
	//assert(channel <= 0x1FU);

	// Select the ADC channel to convert
	MODIFY_BITS(ADCx.MUXPOS, ADC_MUXPOS_gm, channel << ADC_MUXPOS_gp);

#if ADC_TIMEOUT_MS
	timeout = SET_TIMEOUT_MS(ADC_TIMEOUT_MS);
#endif

#if ! SAMPLE_BATCH
	adc = 0;
	for (uiter_t i = ADC_SAMPLE_COUNT; i != 0; --i) {
		// Start conversion
		SET_BIT(ADCx.COMMAND, ADC_STCONV_bm);
		while (!BIT_IS_SET(ADCx.INTFLAGS, ADC_RESRDY_bm)) {
# if ADC_TIMEOUT_MS
			if (TIMES_UP(timeout)) {
				return ERR_ADC;
			}
# endif
		}
		// RESRDY is cleared when the result register is read
		adc += read_reg16(&ADCx.RES);
	}

#else // !SAMPLE_BATCH
	// Start conversion
	SET_BIT(ADCx.COMMAND, ADC_STCONV_bm);
	while (!BIT_IS_SET(ADCx.INTFLAGS, ADC_RESRDY_bm)) {
# if ADC_TIMEOUT_MS
		if (TIMES_UP(timeout)) {
			return ERR_ADC;
		}
# endif
	}
	adc = read_reg16(&ADCx.RES);
#endif // !SAMPLE_BATCH
	adc /= ADC_SAMPLE_COUNT;

	return adc;
}
uint_fast16_t adc_read_vref_mV(void) {
	adc_t adc;

	// Measure the internal bandgap reference voltage
	adc = adc_read_channel(ADC_MUXPOS_INTREF_gc >> ADC_MUXPOS_gp);
	// Calculate the ADC Vref by comparing it to the internal Vref
	// adc / max = 1100mV / vref
	// (adc / max) * vref = 1100mV
	// vref = 1100mV / (adc / max)
	// vref = (1100mV * max) / adc
	//return ((adcm_t )INTERNAL_VREF_mV * (adcm_t )ADC_MAX) / (adcm_t )adc;
	return ((uint32_t )INTERNAL_VREF_mV * ADC_MAX) / adc;
}

adc_t adc_read_ac_amplitude(gpio_pin_t pin, uint32_t period_ms, adc_t *min, adc_t *max) {
	uint8_t channel = 0;
	adc_t adc, adc_min, adc_max;
	utime_t timeout;

	assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return ERR_ADC;
	}
#endif

	channel = adc_find_pin_ain(pin);
	if (channel == NO_AIN_CHANNEL) {
		return ERR_ADC;
	}
	// Select the ADC channel to convert
	MODIFY_BITS(ADCx.MUXPOS, ADC_MUXPOS_gm, channel << ADC_MUXPOS_gp);
	// Enable free-running mode
	SET_BIT(ADCx.CTRLA, ADC_FREERUN_bm);

	// The hardware batch reads won't do what we want here
#if SAMPLE_BATCH
	MODIFY_BITS(ADCx.CTRLB, ADC_SAMPNUM_gm, ADC_SAMPNUM_ACC1_gc);
#endif

	// Start conversion
	SET_BIT(ADCx.COMMAND, ADC_STCONV_bm);
	// RESRDY is cleared by writing 1 to it
	SET_BIT(ADCx.INTFLAGS, ADC_RESRDY_bm);

	adc_min = ADC_MAX;
	adc_max = 0;
	timeout = SET_TIMEOUT_MS(period_ms);
	while (!TIMES_UP(timeout)) {
		while (!BIT_IS_SET(ADCx.INTFLAGS, ADC_RESRDY_bm)) {
			// Nothing to do here
		}
		// RESRDY is cleared when the result register is read
		adc = read_reg16(&ADCx.RES);

		if (adc > adc_max) {
			adc_max = adc;
		}
		if (adc < adc_min) {
			adc_min = adc;
		}
	}

	// Disable free-running mode
	CLEAR_BIT(ADCx.CTRLA, ADC_FREERUN_bm);

#if SAMPLE_BATCH
	MODIFY_BITS(ADCx.CTRLB, ADC_SAMPNUM_gm, ADC_SAMPNUM_ACC);
#endif

	if (min != NULL) {
		*min = adc_min;
	}
	if (max != NULL) {
		*max = adc_max;
	}
	return (adc_max - adc_min)/2U;
}


#endif // uHAL_USE_ADC
