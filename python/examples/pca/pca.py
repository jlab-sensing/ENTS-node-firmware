#!/usr/bin/env python

import os
import argparse
from datetime import datetime
import logging
from pathlib import Path
from dataclasses import dataclass

import statsmodels.api as sm
from statsmodels.formula.api import ols, mixedlm
from statsmodels.stats.outliers_influence import variance_inflation_factor
import pandas as pd
import seaborn as sns
import numpy as np
from sklearn.preprocessing import StandardScaler
from sklearn.decomposition import PCA
import matplotlib.pyplot as plt
from yaml import safe_load

from ents.dirtviz import BackendClient, plot_data


logger = logging.getLogger(__name__)

META_COLS = ["name", "type", "timestamp"]

def load_yaml(filename: str) -> dict:
    path = Path(filename)
    with path.open("r", encoding="utf-8") as f:
        data = safe_load(f)

    return data


def fill_dates(cell_dict: dict) -> dict:
    """Fills in date ranges for each of the cell ranges.

    Makes the dates for the different "states" of the cells fully formed with
    start and dates.

    Args:
        cell_dict: Cell dictionary

    Return
        Reference to cell_dict
    """

    start = cell_dict["start"]
    end = cell_dict["end"]


    for attr in cell_dict["cells"].values():
        # incubation
        if "start" not in attr["incubation"]:
            attr["incubation"]["start"] = start
        if "end" not in attr["incubation"]:
            attr["incubation"]["end"] = attr["open"]["end"]

        if "start" not in attr["open"]:
            attr["open"]["start"] = attr["incubation"]["end"]
        if "end" not in attr["open"]:
            attr["open"]["end"] = attr["closed"]["end"]

        if "start" not in attr["closed"]:
            attr["closed"]["start"] = attr["open"]["end"]
        if "end" not in attr["closed"]:
            attr["closed"]["end"] = end

    return cell_dict



def get_data() -> dict:
    soil_types = load_yaml("soil_types.yaml")
    cells = load_yaml("test_cells.yaml")
    cells = fill_dates(cells)

    start_date = cells["start"]
    end_date = cells["end"]
    cell_map = cells["cells"]

    logger.info(f"Using data from {start_date} to {end_date}.")


    api = BackendClient()

    all_dfs_list = []
    incubation_dfs_list = []
    open_dfs_list = []
    closed_dfs_list = []

    for name, attr in cell_map.items():
        logging.info(f"Getting data for {name}.")
        cell = api.cell_from_name(name)
        if cell is None:
            logging.warning(f"Cell '{name}' not found.")
            continue

        # TODO Add override of global start/end date

        power = api.power_data(
            cell,
            start_date,
            end_date
        )

        teros = api.teros_data(
            cell,
            start_date,
            end_date
        )

        bme280_cell = api.cell_from_name(cell_map[name]["bme280"])

        def get_bme280_data(meas: str):
            bme280 = api.sensor_data_simple(
                bme280_cell,
                "bme280",
                meas,
                start_date,
                end_date,
                resample="hour",
            )
            if len(bme280) == 0:
                logging.warning(f"No bme280 {meas} data found for {name}")

            return bme280

        bme280_t = get_bme280_data("temperature")
        bme280_p = get_bme280_data("pressure")
        bme280_h = get_bme280_data("humidity")

        # Combine all data into single df
        # TODO Handle missing data, statmodels has ways of handling it
        cell_data = pd.merge(power, teros, on="timestamp", how="inner")
        cell_data = cell_data.merge(bme280_t, on="timestamp", how="inner")
        cell_data = cell_data.merge(bme280_p, on="timestamp", how="inner")
        cell_data = cell_data.merge(bme280_h, on="timestamp", how="inner")

        cell_data["name"] = name

        # soil data
        st = attr["soil"]
        cell_data["type"] = st
        cell_data["sand"] = soil_types[st]["sand"]
        cell_data["silt"] = soil_types[st]["silt"]
        cell_data["clay"] = soil_types[st]["clay"]
        cell_data["om"] = soil_types[st]["om"]

        logger.debug(cell_data)

        all_dfs_list.append(cell_data)

        # Filter based on start/end timestamp
        slice_range = lambda key: cell_data[
            (pd.Timestamp(attr[key]["start"]) <= cell_data["timestamp"])
            & (cell_data["timestamp"] <= pd.Timestamp(attr[key]["end"]))
        ]

        incubation_dfs_list.append(slice_range("incubation"))
        open_dfs_list.append(slice_range("open"))
        closed_dfs_list.append(slice_range("closed"))



    # combine data, reset index
    all_data = pd.concat(all_dfs_list, axis=0, ignore_index=True)
    incubation_data = pd.concat(incubation_dfs_list, axis=0, ignore_index=True)
    open_data = pd.concat(open_dfs_list, axis=0, ignore_index=True)
    closed_data = pd.concat(closed_dfs_list, axis=0, ignore_index=True)


    data = {
        "all": all_data,
        "incubation": incubation_data,
        "open": open_data,
        "closed": closed_data
    }

    return data


