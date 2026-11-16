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

To install with Model Context Protocol (MCP) server support:
```bash
pip install -e ".[mcp]"
```

To install the *deprecated* user config gui, use the following:
```bash
pip install -e ".[gui]"
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

## DirtViz Python Client

The `ents.dirtviz` module provides a Python interface to interact directly with the [DirtViz](https://dirtviz.jlab.ucsc.edu/) REST API, including disk-backed response caching.

### Initialization

```python
from ents.dirtviz import BackendClient

# Connect to DirtViz API (default: https://dirtviz.jlab.ucsc.edu/api/)
client = BackendClient()

# Or specify a custom instance and cache location:
client = BackendClient(
    base_url="http://localhost:8000/api/",
    cache_dir="~/.cache/dirtviz",
)
```

### Cell Discovery & Lookup

```python
# List all cells
cells = client.cells()
for cell in cells:
    print(cell.id, cell.name, cell.location, cell.archive)

# Look up a cell by name or ID
cell = client.cell_from_name("Garden_Plot_1")
cell = client.cell_from_id(1)

# Flexible resolver (accepts Cell, int, numeric str, or case-insensitive name)
cell = client.resolve_cell("garden_plot_1")

# List cells by tag
sc_cells = client.cells_from_tag_name("Santa Cruz")
tag_cells = client.cells_from_tag_id(2)
```

### Sensor Metadata & Availability

```python
# List all sensors and measurement types for a cell
sensors = client.cell_sensors(cell)

# Retrieve the chartable dashboard sensor catalog
catalog = client.sensor_catalog(cell)

# Check timestamp range and recent upload status
availability = client.cell_availability(cell)
print("Earliest:", availability["earliest_timestamp"])
print("Latest:", availability["latest_timestamp"])
print("Active in last 14 days:", availability["has_recent_data"])
```

### Querying Timeseries Data

```python
from datetime import datetime

start = datetime(2026, 3, 1)
end = datetime(2026, 3, 20)

# Power measurements (columns: timestamp, v, i, p)
df_power = client.power_data(cell, start, end, resample="hour")

# TEROS-12 soil measurements (columns: timestamp, vwc, temp, ec)
df_teros = client.teros_data(cell, start, end, resample="hour")

# Generic sensor measurements (columns: timestamp, data, measurement, unit)
df_temp = client.sensor_data(
    cell, name="bme280", meas="BME280_TEMP", start=start, end=end
)
```

---

## DirtViz Model Context Protocol (MCP) Server

`ents.dirtviz.mcp` provides a Model Context Protocol (MCP) server that interfaces directly with the DirtViz API. It exposes discovery tools so AI coding assistants (such as Claude Desktop, Cursor, or Antigravity) can inspect the data schema and generate custom statistical analysis or plotting scripts.

### Running the MCP Server

Ensure optional MCP dependencies are installed:
```bash
pip install -e ".[mcp]"
```

#### Via CLI command:
```bash
# Run stdio transport (standard for local desktop AI tools)
ents dirtviz-mcp

# Or run as an SSE server on port 8001
ents dirtviz-mcp --transport sse --host 0.0.0.0 --port 8001

# Override API URL or disable caching:
ents dirtviz-mcp --url http://localhost:8000/api/ --no-cache
```

#### Via module invocation:
```bash
python -m ents.dirtviz.mcp --transport stdio
```

### Desktop AI Assistant Integration (stdio)

Add the following configuration to your MCP client config (e.g. `claude_desktop_config.json` or Antigravity):

```json
{
  "mcpServers": {
    "dirtviz": {
      "command": "ents",
      "args": ["dirtviz-mcp"]
    }
  }
}
```

### Available MCP Tools

| Tool | Parameters | Description |
| :--- | :--- | :--- |
| `list_cells` | `archive: bool = False` | List all cells with ID, name, coordinates, location, and archive status. |
| `get_cell` | `cell: str` | Get metadata for a specific cell by name or ID. |
| `list_cell_sensors` | `cell: str` | List available sensor names, measurement types, and units for a cell. |
| `get_cell_availability` | `cell: str` | Retrieve earliest and latest recorded timestamp bounds for a cell. |
| `get_cells_with_recent_data` | `hours: int = 24` | Find cells that have had data uploaded within the last $N$ hours. |
| `get_cells_by_tag` | `tag: str` | List cells tagged with a specific tag name or ID. |

---

## Testing

To run the package tests, create a virtual environment, install as an editable package (if you haven't done so already), and run `unittest`.

```bash
cd python/
python -m unittest
```
