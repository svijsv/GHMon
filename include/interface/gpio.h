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
/// @brief General-Purpose Input/Output Interface
/// @attention
///    This file should only be included by interface.h.
///

///
/// @name GPIO Basic Interface
/// @{
//
///
/// Define the state of a GPIO pin.
/// @attention
/// @c GPIO_LOW will always be '0U' and @c GPIO_HIGH will always be '1U'; any
/// other values can't have assumptions made about them.
typedef enum {
	GPIO_LOW  = 0U, ///< State HIGH.
	GPIO_HIGH = 1U, ///< State LOW.
	GPIO_FLOAT ///< State UNDEFINED.
} gpio_state_t;

//
// These are defined in the device platform.h, they're included here for
// documentation purposes.
#if __HAVE_DOXYGEN__
///
/// Define the operation mode of a GPIO pin.
///
/// @attention
/// Some modes may be absent or aliased to other modes depending on the platform.
/// Whether a mode exists can be determined by checking if it's name is defined.
///
/// @attention
/// There may be additional modes on some platforms.
///
/// @attention
/// The actual definition is in the device platform header file.
typedef enum {
	GPIO_MODE_RESET = 0U, ///< The reset state of the pin.
	GPIO_MODE_PP,    ///< Push-pull output.
	GPIO_MODE_OD,    ///< Open-drain output.
	GPIO_MODE_IN,    ///< Input.
	GPIO_MODE_AIN,   ///< Analog input.
	GPIO_MODE_HiZ,   ///< High-impedence mode.

	GPIO_MODE_RESET_ALIAS, ///< The mode to which GPIO_MODE_RESET is aliased for GPIO function return values.
	GPIO_MODE_HiZ_ALIAS,   ///< The mode to which GPIO_MODE_HiZ is aliased for GPIO function return values.
} gpio_mode_t;

///
/// An identifier for MCU pins.
///
/// @attention
/// The actual definition is in the device platform header file.
///
/// @attention
/// @c 0 is reserved for unassigned pins.
///
// Using an int is simpler than a struct because I don't need to define
// structures to use pins and can check them with the preprocessor.
typedef uint_t gpio_pin_t;

///
/// The port number offset in gpio_pin_t
#define GPIO_PORT_OFFSET
///
/// The port number mask for gpio_pin_t
#define GPIO_PORT_MASK
///
/// The pin number offset in gpio_pin_t
#define GPIO_PIN_OFFSET
///
/// The pin number mask for gpio_pin_t
#define GPIO_PIN_MASK
#endif // __HAVE_DOXYGEN__

///
/// Derive the pin number from a given @c gpio_pin_t.
///
/// @param _pin_ The @c gpio_pin_t to derive the pin number from.
/// @return The number of the pin.
#define GPIO_GET_PINNO(_pin_) (((_pin_) & GPIO_PIN_MASK) >> GPIO_PIN_OFFSET)
///
/// Derive the pin bit mask from a given @c gpio_pin_t.
///
/// @param _pin_ The @c gpio_pin_t to derive the pin mask from.
/// @return The bit mask of the pin.
#define GPIO_GET_PINMASK(_pin_) AS_BIT(GPIO_GET_PINNO((_pin_)))
///
/// Derive the port number from a given @c gpio_pin_t.
///
/// @param _pin_ The @c gpio_pin_t to derive the pin number from.
/// @return The number of the port.
#define GPIO_GET_PORTNO(_pin_) (((_pin_) & GPIO_PORT_MASK) >> GPIO_PORT_OFFSET)
///
/// Derive the port mask from a given @c gpio_pin_t.
///
/// @param _pin_ The @c gpio_pin_t to derive the port mask from.
/// @return The bit mask of the port.
#define GPIO_GET_PORTMASK(_pin_) ((_pin_) & GPIO_PORT_MASK)
///
/// Derive the pin identifier from a given @c gpio_pin_t.
///
/// @param _pin_ The @c gpio_pin_t to derive the identifier from.
/// @return The identifier of the pin.
#ifndef PINID
# define PINID(_pin_) ((_pin_) & (GPIO_PIN_MASK | GPIO_PORT_MASK))
#endif

