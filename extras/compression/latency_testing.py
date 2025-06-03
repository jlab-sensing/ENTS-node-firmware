from protobuf_encoding.encode import encode_repeated_power_deltas
from delta_encoding_adc import (
    read_voltage_current_from_csv,
    process_signal,
    calculate_lorawan_airtime,
    calculate_energy_consumption,
    convert_to_ones_complement,
    convert_from_ones_complement,
    adc_to_voltage,
    adc_to_current,
)
import numpy as np
import statistics
import matplotlib.pyplot as plt

# Parameters
adc_resolution = 32  # 32-bit ADC
adc_min = 0  # Minimum ADC value
adc_max = 2**adc_resolution - 1  # Maximum ADC value
voltage_min = -3.0  # Minimum voltage (-3V)
voltage_max = 3.0  # Maximum voltage (+3V)
OFFSET = 2**31  # Offset for signed 32-bit integer


class BufferAnalysisData:
    def __init__(self, buffer_size):
        self.buffer_size = buffer_size
        self.raw_airtimes = []
        self.raw_energys = []
        self.delta_airtimes = []
        self.delta_energys = []
        self.raw_payload_sizes = []
        self.delta_payload_sizes = []
        self.delta_cumulative_power_savings = []

    def add_raw_data(self, airtime, energy, payload_size):
        self.raw_airtimes.append(airtime)
        self.raw_energys.append(energy)
        self.raw_payload_sizes.append(payload_size)

    def add_delta_data(self, airtime, energy, payload_size):
        self.delta_airtimes.append(airtime)
        self.delta_energys.append(energy)
        self.delta_payload_sizes.append(payload_size)

    def add_delta_cumulative_data(self, cumulative_power_savings):
        self.delta_cumulative_power_savings.append(cumulative_power_savings)


def plot_energy_savings_vs_buffer_size(
    buffer_sizes: list, raw_energies: list, delta_energies: list
):
    """
    Plots the energy savings percentage versus buffer size.

    Args:
        buffer_sizes (list): List of buffer sizes tested.
        raw_energies (list): List of total energy consumed using raw ADC data.
        delta_energies (list): List of total energy consumed using delta encoding.
    """
    # Compute percent energy savings
    energy_savings_percent = [
        100 * (raw - delta) / raw if raw != 0 else 0
        for raw, delta in zip(raw_energies, delta_energies)
    ]

    # Plot
    plt.figure(figsize=(8, 5))
    plt.plot(
        buffer_sizes, energy_savings_percent, marker="o", linestyle="-", color="green"
    )
    plt.title("Energy Savings Using Delta Encoding vs Raw")
    plt.xlabel("Buffer Size (Number of Measurements per Transmission)")
    plt.ylabel("Energy Saved (%)")
    plt.grid(True)
    plt.tight_layout()


def plot_raw_vs_delta_energy(buffer_sizes, raw_energies, delta_energies):
    """
    Plots the energy consumption of raw and delta encoded data against buffer sizes.
    Args:
        buffer_sizes (list): List of buffer sizes tested.
        raw_energies (list): List of total energy consumed using raw ADC data.
        delta_energies (list): List of total energy consumed using delta encoding.
    """
    plt.figure(figsize=(8, 5))
    plt.plot(buffer_sizes, raw_energies, label="Raw", marker="o", color="blue")
    plt.plot(
        buffer_sizes, delta_energies, label="Delta Encoded", marker="s", color="green"
    )
    plt.title("Energy Consumption vs Buffer Size")
    plt.xlabel("Buffer Size (Number of Measurements per Transmission)")
    plt.ylabel("Total Energy (Joules)")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()


