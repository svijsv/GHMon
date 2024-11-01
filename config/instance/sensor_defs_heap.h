//
// Example sensor configuration
//
// Each sensorr consists of at least a static configurations struct of type
// sensor_cfg_t and a read() function, with an optional init() function also
// possible.
//
// In order to keep the backend code simple while maintaining C99 compatibility,
// the functions used to initialize and read the sensors are defined in one
// section and an array of sensor configurations are defined in the second part.
//

#include "sensor_helpers.h"

#include "ulib/include/halloc.h"

uint16_t ADC_Vref_mV = REGULATED_VOLTAGE_mV;


//
// Section 1
// Initialization/Read Definitions
//

//
// Generic initializers
//
err_t vdiv_init(SENSOR_CFG_STORAGE struct sensor_cfg_t *cfg, sensor_status_t *status) {
	vdiv_helper_t *helper = halloc(sizeof(*helper));
	if (helper == NULL) {
		return ERR_NOMEM;
	}

	gpio_set_mode(cfg->pin, GPIO_MODE_AIN, GPIO_FLOAT);
	status->data = helper;

	return ERR_OK;
}
err_t thermistor_init(SENSOR_CFG_STORAGE struct sensor_cfg_t *cfg, sensor_status_t *status) {
	thermistor_helper_t *helper = halloc(sizeof(*helper));
	if (helper == NULL) {
		return ERR_NOMEM;
	}

	gpio_set_mode(cfg->pin, GPIO_MODE_AIN, GPIO_FLOAT);
	status->data = helper;
	return ERR_OK;
}

//
// Sensor 0, ADC voltage reference (Vcc)
//
sensor_reading_t* vcc_read(SENSOR_CFG_STORAGE struct sensor_cfg_t *cfg, sensor_status_t *status) {
	static sensor_reading_t reading = { 0 };

	bool enable_adc = (!adc_is_on());
	if (enable_adc) {
		adc_on();
	}

	ADC_Vref_mV = reading.value = adc_read_vref_mV();

	if (enable_adc) {
		adc_off();
	}

	return &reading;
}
//
// Sensor 1, Battery voltage
//
err_t battery_init(SENSOR_CFG_STORAGE struct sensor_cfg_t *cfg, sensor_status_t *status) {
	gpio_set_mode(cfg->pin, GPIO_MODE_AIN, GPIO_FLOAT);
	return ERR_OK;
}
sensor_reading_t* battery_read(SENSOR_CFG_STORAGE struct sensor_cfg_t *cfg, sensor_status_t *status) {
	static sensor_reading_t reading = { 0 };
	const uint32_t series_r1 = BATTERY_VDIV_HIGH_SIDE_OHMS;
	const uint32_t series_r2 = BATTERY_VDIV_LOW_SIDE_OHMS;

	if (series_r2 == 0) {
		return NULL;
	}

	bool enable_adc = (!adc_is_on());
	if (enable_adc) {
		adc_on();
	}

	adc_t adc_value = adc_read_pin(cfg->pin);

	if (enable_adc) {
		adc_off();
	}

	uint32_t corrected_value = (adc_value * (series_r1 + series_r2)) / series_r2;
	reading.value = adc_to_voltage(corrected_value, ADC_Vref_mV);

	return &reading;
}

//
// Section 2
// Sensor configuration
//
SENSOR_CFG_STORAGE sensor_cfg_t SENSORS[] = {
//
// Sensor 0, ADC voltage reference (Vcc)
// Check the device Vcc and update the ADC voltage reference value
//static SENSOR_CFG_STORAGE sensor_cfg_t ADC_vref = {
{
	.name = "Vcc",
	.init = NULL,
	.read = vcc_read,
	.pin = 0,
},
//
// Sensor 1, battery voltage
// Check the battery voltage
//static SENSOR_CFG_STORAGE sensor_cfg_t battery = {
{
	.name = "BAT",
	.init = battery_init,
	.read = battery_read,
	.pin = BATTERY_CHECK_PIN,
},
//
// Sensor 2, Indoor thermistor
// Check the temperature inside
//static SENSOR_CFG_STORAGE sensor_cfg_t inside_therm1 = {
{
	.name = "IN_TEMP1",
	.init = thermistor_init,
	.read = thermistor_read,
	.pin = INSIDE_THERM1_PIN,
},
//
// Sensor 3, Outdoor thermistor
// Check the temperature outside
//static SENSOR_CFG_STORAGE sensor_cfg_t outside_therm1 = {
/*
{
	.name = "OUT_TEMP1",
	.init = thermistor_init,
	.read = thermistor_read,
	.pin = OUTSIDE_THERM1_PIN,
},
*/
//
// Sensor 4, Soil moisture
// Check how wet the soil is
//static SENSOR_CFG_STORAGE sensor_cfg_t moist1 = {
{
	.name = "GND_MOIST1",
	.init = vdiv_init,
	.read = vdiv_ohms_read,
	.pin = GND_MOIST1_PIN,
	.cooldown_seconds = 120,
	.data = MOISTURE_SERIES_OHMS,
},
};
