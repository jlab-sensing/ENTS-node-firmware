#!/usr/bin/env python

import os
import argparse

import pandas as pd
import matplotlib.pyplot as plt

# Ranges in mA for each state, end in non-inclusive so [start, end )
RANGES = {
    "overall": (0., 200.),
    "idle": (0., 10.),
    "measure": (10., 20.),
    "comm": (80., 200.),
}

def calculate_metrics(df: pd.DataFrame) -> dict:
    range_stats = {}

    for name, (start, end) in RANGES.items():
        print(f"Metrics for {name} range ({start} mA to {end} mA):")

        subset = df[(df["Reading"] >= start) & (df["Reading"] < end)]
        if not subset.empty:
            stats = subset["Reading"].describe()
            print(stats)
            range_stats[name] = stats
        else:
            print(f"No data found for {name} range")
        print("")

    return range_stats

def plot_trace(df: pd.DataFrame, output_path: str) -> None:

    subset = df.iloc[:100000]

    fig, ax = plt.subplots(figsize=(10, 6))

    ax.plot(subset["Relative Time"], subset["Reading"])

    ax.set_xlabel("Relative Time (s)")
    ax.set_ylabel("Current (mA)")
    ax.grid(True)

    plt.savefig(output_path, bbox_inches='tight', dpi=300)
    plt.show(block=False)


def load_data(filename: str) -> pd.DataFrame:
    df =  pd.read_csv(filename, skiprows=8)
    df["Reading"] = df["Reading"] * 1e3
    return df

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Calculate power metrics for ENTS.")
    parser.add_argument(
        "filename",
        type=str,
        nargs="+",
        help="Path to the data file.",
    )
    
    args = parser.parse_args()

    for f in args.filename:
        # find filename
        name = os.path.splitext(f)[0]

        print(f"Processing file: {f}")
        df = load_data(f)

        calculate_metrics(df)

        plot_trace(df, f"{name}.jpg")
        print("")

    input("Press Enter to close plots...")
