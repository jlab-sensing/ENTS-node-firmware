# Flashing Software Guide

## Needed:
The following are recommended.
- VSCode with PlatformIO installed, STM32 drivers
- CP2102 Driver (Serial to STM32)
- CH340 Driver (Serial and Flashing ESP32)
- STM32CubeProgrammer


## STM32
1) Clone ents firmware repository
2) Connect ST-Link to J4
3) Connect USB-C to computer
4) Open STM32CubeProgrammer
5) Hold down RST followed by BOOT Button near USB C Port
6) Release RST then release BOOT button
7) Press Connect on the software
<img width="463" height="127" alt="image" src="https://github.com/user-attachments/assets/49c69469-7e8f-4a62-b1fe-0c5f70aa9202" />

8) Once connected, press the OB tab, open the Read Out Protection and change it to AA. Then press apply. Disconnect, then physically unplug and replug the USB-C cable.
<img width="1144" height="483" alt="image" src="https://github.com/user-attachments/assets/8c56e0c7-d2ee-46a0-8bdb-fbd3f704fcc4" />
9) Configure ports as detailed in configuring-ports.md
10) Open the stm32 folder in vscode, once platformio initializes, run: pio test -e tests
11) Once tests pass, run: pio run -t upload -t monitor -e stm32

## ESP32
1) Connect USB-TTL Adapter to board, Connect GND to GND, RX to TX, and TX to RX
2) Hold down RST followed by BOOT Button near the pins
3) Release RST then release BOOT button
4) Ensure your ports are configured for the TTL
5) Open the esp32 folder in vscode, once platformio initializes, run: pio run -e release -t uploadfs
6) After this runs succesfully, upload the build with: pio run -e release -t upload -t monitor

## Confirmation
1) With all these steps completed, reset the STM32, and the webserver for the ESP32 should activate.
2) Connect to the wifi network called: ents-XXXXX with password ilovedirt
3) The serial output of the STM32 should include the ip-address that hosts the config page, ie: 192.168.XX.X
4) Configure and apply.
5) Reset STM32 one more time.
