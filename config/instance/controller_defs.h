//
// Example controller configuration
//
// Each controller consists of at least a static configurations struct of type
// controller_cfg_t and a run() function, with optional init() and next_run_time()
// functions also possible.
//
// In order to keep the backend code simple while maintaining C99 compatibility,
// the functions used to initialize and run the controllers are defined in one
// section and an array of controller configurations are defined in the second part.
//

//
// Section 1
// Initialization/Run Definitions
//

//
// Controller 0, heartbeat blink
//
static err_t heartbeat_run(CONTROLLER_CFG_STORAGE controller_cfg_t *cfg, controller_status_t *status) {
	UNUSED(cfg);
	UNUSED(status);

	led_flash(1, 200);

	return ERR_OK;
}

//
// Controller 1, cooling
//
static err_t fan1_init(CONTROLLER_CFG_STORAGE controller_cfg_t *cfg, controller_status_t *status) {
	output_pin_off(FAN1_CTRL_PIN);
	return ERR_OK;
}
static err_t fan1_run(CONTROLLER_CFG_STORAGE controller_cfg_t *cfg, controller_status_t *status) {
	static uint_fast8_t prev_temp = 0;
	SENSOR_READING_T temp = read_sensor_by_name("IN_TEMP1", false, 0);

	if (temp == SENSOR_BAD_VALUE) {
		// If something's wrong with temperature sensor, play it safe and turn the
		// fan on if we don't have reason to believe it's too cold
		if (prev_temp > HEAT_OFF_THRESHOLD) {
			output_pin_on(FAN1_CTRL_PIN);
			status->status = 1;
		}
		return ERR_UNKNOWN;
	}

	if (temp > COOL_ON_THRESHOLD) {
		output_pin_on(FAN1_CTRL_PIN);
		status->status = 1;
	} else if (temp < COOL_OFF_THRESHOLD) {
		output_pin_off(FAN1_CTRL_PIN);
		status->status = 0;
	}
	prev_temp = temp;

	return ERR_OK;
}

//
// Controller 2, start irrigation
//
static bool irr1_is_on = false;
static err_t irr1_start_init(CONTROLLER_CFG_STORAGE controller_cfg_t *cfg, controller_status_t *status) {
	output_pin_off(IRR1_CTRL_PIN);
	return ERR_OK;
}
static err_t irr1_start_run(CONTROLLER_CFG_STORAGE controller_cfg_t *cfg, controller_status_t *status) {
	if (BIT_IS_SET(ghmon_warnings, WARN_BATTERY_LOW | WARN_VCC_LOW)) {
		//status->status = ERR_RETRY;
		return ERR_RETRY;
	}

	if (read_sensor_by_name("GND_MOIST1", true, 0) >= MOIST_READING_DRY) {
		// Turn on the irrigation pump, it's turned off by the irr1_stop controller
		output_pin_on(IRR1_CTRL_PIN);
		status->status = NOW();
		irr1_is_on = true;
	}

	return ERR_OK;
}

//
// Controller 3, stop irrigation
//
static utime_t irr1_stop_next_run_time(CONTROLLER_CFG_STORAGE controller_cfg_t *cfg, controller_status_t *status) {
	return ((uint )status->data == 0) ? 0 : (NOW() + (5 * SECONDS_PER_MINUTE));
}
static err_t irr1_stop_run(CONTROLLER_CFG_STORAGE controller_cfg_t *cfg, controller_status_t *status) {
	//if (!output_pin_is_on(IRR1_CTRL_PIN)) {
	if (!irr1_is_on) {
		return ERR_OK;
	}

	bool stop = true;
	bool dry = (read_sensor_by_name("GND_MOIST1", true, 0) >= MOIST_READING_DRY);
	uint count = (uint )status->data;
	if (count < 2) {
		stop = false;
		status->data = (void *)(count + 1);
	}
	if (stop) {
		output_pin_off(IRR1_CTRL_PIN);
		status->status = NOW();
		irr1_is_on = false;
	}

	// If we're still dry, there may be something wrong so return an error
	return (stop && dry) ? ERR_UNKNOWN : ERR_OK;
}

