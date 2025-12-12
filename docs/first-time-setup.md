# First Time Setup

The following is a list of steps that should be complated on a newly assembled board to flash this repos firmware for the first time. Assembly instructions are located in the top level README of [ENTS-node-hardware](https://github.com/jlab-sensing/ENTS-node-hardware).

- [ ] Clear the bootloader the stm32. (See [stm32/README.md](stm32/README.md) for instructions)
- [ ] Run unit tests on the stm32. (See [stm32/README.md](stm32/README.md) for instructions)
- [ ] Flash `stm32` environment firmware to the stm32. (See [stm32/README.md](stm32/README.md) for instructions)
- [ ] Flash the `release` firmware to the esp32. (See [esp32/README.md](esp32/README.md) for instructions)
- [ ] Calibrate the analog measurement channels. (See TBD for instructions)


# Detailed Setup Guide

0. Prerequisites:
    - Software:
        - Clone this repository and stay on the `main` branch.
        - Download and install PlatformIO ([IDE](https://docs.platformio.org/en/latest/integration/ide/vscode.html#ide-vscode) or [Core/CLI](https://docs.platformio.org/en/latest/core/installation/index.html)).
            - It is a good idea to also add the [PlatformIO executables to the path](https://docs.platformio.org/en/latest/core/installation/shell-commands.html#piocore-install-shell-commands).
        - Download and install [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html).
    - Hardware:
        - [Keithley 2450 Source Measurement Unit (SMU)](https://www.tek.com/en/products/keithley/source-measure-units/2400-graphical-series-sourcemeter), used for calibrating the onboard ADC.
        - 2 kOhm resistor (through-hole 1/8 W), used for calibrating the onboard ADC.
            - Can also be a nearby value (i.e. 2.2 kOhm) because the SMU will regulate the voltage and current provided to the ADC.
        - [STLINK-V3MINIE](https://www.st.com/en/development-tools/stlink-v3minie.html) programmer/debugger, used for programming the STM32.
        - [USB to TTL adapter](https://ftdichip.com/products/ttl-232r-3v3/) (3.3 V logic level), used for programming the ESP32 and serial communication with the ESP32.
            - Alternative [USB to TTL adapter](https://www.amazon.com/JESSINIE-USB-Serial-CP2102-Interworking/dp/B0B1HVCTSW)

1. Solder the remaining components
    - J1: 5pos screw terminal, facing board edge. Purpose: Analog voltage and current input terminal.
    - J4: 3pos screw terminal, facing board edge. Purpose: SDI-12.
    - J3: 2x5 male pin header (0.1" pitch). Purpose: Serial communication header for the ESP32.
    - J8: 1x10 female pin header right angle (0.1" pitch), hanging off of the board edge. The plastic body of the female pin header should be flush against the PCB. Purpose: Battery board connection terminal.
    - U10: Wio-E5 antenna end goes on the bottom edge of the board (closest to the micro SD card holder) and the antenna connector should be pushed flush against the PCB edge. Purpose: LoRaWAN communications and main microcontroller.
    - JP5: solder bridge across middle pad (pin 2) and top pad (pin 1, marked by arrow, closest to the JP5 silkscreen label). Purpose: ESP32 enable pin pullup to 3.3 V.
    - JP1, JP3, J9: solder bridge these jumpers. J9 was originally for inserting a resistor, but should instead be bridged. Purpose: Analog sensing connections between the voltage, current, and ground terminals.
        - NOTE: Leave JP2 unsoldered.

2. Clear STM32 bootloader
    - Prerequistes
        - Connect the STLINK to the D2 JTAG header located near the J4 3pos screw terminal for SDI-12, then connect the STLINK to your computer.
        - Use a USB cable to connect the Wio-E5 to your computer.
    - On the Wio-E5, enter the bootloader by pressing the Wio-E5's buttons in the following sequence:
        - Press and hold the white RST button.
        - Press and hold the white BOOT button.
        - Release the white RST button.
        - Release the white BOOT button.
    - Open STM32CubeProgrammer on your computer.
    - Press the Connect button in the top right of the interface.
        - If this is a new board, the Wio-E5's factory firmware is still on the chip. You will see the error message "Error: data read failed". This is expected since the stock firmware has read protection. (Otherwise, you see the current device memory.)
    - On a new board you will need to disable read protection to fix the "Error: data read failed" error. Go to the Option Bytes tab (box with the letters "OB" on the left) and click Read Out Protection, set the RDP level to AA, or "no protection". Then press apply and wait for confirmation.
        - The "Compact View" tab can be used to show the RDP level easily.
    - After changing the RDP level to AA and applying the change, press the Disconnect button in the top right of the interface.

3. Run unit tests on the STM32
    - Prerequisites:
        - Open a PlatformIO terminal in the `stm32/` folder.
        - (Same as previous step) Connect the STLINK to the D2 JTAG header located near the J4 3pos screw terminal for SDI-12, then connect the STLINK to your computer.
        - (Same as previous step) Use a USB cable to connect the Wio-E5 to your computer.
    - `pio test -e tests --upload-port <COMx> --test-port <COMy>`
        - Replace `<COMx>` and `<COMy>` with the port for the STLINK and Wio-E5 respectively.
        - Use `pio device list` to list the connected devices.
            - The device with STLINK in the description should be used for the upload port.
            - The device with CP2102 in the description (this is the Wio-E5's serial port) should be used for the test port. PlatformIO will read the Wio-E5's serial output to determine if the tests pass or fail.
        - Example: `pio test -e tests --upload-port COM9 --test-port COM10`
        - KNOWN ISSUE: If you attempt to run these tests after clearing the STM32 bootloader, the Wio-E5 will fail to respond. Use CTRL+C in the terminal running the test to cancel it, and physically disconnect/reconnect the USB cable connecting the Wio-E5 to your computer. Then, try to run the tests again.
    - Follow the prompts to press the Wio-E5's RST button if the first test in any of the test environments hangs for more than 10 seconds with no progress.
        - NOTE: `test_fram` will take a few minutes to read and write to the FRAM chip's entire memory.

4. Calibrate the onboard ADC
    - Prerequisites:
        - (Same as previous step) Open a PlatformIO terminal in the `stm32/` folder.
            - Create a Python virtual environment (in the root level of the repo): `python -m venv ../.venv`
            - Activate the Python virtual environment: `source ../.venv/bin/activate` (Linux) or `..\.venv\bin\activate.bat` (Windows)
            - Install the ENTS Python package: `pip install ents`
                - ALTERNATIVE: `pip install -e python` can be used to install the repository's local files as a package. If changes are made to the calibration control code (ex. adjust current sweep range), then use this alternate method.
        - (Same as previous step) Connect the STLINK to the D2 JTAG header located near the J4 3pos screw terminal for SDI-12, then connect the STLINK to your computer.
        - (Same as previous step) Use a USB cable to connect the Wio-E5 to your computer.
        - On the SMU, set SCPI 2400 in menu --> settings.
            - Connect the SMU to an ethernet switch and connect your computer to the switch as well (or get a network connection between your computer and the SMU).
            - ALTERNATIVE: Directly connect your computer to the SMU via ethernet. Most devices should support APIPA and generate IPs for the local network.
    - `pio run -e calibrate_adc -t upload --upload-port <COMx>`
        - Replace `<COMx>` with the port for the STLINK.
        - Use `pio device list` to list the connected devices.
            - The device with STLINK in the description should be used for the upload port.
        - Example: `pio run -e calibrate_adc -t upload --upload-port COM9`
    - Connect the probes to the board as shown in the diagram below, where `R` is a 2 kOhm (or 2.2 kOhm) resistor.. Then follow the prompts in the calibration program.
        - When prompted to change to the current input, just press enter again since the configuration in the diagram is suitable for both the voltage calibration and the current calibration.
        ```
        (SMU+) --+----- [V+]
                 R      [V-]
                 +----- [I+]
                        [I-]
        (SMU-) -------- [GND]
        ```
    - `ents calib <COMy> <SMU IP address>:5025`
        - Replace `<COMy>` and `<SMU IP address>` with the port for the Wio-E5 and the IP address of the SMU respectively.
        - Use `pio device list` to list the connected devices.
            - The device with STLINK in the description should be used for the upload port.
        - The SMU IP address can be found in the SMU's settings. Go to the Communications --> LAN settings tab.
        - Example: `ents calib COM10 128.114.204.201:5025`
    - Save the voltage slope, voltage intercept, current slope, and current intercept. The upper half of the terminal output is the voltage calibration values. The lower half of the terminal output is the current calibration values. The slopes and intercepts are located near the "nCoefficient" text.
        - NOTE FOR UCSC: Just copy and paste the entire terminal output into the [board inventory column K "Calibration Terminal Dump"](https://docs.google.com/spreadsheets/d/1CHnVyCUWfR958FQWvbkh7DhEm4VlopQin3-EASD9zO0/edit?gid=0#gid=0&range=K1). The regex will automatically extract the slope, intercept, and standard deviation values.
        - Regex string: `Slope: \[\[([^\[\]]*)\].*\nIntercept: \[([^\[\]]*)\][\s\S]*std = ([\d\.e\-]*)[\s\S]*Slope: \[\[([^\[\]]*)\].*\nIntercept: \[([^\[\]]*)\][\s\S]*std = ([\d\.e\-]*)`

5. Flash the latest firmware for the STM32 and the ESP32
    - Prerequisites:
        - Open a terminal in the `stm32/` (or reuse the terminal from the previous step). This terminal will be referred to as the STM32 terminal.
        - Open a terminal in the `esp32/` folder. This will be referred to as the ESP32 terminal.
        - (Same as previous step) Connect the STLINK to the D2 JTAG header located near the J4 3pos screw terminal for SDI-12, then connect the STLINK to your computer.
        - (Same as previous step) Use a USB cable to connect the Wio-E5 to your computer.
        - Use a USB to TTL adapter to connect the ESP32 (via the J3 header's GND, TX, and RX pins) to your computer. Be sure to connect your TTL adapter's TX pin to the ESP32's RX pin and vice versa.
    - STM32 terminal: `pio run -e stm32 -t upload -t monitor --upload-port <COMx> --monitor-port <COMy>`
        - Replace `<COMx>` and `<COMy>` with the port for the STLINK and the Wio-E5 respectively.
        - Use `pio device list` to list the connected devices.
            - The device with STLINK in the description should be used for the upload port.
            - The device with CP2102 in the description should be used for the monitor port. Note: If your USB to TTL adapter also uses the CP2102 chip, be sure to correctly identify and select the port corresponding to the Wio-E5.
        - The monitor port can be closed with CTRL+C. Additionally, the monitor port can be opened independently of the `upload` command: `pio device monitor -b 115200 -p <COMy>`
        - Example: `pio run -e stm32 -t upload -t monitor --upload-port COM9 --monitor-port COM10`
    - Save the Wio-E5's LoRaWAN information, which is prepended by `######`. You will need the AppEUI/JoinEUI, DevEUI, and AppKey.
        - NOTE FOR UCSC: Copy and paste the STM32's serial output (only lines with ######) into the [board inventory column L "Key Dump"](https://docs.google.com/spreadsheets/d/1CHnVyCUWfR958FQWvbkh7DhEm4VlopQin3-EASD9zO0/edit?gid=0#gid=0&range=L1). The regex will automatically extract the DevEUI, JoinEUI (AppEUI), and AppKey.
        - The ENTS board's default configuration is to use LoRa communication. In order to print out the LoRaWAN information, the board must be configured to transmit via LoRa, not wifi. 
    - On the ESP32, enter the bootloader by pressing the ESP32's buttons in the following sequence:
        - Press and hold the blue RST button.
        - Press and hold the blue BOOT button.
        - Release the blue RST button.
        - Release the blue BOOT button.
    - ESP32 terminal:
        - `pio run -e release -t uploadfs --upload-port <COMx>`
        - `pio run -e release -t upload -t monitor --upload-port <COMx> --monitor-port <COMx>`
        - Replace `<COMx>` with the port for the USB to TTL adapter.
        - Use `pio device list` to list the connected devices.
            - The device corresponding to your USB to TTL adapter should be used for both the upload port and the monitor port.
        - The monitor port can be closed with CTRL+C. Additionally, the monitor port can be opened independently of the `upload` command: `pio device monitor -b 115200 -p <COMx>`
        - Example: `pio run -e release -t upload -t monitor --upload-port COM7 --monitor-port COM7`
    - NOTE FOR UCSC: Save the ESP32's wifi MAC. The wifi MAC is made available near the tail end of the upload log, as well as by viewing the serial output from the device. The wifi MAC is required to whitelist the device on the UCSC-Devices wifi network.
        - Note that the wifi AP MAC is not the same as the wifi MAC. You must register the regular wifi MAC, not the AP MAC.
    - Restart both the STM32 and the ESP32. First press and release the blue RST button near the ESP32, then press and release the Wio-E5's RST button.

6. Register the logger on DirtViz (which performs The Things Network registration to enable LoRaWAN communication).
    - Log in to DirtViz and go to your profile's Loggers page: https://dirtviz.jlab.ucsc.edu/profile/loggers
    - Click on the plus icon in the top right to begin adding a new logger.
        - Logger Name: This should match the serial number of the device. See column A of the spreadsheet. Ex. 2.2.3-071
        - Logger Type: ENTS
        - Device EUI: Enter the Wio-E5's DeviceEUI from the previous step.
        - Join EUI: Enter the Wio-E5's JoinEUI/AppEUI from the previous step.
        - App Key: Enter the Wio-E5's AppKey from the previous step.
        - Description (Optional): Copy and paste the voltage slope, voltage intercept, current slope, and current intercept. Ex. [-0.00039374, -0.00030046] [-1.18649323e-10, 4.04241107e-05]
    - Once registered, note down the logger ID, which will be used to configure the ENTS board later.
    - Troubleshooting:
        - If the logger fails to be created on DirtViz, the end device may already exist on The Things Network. In this case, you must log into The Things Network console, delete the end device, and try to register the logger on DirtViz again.

7. Create a cell on DirtViz.
    - https://dirtviz.jlab.ucsc.edu/
    - Sign in in the top right
    - Click on your name in the top right and select the Profile dropdown option.
    - Click on the "+" icon in the top right of the "Your Cells" table and enter in the cell details.
        - Name: Try to use something descriptive, since cells are sorted alphabetically in the dropdown list on DirtViz.
            - Example: `nu_local_open_1`
        - Location: Location of the cell.
        - Latitude: Only used for the map feature. Set to 0 if you don't care about the map.
            - WRP Latitude: `36.95633`
        - Longitude: Only used for the map feature. Set to 0 if you don't care about the map.
            - WRP Longitude: `-122.057894`
    - Once created, note down the cell ID, which will be used to configure the ENTS board later.

8. Use the wifi userconfig to set device parameters
    - Prerequisites:
        - Device must have the `stm32` (STM32) and `release` (ESP32) firmware flashed (done in a previous step).
    - Restart both the STM32 and the ESP32. First press and release the blue RST button near the ESP32, then press and release the Wio-E5's RST button.
    - The ESP32 broadcasts a wifi network `ents-<LOGGER_ID>` for a duration after the ESP32 and STM32 boot up.
    - Use your computer to connect to the broadcasted wifi network, then open a web browser and navigate to: [192.168.4.1](http://192.168.4.1/)
    - When this page is initially opened, it polls the ENTS board's current configuration (except the wifi password is not shown).
    - Modify the desired userconfig parameters:
        - Logger ID: Set to the logger ID you obtained on DirtViz.
        - Cell ID: Set to the cell ID you obtained on DirtViz.
        - Upload method: Select between LoRa and wifi communication. Using LoRa requires a LoRaWAN gateway (ex. RG191).
        - Upload Interval: Set the frequency of uploads/measurements.
        - Select the sensors
            - NOTE: Selecting voltage will also take current measurements in addition to voltage measurements. Therefore, do not select current.
        - Enter the calibration values. This step is optional if you do not intend on using the voltage and current measurement feature.
        - Set up wifi SSID and password if upload method is set to wifi.
        - API Endpoint URL: `http://dirtviz.jlab.ucsc.edu/api/sensor/`
            - NOTE: Do not include any trailing spaces after the URL!
    - Click on the green button at the bottom of the page to send the configuration to the board.
    - Follow the prompt to press the white RST button on the Wio-E5.
    - You may disconnect the power cable and set up the board and cell, then reconnect the power cable. To obtain voltage and current readings, follow the connection guide used during the calibration step, but use the MFC's leads in place of the SMU's leads as shown below. `R` may be a 2 kOhm or 2.2 kOhm resistor. The resistor may be omitted (thus disconnecting `[I+]`) to keep the MFC in open circuit configuration.
```
(MFC+) --+----- [V+]
         R      [V-]
         +----- [I+]
                [I-]
(MFC-) -------- [GND]
```
