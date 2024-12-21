import matplotlib.pyplot as plt
from scipy.stats import norm
import numpy as np


def plot_measurements(actual: list, meas: list, title: str = "", block: bool = False):
    """Plot actual values vs measured values

    Args:
        actual: Actual values
        meas: Measured values
        title: Suffix of plot title
        block: Wait for plot to close
    """

    fig, ax = plt.subplots()
    ax.scatter(actual, meas, s=3)
    ax.set_xlabel("Actual")
    ax.set_ylabel("Measured")
    ax.set_title(f"Measurements: {title}")
    plt.show(block=block)


def plot_calib(raw: list, pred: list, title: str = "", block: bool = False):
    """Plot linear relationship between raw measured and calibrated measured

    Args:
        raw: Uncalibrated measurements
        pred: Predicted voltages
        title: Suffix of plot title
        block: Wait for plot to close
    """

    fig, ax = plt.subplots()
    ax.scatter(raw, pred, s=3)
    ax.set_xlabel("Raw measurements")
    ax.set_ylabel("Predicted measurements")
    ax.set_title(f"Calibration: {title}")
    plt.show(block=block)


def plot_residuals(pred: list, residuals: list, title: str = "", block: bool = False):
    """Plot residuals vs predicted measurements

    Residuals is the difference between predicted values and actual values.

    Args:
        pred: Predicted measurements
        residuals: List of residuals
        title: Suffix of plot title
        block: Wait for plot to close
    """

    fig, ax = plt.subplots()
    ax.scatter(pred, residuals)
    ax.axhline(y=0, color="r", linestyle="--")
    ax.set_xlabel("Predicted measurement")
    ax.set_ylabel("Residuals (pred - actual)")
    ax.set_title(f"Residuals: {title}")
    ax.legend()
    plt.show(block=block)


def plot_residuals_hist(residuals: list, title: str = "", block: bool = False):
    """Plot a histogram of residual error

    Args:
        residuals: Residuals
        title: Suffix of plot title
        block: Wait for plot to close
    """

    mu, std = norm.fit(residuals)
    normdist_x = np.linspace(mu - 3 * std, mu + 3 * std, 100)
    normdist_y = norm.pdf(normdist_x, mu, std)

    fig, ax = plt.subplots()
    ax.hist(residuals, bins=30, edgecolor="black")
    ax.plot(normdist_x, normdist_y, color="r")
    ax.set_xlabel("Residuals")
    ax.set_ylabel("Frequency")
    ax.set_title(f"Histogram of Residuals: {title}")
    plt.show(block=block)
