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
// controllers.c
// Manage external device control
// NOTES:
//
#include "common.h"
#if USE_CONTROLLERS

#include "controllers.h"
#include "actuators.h"
#include "sensors.h"

#include "ulib/include/util.h"

#include GHMON_INCLUDE_CONFIG_HEADER(controllers/controller_defs.h)

#define TRACK_GLOBAL_SCHEDULE ((!USE_CONTROLLER_SCHEDULE) && (CONTROLLER_CHECK_MINUTES > 0))

// FIXME: This will give random-ish numbers for non-common controllers, but
// they should stay the same in any given run and this is just for logging so
// it's not a big deal
#define CONTROLLER_ID(_status_) (uint )((_status_) - controllers)

const CONTROLLER_INDEX_T CONTROLLER_COUNT = SIZEOF_ARRAY(CONTROLLERS);
//#define CONTROLLER_COUNT SIZEOF_ARRAY(CONTROLLERS)

controller_status_t controllers[SIZEOF_ARRAY(CONTROLLERS)];

#if TRACK_GLOBAL_SCHEDULE
static utime_t next_scheduled_run = 0;
#endif

controller_status_t* get_controller_status_by_index(CONTROLLER_INDEX_T i) {
	assert(i >= 0 && i < CONTROLLER_COUNT);
	return &controllers[i];
}

static err_t _init_controller(CONTROLLER_CFG_STORAGE controller_cfg_t *cfg, controller_status_t *status) {
	assert(cfg != NULL);
	assert(status != NULL);

#if USE_CONTROLLER_INIT
	if (cfg->init != NULL) {
		err_t res = cfg->init(cfg, status);
		if (res != ERR_OK) {
			SET_BIT(status->status_flags, CONTROLLER_STATUS_FLAG_ERROR);
			return res;
		}
	}
#endif
	// Need to be initialized before setting the alarm
	SET_BIT(status->status_flags, CONTROLLER_STATUS_FLAG_INITIALIZED);

	calculate_controller_alarm(cfg, status);

	return ERR_OK;
}

err_t init_controller(CONTROLLER_CFG_STORAGE controller_cfg_t *cfg, controller_status_t *status) {
	assert(cfg != NULL);
	assert(status != NULL);

	mem_init(status, 0, sizeof(*status));
	return _init_controller(cfg, status);
}
void init_common_controllers(void) {
	CONTROLLER_CFG_STORAGE controller_cfg_t *cfg;
	controller_status_t *status;

	for (CONTROLLER_INDEX_T i = 0; i < CONTROLLER_COUNT; ++i) {
		cfg = &CONTROLLERS[i];
		status = &controllers[i];

#if USE_CONTROLLER_NAME
		if (cfg->name[0] == 0) {
			PRINTF("Unset name in CONTROLLERS[%u]", (uint )i);
		}
#endif
		init_controller(cfg, status);
	}

	return;
}

err_t run_controller(CONTROLLER_CFG_STORAGE controller_cfg_t *cfg, controller_status_t *status) {
	assert(cfg != NULL);
	assert(cfg->run != NULL);
	assert(status != NULL);

	if (!BIT_IS_SET(status->status_flags, CONTROLLER_STATUS_FLAG_INITIALIZED)) {
		err_t res = _init_controller(cfg, status);
		if (res != ERR_OK) {
			return res;
		}
	}

#if USE_CONTROLLER_NAME
	LOGGER("Running controller %s", FROM_FSTR(cfg->name));
#else
	LOGGER("Running controller %u", CONTROLLER_ID(status));
#endif

	err_t res = cfg->run(cfg, status);
	if (res != ERR_OK) {
		SET_BIT(status->status_flags, CONTROLLER_STATUS_FLAG_ERROR);
		return res;
	} else {
		CLEAR_BIT(status->status_flags, CONTROLLER_STATUS_FLAG_ERROR);
	}

	return ERR_OK;
}

#if USE_CONTROLLER_SCHEDULE || USE_CONTROLLER_NEXTTIME
void run_common_controllers(bool manual, bool force) {
	CONTROLLER_CFG_STORAGE controller_cfg_t *cfg;
	controller_status_t *status;

	for (CONTROLLER_INDEX_T i = 0; i < CONTROLLER_COUNT; ++i) {
		cfg = &CONTROLLERS[i];
		status = &controllers[i];

		if ((force && !BIT_IS_SET(cfg->cfg_flags, CONTROLLER_CFG_FLAG_IGNORE_FORCED_RUN)) ||
		    (status->next_run_time != 0 && (NOW() >= status->next_run_time)) ||
#if USE_CONTROLLER_SCHEDULE
		    (manual && cfg->schedule_minutes == 0 && !BIT_IS_SET(cfg->cfg_flags, CONTROLLER_CFG_FLAG_USE_TIME_OF_DAY))
#else
		    (manual)
#endif
		   ) {
			run_controller(cfg, status);
			// Update the next run time here instead of run_controller() so that
			// controllers can be run at arbitrary times without messing with the
			// schedule
			calculate_controller_alarm(cfg, status);
		}
	}

	return;
}
#elif TRACK_GLOBAL_SCHEDULE
void run_common_controllers(bool manual, bool force) {
	CONTROLLER_CFG_STORAGE controller_cfg_t *cfg;
	controller_status_t *status;

	bool do_run = false;
	if (CONTROLLER_CHECK_MINUTES == 0) {
		do_run = manual;
	} else {
		const utime_t now = NOW();

		if (now >= next_scheduled_run) {
			do_run = true;
			const utime_t tmp = CONTROLLER_CHECK_MINUTES * SECONDS_PER_MINUTE;
			next_scheduled_run = SNAP_TO_FACTOR(now, tmp) + (CONTROLLER_CHECK_MINUTES * SECONDS_PER_MINUTE);
		}
	}

	if (do_run || force) {
		for (CONTROLLER_INDEX_T i = 0; i < CONTROLLER_COUNT; ++i) {
			cfg = &CONTROLLERS[i];
			status = &controllers[i];

			if (do_run || !BIT_IS_SET(cfg->cfg_flags, CONTROLLER_CFG_FLAG_IGNORE_FORCED_RUN)) {
				run_controller(cfg, status);
			}
		}
	}

	return;
}
#endif

