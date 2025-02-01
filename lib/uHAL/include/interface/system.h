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
/// @file
/// @brief Interface between the frontend and backend code.
/// @note
///    This file should only be included by interface.h
///

///
/// The type used for system status and configuration flags.
typedef uint_fast8_t uHAL_flags_t;

///
/// @name System Status Flags
/// @{
//
///
/// The container type for system status flags.
typedef enum {
	///
	/// Serial communication is ready.
	/// @note
	/// Set by the backend @c platform_init() if/when the serial communication
	/// interface is available. Can be written by user code afterward to control
	/// serial output without disabling the peripheral.
	uHAL_FLAG_SERIAL_IS_UP = 0x01U,
	///
	/// A system interrupt is requested by the frontend.
	/// @note
	/// Set by user code when e.g. hibernation should be interrupted.
	/// @attention
	/// Must be cleared by user code before entering interruptable hibernation
	/// to avoid immediately exiting.
	uHAL_FLAG_IRQ = 0x02U,
	///
	/// Disallow sleep modes that may interfere with normal functionality by
	/// (for example) disabling outputs or interrupts.
	/// @note
	/// Set and cleared by user code.
	uHAL_FLAG_INHIBIT_HIBERNATION = 0x04U,
} uHAL_status_flag_t;
///
/// The variable used to track system status flags.
extern uHAL_flags_t uHAL_status;
///
/// Set a system status flag.
///
/// @param _FLAG The flag to set in @c uHAL_status.
#define uHAL_SET_STATUS(_FLAG) (SET_BIT(uHAL_status, (_FLAG)))
///
/// Clear a system status flag.
///
/// @param _FLAG The flag to clear in @c uHAL_status.
#define uHAL_CLEAR_STATUS(_FLAG) (CLEAR_BIT(uHAL_status, (_FLAG)))
///
/// Check if a system status flag is set.
///
/// @param _FLAG The flag to check in @c uHAL_status.
/// @retval Non-zero if the flag is set.
/// @retval Zero if the flag is unset.
#define uHAL_CHECK_STATUS(_FLAG) (uHAL_status & (_FLAG))
/// @}


///
/// @name System Function Configuration Flags
/// @{
//
///
/// The container type for configuration flags.
typedef enum {
	/// Allow interrupting supported processes when @c uHAL_FLAG_IRQ is set in uHAL_status.
	uHAL_CFG_ALLOW_INTERRUPTS = 0x01U,
} uHAL_cfg_flag_t;
/// @}

///
/// @name Platform Core Interface
/// @{
//
/// Initialize the platform hardware.
void platform_init(void);
///
/// Reset the device.
/// Depending on the platform and configuration, this may or may not reset the
/// RTC or certain other peripherals.
void platform_reset(void);
///
/// This hook is executed just prior to device reset.
///
/// @note
/// This function is overrideable.
void pre_reset_hook(void);
/// @}

///
/// @name Sleep Interface
/// @{
//
///
/// The type used for specifying hibernate modes.
typedef enum {
	///
	/// Light sleep. All peripheral clocks remain enabled.
	HIBERNATE_LIGHT   = 0x01U,
	///
	/// Deep sleep. Peripheral clocks may be disabled and other power-saving
	/// features which may slow down wakeup are enabled.
	HIBERNATE_DEEP  = 0x02U,
	///
	/// Deepest sleep. Depending on platform and configuration wakeup may only be
	/// possible by device reset or certain specific interrupts and the contents
	/// of SRAM and device registers may be lost.
	HIBERNATE_MAX     = 0x03U,
} sleep_mode_t;

