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
// sensors.c
// Manage sensors
// NOTES:
//
#include "common.h"
#include "sensors.h"

#include "ulib/include/math.h"
#include "ulib/include/util.h"

#include GHMON_INCLUDE_CONFIG_HEADER(sensors/sensor_defs.h)

#define SENSOR_INDEX(_cfg_) (uint )((_cfg_) - SENSORS)
#define SENSOR_ID(_status_) (uint )((_status_) - sensors)

const SENSOR_INDEX_T SENSOR_COUNT = SIZEOF_ARRAY(SENSORS);
//#define SENSOR_COUNT SIZEOF_ARRAY(SENSORS)

sensor_status_t sensors[SIZEOF_ARRAY(SENSORS)];

static err_t _init_sensor(SENSOR_CFG_STORAGE sensor_cfg_t *cfg, sensor_status_t *status) {
	assert(cfg != NULL);
	assert(status != NULL);

#if USE_SENSOR_INIT
	if (cfg->init != NULL) {
		err_t res = cfg->init(cfg, status);
		if (res != ERR_OK) {
			SET_BIT(status->status_flags, SENSOR_STATUS_FLAG_ERROR);
			return res;
		}
	}
#endif
	SET_BIT(status->status_flags, SENSOR_STATUS_FLAG_INITIALIZED);

	return ERR_OK;
}

err_t init_sensor(SENSOR_CFG_STORAGE sensor_cfg_t *cfg, sensor_status_t *status) {
	assert(cfg != NULL);
	assert(status != NULL);

	mem_init(status, 0, sizeof(*status));
	return _init_sensor(cfg, status);
}
void init_common_sensors(void) {
	SENSOR_CFG_STORAGE sensor_cfg_t *cfg;
	sensor_status_t *status;

	for (SENSOR_INDEX_T i = 0; i < SENSOR_COUNT; ++i) {
		cfg = &SENSORS[i];
		status = &sensors[i];

		if (cfg->name[0] == 0) {
			LOGGER("Unset name in SENSORS[%u]", (uint )i);
		}
		init_sensor(cfg, status);
	}

	return;
}

SENSOR_READING_T find_sensor_value_by_type(sensor_reading_t* reading, uint_fast8_t type) {
	if (reading == NULL) {
		return SENSOR_BAD_VALUE;
	}

	if (type == 0) {
		return reading->value;
	}
	do {
		if (reading->type == type) {
			return reading->value;
		}
		++reading;
	} while (reading->more);

	return SENSOR_BAD_VALUE;
}

SENSOR_READING_T read_sensor(SENSOR_CFG_STORAGE sensor_cfg_t *cfg, sensor_status_t *status, bool force_update, uint_fast8_t type) {
	sensor_reading_t *reading;

	assert(cfg != NULL);
	assert(cfg->read != NULL);
	assert(status != NULL);

	if (!BIT_IS_SET(status->status_flags, SENSOR_STATUS_FLAG_INITIALIZED)) {
		err_t res = _init_sensor(cfg, status);
		if (res != ERR_OK) {
			return SENSOR_BAD_VALUE;
		}
	}

#if USE_SENSOR_COOLDOWN
	utime_t now = NOW(), prev = status->previous_reading_time;

	if (!force_update && (prev != 0) && (prev <= now) && (prev + cfg->cooldown_seconds) > now) {
# if USE_SENSOR_NAME
		LOGGER("Using previous reading of sensor %s", FROM_FSTR(cfg->name));
# else
		LOGGER("Using previous reading of sensor %u", SENSOR_ID(status));
# endif
		reading = status->reading;
		goto END;
	}
#endif

#if USE_SENSOR_NAME
	LOGGER("Reading sensor %s", FROM_FSTR(cfg->name));
#else
	LOGGER("Reading sensor %u", SENSOR_ID(status));
#endif

	if (!SKIP_SAFETY_CHECKS && cfg->read == NULL) {
		return SENSOR_BAD_VALUE;
	}

	reading = cfg->read(cfg, status);
	if (reading == NULL) {
		SET_BIT(status->status_flags, SENSOR_STATUS_FLAG_ERROR);
		return SENSOR_BAD_VALUE;
	}

	status->reading = reading;
	CLEAR_BIT(status->status_flags, SENSOR_STATUS_FLAG_ERROR);
#if USE_SENSOR_COOLDOWN
	status->previous_reading_time = NOW();
#endif

END:
	return find_sensor_value_by_type(reading, type);
}

#if USE_SENSOR_NAME
SENSOR_INDEX_T find_sensor_index_by_name(const char *name) {
	assert(name != NULL);
	if (!SKIP_SAFETY_CHECKS && name == NULL) {
		return -1;
	}

	for (SENSOR_INDEX_T i = 0; i < SENSOR_COUNT; ++i) {
		SENSOR_CFG_STORAGE char *cfg_n = SENSORS[i].name;
		const char *find_n = name;

		// Doing our own string compare simplifies things when using devices
		// with separate namespaces for flash and RAM because we don't have to
		// call FROM_FSTR().
		while (*find_n != 0 && (*find_n == *cfg_n)) {
			++find_n;
			++cfg_n;
		}
		if ((*find_n == *cfg_n) && (*find_n == 0)) {
			return i;
		}
	}

	return -1;
}
SENSOR_READING_T read_sensor_by_name(const char *name, bool force_update, uint_fast8_t type) {
	SENSOR_INDEX_T i = find_sensor_index_by_name(name);
	if (!SKIP_SAFETY_CHECKS && i < 0) {
		return SENSOR_BAD_VALUE;
	}
	return read_sensor(&SENSORS[i], &sensors[i], force_update, type);
}
#endif
SENSOR_READING_T read_sensor_by_index(SENSOR_INDEX_T i, bool force_update, uint_fast8_t type) {
	assert(i >= 0 && i < SENSOR_COUNT);
	if (!SKIP_SAFETY_CHECKS && (i < 0 || i >= SENSOR_COUNT)) {
		return SENSOR_BAD_VALUE;
	}
	return read_sensor(&SENSORS[i], &sensors[i], force_update, type);
}

void check_common_sensor_warnings(void) {
	sensor_status_t *status;

	CLEAR_BIT(ghmon_warnings, WARN_SENSOR);
	for (SENSOR_INDEX_T i = 0; i < SENSOR_COUNT; ++i) {
		status = &sensors[i];

		if (BIT_IS_SET(status->status_flags, SENSOR_STATUS_FLAG_ERROR)) {
			SET_BIT(ghmon_warnings, WARN_SENSOR);
			return;
		}
	}

	return;
}
