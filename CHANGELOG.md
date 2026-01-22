# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to
[Semantic Versioning](https://semver.org/spec/v2.0.0.html). The major and
minor versions follow hardware versioning with patches used to iterate on
firmware versions for a hardware version.

## [2.3.6] - 2026-01-22

### Added

- Add python interface to dirtviz [#279](i279)

### Fixed

- Desktop calibration utility encoding [#300][i300]

[i279]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/279
[i300]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/300

## [2.3.5] - 2026-01-04

### Added

- Support for pcap2 sensor [#297](i297)

### Updated

- Esp32 enters a deep sleep state when idle [#150](i150)
- Upload interval for LoRaWAN [#254](i254)
- Updated WiFi user config interface missing sensors [#297](i297)
- Sensor encoding updates [#292](i292)

### Fixed

- Updated clock configuration [#268](i268)

[i150]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/150
[i254]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/254
[i268]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/268
[i292]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/292
[i297]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/297

## [2.3.4] - 2025-08-20

### Added

- Example to force esp32 in deep sleep [#207](i207)
- Utility to change SDI-12 sensor address [#210](i210)
- Added new sensors
    - SEN0308
    - SEN0257
    - YFS210C
- Added controlling of solenoid from esp32 api
- Add user configuration vai WiFi [#259](i259)

### Updated

- Changed WiFi code to instantly upload [#249](pr249)

### Fixed

- Clock configuration consistance between main, examples, tests [#252](i252)

[i207]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/207
[i210]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/210
[i252]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/252
[pr249]: https://github.com/jlab-sensing/ENTS-node-firmware/pull/249
[i259]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/259

## [2.3.3] - 2025-06-18

- Fixed missing fields when decoding messages [#243](i243)
- Refactored WiFi code [#164](i164)
    - Part of this include `-DTEST_USER_CONFIG` to hardcoded values

[i164]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/164
[i243]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/243

## [2.3.2] - 2025-05-23

### Added

- Simulation utility for WiFi uploads [#217](i217)

### Updated

- Restructured python code [#226](pr226)
- Made the doxygen documentation usable [#205](i205)
- Getting started guide [#119](i119)
- Renamed `soil-power-sensor-firmware` to `ents` [#151](i151)
- Renamed `gui` to `ents-gui` [#235](pr235)

[i119]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/119
[i151]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/151
[i205]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/205
[i217]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/217
[pr226]: https://github.com/jlab-sensing/ENTS-node-firmware/pull/226
[pr235]: https://github.com/jlab-sensing/ENTS-node-firmware/pull/235

## [2.3.1] - 2025-05-09

### Added

- WiFi upload method [#46](i46)
- Support for the Teros12 sensor [#172](i172)
- LED status indicator [#197](i197)
- Define for commit sha [#199](i199)
- Turn off analog circuit when not in use [#202](i202)

### Changed

- Fixed issue with buffer addressing [#174](i174)

[i46]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/46
[i172]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/172
[i174]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/174
[i197]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/197
[i199]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/199
[i202]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/202

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

- Support for bme280 sensor [#87](i87), [#99](i99)
- Support for MB85RC1MT fram chip [#136](i136)
- Support for Teros21 sensor [#75](i75)
- Integrated GUI with sensor/measurement configuration [#170](i170)

### Changed

- Measurement mode of bme280 to forced [#99](i99)

[i75]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/75
[i87]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/87
[i99]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/99
[i136]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/136
[i170]: https://github.com/jlab-sensing/ENTS-node-firmware/pull/170

## [2.2.0] - 2024-12-13

### Added
- Contributing resources ([Contributing](./CONTRIBUTING.md),
  [Code ofConduct](./CODE_OF_CONDUCT.md), and Changelog)
- Issue and PR templates
- New sensors library to simplify future sensor integrations
- Transmit buffer [#41](i41)
- RTC time synchronization support through LoRaWAN [#48](i48)
- Calibration support [#50](i50)
- Minimum working example for configuration GUI

### Changed

- Unit tests for fifo library in `test_fifo.c`
- Implementation of fram to a flat address space
- Implementation of fifo to a circular buffer
- Added delay to start of tests to remove requirement of manual reset during testing
- Removed [custom OpenOCD](https://github.com/jlab-sensing/tool-openocd) dependency

[i41]: https://github.com/jlab-sensing/soil-power-sensor-firmware/issues/41
[i48]: https://github.com/jlab-sensing/soil-power-sensor-firmware/issues/48
[i50]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/50

## [2.1.0] - 2024-02-25

Initial functional release of firmware. The firmware can preform the following:

- Measure voltage
- Read measurement from TEROS-12 over SDI-12
- Transmit measurement to Dirtviz through LoRaWAN