///
/// Set the operating mode of a pin.
///
/// @param pin The pin on which to operate.
/// @param mode The operating mode to which the pin should be set.
/// @param istate The initial state of the pin.
///  For digital inputs, this is the bias (whether it's pulled up or down or left floating).
///  For digital outputs, this is whether it's set high or low.
///  For other modes, this is ignored.
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t gpio_set_mode(gpio_pin_t pin, gpio_mode_t mode, gpio_state_t istate);
///
/// Get the operating mode of a pin.
///
/// @param pin The pin to examine.
/// @returns The operating mode of the pin, or GPIO_MODE_RESET on error.
gpio_mode_t gpio_get_mode(gpio_pin_t pin);
///
/// Set the state of a pin, auto-detecting mode.
///
/// @param pin The pin on which to operate.
/// @param new_state The new state of the pin.
///  For digital inputs, this is the bias (whether it's pulled up or down or left floating).
///  For digital outputs, this is whether it's set high or low.
///  For other modes, this is ignored.
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t gpio_set_state(gpio_pin_t pin, gpio_state_t new_state);
///
/// Set the state of an input pin.
///
/// @param pin The pin on which to operate.
/// @param new_state The new state of the pin.
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t gpio_set_input_state(gpio_pin_t pin, gpio_state_t new_state);
///
/// Set the state of an output pin.
///
/// @param pin The pin on which to operate.
/// @param new_state The new state of the pin.
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t gpio_set_output_state(gpio_pin_t pin, gpio_state_t new_state);
///
/// Toggle the state of a pin, auto-detecting mode.
///
/// For digital outputs this toggles the pin high or low IFF already high or low.
/// For digital inputs this toggles the bias. The treatment of floating inputs
/// varies by platform - they're ignored if there are both internal pullups and
/// internal pulldowns or else treated as the complement of the available bias.
/// For other modes this doesn't have an effect.
///
/// @param pin The pin on which to operate.
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t gpio_toggle_state(gpio_pin_t pin);
///
/// Toggle the state of an input pin.
///
/// @param pin The pin on which to operate.
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t gpio_toggle_input_state(gpio_pin_t pin);
///
/// Toggle the state of an output pin.
///
/// @param pin The pin on which to operate.
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t gpio_toggle_output_state(gpio_pin_t pin);
///
/// Get the state of a pin, auto-detecting mode.
///
/// @param pin The pin on which to operate.
/// @returns The actual state of digital inputs.
/// @returns The desired state of digital outputs (so e.g. an open-drain
///  pin thats pulled low externally will return @c GPIO_HIGH).
/// @returns @c GPIO_FLOAT for other modes.
gpio_state_t gpio_get_state(gpio_pin_t pin);
///
/// Get the state of an input pin.
///
/// @param pin The pin on which to operate.
/// @returns The actual state of digital inputs and open-drain outputs, or
///  @c GPIO_FLOAT on error.
gpio_state_t gpio_get_input_state(gpio_pin_t pin);
///
/// Get the state of an output pin.
///
/// @param pin The pin on which to operate.
/// @returns The desired state of push-pull and open-drain outputs, or
///  @c GPIO_FLOAT on error.
gpio_state_t gpio_get_output_state(gpio_pin_t pin);
/// @}


///
/// @name GPIO IRQ Interface
/// @{
//
///
/// The type used to specify GPIO IRQ triggers.
typedef enum {
	///
	/// Trigger on rising edge.
	GPIO_TRIGGER_RISING  = 0x01U,
	///
	/// Trigger on falling edge.
	GPIO_TRIGGER_FALLING = 0x02U,
} irq_trigger_t;

///
/// Define the configuration of a GPIO-generated interrupt.
///
typedef struct {
	///
	/// The pin that generates the IRQ.
	gpio_pin_t pin;
	///
	/// The condition under which the IRQ should be generated.
	/// This may be multiple trigger states ORd together.
	irq_trigger_t trigger;
} gpio_listen_cfg_t;

///
/// Prepare a pin for interrupt triggering.
///
/// @attention
/// While this sets up the trigger, the actual IRQ will need to be handled by
/// the calling code. How that's done will vary by platform but usually just
/// means determining which interrupt handler is involved for a given pin and
/// defining it.
/// @attention
/// Multiple pins may share a handler or interfere with each other, that also
/// varies by platform. Check the reference manual.
///
/// @param handle The handle used to manage the pin. Must not be NULL.
/// @param conf The configuration of the interrupt. Must not be NULL.
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t gpio_listen_init(gpio_listen_t *handle, const gpio_listen_cfg_t *conf);
///
/// Enable the interrupt for a pin. The pin must have been previously configured
/// by @c gpio_listen_init() or @c pin_listen_init().
///
/// @param handle The handle used to manage the pin. Must not be NULL.
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t gpio_listen_on(gpio_listen_t *handle);
///
/// Disable the interrupt for a pin. The pin must have been previously configured
/// by @c gpio_listen_init() or @c pin_listen_init().
///
/// @param handle The handle used to manage the pin. Must not be NULL.
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t gpio_listen_off(gpio_listen_t *handle);
///
/// Check if a pin's interrupt is enabled. The pin does not need to have been
/// previously configured by @c gpio_listen_init() or @c pin_listen_init().
///
/// @param pin The pin to examine.
/// @retval true when the pin's interrupt is enabled.
/// @retval false when the pin's interrupt is disabled.
bool gpio_is_listening(gpio_pin_t pin);
/// @}

