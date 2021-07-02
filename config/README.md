## Configuration
There are configuration file examples in `templates/`. General configuration
takes place in `config.h`, which contains descriptions of the available
options. `advanced.h` has additional rarely-needed options. Sensors and
controllers are defined in `sensors.c` and `controllers.c` respectively, and
look-up tables (if used) are defined tables.c. Descriptions of the settings
are contained in each file.


## Sensors
There are example sensor configurations in `config/templates/sensors.c` and
detailed documentation of the structures used to configure the sensors can be
found in `src/sensors.h` and the headers in `src/sensors/`. Configuration
consists of filling the `SENSORS[]` array with descriptions of the individual
sensors and how to interpret them. At the time of writing available sensor
types are linear-response and look-up table voltage and resistance sensors and
non-linear sensors with reference points and beta coefficients like thermistors
as well as DHT11 and BME280 and BMP280 sensors.

Resistance and voltage sensors are read through the internal ADC. They, along
with generic digital sensors like the DHT11, can be power-switched using
`SENSOR_POWER_PINS` defined in `config.h`; any, all, or none can be connected
to the pin(s).

SPI sensors (at present only the BM[EP]280) must either *all* be switched with
`SPI_POWER_PIN` defined in `config.h` or *all* be unswitched; leaving a device
unpowered when SPI is enabled may damage it when the SPI pins are high.
Additionally, the SD card (if used) must also be switched with them.
The chip select pin of an spi sensor is set using the 'pin' field of the
configuration struct.

I2C sensors (at present only the BM[EP]280) must either *all* be switched -
along with the I2C pullups - with `I2C_POWER_PIN` defined in `config.h` or
*all* be unswitched; leaving a device unpowered when I2C is enabled may damage
it when the pullups are high.
The address of an I2C sensor is set using the 'pin' field of the configuration
struct.

Any used sensor type must be enabled in `config.h`.


## Look-up tables
Look-up tables contain a list of values (such as degrees) and use the index
of the value to determine the resistance or voltage of the sensor
at that value. The fields `min` and `max` are mandatory and represent the
resistance or voltage at the first and last element respectively. If `table_multiplier`
is set and non-zero, the value is divided by it to obtain the final value.
If `Vref` is set and it's a voltage table, the min/max values are recalibrated
relative to the ADC voltage reference on the assumption that they're
input-dependent.

Tables must be ordered such that the first element represents the lowest
resistance or voltage and the increase between each element must be the same.
Values between elements are linearly interpolated.

There's a script at tools/VLUTgen.py for generating voltage look-up tables for
thermistors, but they need to be generated separately for each thermistor/
series resistor pairing.

There are settings related to look-up tables in both `config.h` and `advanced.h`.


## Controllers
There are example controller configurations in `config/templates/controllers.c`
and detailed documentation of the structures used to configure the controllers
can be found in `src/controllers.h`. Configuration consists of filling the
`CONTROLLERS[]` array with descriptions of the individual devices and how they're
tied to the sensors. If a controller needs to be tied to more than one sensor
then `CONTROLLER_SENS_COUNT` can be set to the desired number in `advanced.h`
and any unused inputs can have their sensor index set to `-1`.

A controller with no configured sensors will run every time it's scheduled to
be checked.
