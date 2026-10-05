#!/usr/bin/env python

import os
import argparse
from datetime import datetime
import logging

import statsmodels.api as sm
from statsmodels.formula.api import ols, mixedlm
from statsmodels.stats.outliers_influence import variance_inflation_factor
import pandas as pd
import seaborn as sns
import numpy as np
from sklearn.preprocessing import StandardScaler
from sklearn.decomposition import PCA

import matplotlib.pyplot as plt

from ents.dirtviz import BackendClient, plot_data


logger = logging.getLogger(__name__)


def main(plots, output, _format):
    """Run an evaluation.

    Args:
        plots: Show sensor plots if true.
        output: Output directory for plots.
    """

    # span of data
    start_date = datetime(2025, 11, 1)
    end_date = datetime(2026, 1, 19)
    logger.info(f"Using data from {start_date} to {end_date}.")



    soil_types = {
        "gt_black": {
            "desc": "Georgia natural black soil.",
            "sand": 0.7322,
            "silt": 0.1206,
            "clay": 0.1472,
            "om": 0.02,
        },

        "gt_black_inorganics": {
            "desc": "Black to red texture.",
            "sand": 0.349,
            "silt": 0.33301,
            "clay": 0.3180,
            "om": 0.02,
        },

        "gt_black_inorganics_compost": {
            "desc": "Black to red texture and om.",
            "sand": 0.329,
            "silt": 0.333,
            "clay": 0.338,
            "om": 0.009,
        },

        "gt_red": {
            "desc": "Georgia natural red soil.",
            "sand": 0.4789,
            "silt": 0.3053,
            "clay": 0.2158,
            "om": 0.01,
        },

        # TODO Update
        "gt_red_compost": {
            "desc": "Red to black om.",
            "sand": 0.3498,
            "silt": 0.3195,
            "clay": 0.3307,
            "om": 0.02,
        },

        # TODO upate
        "gt_red_compost_inorganics": {
            "desc": "Red to black texture.",
            "sand": 0.712,
            "silt": 0.141,
            "clay": 0.147,
            "om": 0.01,
        },

        "synth": {
            "desc": "Synthetic soil.",
            "sand": 0.3476,
            "silt": 0.3221,
            "clay": 0.3303,
            "om": 0.11,
        },
    }

    cell_map = {
        ## Black Soil

        # No amendments
        "gt_black_none_1": {
            "soil": "gt_black",
            "bme280": "gt_bme280",
        },
        "gt_black_none_2": {
            "soil": "gt_black",
            "bme280": "gt_bme280",
        },
        "gt_black_none_3": {
            "soil": "gt_black",
            "bme280": "gt_bme280",
        },

        # Adding inorganics
        "gt_black_inorganics_1": {
            "soil": "gt_black_inorganics",
            "bme280": "gt_bme280",
        },
        "gt_black_inorganics_2": {
            "soil": "gt_black_inorganics",
            "bme280": "gt_bme280",
        },
        "gt_black_inorganics_3": {
            "soil": "gt_black_inorganics",
            "bme280": "gt_bme280",
        },
        
        # Adding inorganics and compost
        "gt_black_inorganics_compost_1": {
            "soil": "gt_black_inorganics_compost",
            "bme280": "gt_bme280",
        },
        "gt_black_inorganics_compost_2": {
            "soil": "gt_black_inorganics_compost",
            "bme280": "gt_bme280",
        },
        "gt_black_inorganics_compost_3": {
            "soil": "gt_black_inorganics_compost",
            "bme280": "gt_bme280",
        },


        "gt_red_none_1": {
            "soil": "gt_red",
            "bme280": "gt_bme280",
        },
        "gt_red_none_2": {
            "soil": "gt_red",
            "bme280": "gt_bme280",
        },
        "gt_red_none_3": {
            "soil": "gt_red",
            "bme280": "gt_bme280",
        },
        
        "gt_red_compost_1": {
            "soil": "gt_red_compost",
            "bme280": "gt_bme280",
        },
        "gt_red_compost_2": {
            "soil": "gt_red_compost",
            "bme280": "gt_bme280",
        },
        "gt_red_compost_3": {
            "soil": "gt_red_compost",
            "bme280": "gt_bme280",
        },
        
        "gt_red_compost_inorganics_1": {
            "soil": "gt_red_compost_inorganics",
            "bme280": "gt_bme280",
        },
        "gt_red_compost_inorganics_2": {
            "soil": "gt_red_compost_inorganics",
            "bme280": "gt_bme280",
        },
        "gt_red_compost_inorganics_3": {
            "soil": "gt_red_compost_inorganics",
            "bme280": "gt_bme280",
        },

        "gt_synth_none_1": {
            "soil": "synth",
            "bme280": "gt_bme280",
        },
        "gt_synth_none_2": {
            "soil": "synth",
            "bme280": "gt_bme280",
        },
        "gt_synth_none_3": {
            "soil": "synth",
            "bme280": "gt_bme280",
        },
        
        # Exclude control
        #"gt_synth_auto_1": {
        #    "soil": "synth",
        #    "bme280": "gt_bme280",
        #},
        #"gt_synth_auto_2": {
        #    "soil": "synth",
        #    "bme280": "gt_bme280",
        #},
        #"gt_synth_auto_3": {
        #    "soil": "synth",
        #    "bme280": "gt_bme280",
        #},

    }

    api = BackendClient()

    all_dfs_list = []

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




    def show(name: str):
        """Show a matplotlib figure.

        Handles displaying and saving matplotlib figures. Name is used for the
        filepath of the images.

        Args:
            name: Name of image.
        """

        plt.tight_layout()
        plt.show(block=False)

        if output:
            cwd = os.getcwd()
            directory = os.path.join(cwd, output)
            os.makedirs(directory, exist_ok=True)
            path = os.path.join(directory, f"{name}.{_format}")
            logging.debug(f"Saving to {path}")
            plt.savefig(path)

    # expected columns that can be used
    # i    p    timestamp  v ec  raw_vwc   temp    vwc   name type  sand  silt om


    category_labels = ["name", "type", "sand", "silt", "clay", "om"]
    meas_labels = ["i", "v", "p", "ec", "raw_vwc", "vwc", "temp",
                   "temperature", "pressure", "humidity"]

    # parameters
    target_label = "v"
    time_covariate_labels = ["raw_vwc"]
    categorical_covariate_labels = []
    #group_label = "name"

    # plot data
    #if plots:
    #    for label in meas_labels:
    #        plot_data(all_dfs_list, label)

    # combine data, reset index
    all_data = pd.concat(all_dfs_list, axis=0, ignore_index=True)


    print(f"\nSample data\n\n{all_data}\n")


    # all statistics
    all_stats = all_data.describe()
    print(f"\nDataset\n\n{all_stats}\n")






    #
    # Select features
    #

    feature_cols = [
        "i", "p", "v", "ec", "raw_vwc", "temp", "vwc",
        "temperature", "pressure", "humidity",
        "sand", "silt", "clay", "om",
    ]

    df = all_data[feature_cols + ["name", "type", "timestamp"]].copy()

    # PCA can't handle NaNs / infs
    df = df.replace([np.inf, -np.inf], np.nan).dropna(subset=feature_cols)

    X = df[feature_cols].values


    #
    # Scale data to 0-1 to prevent large values from dominating results
    #

    scaler = StandardScaler()
    X_scaled = scaler.fit_transform(X)

    #
    # Fit PCA (keep all components first to inspect variance)
    #
    pca = PCA(n_components=5, random_state=42)
    pcs = pca.fit_transform(X_scaled)

    pc_names = [f"PC{i+1}" for i in range(pcs.shape[1])]
    pca_df = pd.DataFrame(pcs, columns=pc_names, index=df.index)
    pca_df[["name", "type", "timestamp"]] = df[["name", "type", "timestamp"]]

    #
    # Explained variance
    #
    explained = pd.DataFrame({
        "PC": pc_names,
        "eigenvalues": pca.explained_variance_,
        "explained_variance_ratio": pca.explained_variance_ratio_,
        "cumulative": np.cumsum(pca.explained_variance_ratio_),
    })
    print(explained.to_string(index=False))

    n_95 = np.argmax(np.cumsum(pca.explained_variance_ratio_) >= 0.95) + 1
    print(f"\nComponents needed for 95% variance: {n_95}")


    # 
    # Loadings
    #

    loadings = pd.DataFrame(
        pca.components_.T,
        columns=pc_names,
        index=feature_cols,
    )
    print("\nLoadings (first 4 PCs):")
    print(loadings.iloc[:, :4].round(3))




    # 
    # plots
    #


    _, axes = plt.subplots(1, 3, figsize=(20, 6))

    # 6a. Scree plot with eigenvalues on a second y-axis
    eigenvalues = pca.explained_variance_
    total_var = eigenvalues.sum()
    x = np.arange(1, len(pc_names) + 1)

    ax = axes[0]
    ax.bar(x, pca.explained_variance_ratio_, alpha=0.7, label="Individual ratio")
    ax.plot(x, np.cumsum(pca.explained_variance_ratio_),
            "o-", color="red", label="Cumulative ratio")
    ax.axhline(0.95, color="gray", ls="--", lw=1)
    ax.set_xlabel("Principal component")
    ax.set_ylabel("Explained variance ratio")
    ax.set_title("Scree plot")
    ax.set_ylim(0, 1.05)

    # Second y-axis: eigenvalues
    ax2 = ax.twinx()
    ax2.plot(x, eigenvalues, "s--", color="green", label="Eigenvalue")
    ax2.axhline(1, color="green", ls=":", lw=1, alpha=0.6)   # Kaiser criterion line
    ax2.set_ylabel("Eigenvalue", color="green")
    ax2.tick_params(axis="y", labelcolor="green")
    ax2.set_ylim(0, 1.05 * total_var)   # same scale as the ratio axis (ratio = eigenvalue / total_var)

    # Combine legends from both axes
    h1, l1 = ax.get_legend_handles_labels()
    h2, l2 = ax2.get_legend_handles_labels()
    ax.legend(h1 + h2, l1 + l2, loc="center right")


    # 6b. Scores colored by sensor type
    ax = axes[1]
    for t, grp in pca_df.groupby("type"):
        ax.scatter(grp["PC1"], grp["PC2"], s=4, alpha=0.4, label=t)
    ax.set_xlabel(f"PC1 ({pca.explained_variance_ratio_[0]:.1%})")
    ax.set_ylabel(f"PC2 ({pca.explained_variance_ratio_[1]:.1%})")
    ax.set_title("PCA scores by type")
    ax.legend(markerscale=4)

    # 6c. Biplot-style loading arrows
    ax = axes[2]
    for feat in feature_cols:
        x, y = loadings.loc[feat, "PC1"], loadings.loc[feat, "PC2"]
        ax.arrow(0, 0, x, y, head_width=0.02, color="tab:red", alpha=0.7)
        ax.text(x * 1.08, y * 1.08, feat, fontsize=9)
    ax.set_xlim(-1.1, 1.1)
    ax.set_ylim(-1.1, 1.1)
    ax.axhline(0, color="gray", lw=0.5)
    ax.axvline(0, color="gray", lw=0.5)
    ax.set_xlabel("PC1 loading")
    ax.set_ylabel("PC2 loading")
    ax.set_title("Feature loadings")
    ax.set_aspect("equal")

    plt.tight_layout()
    plt.show(block=False)



    # Subsample for speed (36k rows x 196 panels is very slow)
    n_sample = 2000
    plot_df = df[feature_cols + ["type"]].sample(n=min(n_sample, len(df)), random_state=42)

    g = sns.pairplot(
        plot_df,
        vars=feature_cols,
        hue="type",
        corner=True,                 # lower triangle only, halves the panels
        diag_kind="kde",             # or "hist"
        plot_kws={"s": 6, "alpha": 0.4, "linewidth": 0},
        diag_kws={"common_norm": False, "fill": True},
        height=1.6,
    )
    g.figure.suptitle("Pairplot of sensor and soil parameters", y=1.02)
    plt.show(block=False)

    input("Press enter to close figures...")




    return 0





    def plot_boxplot(meas: str, group: str):
        """Plot a boxplot of a measurement in a group.


        Args:
            meas: Measurement column.
            group: Grouping column.
        """

        # boxplot of type
        _, ax = plt.subplots()

        groups = [g[meas].values for _, g in all_data.groupby(group)]
        labels = all_data[group].unique()
        ax.boxplot(groups, tick_labels=labels)
        ax.set_ylabel(meas)
        show(f"boxplot_{group}_{meas}")


    type_stats = all_data.groupby("type").describe()
    for label in meas_labels:
        plot_boxplot(label, "type")

    # Uncomment to see individual cell boxplots
    #name_stats = all_data.groupby("name").describe()
    #for label in meas_labels:
    #    plot_boxplot(label, "name")




    # TODO 
    # Plot the distribution of measurements

    ## BAD CODE
    ## plot max
    #if plots:
    #    _, ax = plt.subplots()
    #    ax.hist(name_stats["v"]["mean"], edgecolor="black", bins=10)
    #    ax.set_xlabel("Voltage (mV)")
    #    ax.set_ylabel("Frequency")
    #    plt.tight_layout()
    #    plt.show(block=False)




    # Colinearity checks

    # Multicollinearity check
    X = all_data[meas_labels]
    X = sm.add_constant(X)
    vif = pd.DataFrame({
        "variable": X.columns,
        "VIF": [variance_inflation_factor(X.values, i) for i in range(X.shape[1])]
    })
    print("\n\nChecking multicollinearity:")
    print("\tRules of thumb for variation inflation factor:")
    print("\tVIF > 5\t\tModerate")
    print("\tVIF > 10\tSevere")
    print("\tVIF = inf\tLinear dependence")
    print()
    print(vif)

    if plots:
        _, ax = plt.subplots()
        sns.heatmap(all_data[meas_labels].corr(), ax=ax, annot=True)
        show("collinearity")



    # look into the following
    # organic matter
    # characteristics (sand, silt, clay)
    #formula = "v ~ ec + raw_vwc + temp + sand"
    #formula = "v ~ sand + silt + om"
    formula = f"{target_label} ~ "
    formula += " + ".join(time_covariate_labels)
    if categorical_covariate_labels:
        formula += " + "
        formula += " + ".join([f"C({c})" for c in categorical_covariate_labels])
    print(f"\nUsing formula:\n\n\t{formula}")


    # OLS Model
    
    def ols_agg(stat: str, fn: str):
        """Run ordinary least squares on aggregated data.

        Args:
            stat: Statistic metric to aggregate on (eg. "mean")
            fn: Model function
        """

        # format data
        agg_dict = {"timestamp": "first"}
        agg_dict.update({col: stat for col in meas_labels})
        agg_dict.update({col: "first" for col in category_labels})
        stat_data = all_data.groupby("type").agg(agg_dict).reset_index(drop=True)

        # fit model
        model = ols(fn, data=stat_data).fit()
        print()
        print(model.summary())
        print()


        # eval
        anova = sm.stats.anova_lm(model, typ=2)
        # Cohen's values 0.10, 0.25 and 0.40 (Cohen, 1988, pp. 285-287)
        anova["eta_sq"] = anova["sum_sq"] / (anova["sum_sq"].sum())
        anova["part_eta_sq"] = anova["sum_sq"] / (anova["sum_sq"] + anova.loc["Residual", "sum_sq"])

        print(f"\n\nANOVA Results:\n\n{anova}\n\n")


        # fit value plot
        y_hat = model.fittedvalues

        fig, ax = plt.subplots()
        ax.scatter(model.model.endog, y_hat)
        ax.set_title(f"{stat}: {fn}")
        ax.set_xlabel("Observed")
        ax.set_ylabel("Predicted")
        ax.plot([model.model.endog.min(), model.model.endog.max()],
                 [model.model.endog.min(), model.model.endog.max()])
        show(f"ols_agg_{stat}")


    ols_agg("mean", "v ~ raw_vwc + ec + sand + silt + om")
    ols_agg("max", "v ~ raw_vwc + ec + sand + silt + om")



    # MixedLM model
    agg_dict = {"timestamp": "first"}
    agg_dict.update({col: "mean" for col in meas_labels})
    agg_dict.update({col: "first" for col in category_labels})
    mean_data = all_data.groupby("type").agg(agg_dict).reset_index(drop=True)

    model = mixedlm("v ~ raw_vwc + ec + sand + silt + temp", mean_data,
                    groups=mean_data["type"])
    modelf = model.fit()
    print()
    print(modelf.summary())




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
        input("Press enter to close figures...")
