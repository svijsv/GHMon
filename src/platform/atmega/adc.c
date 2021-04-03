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

#include <avr/io.h>
#include <avr/power.h>


#if USE_ADC

/*
* Static values
*/
#define VREF_MASK (_BV(REFS1)|_BV(REFS0))
#define VREF_Vbg  (_BV(REFS1)|_BV(REFS0))
#define VREF_Vcc  (_BV(REFS0))

#define CHANNEL_MASK (_BV(MUX3)|_BV(MUX2)|_BV(MUX1)|_BV(MUX0))
#define CHANNEL_T   0b1000
#define CHANNEL_Vbg 0b1110

#define PRESCALER_MASK   (0b111 << ADPS0)
//#define PRESCALER_DIV2   (0b000 << ADPS0) //There is no scaling option < 2
#define PRESCALER_DIV2   (0b001 << ADPS0)
#define PRESCALER_DIV4   (0b010 << ADPS0)
#define PRESCALER_DIV8   (0b011 << ADPS0)
#define PRESCALER_DIV16  (0b100 << ADPS0)
#define PRESCALER_DIV32  (0b101 << ADPS0)
#define PRESCALER_DIV64  (0b110 << ADPS0)
#define PRESCALER_DIV128 (0b111 << ADPS0)

// mV of the internal temperature sensor at 25C, taken from datasheet
#define TEMP_INT_V25 314
// Change in mV of internal temperature sensor when temperature increases 1C
#define TEMP_INT_SLOPE 1


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
static adc_t adc_read_channel(uint8_t channel);


/*
* Macros
*/


/*
* Interrupt handlers
*/


/*
* Functions
*/
void adc_init(void) {
	power_adc_enable();

	// Set the clock prescaler
	// Per the data sheet the frequency needs to be between 50KHz and 200KHz
	// to get the maximum resolution, which means we probably can't hit the
	// desired sampling time, but we can try
	// There are 13 cycles in a normal conversion and the target sample time
	// is micro seconds, so:
	//    13/Freq = T/1000000
	//    Freq = 13000000/T
	// But trying to figure that out while staying in the 50KHz-200KHz range
	// is complicated so right now just aim for ~100KHz (really 93KHz)
	// TODO: Calculate proper frequency for ADC_SAMPLE_TIME
#if (G_freq_ADCCLK > 12000000)
	G_freq_ADC = G_freq_ADCCLK / 128;
	MODIFY_BITS(ADCSRA, PRESCALER_MASK, PRESCALER_DIV128);
#elif (G_freq_ADCCLK > 6000000)
	G_freq_ADC = G_freq_ADCCLK / 64;
	MODIFY_BITS(ADCSRA, PRESCALER_MASK, PRESCALER_DIV64);
#elif (G_freq_ADCCLK > 3000000)
	G_freq_ADC = G_freq_ADCCLK / 32;
	MODIFY_BITS(ADCSRA, PRESCALER_MASK, PRESCALER_DIV32);
#elif (G_freq_ADCCLK > 1500000)
	G_freq_ADC = G_freq_ADCCLK / 16;
	MODIFY_BITS(ADCSRA, PRESCALER_MASK, PRESCALER_DIV16);
#elif (G_freq_ADCCLK > 750000)
	G_freq_ADC = G_freq_ADCCLK / 8;
	MODIFY_BITS(ADCSRA, PRESCALER_MASK, PRESCALER_DIV8);
#elif (G_freq_ADCCLK > 300000)
	G_freq_ADC = G_freq_ADCCLK / 4;
	MODIFY_BITS(ADCSRA, PRESCALER_MASK, PRESCALER_DIV4);
#else
	G_freq_ADC = G_freq_ADCCLK / 2;
	MODIFY_BITS(ADCSRA, PRESCALER_MASK, PRESCALER_DIV2);
#endif

	// Keep the voltage reference set to Vcc normally, it will be changed if
	// needed
	MODIFY_BITS(ADMUX, VREF_MASK, VREF_Vcc);

	power_adc_disable();

	return;
}
void adc_on(void) {
	power_adc_enable();
	SET_BIT(ADCSRA, _BV(ADEN));

	return;
}
void adc_off(void) {
	// If ADEN isn't cleared before shutting the power off, the ADC will get
	// frozen in an active state
	CLEAR_BIT(ADCSRA, _BV(ADEN));
	power_adc_disable();

	return;
}

adc_t adc_read_pin(pin_t pin) {
	uint8_t channel;
	pin_t pinno;

	channel = 0; // Shut the compiler up

	pinno = GPIO_GET_PINNO(pin);
	switch (PINID(pin)) {
	case PINID_C0:
	case PINID_C1:
	case PINID_C2:
	case PINID_C3:
	case PINID_C4:
	case PINID_C5:
		channel = pinno;
		break;
	case PINID_Z0:
	case PINID_Z1:
		channel = (pinno + 6);
		break;
	default:
		return 0;
		break;
	}

	return adc_read_channel(channel);
}
static adc_t adc_read_channel(uint8_t channel) {
	adc_t tmp;
	adcm_t adc;

	// Four bits of channel selection
	assert(channel <= 0x0F);

	// Select the ADC channel to convert
	MODIFY_BITS(ADMUX, CHANNEL_MASK, channel << MUX0);
	// The readings are inaccurate without this delay()
	delay(5);

	adc = 0;
	for (uiter_t i = ADC_SAMPLE_COUNT; i != 0; --i) {
		// Start conversion
		SET_BIT(ADCSRA, _BV(ADSC));
		while (BIT_IS_SET(ADCSRA, _BV(ADSC))) {
			// Nothing to do here
		}
		// Must read ADCH after reading ADCL, reading ADCL locks the registers
		// and reading ADCH unlocks them
		tmp  = ADCL;
		tmp |= (adc_t )ADCH << 8;
		adc += tmp;
	}
	adc /= ADC_SAMPLE_COUNT;

	return adc;
}
void adc_read_internals(int16_t *vref, int16_t *tempCx10) {
	adcm_t adc;

	assert(vref != NULL);
	assert(tempCx10 != NULL);

	// Measure the internal bandgap reference voltage
	adc = adc_read_channel(CHANNEL_Vbg);
	// Calculate the ADC Vref by comparing it to the internal Vref
	// adc / max = 1100mV / vref
	// (adc / max) * vref = 1100mV
	// vref = 1100mV / (adc / max)
	// vref = (1100mV * max) / adc
	*vref = ((uint32_t )INTERNAL_VREF * (uint32_t )ADC_MAX) / adc;

	// Per the data sheet, the voltage reference must be the internal bandgap
	// reference for temperature measurement
	MODIFY_BITS(ADMUX, VREF_MASK, VREF_Vbg);
	adc = adc_read_channel(CHANNEL_T);
	MODIFY_BITS(ADMUX, VREF_MASK, VREF_Vcc);
	// The temperature sensor voltage increases by approximately 1mV/C with an
	// accuracy of +/-10C
	// adc/max = v/vref
	// v = (adc*vref)/max
	*tempCx10 = (((((adc * INTERNAL_VREF)/ADC_MAX) - TEMP_INT_V25) / TEMP_INT_SLOPE) + 25) * 10;

	return;
}


#endif // USE_ADC

#ifdef __cplusplus
 }
#endif