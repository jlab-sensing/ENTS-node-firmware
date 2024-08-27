#!/usr/bin/env python

"""SPS nonlinear calibration and evaluation

The ADC data might be non-linear, so this file will attempt to fit a non-linear model to the data.

Stephen Taylor 5/20/2024
"""
#%%
import os
import pandas as pd
import numpy as np
from numpy.polynomial import Polynomial
import matplotlib.pyplot as plt
from sklearn import linear_model
from sklearn.preprocessing import PolynomialFeatures
from sklearn.metrics import mean_absolute_error, mean_squared_error, r2_score, mean_absolute_percentage_error
from scipy.fft import fft, fftfreq
from scipy.stats import norm

#%%
### Load the data ###
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

#%%
### Load the calibration CSVs ###
datafiles = ["data/calibration_data/sps4_voltage_calib_-3to3v.csv"]  # Load voltage
data = load_data(datafiles)

#%%
#### Plot the SMU voltage and the raw SPS values to check for linearity ###
plt.figure()
plt.scatter(data["V_in"], data["V_sps"], s=3)
plt.title("Data visualization")
plt.xlabel("Input (V)")
plt.ylabel("ADC Raw")
plt.show()

plt.figure()
plt.hist(data["V_sps"] - data["V_in"], bins=30, edgecolor='k', alpha=0.7)
plt.title("Histogram of Residuals (Pre Calibration)")
plt.xlabel("Residuals")
plt.ylabel("Frequency")
plt.show()

#%%
### Fit the polynomial model (non-segmented) ###
poly = PolynomialFeatures(degree=4)
V_meas_poly = poly.fit_transform(data[["V_meas"]])
model_poly = linear_model.LinearRegression()
model_poly.fit(V_meas_poly, data["V_in"])

print("Polynomial Model Coefficients:")
print(model_poly.coef_)

# Get residuals from the polynomial fit
poly_predicted = model_poly.predict(V_meas_poly)
residuals = data["V_in"] - poly_predicted

#%%
### Estimate period from residuals and combine with original model ###
# Use FFT on residuals to estimate the period
y_fft = fft(residuals.values)
frequencies = fftfreq(len(residuals), data["V_meas"].values[1] - data["V_meas"].values[0])
positive_frequencies = frequencies[frequencies > 0]
magnitudes = np.abs(y_fft[frequencies > 0])

# Find the dominant frequency and estimate the period
dominant_frequency = positive_frequencies[np.argmax(magnitudes)]
estimated_period = 1 / dominant_frequency
print(f"Estimated period: {estimated_period:.4f}")

# Add sine and cosine features based on the estimated period
data["X_sin"] = np.sin(2 * np.pi * data["V_meas"] / estimated_period)
data["X_cos"] = np.cos(2 * np.pi * data["V_meas"] / estimated_period)

# Combine polynomial and trigonometric features
X_poly_sin_cos = np.column_stack((V_meas_poly, data[["X_sin", "X_cos"]]))

# Fit the linear model with combined features
model_combined = linear_model.LinearRegression()
model_combined.fit(X_poly_sin_cos, data["V_in"])

print("Combined Model Coefficients:")
print(model_combined.coef_)

#%%
### Load the eval files ###
evalfiles = ["data/eval_data/sps4_voltage_eval_-3to3v_sweep1.csv"]
eval_data = load_data(evalfiles)

# Add sine and cosine features for the evaluation data
eval_data["X_eval_sin"] = np.sin(2 * np.pi * eval_data["V_meas"] / estimated_period)
eval_data["X_eval_cos"] = np.cos(2 * np.pi * eval_data["V_meas"] / estimated_period)
V_eval_poly = poly.transform(eval_data[["V_meas"]])
X_eval_poly_sin_cos = np.column_stack((V_eval_poly, eval_data[["X_eval_sin", "X_eval_cos"]]))

# Test the linear model with combined features
combined_predicted = model_combined.predict(X_eval_poly_sin_cos)
combined_residuals = eval_data["V_in"] - combined_predicted

print("Evaluate using sklearn.metrics")
mae = mean_absolute_error(eval_data["V_in"], combined_predicted)
rmse = np.sqrt(mean_squared_error(eval_data["V_in"], combined_predicted))
r2 = r2_score(eval_data["V_in"], combined_predicted)
mape = mean_absolute_percentage_error(eval_data["V_in"], combined_predicted)
print(f"Mean absolute error: {mae:.4f}")
print(f"Mean absolute percentage error: {mape:.4f}")
print(f"Root mean square error: {rmse:.4f}")
print(f"R-squared: {r2:.4f}")

plt.figure()
plt.title("SPS to predicted voltage")
plt.scatter(eval_data["V_sps"], combined_predicted, label="Non-linear model with trigonometric features")
plt.ylabel("Predicted (V)")
plt.xlabel("Raw SPS value")
plt.legend()
plt.show()

plt.figure()
plt.title("Residual plot")
plt.scatter(combined_predicted, combined_residuals)
plt.axhline(y=0, color='r', linestyle='--')
plt.ylabel("Residuals")
plt.xlabel("Predicted (V)")
plt.legend()
plt.show()

plt.figure()
plt.title("Histogram of Residuals (Post Calibration)")
plt.hist(combined_residuals, bins=30, edgecolor='black')
plt.xlabel("Residuals")
plt.ylabel("Frequency")
plt.show()

residual_average = np.average(combined_residuals)
print("Average residual: ", residual_average)

mu, std = norm.fit(combined_residuals)
print(f"Norm distribution parameters: mu = {mu:.4f}, std = {std:.4f}")

