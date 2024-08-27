"""SPS V1 Calibration and Evaluation, for postierity

John Madden, Stephen Taylor
"""
import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from rocketlogger.data import RocketLoggerData
from sklearn import linear_model
import yaml
try:
    from yaml import CLoader as Loader
except ImportError:
    from yaml import Loader

def load_data(cfg, datafiles):
    df_list = []
    for d in datafiles:
        df = pd.read_csv(d)
        df_list.append(df)
    
    data = pd.concat(df_list, ignore_index=True)
    data = data.set_index("V")

    data["I_in"] = data["I_in"] * 1e6
    data["I_meas"] = data["V_i"] / cfg["r_sense"] / cfg["gain"] * 1e6
    data["V_meas"] = data["V_2x"]
    
    return data

def load_rld(path, channel, start, stop, step):
    rld = RocketLoggerData(path)
    
    source_range = np.arange(start, stop, step)
    
    def estimate_input(x):
        diff = np.abs(source_range - x)
        idx = np.argmin(diff)
        return source_range[idx]

    rld_df = pd.DataFrame(rld.get_data(), columns=rld.get_channel_names())
    rld_df["source"] = rld_df[channel].apply(estimate_input)
    rld_df["input"] = rld_df["source"]
    rld_df = rld_df.set_index("source")
    return rld_df

cfg_path = "data/config.yaml"
datafiles = ["data/sps3_temp_2022-10-14T09_10_50-07_00.csv"] # ':' not a valid file path in windows or mac
evalfiles = ["data/sps3_temp_2022-10-15T18_37_56-07_00.csv"]

with open(cfg_path, "r") as f:
    cfg = yaml.load(f, Loader=Loader)

data = load_data(cfg, datafiles)

v_input_cols = ["V_meas"]
i_input_cols = ["I_meas"]

v_model = linear_model.LinearRegression()
v_model.fit(data[v_input_cols], data["V_in"])

i_model = linear_model.LinearRegression()
i_model.fit(data[i_input_cols], data["I_in"])

eval_data = load_data(cfg, evalfiles)

eval_data["V_fit"] = v_model.predict(eval_data[v_input_cols])
eval_data["I_fit"] = i_model.predict(eval_data[i_input_cols])

rld_v = load_rld("data/rl/20220826_200050_eval_v.rld", "V1", 0.1, 1, 0.05)
rld_i = load_rld("data/rl/20220826_200050_eval_i.rld", "I1L", 45e-6, 454e-6, 22.7222e-6)
rld_i.index = rld_i.index * 1e6
rld_i["input"] = rld_i["input"] * 1e6
rld_i["I1L"] = rld_i["I1L"] * 1e6

def plot_accuracy(data):
    fig, ax = plt.subplots()
    
    for (x, y) in data:
        rel = y / x
        rel = rel.groupby(level=0).describe()
        ax.errorbar(x.groupby(level=0).mean(), rel["mean"], yerr=rel["std"])
        
    ax.axhline(y=1, linestyle=':', color='k', linewidth='3')
    ax.grid()
    
    return (fig, ax)

fig, ax = plot_accuracy([
    (data["V_in"], data["V_meas"]),
])

ax.set_title("Voltage Channel Accuracy Raw")
ax.set_xlabel("Input Voltage (V)")
ax.set_ylabel("Relative Measurement (V/V)")

fig, ax = plot_accuracy([
    (data["I_in"], data["I_meas"]),
])

ax.set_title("Current Channel Accuracy Raw")
ax.set_xlabel("Input Current (uA)")
ax.set_ylabel("Relative Measurement (A/A)")

fig, ax = plt.subplots()
ax.axhline(y=0, linestyle=':', color='r', linewidth='2')
ax.scatter(data["V_in"], (data["V_meas"] - data["V_in"]) * 1e3)
ax.set_title("Error between Measured and Sourced Voltage")
ax.set_xlabel("Input Voltage (V)")
ax.set_ylabel("Error (mV)")
ax.legend(["Optimal", "Measured"])
ax.grid(True)

fig, ax = plt.subplots()
ax.axhline(y=0, linestyle=':', color='r', linewidth='2')
ax.scatter(data["I_in"], data["I_meas"] - data["I_in"])
ax.set_title("Error between Measured and Sourced Current")
ax.set_xlabel("Input Voltage (uA)")
ax.set_ylabel("Error (uA)")
ax.legend(["Optimal", "Measured"])
ax.grid(True)

fig, ax = plot_accuracy([
    (eval_data["V_in"], eval_data["V_fit"]),
    (rld_v["input"], rld_v["V1"]),
])

ax.set_title("Voltage Channel Accuracy Calibrated")
ax.set_xlabel("Input Voltage (V)")
ax.set_ylabel("Relative Measurement (V/V)")
ax.legend(["Idea", "SPS", "RocketLogger"])

fig, ax = plot_accuracy([
    (eval_data["I_in"], eval_data["I_fit"]),
    (rld_i["input"], rld_i["I1L"]),
])

ax.set_title("Current Channel Accuracy Calibrated")
ax.set_xlabel("Input Current (uA)")
ax.set_ylabel("Relative Measurement (A/A)")
ax.set_ylim(0.99, 1.06)
ax.legend(["Ideal", "SPS", "RocketLogger"])

print("SPS Voltage Accuracy")
mae_v = abs(eval_data["V_in"] - eval_data["V_fit"]) / eval_data["V_in"]
print(mae_v.describe())
print()

print("SPS Current Accuracy")
mae_i = abs(eval_data["I_in"] - eval_data["I_fit"]) / eval_data["I_in"]
print(mae_i.describe())
print()

print("Rocketlogger Voltage Accuracy")
rl_mae_v = abs(rld_i["input"] - rld_i["I1L"]) / rld_i["input"]
print(rl_mae_v.describe())
print()

print("Rocketlogger Current Accuracy")
rl_mae_i = abs(rld_v["input"] - rld_v["V1"]) / rld_v["input"]
print(rl_mae_i.describe())
print()
