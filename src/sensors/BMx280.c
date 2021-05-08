// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                       *
*                                                                       *
* Copyright (C) 2021 svijsv
* This program is free software: you can redistribute it and/or modify  *
* it under the terms of the GNU General Public License as published by  *
* the Free Software Foundation, version 3.                              *
*                                                                       *
* This program is distributed in the hope that it will be useful, but   *
* WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU      *
* General Public License for more details.                              *
*                                                                       *
* You should have received a copy of the GNU General Public License     *
* along with this program. If not, see <https://www.gnu.org/licenses/>. *
*                                                                       *
*                                                                       *
***********************************************************************/
// BMx280.c
// Manage BME280 and BMP280 sensors
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
//#define NDEBUG 1

//#include "bmx280.h"
#include "sensors.h"
#include "private.h"

#include "ulib/time.h"


#if USE_BMx280_SPI_SENSORS || USE_BMx280_I2C_SENSORS
typedef struct {
	int32_t temp, press, hum;
} bmx280_status_t;
typedef struct {
	uint8_t adc[8];
	uint8_t cal[33];
} bmx280_raw_t;


static void calculate_bmx280(bmx280_status_t *status, bmx280_raw_t *raw, bool do_humidity);
static uint16_t sensor_read_bmx280(uiter_t si, bool use_spi);


#if USE_BMx280_SPI_SENSORS
void sensor_init_bmx280_spi(uiter_t si) {
	SET_BIT(G_sensors[si].iflags, SENS_FLAG_SPI);

#if ! SPI_POWER_PIN
	// The CS pin needs to be pulled low once to put the device into
	// SPI mode
	gpio_set_mode(SENSORS[si].pin, GPIO_MODE_PP, GPIO_LOW);
	gpio_set_state(SENSORS[si].pin, GPIO_HIGH);
#endif

	return;
}
uint16_t sensor_read_bmx280_spi(uiter_t si) {
	return sensor_read_bmx280(si, true);
}
#endif // USE_BMx280_SPI_SENSORS

#if USE_BMx280_I2C_SENSORS
void sensor_init_bmx280_i2c(uiter_t si) {
	_FLASH const sensor_static_t *cfg;

	cfg = &SENSORS[si];

	SET_BIT(G_sensors[si].iflags, SENS_FLAG_I2C);
	// The 'pin' is actually the I2C address, which is 0x77 or 0x76 for both
	// the BMP280 and BME280
	if ((cfg->pin != 0x77) && (cfg->pin != 0x76)) {
		SETUP_ERR(si, "I2C address must be 0x76 or 0x77");
	}

	return;
}
uint16_t sensor_read_bmx280_i2c(uiter_t si) {
	return sensor_read_bmx280(si, false);
}
#endif // USE_BMx280_I2C_SENSORS

#if USE_BMx280_SPI_SENSORS
static err_t read_reg_spi(uint8_t cs, uint8_t reg, uint8_t *buf, uint8_t size) {
	err_t err = EOK;

	gpio_set_state((cs), GPIO_LOW);
	err = spi_exchange_byte(((reg) | 0x80), (buf), 100);
	if (err == EOK) {
		spi_receive_block((buf), (size), 0xFF, 250);
	}
	gpio_set_state((cs), GPIO_HIGH);

	return err;
}
static err_t write_reg_spi(uint8_t cs, uint8_t reg, uint8_t data) {
	err_t err = EOK;
	uint8_t buf[2];

	(buf)[0] = ((reg) & 0x7F);
	(buf)[1] = (data);
	gpio_set_state((cs), GPIO_LOW);
	err = spi_transmit_block((buf), 2, 100);
	gpio_set_state((cs), GPIO_HIGH);

	return err;
}
#endif // USE_BMx280_SPI_SENSORS

