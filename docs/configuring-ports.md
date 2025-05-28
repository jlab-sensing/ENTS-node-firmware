# Configuring Ports

In both `stm32/platformio.ini` and `esp32/platformio.ini` the `upload_port`, `monitor_port`, and `test_port` will need to be changed to match the USB port. ***DO NOT*** change the `debug_port` as it will cause issues when launching the debugger. To get a list of connected devices in VSCode, click the following *PlatformIO Tab -> Project Tasks -> General -> Devices*. There is also an equivalent CLI command.

**VSCode**

![VSCode Devices](images/vscode_devices.png)

**CLI**

```bash
pio device list
```

The following is the expected output with the Wio-E5 and ST-Link connected via USB. The esp32 port definition will depend on the USB to TTL used to interface with the exposed UART pins. In my case `/dev/ttyUSB0` with description *CP2102N USB to UART Bridge Controller* is the serial connection with the Wio-E5 module and `/dev/ttyACM0` is the ST-Link.

```
/dev/ttyUSB0
------------
Hardware ID: USB VID:PID=10C4:EA60 SER=fe18dcf14e87ed119ee029d7a603910e LOCATION=1-2
Description: CP2102N USB to UART Bridge Controller

/dev/ttyACM0
------------
Hardware ID: USB VID:PID=0483:3754 SER=004B00233033510635393935 LOCATION=1-4:1.1
Description: STLINK-V3 - ST-Link VCP Ctrl
```

The USB ports can be copied into `platformio.ini` as follows:

```ini
debug_port = localhost:3333
upload_port = /dev/ttyACM0

monitor_port = /dev/ttyUSB0
monitor_speed = 115200

test_port = /dev/ttyUSB0
test_speed = 115200
```
