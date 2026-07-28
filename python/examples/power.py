#!/usr/bin/env python

from datetime import datetime, timedelta

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

from tqdm import tqdm

from ents.dirtviz.client import BackendClient


# different plots
# - Plots for each individual tag with background lines and s
# - Plots for all at
# - 

def plot_timeseries(ax, data: pd.DataFrame, key: str, color="blue"):
    """Plots data from a single tag.

    Takes a pandas Series where the index represents time and values are voltages.

    Shows the following:
    - Average voltage over time for all cells.
    - Percentile ranges in the background (25-75)
    - Individual cell lines ontop of percentile ranges
    """


    for name, group in data.groupby("name"):
        ax.plot(group.index, group[key], linewidth=0.5, alpha=0.3,
                color=color, label=name)


def plot_mean(ax, data: pd.DataFrame, key: str, color="blue", label : str=""):
    grouped = data.groupby("timestamp")[key].agg("mean")

    # see https://matplotlib.org/stable/gallery/color/named_colors.html#css-colors
    modifier = "dark"

    linecolor = f"{modifier}{color}"

    if not isinstance(data.index, pd.DatetimeIndex):
        grouped = grouped.reset_index()

    # Plot mean line
    ax.plot(grouped.index, grouped, linewidth=2, color=linecolor, label=label)


def plot_quantiles(ax, data: pd.DataFrame, key: str, color="blue"):

    import pdb; pdb.set_trace()

    grouped = data.groupby(pd.Grouper("timestamp", freq="D"))[key].agg(
        [
             ("25%", lambda x: x.quantile(0.25)),
             ("75%", lambda x: x.quantile(0.75)),
         ])

    if not isinstance(data.index, pd.DatetimeIndex):
        grouped = grouped.reset_index()

    # Plot percentile range as shaded background
    ax.fill_between(grouped.index, grouped["25%"], grouped["75%"], alpha=0.3,
                    color=color)


def plot_all(ax, series_list):
    """Plots data from all tags.

    Takes a list of pandas Series, one per tag.

    Overlays multiple individual plots without individual cell lines.
    """
    colors = ['blue', 'orange', 'green', 'red', 'purple', 'brown']

    for idx, series in enumerate(series_list):
        # For plot_all, we just plot the mean (no percentiles shown per spec)
        color = colors[idx % len(colors)]
        ax.plot(series.values, linewidth=2, color=color)


def plot_extra(ax):
    # add horizontal line at y=0
    ax.axhline(
        y=0,
        color="black",
        linewidth=2.5,
        dashes=(2,2),
    )

    # set y limits
    ax.set_ylim(-1200, 1200)

    # add grid
    ax.grid(True)

    # legend
    ax.legend()