#if USE_BMx280_I2C_SENSORS
static err_t read_reg_i2c(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t size) {
	err_t err = EOK;

	(buf)[0] = (reg);
	err = i2c_transmit_block((addr), (buf), 1, 100);
	if (err == EOK) {
		i2c_receive_block((addr), (buf), (size), 250);
	}

	return err;
}
static err_t write_reg_i2c(uint8_t addr, uint8_t reg, uint8_t data) {
	err_t err = EOK;
	uint8_t buf[2];

	(buf)[0] = (reg);
	(buf)[1] = (data);
	i2c_transmit_block((addr), (buf), 2, 100);

	return err;
}
#endif // USE_BMx280_I2C_SENSORS

#if USE_BMx280_SPI_SENSORS && USE_BMx280_I2C_SENSORS
# define READ_REG(addr, reg, buf, size) \
	((use_spi) ? read_reg_spi((addr), (reg), (buf), (size)) : read_reg_i2c((addr), (reg), (buf), (size)))
# define WRITE_REG(addr, reg, data) \
	((use_spi) ? write_reg_spi((addr), (reg), (data)) : write_reg_i2c((addr), (reg), (data)))

#elif USE_BMx280_SPI_SENSORS
# define READ_REG( addr, reg, buf, size) read_reg_spi((addr), (reg), (buf), (size))
# define WRITE_REG(addr, reg, data)      write_reg_spi((addr), (reg), (data))

#else
# define READ_REG( addr, reg, buf, size) read_reg_i2c((addr), (reg), (buf), (size))
# define WRITE_REG(addr, reg, data)      write_reg_i2c((addr), (reg), (data))
#endif

