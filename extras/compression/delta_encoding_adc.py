import math
import matplotlib.pyplot as plt
import re
from protobuf_encoding.encode import (
    encode_adc_measurement,
    encode_power_measurement_delta,
)  # Import the encoding function
import argparse  # Import argparse for argument parsing
import os
import csv  # For reading the CSV file
import statistics
from datetime import datetime
import numpy as np  # For numerical operations

# Argument Parsing
parser = argparse.ArgumentParser(description="Process voltage data from CSV.")
parser.add_argument(
    "--csv_file",
    type=str,
    required=True,
    help="Path to the CSV file containing voltage data.",
)
args = parser.parse_args()

# Parameters
adc_resolution = 32  # 32-bit ADC
adc_min = 0  # Minimum ADC value
adc_max = 2**adc_resolution - 1  # Maximum ADC value
voltage_min = -3.0  # Minimum voltage (-3V)
voltage_max = 3.0  # Maximum voltage (+3V)
OFFSET = 2**31  # Offset for signed 32-bit integer


def calculate_energy_consumption(airtime, V=3.7, I=0.02) -> float:
    """
    Calculate energy consumption in Joules based on airtime, voltage, and current.

    Parameters:
        airtime: airtime in  ms
        V: voltage in Volts (default = 3.7V)
        I: current in Amperes (default = 0.02A)

    Returns:
        Energy consumption in Joules
    """
    return airtime * V * I  # Energy = Power * Time (Power = V * I)


def calculate_lorawan_airtime(
    pl_bytes: int,
    overhead_bytes: int = 13,
    sf: int = 7,
    bw: int = 125000,
    cr: int = 1,
    preamble_len: float = 13,
    h: int = 0,
    de: int = 0,
) -> float:
    """
    Calculate LoRaWAN airtime in milliseconds.

    Parameters:
        pl_bytes: total payload size (App payload + ~13B overhead for LoRaWAN)
        overhead_bytes: overhead size in bytes (default = 13)
        sf: Spreading Factor (e.g., 7)
        bw: Bandwidth in Hz (e.g., 125000)
        cr: Coding Rate offset (e.g., 1 for 4/5)
        preamble_len: number of preamble symbols (default = 8)
        h: Header enabled (0 = yes, 1 = no)
        de: Low data rate optimization (1 = yes, 0 = no)

    Returns:
        Airtime in milliseconds
    """
    # Symbol duration in seconds
    t_sym = (2**sf) / bw

    # Add overhead to payload bytes
    pl_bytes += overhead_bytes

    # Payload symbol count
    payload_symb_nb = 8 + max(
        math.ceil((8 * pl_bytes - 4 * sf + 28 + 16 - 20 * h) / (4 * (sf - 2 * de)))
        * (cr + 4),
        0,
    )

    # Total airtime in seconds
    t_preamble = (preamble_len + 4.25) * t_sym
    t_payload = payload_symb_nb * t_sym
    t_air = t_preamble + t_payload

    return t_air * 1000  # convert to milliseconds


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
        mask = (
            1 << bit_width
        ) - 1  # Create a mask for the given bit width (e.g., 32 bits -> 0xFFFFFFFF)
        ones_complement = ~abs_delta & mask  # Invert all bits and apply the mask

        return ones_complement
    return delta


def convert_from_ones_complement(value, bit_width=32):
    """
    Convert a 1's complement representation back to a signed integer (2's complement logic).

    Args:
        value (int): The 1's complement value to convert.
        bit_width (int): The bit width of the integer (default: 32 bits).

    Returns:
        int: The signed integer value.
    """
    sign_bit = 1 << (bit_width - 1)

    if value & sign_bit:
        # It's a negative value in 1's complement -> flip all bits and make negative
        flipped = ~value & ((1 << bit_width) - 1)
        return -flipped
    else:
        return value