def get_data() -> pd.DataFrame:
    # data labels
    tags = {
        "Santa Cruz": {
            # full range
            #"start": datetime(2025, 1, 3),
            #"end": datetime(2026, 12, 1),

            # good range
            "start": datetime(2025, 2, 12),
            "end": datetime(2025, 7, 25),
        },
        #"MOTE NW": {
        #    "start": datetime(2025, 1, 3),
        #    "end": datetime(2026, 12, 1),
        #},
        "MOTE GT": {
            "start": datetime(2025, 1, 3),
            "end": datetime(2026, 12, 1),
        },
        "MOTE UCSD": {
            "start": datetime(2025, 1, 3),
            "end": datetime(2026, 12, 1),
        },
    }


    # Create backend instance
    api = BackendClient()

    tag_data = []

    for name, info in tags.items():
        print(f"Getting data for tag: {name}")

        cells = api.cells_from_tag_name(name)

        cell_data = []

        # plot the data
        for cell in tqdm(cells):
            if cells == []:
                print(f"No cells found for tag '{name}'.")
                continue

            sensor_data = []

            # get powerd data
            power_data = api.power_data(
                cell,
                info["start"],
                info["end"],
            )
            if len(power_data) > 0:
                power_data = power_data.rename(
                    columns = {
                        "v": "Voltage",
                    },
                )
                sensor_data.append(power_data)
            else:
                voltage_data = api.sensor_data_simple(
                    cell,
                    "POWER_VOLTAGE",
                    "Voltage",
                    info["start"],
                    info["end"]
                )
                if len(voltage_data) > 0:
                    voltage_data["Voltage"] *= 1000
                    sensor_data.append(voltage_data)

                current_data = api.sensor_data_simple(
                    cell,
                    "POWER_CURRENT",
                    "CURRENT",
                    info["start"],
                    info["end"]
                )
                if len(current_data) > 0:
                    sensor_data.append(current_data)


            teros_data = api.teros_data(
                cell,
                info["start"],
                info["end"]
            )
            if len(teros_data) > 0:
                sensor_data.append(teros_data)
            else:
                vwc_data = api.sensor_data_simple(
                    cell,
                    "TEROS12_VWC",
                    "Volumetric Water Content",
                    info["start"],
                    info["end"],
                )
                if len(vwc_data) > 0:
                    sensor_data.append(vwc_data)

                temp_data = api.sensor_data_simple(
                    cell,
                    "TEROS12_TEMP",
                    "Temperature",
                    info["start"],
                    info["end"],
                )
                if len(temp_data) > 0:
                    sensor_data.append(temp_data)

                ec_data = api.sensor_data_simple(
                    cell,
                    "TEROS12_EC",
                    "Electrical Conductivity",
                    info["start"],
                    info["end"],
                )
                if len(ec_data) > 0:
                    sensor_data.append(ec_data)


            # check for data
            if (len(sensor_data) == 0):
                print(f"No data for {cell.name}")
                continue

            # presort and merge
            if (len(sensor_data) > 0):
                sensor_data = [s_d.sort_values("timestamp") for s_d in sensor_data]
                sensor_data_df = sensor_data[0]
                if (len(sensor_data) > 1):
                    for s_d in sensor_data[1:]:
                        sensor_data_df = pd.merge_asof(sensor_data_df, s_d,
                                                       on="timestamp",
                                                       direction="nearest")
            #sensor_data_df = pd.concat(sensor_data)

            # transform data
            sensor_data_df["name"] = cell.name
            sensor_data_df.set_index("timestamp", inplace=True)

            # store data
            cell_data.append(sensor_data_df)


            #axs[0].plot(
            #    data["v"],
            #    linewidth=3,
            #)

        print("")

        cell_data_df = pd.concat(cell_data)
        tag_data.append(cell_data_df)

    return tag_data


def plot_individual(data: list[pd.DataFrame], key="v"):
    """Plot individual tags"""

    num_plots = len(data)
    colors = ["blue", "orange", "green"]

    assert (num_plots <= len(colors))

    # double column
    fig, axs = plt.subplots(num_plots, 1,
                            sharey=True,
                            figsize=(7, 10),
                            )

    for d, ax, c in zip(data, axs, colors[:num_plots]):
        plot_timeseries(ax, d, key, c)
        plot_mean(ax, d, key, c, "Mean")
        plot_quantiles(ax, d, key, c)
        plot_extra(ax)




def plot_together(data: list[pd.DataFrame], groups: list[str], key="v"):
    """Plot individual tags"""

    num_plots = len(data)
    colors = ["blue", "orange", "green"]

    assert (num_plots <= len(colors))

    # double column
    fig, ax = plt.subplots(1, 1,
                            sharey=True,
                            figsize=(7, 10),
                            )

    for d, c, g in zip(data, colors[:num_plots], groups):
        d_idx = d.reset_index()

        #plot_timeseries(ax, d_idx, key, c)
        plot_mean(ax, d_idx, key, c, g)
        plot_quantiles(ax, d_idx, key, c)

    plot_extra(ax)



def filter_data(data: pd.DataFrame, expr: list[str]) -> list[pd.DataFrame]:
    """Filter data by cell names.

    See string matching documentation on pandas.
    https://pandas.pydata.org/docs/reference/api/pandas.Series.str.contains.html

    Args:
        data: Dataframe to filter.
        expr: List of expression to filter by.

    Returns:
        Dataframe per string expression.
    """

    filtered_data = []

    for e in expr:
        filtered_data.append(data[data["name"].str.contains(rf"^{e}_\d+$")])

    return filtered_data

