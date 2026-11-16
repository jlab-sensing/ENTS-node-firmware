"""MCP tool implementations for DirtViz API.

These tools provide discovery, metadata inspection, and data availability querying
directly against the DirtViz REST API using BackendClient.
"""

from datetime import UTC, datetime, timedelta
from typing import Any

import requests

from ents.dirtviz.client import BackendClient


def list_cells(client: BackendClient, archive: bool = False) -> list[dict[str, Any]]:
    """List all available cells with their names, IDs, locations, and archive status.

    Args:
        client: BackendClient instance configured with the DirtViz API.
        archive: Whether to include archived cells (default: False).

    Returns:
        List of cell dictionaries containing id, name, location, latitude,
        longitude, and archive status.
    """
    cells = client.cells()
    if not archive:
        cells = [c for c in cells if not c.archive]
    return [c.to_dict() for c in cells]


def get_cell(client: BackendClient, cell: str | int) -> dict[str, Any]:
    """Get metadata for a specific cell by name or ID.

    Args:
        client: BackendClient instance configured with the DirtViz API.
        cell: Cell name (case-insensitive) or integer ID.

    Returns:
        Dictionary containing cell id, name, location, latitude, longitude, and archive.
    """
    cell_obj = client.resolve_cell(cell)
    return cell_obj.to_dict()


def list_cell_sensors(client: BackendClient, cell: str | int) -> list[dict[str, Any]]:
    """List all available sensors and measurements for a specific cell.

    Args:
        client: BackendClient instance configured with the DirtViz API.
        cell: Cell name or integer ID.

    Returns:
        List of sensor dictionaries detailing sensor name, measurement type,
        data type, and engineering unit.
    """
    cell_obj = client.resolve_cell(cell)
    return client.cell_sensors(cell_obj)


def get_cell_availability(client: BackendClient, cell: str | int) -> dict[str, Any]:
    """Get the earliest and latest recorded data timestamps for a cell.

    Args:
        client: BackendClient instance configured with the DirtViz API.
        cell: Cell name or integer ID.

    Returns:
        Dictionary containing cell_id, cell_name, earliest_timestamp,
        latest_timestamp, and whether data exists in the last 14 days (has_recent_data).
    """
    cell_obj = client.resolve_cell(cell)
    avail = client.cell_availability(cell_obj.id)
    return {
        "cell_id": cell_obj.id,
        "cell_name": cell_obj.name,
        "earliest_timestamp": avail.get("earliest_timestamp"),
        "latest_timestamp": avail.get("latest_timestamp"),
        "has_recent_data": avail.get("has_recent_data", False),
    }


def get_cells_with_recent_data(
    client: BackendClient, hours: int = 24
) -> list[dict[str, Any]]:
    """Find all cells that have had data uploaded within the last N hours.

    Args:
        client: BackendClient instance configured with the DirtViz API.
        hours: Threshold in hours to consider data recent (default: 24).

    Returns:
        List of cell objects with latest_timestamp that have recorded readings
        within the requested time window, sorted with most recent first.
    """
    cells = [c for c in client.cells() if not c.archive]
    recent_cells: list[dict[str, Any]] = []

    now = datetime.now(UTC)
    threshold = now - timedelta(hours=hours)

    for cell in cells:
        try:
            avail = client.cell_availability(cell.id)
            latest_str = avail.get("latest_timestamp")
            if not latest_str:
                continue

            # Parse ISO timestamp
            if latest_str.endswith("Z"):
                latest_str = latest_str[:-1] + "+00:00"
            latest_dt = datetime.fromisoformat(latest_str)

            # Ensure timezone-aware comparison
            if latest_dt.tzinfo is None:
                latest_dt = latest_dt.replace(tzinfo=UTC)

            if latest_dt >= threshold:
                delta = now - latest_dt
                hours_ago = delta.total_seconds() / 3600.0
                recent_cells.append(
                    {
                        "cell_id": cell.id,
                        "cell_name": cell.name,
                        "location": cell.location,
                        "latest_timestamp": avail.get("latest_timestamp"),
                        "hours_ago": round(hours_ago, 2),
                    }
                )
        except (requests.RequestException, ValueError, KeyError):
            continue

    recent_cells.sort(key=lambda c: c["hours_ago"])
    return recent_cells


def get_cells_by_tag(client: BackendClient, tag: str | int) -> list[dict[str, Any]]:
    """List all cells associated with a given tag name or tag ID.

    Args:
        client: BackendClient instance configured with the DirtViz API.
        tag: Tag name (e.g. 'Santa Cruz') or numeric tag ID.

    Returns:
        List of cell dictionaries associated with the specified tag.
    """
    if isinstance(tag, int) or (isinstance(tag, str) and tag.isdigit()):
        cells = client.cells_from_tag_id(int(tag))
    else:
        cells = client.cells_from_tag_name(str(tag))

    return [c.to_dict() for c in cells]