/*
//
// Controller 4, OK alarm
// Sound an alarm every 1 minute as long as everything's OK
//
static err_t OK_alarm_init(CONTROLLER_CFG_STORAGE controller_cfg_t *cfg, controller_status_t *status) {
	// We want to know if everything was OK before too and static variables are weird,
	// so let's abuse the status data pointer to track that
	status->data = (void *)true;

	input_pin_on(NOT_OK_PIN);
	output_pin_off(OK_BUZZER_PIN);
}
static err_t OK_alarm_run(CONTROLLER_CFG_STORAGE controller_cfg_t *cfg, controller_status_t *status) {
	bool OK = true;
	bool was_OK = (bool )status->data;
	bool do_buzzer = true;

	// We don't want the buzzer to get tired, so give it an 8-hour work day
	uint8_t hour, minute, second;
	seconds_to_time(NOW(), &hour, &minute, &second);
	if (hour < 9 || hour > 17) {
		do_buzzer = false;
	}

	// Not OK if something is holding NOT_OK_PIN on
	if (input_pin_is_on(NOT_OK_PIN)) {
		OK = false;
	}

	// Not OK when below 10C (50F) or above 30C (86F)
	uint_fast8_t temp = read_sensor(TEMP1_SENSOR);
	if (temp < DEGREES_CELSIUS(10) || temp > DEGREES_CELSIUS(30)) {
		OK = false;
	}

	// Not OK if soil is dry
	if (read_sensor(MOIST1_SENSOR) > MOIST_READING_DRY) {
		OK = false;
	}

	status->data = (void *)OK;
	// If we're still OK, sound the buzzer
	if (OK) {
		if (do_buzzer) {
			output_pin_on(OK_BUZZER_PIN);
			sleep_ms(1000);
			output_pin_off(OK_BUZZER_PIN);
		}
		status->status = NOW();
		return ERR_OK;
	}

	return ERR_UNKNOWN;
}
*/

//
// Section 2
// Controller configuration
//
CONTROLLER_CFG_STORAGE controller_cfg_t CONTROLLERS[] = {
//
// Controller 0, heartbeat blink
// Every minute, briefly flash the status LED
//static CONTROLLER_CFG_STORAGE controller_cfg_t heartbeat = {
{
	.name = "FLASHY",
	.init = NULL,
	.run = heartbeat_run,
	.next_run_time = NULL,
	.schedule_minutes = 1,
	.cfg_flags = CONTROLLER_CFG_FLAG_IGNORE_FORCED_RUN
},
//
// Controller 1, cooling
// Every 10 minutes, check the temperature and turn a fan on or off if required
//static CONTROLLER_CFG_STORAGE controller_cfg_t fan1 = {
{
	.name = "FAN1",
	.init = fan1_init,
	.run = fan1_run,
	.next_run_time = NULL,
	.schedule_minutes = 10,
	.cfg_flags = 0
},
//
// Controller 2, start irrigation
// Every afternoon at 17:00, check soil moisture and irrigate if required
//static CONTROLLER_CFG_STORAGE controller_cfg_t irr1_start = {
{
	.name = "IRR1_START",
	.init = irr1_start_init,
	.run = irr1_start_run,
	.next_run_time = NULL,
	.schedule_minutes = (17 * MINUTES_PER_HOUR),
	.cfg_flags = CONTROLLER_CFG_FLAG_USE_TIME_OF_DAY
},
//
// Controller 3, stop irrigation
// Every afternoon at 17:10, stop irrigation if it's running and no longer
// required
//static CONTROLLER_CFG_STORAGE controller_cfg_t irr1_stop = {
{
	.name = "IRR1_STOP",
	.init = NULL,
	.run = irr1_stop_run,
	.next_run_time = irr1_stop_next_run_time,
	.schedule_minutes = (17 * MINUTES_PER_HOUR) + 10,
	.cfg_flags = CONTROLLER_CFG_FLAG_USE_TIME_OF_DAY
},
/*
//
// Controller 4, OK alarm
// Sound an alarm every 1 minute as long as everything's OK
//static CONTROLLER_CFG_STORAGE controller_cfg_t OK_alarm = {
{
	.name = "OK_ALARM",
	.init = OK_alarm_init,
	.run = OK_alarm_run,
	.next_run_time = NULL,
	.schedule_minutes = 1,
	.cfg_flags = 0
},
*/
};