# Helper function to convert voltage to ADC value
def voltage_to_adc(voltage):
    """Convert a voltage to a signed 32-bit ADC value."""
    normalized = (voltage - voltage_min) / (
        voltage_max - voltage_min
    )  # Normalize to [0, 1]
    adc_value = int(normalized * (2**31 - 1))  # Scale to signed int32 range
    return adc_value


def current_to_adc(current_microamps):
    # Define the current range in microamps
    current_min_microamps = -800  # -0.8mA
    current_max_microamps = 800  # +0.8mA
    # Normalize the current to the range [0, 1]
    normalized = (current_microamps - current_min_microamps) / (
        current_max_microamps - current_min_microamps
    )
    # Scale to the signed 32-bit ADC range
    adc_value = int(normalized * (2**31 - 1))
    return adc_value


def adc_to_voltage(adc_value, voltage_min, voltage_max):
    """Convert a signed 32-bit ADC value back to voltage."""
    normalized = adc_value / (2**31 - 1)
    voltage = normalized * (voltage_max - voltage_min) + voltage_min
    return voltage


def adc_to_current(adc_value, current_min_microamps=-800, current_max_microamps=800):
    """Convert a signed 32-bit ADC value back to current in microamps."""
    normalized = adc_value / (2**31 - 1)
    current = (
        normalized * (current_max_microamps - current_min_microamps)
        + current_min_microamps
    )
    return current


# Read voltage data from the CSV file
def read_voltage_current_from_csv(csv_file):
    """
    Read the voltage and current data along with timestamps from the CSV file.

    Args:
        csv_file (str): Path to the CSV file.

    Returns:
        tuple: A tuple containing three lists: timestamps (in seconds), voltages (in volts), and currents (in microamps).
    """
    timestamps = []
    voltages = []
    currents = []
    with open(csv_file, "r") as f:
        reader = csv.reader(f)
        next(reader)  # Skip the header
        for row in reader:
            try:
                # Parse the timestamp
                timestamp = datetime.strptime(row[0], "%Y-%m-%d %H:%M:%S")

                # Parse the voltage (6th column) and convert millivolts to volts
                millivolts = float(row[5])
                voltage = millivolts / 1000.0  # Convert millivolts to volts

                # Parse the current (7th column) in microamps
                current_microamps = float(row[6])

                # Append the data
                timestamps.append(timestamp)
                voltages.append(voltage)
                currents.append(current_microamps)
            except (ValueError, IndexError):
                continue  # Skip rows with invalid data

    return timestamps, voltages, currents


