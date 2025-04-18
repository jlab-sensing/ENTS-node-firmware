import math
import matplotlib.pyplot as plt
import re
from protobuf_encoding.encode import encode_adc_measurement  # Import the encoding function
import argparse  # Import argparse for argument parsing
import os
import csv  # For reading the CSV file
from datetime import datetime, timedelta

# Argument Parsing
parser = argparse.ArgumentParser(description="Generate sine wave, logarithmic data, and process voltage data from CSV.")
parser.add_argument(
    "--samples", type=int, required=True,
    help="Number of samples to generate."
)
parser.add_argument(
    "--csv_file", type=str, required=True,
    help="Path to the CSV file containing voltage data."
)
args = parser.parse_args()

# Parameters
frequency = 1  # Hz for sine wave
sample_rate = 100  # Hz
adc_resolution = 32  # 32-bit ADC
adc_min = 0  # Minimum ADC value
adc_max = 2**adc_resolution - 1  # Maximum ADC value
voltage_min = -3.0  # Minimum voltage (-3V)
voltage_max = 3.0  # Maximum voltage (+3V)
OFFSET = 2**31  # Offset for signed 32-bit integer

# Function to perform zigzag encoding
def zigzag_encode(val):
    if val < 0:
        return -2 * val - 1
    return 2 * val

def convert_to_ones_complement(delta, bit_width=32):
    """
    Convert a signed integer delta value from 2's complement to 1's complement if it is negative.

    Args:
        delta (int): The delta value to check and convert.
        bit_width (int): The bit width of the integer (default: 32 bits).

    Returns:
        int: The delta value in 1's complement representation if negative, otherwise unchanged.
    """
    if delta < 0:
        # Take the absolute value of the delta
        abs_delta = abs(delta)

        # Flip all the bits individually
        mask = (1 << bit_width) - 1  # Create a mask for the given bit width (e.g., 32 bits -> 0xFFFFFFFF)
        ones_complement = ~abs_delta & mask  # Invert all bits and apply the mask

        return ones_complement
    return delta

# Helper function to convert voltage to ADC value
def voltage_to_adc(voltage):
    """Convert a voltage to a signed 32-bit ADC value."""
    normalized = (voltage - voltage_min) / (voltage_max - voltage_min)  # Normalize to [0, 1]
    adc_value = int(normalized * (2**31 - 1))  # Scale to signed int32 range
    return adc_value

# Generate time steps based on the number of samples
time = [i / sample_rate for i in range(args.samples)]

# Generate timestamps for sine wave and logarithmic signals
start_time = datetime.now()  # Use the current time as the starting timestamp
sine_timestamps = [start_time + timedelta(seconds=i / sample_rate) for i in range(args.samples)]
logarithmic_timestamps = sine_timestamps  # Same timestamps as sine wave

# Convert timestamps to seconds relative to the first timestamp
sine_time_seconds = [(t - start_time).total_seconds() for t in sine_timestamps]
logarithmic_time_seconds = sine_time_seconds  # Same relative time as sine wave

# Function to process a signal (sine wave, logarithmic, or CSV-based)
def process_signal(signal_name, signal_values):
    messages = []
    serialized_data = bytearray()  # To store serialized Protobuf data for full ADC values
    delta_serialized_data = bytearray()  # To store serialized Protobuf data for delta-encoded values
    previous_adc_value = None  # To store the previous ADC value for delta encoding

    savings = [] # Size in bytes of the savings
    cumulative_savings = [] # Cumulative savings in bytes

    for idx, V in enumerate(signal_values):
        # Convert voltage to ADC value
        adc_value = voltage_to_adc(V)

        # Calculate delta value
        if previous_adc_value is None:
            delta_value = 0  # No delta for the first value
        else:
            delta_value = adc_value - previous_adc_value

        delta_value = convert_to_ones_complement(delta_value)  # Convert to 1's complement if negative

        # Serialize the raw ADC value using Protobuf
        serialized_message = encode_adc_measurement(adc_value)
        serialized_data.extend(serialized_message)

        # Serialize the delta-encoded value using Protobuf
        delta_serialized_message = encode_adc_measurement(delta_value)
        delta_serialized_data.extend(delta_serialized_message)

        savings.append(len(serialized_message) - len(delta_serialized_message))
        cumulative_savings.append(sum(savings[:idx + 1]))

        # Create a human-readable message
        message = f"Raw ADC: {adc_value}, Delta ADC: {delta_value}\n"
        messages.append(message)

        # Update the previous ADC value
        previous_adc_value = adc_value

    # Write human-readable messages to a text file
    with open(f"{signal_name}_raw_adc_data.txt", "w") as f:
        f.writelines(messages)

    # Write serialized Protobuf data for full ADC values to a binary file
    with open(f"{signal_name}_raw_adc_data.bin", "wb") as f:
        f.write(serialized_data)

    # Write serialized Protobuf data for delta-encoded values to a binary file
    with open(f"{signal_name}_delta_adc_data.bin", "wb") as f:
        f.write(delta_serialized_data)

    # Get file sizes in bytes
    raw_adc_size = os.path.getsize(f"{signal_name}_raw_adc_data.bin")
    delta_adc_size = os.path.getsize(f"{signal_name}_delta_adc_data.bin")

    # Calculate compression ratio
    compression_ratio = raw_adc_size / delta_adc_size if delta_adc_size != 0 else float('inf')

    # Print file sizes and compression ratio
    print(f"Size of {signal_name}_raw_adc_data.bin: {raw_adc_size} bytes")
    print(f"Size of {signal_name}_delta_adc_data.bin: {delta_adc_size} bytes")
    print(f"Compression Ratio (raw/delta) for {signal_name}: {compression_ratio:.4f}")

    return messages, serialized_data, delta_serialized_data, cumulative_savings

