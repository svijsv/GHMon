## Configuration
There are configuration file examples in `templates/`. General configuration
takes place in `config.h`, which contains descriptions of the available
options. `advanced.h` has additional rarely-needed options. Sensors and
controllers are defined in `sensors.c` and `controllers.c` respectively.


## Sensors
Complete documentation of the structures used to configure the sensors can be
found in `src/sensors.h`. Configuration consists of filling the SENSORS
array with descriptions of the individual sensors and how to interpret them.
At the time of writing available sensor types are linear, lookup-table, and
direct measurment voltage- and resistance-based sensors and sensors with
reference points and beta coefficients like thermistors.

Any used sensor type must be enabled in `config.h`.


## Controllers
Complete documentation of the structures used to configure the controllers can
be found in `src/controllers.h`. Configuration consists of filling the
CONTROLLERS array with descriptions of the individual devices and how they're
tied to the sensors. If a controller needs to be tied to more than one sensor
then `CONTROLLER_SENS_COUNT` can be set to the desired number in `advanced.h`
and any unused inputs can have their sensor index set to `-1`.


## Look-up tables
Look-up tables contain a list of values (such as degrees in any unit) and use
the index of the value to determine the resistance or voltage of the sensor
at that value. The fields `min` and `max` are mandatory and represent the
resistance or voltage at the first and last element respectively. If `scale`
is set and non-zero, the value is divided by it to obtain the final value.
If `Vref` is set and it's a voltage table, the min/max values are recalibrated
relative to the ADC voltage reference on the assumption that they're
input-dependent.

Tables must be ordered such that the first element represents the lowest
resistance or voltage and the increase between each element must be the same.
Values between elements are linearly interpolated.

There's a script at tools/VLUTgen.py for generating voltage lookup tables for
thermistors, but they need to be generated separately for each thermistor/
series resistor pairing.

There are settings related to look-up tables in both `config.h` and `advanced.h`.
