"""Client interface with dirtviz."""

import hashlib
import json
from datetime import datetime
from pathlib import Path

import pandas as pd
import requests


class Cell:
    """Class representing a cell in the Dirtviz API."""

    def __init__(self, data: dict):
        """Initialize the Cell object from a cell dictionary.

        Args:
            data: json data dictionary from the Dirtviz API containing cell information.
        """

        self.id = data["id"]
        self.name = data["name"]
        self.location = data.get("location")
        self.latitude = data.get("latitude")
        self.longitude = data.get("longitude")
        self.archive = data.get("archive", False)

    def to_dict(self) -> dict:
        """Convert Cell to dictionary representation."""
        return {
            "id": self.id,
            "name": self.name,
            "location": self.location,
            "latitude": self.latitude,
            "longitude": self.longitude,
            "archive": self.archive,
        }

    def __repr__(self):
        return f"Cell(id={self.id}, name={self.name})"


class BackendClient:
    """Client for interacting with the Dirtviz API."""

    def __init__(
        self,
        base_url: str = "https://dirtviz.jlab.ucsc.edu/api/",
        cache_dir: str | Path | None = None,
    ):
        """Initialize the BackendClient.

        Sets the base URL for the API. Defaults to the Dirtviz API.

        Args:
            base_url: Base URL for the API.
            cache_dir: Directory to store cached responses. Defaults to ~/.cache/dirtviz/.
        """

        self.base_url = base_url

        if cache_dir is None:
            cache_dir = Path.home() / ".cache" / "dirtviz"
        else:
            cache_dir = Path(cache_dir)

        self.cache_dir = cache_dir
        self.cache_dir.mkdir(parents=True, exist_ok=True)

    def get(self, endpoint: str, params: dict | None = None):
        """Get request to the API with disk-based caching.

        Args:
            endpoint: The API endpoint to request.
            params: Optional parameters for the request.

        Returns:
            The parsed response data (dict or list).
        """

        cache_path = self._get_cache_path(endpoint, params)

        # Check if response is cached
        if cache_path.exists():
            with open(cache_path, "r") as f:
                return json.load(f)

        url = f"{self.base_url}{endpoint.lstrip('/')}"
        response = requests.get(url, params=params)
        response.raise_for_status()

        result = response.json()

        # Save to cache
        with open(cache_path, "w") as f:
            json.dump(result, f)

        return result

    def _get_cache_path(self, endpoint: str, params: dict | None = None) -> Path:
        """Generate a cache file path from endpoint and parameters.

        Args:
            endpoint: The API endpoint.
            params: Optional parameters dictionary.

        Returns:
            A Path object for the cache file.
        """

        # Create a hash of the endpoint and params
        if params is None:
            params_str = ""
        else:
            params_str = json.dumps(params, sort_keys=True)

        key = f"{endpoint}:{params_str}"
        cache_hash = hashlib.md5(key.encode()).hexdigest()

        return self.cache_dir / f"{cache_hash}.json"

    def clear_cache(self) -> None:
        """Clear all cached data."""
        import shutil

        if self.cache_dir.exists():
            shutil.rmtree(self.cache_dir)
            self.cache_dir.mkdir(parents=True, exist_ok=True)

    @staticmethod
    def time_to_params(start: datetime, end: datetime) -> dict:
        """Puts start and end datetime into an API paramter dictionary

        Args:
            dt: The datetime object to format.

        Returns:
            A string representing the formatted datetime.
        """

        timestamp_format = "%a, %d %b %Y %H:%M:%S GMT"

        start_str = start.strftime(timestamp_format)
        end_str = end.strftime(timestamp_format)

        params = {
            "startTime": start_str,
            "endTime": end_str,
        }

        return params

    def power_data(
        self,
        cell: Cell | str | int,
        start: datetime,
        end: datetime,
        resample: str = "none",
    ) -> pd.DataFrame:
        """Gets power data for a specific cell by name, id, or Cell object.

        Args:
            cell: The Cell object, cell name, or cell ID for which to get power data.
            start: The start date of the data.
            end: The end date of the data.
            resample: Optional resampling interval ('none', 'hour', 'day', etc.).

        Returns:
            A pandas DataFrame containing the power data.
        """

        cell_obj = self.resolve_cell(cell)
        endpoint = f"/power/{cell_obj.id}"

        params = self.time_to_params(start, end)
        if resample != "none":
            params["resample"] = resample

        data = self.get(endpoint, params=params)

        if not data or (isinstance(data, dict) and not any(data.values())):
            return pd.DataFrame(columns=["timestamp", "v", "i", "p"])

        data_df = pd.DataFrame(data)
        if "timestamp" in data_df.columns:
            data_df["timestamp"] = pd.to_datetime(data_df["timestamp"])

        return data_df

    def teros_data(
        self,
        cell: Cell | str | int,
        start: datetime,
        end: datetime,
        resample: str = "none",
    ) -> pd.DataFrame:
        """Gets teros data for a specific cell by name, id, or Cell object.

        Args:
            cell: The Cell object, cell name, or cell ID for which to get teros data.
            start: The start date of the data.
            end: The end date of the data.
            resample: Optional resampling interval ('none', 'hour', 'day', etc.).

        Returns:
            A pandas DataFrame containing the teros data with columns vwc_raw,
            vwc_adj, temp, ec.
        """

        cell_obj = self.resolve_cell(cell)
        endpoint = f"/teros/{cell_obj.id}"

        params = self.time_to_params(start, end)
        if resample != "none":
            params["resample"] = resample

        data = self.get(endpoint, params=params)

        if not data or (isinstance(data, dict) and not any(data.values())):
            return pd.DataFrame(columns=["timestamp", "vwc", "temp", "ec", "raw_vwc"])

        data_df = pd.DataFrame(data)
        if "timestamp" in data_df.columns:
            data_df["timestamp"] = pd.to_datetime(data_df["timestamp"])

        return data_df

    def sensor_data(
        self,
        cell: Cell | str | int,
        name: str,
        meas: str,
        start: datetime,
        end: datetime,
        resample: str = "none",
    ) -> pd.DataFrame:
        """Gets generic sensor data for a specific cell.

        Returns a dataframe with the following columns:
        data, measurement, timestamp, type, unit

        Args:
            cell: The Cell object, cell name, or cell ID for which to get sensor data.
            name: Name of the sensor (e.g., "power", "teros", "bme280").
            meas: The measurement type (e.g., "v", "i", "vwc", "temp", "temperature").
            start: The start date of the data.
            end: The end date of the data.
            resample: Optional resampling interval ('none', 'hour', 'day', etc.).

        Returns:
            A pandas DataFrame containing the sensor data.
        """

        cell_obj = self.resolve_cell(cell)
        endpoint = "/sensor/"

        params = {
            "cellId": cell_obj.id,
            "name": name,
            "measurement": meas,
        }

        if resample != "none":
            params["resample"] = resample

        params = params | self.time_to_params(start, end)

        data = self.get(endpoint, params=params)

        if not data or (isinstance(data, dict) and not any(data.values())):
            return pd.DataFrame(
                columns=["timestamp", "data", "measurement", "type", "unit"]
            )

        data_df = pd.DataFrame(data)
        if "timestamp" in data_df.columns:
            data_df["timestamp"] = pd.to_datetime(data_df["timestamp"])

        return data_df

    def sensor_data_simple(
        self,
        cell: Cell | str | int,
        name: str,
        meas: str,
        start: datetime,
        end: datetime,
        resample: str = "none",
    ) -> pd.DataFrame:
        """Gets simplified version of sensor data.

        See self.sensor_data. Returns a DataFrame with columns:
        timestamp, {meas}

        Args:
            cell: Cell instance, name, or ID.
            name: Sensor name (e.g. "bme280", "power").
            meas: Measurement name (used as the renamed data column).
            start: Start datetime.
            end: End datetime.
            resample: Optional resampling interval ('none', 'hour', 'day').

        Returns:
            A pandas DataFrame with columns 'timestamp' and '{meas}'.
        """

        raw = self.sensor_data(
            cell=cell,
            name=name,
            meas=meas,
            start=start,
            end=end,
            resample=resample,
        )

        if raw.empty or "data" not in raw.columns:
            return pd.DataFrame(columns=["timestamp", meas])

        simplified = raw[["timestamp", "data"]].copy()
        simplified = simplified.rename(columns={"data": meas})

        return simplified

    def cell_from_id(self, cell_id: int) -> Cell | None:
        """Get a Cell object from its ID.

        Args:
            cell_id: The ID of the cell.

        Returns:
            A Cell object. None if the cell does not exist.
        """

        cell_list = self.cells()

        for cell in cell_list:
            if cell.id == cell_id:
                return cell

        return None

    def cell_from_name(self, name: str) -> Cell | None:
        """Get a Cell object from its name.

        Args:
            name: The name of the cell.

        Returns:
            A Cell object. None if the cell does not exist.
        """

        cell_list = self.cells()

        for cell in cell_list:
            if cell.name == name:
                return cell

        return None

    def cells_from_tag_id(self, tag: int) -> list[Cell]:
        """Gets a list of Cell objects that have a specific tag.

        Args:
            tag: The tag to search for.

        Returns:
            A list of Cell objects that have the specified tag.
        """

        cell_list = []

        endpoint = f"/tags/{tag}/cells"
        cell_data_list = self.get(endpoint)

        for c in cell_data_list["cells"]:
            cell = Cell(c)
            cell_list.append(cell)

        return cell_list

    def cells_from_tag_name(self, tag: str) -> list[Cell]:
        """Gets a list of Cell objects that have a specific tag.

        Args:
            tag: The tag to search for.

        Returns:
            A list of Cell objects that have the specified tag.
        """

        # Get all tags
        endpoint = "/tag/"
        cell_data_list = self.get(endpoint)

        # find tag id from name
        tag_id = None
        for t in cell_data_list:
            if t["name"] == tag:
                tag_id = t["id"]
                break

        # check that tag exists
        if tag_id is None:
            return []

        # return cells from tag id
        return self.cells_from_tag_id(tag_id)

    def cells(self) -> list[Cell]:
        """Gets a list of all cells from the API.

        Returns:
            A list of Cell objects.
        """

        cell_list = []

        endpoint = "/cell/id"
        cell_data_list = self.get(endpoint)

        for c in cell_data_list:
            cell = Cell(c)
            cell_list.append(cell)

        return cell_list

    def resolve_cell(self, identifier: Cell | str | int) -> Cell:
        """Resolve a cell from a Cell object, numeric ID, or cell name.

        Matching is case-insensitive if an exact name match is not found.

        Args:
            identifier: Cell instance, integer ID, numeric string ID, or cell name.

        Returns:
            The resolved Cell instance.

        Raises:
            ValueError: If the cell cannot be found.
        """

        if isinstance(identifier, Cell):
            return identifier

        if isinstance(identifier, int) or (
            isinstance(identifier, str) and identifier.isdigit()
        ):
            cell = self.cell_from_id(int(identifier))
            if cell is not None:
                return cell
        else:
            cell = self.cell_from_name(str(identifier))
            if cell is not None:
                return cell

            # Case-insensitive search fallback
            cells = self.cells()
            ident_lower = str(identifier).lower()
            for c in cells:
                if c.name.lower() == ident_lower:
                    return c

        all_names = [c.name for c in self.cells()]
        raise ValueError(f"Cell '{identifier}' not found. Available cells: {all_names}")

    def cell_sensors(self, cell: Cell | int | str) -> list[dict]:
        """Get all sensors associated with a specific cell.

        Queries the cell's sensor list endpoint (/cell/<id>/sensors). If empty or
        unavailable, falls back to the sensor catalog endpoint.

        Args:
            cell: Cell instance, integer ID, or cell name.

        Returns:
            A list of dictionaries describing each sensor.
        """

        cell_obj = self.resolve_cell(cell)
        endpoint = f"/cell/{cell_obj.id}/sensors"

        try:
            sensors = self.get(endpoint)
            if isinstance(sensors, list) and len(sensors) > 0:
                return sensors
        except (requests.RequestException, KeyError):
            pass

        try:
            catalog = self.sensor_catalog(cell_obj)
            return catalog.get("entries", [])
        except (requests.RequestException, KeyError):
            return []

    def sensor_catalog(self, cell: Cell | int | str) -> dict:
        """Get the chartable sensor catalog for a cell.

        Args:
            cell: Cell instance, integer ID, or cell name.

        Returns:
            A dictionary containing the cell ID and catalog entries.
        """

        cell_obj = self.resolve_cell(cell)
        endpoint = f"/catalog/sensors?cell_id={cell_obj.id}"
        return self.get(endpoint)

    def cell_availability(
        self, cell_ids: list[int | str | Cell] | int | str | Cell
    ) -> dict:
        """Get data availability (earliest, latest timestamp) for one or more cells.

        Args:
            cell_ids: Single cell or list of cells (Cell instances, IDs, or names).

        Returns:
            Dictionary with earliest_timestamp, latest_timestamp, and has_recent_data.
        """

        if isinstance(cell_ids, (Cell, str)):
            resolved = self.resolve_cell(cell_ids)
            ids_param = str(resolved.id)
        elif isinstance(cell_ids, int):
            ids_param = str(cell_ids)
        elif isinstance(cell_ids, (list, tuple)):
            resolved_ids = []
            for item in cell_ids:
                if isinstance(item, Cell):
                    resolved_ids.append(item.id)
                elif isinstance(item, (int, str)):
                    resolved_ids.append(self.resolve_cell(item).id)
            ids_param = ",".join(str(i) for i in resolved_ids)
        else:
            ids_param = str(cell_ids)

        endpoint = "/data-availability/"
        return self.get(endpoint, params={"cell_ids": ids_param})
