import math
import matplotlib.pyplot as plt
import argparse

# ADC Parameters
adc_resolution = 32  # 32-bit ADC
adc_min = 0  # Minimum ADC value
adc_max = 2**adc_resolution - 1  # Maximum ADC value
voltage_min = -3.0  # Minimum voltage (-3V)
voltage_max = 3.0  # Maximum voltage (+3V)
OFFSET = 2**31  # Offset for signed 32-bit integer

# Helper function to convert voltage to ADC value
def voltage_to_adc(voltage):
    """Convert a voltage to a signed 32-bit ADC value."""
    normalized = (voltage - voltage_min) / (voltage_max - voltage_min)  # Normalize to [0, 1]
    adc_value = int(normalized * (2**31 - 1))  # Scale to signed int32 range
    return adc_value

def generate_voltage_signal(frequency=1, sample_rate=100, duration=1):
    """
    Generate a sine wave voltage signal.

    Args:
        frequency (float): Frequency of the sine wave in Hz.
        sample_rate (int): Number of samples per second.
        duration (float): Duration of the signal in seconds.

    Returns:
        tuple: A tuple containing the time steps and the corresponding voltage values.
    """
    time = [i / sample_rate for i in range(int(sample_rate * duration))]
    voltage = [2 * math.sin(2 * math.pi * frequency * t) for t in time]
    return time, voltage

def plot_voltage_signal(time, voltage):
    """
    Plot the voltage signal.

    Args:
        time (list): List of time steps.
        voltage (list): List of voltage values.
    """
    plt.figure(figsize=(10, 4))
    plt.plot(time, voltage, color='blue')
    plt.title("Voltage Signal")
    plt.xlabel("Time (s)")
    plt.ylabel("Voltage (V)")
    plt.grid(True)
    plt.tight_layout()
    plt.show()

def write_voltage_to_file(time, voltage, output_file):
    """
    Write the voltage signal to a text file.

    Args:
        time (list): List of time steps.
        voltage (list): List of voltage values.
        output_file (str): Path to the output text file.
    """
    with open(output_file, "w") as f:
        f.write("Time (s), Voltage (V)\n")
        for t, v in zip(time, voltage):
            f.write(f"{t:.6f}, {v}\n")
    print(f"Voltage signal written to {output_file}")

# Argument Parsing
parser = argparse.ArgumentParser(description="Generate and plot a sine wave voltage signal.")
parser.add_argument("--frequency", type=float, default=1.0, help="Frequency of the sine wave in Hz (default: 1 Hz)")
parser.add_argument("--sample_rate", type=int, default=100, help="Number of samples per second (default: 100 Hz)")
parser.add_argument("--duration", type=float, default=1.0, help="Duration of the signal in seconds (default: 1 second)")
parser.add_argument("--output_file", type=str, default="voltage_signal.txt", help="Output file to save the voltage signal (default: voltage_signal.txt)")
args = parser.parse_args()

# Generate and plot the voltage signal
time, voltage = generate_voltage_signal(args.frequency, args.sample_rate, args.duration)

# Convert the voltage signal to ADC values
adc_values = [voltage_to_adc(v) for v in voltage]

# Plot the ADC signal
plot_voltage_signal(time, adc_values)

# Write the ADC signal to a text file
write_voltage_to_file(time, adc_values, args.output_file)