///
/// @name GPIO High-Level Digital Interface
///
/// A pin whose control bits are unset (that is, they remain '0') will default to
/// a floating input or a push-pull output.
///
/// This interface can generally be mixed with the basic GPIO interface without
/// issue, but the latter won't respect the control bits in the @c gpio_pin_t
/// value
/// @{
#if __HAVE_DOXYGEN__
///
/// The pin control offset in gpio_pin_t
#define GPIO_CTRL_OFFSET
///
/// The pin control mask for gpio_pin_t
#define GPIO_CTRL_MASK
#endif
//
// The values of the control bits were selected such that when only using 2 bits
// open-drain outputs, pulled-up inputs, and inverted pins are still supported in
// addition to the default push-pull outputs and floating inputs. If that's ever
// takend advantage of the exported documentation should be updated to mention it.
//
// I'd like to use an enum here but that interferes with preprocessor tests
// of pins when the control bits are used.
/*
///
/// The container type for GPIO control flags.
typedef enum {
	///
	/// If set, an output pin will be tristated instead of being set HIGH.
	GPIO_CTRL_TRISTATE_HIGH = (0b010U << GPIO_CTRL_OFFSET),
	///
	/// If set, an output pin will be tristated instead of being set LOW.
	GPIO_CTRL_TRISTATE_LOW  = (0b100U << GPIO_CTRL_OFFSET),
	///
	/// If set, an input pin will be biased = (pulled up or down).
	GPIO_CTRL_BIAS_INPUT    = (0b010U << GPIO_CTRL_OFFSET),
	///
	/// If set, a biased input pin will be pulled down instead of up.
	GPIO_CTRL_BIAS_LOW      = (0b100U << GPIO_CTRL_OFFSET),
	///
	/// If set, an output pin will be set LOW when on and HIGH when off.
	/// If set, an input pin will read @c true when LOW and @c false when HIGH.
	/// If unset, the inverse is true.
	GPIO_CTRL_INVERT        = (0b001U << GPIO_CTRL_OFFSET),
} gpio_control_t;
*/
///
/// If set, an output pin will be tristated instead of being set HIGH.
#define GPIO_CTRL_TRISTATE_HIGH (0b010U << GPIO_CTRL_OFFSET)
///
/// If set, an output pin will be tristated instead of being set LOW.
#define GPIO_CTRL_TRISTATE_LOW  (0b100U << GPIO_CTRL_OFFSET)
///
/// If set, an input pin will be biased (pulled up or down).
#define GPIO_CTRL_BIAS_INPUT    (0b010U << GPIO_CTRL_OFFSET)
///
/// If set, a biased input pin will be pulled down instead of up.
#define GPIO_CTRL_BIAS_LOW      (0b100U << GPIO_CTRL_OFFSET)
///
/// If set, a pin will be set/read LOW when on and HIGH when off.
#define GPIO_CTRL_INVERT        (0b001U << GPIO_CTRL_OFFSET)
///
/// Convinience macro for setting an input with a pullup
#define GPIO_CTRL_PULLUP (GPIO_CTRL_BIAS_INPUT)
///
/// Convinience macro for setting an input with a pulldown
#define GPIO_CTRL_PULLDOWN (GPIO_CTRL_BIAS_INPUT|GPIO_CTRL_BIAS_LOW)
///
/// Convinience macro for setting a floating input
#define GPIO_CTRL_FLOAT (0U)
///
/// Convinience macro for setting an open-drain output
#define GPIO_CTRL_OPENDRAIN (GPIO_CTRL_TRISTATE_HIGH)
///
/// Convinience macro for setting a push-pull output
#define GPIO_CTRL_PUSHPULL (0U)

///
/// Derive the pin control mask from a given @c gpio_pin_t.
///
/// @param _pin_ The @c gpio_pin_t to derive the mask from.
///
/// @return The control mask of the pin.
#define GPIO_GET_CTRL(_pin_) ((_pin_) & GPIO_CTRL_MASK)