static uint16_t sensor_read_bmx280(uiter_t si, bool use_spi) {
	err_t err = EOK;
	pin_t addr;
	uint8_t byte;
	bmx280_raw_t raw = { { 0 }, { 0 } };
	bmx280_status_t status = { 0 };
	bool do_humidity = false;
	utime_t timeout;
	_FLASH const sensor_static_t *cfg;

	cfg = &SENSORS[si];
	addr = cfg->pin;

#if USE_BMx280_SPI_SENSORS
	if (use_spi) {
		gpio_set_mode(addr, GPIO_MODE_PP, GPIO_HIGH);
	}
#endif

	// Check if this is a BMP280 (pressure and temperature) or a BME280 (that
	// plus humidity)
	if ((err = READ_REG(addr, 0xD0, &byte, 1)) == ETIMEOUT) {
		goto END;
	}
	if (byte == 0x60) {
		do_humidity = true;
	}

	if (do_humidity) {
		// Set ctrl_hum to oversample humidity 1x
		// This doesn't take effect until ctrl_meas has been set
		if (WRITE_REG(addr, 0xF2, 0b001) == ETIMEOUT) {
			goto END;
		}
	}
	//
	// Set ctrl_meas register to oversample temperature and pressure 1x and
	// begin measuring
	if ((err = WRITE_REG(addr, 0xF4, 0b00100101)) == ETIMEOUT) {
		goto END;
	}

	//
	// Wait for measurement to finish
	// The datasheet gives 5.5-6.4ms measurement time for 1x oversampling
	timeout = SET_TIMEOUT(100);
	do {
		delay_ms(10);
		if ((err = READ_REG(addr, 0xF3, &byte, 1)) == ETIMEOUT) {
			goto END;
		}
	} while ((byte != 0) && (!TIMES_UP(timeout)));

	//
	// Read device calibration data part 1; 26 bytes
	if ((err = READ_REG(addr, 0x88, raw.cal, 26)) == ETIMEOUT) {
		goto END;
	}
	//
	// Read device calibration data part 2; 16 bytes (but only 7 are used?)
	if (do_humidity) {
		if ((err = READ_REG(addr, 0xE1, &raw.cal[26], 7)) == ETIMEOUT) {
			goto END;
		}
	}
	//
	// Read sensor measurement data; 8 bytes
	// The humidity register is read even when it's not supported so that the
	// registers can be read in a single go; it shouldn't hurt anything.
	if ((err = READ_REG(addr, 0xF7, raw.adc, 8)) == ETIMEOUT) {
		goto END;
	}

END:
#if USE_BMx280_SPI_SENSORS
	if (use_spi) {
		gpio_set_mode(addr, GPIO_MODE_PP, GPIO_HIGH);
	}
#endif
	if (err != EOK) {
		PRINTF("BMx280 sensor at 0x%02X: communication error: %u", (uint )addr, (uint )err);
	}

	calculate_bmx280(&status, &raw, do_humidity);
#if USE_SERIAL
	if ((status.temp | status.press | status.hum) == 0) {
		LOGGER("BMx280 sensor at 0x%02X: all readings were invalid", (uint )addr);
	}
#endif

	if (use_spi) {
#if USE_BMx280_SPI_SENSORS
	for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
		if (PINID(SENSORS[i].pin) == PINID(addr)) {
			imath_t tmp, adjust;

			cfg = &SENSORS[i];
#if USE_SMALL_SENSORS < 2
			adjust = cfg->adjust;
#else
			adjust = 0;
#endif
			switch (cfg->type) {
			case SENS_BMx280_SPI_PRESSURE:
				SET_BIT(G_sensors[i].iflags, SENS_FLAG_DONE);
				// Adjust the adjustment to compensate for the fractional part of
				// the measurement
				adjust *= 100;
				G_sensors[i].status = (SCALE_INT(status.press + adjust)) / 100;
				break;
			case SENS_BMx280_SPI_TEMPERATURE:
				SET_BIT(G_sensors[i].iflags, SENS_FLAG_DONE);
				adjust *= 100;
				tmp = (SCALE_INT(status.temp + adjust)) / 100;
				G_sensors[i].status = tmp;
				break;
			case SENS_BMx280_SPI_HUMIDITY:
				SET_BIT(G_sensors[i].iflags, SENS_FLAG_DONE);
				adjust <<= 10;
				tmp = (SCALE_INT(status.hum + adjust)) >> 10;
				G_sensors[i].status = tmp;
				break;
			default:
				// There may be accidental matches with other 'pins' like the
				// I2C address used by the sister function
				//UNKNOWN_MSG(cfg->type);
				LOGGER("Sensor %u pin matches sensor %u pin", (uint )i, (uint )si);
				break;
			}
		}
	}
#endif // USE_BMx280_SPI_SENSORS

	} else { // !use_spi
#if USE_BMx280_I2C_SENSORS
	for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
		// The 'pin' is actually the I2C address so don't use PINID()
		if (SENSORS[i].pin == addr) {
			imath_t tmp, adjust;

			cfg = &SENSORS[i];
#if USE_SMALL_SENSORS < 2
			adjust = cfg->adjust;
#else
			adjust = 0;
#endif
			switch (cfg->type) {
			case SENS_BMx280_I2C_PRESSURE:
				SET_BIT(G_sensors[i].iflags, SENS_FLAG_DONE);
				// Adjust the adjustment to compensate for the fractional part of
				// the measurement
				adjust *= 100;
				G_sensors[i].status = (SCALE_INT(status.press + adjust)) / 100;
				break;
			case SENS_BMx280_I2C_TEMPERATURE:
				SET_BIT(G_sensors[i].iflags, SENS_FLAG_DONE);
				adjust *= 100;
				tmp = (SCALE_INT(status.temp + adjust)) / 100;
				G_sensors[i].status = tmp;
				break;
			case SENS_BMx280_I2C_HUMIDITY:
				SET_BIT(G_sensors[i].iflags, SENS_FLAG_DONE);
				adjust <<= 10;
				tmp = (SCALE_INT(status.hum + adjust)) >> 10;
				G_sensors[i].status = tmp;
				break;
			default:
				// There may be accidental matches with actual pins
				//UNKNOWN_MSG(cfg->type);
				LOGGER("Sensor %u pin matches sensor %u address", (uint )i, (uint )si);
				break;
			}
		}
	}
