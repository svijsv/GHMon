//
// These are the hooks and associated data structures used to alter program
// operation
//

//
// These variables are used by main() and must be declared here.
// They are reset at the start of each loop unless indicated otherwise.
//
// If true, controllers with ripe timers are run in this loop.
static bool do_controllers = false;
//
// If true, controllers are run reguardless of their timers in this loop.
static bool force_controllers = false;
//
// If true, update the status log in this loop.
static bool do_log         = false;
//
// If true, sync the status log to external media.
static bool force_sync     = false;
//
// If true, update the system status in this loop.
static bool do_status      = false;
//
// If > 0, set a wake alarm for this time (system time designated in seconds)
// This variable is not reset ever, it's the responsibility of the hooks to disable
// or update it as required.
static utime_t wake_alarm = 0;

//
// Hook executed right after platform initialization
static void early_init_hook(void) {
	return;
}
//
// Hook executed right before entering main loop
static void late_init_hook(void) {
	return;
}
//
// Hook executed each loop after dealing with interrupts and before handling
// controllers
static void early_loop_hook(void) {
	return;
}
//
// Hook executed each loop after handling controllers and before setting alarm
// and entering sleep mode
static void late_loop_hook(void) {
	return;
}
//
// Hook executed when the UI button is pressed
// The argument is the number of CTRL_PRESS_MS periods that the button was
// held for.
// CTRL_PRESS x 0  : Take any actions with a check period of 0
// CTRL_PRESS x 1  : Sync log data to file
// CTRL_PRESS x 2  : Run device controllers
// CTRL_PRESS x 3  : Set time of day to 00:00:00 + RESET_TIME_OFFSET_MINUTES
// CTRL_PRESS x >=4: Cancel
static void ctrl_button_hook(uint_fast8_t held_periods) {
	led_flash(held_periods+1, STATUS_LED_ACK_DELAY_MS);

	switch (held_periods) {
	case 0:
		if (CONTROLLER_CHECK_MINUTES == 0) {
			do_controllers = true;
			LOGGER("Running controllers");
		}
		if (USE_LOGGING && LOG_APPEND_MINUTES == 0) {
			do_log = true;
			LOGGER("Appending to log");
		}
		LOGGER("Updating status");
		do_status = true;
		break;

	case 1:
		if (USE_LOGGING) {
			LOGGER("Forcing log sync");
			force_sync = true;
		} else {
			LOGGER("No action taken, logging disabled");
		}
		break;

	case 2:
		LOGGER("Forcing controller run");
		do_controllers = true;
		force_controllers = true;
		break;

	case 3: {
		datetime_t dt = {
			.hour   = RESET_TIME_OFFSET_MINUTES / 60,
			.minute = RESET_TIME_OFFSET_MINUTES % 60,
		};

		led_flash(4, STATUS_LED_LONG_DELAY_MS);
		LOGGER("Setting system time to %u:%02u:00", (uint )dt.hour, (uint )dt.minute);
		set_system_datetime(&dt);
		break;
	}

	default:
		led_flash(2, STATUS_LED_ACK_DELAY_MS);
		LOGGER("No action taken");
		break;
	}

	return;
}
