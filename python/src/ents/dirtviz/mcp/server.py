"""DirtViz MCP Server factory and runner.

Sets up MCPServer, binds tools with a configured BackendClient instance,
provides entry points for stdio and SSE transport execution, and exposes
scripting instructions, resources, and prompt templates for AI agents.
"""

import os
import tempfile
from pathlib import Path
from typing import Any

from mcp.server import MCPServer

from ents.dirtviz.client import BackendClient
from ents.dirtviz.mcp.tools import (
    get_cell,
    get_cell_availability,
    get_cells_by_tag,
    get_cells_with_recent_data,
    list_cell_sensors,
    list_cells,
)

DIRTVIZ_INSTRUCTIONS = """DirtViz MCP Server: AI Discovery & Analysis Script Synthesis Guide

## Primary Purpose
This server provides discovery and metadata endpoints directly interfacing with the
DirtViz REST API (cells, sensor catalogs, data availability, tags).

## Agent Workflow
1. DISCOVERY:
   - Call `list_cells(archive=False)` or `get_cells_by_tag(tag)` to identify cells of interest.
   - Call `list_cell_sensors(cell)` to discover valid sensor measurements, engineering units, and sensor names.
   - Call `get_cell_availability(cell)` to find the earliest and latest recorded timestamp bounds.
   - Call `get_cells_with_recent_data(hours=24)` to check which cells are actively streaming data.

2. SCRIPT SYNTHESIS (DO NOT ATTEMPT TO PULL MASSIVE TIMESERIES OVER MCP):
   - Rather than streaming large timeseries data through MCP tool responses, write a standalone
     Python script using `ents.dirtviz.BackendClient` to fetch data with server-side resampling,
     perform statistical analysis (pandas, scipy, statsmodels, scikit-learn), or generate publication-grade
     plots (matplotlib, seaborn).

3. SCRIPT IMPLEMENTATION PATTERNS (from python/examples and ents.dirtviz.client):
   - Initializing Client:
     ```python
     from datetime import datetime
     from ents.dirtviz import BackendClient
     import matplotlib.pyplot as plt
     import pandas as pd

     client = BackendClient()  # Uses DIRTVIZ_API_URL or default https://dirtviz.jlab.ucsc.edu/api/
     ```

   - Resolving Cells & Querying Data:
     ```python
     cell = client.resolve_cell("sc_synth_open_1")  # Accepts Cell, int ID, or name
     start = datetime(2025, 2, 12)
     end = datetime(2025, 7, 25)

     # Power Data (columns: timestamp, v, i, p)
     df_power = client.power_data(cell, start, end, resample="hour")

     # TEROS Soil Data (columns: timestamp, vwc, temp, ec)
     df_teros = client.teros_data(cell, start, end, resample="hour")

     # Generic / Ambient Sensors (e.g. bme280, teros21)
     df_temp = client.sensor_data_simple(
         cell, name="bme280", meas="temperature", start=start, end=end, resample="hour"
     )
     ```

   - Merging Multi-Sensor Datasets (from python/examples/pca/pca.py):
     ```python
     # Inner join on timestamp to align all time-series measurements
     df_cell = pd.merge(df_power, df_teros, on="timestamp", how="inner")
     df_cell = pd.merge(df_cell, df_temp, on="timestamp", how="inner")
     ```

   - Multi-Cell Comparison Plotting (from python/examples/power.py):
     ```python
     # Plot individual cell lines with low opacity (alpha=0.3)
     # Overlay group mean line (linewidth=2) and 25%-75% quantile shaded region
     fig, ax = plt.subplots(figsize=(10, 6))
     for cell in cells:
         df = client.power_data(cell, start, end, resample="hour")
         ax.plot(df["timestamp"], df["v"], alpha=0.3, linewidth=0.5, label=cell.name)
     ```
"""