err_t calculate_controller_alarm(CONTROLLER_CFG_STORAGE controller_cfg_t *cfg, controller_status_t *status) {
	assert(cfg != NULL);
	assert(status != NULL);

#if USE_CONTROLLER_SCHEDULE || USE_CONTROLLER_NEXTTIME
	utime_t next = 0;
	utime_t now = NOW();

	if (!BIT_IS_SET(status->status_flags, CONTROLLER_STATUS_FLAG_INITIALIZED)) {
		return ERR_INIT;
	}

# if USE_CONTROLLER_NEXTTIME
	if (cfg->next_run_time != NULL) {
		next = cfg->next_run_time(cfg, status, now);
		if (next != 0) {
			goto END;
		}
	}
# endif

# if USE_CONTROLLER_SCHEDULE
	//
	// Use the default polling frequency
	if (cfg->schedule_minutes == 0 && !BIT_IS_SET(cfg->cfg_flags, CONTROLLER_CFG_FLAG_USE_TIME_OF_DAY)) {
		if (CONTROLLER_CHECK_MINUTES > 0) {
			const utime_t tmp = CONTROLLER_CHECK_MINUTES * SECONDS_PER_MINUTE;
			next = now + tmp;
			next = SNAP_TO_FACTOR(next, tmp);
		}
	//
	// Controller-specific polling frequency, time of day
	} else if (BIT_IS_SET(cfg->cfg_flags, CONTROLLER_CFG_FLAG_USE_TIME_OF_DAY)) {
		utime_t sm;

		sm = cfg->schedule_minutes * SECONDS_PER_MINUTE;
		next = SNAP_TO_FACTOR(now, SECONDS_PER_DAY) + sm;
		//
		// If the scheduled time would be earlier than now and we're outside
		// the clock scew window, wait until tomorrow
		if ((next + (CONTROLLER_SCHEDULE_SKEW_WINDOW_MINUTES * SECONDS_PER_MINUTE)) < now) {
			next += SECONDS_PER_DAY;
		}
	//
	// Controller-specific polling frequency, periodic
	} else {
		utime_t sm;

		sm = cfg->schedule_minutes * SECONDS_PER_MINUTE;
		next = now + sm;
		next = SNAP_TO_FACTOR(next, sm);
	}
# endif

END:
# if USE_CONTROLLER_NAME
	if (next != 0) {
		LOGGER("Next alarm for controller %s at %lu", FROM_FSTR(cfg->name), next);
	} else {
		LOGGER("No alarm scheduled for controller %s", FROM_FSTR(cfg->name));
	}
# else
	if (next != 0) {
		LOGGER("Next alarm for controller %u at %lu", CONTROLLER_ID(status), next);
	} else {
		LOGGER("No alarm scheduled for controller %u", CONTROLLER_ID(status));
	}
# endif

	status->next_run_time = next;
#endif // USE_CONTROLLER_SCHEDULE || USE_CONTROLLER_NEXTTIME

	return ERR_OK;
}
void calculate_common_controller_alarms(bool force) {
	// Get rid of compiler warning when scheduling isn't used
	UNUSED(force);

#if USE_CONTROLLER_SCHEDULE || USE_CONTROLLER_NEXTTIME
	CONTROLLER_CFG_STORAGE controller_cfg_t *cfg;
	controller_status_t *status;

	for (CONTROLLER_INDEX_T i = 0; i < CONTROLLER_COUNT; ++i) {
		cfg = &CONTROLLERS[i];
		status = &controllers[i];

		if (force || (status->next_run_time == 0)) {
			calculate_controller_alarm(cfg, status);
		}
	}

#elif TRACK_GLOBAL_SCHEDULE
	if (force || next_scheduled_run == 0) {
		const utime_t tmp = (CONTROLLER_CHECK_MINUTES * SECONDS_PER_MINUTE);
		const utime_t n = NOW() + tmp;

		next_scheduled_run = SNAP_TO_FACTOR(n, tmp);
	}
#endif

	return;
}
utime_t find_next_common_controller_alarm(void) {
	utime_t next = 0;

#if USE_CONTROLLER_SCHEDULE || USE_CONTROLLER_NEXTTIME
	controller_status_t *status;

	for (CONTROLLER_INDEX_T i = 0; i < CONTROLLER_COUNT; ++i) {
		status = &controllers[i];

		if (next == 0 || (status->next_run_time != 0 && status->next_run_time < next)) {
			next = status->next_run_time;
		}
	}
#else
	if (TRACK_GLOBAL_SCHEDULE) {
		next = next_scheduled_run;
	}
#endif

	return next;
}

void check_common_controller_warnings(void) {
	controller_status_t *status;

	CLEAR_BIT(ghmon_warnings, WARN_CONTROLLER);
	for (CONTROLLER_INDEX_T i = 0; i < CONTROLLER_COUNT; ++i) {
		status = &controllers[i];

		if (BIT_IS_SET(status->status_flags, CONTROLLER_STATUS_FLAG_ERROR)) {
			SET_BIT(ghmon_warnings, WARN_CONTROLLER);
			return;
		}
	}

	return;
}

#endif // USE_CONTROLLERS
