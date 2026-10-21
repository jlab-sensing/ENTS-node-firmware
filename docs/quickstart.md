# Quickstart

This Quickstart guide explains how to get started with a prepared ENTS board (board assembled and bootloader unlocked).

## Prerequisites
- Prepared ENTS board (board assembled and bootloader unlocked/cleared).
    - To know if your ENTS board is prepared or not, check the back of the board. If some of the solder jumpers on the back of the board are soldered (such as JP12 for enabling the PB5 user LED), then you have a prepared ENTS board.
- ST-LINKV3 MINIE STM32 programming tool
- USB-TTL adapter
- USB-C cable (C to A or C to C depending on your computer's available USB ports)
- A computer running Windows, Linux, or MacOS.
    - At least two available USB ports. You can use a USB hub if you need additional USB ports.
    - VS Code with the PlatformIO extension installed (see [Installing PlatformIO](#installing-platformio))

## About selecting ports
In this guide, you will be asked to select the correct ports for uploading programs, running tests, and opening serial monitors.
- In any PlatformIO terminal, use the command `pio device list` to list all currently connected devices.
- PlatformIO automatically detects the ST-LINK and uses it for uploading the program to the STM32, so the `--upload-port` argument does not need to specified for STM32-related commands.
- If your USB to TTL adapter also uses the CP2102 chip, be sure to correctly identify which device corresponds to the Wio-E5 and which corresponds to your USB to TTL adapter. You can determine this easily by unplugging the USB to TTL adapter and running `pio device list` again to show the currently connected devices.

## 1. Run unit tests on the STM32
- Prerequisites:
    - Connect the STLINK to the J4 JTAG header located near the USB-C port, then connect the STLINK to your computer.
    - Use a USB-C cable to connect the ENTS board to your computer.
- Open a PlatformIO terminal in the `stm32/` folder and run the following command: `pio test -e --test-port COMx`
    - Replace `COMx` with the port for the Wio-E5 (identified as the CP2102 chip).
    - Example: `pio test -e tests --test-port COM10`
    - KNOWN ISSUE: If you attempt to run these tests after clearing the STM32 bootloader, the Wio-E5 will fail to respond. Use CTRL+C in the terminal running the test to cancel it, and physically disconnect/reconnect the USB cable connecting the Wio-E5 to your computer. Then, try to run the tests again.
- Follow the prompts to press the Wio-E5's RST button if the first test in any of the test environments hangs for more than 10 seconds with no progress.
    - `test_fram` will take a few minutes to read and write to the FRAM chip's entire memory.

## 2. Flash the latest firmware for the STM32 and the ESP32
- Prerequisites:
    - (Same as previous step) Connect the STLINK to the J4 JTAG header located near the USB-C port, then connect the STLINK to your computer.
    - (Same as previous step) Use a USB-C cable to connect the ENTS board to your computer.
    - Use a USB to TTL adapter to connect the ESP32 (via the J14 header's GND, TX, and RX pins) to your computer. Be sure to connect your TTL adapter's TX pin to the ESP32's RX pin and vice versa. If you only have two USB ports available, you can disconnect the STLINK after you have finished flashing the STM32 and plug in your USB to TTL adapter.
- Open a PlatformIO terminal in the `stm32/` folder and run the following command: `pio run -e stm32 -t upload -t monitor --monitor-port COMx`
    - Replace `COMx` with the port for the Wio-E5 (identified as the CP2102 chip).
    - Example: `pio run -e stm32 -t upload -t monitor --monitor-port COM10`
- Save the Wio-E5's LoRaWAN information, which is prepended by `######`. You will need the AppEUI/JoinEUI, DevEUI, and AppKey. Only the last 4 bytes of the DevEUI (i.e. DevAddr) should vary between boards.
    - The monitor port can be closed with CTRL+C. Additionally, the monitor port can be opened independently of the `upload` command: `pio device monitor -b 115200 -p COMx`
    - NOTE FOR UCSC: Copy and paste the STM32's serial output (only lines with ######) into the [board inventory column L "Key Dump"](https://docs.google.com/spreadsheets/d/1CHnVyCUWfR958FQWvbkh7DhEm4VlopQin3-EASD9zO0/edit?gid=0#gid=0&range=L1). The regex will automatically extract the DevEUI, JoinEUI (AppEUI), and AppKey.
- Open a PlatformIO terminal in (or move your existing terminal to) the `esp32/` folder and perform/run the following:
    - On the ENTS board, enter the ESP32's bootloader mode by:
        1. Hold down SW4 BOOT (near the center of the board).
        2. Press and release SW3 RESET (next to SW4).
        3. Release SW4 BOOT.
    - In both of the following commands, replace `COMy` with the port for the USB to TTL adapter.
    - `pio run -e release -t uploadfs --upload-port COMy`
        - This command updates the ESP32 file system which holds the HTML webform for configuring the ENTS board wirelessly.
    - Enter the bootloader mode again (tap SW3 RESET while holding down SW4 BOOT).
    - `pio run -e release -t upload --upload-port COMy`
        - This command programs the ESP32 with the standard program.
    - Note: The ESP32 enable (EN) pin is controlled by the STM32 through a GPIO pin. In order to program the ESP32, you first have to ensure that the STM32 has enabled the ESP32. This is most easily done by loading the `stm32` program which you have already done earlier in this guide.
- NOTE FOR UCSC: Save the ESP32's wifi MAC. The wifi MAC is made available near the tail end of the ESP32's build log, after it connects to the ESP32 but before it begins writing the firmware. The wifi MAC is required to whitelist the device on the UCSC-Devices wifi network. This is the wifi join MAC, which is not the same as the wifi AP MAC.
- Tap the SW1 RESET button next to the USB-C port to restart the ENTS board.

## 3. Register the logger on ENTS-backend (which performs The Things Network registration to enable LoRaWAN communication)
- Log in to your instance of the ENTS-backend and go to your profile's Loggers page. For DirtViz, this would be: https://dirtviz.jlab.ucsc.edu/profile/loggers
- Click on the "+" icon in the top right to begin adding a new logger.
    - Logger Name: Provide a unique name for the logger.
        - NOTE FOR UCSC: This should match the hardware revision and serial number of the device. See column A of the spreadsheet. Ex. 2.2.3-071
    - Logger Type: ENTS
    - Device EUI: Enter the DeviceEUI
    - Join EUI: Enter the JoinEUI/AppEUI
    - App Key: Enter the AppKey
    - Description: optional
- Once registered, note down the logger ID which will be used to configure the ENTS board later.
    - NOTE FOR UCSC: Save the logger ID and the owner's name into the board inventory spreadsheet.

## 4. Create a cell on ENTS-backend
- Log in to your instance of the ENTS-backend and go to your profile's Cells page. For DirtViz, this would be: https://dirtviz.jlab.ucsc.edu/profile/cells
- Click on the "+" icon in the top right to begin adding a new cell.
    - Cell Name: Try to use something descriptive, since cells are sorted alphabetically in the dropdown list on the dashboard.
        - Example: `nu_local_open_1`
    - Location: Location of the cell. Used for your own reference.
        - Example: `WRP`
    - Latitude: Only used for the map feature. Set to 0 if you don't care about the map.
        - WRP Latitude: `36.95633`
    - Longitude: Only used for the map feature. Set to 0 if you don't care about the map.
        - WRP Longitude: `-122.057894`
- Once created, note down the cell ID which will be used to configure the ENTS board later.
    - NOTE FOR UCSC: Save the cell ID to the board inventory spreadsheet (to the logger(s) which upload to that cell).

## 5. Use the wifi userconfig to set device parameters
- Prerequisites:
    - Device must have the `stm32` (STM32) and `release` (ESP32) firmware flashed (done in the previous step).
- Restart the ENTS board by tapping the SW1 RESET button next to the USB-C port.
- On your computer, connect to the wifi network broadcasted by the ENTS board. The network name is `ents-<last_4_bytes_of_devEUI>`.
    - Example: `ents-05471EA7`
    - This network is disabled after a few minutes with no clients connected, so you may need to restart the ENTS board again if you do not join it in time.
- Use a web browser to navigate to http://192.168.4.1
    - If you are reconfiguring an ENTS board, the board's pre-existing configuration is shown when the webpage is entered or refreshed.
- Input the values into the userConfig interface. You can click on the expandable text above most fields to expand the help text.
    - Logger ID: Enter the logger ID that you registered previously. You can find it on your profile's Loggers page.
    - Default Cell ID: This Default Cell ID is applied to any sensors in which you do not specify a Cell ID. Enter the cell ID that you created previously. You can find it on your profile's Cells page.
    - Upload method: Using LoRa requires a LoRaWAN gateway such as the [Sentrius RG191](https://www.ezurio.com/part/rg191). To get started more easily, wifi is recommended. Enterprise wifi requiring a username and password (or a login / acknowledgement portal) will not work. For best results, use an open or WPA2-PSK standard password-protected wifi network.
    - Upload Interval: The upload interval may not be less than 10 seconds. It is recommended to choose an upload interval >= 60 seconds.
    - Select the sensors that you have attached to the ENTS board.
        - In some cases, the Index field may be clicked twice (or press the up/down arrow while selecting it) to see the valid inputs, typically for selecting which GPIO pin the sensor uses.
        - Make sure not to have multiple sensors mapped mapped to the same GPIO pin.
    - ADS1219 Voltage and Current Calibration: This is only necessary if you have selected to use the ADS1219 Voltage or Current sensors.
    - WiFi Settings:
        - These settings are only relevant if you have selected WiFi as the upload method.
        - If you are reconfiguring an ENTS board, you can check the box to reuse the wifi password that was previously entered into the device.
        - NOTE FOR UCSC: For wifi, you should use the `UCSC-Devices` SSID. You must register your devices on the portal https://wifi-devices.ucsc.edu/ and use the password provided to you after registration on the portal.
        - API Endpoint URL: The API endpoint URL is typically `http://<your.endpoint.here.com>/api/sensor/` (defaulting to port 80 when no port is specified). For example, DirtViz's API endpoint URL is `http://dirtviz.jlab.ucsc.edu/api/sensor/`.
            - If your endpoint's port is not 80, you can append it to the end of your URL: http://your.endpoint.here.com/api/sensor/:12345
- IMPORTANT: When you are done selecting your configuration, press the `Save Configuration` button at the bottom of the page.
- Optional: To verify that the userConfig was saved successfully, you can either:
    - [A] Restart the ENTS board (tap SW1 RESET), then reconnect to the ENTS board's wifi network and see if your configuration displays correctly on the webform http://192.168.4.1
    - [B] Use a serial monitor to examine the Wio-E5's output upon startup: `pio device monitor -b 115200 -p COMy`

## Installing PlatformIO

The Wio-E5 (stm32 based) and the esp32 have different methods of flashing but both use the [PlatformIO](pio) system. The VSCode extension is the most intuitive to use with setup instructions available [here](https://platformio.org/install/ide?install=vscode). After installing the PlatformIO extension, be sure to familiarize yourself with the [PlatformIO interface](https://docs.platformio.org/en/latest/integration/ide/vscode.html#platformio-toolbar).

If you want to be able to access PlatformIO command line tools without having to open a PlatformIO terminal through VS Code, it is recommended to [install the shell commands](https://docs.platformio.org/en/latest/core/installation/shell-commands.html#piocore-install-shell-commands).