def create_mcp_server(
    client: BackendClient | None = None,
    name: str = "DirtViz MCP Server",
    base_url: str | None = None,
    cache_dir: str | Path | None = None,
) -> Any:
    """Create and configure a DirtViz MCP server instance.

    Args:
        client: Optional preconfigured BackendClient. If not provided, one will be
            created using base_url or the DIRTVIZ_API_URL environment variable.
        name: Name for the MCP server.
        base_url: API base URL override (defaults to DIRTVIZ_API_URL or DirtViz production).
        cache_dir: Directory for response caching.

    Returns:
        Configured MCPServer instance with all tools, resources, and prompts registered.
    """
    if client is None:
        url = base_url or os.environ.get(
            "DIRTVIZ_API_URL", "https://dirtviz.jlab.ucsc.edu/api/"
        )
        client = BackendClient(base_url=url, cache_dir=cache_dir)

    server = MCPServer(
        name,
        instructions=DIRTVIZ_INSTRUCTIONS,
        description=(
            "DirtViz MCP Server: Tools for discovering cells, sensor catalogs, "
            "and data availability directly from the DirtViz REST API."
        ),
    )

    # Attach client for external access in tests
    server.client = client

    #
    # Tools
    #
    @server.tool(
        name="list_cells",
        description=(
            "List all available cells with their IDs, names, locations, coordinates, and archive status. "
            "Call this first to discover available cell targets before authoring analysis scripts."
        ),
    )
    def tool_list_cells(archive: bool = False) -> list[dict[str, Any]]:
        return list_cells(client=client, archive=archive)

    @server.tool(
        name="get_cell",
        description=(
            "Get full metadata for a specific cell by name or numeric ID. "
            "Returns cell ID, name, location, latitude, longitude, and archive status."
        ),
    )
    def tool_get_cell(cell: str) -> dict[str, Any]:
        return get_cell(client=client, cell=cell)

    @server.tool(
        name="list_cell_sensors",
        description=(
            "List all available sensor measurements, data types, and engineering units for a specific cell. "
            "Always inspect this before querying sensor data or authoring analysis scripts to know valid sensor/measurement keys."
        ),
    )
    def tool_list_cell_sensors(cell: str) -> list[dict[str, Any]]:
        return list_cell_sensors(client=client, cell=cell)

    @server.tool(
        name="get_cell_availability",
        description=(
            "Get the earliest and latest recorded data timestamps for a cell. "
            "Always check this before writing scripts to select valid date ranges and avoid empty query results."
        ),
    )
    def tool_get_cell_availability(cell: str) -> dict[str, Any]:
        return get_cell_availability(client=client, cell=cell)

    @server.tool(
        name="get_cells_with_recent_data",
        description=(
            "Find all cells that have had data uploaded within the last N hours (default: 24). "
            "Returns active cells sorted with the most recently updated first."
        ),
    )
    def tool_get_cells_with_recent_data(hours: int = 24) -> list[dict[str, Any]]:
        return get_cells_with_recent_data(client=client, hours=hours)

    @server.tool(
        name="get_cells_by_tag",
        description=(
            "List all cells associated with a given tag name (e.g. 'Santa Cruz') or numeric tag ID. "
            "Useful for discovering groups of cells deployed in the same experimental batch."
        ),
    )
    def tool_get_cells_by_tag(tag: str) -> list[dict[str, Any]]:
        return get_cells_by_tag(client=client, tag=tag)

    #
    # Resources
    #
    @server.resource("dirtviz://docs/scripting_guide")
    def get_scripting_guide() -> str:
        """Provide the complete Python scripting guide and code templates for DirtViz."""
        return DIRTVIZ_INSTRUCTIONS

    #
    # Prompts
    #
    @server.prompt(
        name="generate_analysis_script",
        description=(
            "Guide the agent to generate a self-contained Python analysis and plotting script "
            "for a specific DirtViz cell using BackendClient."
        ),
    )
    def prompt_generate_analysis_script(
        cell: str, start_date: str = "", end_date: str = ""
    ) -> str:
        return (
            f"You are writing a Python analysis script for DirtViz cell '{cell}'.\n"
            "Follow these steps:\n"
            f"1. Use `list_cell_sensors('{cell}')` to see available sensors (power, teros, bme280, etc.).\n"
            f"2. Use `get_cell_availability('{cell}')` to verify the earliest and latest recorded timestamp bounds.\n"
            "3. Write and execute a standalone Python script using `ents.dirtviz.BackendClient`:\n"
            "   - Initialize `client = BackendClient()`.\n"
            f"   - Resolve cell: `cell_obj = client.resolve_cell('{cell}')`.\n"
            f"   - Query desired sensors between '{start_date}' and '{end_date}' (or the discovered available bounds) with `resample='hour'`.\n"
            "   - Merge sensor DataFrames on 'timestamp' if analyzing multiple sensors.\n"
            "   - Compute summary statistics (mean, median, min, max, std).\n"
            "   - Generate a publication-quality Matplotlib/Seaborn plot and save it to disk."
        )

    @server.prompt(
        name="compare_cells_script",
        description=(
            "Guide the agent to generate a multi-cell comparative timeseries plot "
            "(individual lines + group mean and quantile bands, following python/examples/power.py)."
        ),
    )
    def prompt_compare_cells_script(
        cells: str, tag: str = "", measurement: str = "power"
    ) -> str:
        cells_context = f"cells tagged with '{tag}'" if tag else f"cells: {cells}"
        return (
            f"You are generating a multi-cell comparative plotting script for {cells_context}.\n"
            "Follow the visualization design pattern from `python/examples/power.py`:\n"
            "1. If a tag is specified, use `get_cells_by_tag(tag)` to find all associated cells.\n"
            "2. Check availability bounds across the cells to determine a common overlapping time window.\n"
            "3. Write a Python script using `ents.dirtviz.BackendClient`:\n"
            "   - Fetch measurements for each cell with `resample='hour'`.\n"
            "   - For each cell, plot individual lines with low opacity (alpha=0.3, linewidth=0.5).\n"
            "   - Calculate the overall timestamp group mean and overlay a thick solid line (linewidth=2).\n"
            "   - Calculate the 25th and 75th percentiles (quantiles) across cells and fill between them with alpha=0.2.\n"
            "   - Add dashed zero-line (y=0), grid, labels, and legend, and save the figure."
        )

    @server.prompt(
        name="pca_analysis_script",
        description=(
            "Guide the agent to synthesize a multi-sensor PCA or regression pipeline across cells "
            "(following python/examples/pca/pca.py and python/examples/anova/anova.py)."
        ),
    )
    def prompt_pca_analysis_script(
        cells: str, start_date: str = "", end_date: str = ""
    ) -> str:
        return (
            f"You are creating a Principal Component Analysis (PCA) script for cells: {cells}.\n"
            "Follow the multi-sensor feature alignment pattern from `python/examples/pca/pca.py`:\n"
            "1. For each cell, discover all available sensors using `list_cell_sensors`.\n"
            "2. Write a Python script using `ents.dirtviz.BackendClient`:\n"
            f"   - Query power, teros, and ambient sensors over '{start_date}' to '{end_date}' with `resample='hour'`.\n"
            "   - Inner merge the DataFrames on 'timestamp' to construct an aligned multi-sensor feature matrix.\n"
            "   - Handle missing values.\n"
            "   - Standardize features using `sklearn.preprocessing.StandardScaler`.\n"
            "   - Apply `sklearn.decomposition.PCA(n_components=...)` and compute explained variance ratios.\n"
            "   - Plot the PCA biplot / scree plot and output factor loadings."
        )

    return server


def run_mcp_server(
    transport: str = "stdio",
    host: str = "0.0.0.0",
    port: int = 8001,
    url: str | None = None,
    no_cache: bool = False,
    argv: list[str] | None = None,
) -> None:
    """Run the DirtViz MCP server using the specified transport.

    Args:
        transport: Transport type ('stdio' or 'sse'). Defaults to 'stdio'.
        host: Host address for SSE transport. Defaults to '0.0.0.0'.
        port: Port number for SSE transport. Defaults to 8001.
        url: DirtViz API base URL override.
        no_cache: If True, uses a temporary cache directory discarded on exit.
        argv: Optional CLI arguments list.
    """
    cache_dir = None
    temp_dir_obj = None

    if no_cache:
        temp_dir_obj = tempfile.TemporaryDirectory(prefix="dirtviz_mcp_")
        cache_dir = temp_dir_obj.name

    try:
        server = create_mcp_server(base_url=url, cache_dir=cache_dir)
        if transport == "sse":
            server.run(transport="sse", host=host, port=port)
        else:
            server.run(transport="stdio")
    finally:
        if temp_dir_obj is not None:
            temp_dir_obj.cleanup()
