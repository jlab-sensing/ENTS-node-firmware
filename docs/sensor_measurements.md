## Sensor Measurements

This document provides an overview on the sensor measurement protocol.


Every field from a sensor measurement is represented as an individual message. For example power data is comprised of voltage and current field which each have their own measurement. They can be combined into a repeated measurement message that allows for better packing of variables. Even if a single measurement is sent, it is still wrapped in a repeated measurement message.

Each measurement can hold their own metadata that will override the metadata for in repeated sensor measurements. For example, voltage and current fields for power data likely are measuring the same thing, at the same time, and from the same device. Thus metadata can be combined in the repeated measurement to reduce number of bytes sent.

Another example is when multiple SDI-12 sensors are connected to the same device but are measuring different things. A single repeated measurement can contain multiple SDI-12 sensors all with different metadata.

The metadata is an all-or-nothing field. You cannot store just timestamp in the repeated measurement message and store additional metadata in the individual measurement messages. Metadata of the individual measurements will superceed the repeated measurement metadata if it is present.


### Versioning

There are two version of the sensor measurement protocol. Version 1 is the original version that uses individual messages for each sensor measurement. Version 2 is described above.

When uploading over LoRaWAN `fport` is used to indicate which version of the protocol is being used. `fport=1` indicates version 1 and `fport=2` indicates version 2.

When uploading over WiFi a header is used to indicate the version of the protocol. The header `SensorVersion: 1` indicates version 1 and `SensorVersion: 2` indicates version 2. If the hedaer is not specificed then the fallback should be version 1.


### Contributing

Adding support for a new sensor to the protocol involves (1) updating sensors list in protobuf definitions, (2) implementing drivers for the sensor, and (3) updating decoding logic in the python module.

#### Protobuf Definitions

To add a new sensor a additional enum value needs to be added to `SensorType` in `sensor.proto`. The convention is `[SENSOR]_[FIELD]`. The sensor should be added at the end of the enum and incremented by 1 from the previous value. *Never* change the value of an existing sensor measurement if it has been merged into main.

```
enum SensorType {
  NONE = 0;

  /** Onboard power measurements */
  POWER_VOLTAGE = 1;
  POWER_CURRENT = 2;
  ...
```

Also add it to `soil_power_sensor.proto`:

New message
```
message BME280Measurement {
  // pressure
  uint32 pressure = 1;
  // temperature
  int32 temperature = 2;
  // humidity
  uint32 humidity = 3;
}
```
New entry to the end of the `Measurement` message:
```
/* Top level measurement message */
message Measurement {
  // Metadata
  MeasurementMetadata meta = 1;

  // Possible measurements
  oneof measurement {
    PowerMeasurement power = 2;
    Teros12Measurement teros12 = 3;
    Phytos31Measurement phytos31 = 4;
    BME280Measurement bme280 = 5;
    Teros21Measurement teros21 = 6;
    SEN0308Measurement sen0308 = 7;
    SEN0257Measurement sen0257 = 8;
    YFS210CMeasurement yfs210c = 9;
    PCAP02Measurement pcap02 = 10;
    D10Measurement d10 = 11;
    WATERMARK200SSMeasurement watermark200ss = 12;
    WATERMARK200TSMeasurement watermark200ts = 13;
    EDU0157Measurement edu0157 = 14;
  }
}
```

Add to the end of `EnabledSensor`:
```
enum EnabledSensor {
  Voltage = 0;
  Current = 1;
  Teros12 = 2;
  Teros21 = 3;
  BME280 = 4;
  Phytos31 = 5;
  SEN0308 = 6;
  SEN0257 = 7;
  YFS210C = 8;
  PCAP02 = 9;
  D10 = 10;
  WATERMARK200SS = 11;
  WATERMARK200TS = 12;
  EDU0157 = 13;
}
```

##### Regenerate the Protobuf Files
In the `proto/` folder, use the `make` command to build the generated C code and Python code based on the updated `*.proto` files.

*Note*: The generated Python files might not pass the linter. When linting, use `ruff check python --fix` to apply fixes to the Python files.

*Note*: In `python/src/ents/proto/soil_power_sensor_pb2.py` you may need to make the import on line 25 relative by prepending `from .` to the beginning of the line like so: `from . import sensor_pb2 as sensor__pb2`