def run_pca(df: pd.DataFrame, feature_cols: list[str],
            n_components: int = 5, random_state: int = 42) -> PCAResult:
    """Standardize the features and fit a PCA. Pure computation, no I/O."""
    # Standardize to zero mean / unit variance so large-valued features
    # don't dominate the components
    scaler = StandardScaler()
    X_scaled = scaler.fit_transform(df[feature_cols].values)
 
    pca = PCA(n_components=n_components, random_state=random_state)
    pcs = pca.fit_transform(X_scaled)
 
    pc_names = [f"PC{i + 1}" for i in range(pcs.shape[1])]
 
    scores = pd.DataFrame(pcs, columns=pc_names, index=df.index)
    scores[META_COLS] = df[META_COLS]
 
    cumulative = np.cumsum(pca.explained_variance_ratio_)
    explained = pd.DataFrame({
        "PC": pc_names,
        "eigenvalues": pca.explained_variance_,
        "explained_variance_ratio": pca.explained_variance_ratio_,
        "cumulative": cumulative,
    })
 
    loadings = pd.DataFrame(pca.components_.T, columns=pc_names, index=feature_cols)
 
    # Note: if the 5 kept components never reach 95%, this returns 1.
    n_95 = int(np.argmax(cumulative >= 0.95) + 1)
 
    return PCAResult(pca, scaler, feature_cols, pc_names,
                     scores, explained, loadings, n_95)
  
 
def print_pca_results(result: PCAResult, n_loading_pcs: int = 4) -> None:
    print(result.explained.to_string(index=False))
    print(f"\nComponents needed for 95% variance: {result.n_95}")
 
    print(f"\nLoadings (first {n_loading_pcs} PCs):")
    print(result.loadings.iloc[:, :n_loading_pcs].round(3))
 
 
 
def plot_scree(ax: plt.Axes, result: PCAResult) -> None:
    """Scree plot: variance ratios on the left axis, eigenvalues on the right."""
    pca = result.pca
    ratios = pca.explained_variance_ratio_
    eigenvalues = pca.explained_variance_
    x = np.arange(1, len(result.pc_names) + 1)
 
    ax.bar(x, ratios, alpha=0.7, label="Individual ratio")
    ax.plot(x, np.cumsum(ratios), "o-", color="red", label="Cumulative ratio")
    ax.axhline(0.95, color="gray", ls="--", lw=1)
    ax.set_xlabel("Principal component")
    ax.set_ylabel("Explained variance ratio")
    ax.set_title("Scree plot")
    ax.set_ylim(0, 1.05)
 
    # Total variance across *all* components (ratio = eigenvalue / total_var),
    # so the eigenvalue axis lines up with the ratio axis even when only a
    # subset of components is kept.
    total_var = eigenvalues.sum() / ratios.sum()
 
    ax2 = ax.twinx()
    ax2.plot(x, eigenvalues, "s--", color="green", label="Eigenvalue")
    ax2.axhline(1, color="green", ls=":", lw=1, alpha=0.6)  # Kaiser criterion
    ax2.set_ylabel("Eigenvalue", color="green")
    ax2.tick_params(axis="y", labelcolor="green")
    ax2.set_ylim(0, 1.05 * total_var)
 
    h1, l1 = ax.get_legend_handles_labels()
    h2, l2 = ax2.get_legend_handles_labels()
    ax.legend(h1 + h2, l1 + l2, loc="center right")
 
 
def plot_scores(ax: plt.Axes, result: PCAResult, by: str = "type") -> None:
    """PC1 vs PC2 scores, colored by a metadata column."""
    ratios = result.pca.explained_variance_ratio_
    for label, grp in result.scores.groupby(by):
        ax.scatter(grp["PC1"], grp["PC2"], s=4, alpha=0.4, label=label)
    ax.set_xlabel(f"PC1 ({ratios[0]:.1%})")
    ax.set_ylabel(f"PC2 ({ratios[1]:.1%})")
    ax.set_title(f"PCA scores by {by}")
    ax.legend(markerscale=4)
 
 