def process_signal(signal_name, voltages, currents, timestamps):
    """
    Process the voltage signal to calculate ADC values, delta values, and other metrics.

    Args:
        signal_name (str): Name of the signal being processed.
        voltages (list): List of voltage values in volts.
        currents (list): List of current values in microamps.

    Returns:
        tuple: Messages, serialized data, delta serialized data, cumulative savings, airtime, and delta airtime.
    """
    messages = []
    serialized_data = bytearray()
    delta_serialized_data = bytearray()
    cumulative_savings = []
    adc_airtime = []
    delta_airtime = []
    savings = []
    previous_voltage_adc = None
    previous_current_adc = None
    serialized_sizes = []  # Track sizes of raw serialized packets
    delta_serialized_sizes = []  # Track sizes of delta serialized packets

    for idx, (voltage, current, timestamp) in enumerate(
        zip(voltages, currents, timestamps)
    ):
        # Convert voltage to ADC value
        voltage_adc = voltage_to_adc(voltage)
        current_adc = current_to_adc(current)

        # Calculate voltage delta value
        if previous_voltage_adc is None:
            voltage_delta = voltage_adc  # No delta for the first value
        else:
            voltage_delta = voltage_adc - previous_voltage_adc

        if previous_current_adc is None:
            current_delta = current_adc
        else:
            current_delta = current_adc - previous_current_adc

        voltage_delta = convert_to_ones_complement(voltage_delta)
        current_delta = convert_to_ones_complement(
            current_delta
        )  # Convert to 1's complement if negative

        # Serialize the raw ADC value using Protobuf
        serialized_message = encode_power_measurement_delta(
            int(timestamp.timestamp()), 0, 0, voltage_adc, current_adc
        )
        serialized_data.extend(serialized_message)
        serialized_sizes.append(len(serialized_message))

        # Serialize the delta-encoded value using Protobuf
        delta_serialized_message = encode_power_measurement_delta(
            int(timestamp.timestamp()), 0, 0, voltage_delta, current_delta
        )
        delta_serialized_data.extend(delta_serialized_message)
        delta_serialized_sizes.append(
            len(delta_serialized_message)
        )  # Track size of delta serialized packet

        # Calculate savings in bytes
        savings.append(len(serialized_message) - len(delta_serialized_message))
        cumulative_savings.append(sum(savings))

        # Calculate airtime for raw and delta-encoded data
        adc_airtime.append(calculate_lorawan_airtime(len(serialized_message)))
        delta_airtime.append(calculate_lorawan_airtime(len(delta_serialized_message)))

        # Create a human-readable message
        message = f"Raw V: {voltage_adc}, Raw I: {current_adc}, Delta V: {voltage_delta}, Delta I: {current_delta}\n"
        messages.append(message)

        # Update the previous ADC value
        previous_voltage_adc = voltage_adc
        previous_current_adc = current_adc

    # print(f"Mean size of raw serialized encoded packet: {statistics.mean(serialized_sizes):.2f} bytes")
    # print(f"Mean size of delta serialized encoded packet: {statistics.mean(delta_serialized_sizes):.2f} bytes")
    # print(f"Byte saving statistics")
    # print(f"Mean savings: {statistics.mean(savings):.2f} bytes")
    # print(f"Median savings: {statistics.median(savings):.2f} bytes")
    # print(f"Max savings: {max(savings):.2f} bytes")
    # print(f"Min savings: {min(savings):.2f} bytes")
    return (
        messages,
        serialized_data,
        delta_serialized_data,
        cumulative_savings,
        adc_airtime,
        delta_airtime,
    )


