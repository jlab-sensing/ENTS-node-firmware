import sys
import os
import csv
import matplotlib.pyplot as plt
import pandas as pd

# Add the rocketlogger module path
sys.path.append(os.path.abspath(r'C:\Users\Steph\Documents\GitRepos\RocketLogger\script\python'))

from rocketlogger.data import RocketLoggerData

def plot_rld(input_file, output_file):
    r = RocketLoggerData(input_file)
    r.merge_channels()

    # Plot (optional)
    r.plot(["voltages", "currents"])
    plt.show()


def rld_to_csv(input_file, output_file):
    """Convert RocketLogger .rld file to CSV format."""
    r = RocketLoggerData(input_file)
    r.merge_channels()

    channel_1 = r.get_data(["V1", "I1"])
    time_1 = r.get_time()

    with open(output_file, "w", newline="") as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["time", "V1", "I1"])  # FIXED: write column names, not data

        for i in range(len(time_1)):
            row = [time_1[i]] + list(channel_1[i])
            writer.writerow(row)

def extract_spike_from_csv(input_csv, output_csv, current_col='I1', time_col='time', threshold=0.36, left_window=0.1, right_window=0.1):
    df = pd.read_csv(input_csv)

    # Sampling interval (assumes uniform sampling)
    time_diffs = df[time_col].diff().dropna()
    sampling_interval = time_diffs.median()
    window_s = left_window + right_window
    samples_in_window = int(window_s / sampling_interval)
    left_samples = int(left_window / sampling_interval)
    right_samples = int(right_window / sampling_interval)

    # Find spike
    spike_indices = df.index[df[current_col] > threshold].tolist()
    if not spike_indices:
        print("No spike found above threshold.")
        return

    spike_index = spike_indices[0]
    start_idx = max(spike_index - left_samples, 0)
    end_idx = min(spike_index + right_samples, len(df))
    print(f"🏔️  Spike detected at index {spike_index}, at time {df[time_col].iloc[spike_index]}s")

    spike_window = df.iloc[start_idx:end_idx]
    spike_window.to_csv(output_csv, index=False)

    print(f"Extracted spike window saved) to {output_csv}")

def analyze_spike(csv_file, current_col='I1', time_col='time'):
    # Load CSV
    df = pd.read_csv(csv_file)

    # Calculate average current
    avg_current = df[current_col].mean()

    # Calculate total time
    total_time = df[time_col].iloc[-1] - df[time_col].iloc[0]
    # Then detect steep drops
    df = pd.read_csv(csv_file)
    drop_times = detect_steep_changes(df, current_col='I1', time_col='time', drop_threshold=-50, rise_threshold=50)

    calculate_power_and_energy(df, current_col='I1', voltage_col='V1', time_col='time')

    # Print results
    print("📉 Steep changes detected at times (s):", drop_times)
    print(f"🔋 Average current during spike: {avg_current:.6f} A")
    print(f"⏱️  Total duration: {total_time:.6f} s")
    print(f"⚡  Total energy consumed during spike: {df['Energy'].sum():.6f} J")


    # Plot spike
    plt.figure(figsize=(10, 4))
    plt.plot(df[time_col], df[current_col], label='Current (A)', color='blue')
    plt.title('Current Spike Analysis')
    plt.xlabel('Time [s]')
    plt.ylabel('Current [A]')
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.show()

    

def detect_steep_changes(df, current_col='I1', time_col='time', drop_threshold=-50, rise_threshold=50):
    # Calculate the current difference and time difference
    dI = df[current_col].diff()
    dt = df[time_col].diff()
    
    # Compute derivative (A/s)
    dIdt = dI / dt
    
    # Find where slope is below threshold
    drop_indices = dIdt[(dIdt < drop_threshold) | (dIdt > rise_threshold)].index
    
    drop_times = df.loc[drop_indices, time_col].values.tolist()
    
    return drop_times

def calculate_power_and_energy(df, current_col='I1', voltage_col='V1', time_col='time'):
    # Calculate power (P = V * I)
    df['Power'] = df[voltage_col] * df[current_col]

    # Calculate energy (E = P * dt)
    df['Energy'] = df['Power'] * df[time_col].diff().fillna(0)

    # Sum energy
    total_energy = df['Energy'].sum()

    print(f"Total Energy: {total_energy:.6f} J")
    return df[['time', 'Power', 'Energy']]

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python rld_to_csv.py <input.rld> <output.csv>")
    else:
        plot_rld(sys.argv[1], sys.argv[2])
        rld_to_csv(sys.argv[1], sys.argv[2])  
        extract_spike_from_csv(sys.argv[2], "spike_window.csv", current_col='I1', time_col='time', threshold=0.3, left_window=0.28, right_window=0.3)
        analyze_spike("spike_window.csv", current_col='I1', time_col='time')
        
