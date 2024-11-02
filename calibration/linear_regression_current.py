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
########## CURRENT CALIBRARTION ###########
print("Current Calibration")
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
curr_datafiles = ["current.csv"] # load voltage


curr_data = load_data(curr_datafiles)


#%%
#### Plot the SMU voltage and the raw SPS values to check for linearity ###
plt.figure()
plt.scatter(curr_data["I_in"], curr_data["I_sps"], s=3)
plt.title("Data visualization")
plt.xlabel("Input (I)")
plt.ylabel("ADC Raw")
plt.legend()
plt.show()

#%%
### Fit the linear model ###
i_input_cols = ["I_meas"]
i_model = linear_model.LinearRegression()
i_model.fit(curr_data[i_input_cols], curr_data["I_in"])

print("Current coefficients: ", i_model.coef_, "Current intercept: ", i_model.intercept_)

#%%
### Load the eval files ###
evalfiles = ["current_2.csv"]
eval_data = load_data(evalfiles)

#%%
### Test the fit ###
predicted = i_model.predict(eval_data["I_meas"].values.reshape(-1, 1))

residuals = eval_data["I_in"] - predicted # Calculate the residuals

print("Evaluate using sklearn.metrics")
mae = mean_absolute_error(eval_data["I_in"], predicted)
rmse = np.sqrt(mean_squared_error(eval_data["I_in"], predicted))
r2 = r2_score(eval_data["I_in"], predicted)
mape = mean_absolute_percentage_error(eval_data["I_in"], predicted)
print(f"Mean absolute error: {mae:.4f}")
print(f"Mean absolute percentage error: {mape:.4f}")
print(f"Root mean square error: {rmse:.4f}")
print(f"R-squared: {r2:.4f}")


plt.figure()
plt.title("SPS to predicted current")
plt.scatter(eval_data["I_sps"], predicted, label = "Linear model")
plt.ylabel("Predicted (I)")
plt.xlabel("Raw SPS value")
plt.legend()
plt.show()

plt.figure()
plt.title("Residual plot")
plt.scatter(predicted, residuals)
plt.axhline(y=0, color='r', linestyle='--')
plt.ylabel("Residuals")
plt.xlabel("Predicted (I)")
plt.legend()
plt.show()

mu, std = norm.fit(residuals)
normdist_x = np.linspace(mu - 3*std, mu + 3*std, 100)
normdist_y = norm.pdf(normdist_x, mu, std)

# Plot residuals histogram
plt.figure()
plt.title("Histogram of Residuals")
plt.hist(residuals, bins=30, edgecolor='black')
#plt.plot(normdist_x, normdist_y, color="r")
plt.xlabel("Residuals")
plt.ylabel("Frequency")
plt.show()

print("STD of residuals: ", np.std(residuals))
# %%
