# Soil Power Sensor Python Protobuf Bindings

The soil power sensor protobuf protocol is implemented as a Python package that allows for `Measurement` messages to be decoded into a dictionary and `Response` messages to be encoded. The generated files from protobuf are also accessible for more complex use cases.


## Installation

Use the following to install the `ents` package with gui via `pip`:

```bash
pip install ents
```

You can also install the package from source with the following:

```bash
# install package
pip install .
```

If you are planning to develop the package we recommend you install the package
in editable mode with development dependencies. This allows you to make changes
to the source code and have them reflected in the package without needing to
reinstall it.

```bash
# install development dependencies
pip install -e .[dev]
```

To install the *deprecated* user config gui, use the following: =
```bash
pip install -e ents[gui]
```



## Simulator (New)

The webserver `tools/http_decoder.py` can be used to decode uploaded measurements.

### CLI Usage

```
usage: ents sim_generic [-h] [-v] [--url URL] --sensor SENSOR [SENSOR ...] [--min MIN] [--max MAX] --cell CELL --logger LOGGER [--start START] [--end END] [--freq FREQ] {batch,stream}

positional arguments:
  {batch,stream}        Upload mode

options:
  -h, --help            show this help message and exit
  -v, --verbose         Print addiitional request information.
  --url URL             URL of the dirtviz instance (default: http://localhost:8000)
  --sensor SENSOR [SENSOR ...]
                        Type of sensor to simulate
  --min MIN             Minimum sensor value (default: -1.0)
  --max MAX             Maximum sensor value (default: 1.0)
  --cell CELL           Cell Id
  --logger LOGGER       Logger Id

Batch:
  --start START         Start date
  --end END             End date

Stream:
  --freq FREQ           Frequency of uploads (default: 10s)
```

### Examples

You can find the available sensors in the `sensors.proto` file.

Example uploading single measurement
```
ents sim_generic stream --sensor POWER_VOLTAGE --min 20 --max 30 --cell 1 --logger 1
```

Example uploading multiple measuremnets
```
ents sim_generic stream --sensor TEROS12_VWC_ADJ TEROS12_TEMP TEROS12_EC --min 10 --max 100 --cell 1 --logger 1
```

Example batch uploads
```
ents sim_generic batch --sensor POWER_CURRENT --cell 1 --logger 1 --start 2026-01-19 --end 2026-01-20 --freq 60
```


## Simulator (Old)

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
