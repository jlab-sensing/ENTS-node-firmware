# ESP32 User Configuration Interface

This library provides an interactive web-based interface for configuring the user configurations using Wi-Fi. Users can connect to an access point hosted by the ESP32 and enter their desired configuration via a webpage. These settings are validated and stored locally on the device using Protobuf for efficient serialization.

## user_cofig Library file Structure

```
.
├── src/
│   └── config_server.cpp      # Sets up WiFi Access Point and HTTP server, handles web routes
│   └── configuration.cpp      # contains Function that serially print the user configuration
│   └── protobuf_utils.cpp     # contains Functions that serially print Encoded/decoded configuration
│   └── validation.cpp         # Validates user input from the web form
├── include/
│   └── config_server.h        # Declarations for WiFi + HTTP server setup and handlers
│   └── configuration.h        # Declarations for configuration structure
│   └── protobuf_utils.h       # Declarations for Protobuf encode/decode helpers
│   └── validation.h           # Declarations for input validation functions
├── README.md                  # Project documentation and usage instructions
```

## How It Works

1. The ESP32 boots and starts an Access Point (`ESP32-C3-Config`).
2. It serves an HTML-based web page for configuration entry.
3. Users connect to this AP and open a browser to `192.168.4.1`.
4. On the webpage, users fill in and submit their desired configuration.
5. Input data is validated. If invalid, feedback is shown on the page.
6. Valid configurations are encoded using Protobuf and stored locally.

## Getting Started

### Flash the Code

1. Connect the ESP32 to your PC via USB to ttl.
2. Open a terminal and run the following command to build, upload, and monitor:

```bash
pio run -e userconfig -t upload -t monitor
```

3. After upload, **press the reset button** on the ESP32.
4. The serial monitor should display:

```
ESP-ROM:esp32c3-api1-20210207
Build:Feb  7 2021
rst:0x1 (POWERON),boot:0xd (SPI_FAST_FLASH_BOOT)
SPIWP:0xee
mode:DIO, clock div:1
load:0x3fcd5810,len:0x438
load:0x403cc710,len:0x90c
load:0x403ce710,len:0x2624
entry 0x403cc710
Access Point started
IP Address: 192.168.4.1
HTTP server started
```

## Connecting to the Web Interface

1. On your phone or computer, go to Wi-Fi settings.
2. Connect to the access point named `ESP32-C3-Config`.
3. Enter the Wi-Fi password provided in the code.
4. Open a browser and go to:

```
http://192.168.4.1
```

5. You will see a configuration webpage.
6. Fill in all fields carefully and click **Save Configuration**.
7. If any field is incorrect, a validation message will appear.

## Features

- Local AP with no internet needed
- Interactive web interface for user config
- Configurable fields (e.g., Logger ID, WiFi credentials, Calibration values)
- Field validation and feedback
- Configurations saved using Protobuf
