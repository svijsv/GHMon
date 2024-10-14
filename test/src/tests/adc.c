#include "common.h"

#if TEST_ADC

//
// Globals initialization


//
// main() initialization
void init_ADC(void) {
	adc_on();

	gpio_set_mode(ADC_TEST_PIN, GPIO_MODE_AIN, GPIO_FLOAT);
	if (ADC_TEST_PIN_GND) {
		gpio_set_mode(ADC_TEST_PIN_GND, GPIO_MODE_AIN, GPIO_FLOAT);
	}
	if (ADC_TEST_PIN_VCC) {
		gpio_set_mode(ADC_TEST_PIN_VCC, GPIO_MODE_AIN, GPIO_FLOAT);
	}

	return;
}


//
// Main loop
void loop_ADC(void) {
	uint16_t vref = adc_read_vref_mV();
	adc_t adc;
	uint_t v;

	PRINTF("Vcc %umV\r\n", (uint_t )vref);

	adc = adc_read_pin(ADC_TEST_PIN);
	v = (adc * (uint32_t )vref) / ADC_MAX;
	PRINTF("ADC pin 0x%02X: %umV (%u * %umV / %u)\r\n", (uint_t )ADC_TEST_PIN, (uint_t )v, (uint_t )adc, (uint_t )vref, (uint_t )ADC_MAX);

	if (ADC_TEST_PIN_GND) {
		adc = adc_read_pin(ADC_TEST_PIN_GND);
		v = (adc * vref) / ADC_MAX;
		PRINTF("ADC pin 0x%02X (GND): %umV (%u)\r\n", (uint_t )ADC_TEST_PIN_GND, (uint_t )v, (uint_t )adc);
	}

	if (ADC_TEST_PIN_VCC) {
		adc = adc_read_pin(ADC_TEST_PIN_VCC);
		v = (adc * vref) / ADC_MAX;
		PRINTF("ADC pin 0x%02X (VCC): %umV (%u)\r\n", (uint_t )ADC_TEST_PIN_VCC, (uint_t )v, (uint_t )adc);
	}

	return;
}


#endif // TEST_ADC
