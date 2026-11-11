# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to
[Semantic Versioning](https://semver.org/spec/v2.0.0.html). The major and
minor versions follow hardware versioning with patches used to iterate on
firmware versions for a hardware version.

## [Unreleased] - 2026-XX-XX

### Added

- Watermark 200TS soil temperature sensor (thermistor) added option to read the temperature using a resistor divider [#360](https://github.com/jlab-sensing/ENTS-node-firmware/pull/360).
- Solenoid functionality via downlink [#351](https://github.com/jlab-sensing/ENTS-node-firmware/pull/351).
- Added instructions for setting up a SenseCAP M2 LoRaWAN gateway [#369](https://github.com/jlab-sensing/ENTS-node-firmware/pull/369).
- Added quickstart guide for getting started with a prepared ENTS board [#369](https://github.com/jlab-sensing/ENTS-node-firmware/pull/369).

### Updated

- SEN0257 and SEN0308 now use the sensor context (index) to determine which ADC channel to sample [#361](https://github.com/jlab-sensing/ENTS-node-firmware/pull/361).
- `lora_app.c` onRXData fport switch statement modified to incorporate solenoid [#351](https://github.com/jlab-sensing/ENTS-node-firmware/pull/351).
- Fixed FRAM addresses to prevent user configuration overwrites [#362](https://github.com/jlab-sensing/ENTS-node-firmware/pull/362).
- Removed `test_ads` from the standard internal test suite since the ADS1219 is not built into the >=v3.0.0 ENTS boards anymore [#369](https://github.com/jlab-sensing/ENTS-node-firmware/pull/369) [#371](https://github.com/jlab-sensing/ENTS-node-firmware/pull/371).
- Adjusted ESP32 partition table to expand program flash by reducing file system space [#369](https://github.com/jlab-sensing/ENTS-node-firmware/pull/369).
- CI/CD pipeline linting and python version fixed [#384](https://github.com/jlab-sensing/ENTS-node-firmware/pull/384).

## [2.3.12] - 2026-08-19

### Added

- Additional information on web userconfig interface [#308](https://github.com/jlab-sensing/ENTS-node-firmware/pull/308)
- Wifi user config through the ESP32 fixed to properly work with the multiple sensors scheme [#347](https://github.com/jlab-sensing/ENTS-node-firmware/pull/347)
- All existing sensors updated to use user-deisgnated per-sensor Cell ID (or default Cell ID) [#353](https://github.com/jlab-sensing/ENTS-node-firmware/pull/353)
- Added new sensors:
    - Watermark 200SS Soil Tension Sensor
    - Watermark 200TS Soil Temperature Sensor
    - DFRobot EDU1057 Lark Weather Station [#339](https://github.com/jlab-sensing/ENTS-node-firmware/pull/339)
    - ALS-MPM-2F (TL-136 / GL-136) Water Level Sensor [#356](https://github.com/jlab-sensing/ENTS-node-firmware/pull/356)

### Updated

- Teros12 outputs in percentage rather than decimal
- `lora_app.c` modified to retry LoRa clock sync if unsuccessful on the first try.
- Fixed BME280 code reading only 0 output [#355](https://github.com/jlab-sensing/ENTS-node-firmware/pull/355).

## [2.3.7] - 2026-02-03

### Added

- Added documentation on publishing new releases.

### Fixed

- Fixed python library to distinguish between raw TEROS12 VWC readings and adjusted TEROS12 VWC readings. [ents-backend#610](https://github.com/jlab-sensing/ENTS-backend/issues/610)

## [2.3.6] - 2026-01-22

### Added

- Add python interface to dirtviz [#279](https://github.com/jlab-sensing/ENTS-node-firmware/issues/279)

### Fixed

- Desktop calibration utility encoding [#300][i300]

## [2.3.5] - 2026-01-04

### Added

- Support for pcap2 sensor [#297](https://github.com/jlab-sensing/ENTS-node-firmware/issues/297)

### Updated

- Esp32 enters a deep sleep state when idle [#150](https://github.com/jlab-sensing/ENTS-node-firmware/issues/150)
- Upload interval for LoRaWAN [#254](https://github.com/jlab-sensing/ENTS-node-firmware/issues/254)
- Updated WiFi user config interface missing sensors [#297](https://github.com/jlab-sensing/ENTS-node-firmware/issues/297)
- Sensor encoding updates [#292](https://github.com/jlab-sensing/ENTS-node-firmware/issues/292)

### Fixed

- Updated clock configuration [#268](https://github.com/jlab-sensing/ENTS-node-firmware/issues/268)

## [2.3.4] - 2025-08-20

### Added

- Example to force esp32 in deep sleep [#207](https://github.com/jlab-sensing/ENTS-node-firmware/issues/207)
- Utility to change SDI-12 sensor address [#210](https://github.com/jlab-sensing/ENTS-node-firmware/issues/210)
- Added new sensors
    - SEN0308
    - SEN0257
    - YFS210C
- Added controlling of solenoid from esp32 api
- Add user configuration vai WiFi [#259](https://github.com/jlab-sensing/ENTS-node-firmware/issues/259)

### Updated

- Changed WiFi code to instantly upload [#249](https://github.com/jlab-sensing/ENTS-node-firmware/pull/249)

### Fixed

- Clock configuration consistance between main, examples, tests [#252](https://github.com/jlab-sensing/ENTS-node-firmware/issues/252)

## [2.3.3] - 2025-06-18

- Fixed missing fields when decoding messages [#243](https://github.com/jlab-sensing/ENTS-node-firmware/issues/243)
- Refactored WiFi code [#164](https://github.com/jlab-sensing/ENTS-node-firmware/issues/164)
    - Part of this include `-DTEST_USER_CONFIG` to hardcoded values

## [2.3.2] - 2025-05-23

### Added

- Simulation utility for WiFi uploads [#217](https://github.com/jlab-sensing/ENTS-node-firmware/issues/217)

### Updated

- Restructured python code [#226](https://github.com/jlab-sensing/ENTS-node-firmware/pull/226)
- Made the doxygen documentation usable [#205](https://github.com/jlab-sensing/ENTS-node-firmware/issues/205)
- Getting started guide [#119](https://github.com/jlab-sensing/ENTS-node-firmware/issues/119)
- Renamed `soil-power-sensor-firmware` to `ents` [#151](https://github.com/jlab-sensing/ENTS-node-firmware/issues/151)
- Renamed `gui` to `ents-gui` [#235](https://github.com/jlab-sensing/ENTS-node-firmware/pull/235)

## [2.3.1] - 2025-05-09

### Added

- WiFi upload method [#46](https://github.com/jlab-sensing/ENTS-node-firmware/issues/46)
- Support for the Teros12 sensor [#172](https://github.com/jlab-sensing/ENTS-node-firmware/issues/172)
- LED status indicator [#197](https://github.com/jlab-sensing/ENTS-node-firmware/issues/197)
- Define for commit sha [#199](https://github.com/jlab-sensing/ENTS-node-firmware/issues/199)
- Turn off analog circuit when not in use [#202](https://github.com/jlab-sensing/ENTS-node-firmware/issues/202)

### Changed

- Fixed issue with buffer addressing [#174](https://github.com/jlab-sensing/ENTS-node-firmware/issues/174)

## [2.3.0] - 2025-02-26

Code used for March of the ENTS experiment. Release includes support for the
bme280, larger fram chip, teros21, and integration with a desktop GUI for
device configuration. The experiment revelated multiple bugs that will be fixed
in future releases including the following:

Code used for March of the ENTS experiment. Release includes support for the
bme280, larger fram chip, teros21, and integration with a desktop GUI for
device configuration. The experiment revelated multiple bugs that will be fixed
in future releases including the following:

- Circular buffer overwriting user config [#174](https://github.com/jlab-sensing/ENTS-node-firmware/issues/174)
- Support for Teros12 functionality (got lost in merges) [#172](https://github.com/jlab-sensing/ENTS-node-firmware/issues/172)

### Added

- Support for bme280 sensor [#87](https://github.com/jlab-sensing/ENTS-node-firmware/issues/87), [#99](https://github.com/jlab-sensing/ENTS-node-firmware/issues/99)
- Support for MB85RC1MT fram chip [#136](https://github.com/jlab-sensing/ENTS-node-firmware/issues/136)
- Support for Teros21 sensor [#75](https://github.com/jlab-sensing/ENTS-node-firmware/issues/75)
- Integrated GUI with sensor/measurement configuration [#170](https://github.com/jlab-sensing/ENTS-node-firmware/issues/170)

### Changed

- Measurement mode of bme280 to forced [#99](https://github.com/jlab-sensing/ENTS-node-firmware/issues/99)

## [2.2.0] - 2024-12-13

### Added
- Contributing resources ([Contributing](./CONTRIBUTING.md),
  [Code ofConduct](./CODE_OF_CONDUCT.md), and Changelog)
- Issue and PR templates
- New sensors library to simplify future sensor integrations
- Transmit buffer [#41](https://github.com/jlab-sensing/ENTS-node-firmware/issues/41)
- RTC time synchronization support through LoRaWAN [#48](https://github.com/jlab-sensing/ENTS-node-firmware/issues/48)
- Calibration support [#50](https://github.com/jlab-sensing/ENTS-node-firmware/issues/50)
- Minimum working example for configuration GUI

### Changed

- Unit tests for fifo library in `test_fifo.c`
- Implementation of fram to a flat address space
- Implementation of fifo to a circular buffer
- Added delay to start of tests to remove requirement of manual reset during testing
- Removed [custom OpenOCD](https://github.com/jlab-sensing/tool-openocd) dependency

## [2.1.0] - 2024-02-25

Initial functional release of firmware. The firmware can preform the following:

- Measure voltage
- Read measurement from TEROS-12 over SDI-12
- Transmit measurement to Dirtviz through LoRaWAN
