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
// actuators.c
// Manage actuators
// NOTES:
//
#include "common.h"
#include "actuators.h"

#if USE_ACTUATORS

#include "ulib/include/math.h"
#include "ulib/include/util.h"

#include GHMON_INCLUDE_CONFIG_HEADER(actuators/actuator_defs.h)

#define ACTUATOR_INDEX(_cfg_) (uint )((_cfg_) - ACTUATORS)
#define ACTUATOR_ID(_status_) (uint )((_status_) - actuators)

const ACTUATOR_INDEX_T ACTUATOR_COUNT = SIZEOF_ARRAY(ACTUATORS);
//#define ACTUATOR_COUNT SIZEOF_ARRAY(ACTUATORS)

actuator_status_t actuators[SIZEOF_ARRAY(ACTUATORS)];

static err_t _init_actuator(ACTUATOR_CFG_STORAGE actuator_cfg_t *cfg, actuator_status_t *status) {
	assert(cfg != NULL);
	assert(status != NULL);

#if USE_ACTUATOR_INIT
	if (cfg->init != NULL) {
		err_t res = cfg->init(cfg, status);
		if (res != ERR_OK) {
			SET_BIT(status->status_flags, ACTUATOR_STATUS_FLAG_ERROR);
			return res;
		}
	}
#endif
	SET_BIT(status->status_flags, ACTUATOR_STATUS_FLAG_INITIALIZED);

	return ERR_OK;
}

err_t init_actuator(ACTUATOR_CFG_STORAGE actuator_cfg_t *cfg, actuator_status_t *status) {
	assert(cfg != NULL);
	assert(status != NULL);

	mem_init(status, 0, sizeof(*status));
	return _init_actuator(cfg, status);
}
void init_common_actuators(void) {
	ACTUATOR_CFG_STORAGE actuator_cfg_t *cfg;
	actuator_status_t *status;

	for (ACTUATOR_INDEX_T i = 0; i < ACTUATOR_COUNT; ++i) {
		cfg = &ACTUATORS[i];
		status = &actuators[i];

		if (cfg->name[0] == 0) {
			LOGGER("Unset name in ACTUATORS[%u]", (uint )i);
		}
		init_actuator(cfg, status);
	}

	return;
}

err_t set_actuator(ACTUATOR_CFG_STORAGE actuator_cfg_t *cfg, actuator_status_t *status, ACTUATOR_STATUS_T value) {
	assert(cfg != NULL);
	assert(cfg->set != NULL);
	assert(status != NULL);

	if (!BIT_IS_SET(status->status_flags, ACTUATOR_STATUS_FLAG_INITIALIZED)) {
		err_t res = _init_actuator(cfg, status);
		if (res != ERR_OK) {
			return res;
		}
	}

#if USE_ACTUATOR_NAME
	LOGGER("Setting actuator %s to 0x%02X", FROM_FSTR(cfg->name), (uint )value);
#else
	LOGGER("Setting actuator %u to 0x%02X", ACTUATOR_ID(status), (uint )value);
#endif

	ACTUATOR_STATUS_T prev_status = status->status;
	bool prev_on = false;
#if USE_ACTUATOR_ON_TIME_COUNT
	if (cfg->is_on != NULL) {
		prev_on = cfg->is_on(cfg, status);
	}
#endif
	err_t res = cfg->set(cfg, status, value);

	if (status->status != prev_status) {
#if USE_ACTUATOR_STATUS_CHANGE_TIME
		const utime_t now = NOW();
#endif

#if USE_ACTUATOR_STATUS_CHANGE_COUNT
		++status->status_change_count;
#endif
#if USE_ACTUATOR_ON_TIME_COUNT
		if (prev_on && (cfg->is_on != NULL && cfg->is_on(cfg, status))) {
			// FIXME: This is going to be wrong if the system time changes, but I'll
			// need to track up-time separately from clock time to change that.
			status->on_time_seconds += (status->status_change_time - now);
		}
#endif
#if USE_ACTUATOR_STATUS_CHANGE_TIME
		status->status_change_time = now;
#endif
	}

	if (res != ERR_OK) {
		SET_BIT(status->status_flags, ACTUATOR_STATUS_FLAG_ERROR);
		return res;
	}
	CLEAR_BIT(status->status_flags, ACTUATOR_STATUS_FLAG_ERROR);

	return ERR_OK;
}

#if USE_ACTUATOR_NAME
ACTUATOR_INDEX_T find_actuator_index_by_name(const char *name) {
	assert(name != NULL);
	if (!SKIP_SAFETY_CHECKS && name == NULL) {
		return -1;
	}

	for (ACTUATOR_INDEX_T i = 0; i < ACTUATOR_COUNT; ++i) {
		ACTUATOR_CFG_STORAGE char *cfg_n = ACTUATORS[i].name;
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
err_t set_actuator_by_name(const char *name, ACTUATOR_STATUS_T value) {
	ACTUATOR_INDEX_T i = find_actuator_index_by_name(name);
	if (!SKIP_SAFETY_CHECKS && i < 0) {
		return ERR_BADARG;
	}
	return set_actuator(&ACTUATORS[i], &actuators[i], value);
}
#endif

err_t set_actuator_by_index(ACTUATOR_INDEX_T i, ACTUATOR_STATUS_T value) {
	assert(i >= 0 && i < ACTUATOR_COUNT);
	if (!SKIP_SAFETY_CHECKS && (i < 0 || i >= ACTUATOR_COUNT)) {
		return ERR_BADARG;
	}
	return set_actuator(&ACTUATORS[i], &actuators[i], value);
}

void check_common_actuator_warnings(void) {
	actuator_status_t *status;

	CLEAR_BIT(ghmon_warnings, WARN_ACTUATOR);
	for (ACTUATOR_INDEX_T i = 0; i < ACTUATOR_COUNT; ++i) {
		status = &actuators[i];

		if (BIT_IS_SET(status->status_flags, ACTUATOR_STATUS_FLAG_ERROR)) {
			SET_BIT(ghmon_warnings, WARN_ACTUATOR);
			return;
		}
	}

	return;
}

#endif // USE_ACTUATORS