# Generate sine wave signal
sine_wave = [2 * math.sin(2 * math.pi * frequency * t) for t in time]

# Generate logarithmic signal
logarithmic_signal = [2 * math.log10(t + 1) for t in time]  # Add 1 to avoid log(0)

# Read voltage data from the CSV file
def read_voltage_from_csv(csv_file):
    """
    Read the voltage data and timestamps from the CSV file.

    Args:
        csv_file (str): Path to the CSV file.

    Returns:
        tuple: A tuple containing two lists: timestamps (in seconds) and voltages (in volts).
    """
    timestamps = []
    voltages = []
    with open(csv_file, "r") as f:
        reader = csv.reader(f)
        next(reader)  # Skip the header
        for row in reader:
            try:
                # Parse the timestamp
                timestamp = datetime.strptime(row[0], "%Y-%m-%d %H:%M:%S")
                millivolts = float(row[5])  # Voltage is in the 6th column
                voltage = millivolts / 1000.0  # Convert millivolts to volts

                # Append the data
                timestamps.append(timestamp)
                voltages.append(voltage)
            except (ValueError, IndexError):
                continue  # Skip rows with invalid data

    # Convert timestamps to seconds relative to the first timestamp
    # start_time = timestamps[0]
    # timestamps = [(t - start_time).total_seconds() for t in timestamps]

    return timestamps, voltages


# Process sine wave
sine_messages, sine_serialized_data, sine_delta_serialized_data, sine_cumulative_savings = process_signal("sine_wave", sine_wave)

# Process logarithmic signal
log_messages, log_serialized_data, log_delta_serialized_data, log_cumulative_savings = process_signal("logarithmic_signal", logarithmic_signal)

# Process CSV-based voltage signal
# Read voltage data and timestamps from the CSV file
csv_timestamps, csv_voltages = read_voltage_from_csv(args.csv_file)
csv_messages, csv_serialized_data, csv_delta_serialized_data, csv_cumulative_savings = process_signal("csv_voltage", csv_voltages)
# Parse and plot signals
def parse_and_plot(signal_name, time, messages, cumulative_savings):
    adc_values = []
    delta_values = []

    for msg in messages:
        match = re.match(r"Raw ADC: (\d+), Delta ADC: ([\d\-]+)", msg)
        if match:
            adc, delta = map(int, match.groups())
            adc_values.append(adc)
            delta_values.append(delta)

    # Plot ADC Values
    plt.figure(figsize=(10, 4))
    plt.plot(time[:len(adc_values)], adc_values, color='blue')
    plt.title(f"{signal_name.capitalize()} - Raw ADC Values over Time")
    plt.xlabel("Time (s)")
    plt.ylabel("ADC Value")
    plt.grid(True)
    plt.tight_layout()

    # Plot Delta Values
    plt.figure(figsize=(10, 4))
    plt.plot(time[:len(delta_values)], delta_values, color='green')
    plt.title(f"{signal_name.capitalize()} - Delta ADC Values over Time")
    plt.xlabel("Time (s)")
    plt.ylabel("Delta ADC Value")
    plt.grid(True)
    plt.tight_layout()

    # Plot Cumulative Byte Savings
    plt.figure(figsize=(10, 4))
    plt.plot(time[:len(cumulative_savings)], cumulative_savings, color='orange')
    plt.title(f"{signal_name.capitalize()} - Cumulative Byte Savings over Time")
    plt.xlabel("Time (s)")
    plt.ylabel("Cumulative Savings (Bytes)")
    plt.grid(True)
    plt.tight_layout()

# Plot sine wave results
parse_and_plot("sine_wave", logarithmic_timestamps, sine_messages, sine_cumulative_savings)

# Plot logarithmic signal results
parse_and_plot("logarithmic_signal", sine_timestamps, log_messages, log_cumulative_savings)

# Plot CSV-based voltage signal results
parse_and_plot("csv_voltage", csv_timestamps, csv_messages, csv_cumulative_savings)

plt.show()
