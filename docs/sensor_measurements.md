## Sensor Measurements

This document provides an overview on the sensor measurement protocol.


Every field from a sensor measurement is represented as an individual message. For example power data is comprised of voltage and current field which each have their own measurement. They can be combined into a repeated measurement message that allows for better packing of variables. Even if a single measurement is sent, it is still wrapped in a repeated measurement message.

Each measurement can hold their own metadata that will override the metadata for in repeated sensor measurements. For example, voltage and current fields for power data likely are measuring the same thing, at the same time, and from the same device. Thus metadata can be combined in the repeated measurement to reduce number of bytes sent.

Another example is when multiple SDI-12 sensors are connected to the same device but are measuring different things. A single repeated measurement can contain multiple SDI-12 sensors all with different metadata.

The metadata is an all-or-nothing field. You cannot store just timestamp in the repeated measurement message and store additional metadata in the individual measurement messages. Metadata of the individual measurements will superceed the repeated measurement metadata if it is present.


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


#### Implementation

This part is entirely up to you. We recommend using I2C or SDI12 sensors for better support. In the case of other sensors (analog, SPI, UART), we recommend wrapping them in a Arduino framework microcontroller and interfacing with the stm32 over I2C. 

#### Updating the Python module

The python module is responsible for decoding the sensor measuremnts and providing other baked in metdata. See the following [bme280 example](https://github.com/jlab-sensing/ENTS-node-firmware/blob/e174cd829de5b36758efc08350ed5bb71eb321a0/python/src/ents/proto/decode.py#L73-L76) for reference. This allows integer values to be sent and converted to floats for user convenience.

```
if meta_dict["type"] == "bme280":
    meta_dict["data"]["pressure"] /= 10.0
    meta_dict["data"]["temperature"] /= 100.0
    meta_dict["data"]["humidity"] /= 1000.0
```

