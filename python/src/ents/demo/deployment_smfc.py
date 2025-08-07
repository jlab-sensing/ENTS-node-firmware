#!/usr/bin/env python

from datetime import datetime, timedelta
from dataclasses import dataclass

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

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

    DEFAULT_BASE_URL = "https://dirtviz.jlab.ucsc.edu/api/"

    def __init__(self, base_url: str = DEFAULT_BASE_URL):
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

    def power_data(self, cell: Cell, start: datetime, end: datetime) -> pd.DataFrame:
        """Gets power data for a specific cell by name.

        Args:
            cell: The Cell object for which to get power data.
            start: The start date of the data.
            end: The end date of the data.

        Returns:
            A pandas DataFrame containing the power data.
        """

        timestamp_format = "%a, %d %b %Y %H:%M:%S GMT"

        start_str = start.strftime(timestamp_format)
        end_str = end.strftime(timestamp_format)

        endpoint = f"/power/{cell.id}"
        params = {
            "startTime": start_str,
            "endTime": end_str,
        }

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


def plot_cell_data(
    ax: plt.Axes, name: str, label: str, start: datetime, end: datetime, **kwargs
) -> None:
    """Plots the power data for a cell on a given axes.

    Args:
        ax: The matplotlib Axes object to plot on.
        name: The name of the cell.
        label: Column to plot.
        start: The start date of the data.
        end: The end date of the data.
        kwargs: Additional keyword arguments for matplotlib plot
    """

    api = BackendClient()

    cell = api.cell_from_name(name)

    data = api.power_data(cell, start=start, end=end)

    ax.plot(data["timestamp"], data[label], linewidth=3, label=name, **kwargs)


@dataclass
class CellGroupPlot:
    names: list[str]
    start: datetime
    end: datetime
    water_dates: list[datetime] = None


def plot_cell_voltage_group(
    ax: plt.Axes, group: CellGroupPlot, ticks: bool = False
) -> None:
    """Plots the power data for a group of cells on a given axes.

    Args:
        ax: The matplotlib Axes object to plot on.
        cell_names: A list of cell names to plot.
        start: The start date of the data.
        end: The end date of the data.
    """

    for name in group.names:
        plot_cell_data(ax, name, "v", group.start, group.end)
    ax.grid(True)
    ax.legend()
    ax.set_xlim(group.start, group.end)
    ax.set_ylim(-200, 900)
    ax.set_ylabel("Voltage (mV)")

def plot_cell_power_group(
    ax: plt.Axes, group: CellGroupPlot, ticks: bool = False
) -> None:
    """Plots the power data for a group of cells on a given axes.

    Args:
        ax: The matplotlib Axes object to plot on.
        cell_names: A list of cell names to plot.
        start: The start date of the data.
        end: The end date of the data.
    """

    for name in group.names:
        plot_cell_data(ax, name, "p", group.start, group.end)
    ax.grid(True)
    ax.legend()
    ax.set_xlim(group.start, group.end)
    ax.set_ylim(-50, 100)
    ax.set_ylabel("Power (uW)")

class Plots:
    """Class to hold configuration for plotting the deployment data."""

    open_circuit_startup = CellGroupPlot(
        names=[
            "sc_local_open_1",
            "sc_local_open_3",
            # did not reach the expected voltage
            # "sc_synthA_open_1",
        ],
        start=datetime(2025, 2, 16),
        end=datetime(2025, 4, 1),
    )

    # lack of watering
    watering_cycle = CellGroupPlot(
        names=[
            "sc_local_open_2",
            "sc_synth_open_1",
            "sc_synth_open_3",
            "sc_synthA_open_2",
        ],
        start=datetime(2025, 2, 13),
        end=datetime(2025, 6, 28),
    )

    random_behavior_open = CellGroupPlot(
        names=[
            "sc_synth_open_2",
            "sc_synthA_open_3",
        ],
        start=datetime(2025, 2, 13),
        end=datetime(2025, 6, 28),
    )

    logging_issues = CellGroupPlot(
        names=[
            "sc_local_closed_1",
            "sc_synth_closed_1",
            # Likely something up with current measurements
            # "sc_synthA_closed_1",
        ],
        start=datetime(2025, 2, 13),
        end=datetime(2025, 6, 28),
    )

    poisening_recovery = CellGroupPlot(
        names=[
            "sc_local_closed_2",
            "sc_local_closed_3",
            "sc_synth_closed_3",
            "sc_synthA_closed_2",
            "sc_synthA_closed_3",
        ],
        start=datetime(2025, 6, 24),
        end=datetime(2025, 6, 28),
    )

    random_behavior_closed = CellGroupPlot(
        names=[
            "sc_synth_closed_2",
        ],
        start=datetime(2025, 2, 13),
        end=datetime(2025, 6, 28),
    )

def plot_all() -> None:
    """Plot the deployment data.
    """

    fig, ax = plt.subplots(2, 1, layout="constrained", figsize=(16, 8))

    plot_cell_voltage_group(
        ax[0],
        Plots.open_circuit_startup,
    )

    plot_cell_voltage_group(
        ax[1],
        Plots.watering_cycle,
    )

    # render figure
    plt.savefig("deployment_smfc.pdf", format="pdf", bbox_inches="tight")
    plt.show(block=False)

def plot_startup():
    fig, ax = plt.subplots(layout="constrained", figsize=(16, 3))

    plot_cell_voltage_group(
        ax,
        Plots.open_circuit_startup,
    )

    plt.savefig("deployment_smfc_startup.pdf", format="pdf", bbox_inches="tight")
    plt.show(block=False)

def plot_watering():
    fig, ax = plt.subplots(layout="constrained", figsize=(16, 3))

    plot_cell_voltage_group(
        ax,
        Plots.watering_cycle,
    )

    plot_wet_dry(ax, datetime(2025,4,8), Plots.watering_cycle.start, Plots.watering_cycle.end)
    ax.legend()

    # watering

    plt.savefig("deployment_smfc_watering.pdf", format="pdf", bbox_inches="tight")
    plt.show(block=False)

def plot_wet_dry(ax: plt.Axes, date: datetime, start: datetime, end: datetime) -> None:
    """Plots the wet/dry cycle on the given Axes.

    Args:
        ax: The matplotlib Axes object to plot on.
        date: The datetime object representing the date of the wet/dry cycle.
    """

    ax.axvspan(start, date, color="lightgreen", alpha=0.5,
               label="Weting Phase")

    ax.axvspan(date, end, color="lightcoral", alpha=0.5,
               label="Drying Phase")

def plot_watering_events(ax: plt.Axes, dates: list[datetime]) -> None:
    """Plots the watering event on the given Axes.

    Args:
        ax: The matplotlib Axes object to plot on.
        dates: A list of datetime objects representing the watering event
        timestamps.
    """

    width = timedelta(days=1)

    for date in dates:
        ax.axvspan(date, date + width, color="lightblue", alpha=0.5,
                   label="Watering Event")


if __name__ == "__main__":
    # set default font size
    plt.rcParams.update({"font.size": 18})

    #plot_all()
    plot_startup()
    plot_watering()
    input("Press Enter to exit...")

