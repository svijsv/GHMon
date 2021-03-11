##Introduction
GHMon is a configurable low-power firmware for autonomous small-scale
greenhouse control and data logging. At present it supports STM32F103
bluepill and compatible devices with wired resistance- and voltage-based
sensors.


##Configuring
Configuration is spread over 3-5 files (depending on enabled features) in
`config/` and examples for all the files can be found in `config/templates/`.
`config.h` houses basic setup like enabled features and pin configuration,
`advanced.h` is similar but only contains rarely-used settings, `sensors.c`
is the definitions for the sensors, `controllers.c` is the definitions of the
external device controllers (if enabled), and `tables.c` is used for lookup
tables (if a compatible sensor type is enabled). Descriptions of the settings
are contained in each file.


##Installing
GHMon uses [PlatformIO](https://platformio.org/) for building and installing.

By default flashing the firmware requires a USB-to-serial adapter with the
adapter's RX pin connected to pin A9 and the adapter's TX pin connected to
pin A10; any other supported method such as an stlink can be used by editing
platformio.ini appropriately.

To build and upload connect the serial adapter's RX, TX, GND, and 3.3V (if
otherwise unpowered) pins to the device, make sure the BOOT0 jumper is set to
1, connect the USB adapter to your PC, and follow the PlatformIO build and
upload directions. Once flashed set the BOOT0 jumper back to 0.


##Usage
The primary user interface is a button in combination with an LED.

* Pressing the button briefly will wake the monitor, blink the LED once, then
check the sensors and blink a few times if there are any warnings.
* Holding the button until it has blinked a total of two times will sync any
cached log data to the SD card (if it's enabled); it will blink once when done
if everything's OK or a few times if there was an error.
* Holding until three blinks forces a check for any device controllers that
need to be run (if they're enabled).
* Holding until four blinks sets the internal time to 12PM; the date is left
unchanged.
* Holding for five or more blinks cancels the command.

After releasing the button, the LED will blink a number of times to indicate
which action it will take, or else flash briefly twice to indicate it will
take no action.

If the monitor is busy when the button is pressed, the LED will flash briefly
and then continue with whatever it's doing.

A primitive serial terminal can be reached if nothing goes wrong during
initialiation by setting `USE_TERMINAL` in `config.h`.


##Hardware
See the schematics in `schematics/` for some typical setups.

If power usage is a concern, the power LED should be removed from the
development board.

Certain SD card Arduino modules designed to interface with 5V logic circuits
may not work.


##Troubleshooting
It's normal for the LED to blink several times at irregular intervals when
starting up.

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

If using an SD card for logging, a FAT codepage other then the default latin1
can be selected in `lib/fatfs/ffconf.h`.
