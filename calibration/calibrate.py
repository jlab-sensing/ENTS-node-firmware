from recorder import SoilPowerSensorController
from recorder import SMUSerialController
from recorder import SMULANController
import time
from tqdm import tqdm
import pandas as pd

if __name__ == "__main__":
    # Hardcoded parameters
    source_mode = "voltage"  # or "current"
    samples = 10
    startV = -2.0
    stopV = 2.0
    stepV = 0.1
    startI = -0.0009
    stopI = 0.0009
    stepI = 0.0001

    smu_port = None #"/dev/ttyUSB0"  # Serial port
    smu_host = "128.114.204.90:5025"  # Set to None if using serial port; otherwise set to "ip:port" format

    sps_port = "/dev/ttyUSB0"
    data_file = "data.csv"
     
    # Initialize controllers
    sps = SoilPowerSensorController(sps_port) 
    if smu_port:
        smu = SMUSerialController(smu_port, source_mode)
    elif smu_host:
        host, port = smu_host.split(":")
        smu = SMULANController(host, int(port), source_mode)

    # Data dictionary to store results
    data = {
        "V": [],
        "I": [],
        "V_in": [],
        "I_in": [],
        "I_sps": [],
        "V_sps": [],
    }

    # Set range iterator based on source_mode
    if source_mode == "voltage":
        iterator = smu.vrange(startV, stopV, stepV)
    elif source_mode == "current":
        iterator = smu.irange(startI, stopI, stepI)

    # Data collection loop
    for value in tqdm(iterator):
        #time.sleep(500)
        for _ in range(samples):
            if source_mode == "voltage":
                data["V"].append(value)
                data["I"].append(0)
            elif source_mode == "current":
                data["I"].append(value)
                data["V"].append(0)
            
            measured_voltage, measured_current = sps.get_power()

            data["I_in"].append(smu.get_current())
            data["V_in"].append(smu.get_voltage())

            data["V_sps"].append(measured_voltage)
            data["I_sps"].append(measured_current)

    # Convert data dictionary to DataFrame and save as CSV
    data_df = pd.DataFrame(data)
    print(data_df)
    data_df.to_csv(data_file, index=False)