def plot_raw_vs_delta_airtime(
    buffer_sizes,
    raw_airtimes,
    delta_airtimes,
    raw_airtimes_min,
    raw_airtimes_max,
    delta_airtimes_min,
    delta_airtimes_max,
):
    """
    Plots the airtime of raw and delta encoded data against buffer sizes with error bars for min/max values.
    Args:
        buffer_sizes (list): List of buffer sizes tested.
        raw_airtimes (list): List of total airtime using raw ADC data.
        delta_airtimes (list): List of total airtime using delta encoding.
        raw_airtimes_min (list): List of minimum airtime values for raw ADC data.
        raw_airtimes_max (list): List of maximum airtime values for raw ADC data.
        delta_airtimes_min (list): List of minimum airtime values for delta encoded data.
        delta_airtimes_max (list): List of maximum airtime values for delta encoded data.
    """
    plt.figure(figsize=(8, 5))

    # plt.axhline(y=390.1, color='red', linestyle='--', label='SF9 Max Airtime (390.1 ms)')
    plt.axhline(
        y=368.9, color="orange", linestyle="--", label="SF7 Max Airtime (368.9 ms)"
    )

    # Plot raw airtime with error bars (min and max)
    plt.errorbar(
        buffer_sizes,
        raw_airtimes,
        yerr=[
            np.array(raw_airtimes) - np.array(raw_airtimes_min),
            np.array(raw_airtimes_max) - np.array(raw_airtimes),
        ],
        label="Raw ADC",
        fmt="o",
        color="blue",
        capsize=5,
        elinewidth=2,
    )

    # Plot delta-encoded airtime with error bars (min and max)
    plt.errorbar(
        buffer_sizes,
        delta_airtimes,
        yerr=[
            np.array(delta_airtimes) - np.array(delta_airtimes_min),
            np.array(delta_airtimes_max) - np.array(delta_airtimes),
        ],
        label="Delta Encoded",
        fmt="s",
        color="green",
        capsize=5,
        elinewidth=2,
    )

    plt.title("Total Airtime vs Buffer Size with Min and Max Airtime")
    plt.xlabel("Buffer Size (Number of Measurements per Transmission)")
    plt.ylabel("Total Airtime (ms)")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()


def plot_adc_readings_and_savings(
    adc_v_values,
    adc_i_values,
    timestamps,
    buffer_results,
):
    """
    Plots V and I over time on a dual-axis plot,
    along with cumulative energy savings over time for each buffer size.
    """
    fig, axs = plt.subplots(2, 1, figsize=(10, 8), sharex=True)

    # Convert timestamps to days since start
    time_days = np.array(
        [(ts - timestamps[0]).total_seconds() / 86400 for ts in timestamps]
    )

    # --- TOP PLOT: Voltage and Current over time ---
    ax1 = axs[0]  # main axis for voltage
    ax2 = ax1.twinx()  # twin axis for current

    ax1.plot(
        time_days[: len(adc_v_values)], adc_v_values, color="blue", label="Voltage"
    )
    ax2.plot(
        time_days[: len(adc_i_values)],
        adc_i_values,
        color="orange",
        linestyle="--",
        label="Current",
    )

    ax1.set_ylabel("Voltage (V)", color="black")
    ax2.set_ylabel("Current (uA)", color="black")
    ax1.tick_params(axis="y", labelcolor="black")
    ax2.tick_params(axis="y", labelcolor="black")
    ax1.set_title(" V and I of SMFC Values over Time")
    ax1.set_xlabel("Time (days)")
    ax1.grid(True)

    lines_1, labels_1 = ax1.get_legend_handles_labels()
    lines_2, labels_2 = ax2.get_legend_handles_labels()
    axs[0].legend(lines_1 + lines_2, labels_1 + labels_2, loc="upper left")

    # --- BOTTOM PLOT: Cumulative energy savings ---
    for buffer_size, data in buffer_results.items():
        raw_cumulative = np.cumsum(data.raw_energys)
        delta_cumulative = np.cumsum(data.delta_energys)
        savings_cumulative = raw_cumulative - delta_cumulative

        transmission_indices = np.array(
            [i * buffer_size for i in range(len(data.delta_energys))]
        )
        transmission_days = [
            time_days[min(idx, len(time_days) - 1)] for idx in transmission_indices
        ]
        axs[1].plot(
            transmission_days, savings_cumulative, label=f"{buffer_size} Buffer Size"
        )

    axs[1].set_title(
        "Cumulative Energy Savings From Delta Encoding at Different Latency Buffer Sizes"
    )
    axs[1].set_xlabel("Time (days)")
    axs[1].set_ylabel("Energy Saved (Joules)")
    axs[1].grid(True)
    axs[1].legend(title="Buffer Size", loc="upper left")

    plt.tight_layout()


