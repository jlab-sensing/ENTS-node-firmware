#!/usr/bin/env python

"""SPS nonlinear calibration and evaluation

The adc data might be non-linear, so this file will attempt to fit a non-linear model to the data.

Stephen Taylor 5/20/2024
"""
#%%
import os
import pandas as pd
import numpy as np
from numpy.polynomial import Polynomial
import matplotlib.pyplot as plt
from rocketlogger.data import RocketLoggerData
from sklearn import linear_model
from sklearn.metrics import mean_absolute_error, mean_squared_error, r2_score, mean_absolute_percentage_error
import yaml
import pdb
try:
    from yaml import CLoader as Loader
except ImportError:
    from yaml import Loader
from scipy.stats import norm

#%%
### Load the data ###
def load_data(datafiles):
    df_list = []
    for d in datafiles:
        df = pd.read_csv(d)
        df_list.append(df)
    
    data = pd.concat(df_list, ignore_index=True)
    #data = data.set_index("V")

    data["V_in"] = data["V_in"]*1000
    data["I_in"] = data["I_in"] 
    data["I_meas"] = data["I_sps"] 
    data["V_meas"] = data["V_sps"]
    
    return data

#%%
### Load the calibration CSVs ###
datafiles = ["data/calibration_data/voltage_calib_-3.3to3.3v_C3removed.csv"] # load voltage
data = load_data(datafiles)

#%%
#### Plot the SMU voltage and the raw SPS values to check for linearity ###
plt.figure()
#for i, _ in enumerate(data["V_in"].to_list()[10:20]):
    #plt.scatter(data["V_in"].to_list()[i], data["V_sps"].to_list()[i], s=3, label = f"{i}")
plt.scatter(data["V_in"], data["V_sps"], s=3)
plt.title("Data visualization")
plt.xlabel("Input (V)")
plt.ylabel("ADC Raw")
plt.legend()
plt.show()

# plt.figure()
# plt.plot(data["V_in"], label = "SMU Voltage")
# plt.title("SMU")
# plt.xlabel("Index")
# plt.ylabel("(V)")
# plt.legend()
# plt.show()

# plt.figure()
# plt.plot(data["V_sps"], label = "Soil Power Sensor Raw")
# plt.title("Raw SPS")
# plt.xlabel("Index")
# plt.ylabel("SPS raw values")
# plt.legend()
# plt.show()

plt.figure()
plt.hist(data["V_sps"] - data["V_in"], bins=30, edgecolor='k', alpha=0.7)
plt.title("Histogram of Residuals (Pre Calibration)")
plt.xlabel("Residuals")
plt.ylabel("Frequency")
plt.show()

#%%
### Fit the linear model ###
model, params = Polynomial.fit(data["V_meas"], data["V_in"], 5, full=True)
print(model)
print(model.convert().coef)

#print("Voltage coefficients ax^2 + bx + c: ", "a:", coefficients[0], "b", coefficients[1], "c", coefficients[2])

#%%
### Load the eval files ###
evalfiles = ["data/eval_data/voltage_eval_-3.3to3.3v_C3removed.csv"]
eval_data = load_data(evalfiles)

#%%
### Test the fit ###
predicted = model(eval_data["V_meas"])

residuals = eval_data["V_in"] - predicted # Calculate the residuals


print("Evaluate using sklearn.metrics")
mae = mean_absolute_error(eval_data["V_in"], predicted)
rmse = np.sqrt(mean_squared_error(eval_data["V_in"], predicted))
r2 = r2_score(eval_data["V_in"], predicted)
mape = mean_absolute_percentage_error(eval_data["V_in"], predicted)
print(f"Mean absolute error: {mae:.4f}")
print(f"Mean absolute percentage error: {mape:.4f}")
print(f"Root mean square error: {rmse:.4f}")
print(f"R-squared: {r2:.4f}")


plt.figure()
plt.title("SPS to predicted voltage")
plt.scatter(eval_data["V_sps"], predicted, label = "Linear model")
plt.ylabel("Predicted (V)")
plt.xlabel("Raw SPS value")
plt.legend()
plt.show()

plt.figure()
plt.title("Residual plot")
plt.scatter(predicted, residuals)
plt.axhline(y=0, color='r', linestyle='--')
plt.ylabel("Residuals")
plt.xlabel("Predicted (V)")
plt.legend()
plt.show()

# Plot residuals histogram
plt.figure()
plt.title("Histogram of Residuals")
plt.hist(residuals, bins=30, edgecolor='black')
plt.xlabel("Residuals")
plt.ylabel("Frequency")
plt.show()

residual_average = np.average(residuals)
print("Average residual: ", residual_average)

#%%

def plot_residual_histogram(res, ax=None):
    """Plot a histogram of residuals on a set of axis
    
    Args:
        res: Iterable of residuals values
        ax: Axis to plot on. If "None", a new figure is created
        
    Returns:
        Mean and standard deviation of the normal distribution
    """
    
    if ax == None:
        pass
   
    # 
    mu, std = norm.fit(residuals)
    print(f"Norm distribution parameters: mu = {mu:.4f}, std = {std:.4f}")

    normdist_x = np.linspace(mu - 3*std, mu + 3*std, 100)
    normdist_y = norm.pdf(normdist_x, mu, std)
    
    return mu, std

mu, std = norm.fit(residuals)
print(f"Norm distribution parameters: mu = {mu:.4f}, std = {std:.4f}")

normdist_x = np.linspace(mu - 3*std, mu + 3*std, 100)
normdist_y = norm.pdf(normdist_x, mu, std)

### Histogram of residuals ###
plt.figure()
plt.hist(residuals, bins=30, density=True, edgecolor='k', alpha=0.7)
plt.plot(normdist_x, normdist_y, color="r")
plt.title("Histogram of Residuals (Post Calibration)")
plt.xlabel("Residuals")
plt.ylabel("Frequency")
plt.show()

#%%
### Calculate standard deviation of residuals ###
std_dev = np.std(residuals)
mean = np.mean(residuals)
print(f"Residual mean: {mean:.4f}, std: {std_dev:.4f}")

#%%
for index, value in enumerate(residuals):
    if np.abs(value) > 100:
        print(f"Index: {index}, Value: {value}")
           
quit()
