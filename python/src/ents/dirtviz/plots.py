"""Common plots for data on dirtviz.

Still need to fill in with common functions:
- Plot power (voltage, current, calcualted power)
- Plot teros12
- Plot bme280
- Plot all (take all available sensors)
- Plot group (find mean/stddev for each point)

Can pull from personal scripts from SenSys (jtmadden)
"""

import pandas as pd

import matplotlib.pyplot as plt


# matplotlib formatting
# plt.rcParams["font.size"] = 7
# plt.rcParams['font.weight'] = 'medium'
# plt.ion()


def plot_data(data: list[pd.DataFrame], name, **kwargs):
    """Plots data from one or many cells

    Args:
        data: List of dataframes
        name: Column or measurement name
    """

    fig, ax = plt.subplots()
    for d in data:
        ax.plot(d["timestamp"], d[name], **kwargs)

    ax.axhline(
        y=0,
        color="black",
        linewidth=2,
        dashes=(2, 2),
    )

    ax.set_xlabel("Timestamp")
    ax.set_ylabel(name)

    ax.grid()

    plt.show(block=False)

    return ax