#if uHAL_USE_HIGH_LEVEL_GPIO || __HAVE_DOXYGEN__
///
/// Set an input pin ON based on it's control mask.
///
/// @param pin The pin to operate on.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t input_pin_on(gpio_pin_t pin);
///
/// Set an input pin OFF based on it's control mask.
///
/// @param pin The pin to operate on.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t input_pin_off(gpio_pin_t pin);
///
/// Toggle an input pin's state.
///
/// @param pin The pin to operate on.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t input_pin_toggle(gpio_pin_t pin);
///
/// Read a pin's state based on it's control mask.
///
/// @param pin The pin to operate on.
///
/// @returns @c true if the pin is ON or @c false if the pin is OFF or there
///  was an error.
bool input_pin_is_on(gpio_pin_t pin);
///
/// Set an output pin's state ON based on it's control mask.
///
/// @param pin The pin to operate on.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t output_pin_on(gpio_pin_t pin);
///
/// Set an output pin's state OFF based on it's control mask.
///
/// @param pin The pin to operate on.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t output_pin_off(gpio_pin_t pin);
///
/// Toggle an output pin's state.
///
/// @param pin The pin to operate on.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t output_pin_toggle(gpio_pin_t pin);
///
/// Check if an output pin is turned on
///
/// @param pin The pin to examine.
///
/// @returns @c true if the output pin is turned on.or @c false if it's turned
/// off or there was an error.
bool output_pin_is_on(gpio_pin_t pin);

///
/// Prepare a pin interrupt trigger based on it's control mask.
///
/// Inputs biased HIGH are set to trigger on a falling edge.
/// Inputs biased LOW are set to trigger on a falling edge.
/// Unbiased inputs are set to trigger on either edge.
///
/// @attention
/// While this sets up the trigger, the actual IRQ will need to be handled by
/// the calling code. How that's done will vary by platform but usually just
/// means determining which interrupt handler is involved for a given pin and
/// defining it.
/// @attention
/// Multiple pins may share a handler or interfere with each other, that also
/// varies by platform. Check the reference manual.
///
/// @param handle The handle used to manage the pin. Must not be NULL.
/// @param pin The pin to set up.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t input_pin_listen_init(gpio_listen_t *handle, gpio_pin_t pin);
///
/// Enable the interrupt for a pin.
///
/// The pin must have been previously configured by @c gpio_listen_init()
/// or @c pin_listen_init().
///
/// The pin may need to be turned on before listening.
///
/// @param handle The handle used to manage the pin. Must not be NULL.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t input_pin_listen_on(gpio_listen_t *handle);
///
/// Disable the interrupt for a pin.
///
/// The pin must have been previously configured by @c gpio_listen_init()
/// or @c pin_listen_init().
///
/// @param handle The handle used to manage the pin. Must not be NULL.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t input_pin_listen_off(gpio_listen_t *handle);
#endif // uHAL_USE_HIGH_LEVEL_GPIO
/// @}