def plot_loadings(ax: plt.Axes, result: PCAResult) -> None:
    """Biplot-style loading arrows for PC1 vs PC2."""
    for feat in result.feature_cols:
        lx, ly = result.loadings.loc[feat, "PC1"], result.loadings.loc[feat, "PC2"]
        ax.arrow(0, 0, lx, ly, head_width=0.02, color="tab:red", alpha=0.7)
        ax.text(lx * 1.08, ly * 1.08, feat, fontsize=9)
    ax.set_xlim(-1.1, 1.1)
    ax.set_ylim(-1.1, 1.1)
    ax.axhline(0, color="gray", lw=0.5)
    ax.axvline(0, color="gray", lw=0.5)
    ax.set_xlabel("PC1 loading")
    ax.set_ylabel("PC2 loading")
    ax.set_title("Feature loadings")
    ax.set_aspect("equal")
 
 
def plot_pca_summary(result: PCAResult) -> plt.Figure:
    """Compose the three PCA subplots into one figure."""
    fig, axes = plt.subplots(1, 3, figsize=(20, 6))
    plot_scree(axes[0], result)
    plot_scores(axes[1], result, by="type")
    plot_loadings(axes[2], result)
    fig.tight_layout()
    return fig
 
 
def plot_pairplot(df: pd.DataFrame, feature_cols: list[str],
                  n_sample: int = 2000, random_state: int = 42) -> sns.PairGrid:
    """Pairplot of the raw features, subsampled for speed."""
    plot_df = df[feature_cols + ["type"]].sample(
        n=min(n_sample, len(df)), random_state=random_state
    )
    g = sns.pairplot(
        plot_df,
        vars=feature_cols,
        hue="type",
        corner=True,                 # lower triangle only, halves the panels
        diag_kind="kde",
        plot_kws={"s": 6, "alpha": 0.4, "linewidth": 0},
        diag_kws={"common_norm": False, "fill": True},
        height=1.6,
    )
    g.figure.suptitle("Pairplot of sensor and soil parameters", y=1.02)
    return g


@dataclass
class PCAResult:
    """Everything produced by a PCA run, so printing/plotting need no refit."""

    pca: PCA
    scaler: StandardScaler
    feature_cols: list[str]
    pc_names: list[str]
    scores: pd.DataFrame       # PC scores + metadata columns
    explained: pd.DataFrame    # eigenvalues / variance ratios per PC
    loadings: pd.DataFrame     # features x PCs
    n_95: int                  # components needed for 95% variance




def analyze(data: dict, key: str):
    print(f"Dataset '{key}'")
    print(f"\nSample data for '{key}'\n\n{data[key]}\n")
    stats = data[key].describe()
    print(f"\nDataset\n\n{stats}\n")

    #
    # Select features
    #

    feature_cols = [
        "i", "p", "v", "ec", "raw_vwc", "temp", "vwc",
        "temperature", "pressure", "humidity",
        "sand", "silt", "clay", "om",
    ]

    df = data[key][feature_cols + META_COLS].copy()

    # PCA can't handle NaNs / infs
    df = df.replace([np.inf, -np.inf], np.nan).dropna(subset=feature_cols)



    result = run_pca(df, feature_cols, n_components=5)
    print_pca_results(result)

    summary_fig = plot_pca_summary(result)
    pair_grid = plot_pairplot(df, feature_cols)




def main(plots, output, _format):
    """Run an evaluation.

    Args:
        plots: Show sensor plots if true.
        output: Output directory for plots.
    """


    data = get_data()

    analyze(data, "incubation")
    analyze(data, "open")
    analyze(data, "closed")

    return 0


def parse_args():
    parser = argparse.ArgumentParser(
        description="SMFC analysis script"
    )

    # Verbosity: -v, -vv, -vvv
    parser.add_argument(
        "-v",
        "--verbose",
        action="count",
        default=0,
        help="Increase verbosity level (up to 2)",
    )

    # Optional flag to display plots
    parser.add_argument(
        "-p", "--plots",
        action="store_true",
        default=True,
        help="Display plots",
    )

    parser.add_argument(
        "-o", "--output",
        type=str,
        help="Output directory for plots",
    )

    parser.add_argument(
        "-f", "--format",
        type=str,
        default="jpg",
        help="File format of plots",
    )

    args = parser.parse_args()

    # Cap verbosity at 5
    args.verbose = min(args.verbose, 2)

    return args


if __name__ == "__main__":
    args = parse_args()

    if args.verbose == 0:
        log_level = logging.WARNING
    elif args.verbose == 1:
        log_level = logging.INFO
    elif args.verbose == 2:
        log_level = logging.DEBUG
    logging.basicConfig(level=log_level)

    if args.output:
        logging.info(f"Saving plots to '{args.output}' in '{args.format}' format")

    #print(f"Verbosity level: {args.verbose}")
    #print(f"Show plots? {args.plots}")
    main(args.plots, args.output, args.format)

    if (args.plots):
        plt.show(block=False)
        input("\nPress enter to close figures...")
