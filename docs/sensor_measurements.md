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