if __name__ == "__main__":

    # matplotlib formatting
    plt.rcParams["font.size"] = 7
    plt.rcParams['font.weight'] = 'medium'


    # Open circuit data
    data = get_data()

    #import pdb; pdb.set_trace()

    #plot_individual(data, "Voltage")
    #plt.savefig("individual_voltage.pdf", dpi=300)
    #plot_together(data, "Voltage")
    #plt.savefig("together_voltage.pdf", dpi=300)


    gt_black_names = ["gt_black_none", "gt_black_inorganics", "gt_black_inorganics_compost"]
    gt_black = filter_data(data[1], gt_black_names)
    plot_individual(gt_black, "Voltage")
    plt.savefig("gt_black_individual.jpg", dpi=300)
    #plot_together(gt_black, gt_black_names, "Voltage")
    #plt.savefig("gt_black_together.jpg", dpi=300)


    #
    # Georgia Tech
    #

    # Available cells for reference
    # ['gt_black_inorganics_1', 'gt_black_inorganics_2',
    # 'gt_black_inorganics_3', 'gt_black_inorganics_compost_1',
    # 'gt_black_inorganics_compost_2', 'gt_black_inorganics_compost_3',
    # 'gt_black_none_1', 'gt_black_none_2', 'gt_black_none_3',
    # 'gt_red_compost_1', 'gt_red_compost_2', 'gt_red_compost_3',
    # 'gt_red_compost_inorganics_1', 'gt_red_compost_inorganics_2',
    # 'gt_red_compost_inorganics_3', 'gt_red_none_1', 'gt_red_none_2',
    # 'gt_red_none_3', 'gt_synth_auto_1', 'gt_synth_auto_2', 'gt_synth_auto_3',
    # 'gt_synth_none_1', 'gt_synth_none_2', 'gt_synth_none_3', 'gt_test']

    # black_soil plots
    #gt_black = filter_data(data[1], ["gt_black_none", "gt_black_inorganics",
    #                           "gt_black_inorganics_compost"])
    #plot_individual(gt_black, "Voltage")
    #plt.savefig("gt_black_individual.jpg", dpi=300)
    #plot_together(gt_black, "Voltage")
    #plt.savefig("gt_black_together.jpg", dpi=300)

    ## red soil plots
    #gt_red = filter_data(data[1], ["gt_red_none", "gt_red_compost",
    #                               "gt_red_compost_inorganics"])
    #plot_individual(gt_red, "Voltage")
    #plt.savefig("gt_red_individual.jpg", dpi=300)
    #plot_together(gt_red, "Voltage")
    #plt.savefig("gt_red_together.jpg", dpi=300)

    ## Black soil to red texture
    #gt_black_to_red = filter_data(data[1], [ "gt_red_none", "gt_black_inorganics"])
    #plot_individual(gt_black_to_red, "Voltage")
    #plt.savefig("gt_black_to_red_individual.jpg", dpi=300)
    #plot_together(gt_black_to_red, "Voltage")
    #plt.savefig("gt_black_to_red_together.jpg", dpi=300)

    ## Red soil to the OM of black
    #gt_red_to_black = filter_data(data[1], [ "gt_black_none", "gt_red_compost"])
    #plot_individual(gt_red_to_black, "Voltage")
    #plt.savefig("gt_red_to_black_individual.jpg", dpi=300)
    #plot_together(gt_red_to_black, "Voltage")
    #plt.savefig("gt_red_to_black_together.jpg", dpi=300)

    ## Red and black soil same OM and texture
    #gt_amend_same = filter_data(data[1], [ "gt_black_inorganics_compost",
    #                                      "gt_red_compost_inorganics"])
    #plot_individual(gt_amend_same, "Voltage")
    #plt.savefig("gt_amend_same_individual.jpg", dpi=300)
    #plot_together(gt_amend_same, "Voltage")
    #plt.savefig("gt_amend_same_together.jpg", dpi=300)


    #plot_individual(data[1:], "vwc")
    #plot_together(data[1:], "vwc")


    #plot_individual(data[1:], "Temperature")
    #plot_together(data[1:], "Temperature")


    #plot_individual(data[1:], "Electrical Conductivity")
    #plot_together(data[1:], "Electrical Conductivity")


    # Closed circuit
    # not until have closed circuit
    #plot_individual(data, "i")
    #plot_individual(data, "p")


    plt.tight_layout()

    #plt.savefig("deployment_field.pdf", dpi=300)
    plt.show()

