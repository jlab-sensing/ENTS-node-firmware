"""SPS calibration and evaluation

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
from scipy.stats import norm
try:
    from yaml import CLoader as Loader
except ImportError:
    from yaml import Loader
#%%
########## VOLTAGE CALIBRARTION ###########
print("Voltage calibration")
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
datafiles = ["voltage.csv"] # load voltage

#%%
### Load into a data frame ##


data = load_data(datafiles)
#data = data[(data["V_in"] > -2000) & (data["V_in"] < 2000)]


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
### Fit the linear model ###
v_input_cols = ["V_meas"]
v_model_pos = linear_model.LinearRegression()
v_model_pos.fit(data[v_input_cols], data["V_in"])

print("Voltage coefficients: ", v_model_pos.coef_, "Voltage intercept: ", v_model_pos.intercept_)

#%%
### Load the eval files ###
evalfiles = ["voltage.csv"]
eval_data = load_data(evalfiles)
eval_data = eval_data[(eval_data["V_in"] > -2000) & (eval_data["V_in"] < 2000)]

#%%
### Test the fit ###
predicted = v_model_pos.predict(eval_data["V_meas"].values.reshape(-1, 1))

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

mu, std = norm.fit(residuals)
normdist_x = np.linspace(mu - 3*std, mu + 3*std, 100)
normdist_y = norm.pdf(normdist_x, mu, std)

# Plot residuals histogram
plt.figure()
plt.title("Histogram of Residuals")
plt.hist(residuals, bins=30, edgecolor='black')
plt.plot(normdist_x, normdist_y, color="r")
plt.xlabel("Residuals")
plt.ylabel("Frequency")
plt.show()

print("STD of residuals: ", np.std(residuals))