#%%
### Print the full equation of the combined model ###
def print_equation(poly_features, coefs):
    terms = poly_features.get_feature_names_out()
    equation = "V_in = "
    for coef, term in zip(coefs, terms):
        if coef != 0:
            equation += f"{coef:.4f} * {term} + "
    equation = equation.rstrip(" + ")
    print(f"Full Model Equation: {equation}")

print_equation(poly, model_combined.coef_)


#%%
def plot_residual_histogram(res, ax=None):
    """Plot a histogram of residuals on a set of axis
    
    Args:
        res: Iterable of residuals values
        ax: Axis to plot on. If "None", a new figure is created
        
    Returns:
        Mean and standard deviation of the normal distribution
    """
    
    if ax is None:
        pass
   
    mu, std = norm.fit(res)
    print(f"Norm distribution parameters: mu = {mu:.4f}, std = {std:.4f}")

    normdist_x = np.linspace(mu - 3*std, mu + 3*std, 100)
    normdist_y = norm.pdf(normdist_x, mu, std)
    
    return mu, std
#%%

mu, std = norm.fit(residuals)
print(f"Norm distribution parameters: mu = {mu:.4f}, std = {std:.4f}")

normdist_x = np.linspace(mu - 3*std, mu + 3*std, 100)
normdist_y = norm.pdf(normdist_x, mu, std)

plt.figure()
plt.hist(residuals, bins=30, density=True, edgecolor='k', alpha=0.7)
plt.plot(normdist_x, normdist_y, color="r")
plt.title("Histogram of Residuals (Post Calibration)")
plt.xlabel("Residuals")
plt.ylabel("Frequency")
plt.show()

std_dev = np.std(residuals)
mean = np.mean(residuals)
print(f"Residual mean: {mean:.4f}, std: {std_dev:.4f}")

within_one_std_dev = np.sum(np.abs(residuals) <= std_dev) / len(residuals) * 100
print(f"Percentage of residuals within one standard deviation: {within_one_std_dev:.2f}%")

for index, value in enumerate(residuals):
    if np.abs(value) > 100:
        print(f"Index: {index}, Value: {value}")

#%%
### Segment the data ###
segments_scheme_1 = [(-3, -0.5), (-0.5, 0.5), (0.5, 3)]
segments_scheme_2 = [(-3, -2), (-2, 0), (0, 2), (2, 3)]

def segment_data(data, segments):
    segmented_data = []
    for (start, end) in segments:
        seg = data[(data["V_in"] >= start) & (data["V_in"] < end)]
        print(f"Segment {start} to {end} contains {len(seg)} samples")  # Debugging statement
        if not seg.empty:
            segmented_data.append(seg)
    return segmented_data

segmented_data_1 = segment_data(data, segments_scheme_1)
segmented_data_2 = segment_data(data, segments_scheme_2)

#%%
### Fit and evaluate segmented models ###
def fit_and_evaluate_segmented_models(segmented_data, eval_data):
    metrics_list = []
    for i, seg in enumerate(segmented_data):
        if not seg.empty:
            print(f"Fitting model for segment {i+1} with {len(seg)} samples")  # Debugging statement
            model, _ = Polynomial.fit(seg["V_meas"], seg["V_in"], 3, full=True)
            predicted = model(eval_data["V_meas"])
            residuals = eval_data["V_in"] - predicted
            mae = mean_absolute_error(eval_data["V_in"], predicted)
            rmse = np.sqrt(mean_squared_error(eval_data["V_in"], predicted))
            r2 = r2_score(eval_data["V_in"], predicted)
            mape = mean_absolute_percentage_error(eval_data["V_in"], predicted)
            metrics_list.append((mae, rmse, r2, mape, residuals))
    return metrics_list

segmented_metrics_1 = fit_and_evaluate_segmented_models(segmented_data_1, eval_data)
segmented_metrics_2 = fit_and_evaluate_segmented_models(segmented_data_2, eval_data)

#%%
### Print segmented model performance ###
print("Segmented model scheme 1 performance:")
for i, metrics in enumerate(segmented_metrics_1):
    if metrics:
        print(f"Segment {i+1}: MAE: {metrics[0]:.4f}, RMSE: {metrics[1]:.4f}, R2: {metrics[2]:.4f}, MAPE: {metrics[3]:.4f}")
    else:
        print(f"Segment {i+1}: No data")

print("Segmented model scheme 2 performance:")
for i, metrics in enumerate(segmented_metrics_2):
    if metrics:
        print(f"Segment {i+1}: MAE: {metrics[0]:.4f}, RMSE: {metrics[1]:.4f}, R2: {metrics[2]:.4f}, MAPE: {metrics[3]:.4f}")
    else:
        print(f"Segment {i+1}: No data")

#%%
### Plot residual histograms for segmented models ###
for i, metrics in enumerate(segmented_metrics_1):
    if metrics:
        mu, std = plot_residual_histogram(metrics[4])
        plt.hist(residuals, bins=30, density=True, edgecolor='k', alpha=0.7)
        plt.plot(normdist_x, normdist_y, color="r")
        plt.title(f"Histogram of Residuals (Segmented Model Scheme 1 - Segment {i+1})")
        plt.xlabel("Residuals")
        plt.ylabel("Frequency")
        plt.show()

for i, metrics in enumerate(segmented_metrics_2):
    if metrics:
        mu, std = plot_residual_histogram(metrics[4])
        plt.hist(residuals, bins=30, density=True, edgecolor='k', alpha=0.7)
        plt.plot(normdist_x, normdist_y, color="r")
        plt.title(f"Histogram of Residuals (Segmented Model Scheme 2 - Segment {i+1})")
        plt.xlabel("Residuals")
        plt.ylabel("Frequency")
        plt.show()
#%%