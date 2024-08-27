"""SPS normalizing to remove non-linearity

The adc data might be non-linear, so this file will attempt to remove said non-linearity from the ADC readings

Stephen Taylor 5/20/2024
"""
#%%
import os
import pandas as pd
import numpy as np
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

#%%
### Load the data ###
def load_data(datafiles):
    df_list = []
    for d in datafiles:
        df = pd.read_csv(d)
        df_list.append(df)
    
    data = pd.concat(df_list, ignore_index=True)
    #data = data.set_index("V")

    data["V_in"] = data["V_in"]
    data["I_in"] = data["I_in"] 
    data["I_meas"] = data["I_sps"] 
    data["V_meas"] = data["V_sps"]
    
    return data

#%%
### Load the calibration CSVs ###
datafiles = ["data/calibration_data/sps4_voltage_calib_-3to3v.csv"] # load voltage
data = load_data(datafiles)

#%%
### Fit the linear model ###
v_input_cols = ["V_meas"]
v_model = linear_model.LinearRegression()
v_model.fit(data[v_input_cols], data["V_in"])

print("Voltage coefficients: ", v_model.coef_, "Voltage intercept: ", v_model.intercept_)

#%%
### Load the eval files ###
n = 5 # Number of eval files
evalfiles_pattern = "data/eval_data/sps4_voltage_eval_-3to3v_sweep"
evalfiles = [f"{evalfiles_pattern}{i}.csv" for i in range(1, n+1)] # Replace 'n' with the number of CSV files
eval_data_list = []
for f in evalfiles:
    eval_data = load_data([f])
    eval_data_list.append(eval_data)

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

#%%
### Test the fit and plot residuals ###
colors = plt.cm.jet(np.linspace(0, 1, len(eval_data_list)))

plt.figure()
plt.title("Residual plot SPS4")

for i, eval_data in enumerate(eval_data_list):
    predicted = v_model.predict(eval_data["V_meas"].values.reshape(-1, 1))
    residuals = eval_data["V_in"] - predicted # Calculate the residuals
    plt.scatter(predicted, residuals, color=colors[i], label=f'Sweep {i+1}')

plt.axhline(y=0, color='r', linestyle='--')
plt.ylabel("Residuals")
plt.xlabel("Predicted (V)")
plt.legend()
plt.show()

#%%