#if uHAL_USE_EXPERIMENTAL_GPIO_INTERFACE || __HAVE_DOXYGEN__
///
/// @name Experimental GPIO High-Level Digital Interface
///
/// @attention
/// This interface should not be mixed with the mode or state setting functions of
/// other GPIO interfaces because it tracks state. Calling @c pinctrl_init()  will
/// put it back in order again if needed.
///
/// @attention
/// This interface was intended to be a more runtime-efficient, reliable, and powerful
/// replacement for the other high-level interface but seems to produce notably
/// larger code at least in small test programs and requires a lot more overhead
/// in configuring, so it's relegated to EXPERMENTAL status until it can be tested
/// and refined further.
///
/// @{
//
///
/// The structure used to configure a GPIO pin.
typedef struct {
	gpio_pin_t pin; ///< The GPIO pin controlled by the handle.

	bool as_input:1; ///< If @c true, use as an input. Otherwise, use as an output.
	bool hi_is_on:1; ///< if @c true, a high input reads as ON and an output is set high when turned ON.
	                 /// Otherwise, a low input reads as ON and an output is set low when turned ON.
	bool hi_active:1; ///< If @c true, the pin is high/pulled up when set HIGH. Otherwise it tristates/floats.
	bool lo_active:1; ///< If @c true, the pin is low/pullded down when set LOW. Otherwise it tristates/floats.
	bool turn_on:1; ///< If @c true, turn the pin ON when initialized.
} pinctrl_cfg_t;
//
// This whole struct is subject to change and only defined here so the user can
// declare handles, so don't document the details.
#if ! __HAVE_DOXYGEN__
typedef struct {
	gpio_pin_t pin;

	gpio_mode_t on_gpio_mode:4;
	gpio_mode_t off_gpio_mode:4;

	gpio_state_t on_gpio_state:4;
	gpio_state_t off_gpio_state:4;

	bool is_input:1;
	bool set_state:1;
	bool on_state:1;
} pinctrl_handle_t;
#else // ! __HAVE_DOXYGEN__
///
/// The handle used to manage a GPIO pin.
typedef struct pinctrl_handle_t;
#endif
///
/// Initialize a @c pinctrl_handle_t based on a @c pinctrl_cfg_t struct.
///
/// @param handle The handle to initialize.
/// @param cfg The configuration of the pin.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t pinctrl_init(pinctrl_handle_t *handle, const pinctrl_cfg_t *cfg);
///
/// Initialize a @c pinctrl_handle_t based on an input pin's control bits.
///
/// @param handle The handle to initialize.
/// @param pin The pin on which to base the initialization.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t pinctrl_init2_input(pinctrl_handle_t *handle, gpio_pin_t pin);
///
/// Initialize a @c pinctrl_handle_t based on an output pin's control bits.
///
/// @param handle The handle to initialize.
/// @param pin The pin on which to base the initialization.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t pinctrl_init2_output(pinctrl_handle_t *handle, gpio_pin_t pin);
///
/// Turn a pin ON.
///
/// @param handle A handle to the pin to operate on.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t pinctrl_on(pinctrl_handle_t *handle);
///
/// Turn a pin OFF.
///
/// @param handle A handle to the pin to operate on.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t pinctrl_off(pinctrl_handle_t *handle);
///
/// Toggle the state of a pin.
///
/// @param handle A handle to the pin to operate on.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t pinctrl_toggle(pinctrl_handle_t *handle);
///
/// Read the state of a pin.
///
/// @param handle A handle to the pin to operate on.
///
/// @returns @c true if the pin is ON or @c false if the pin is OFF or there
///  was an error.
bool pinctrl_is_on(pinctrl_handle_t *handle);
///
/// Put a pin in low-power mode.
///
/// @param handle A handle to the pin to operate on.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t pinctrl_suspend(pinctrl_handle_t *handle);
///
/// Return a pin to it's previous state from low-power mode.
///
/// @param handle A handle to the pin to operate on.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t pinctrl_resume(pinctrl_handle_t *handle);
/// @}
#endif // uHAL_USE_EXPERIMENTAL_GPIO_INTERFACE


///
/// @name GPIO Low-Latency Digital Interface
/// @{
//
///
/// Prepare to read the state of a pin quickly.
///
/// @param qpin The handle used to manage the pin.
/// @param pin The pin to prepare.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t gpio_quickread_prepare(gpio_quick_t *qpin, gpio_pin_t pin);

//
// These are defined in the device platform.h, they're included here for
// documentation purposes.
#if __HAVE_DOXYGEN__
///
/// The handle used to manage low-latency GPIO actions.
typedef struct gpio_quick_t gpio_quick_t;
///
/// Read a pin quickly.
///
/// @attention
/// @c _qpin_ must be prepared with @c gpio_quick_read_prepare() prior to use.
///
/// @param _qpin_ The @c gpio_quick_t handle to read.
///
/// @returns The current state of @c _qpin_.
#define GPIO_QUICK_READ(_qpin_)

///
/// Read an input pin (kind of) quickly.
///
/// @attention
/// The pin must be properly configured for digital input before calling.
///
/// @attention
/// There are no safety checks with this macro.
///
/// @param _pin_ The @c gpio_pin_t pin to read.
///
/// @returns Non-zero if current state of @c _pin_ is HIGH.
#define GPIO_INPUT_IS_HIGH(_pin_)

///
/// Set an output pin HIGH (kind of) quickly.
///
/// @attention
/// The pin must be properly configured for digital output before calling.
///
/// @attention
/// There are no safety checks with this macro.
///
/// @param _pin_ The @c gpio_pin_t pin to set.
///
#define SET_GPIO_OUTPUT_HIGH(_pin_)
///
/// Set an output pin LOW (kind of) quickly.
///
/// @attention
/// The pin must be properly configured for digital output before calling.
///
/// @attention
/// There are no safety checks with this macro.
///
/// @param _pin_ The @c gpio_pin_t pin to set.
///
#define SET_GPIO_OUTPUT_LOW(_pin_)
#endif // __HAVE_DOXYGEN__
/// @}
