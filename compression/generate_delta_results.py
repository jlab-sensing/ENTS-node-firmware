import math
import matplotlib.pyplot as plt
import re
from protobuf_encoding.encode import encode_power_measurement  # Import the encoding function
import time as time_module
import argparse  # Import argparse for argument parsing

# Argument Parsing
parser = argparse.ArgumentParser(description="Generate sine wave data with delta encoding and serialize it.")
parser.add_argument(
    "--bytes", type=int, required=True,
    help="Maximum number of bytes for the generated data."
)
args = parser.parse_args()

# Parameters
frequency = 1  # Hz
sample_rate = 100  # Hz
current_timestamp = int(time_module.time())  # Use current time as the base timestamp

# Measure the size of a single serialized message
test_message = encode_power_measurement(
    ts=current_timestamp,
    cell_id=1,
    logger_id=1,
    voltage=2.0,
    current=0.0009,
)
sample_size_estimate = len(test_message)
print(f"Estimated size of one serialized message: {sample_size_estimate} bytes")

max_samples = args.bytes // sample_size_estimate  # Calculate the maximum number of samples
time = [i / sample_rate for i in range(max_samples)]  # Generate time steps

# Simulate sine waves and generate delta-encoded messages
messages = []
serialized_data = bytearray()  # To store serialized protobuf data

voltages = []
currents = []
delta_voltages = []
delta_currents = []

for idx, t in enumerate(time):
    V = 2 * math.sin(2 * math.pi * frequency * t)
    I = 0.0009 * math.sin(2 * math.pi * frequency * t + math.pi / 6)  # small phase shift

    # Apply delta encoding
    if idx == 0:
        delta_V = V
        delta_I = I
    else:
        delta_V = V - voltages[-1]
        delta_I = I - currents[-1]

    voltages.append(V)
    currents.append(I)
    delta_voltages.append(delta_V)
    delta_currents.append(delta_I)

    # Create a human-readable message
    message = f"Delta V: {delta_V:.4f}, Delta I: {delta_I:.6f}\n"
    messages.append(message)

    # Serialize the delta-encoded data using encode_power_measurement
    serialized_message = encode_power_measurement(
        ts=current_timestamp + idx,  # Increment timestamp for each sample
        cell_id=1,  # Example cell ID
        logger_id=1,  # Example logger ID
        voltage=delta_V,
        current=delta_I,
    )
    serialized_data.extend(serialized_message)

    # Stop if the serialized data exceeds the byte limit
    if len(serialized_data) > args.bytes:
        break

# Write delta-encoded messages to a text file
with open("delta_power_data.txt", "w") as f:
    f.writelines(messages)

# Write serialized delta-encoded protobuf data to a binary file
with open("delta_power_data.bin", "wb") as f:
    f.write(serialized_data)

# Plot Delta Voltage
plt.figure(figsize=(10, 4))
plt.plot(time[:len(delta_voltages)], delta_voltages, color='blue')
plt.title("Delta Voltage over Time")
plt.xlabel("Time (s)")
plt.ylabel("Delta Voltage (V)")
plt.grid(True)
plt.tight_layout()

# Plot Delta Current
plt.figure(figsize=(10, 4))
plt.plot(time[:len(delta_currents)], delta_currents, color='green')
plt.title("Delta Current over Time")
plt.xlabel("Time (s)")
plt.ylabel("Delta Current (A)")
plt.grid(True)
plt.tight_layout()

plt.show()