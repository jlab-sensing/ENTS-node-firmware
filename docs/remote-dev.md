# Remote Development

Platformio has the unique feature to remotely flash and monitor devices, see [here](https://docs.platformio.org/en/latest/plus/pio-remote.html) for more information. This describes how to setup remote development for *stm32* portion of the project and any pitfalls you may encounter. Remote development for the *esp32* is a WIP ([#109](https://github.com/jlab-sensing/soil-power-sensor-firmware/issues/109)).

## Setup account

Remote agent access is only available within a platformio account. Multiple accounts cannot connect to the same agent. The account is shared along collaborators and the login information can be obtained from one of the maintainers. It is recommended to interactively enter the username and password to prevent them from being stored in terminal history. You can login with the following command:

```bash
platformio account login
```

Verify the remote agent is accessible with the following:

```bash
pio remote agent list
```

Expect output:

```
sps-dev
-------
ID: a06b13f8a12b89b2fc1d247a4c3d6e507ffa53b9
Started: 2024-07-03 08:59:31
```

Check that the devices are recognized with the following. `/dev/ttyUSB0` is the Wio-E5 mini with the stm32 LoRa hardware. `/dev/ttyACM0` is the [STLINK-V3MINIE](https://www.st.com/en/development-tools/stlink-v3minie.html) for flashing. These may be useful for specifying the *upload* and *test/monitor* port when flashing as they could change if devices are unplugged.

```bash
pio remote device list
```

Expect output: 

```
Agent sps-dev
=============
/dev/ttyUSB0
------------
Hardware ID: USB VID:PID=10C4:EA60 SER=d0a4692d5587ed11812a32d7a603910e LOCATION=1-1.5
Description: CP2102N USB to UART Bridge Controller

/dev/ttyACM0
------------
Hardware ID: USB VID:PID=0483:3754 SER=002700193033511835393935 LOCATION=1-1.3:1.1
Description: STLINK-V3 - ST-Link VCP Ctrl
```

## Usage

Starting the serial monitor **before** uploading is recommended to view the complete output from the device. There is no way to physically reset the device but it can be accomplished by re-uploading the firmware. This is not a problem since we use an external debugger used `SWD` rather than uploading through the serial port that is used with Arduino.

### Uploading

Use the following to build and upload an environment. The firmware is built locally and transferred to the agent for flashing

```bash
pio remote run -e stm32 -t upload
```

## Monitoring

Use the following to open a serial monitor to the device. The serial monitor can be stopped with `Ctrl+C`.

```bash
pio remote device monitor
```

### Testing

Run unit tests with the following command. Some tests require external hardware to be connect and have to be ignored. For `test_sdi12` this is done with the argument `-i test_sdi12`. There is a platformio bug that only the last test that is built is uploaded (see [this](https://github.com/platformio/platformio-core/issues/3270) issue) requiring the `-r` option for building the test firmware remotely.

```bash
pio remote test -e tests -i test_sdi12 -r 
```
