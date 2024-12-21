import numpy as np
from sklearn import linear_model
from sklearn.metrics import (
    mean_absolute_error,
    mean_squared_error,
    r2_score,
    mean_absolute_percentage_error,
)
from scipy.stats import norm


def linear_regression(meas: list, actual: list):
    """Performs linear regression between measured and actual values

    Args:
        meas: Measure values
        actual: Ground truth values

    Returns:
        Linear regression model
    """

    model = linear_model.LinearRegression()
    model.fit(meas, actual)

    return model


def print_eval(pred: list, actual: list):
    """Prints various evaluation parameters of the model

    The following metrics are used:
        - Mean absolute error
        - Root mean square error
        - R2 score
        - Mean absolute percentage error

    Args:
        pred: Predicted values
        actual: Actual values
    """
    mae = mean_absolute_error(actual, pred)
    print(f"Mean absolute error: {mae:.4f}")

    rmse = np.sqrt(mean_squared_error(actual, pred))
    print(f"Root mean square error: {rmse:.4f}")

    r2 = r2_score(actual, pred)
    print(f"R-squared: {r2:.4f}")

    mape = mean_absolute_percentage_error(actual, pred)
    print(f"Mean absolute percentage error: {mape:.4f}")


def print_coef(model):
    """Print coefficients of a model

    Args:
        model: Linear model
    """

    slope = model.coef_
    inter = model.intercept_

    print(f"Slope: {slope}")
    print(f"Intercept: {inter}")


def print_norm(residuals: list):
    """Print mean offset and standard deviation

    Args:
        residuals: List of residuals
    """

    mu, std = norm.fit(residuals)
    print(f"mu = {mu}")
    print(f"std = {std}")