##### Updating Protobuf-dependent Files
The following files should be updated:
- `esp32/data/index.html` : Add the sensor to the sensor selection list using the `EnabledSensor` name, and give the entry a brief description for the user to read when selecting sensors. Additionally, add the sensor to the local input validation.
```
<select name="selected_sensor_${i}" id="sensor_dropdown_${i}">
    <option value="">none</option>
    <option value="Voltage">ADS1219 Voltage</option>
    <option value="Current">ADS1219 Current</option>
    <option value="Teros12">Teros12 Soil Moisture Sensor</option>
    <option value="Teros21">Teros21 Soil Tensiometer</option>
    <option value="BME280">BME280 Temperature, Humidity, Barometric Pressure</option>
    <option value="Phytos31">Phytos31 Leaf Wetness Sensor</option>
    <option value="SEN0308">SEN0308 Capacitive Soil Moisture Sensor</option>
    <option value="SEN0257">SEN0257 Water Pressure Sensor</option>
    <option value="YFS210C">YFS210C Water Flow Meter</option>
    <option value="PCAP02">PCAP02 Capacitance Sensor</option>
    <option value="D10">D10 Water Meter</option>
    <option value="WATERMARK200SS">Watermark 200SS Soil Moisture Sensor</option>
    <option value="WATERMARK200TS">Watermark 200TS Temperature Sensor</option>
    <option value="EDU0157">EDU0157 Weather Sensor</option>
</select>
...
const SENSORS = {
    Voltage: {
      type: "i2c",
      defaultIndex: "0x40",
      validIndexes: ["0x40"], // need to add others once i find out
      conflicts: ["SEN0308"]
    },
    ...
```
- `esp32/lib/user_config/src/config_server.cpp` : Add `EnabledSensor_{sensorname}` to the switch case to parse the provided index appropriately (based on whether it is to be interpreted as an I2C address, SDI-12 address, ADC / GPIO pin number, etc) and add the default index/address.
```
// Apply sensor index default or user-provided value.
switch (config.enabled_sensors_multiple[config.enabled_sensors_multiple_count].enabled_sensor) {
    case EnabledSensor_Teros12:
    case EnabledSensor_Teros21:
    // SDI-12: Index field may hold an ASCII character indicating sensor address.
        if (selected_sensor_index == "") {
          // Default SDI-12 address is '0'
          config.enabled_sensors_multiple[config.enabled_sensors_multiple_count]
              .index = '0';
        } else {
          config.enabled_sensors_multiple[config.enabled_sensors_multiple_count]
              .index = selected_sensor_index.c_str()[0];
        }
        break;
    case EnabledSensor_BME280:
    case EnabledSensor_PCAP02:
    case EnabledSensor_Voltage:
    case EnabledSensor_Current:
    case EnabledSensor_EDU0157:
```

#### Implementation

This part is entirely up to you. We recommend using I2C or SDI12 sensors for better support. In the case of other sensors (analog, SPI, UART), we recommend wrapping them in a Arduino framework microcontroller and interfacing with the stm32 over I2C.

In addition to your implementation (in `stm32/lib/<new_library>/`), you should also update/add the following:
- `stm32/Src/examples/example_<new_library>.c` : It is recommended to create an example program that reads the sensor and prints it to serial.
- `stm32/Src/main.c` : Include the library at the top of this file, and also add its initialization and measurement function to the for loop which builds the sensor measurement list.
```
// configure enabled sensors
  for (int i = 0; i < cfg->enabled_sensors_multiple_count; i++) {
    EnabledSensor sensor = cfg->enabled_sensors_multiple[i].enabled_sensor;
    EnabledSensorMultiple* sensor_ctx = &(cfg->enabled_sensors_multiple[i]);
    if (sensor == EnabledSensor_Voltage) {
      ADC_init();
      SensorsAdd(ADC_measureVoltage, sensor_ctx);
      APP_LOG(TS_OFF, VLEVEL_M, "Voltage Enabled!\n");
    }
    if (sensor == EnabledSensor_Current) {
      ADC_init();
      SensorsAdd(ADC_measureCurrent, sensor_ctx);
      APP_LOG(TS_OFF, VLEVEL_M, "Current Enabled!\n");
    }
    ...
```
- `stm32/test/test_<new_library>/test_<new_library>.c` : Add a test harness that can be added to the `platformio.ini` file to test the functionality of the library on subsequent builds. Useful for catching regressions.
- `stm32/platformio.ini` : Add an environment for the example program. Additionally, add the test harness to the [TODO] `test_filter` for hardware verification.
```
[env:example_watermark200SSVA3]
build_src_filter = +<*> -<.git/> -<main.c> -<examples/**> +<examples/example_watermark200SSVA3.c>
```

#### Updating the Python module