# Parse and plot signals
def parse_and_plot(
    signal_name, time, messages, cumulative_savings, adc_airtime, delta_airtime
):
    adc_v_values = []
    delta_v_values = []
    adc_i_values = []
    delta_i_values = []

    for msg in messages:
        match = re.match(
            r"Raw V: (\d+), Raw I: (\d+), Delta V: (\d+), Delta I: (\d+)", msg
        )
        if match:
            raw_v, raw_i, delta_v, delta_i = map(int, match.groups())
            adc_v_values.append(raw_v)
            delta_v_values.append(delta_v)
            adc_i_values.append(raw_i)
            delta_i_values.append(delta_i)

    delta_v_values = [convert_from_ones_complement(v) for v in delta_v_values]
    delta_i_values = [convert_from_ones_complement(i) for i in delta_i_values]
    delta_v_values = [
        adc_to_voltage(v, voltage_min, voltage_max) for v in delta_v_values
    ]
    delta_i_values = [adc_to_current(i) for i in delta_i_values]

    adc_v_values = [adc_to_voltage(v, voltage_min, voltage_max) for v in adc_v_values]
    adc_i_values = [adc_to_current(i) for i in adc_i_values]

    # Create subplots
    fig, axs = plt.subplots(2, 1, figsize=(10, 8), sharex=True)

    # Plot ADC Values
    # Create a secondary y-axis for current
    ax1 = axs[0]  # main axis for voltage
    ax2 = ax1.twinx()  # twin axis for current

    # Plot Voltage (Raw and Delta) on ax1
    ax1.plot(time[: len(adc_v_values)], adc_v_values, color="blue", label="Voltage")
    # ax1.plot(time[:len(delta_v_values)], delta_v_values, color='red', label='Voltage Delta')
    ax1.set_ylabel("Voltage (V)", color="black")
    ax1.tick_params(axis="y", labelcolor="black")

    # Plot Current (Raw and Delta) on ax2
    ax2.plot(
        time[: len(adc_i_values)],
        adc_i_values,
        color="orange",
        linestyle="--",
        label="Current",
    )
    # ax2.plot(time[:len(delta_i_values)], delta_i_values, color='green', linestyle='--', label='Current Delta')
    ax2.set_ylabel("Current (uA)", color="black")
    ax2.tick_params(axis="y", labelcolor="black")

    # Title and grid
    ax1.set_title(f"{signal_name.capitalize()} - Raw and Delta ADC Values over Time")
    ax1.set_xlabel("Time (s)")
    ax1.grid(True)

    # Combine legends from both axes
    lines_1, labels_1 = ax1.get_legend_handles_labels()
    lines_2, labels_2 = ax2.get_legend_handles_labels()
    axs[0].legend(lines_1 + lines_2, labels_1 + labels_2, loc="upper left")

    # Plot Cumulative Byte Savings
    axs[1].plot(time[: len(cumulative_savings)], cumulative_savings, color="orange")
    axs[1].set_title(f"{signal_name.capitalize()} - Cumulative Byte Savings over Time")
    axs[1].set_xlabel("Time")
    axs[1].set_ylabel("Cumulative Savings (Bytes)")
    axs[1].grid(True)

    #

    # Plot LoRaWAN Airtime
    fig2, axs2 = plt.subplots(2, 1, figsize=(10, 8), sharex=True)
    axs2[0].plot(time[: len(adc_airtime)], adc_airtime, color="green")
    axs2[0].set_title(f"{signal_name.capitalize()} - Raw ADC Airtime over Time")
    axs2[0].set_ylabel("Airtime (ms)")
    axs2[0].grid(True)

    axs2[1].plot(time[: len(delta_airtime)], delta_airtime, color="red")
    axs2[1].set_title(f"{signal_name.capitalize()} - Delta ADC Airtime over Time")
    axs2[1].set_xlabel("Time")
    axs2[1].set_ylabel("Airtime (ms)")
    axs2[1].grid(True)

    # Find total energy usage
    total_energy_raw = sum(calculate_energy_consumption(t / 1000) for t in adc_airtime)
    total_energy_delta = sum(
        calculate_energy_consumption(t / 1000) for t in delta_airtime
    )

    # print(f"Total energy usage for {signal_name} (Raw): {total_energy_raw:.2f} J")
    # print(f"Total energy usage for {signal_name} (Delta): {total_energy_delta:.2f} J")
    # print(f"Total energy savings for {signal_name}: {total_energy_raw - total_energy_delta:.2f} J")
    # print(f"Mean LoRaWAN airtime for {signal_name} (Raw ADC): {sum(adc_airtime) / len(adc_airtime):.2f} ms")
    # print(f"Mean LoRaWAN airtime for {signal_name} (Delta ADC): {sum(delta_airtime) / len(delta_airtime):.2f} ms")

    # Adjust layout
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    # Check if the CSV file exists
    if not os.path.exists(args.csv_file):
        print(f"Error: The file {args.csv_file} does not exist.")
        exit(1)

    # Read and process the voltage signal from the CSV file
    print(f"Processing voltage data from {args.csv_file}...")
    # Process CSV-based voltage signal
    csv_timestamps, csv_voltages, csv_currents = read_voltage_current_from_csv(
        args.csv_file
    )
    (
        csv_messages,
        csv_serialized_data,
        csv_delta_serialized_data,
        csv_cumulative_savings,
        adc_airtime,
        delta_airtime,
    ) = process_signal("voltage", csv_voltages, csv_currents, csv_timestamps)

    # Plot CSV-based voltage signal results
    parse_and_plot(
        "MFC voltage",
        csv_timestamps,
        csv_messages,
        csv_cumulative_savings,
        adc_airtime,
        delta_airtime,
    )

    # Plot cost in joules per a byte of payload
    demo_bytes = list(range(0, 256))
    airtime_costs = [calculate_lorawan_airtime(b) for b in demo_bytes]
    energy_costs = [calculate_energy_consumption(a / 1000) for a in airtime_costs]
    plt.figure(figsize=(10, 5))
