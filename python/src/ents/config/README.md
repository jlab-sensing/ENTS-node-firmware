# GUI Application

## Overview

This PyQt5-based graphical user interface (GUI) allows users to configure settings for a device. It supports input fields for Logger ID, Cell ID, upload method (WiFi or LoRa), upload interval, enabled sensors, and calibration parameters for voltage and current (V/I Slope and Offset). Settings can be saved to a JSON file, loaded from a saved file, or directly transmitted to the device via UART.

The application provides options to:

- **Save configurations to a JSON file**
- **Load previously saved configurations**
- **Directly transmit settings to the device via UART**, using Protobuf for serialization.
- **Load current configuration** directly from the device.

## Key Features

- **Save and Load Configurations**: Save configurations to a JSON file named with the cell ID, or load an existing configuration.
- **Real-time Configuration Transmission**: Settings are serialized using Protobuf when the "Send Configuration" button is clicked, then transmitted over UART to the STM32 device.
- **Load Current Configuration**: Users can load the current configuration from the device by clicking the "Load Current Configuration" button, which reads the configuration stored in FRAM and displays it in the fields.
- **Input Validation**: Ensures only valid data is entered, prompting error messages for any invalid fields to guide users.

## Installation

To install the required dependencies, use the following command:

```bash
pip install -r requirements.txt
```

Run the application by executing the following command:

```bash
python user_config.py
```

## Opening the GUI from the Executable (No Python Required)

If you don't have Python installed or prefer to use a pre-built executable, follow these steps to open the GUI from the desktop executable:

**1- Open the Executable file path:**

- The executable file can be found in the following path:

```bash
\proto\python\src\ents\config\dist
```

**2- Run the Executable:**

- Navigate to the directory where **userConfig.exe** is located.

**3- Start Using the GUI:**

- Once the executable is opened, the GUI will appear. You can now configure your device settings as usual without needing Python or any development tools.

- The GUI will work on Windows, MacOS, and Linux platforms.

## Example JSON Configuration File

The following is an example JSON file generated after saving or sending user configuration settings:

```bash
{
    "Logger ID": 5,
    "Cell ID": 5,
    "Upload Method": "LoRa",
    "Days": 5,
    "Hours": 5,
    "Minutes": 5,
    "Seconds": 5,
    "Enabled Sensors": {
        "Voltage": true,
        "Current": true,
        "Teros12": true,
        "Teros21": true,
        "BME280": true
    },
    "Calibration V Slope": 5.0,
    "Calibration V Offset": 5.0,
    "Calibration I Slope": 5.0,
    "Calibration I Offset": 5.0,
    "WiFi SSID": "",
    "WiFi Password": "",
    "API Endpoint URL": "",
    "API Port": 0
}
```

## Screenshots

### Application Screenshots

The following images show the GUI interface in different configurations:

|**WiFi Upload Method**|**LoRa Upload Method**|
|---------------------|-----------|
|![User Configuration diagram](../../../../../images/user_config_GUI_WiFi.png)|![User Configuration diagram](../../../../../images/user_config_GUI_LoRa.png)|

### GUI Flow Diagram

The flow diagram below illustrates the general process and structure of the application:

![User Configuration diagram](../../../../../images/GUI_flow_diagram.png)

## User Manual

### How to Use the GUI

**1- Save Configuration:**

- Fill in the necessary fields such as Logger ID, Cell ID, upload method, etc.
Click on the **"Save"** button to save the configuration to a JSON file. The file will be named with the Cell ID as the filename.

**2- Load Configuration:**

- To load a previously saved configuration, click on the **"Load"** button. Browse to the desired JSON file and load it into the application.

**3- Send Configuration to Device:**

- After entering the configuration settings, click on the **"Send Configuration"** button to transmit the settings to the device via UART. The data will be serialized using Protobuf and sent to the device.

**4- Load Current Configuration from Device:**

- To read the current configuration from the device, click on the **"Load Current Configuration"** button. This retrieves the stored configuration from FRAM and populates the fields with the current values.