def plot_airtime_for_bytes(bytes):
    """
    Given a list of airtimes in milliseconds and representative ADC values
    for voltage and current during transmission, compute energy used (in joules)
    for each airtime using existing conversion functions.

    Parameters:
    - bytes: List of bytes

    """
    bytes_filtered_sf7 = []
    airtimes_sf7 = []
    bytes_filtered_sf9 = []
    airtimes_sf9 = []

    for b in bytes:
        airtime_sf7 = calculate_lorawan_airtime(b, sf=7)
        if airtime_sf7 < 400:
            bytes_filtered_sf7.append(b)
            airtimes_sf7.append(airtime_sf7)
        airtime_sf9 = calculate_lorawan_airtime(b, sf=9)
        if airtime_sf9 < 400:
            bytes_filtered_sf9.append(b)
            airtimes_sf9.append(airtime_sf9)

    plt.figure(figsize=(8, 5))
    plt.plot(bytes_filtered_sf7, airtimes_sf7, color="green", label="SF7 Airtime")
    plt.plot(bytes_filtered_sf9, airtimes_sf9, color="blue", label="SF9 Airtime")
    plt.axhline(
        y=368.9,
        color="orange",
        linestyle="--",
        label="Max SF7 Dwell Time (222 byte payload)",
    )
    plt.axhline(
        y=390.1,
        color="red",
        linestyle="--",
        label="Max SF9 Dwell Time (53 byte payload)",
    )
    plt.title("LoRaWAN Packet Size vs Airtime")
    plt.xlabel("Bytes")
    plt.ylabel("Logarithmic Airtime (ms)")
    plt.yscale("log")  # <-- Make y-axis logarithmic
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()


def plot_energy_vs_payload(buffer_results):
    plt.figure(figsize=(10, 6))

    for result in buffer_results:
        plt.plot(
            result.raw_payload_sizes,
            result.raw_energys,
            marker="o",
            label=f"Buffer size {result.buffer_size}",
        )

    plt.xlabel("Payload Size (bytes)")
    plt.ylabel("Energy per Transmission (Joules)")
    plt.title("Energy vs. Data Transmitted per Transmission")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()


