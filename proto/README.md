# Soil Power Sensor Protocol Buffers

[Protobuf](https://protobuf.dev/) is used to serialize measurement data when uploading data. Over LoRa this is strictly necessary due to payload size restrictions of the LoRaWAN protocol.

![Serialized data block diagram](/images/Soil Power Sensor - meas.jpeg) {html: width=80%}

## Building

The generation of necessary source files is handled with the `Makefile`. The `nanopb_generator` binary is required for the `c` implementation and can be installed through the top-level `requirements.txt`. Run the following to generate the source files.

```bash
make
```

## Steps for adding a new sensor

1. Create a new protobuf file in `proto/` with the name of the sensor. The file should contain the message structure for the sensor.
2. Add the new message to the message `Measurement` in `proto/soil_power_sensor.proto`. This allows the sensor to be decoded on the backend.
3. Build the protocol buffers
4. Add *python* bindings for the new message. This includes writing the necessary unit tests and expanding the cli interface.
5. Add *c* bindings for the new message. This includes writing unit tests for the functions.
6. Implement the hardware interface to the sensor and encode the data with the new message format.


TODO expand

## Decision for message structure

> NOTE: The voltage and current measurements are implemented differently than what is described below and will be changed in the future.

We wanted to create a message interface for encoding that allowed for efficient serialization of data and ability to support new messages in the future. We decided to have a single message per *sensor reading*. For example the Teros12 reads volumetric water content, temperature and electrical conductivity which is all read at the same time so its included in a single message. Power data consists of a voltage and current reading but we do not combine them into a single measurement since they can also be read individual (ie voltage is only measured from SMFCs in open circuit configuration).

Having specifically designed messages was chosen over an arbitrary interface like that of the backend that accepts measurements of `int`, `float`, and `string`. This was due to the firmware already requiring modification to support additional sensors. This essentially means that the list of supported sensors is hardcoded into the firmware. The gui could be modified to support creating new sensors and uploading the `id` to indicate the measurement type, but would require internet access when configuring the sensors which we cannot assume.

New sensors added to the platform should follow this convention of having a single message per sensor reading. This allows for easy extensibility of the protocol and allows for the addition of new sensors without needing to change the existing messages.

## Best practices

1. Integer data types are preferred. Protobuf encoded integers using varints, so there are space savings to be had by using integers instead of floats.
2. For strings and byte arrays, specify the max length in the `*.options` file. See more details [here](https://jpa.kapsi.fi/nanopb/docs/reference.html#generator-options)

## Example messages

In `examples/` there are example binary files for each measurement type

*PowerMeasurement* (`examples/meas_power.bin`)

| field | value |
| --- | --- |
| ts | 1436079600 |
| logger_id | 7 |
| cell_id | 4 |
| voltage | 2400.0 |
| current | 100.0 |

*Teros12Measurement* (`examples/meas_teros.bin`)

| field | value |
| --- | --- |
| ts | 1436079600 |
| logger_id | 0 |
| cell_id | 0 |
| vwc_raw | 1832.43 |
| vwc_adj | 34.4 |
| temp | 24.2 |
| ec | 2 | 

## Decoding a message using the CLI

On linux you are able to decode a payload directly from The Things Network by using the `protoc` binary. Replace the hex at the start of your command with the payload.

``` bash
echo "0A1008A3A18A820410041A0608F0ABE3AC75121211713D0AD7A390424019E17A14AE47296740" | xxd -r -p | protoc --decode Measurement soil_power_sensor.proto 
meta {
  cell_id: 1078104227
  logger_id: 4
  3 {
    1: 31500850672
  }
}
power {
  voltage: 37.13
  current: 185.29
}
```

## C Implementation

> See @subpage protobuf-c for implementation details. 

On the *Wio-E5 mini* module, protocol buffers are implemented using [Nanopb](https://jpa.kapsi.fi/nanopb/) allowing for a plain-C implementation with minimal code size. The source files are structured into a PlatformIO library so the *stm32* project can build the source files using `lib_deps = soil_power_sensor_protobuf=symlink://../proto/c`. The source files for **Nanopb** were copied directly due to mismatching file structure. Additionally `timestamp.proto` was included from the **protobuf** to get compatable sources file for the well known `Timestamp` message type. A unit test is proved in `stm32/tests/test_proto`.

To generate only the C source files run the following:

```bash
make c
```

## Python package

> See @subpage protobuf-python "Python Protobuf Bindings" for implementation details.

The python bindings for the Soil Power Sensor messages are provided within a python package uploaded to PyPI. See [README.md](./python/README.md) in `python/` for details on usage.

### Development

It is recommended to make a python virtual environment and install the package as editable. From the repository root run the following:

```bash
cd proto/
python -m venv .venv
source .venv/bin/activate
pip install -e python
```

To generate the python source files for protobuf, run the following:

```bash
make python
```