#if uHAL_USE_HIBERNATE || __HAVE_DOXYGEN__
///
/// Sleep (low-power mode). This is similar to hibernating with @c HIBERNATE_LIGHT
/// but @c uHAL_FLAG_IRQ is never respected and there are no pre- or post- hooks.
///
/// @param ms Duration of sleep (milliseconds).
void sleep_ms(utime_t ms);
///
/// Hibernate (lower-power mode).
///
/// @param s Duration of sleep (seconds).
/// @param sleep_mode The desired sleep mode. This may be overridden by either
///  device configuration or system status flags.
/// @param flags Configuration flags.
///  @c uHAL_CFG_ALLOW_INTRERUPTS will respect @c uHAL_FLAG_IRQ.
void hibernate_s(utime_t s, sleep_mode_t sleep_mode, uHAL_flags_t flags);
#endif // uHAL_USE_HIBERNATE
///
/// Hibernate (lower-power mode) - Untimed.
///
/// @attention
/// Sleeping via this function won't set up any default methods of waking, it's
/// up to the caller to make sure that's taken care of.
///
/// @attention
/// On platforms that emulate an RTC, the system clock can not be updated during
/// hibernation when initiated by this function.
///
/// @param sleep_mode The desired sleep mode. This may be overridden by either
///  device configuration or system status flags.
/// @param flags Configuration flags. Currently this is unused.
void hibernate(sleep_mode_t sleep_mode, uHAL_flags_t flags);
///
/// Pre-hibernation hook.
///
/// @note
/// This function is overrideable.
///
/// @param s Duration of sleep (seconds).
///  If NULL, the hook was called by @c hibernate().
///  If modified, @c hibernate_s() respects the new value.
/// @param sleep_mode The desired sleep mode. If modified, @c hibernate_s() respects the new value.
/// @param flags Configuration flags as passed to @c hibernate_s().
void pre_hibernate_hook(utime_t *s, sleep_mode_t *sleep_mode, uHAL_flags_t flags);
///
/// Post-hibernation hook.
///
/// @note
/// This function is overrideable.
///
/// @param s Duration of sleep (seconds).
///  If @c (utime_t )-1, the hook was called by @c hibernate().
/// @param sleep_mode The executed sleep mode.
/// @param flags Configuration flags as passed to @c hibernate_s().
void post_hibernate_hook(utime_t s, sleep_mode_t sleep_mode, uHAL_flags_t flags);
/// @}

///
/// @name Error Handling
/// @{
//
///
/// Go to an endless error loop. No information is printed.
/// @c error_state_hook() is executed periodically.
/// @note
/// This function is overrideable
void error_state_crude(void);
///
/// Go to an endless error loop. If using serial communication and debugging
/// is enabled, information about the location of the error is printed
/// periodically.
/// @c error_state_hook() is executed periodically.
///
/// @note
/// This function is overrideable.
///
/// @param file_path The path of the file where the error originated.
/// @param lineno The line number the error originated from.
/// @param func_name The name of the function the error originated from.
/// @param msg A message elaborating on the state of the error.
void error_state(const char *file_path, uint32_t lineno, const char *func_name, const char *msg);
///
/// This hook is executed periodically in the default @c error_state_crude() and
/// @c error_state().
/// By default this does nothing.
///
/// @note
/// This function is overrideable.
void error_state_hook(void);

#if ((DEBUG || (!uHAL_USE_SMALL_MESSAGES)) && uHAL_USE_UART_COMM) || __HAVE_DOXYGEN__
# ifndef ERROR_STATE
///
/// Convenience macro for @c error_state().
///
/// @note
/// This macro uses the @c F() and @c F1() macros on systems with separate namespaces.
///
/// @param msg A message elaborating on the state of the error.
#  define ERROR_STATE(msg)     error_state(F1(__FILE__), __LINE__, __func__, F(msg))
# endif
# ifndef ERROR_STATE_NOF
///
/// Convenience macro for @c error_state().
///
/// @param msg A message elaborating on the state of the error.
#  define ERROR_STATE_NOF(msg) error_state(F1(__FILE__), __LINE__, __func__, msg)
# endif

#else // (DEBUG || (!uHAL_USE_SMALL_MESSAGES)) && uHAL_USE_UART_COMM
# ifndef ERROR_STATE
#  define ERROR_STATE(msg)     error_state_crude()
# endif
# ifndef ERROR_STATE_NOF
#  define ERROR_STATE_NOF(msg) error_state_crude()
# endif
#endif
/// @}