#endif // USE_BMx280_I2C_SENSORS
	} // use_spi

	return 0;
}
//
// Calculate temperature, pressure, and humidity from the raw measurements
// Adapted from the code given in the data sheet and in the Bosh Sensortec
// github repos
//
// The github code uses division instead of right shifts (I assume because
// someone had a problem with the sign bit at some point) but I'm sticking
// with the shifts for now because the resulting binary is smaller.
//
// Sorry for the names, I copied them from the data sheet
static void calculate_bmx280(bmx280_status_t *status, bmx280_raw_t *raw, bool do_humidity) {
	int32_t temp = 0;
	uint32_t press = 0, hum = 0;
	uint32_t t_adc, p_adc, h_adc;
	// Don't blame me for the names, I copied them out of the datasheet
	// And why are T1 and P1 unsigned? they're cast to signed the only places
	// they're used.
	uint16_t dig_T1, dig_P1;
	int16_t dig_T2, dig_T3, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9, dig_H2, dig_H4, dig_H5;
	uint8_t dig_H1, dig_H3;
	int8_t dig_H6;
	int32_t var1, var2, var3, var4, var5, t_fine = 0, tmp;

	assert(raw != NULL);
	assert(status != NULL);

	//
	// Calibration data; 33ish bytes
	// The MSB is the higher byte for calibration data
	//
	// Temperature calibration; 3 16-bit words
	READ_SPLIT_U16(dig_T1, raw->cal[1],  raw->cal[0]);
	READ_SPLIT_S16(dig_T2, raw->cal[3],  raw->cal[2]);
	READ_SPLIT_S16(dig_T3, raw->cal[5],  raw->cal[4]);
	//
	// Pressure calibration; 9 16-bit words
	READ_SPLIT_U16(dig_P1, raw->cal[7],  raw->cal[6]);
	READ_SPLIT_S16(dig_P2, raw->cal[9],  raw->cal[8]);
	READ_SPLIT_S16(dig_P3, raw->cal[11], raw->cal[10]);
	READ_SPLIT_S16(dig_P4, raw->cal[13], raw->cal[12]);
	READ_SPLIT_S16(dig_P5, raw->cal[15], raw->cal[14]);
	READ_SPLIT_S16(dig_P6, raw->cal[17], raw->cal[16]);
	READ_SPLIT_S16(dig_P7, raw->cal[19], raw->cal[18]);
	READ_SPLIT_S16(dig_P8, raw->cal[21], raw->cal[20]);
	READ_SPLIT_S16(dig_P9, raw->cal[23], raw->cal[22]);
	//
	// Humidity calibration; 8 bytes
	// cal[24] is unused
	dig_H1 = (int8_t )(raw->cal[25]);
	READ_SPLIT_S16(dig_H2, raw->cal[27], raw->cal[26]);
	dig_H3 = raw->cal[28];
	dig_H4 = (((int16_t )(raw->cal[29])) << 4) | ((int16_t )(raw->cal[30] & 0x0F));
	dig_H5 = (((int16_t )(raw->cal[31])) << 4) | (((int16_t )(raw->cal[30] & 0xF0)) >> 4);
	dig_H6 = (int8_t )(raw->cal[32]);

	//
	// Sensor measurement data; 8 bytes
	// The MSB is the lower byte for measurement data
	// Pressure measurement
	p_adc = ((uint32_t )raw->adc[0] << 12) | ((uint32_t )raw->adc[1] << 4) | ((uint32_t )raw->adc[2] >> 4);
	// Temperature measurement
	t_adc = ((uint32_t )raw->adc[3] << 12) | ((uint32_t )raw->adc[4] << 4) | ((uint32_t )raw->adc[5] >> 4);
	// Humidity measurement
	h_adc = ((uint32_t )raw->adc[6] << 8) | ((uint32_t )raw->adc[7]);

	// The readings for pressure, temperature, and humidity are 0x80000,
	// 0x80000, and 0x8000 respectively when they're skipped and 0xFFFFF,
	// 0xFFFFF, and 0xFFFF when the sensor is absent; it's easier to track this
	// stuff by just setting them to 0 though
	if ((t_adc == 0x80000) || (t_adc == 0xFFFFF)) {
		t_adc = 0;
	}
	if ((p_adc == 0x80000) || (p_adc == 0xFFFFF)) {
		p_adc = 0;
	}
	if ((h_adc == 0x8000) || (h_adc == 0xFFFF)) {
		h_adc = 0;
	}

	//
	// Temperature; result is DegC * 100
	if (t_adc != 0) {
		var1 = ((((t_adc >> 3) - ((int32_t )dig_T1 << 1))) * ((int32_t )dig_T2)) >> 11;
		tmp = (t_adc >> 4) - (int32_t )dig_T1;
		var2 = (((tmp * tmp) >> 12) * (int32_t )dig_T3) >> 14;
		t_fine = var1 + var2;
		temp = ((t_fine * 5) + 128) >> 8;
	}
	//
	// Pressure; result is Pa (hPa * 100, or millibars * 100)
	if (p_adc != 0) {
		var1 = (t_fine >> 1) - (int32_t )64000;
		tmp = var1 >> 2;
		var2 = (tmp * tmp) * (int32_t )dig_P6;
		var2 = var2 + ((var1 * ((int32_t )dig_P5)) << 1);
		var2 = (var2 >> 2) + (((int32_t )dig_P4) << 16);
		var1 = (((dig_P3 * ((tmp * tmp) >> 13)) >> 3) + ((((int32_t )dig_P2) * var1) >> 1)) >> 18;
		var1 = ((((32768 + var1)) * ((int32_t )dig_P1)) >> 15);
		if (var1 != 0) {
			press = (((uint32_t )(((int32_t )1048576) - p_adc) - (var2 >> 12))) * 3125;
			if (press < 0x80000000) {
				press = (press << 1) / ((uint32_t )var1);
			} else {
				press = (press / (uint32_t )var1) * 2;
			}
			var1 = (((int32_t )dig_P9) * ((int32_t )(((press >> 3) * (press >> 3)) >> 13))) >> 12;
			var2 = (((int32_t )(press >> 2)) * ((int32_t )dig_P8)) >> 13;
			press = (uint32_t )((int32_t )press + ((var1 + var2 + dig_P7) >> 4));
		} else {
			press = 0;
		}
	}
	//
	// Humidity; %RH in 22.10 fixed-point format (or %RH * 1024)
	if (do_humidity && (h_adc != 0)) {
		var1 = t_fine - ((int32_t )76800);
		var2 = (int32_t )(h_adc << 14);
		var3 = (int32_t )(((int32_t )dig_H4) << 20);
		var4 = ((int32_t )dig_H5) * var1;
		var5 = (((var2 - var3) - var4) + (int32_t )16384) >> 15;
		var2 = (var1 * ((int32_t )dig_H6)) >> 10;
		var3 = (var1 * ((int32_t )dig_H3)) >> 11;
		var4 = ((var2 * (var3 + (int32_t )32768)) >> 10) + (int32_t )2097152;
		var2 = ((var4 * ((int32_t )dig_H2)) + 8192) >> 14;
		var3 = var5 * var2;
		tmp = (var3 >> 15);
		var4 = (tmp * tmp) / 128;
		var5 = var3 - ((var4 * ((int32_t )dig_H1)) >> 4);
		var5 = (var5 < 0 ? 0 : var5);
		var5 = (var5 > 419430400 ? 419430400 : var5);
		hum = (uint32_t )(var5 >> 12);
	}

	status->temp = temp;
	status->press = press;
	status->hum = hum;

	return;
}

#endif // USE_BMx280_SPI_SENSORS || USE_BMx280_I2C_SENSORS
#ifdef __cplusplus
 }
#endif
