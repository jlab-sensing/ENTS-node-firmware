# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to
[Semantic Versioning](https://semver.org/spec/v2.0.0.html). The major and
minor versions follow hardware versioning with patches used to iterate on
firmware versions for a hardware version.

## [Unreleased]

### Added

- Support for bme280 sensor [#87](i87), [#99](i99)
- Support for MB85RC1MT fram chip [#136](i136)

### Changed

- Measurement mode of bme280 to forced [#99](i99)

[i87]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/87
[i99]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/99
[i136]: https://github.com/jlab-sensing/ENTS-node-firmware/issues/136

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
