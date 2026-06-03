"""Client interface with dirtviz.
"""

from datetime import datetime
import json
import hashlib
from pathlib import Path

import pandas as pd

import requests


class Cell:
    """Class representing a cell in the Dirtviz API."""

    def __init__(self, data: str):
        """Initialize the Cell object from a cell ID.

        Args:
            data: json data from the Dirtviz API containing cell information.
        """

        self.id = data["id"]
        self.name = data["name"]
        self.location = data["location"]
        self.latitude = data["latitude"]
        self.longitude = data["longitude"]

    def __repr__(self):
        return f"Cell(id={self.id}, name={self.name})"


class BackendClient:
    """Client for interacting with the Dirtviz API."""

    def __init__(self, base_url: str = "https://dirtviz.jlab.ucsc.edu/api/", cache_dir: str = None):
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

    def get(self, endpoint: str, params: dict = None) -> dict:
        """Get request to the API with disk-based caching.

        Args:
            endpoint: The API endpoint to request.
            params: Optional parameters for the request.

        Returns:
            A dictionary containing the response data.
        """

        cache_path = self._get_cache_path(endpoint, params)

        # Check if response is cached
        if cache_path.exists():
            with open(cache_path, 'r') as f:
                return json.load(f)

        url = f"{self.base_url}{endpoint}"
        response = requests.get(url, params=params)
        response.raise_for_status()

        result = response.json()

        # Save to cache
        with open(cache_path, 'w') as f:
            json.dump(result, f)

        return result

    def _get_cache_path(self, endpoint: str, params: dict = None) -> Path:
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

    def power_data(self, cell: Cell, start: datetime, end: datetime) -> pd.DataFrame:
        """Gets power data for a specific cell by name.

        Args:
            cell: The Cell object for which to get power data.
            start: The start date of the data.
            end: The end date of the data.

        Returns:
            A pandas DataFrame containing the power data.
        """

        endpoint = f"/power/{cell.id}"

        params = self.time_to_params(start, end)

        data = self.get(endpoint, params=params)

        data_df = pd.DataFrame(data)
        data_df["timestamp"] = pd.to_datetime(data_df["timestamp"])

        return data_df

    def teros_data(self, cell: Cell, start: datetime, end: datetime) -> pd.DataFrame:
        """Gets teros data for a specific cell

        Args:
            cell: The Cell object for which to get teros data.
            start: The start date of the data.
            end: The end date of the data.

        Returns:
            A pandas DataFrame containing the teros data with columns vwc_raw,
            vwc_adj, temp, ec.
        """

        endpoint = f"/teros/{cell.id}"

        params = self.time_to_params(start, end)

        data = self.get(endpoint, params=params)

        data_df = pd.DataFrame(data)
        data_df["timestamp"] = pd.to_datetime(data_df["timestamp"])

        return data_df

    def sensor_data(
        self,
        cell: Cell,
        name: str,
        meas: str,
        start: datetime,
        end: datetime,
        resample: str = "none",
    ) -> pd.DataFrame:
        """Gets generic sensor data for a specific cell.

        Returns a dataframe with the following column:
        data, measurement, timestamp, type, unit

        Args:
            cell: The Cell object for which to get sensor data.
            name: Name of the sensor (e.g., "power", "teros").
            meas: The measurement type (e.g., "v", "i", "vwc", "temp", "ec").
            start: The start date of the data.
            end: The end date of the data.

        Returns:
            A pandas DataFrame containing the sensor data.
        """

        endpoint = "/sensor/"

        params = {
            "cellId": cell.id,
            "name": name,
            "measurement": meas,
        }

        params = params | self.time_to_params(start, end)

        data = self.get(endpoint, params=params)

        data_df = pd.DataFrame(data)
        data_df["timestamp"] = pd.to_datetime(data_df["timestamp"])

        return data_df

    def sensor_data_simple(
        self,
        *args,
        **kwargs
    ) -> pd.DataFrame:
        """Gets simplified version of sensor data.

        See self.sensor_data

        Has the following columns:
        timestamp, {meas}

        Returns:
            A pandas DataFrame containing the sensor data.
        """

        raw = self.sensor_data(*args, **kwargs)

        simplified = raw[["timestamp", "data"]]
        simplified = simplified.rename(columns={"data": args[2]})

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

        cell_list = []

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
