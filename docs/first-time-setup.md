# First Time Setup

The following is a list of steps that should be complated on a newly assembled board to flash this repos firmware for the first time. Assembly instructions are located in the top level README of [ENTS-node-hardware](https://github.com/jlab-sensing/ENTS-node-hardware).

- [ ] Clear the bootloader the stm32. (See [stm32/README.md](stm32/README.md) for instructions)
- [ ] Run unit tests on the stm32. (See [stm32/README.md](stm32/README.md) for instructions)
- [ ] Flash `stm32` environment firmware to the stm32. (See [stm32/README.md](stm32/README.md) for instructions)
- [ ] Flash the `release` firmware to the esp32. (See [esp32/README.md](esp32/README.md) for instructions)
- [ ] Calibrate the analog measurement channels. (See TBD for instructions)


# Detailed Setup Guide

0. Prerequisites:
    - Clone this repository and stay on the `main` branch.
    - Download and install PlatformIO ([IDE](https://docs.platformio.org/en/latest/integration/ide/vscode.html#ide-vscode) or [Core/CLI](https://docs.platformio.org/en/latest/core/installation/index.html)).
        - It is a good idea to also add the [PlatformIO executables to the path](https://docs.platformio.org/en/latest/core/installation/shell-commands.html#piocore-install-shell-commands).
    - Download and install [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html).
    - Obtain:
        - [Keithley 2450 Source Measurement Unit (SMU)](https://www.tek.com/en/products/keithley/source-measure-units/2400-graphical-series-sourcemeter), used for calibrating the onboard ADC.
        - 2 kOhm resistor (through-hole 1/8 W), used for calibrating the onboard ADC.
            - Can also be a nearby value (i.e. 2.2 kOhm) because the SMU will regulate the voltage and current provided to the ADC.
        - [STLINK-V3MINIE](https://www.st.com/en/development-tools/stlink-v3minie.html) programmer/debugger, used for programming the STM32.
        - [USB to TTL adapter](https://ftdichip.com/products/ttl-232r-3v3/), used for programming the ESP32 (3.3 V logic level).
1. Solder the remaining components
    - J1: 5pos screw terminal, facing board edge.
    - J4: 3pos screw terminal, facing board edge.
    - J3: 2x5 male pin header (0.1" pitch).
    - J8: 1x10 female pin header right angle (0.1" pitch), hanging off of the board edge. The plastic body of the female pin header should be flush against the PCB.
    - U10: Wio-E5 antenna end goes on the bottom edge of the board (closest to the micro SD card holder) and the antenna connector should be pushed flush against the PCB edge.
    - JP5: solder bridge across middle pad and top pad (marked by arrow, closest to the JP5 silkscreen label).
    - JP1, JP3, J9: solder bridge these jumpers. J9 was originally for inserting a resistor, but should instead be bridged.
        - NOTE: Leave JP2 unsoldered.
2. Clear STM32 bootloader
    - Prerequistes
        - Use the STLINK to connect the D2 JTAG header to your computer.
        - Use a USB cable to connect the Wio-E5 to your computer.
    - On the Wio-E5, enter the bootloader:
        - Press and hold the RST button.
        - Press and hold the BOOT button.
        - Release the RST button.
        - Release the BOOT button.
    - Open STM32CubeProgrammer.
    - Press the Connect button in the top right of the interface.
        - If this is a new board, the Wio-E5's factory firmware is still on the chip. You will see the error message "Error: data read failed". This is expected since the stock firmware has read protection. (Otherwise, you see the current device memory.)
    - On a new board you will need to disable read protection to fix the "Error: data read failed" error. Go to the Option Bytes tab (box with the letters "OB" on the left) and click Read Out Protection, set the RDP level to AA, or "no protection". Then press apply and wait for confirmation.
        - The "Compact View" tab can be used to show the RDP level easily.
    - After changing the RDP level to AA and applying the change, press the Disconnect button in the top right of the interface.

3. Run unit tests on the STM32
    - Prerequisites:
        - Open a PlatformIO terminal in the `stm32/` folder.
        - Use the STLINK to connect the D2 JTAG header to your computer.
        - Use a USB cable to connect the Wio-E5 to your computer.
    - `pio test -e tests`
        - If necessary, specify the upload port and test ports as additional flags:
            - `--upload-port <COMx>`
            - `--test-port <COMy>`
            - Example: `pio test -e tests --upload-port <COMx> --test-port <COMy>`
            - Use `pio device list` to list the connected devices.
                - The device with STLINK in the description should be used for the upload port.
                - The device with CP2102 in the description (this is the Wio-E5's serial port) should be used for the test port.
        - KNOWN ISSUE: If you attempt to run these tests after clearing the STM32 bootloader, the Wio-E5 will fail to respond. Use "CTRL+C" in the terminal running the test to cancel it, and physically disconnect/reconnect the USB cable connecting the Wio-E5 to your computer.
    - Follow the prompts to press the Wio-E5's RST button if the first test in any of the test environments hangs for more than 10 seconds with no progress.
        - NOTE: `test_fram` will take a few minutes to read and write to the FRAM chip's entire memory.

4. Calibrate the onboard ADC
    - Prerequisites:
        - Open a PlatformIO terminal in the root level of the repository.
        - Create a python virtual environment: `python -m venv .venv`
        - Activate the python virtual environment: `source .venv/bin/activate` (Linux) or `.\venv\bin\activate.bat` (Windows)
        - Install the ENTS python package: `pip install ents`
            - ALTERNATIVE: `pip install -e python` can be used to install the repository's local files as a package. If changes are made to the calibration control code (ex. adjust current sweep range), then use this alternate method.
        On the SMU, set SCPI 2400 in menu --> settings
    pio run -t upload -e calibrate_adc --upload-port <COMxx>
    ents calib <COMxx for Wio-E5 (CP2102)> <SMU IP address>:5025
        EXAMPLE: ents calib COM19 169.254.211.2:5025
        NOTE: The SMU IP address can be found in the SMU's settings.
        
        pio run -t upload -e calibrate_adc --upload-port COM5 && ents calib --plot COM6 128.114.204.24:5025
            combo, run from stm32 folder
    Apparently the value of the resistor doesn't really matter, just choose 2.2k. (I used 1.978kOhm.)
        2 kOhm resistor between V+ and I+
        + probe on V+
        - probe on GND
    
        slight differences in stdev can be from the duty cycling of the ads to prevent bias current backflow
    note: make sure that no other device is holding the COM port.
    note2: image is skipped if in vscode terminal (non-interactive)
    
    upper half of the terminal output is voltage
    lower half of the terminal output is current
    grab the nCoefficient for slope and intercept (m and b values)
        NOTE: Just copy and paste the entire terminal output into the board inventory column K "Calibration Terminal Dump"
        https://docs.google.com/spreadsheets/d/1CHnVyCUWfR958FQWvbkh7DhEm4VlopQin3-EASD9zO0/edit?gid=0#gid=0&range=K1
        The regex will automatically extract the slope, intercept, and standard deviation values.

5. Flash the latest release firmware for stm32 and esp32
    stm32:
    prereqs:
        main or latest release branch
    pio run -t upload -t monitor -e stm32 --upload-port COM5 --monitor-port COM6
    Copy and paste the device's serial output (only lines with ######) into the board inventory column L "Key Dump"
        https://docs.google.com/spreadsheets/d/1CHnVyCUWfR958FQWvbkh7DhEm4VlopQin3-EASD9zO0/edit?gid=0#gid=0&range=L1
        The regex will automatically extract the certain values.

    esp32:
    connect USB TTL adapter (3.3V) to UART on J3 (TX, RX, GND)
    bootloader
        RST down
        BOOT down
        RST up
        BOOT up
    pio run -t upload -t monitor -e release --upload-port COM7 --monitor-port COM7

6. Use the GUI to set device parameters
    prereqs:
        Device must have the firmware flashed or example_gui
        open a terminal at the root level of the repository
        python -m venv .venv
         ... activate.bat
            NOTE: after this, you should see (.venv) on your terminal
        pip install -e python[gui]
    
    .venv\Scripts\activate.bat
        NOTE: Use the activate script based on your OS.
        NOTE: I had to use an absolute path to call my activate script.
            "C:\Users\jlin\Desktop\ents\gui\ENTS-node-firmware\.venv\Scripts\activate.bat"
    ents-gui
    Then input the values
        Logger ID: 200 is the default (not used in the backend yet)
        Cell ID: Based on the cell you set up in dirtviz (200 for junk tests)
        5 minutes
        Select the sensors (never use Current)
        Copy and paste the calibration values
        Set up wifi SSID and password if used, and API
            http://dirtviz.jlab.ucsc.edu/api/sensor/
            443

7. Register device on The Things Network
    Follow instructions here: https://github.com/jlab-sensing/ENTS-node-firmware/blob/release/2.3.0/docs/add-device-to-ttn.md
        NOTE: If you followed step 5 to flash the latest release firmware, skip to step 2
        NOTE: You can also register devices in bulk via a CSV upload. One of the tabs in the Board Inventory can help you do that.
        NOTE: If you do not have access to the soil power sensor group, ask John to add you.
        NOTE: The following fields are always the same
            app_key 2B7E151628AED2A6ABF7158809CF4F3C
            join_eui 0101010101010101
        NOTE: The id might not autogenerate. In that case, follow the convention based on the device EUI. Example: eui-0080e1150609bcf0
        NOTE: The name of the logger should naturally follow the board inventory. Example: 2.2.3-031

8. Create a cell on dirtviz (DO AT NORTHWESTERN)
    https://dirtviz.jlab.ucsc.edu/
    Sign in in the top right
    Click on your name in the top right and select the Profile dropdown option.
    Click on the "+" icon in the top right of the "Your Cells" table and enter in the cell details.
        Name: try to use something descriptive
        Location: WRP
        Longitude: -122.057894
        Latitude: 36.95633