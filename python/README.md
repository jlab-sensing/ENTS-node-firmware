# Soil Power Sensor Python Protobuf Bindings

The soil power sensor protobuf protocol is implemented as a Python package that allows for `Measurement` messages to be decoded into a dictionary and `Response` messages to be encoded. The generated files from protobuf are also accessible for more complex use cases.


## Installation

Use the following to install the `ents` package with gui via `pip`:

```bash
pip install ents[gui]
```

You can also install the package from source with the following:

```bash
# install package
pip install .[gui]
```

If you are planning to develop the package we recommend you install the package
in editable mode with development dependencies. This allows you to make changes
to the source code and have them reflected in the package without needing to
reinstall it.

```bash
# install development dependencies
pip install -e .[gui,dev]
```

## Usage

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


## Simulator

Simulate WiFi sensor uploads without requiring ENTS hardware.

### Examples

The examples below can be tested standalone (without ents-backend), by running the http server in `tools/http_server.py` to see the request format.

#### Upload a days worth of power measurements on a 60 second interval

```shell
ents sim --url http://localhost:3000/api/sensor/ --mode batch --sensor power --cell 200 --logger 200 --start 2025-05-01 --end 2025-05-02 --freq 60
```

```
...
total: 1437, failed: 0, avg (ms): 0.10716012526096033, last (ms): 0.0896
total: 1438, failed: 0, avg (ms): 0.10714290681502087, last (ms): 0.0824
total: 1439, failed: 0, avg (ms): 0.10712599027102154, last (ms): 0.0828
total: 1440, failed: 0, avg (ms): 0.10710909722222223, last (ms): 0.0828
total: 1441, failed: 0, avg (ms): 0.10709035392088828, last (ms): 0.08009999999999999
Done!
```

#### Upload measurements every 10 seconds

```shell
ents sim --url http://localhost:3000/api/sensor/ --mode stream --sensor power --cell 200 --logger 200 --freq 10
```

```
Use CTRL+C to stop the simulation
total: 1, failed: 1, avg (ms): 23.386100000000003, last (ms): 23.386100000000003
total: 2, failed: 2, avg (ms): 13.668950000000002, last (ms): 3.9517999999999995
total: 3, failed: 3, avg (ms): 10.795566666666668, last (ms): 5.0488
total: 4, failed: 4, avg (ms): 8.97235, last (ms): 3.5027000000000004
```

## Testing

To run the package tests, create a virtual environment, install as an editable package (if you haven't done so already), and run `unittest`.

```bash
cd python/
python -m unittest
```
