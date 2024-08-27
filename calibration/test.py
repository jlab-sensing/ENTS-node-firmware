#%%mport os
import pandas as pd
import numpy as np
from numpy.polynomial import Polynomial
import matplotlib.pyplot as plt
from sklearn import linear_model
from sklearn.metrics import mean_absolute_error, mean_squared_error, r2_score, mean_absolute_percentage_error
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import PolynomialFeatures
from sklearn.linear_model import Ridge
from scipy.fft import fft, fftfreq
from scipy.stats import norm

# Load and prepare the data
def load_data(datafiles):
    df_list = []
    for d in datafiles:
        df = pd.read_csv(d)
        df_list.append(df)
    data = pd.concat(df_list, ignore_index=True)
    data["V_in"] = data["V_in"] * 1000
    data["I_in"] = data["I_in"] 
    data["I_meas"] = data["I_sps"] 
    data["V_meas"] = data["V_sps"]
    return data

# Load calibration data
datafiles = ["data/calibration_data/sps4_voltage_calib_-3to3v.csv"]
data = load_data(datafiles)

# Polynomial and trigonometric feature transformation
def apply_transformations(data):
    # Fit polynomial features
    poly = PolynomialFeatures(degree=4, include_bias=False)
    X_poly = poly.fit_transform(data[["V_meas"]])
    
    # Estimate period and apply Fourier transform
    y_fft = fft(data["V_meas"].values)
    frequencies = fftfreq(len(data["V_meas"]), data["V_meas"].iloc[1] - data["V_meas"].iloc[0])
    positive_frequencies = frequencies[frequencies > 0]
    magnitudes = np.abs(y_fft[frequencies > 0])
    dominant_frequency = positive_frequencies[np.argmax(magnitudes)]
    estimated_period = 1 / dominant_frequency

    # Add sine and cosine features
    data["X_sin"] = np.sin(2 * np.pi * data["V_meas"] / estimated_period)
    data["X_cos"] = np.cos(2 * np.pi * data["V_meas"] / estimated_period)
    X_trig = data[["X_sin", "X_cos"]]
    
    # Combine polynomial and trigonometric features
    X_combined = np.hstack((X_poly, X_trig))
    return X_combined, estimated_period, poly

# Apply transformations
X_combined, estimated_period, poly = apply_transformations(data)
y = data["V_in"]

# Split the data
X_train, X_test, y_train, y_test = train_test_split(X_combined, y, test_size=0.2, random_state=42)

# Fit Ridge regression model
ridge_model = Ridge(alpha=1.0)
ridge_model.fit(X_train, y_train)

# Predict and evaluate
y_pred = ridge_model.predict(X_test)
residuals = y_test - y_pred

mae = mean_absolute_error(y_test, y_pred)
rmse = np.sqrt(mean_squared_error(y_test, y_pred))
r2 = r2_score(y_test, y_pred)
mape = mean_absolute_percentage_error(y_test, y_pred)
print(f"Mean absolute error: {mae:.4f}")
print(f"Mean absolute percentage error: {mape:.4f}")
print(f"Root mean square error: {rmse:.4f}")
print(f"R-squared: {r2:.4f}")

plt.figure()
plt.title("SPS to predicted voltage")
plt.scatter(y_test, y_pred, label="Ridge Regression Model")
plt.ylabel("Predicted (mV)")
plt.xlabel("Actual (mV)")
plt.legend()
plt.show()

plt.figure()
plt.title("Residual plot")
plt.scatter(y_pred, residuals)
plt.axhline(y=0, color='r', linestyle='--')
plt.ylabel("Residuals")
plt.xlabel("Predicted (mV)")
plt.legend()
plt.show()

plt.figure()
plt.title("Histogram of Residuals")
plt.hist(residuals, bins=30, edgecolor='black')
plt.xlabel("Residuals")
plt.ylabel("Frequency")
plt.show()

residual_average = np.average(residuals)
print("Average residual: ", residual_average)
mu, std = norm.fit(residuals)
print(f"Norm distribution parameters: mu = {mu:.4f}, std = {std:.4f}")

# Print the final model equation
def print_model_equation(coefficients, poly, trig_period):
    terms = []
    feature_names = poly.get_feature_names_out()
    for i, coef in enumerate(coefficients):
        term = f"{coef:.30f} * "
        term += " + ".join([f"({name})" for name in feature_names[i: i + 1]])
        terms.append(term)
    equation = " + ".join(terms)
    print(f"Final Model Equation: V_in = {equation}")

# Extract coefficients and print equation
coefficients = ridge_model.coef_
print_model_equation(coefficients, poly, estimated_period)
#%%