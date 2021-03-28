## Introduction
GHMon is a configurable low-power firmware for autonomous small-scale
greenhouse control and data logging. At present it supports STM32F103-based
Bluepill and ATMega328-based Arduino devices with wired resistance- and
voltage-based sensors.


## Configuring
Configuration is spread over 3-5 files (depending on enabled features) in
`config/` and examples for all the files can be found in `config/templates/`.
More detailed configuration information can be found in `config/README.md`.


## Installing
GHMon uses [PlatformIO](https://platformio.org/) for building and installing.

Build configuration files are provided for a few boards and can be found in
`pio_inis/`; they're included by `platformio.ini` by default. New (compatible)
boards can be added by copying an existing .ini file and editing it where
appropriate, typically just by changing the `board = X` line.


## Usage
The primary user interface is a button in combination with an LED.

* Pressing the button briefly will wake the monitor, blink the LED once, then
check the sensors and blink a few times if there are any warnings.
* Holding the button until it has blinked a total of two times will sync any
cached log data to the SD card (if it's enabled); it will blink once when done
if everything's OK or a few times if there was an error.
* Holding until three blinks forces a check for any device controllers that
need to be run (if they're enabled).
* Holding until four blinks sets the internal time to 12PM.
* Holding for five or more blinks cancels the command.

After releasing the button, the LED will blink a number of times to indicate
which action it will take, or else flash briefly twice to indicate it will
take no action.

If the monitor is busy when the button is pressed, the LED will flash briefly
and then continue with whatever it's doing.

A primitive serial terminal can be reached if nothing goes wrong during
initialiation by setting `USE_TERMINAL` in `config.h`.


## Hardware
Testing has been done with STM32F103 Bluepill, Arduino Pro Mini 3.3V/8MHz, and
Arduino UNO R3 boards but it should be possible to run on any board with an
STM32F103, ATMega328, or ATMega168 MCU provided a new PlatformIO build target
is configured for it.

See the schematics in `schematics/` for some typical setups.

If power usage is a concern, the power LED should be removed from the
development board.

Certain SD card Arduino modules designed to interface with 5V logic circuits
may not work with 3.3v boards.


## Troubleshooting
It's normal for the LED to blink several times at irregular intervals when
powering on.

If there's been an error in configuration or in device initialization, the
LED will turn on then off every half second. Connecting to a serial monitor
with `USE_SERIAL` set in `config.h` will give a more detailed error message.
Otherwise, check to make sure that:

* `SENSOR_COUNT` and `CONTROLLER_COUNT` match the number of sensors and
controllers defined in `sensors.c` and `controllers.c`
* The correct `devcfg` field has been set for each chosen sensor type

If using an SD card for logging and it isn't being recognized, [this](https://www.sdcard.org/downloads/formatter/)
(ideally) or the internal formatter (with `USE_FDISK` set in `config.h`) may
fix the problem.

If using an SD card for logging, a FAT codepage other than the default latin1
can be chosen in `lib/fatfs/ffconf.h`.
