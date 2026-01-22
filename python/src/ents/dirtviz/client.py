"""Client interface with dirtviz.

TODO:
- Add caching of data
"""

from datetime import datetime

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
        return f"Cell(id={self.cell_id}, name={self.name})"


class BackendClient:
    """Client for interacting with the Dirtviz API."""

    def __init__(self, base_url: str = "https://dirtviz.jlab.ucsc.edu/api/"):
        """Initialize the BackendClient.

        Sets the base URL for the API. Defaults to the Dirtviz API.
        """

        self.base_url = base_url

    def get(self, endpoint: str, params: dict = None) -> dict:
        """Get request to the API.

        Args:
            endpoint: The API endpoint to request.
            params: Optional parameters for the request.

        Returns:
            A dictionary containing the response data.
        """

        url = f"{self.base_url}{endpoint}"
        response = requests.get(url, params=params)
        response.raise_for_status()

        return response.json()

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
        """Gets generic sensor data for a specific cell

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
