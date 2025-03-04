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
