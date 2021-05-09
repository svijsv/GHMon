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
// adc.c
// Manage the ADC peripheral
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
/*
* Includes
*/
#include "adc.h"
#include "system.h"
#include "gpio.h"

#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/rcc.h>


#if USE_ADC

/*
* Static values
*/
// Datasheet value "Avg_Slope" converted from mV to uV
#define TEMP_INT_SLOPE 4300
// Datasheet value "V25" converted from V to mV
#define TEMP_INT_V25   1430
// Temperature sensor startup time specified as Tstart in the datasheet is
// 4-10us
#define TEMP_START_TIME_uS 10
// The recommended sample time for the internal temperature is 17.1us
#define TEMP_SAMPLE_uS     17

// ADC stabilization time is specified under Tstab in the datasheet as
// 1us max
#define ADC_STAB_TIME_uS 1

//#define SAMPLE_TIME ADC_SMPR_SMP_239DOT5CYC
#define SAMPLE_TIME ADC_SMPR_SMP_71DOT5CYC


/*
* Types
*/


/*
* Variables
*/


/*
* Local function prototypes
*/
static adc_t adc_read_channel(uint8_t channel);


/*
* Interrupt handlers
*/


/*
* Functions
*/
void adc_init(void) {
	rcc_periph_clock_enable(RCC_ADCx);
	adc_power_off(ADCx);
	rcc_periph_reset_pulse(RST_ADCx);

	adc_set_sample_time_on_all_channels(ADCx, SAMPLE_TIME);

	adc_set_dual_mode(ADC_CR1_DUALMOD_IND);
	adc_disable_scan_mode(ADCx);
	adc_set_single_conversion_mode(ADCx);
	adc_enable_external_trigger_regular(ADCx, ADC_CR2_EXTSEL_SWSTART);

	adc_power_on(ADCx);
	adc_reset_calibration(ADCx);
	adc_calibrate(ADCx);

	adc_power_off(ADCx);
	rcc_periph_clock_disable(RCC_ADCx);

	return;
}
void adc_on(void) {
	rcc_periph_clock_enable(RCC_ADCx);
	adc_power_on(ADCx);
	// ADC stabilization time is specified under Tstab in the datasheet as
	// 1us max
	dumb_delay_cycles(ADC_STAB_TIME_uS * (G_freq_HCLK/1000000U));

	return;
}
void adc_off(void) {
	adc_power_off(ADCx);
	rcc_periph_clock_disable(RCC_ADCx);

	return;
}

adc_t adc_read_pin(pin_t pin) {
	uint32_t channel;
	pin_t pinno;

	channel = 0; // Shut the compiler up

	pinno = GPIO_GET_PINNO(pin);
	switch (PINID(pin)) {
	case PINID_A0:
	case PINID_A1:
	case PINID_A2:
	case PINID_A3:
	case PINID_A4:
	case PINID_A5:
	case PINID_A6:
	case PINID_A7:
		channel = pinno;
		break;
	case PINID_B0:
	case PINID_B1:
		channel = pinno + 8;
		break;
	default:
		return 0;
		break;
	}

	return adc_read_channel(channel);
}
static adc_t adc_read_channel(uint8_t channel) {
	adcm_t adc;

	// Five bits of channel selection
	assert(channel <= 0b11111);

	adc_set_regular_sequence(ADCx, 1, &channel);

	adc = 0;
	for (uiter_t i = 0; i < ADC_SAMPLE_COUNT; ++i) {
		adc_start_conversion_regular(ADCx);
		while (!adc_eoc(ADCx)) {
			// Nothing to do here
		}
		adc += adc_read_regular(ADCx);
	}
	adc /= ADC_SAMPLE_COUNT;

	return adc;
}

int16_t adc_read_vref_mV(void) {
	adc_t adc;
	uint16_t vref;

#if ADCx_IS_ADC1
	// This enables both temperature and reference sensors
	adc_enable_temperature_sensor();

	adc = adc_read_channel(ADC_CHANNEL_VREF);
	// Calculate the ADC Vref by comparing it to the internal Vref
	// adc / max = 1200mV / vref
	// (adc / max) * vref = 1200mV
	// vref = 1200mV / (adc / max)
	// vref = (1200mV * max) / adc
	vref = (INTERNAL_VREF_mV * ADC_MAX) / (uint32_t )adc;

	adc_disable_temperature_sensor();

#else // !ADCx_IS_ADC1
	vref = REGULATED_VOLTAGE_mV;
#endif // ADCx_IS_ADC1

	return vref;
}

#endif // USE_ADC

#ifdef __cplusplus
 }
#endif
