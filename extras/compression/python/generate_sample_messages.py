import math
import matplotlib.pyplot as plt
import re
from protobuf_encoding.encode import (
    encode_power_measurement,
)  # Import the encoding function
import time as time_module
import argparse  # Import argparse for argument parsing

# Argument Parsing
parser = argparse.ArgumentParser(
    description="Generate sine wave data and serialize it."
)
parser.add_argument(
    "--bytes",
    type=int,
    required=True,
    help="Maximum number of bytes for the generated data.",
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


max_samples = (
    args.bytes // sample_size_estimate
)  # Calculate the maximum number of samples
time = [i / sample_rate for i in range(max_samples)]  # Generate time steps

# Simulate sine waves and generate messages
messages = []
serialized_data = bytearray()  # To store serialized protobuf data

for idx, t in enumerate(time):
    V = 2 * math.sin(2 * math.pi * frequency * t)
    I = 0.0009 * math.sin(
        2 * math.pi * frequency * t + math.pi / 6
    )  # small phase shift
    P = V * I
    message = f"V: {V:.4f}, I: {I:.6f}, P: {P:.8f}\n"
    messages.append(message)

    # Serialize the data using encode_power_measurement
    serialized_message = encode_power_measurement(
        ts=current_timestamp + idx,  # Increment timestamp for each sample
        cell_id=1,  # Example cell ID
        logger_id=1,  # Example logger ID
        voltage=V,
        current=I,
    )
    serialized_data.extend(serialized_message)

    # Stop if the serialized data exceeds the byte limit
    if len(serialized_data) > args.bytes:
        break

# Write messages to a text file
with open("power_data.txt", "w") as f:
    f.writelines(messages)

# Write serialized protobuf data to a binary file
with open("power_data.bin", "wb") as f:
    f.write(serialized_data)

# Parse messages
voltages = []
currents = []
powers = []

for msg in messages:
    match = re.match(r"V: ([\d\.\-e]+), I: ([\d\.\-e]+), P: ([\d\.\-e]+)", msg)
    if match:
        v, i, p = map(float, match.groups())
        voltages.append(v)
        currents.append(i)
        powers.append(p)

# Plot Voltage
plt.figure(figsize=(10, 4))
plt.plot(time[: len(voltages)], voltages, color="blue")
plt.title("Voltage over Time")
plt.xlabel("Time (s)")
plt.ylabel("Voltage (V)")
plt.grid(True)
plt.tight_layout()

# Plot Current
plt.figure(figsize=(10, 4))
plt.plot(time[: len(currents)], currents, color="green")
plt.title("Current over Time")
plt.xlabel("Time (s)")
plt.ylabel("Current (A)")
plt.grid(True)
plt.tight_layout()

# Plot Power
plt.figure(figsize=(10, 4))
plt.plot(time[: len(powers)], powers, color="red")
plt.title("Power over Time")
plt.xlabel("Time (s)")
plt.ylabel("Power (W)")
plt.grid(True)
plt.tight_layout()

plt.show()
