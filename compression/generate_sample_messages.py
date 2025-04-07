import math
import matplotlib.pyplot as plt
import re

# Parameters
samples = 1000
frequency = 1  # Hz
sample_rate = 100  # Hz
duration = samples / sample_rate
time = [i / sample_rate for i in range(samples)]

# Simulate sine waves and generate messages
messages = []
for t in time:
    V = 2 * math.sin(2 * math.pi * frequency * t)
    I = 0.0009 * math.sin(2 * math.pi * frequency * t + math.pi / 6)  # small phase shift
    P = V * I
    message = f"V: {V:.4f}, I: {I:.6f}, P: {P:.8f}\n"
    messages.append(message)

# Write messages to a text file
with open("power_data.txt", "w") as f:
    f.writelines(messages)

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
plt.plot(time, voltages, color='blue')
plt.title("Voltage over Time")
plt.xlabel("Time (s)")
plt.ylabel("Voltage (V)")
plt.grid(True)
plt.tight_layout()

# Plot Current
plt.figure(figsize=(10, 4))
plt.plot(time, currents, color='green')
plt.title("Current over Time")
plt.xlabel("Time (s)")
plt.ylabel("Current (A)")
plt.grid(True)
plt.tight_layout()

# Plot Power
plt.figure(figsize=(10, 4))
plt.plot(time, powers, color='red')
plt.title("Power over Time")
plt.xlabel("Time (s)")
plt.ylabel("Power (W)")
plt.grid(True)
plt.tight_layout()

plt.show()