def analyze_buffering_effect(csv_file, buffer_sizes):
    """
    Analyze how buffering multiple samples affects power usage and airtime for both raw ADC and delta-encoded values.

    Args:
        csv_file (str): Path to the CSV file containing voltage, current, and timestamp data.
        buffer_sizes (list): List of buffer sizes to test (number of measurements per transmission).
    """
    buffer_results = {size: BufferAnalysisData(size) for size in buffer_sizes}

    # Read data from the CSV file
    timestamps, voltages, currents = read_voltage_current_from_csv(csv_file)

    # Prepare measurements as dictionaries
    measurements = [
        {
            "ts": int(timestamp.timestamp()),
            "voltage_delta": int(voltage),
            "current_delta": int(current),
        }
        for timestamp, voltage, current in zip(timestamps, voltages, currents)
    ]
    # Prepare delta-encoded measurements
    delta_measurements = []
    previous_voltage = None
    previous_current = None

    for i, (timestamp, voltage, current) in enumerate(
        zip(timestamps, voltages, currents)
    ):
        if previous_voltage is None or previous_current is None:
            # First measurement uses raw values
            delta_measurements.append(
                {
                    "ts": int(timestamp.timestamp()),
                    "voltage_delta": int(voltage),
                    "current_delta": int(current),
                }
            )
        else:
            # Calculate deltas for subsequent measurements
            delta_measurements.append(
                {
                    "ts": int(timestamp.timestamp()),
                    "voltage_delta": convert_to_ones_complement(
                        int(voltage - previous_voltage)
                    ),
                    "current_delta": convert_to_ones_complement(
                        int(current - previous_current)
                    ),
                }
            )

        # Update previous values
        previous_voltage = voltage
        previous_current = current

    # Test each buffer size
    for buffer_size in buffer_sizes:
        print(f"Buffer Size: {buffer_size}")
        print("-" * 40)

        # Analyze raw ADC values
        total_airtime_raw = 0
        total_energy_raw = 0
        transmissions_raw = 0
        payload_sizes = []

        # Group measurements into batches based on the buffer size
        for i in range(0, len(measurements), buffer_size):
            batch = measurements[i : i + buffer_size]

            # Serialize the batch
            serialized = encode_repeated_power_deltas(
                logger_id=1, cell_id=2, entries=batch
            )

            # Calculate airtime and energy consumption
            serialized_size = len(serialized)
            payload_sizes.append(serialized_size)
            airtime = calculate_lorawan_airtime(pl_bytes=serialized_size)
            energy = calculate_energy_consumption(
                airtime / 1000
            )  # Convert ms to seconds

            # Store for visualization
            buffer_results[buffer_size].add_raw_data(airtime, energy, serialized_size)

            # Accumulate results
            total_airtime_raw += airtime
            total_energy_raw += energy
            transmissions_raw += 1

        print(f"  Raw ADC Values:")
        print(f"    Total Transmissions: {transmissions_raw}")
        print(f"    Average Payload Size: {statistics.mean(payload_sizes):.2f} bytes")
        print(f"    Average Airtime: {total_airtime_raw / transmissions_raw:.2f} ms")
        print(f"    Total Airtime: {total_airtime_raw:.2f} ms")
        print(
            f"    Average Energy per Transmission: {total_energy_raw / transmissions_raw:.6f} J"
        )
        print(f"    Total Energy: {total_energy_raw:.6f} J")

        # Store for visualization

        # Analyze delta-encoded values
        total_airtime_delta = 0
        total_energy_delta = 0
        transmissions_delta = 0
        delta_payload_sizes = []

        # Group delta measurements into batches based on the buffer size
        for i in range(0, len(delta_measurements), buffer_size):
            batch = delta_measurements[i : i + buffer_size]

            # Serialize the batch
            serialized = encode_repeated_power_deltas(
                logger_id=1, cell_id=2, entries=batch
            )

            # Calculate airtime and energy consumption
            serialized_size = len(serialized)
            delta_payload_sizes.append(serialized_size)
            airtime = calculate_lorawan_airtime(pl_bytes=serialized_size)
            energy = calculate_energy_consumption(
                airtime / 1000
            )  # Convert ms to seconds

            # Accumulate results
            total_airtime_delta += airtime
            total_energy_delta += energy
            transmissions_delta += 1

            buffer_results[buffer_size].add_delta_data(airtime, energy, serialized_size)
            buffer_results[buffer_size].add_delta_cumulative_data(total_energy_delta)

        print(f"  Delta-Encoded Values:")
        print(f"    Total Transmissions: {transmissions_delta}")
        print(
            f"    Average Payload Size: {statistics.mean(delta_payload_sizes):.2f} bytes"
        )
        print(
            f"    Average Airtime: {total_airtime_delta / transmissions_delta:.2f} ms"
        )
        print(f"    Total Airtime: {total_airtime_delta:.2f} ms")
        print(
            f"    Average Energy per Transmission: {total_energy_raw / transmissions_raw:.6f} J"
        )
        print(f"    Total Energy: {total_energy_delta:.6f} J")
        print("-" * 40)

    # Plotting
    raw_energys = [statistics.mean(res.raw_energys) for res in buffer_results.values()]
    delta_energys = [
        statistics.mean(res.delta_energys) for res in buffer_results.values()
    ]
    raw_airtimes = [
        statistics.mean(res.raw_airtimes) for res in buffer_results.values()
    ]
    delta_airtimes = [
        statistics.mean(res.delta_airtimes) for res in buffer_results.values()
    ]

    min_raw_airtimes = [min(res.raw_airtimes) for res in buffer_results.values()]
    max_raw_airtimes = [max(res.raw_airtimes) for res in buffer_results.values()]
    min_delta_airtimes = [min(res.delta_airtimes) for res in buffer_results.values()]
    max_delta_airtimes = [max(res.delta_airtimes) for res in buffer_results.values()]

    # plot_energy_savings_vs_buffer_size(buffer_sizes, raw_energys, delta_energys)
    # plot_raw_vs_delta_energy(buffer_sizes, raw_energys, delta_energys)
    plot_raw_vs_delta_airtime(
        buffer_sizes,
        raw_airtimes,
        delta_airtimes,
        min_raw_airtimes,
        max_raw_airtimes,
        min_delta_airtimes,
        max_delta_airtimes,
    )
    plot_adc_readings_and_savings(voltages, currents, timestamps, buffer_results)
    plot_airtime_for_bytes(range(20, 222))  # Example airtimes for SF9 and SF7
    plot_energy_vs_payload(buffer_results)
    plt.show()


if __name__ == "__main__":
    # Path to the CSV file
    csv_file = "./sc_local_open_2.csv"

    print("-" * 40)
    print(f"SF9 payload size: {53} bytes, and maximum airtime {185.3} ms")
    print(f"SF7 payload size: {222} bytes, and maximum airtime {368.9} ms")
    print("-" * 40)

    # Test with different buffer sizes
    buffer_sizes = [1, 2, 3, 5, 10, 15, 20, 25, 30]
    analyze_buffering_effect(csv_file, buffer_sizes)
