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


/*
* Types
*/


/*
* Variables
*/
uint32_t G_freq_ADC;


/*
* Local function prototypes
*/
static uint32_t calculate_prescaler(uint32_t max_hz);
static uint8_t calculate_sample_rate(uint32_t uS);
static adc_t adc_read_channel(uint8_t channel, utime_t timeout);


/*
* Interrupt handlers
*/


/*
* Functions
*/
void adc_init(void) {
	uint32_t prescaler;

	rcc_periph_clock_enable(RCC_ADCx);
	adc_power_off(ADCx);
	rcc_periph_reset_pulse(RST_ADCx);

	// Max 14MHz on STM32F1
	prescaler = calculate_prescaler(14000000);
	rcc_set_adcpre(prescaler);
	switch (prescaler) {
		case RCC_CFGR_ADCPRE_PCLK2_DIV2:
			G_freq_ADC = rcc_apb2_frequency/2;
			break;
		case RCC_CFGR_ADCPRE_PCLK2_DIV4:
			G_freq_ADC = rcc_apb2_frequency/4;
			break;
		case RCC_CFGR_ADCPRE_PCLK2_DIV6:
			G_freq_ADC = rcc_apb2_frequency/6;
			break;
		case RCC_CFGR_ADCPRE_PCLK2_DIV8:
			G_freq_ADC = rcc_apb2_frequency/8;
			break;
		default:
			break;
	}
	adc_set_sample_time_on_all_channels(ADCx, calculate_sample_rate(ADC_SAMPLE_TIME));
#if ADCx_IS_ADC1
	adc_set_sample_time(ADCx, ADC_CHANNEL_TEMP, calculate_sample_rate(TEMP_SAMPLE_uS));
#endif

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
	dumber_delay(ADC_STAB_TIME_uS * (rcc_ahb_frequency/1000000U));

	return;
}
void adc_off(void) {
	rcc_periph_clock_disable(RCC_ADCx);
	adc_power_off(ADCx);

	return;
}
static uint32_t calculate_prescaler(uint32_t max_hz) {
	uint32_t prescaler;

	if ((rcc_apb2_frequency / 2) < max_hz) {
		prescaler = RCC_CFGR_ADCPRE_PCLK2_DIV2;
	} else if ((rcc_apb2_frequency / 4) < max_hz) {
		prescaler = RCC_CFGR_ADCPRE_PCLK2_DIV4;
	} else if ((rcc_apb2_frequency / 6) < max_hz) {
		prescaler = RCC_CFGR_ADCPRE_PCLK2_DIV6;
	} else {
		prescaler = RCC_CFGR_ADCPRE_PCLK2_DIV8;
	}

	return prescaler;
}
static uint8_t calculate_sample_rate(uint32_t uS) {
	uint8_t tmp;

	tmp = (G_freq_ADC * uS) / 1000000;
	if (tmp > 72) {
		tmp = ADC_SMPR_SMP_239DOT5CYC;
	} else if (tmp > 56) {
		tmp = ADC_SMPR_SMP_71DOT5CYC;
	} else if (tmp > 42) {
		tmp = ADC_SMPR_SMP_55DOT5CYC;
	} else if (tmp > 29) {
		tmp = ADC_SMPR_SMP_41DOT5CYC;
	} else if (tmp > 14) {
		tmp = ADC_SMPR_SMP_28DOT5CYC;
	} else if (tmp > 8) {
		tmp = ADC_SMPR_SMP_13DOT5CYC;
	} else if (tmp > 2) {
		tmp = ADC_SMPR_SMP_7DOT5CYC;
	} else {
		tmp = ADC_SMPR_SMP_1DOT5CYC;
	}

	return tmp;
}

adc_t adc_read_pin(pin_t pin, utime_t timeout) {
	uint32_t channel;
	pin_t pinno;

	channel = 0; // Shut the compiler up

	pinno = GPIO_GET_PINNO(pin);
	switch (pin) {
	case (GPIO_PORTA|GPIO_PINNO(0)):
	case (GPIO_PORTA|GPIO_PINNO(1)):
	case (GPIO_PORTA|GPIO_PINNO(2)):
	case (GPIO_PORTA|GPIO_PINNO(3)):
	case (GPIO_PORTA|GPIO_PINNO(4)):
	case (GPIO_PORTA|GPIO_PINNO(5)):
	case (GPIO_PORTA|GPIO_PINNO(6)):
	case (GPIO_PORTA|GPIO_PINNO(7)):
		channel = pinno;
		break;
	case (GPIO_PORTB|GPIO_PINNO(0)):
	case (GPIO_PORTB|GPIO_PINNO(1)):
		channel = pinno + 8;
		break;
	default:
		return 0;
		break;
	}

	return adc_read_channel(channel, timeout);
}
static adc_t adc_read_channel(uint8_t channel, utime_t timeout) {
	adcm_t adc;

	// Five bits of channel selection
	assert(channel <= 0b11111);

	timeout = SET_TIMEOUT(timeout);
	adc_set_regular_sequence(ADCx, 1, &channel);

	adc = 0;
	for (uiter_t i = 0; i < ADC_SAMPLE_COUNT; ++i) {
		adc_start_conversion_regular(ADCx);
		while (!adc_eoc(ADCx)) {
			if (TIMES_UP(timeout)) {
				goto END;
			}
		}
		adc += adc_read_regular(ADCx);
	}
	adc /= ADC_SAMPLE_COUNT;

END:
	return adc;
}

void adc_read_internals(int16_t *vref, int16_t *tempCx10) {
	adc_t adc;

	assert(vref != NULL);
	assert(tempCx10 != NULL);

#if ADCx_IS_ADC1
	// This enables both temperature and reference sensors
	adc_enable_temperature_sensor();
	// Rely on the overhead between here and measuring for the startup delay
	//dumber_delay(TEMP_START_TIME_uS * (freq_CoreCLK/1000000U));

	adc = adc_read_channel(ADC_CHANNEL_VREF, 1000);
	// Calculate the ADC Vref by comparing it to the internal Vref
	// adc / max = 1200mV / vref
	// (adc / max) * vref = 1200mV
	// vref = 1200mV / (adc / max)
	// vref = (1200mV * max) / adc
	*vref = (INTERNAL_VREF * ADC_MAX) / (uint32_t )adc;

	adc = adc_read_channel(ADC_CHANNEL_TEMP, 1000);
	// adc/max = v/vref
	// (adc/max)*vref = v
	// v = (adc*vref)/max
	// From the reference manual:
	// tempC = ((V25-Vsense)/avg_slope)+25
	*tempCx10 = (((TEMP_INT_V25 - ((adc * *vref) / ADC_MAX)) * 1000) / TEMP_INT_SLOPE) + 25;

	adc_disable_temperature_sensor();

#else // !ADCx_IS_ADC1
	*vref = REGULATED_VOLTAGE;
	*tempCx10 = 0;
#endif // ADCx_IS_ADC1

	return;
}

#endif // USE_ADC

#ifdef __cplusplus
 }
#endif
