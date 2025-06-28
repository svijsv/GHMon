## Introduction
GHMon is a configurable low-power firmware for autonomous small-scale
greenhouse control and data logging. At present it supports STM32F103-,
STM32F401-, and ATTiny402-based devices. It doesn't do very much and it doesn't
do it very well, but it's mine.


## Overview
There are three broad groups of devices in the GHMon worldview:
* Sensors read the world.
* Actuators control the world.
* Controllers mediate between the two.

GHMon provides a means of organizing these three catagories while also providing
scheduling, logging, and rudimentary control terminal support.


## Configuration
In order to maintain flexibility, configuration is performed by defining settings,
data structures, and functions in `config/`.

General settings universal to all builds are located in `config/general`. This
directory can mostly be ignored.

Settings specific to a particular build are taken from an 'instance' directory
defined in the environment variable `INSTANCE_DIR`, examples of which can be
found at `config/examples`.
* `config.h` and `advanced.h` contain general configuration details.
* `sensors/sensor_defs.h` contains the implementation details of the sensors.
* `actuators/actuator_defs.h` contains the implementation details of the actuators.
* `controllers/controller_defs.h` contains the implementation details of the controllers.
* `log/logfile.h` contains the implementation details of the logger.
* `main_hooks.h` contains hooks used to influence the main control loop.
* `lib/` contains overrides for library configuration.

The example instance directory additionally contains files included by the primary
ones for organizational reasons.

Details can be found in the example files.


## Installing
GHMon uses [PlatformIO](https://platformio.org/) for building and uploading.

Build configuration files are provided for a few devices and can be found in
`pio_inis/`; they're included by `platformio.ini` by default. New (compatible)
devices can be added by copying an existing .ini file and editing it where
appropriate.


## Usage
The primary user interface is a button in combination with an LED.

Pressing the button:
* Briefly will wake the monitor and check for warnings, then:
  * Issue status flashes if required
  * Run any controllers without scheduled run times if there is no globally-scheduled run time
  * Append to the log if there is no scheduled time to do so
* Holding for two LED blinks forces the log buffer to be writting out
* Holding for three LED blinks forces the controllers to be run
* Holding for four LED blinks sets the device time to a preset value, 12:00PM
by default
* Holding for longer will flash quickly twice and then do nothing

The status LED may flash as follows either when the button is pressed or when
status checks are scheduled (multiple warnings may be issued in succession):
* Once to indicate a power warning (low battery or low Vcc)
* Twice to indicate an actuator warning
* Three times to indicate a sensor warning
* Four times to indicate a controller warning
* Five times to indicate a logging warning

A primitive terminal can be reached over UART if nothing goes wrong during
initialiation by sending any data (e.g. a key press) while connected.


## Troubleshooting
If there's been an error in configuration or in device initialization, the
LED will turn on then off every half second. Connecting to a serial monitor
with `USE_UART_OUTPUT` set in `config.h` will give a more detailed error message.


## Libraries
GHMon uses my own uHAL and ulib libraries as well as elmchan's FatFS library. All
three are included in the `lib/` root directory.
