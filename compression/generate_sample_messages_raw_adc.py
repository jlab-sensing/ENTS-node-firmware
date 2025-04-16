import math
import matplotlib.pyplot as plt
import re
from protobuf_encoding.encode import encode_adc_measurement  # Import the encoding function
import time as time_module
import argparse  # Import argparse for argument parsing

# Argument Parsing
parser = argparse.ArgumentParser(description="Generate sine wave data and serialize raw ADC values with delta encoding.")
parser.add_argument(
    "--samples", type=int, required=True,
    help="Number of samples to generate."
)
args = parser.parse_args()

# Parameters
frequency = 1  # Hz
sample_rate = 100  # Hz
current_timestamp = int(time_module.time())  # Use current time as the base timestamp

# ADC Parameters
adc_resolution = 32  # 32-bit ADC
adc_min = 0  # Minimum ADC value
adc_max = 2**adc_resolution - 1  # Maximum ADC value
voltage_min = -3.0  # Minimum voltage (-3V)
voltage_max = 3.0  # Maximum voltage (+3V)

# Helper function to convert voltage to ADC value
def voltage_to_adc(voltage):
    """Convert a voltage to a 32-bit ADC value."""
    normalized = (voltage - voltage_min) / (voltage_max - voltage_min)  # Normalize to [0, 1]
    adc_value = int(normalized * adc_max)  # Scale to ADC range
    return adc_value

# Generate time steps based on the number of samples
time = [i / sample_rate for i in range(args.samples)]

# Simulate sine waves and generate serialized messages
messages = []
serialized_data = bytearray()  # To store serialized Protobuf data for full ADC values
delta_serialized_data = bytearray()  # To store serialized Protobuf data for delta-encoded values

previous_adc_value = None  # To store the previous ADC value for delta encoding

for idx, t in enumerate(time):
    # Generate sine wave voltage
    V = 2 * math.sin(2 * math.pi * frequency * t)  # Voltage in volts

    # Convert voltage to ADC value
    adc_value = voltage_to_adc(V)

    # Calculate delta value
    if previous_adc_value is None:
        delta_value = 0  # No delta for the first value
    else:
        delta_value = adc_value - previous_adc_value

    # Serialize the raw ADC value using Protobuf
    serialized_message = encode_adc_measurement(adc_value)
    serialized_data.extend(serialized_message)

    # Serialize the delta-encoded value using Protobuf
    delta_serialized_message = encode_adc_measurement(delta_value)
    delta_serialized_data.extend(delta_serialized_message)

    # Create a human-readable message
    message = f"Raw ADC: {adc_value}, Delta ADC: {delta_value}\n"
    messages.append(message)

    # Update the previous ADC value
    previous_adc_value = adc_value

# Write human-readable messages to a text file
with open("raw_adc_data.txt", "w") as f:
    f.writelines(messages)

# Write serialized Protobuf data for full ADC values to a binary file
with open("raw_adc_data.bin", "wb") as f:
    f.write(serialized_data)

# Write serialized Protobuf data for delta-encoded values to a binary file
with open("delta_adc_data.bin", "wb") as f:
    f.write(delta_serialized_data)

# Parse messages
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
plt.title("Raw ADC Values over Time")
plt.xlabel("Time (s)")
plt.ylabel("ADC Value")
plt.grid(True)
plt.tight_layout()

# Plot Delta Values
plt.figure(figsize=(10, 4))
plt.plot(time[:len(delta_values)], delta_values, color='green')
plt.title("Delta ADC Values over Time")
plt.xlabel("Time (s)")
plt.ylabel("Delta ADC Value")
plt.grid(True)
plt.tight_layout()

plt.show()