The python module is responsible for decoding the sensor measuremnts and providing other baked in metdata. See the following [bme280 example](https://github.com/jlab-sensing/ENTS-node-firmware/blob/e174cd829de5b36758efc08350ed5bb71eb321a0/python/src/ents/proto/decode.py#L73-L76) for reference. This allows integer values to be sent and converted to floats for user convenience. This is not necessary for sensor data that is scaled properly or does not require post-processing.

```
if meta_dict["type"] == "bme280":
    meta_dict["data"]["pressure"] /= 10.0
    meta_dict["data"]["temperature"] /= 100.0
    meta_dict["data"]["humidity"] /= 1000.0
```

In `python/src/ents/proto/sensor.py` add an entry to the SENSOR_DATA dictionary in `get_sensor_data()` to provide a human name for the measurement and the units. These are used for parsing and displaying the data on a graph.
```
SENSOR_DATA = {
    SensorType.POWER_VOLTAGE: {
        "name": "Voltage",
        "unit": "mV",
    },
    SensorType.POWER_CURRENT: {
        "name": "Current",
        "unit": "uA",
    },
    ...
```
TODO: Users can update the `encode_user_configuration()` function (and add an encode function?) in `python/src/ents/proto/encode.py`.

#### Publishing the Release

1. Update [changelog](https://github.com/jlab-sensing/ENTS-node-firmware/blob/main/CHANGELOG.md) (what changed & links to issues and PRs). In this step, the changelog should be labeled as "Unreleased".
2. Merge the PR.
3. Update version numbers: Python package in [pyproject.toml](https://github.com/jlab-sensing/ENTS-node-firmware/blob/main/python/pyproject.toml#L7), Doxygen [Doxyfile](https://github.com/jlab-sensing/ENTS-node-firmware/blob/main/doxygen/Doxyfile#L51), protobuf platformIO [library.json](https://github.com/jlab-sensing/ENTS-node-firmware/blob/main/proto/c/library.json#L3), and update the [changelog](https://github.com/jlab-sensing/ENTS-node-firmware/blob/main/CHANGELOG.md) from unreleased to the current date.
4. Commit the changes with a message describing the version bump.
5. `git tag <version_number>`
6. `git push`
7. `git push --tags`
8. Wait for the Python action to run. This will automatically create a GitHub release for you.
9. Copy over the changelog to the newly created release on GitHub.
10. Check the [ents Python package](https://pypi.org/project/ents/) to see if it updated correctly. The [Release History](https://pypi.org/project/ents/#history) tab should show you the updated version and corrected timestamp.

### New Sensor Interface (V2)

The new sensor interface the protojson format as an intermediary. See `sensors.py` for usage.

TODO Add examples of usage.


### Old Implementation (v1)

> NOTE: This functionality is deprecated and the new interface should be used instead.

The following example code demonstrates decoding the measurement message and encoding a response.

```python
from ents import encode, decode

# get data encoded by the soil power sensor
data = ...

meas_dict = decode(data)

# process data
...

# send response
resp_str = encode(success=True)
```

The formatting of the dictionary depends on the type of measurement sent. The key `type` is included on all measurement types and can be used to determine the type of message. See the source `*.proto` files to get the full list of types to get the full list of types and keys. A list is provided in [Message Types](#message-types). The Python protobuf API uses camel case when naming keys. The key `ts` is in ISO 8601 format as a string.

## Message Types

Type `power`
```python
meas_dict = {
  "type": "power",
  "loggerId": ...,
  "cellId": ...,
  "ts": ...,
  "data": {
    "voltage": ...,
    "current": ...
  },
  "data_type": {
    "voltage": float,
    "voltage": float
  }
}
```

Type `teros12`
```python
meas_dict = {
  "type": "teros12",
  "loggerId": ...,
  "cellId": ...,
  "ts": ...,
  "data": {
    "vwcRaw": ...,
    "vwcAdj": ...,
    "temp": ...,
    "ec": ...
  },
  "data_type": {
    "vwcRaw": float,
    "vwcAdj": float,
    "temp": float,
    "ec": int
  }
}
```

Type `bme280` with `raw=True` (default)
```python
meas_dict = {
  "type": "bme280",
  "loggerId": ...,
  "cellId": ...,
  "ts": ...,
  "data": {
    "pressure": ...,
    "temperature": ...,
    "humidity": ...,
  },
  "data_type": {
    "pressure": int,
    "temperature": int,
    "humidity": int, 
  }
}
```

Type `bme280` with `raw=False`
```python
meas_dict = {
  "type": "bme280",
  "loggerId": ...,
  "cellId": ...,
  "ts": ...,
  "data": {
    "pressure": ...,
    "temperature": ...,
    "humidity": ...,
  },
  "data_type": {
    "pressure": float,
    "temperature": float,
    "humidity": float, 
  }
}
```
