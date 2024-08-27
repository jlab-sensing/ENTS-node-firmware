#%%
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from sklearn.model_selection import train_test_split
from sklearn.metrics import mean_absolute_error, mean_squared_error, r2_score
from sklearn.linear_model import LinearRegression, Ridge, Lasso, ElasticNet
from sklearn.svm import SVR
from sklearn.tree import DecisionTreeRegressor
from sklearn.ensemble import RandomForestRegressor, GradientBoostingRegressor
from sklearn.neighbors import KNeighborsRegressor
from sklearn.neural_network import MLPRegressor
from sklearn.gaussian_process import GaussianProcessRegressor

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
# Load the calibration and evaluation CSVs
datafiles = ["data/calibration_data/sps4_voltage_calib_-3to3v_sweep6.csv"] # Load voltage calibration data
evalfiles = [f"data/eval_data/sps4_voltage_eval_-3to3v_sweep{i}.csv" for i in range(1, 6)] # Load evaluation data

# Load calibration data
data = load_data(datafiles)

# Load evaluation data
eval_data = load_data(evalfiles)

# Combine calibration and evaluation data if needed, otherwise just use eval_data
combined_data = pd.concat([data, eval_data])

# Extract features and target variables
x = combined_data["V_meas"].values.reshape(-1, 1)
y = combined_data["V_in"].values
#%%
# Split data into training and testing sets
X_train, X_test, y_train, y_test = train_test_split(x, y, test_size=0.2, random_state=42)

#%%
# List of models to evaluate
models = {
    "Linear Regression": LinearRegression(),
    "Ridge Regression": Ridge(),
    "Lasso Regression": Lasso(),
    "Elastic Net": ElasticNet(),
    "SVR": SVR(),
    "Decision Tree": DecisionTreeRegressor(),
    "Random Forest": RandomForestRegressor(),
    "Gradient Boosting": GradientBoostingRegressor(),
    "KNN": KNeighborsRegressor(),
    #"Neural Network": MLPRegressor(max_iter=1000),
    #"Gaussian Process": GaussianProcessRegressor()
}

#%%
# Train and evaluate each model
results = []
for name, model in models.items():
    model.fit(X_train, y_train)
    y_pred = model.predict(X_test)
    residuals = y_test - y_pred
    std_residuals = np.std(residuals)
    mae = mean_absolute_error(y_test, y_pred)
    mse = mean_squared_error(y_test, y_pred)
    rmse = np.sqrt(mse)
    r2 = r2_score(y_test, y_pred) # Remove R² calculation if not needed
    results.append((name, mae, rmse))
    print(f"{name}: MAE={mae:.4f}, RMSE={rmse:.4f}, R2={r2:.4f}, Std Residuals={std_residuals:.4f}")

#%%
# Visualize results
results_df = pd.DataFrame(results, columns=["Model", "MAE", "RMSE"])
results_df.set_index("Model", inplace=True)
results_df.plot(kind="bar", figsize=(10, 6))
plt.title("Model Comparison SPS4")
plt.ylabel("Error Metrics")
plt.xticks(rotation=45)
plt.show()

#%%
