# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to
[Semantic Versioning](https://semver.org/spec/v2.0.0.html). The major and
minor versions follow hardware versioning with patches used to iterate on
firmware versions for a hardware version.

## [Unreleased]

### Added

- Contributing resources ([Contributing](./CONTRIBUTING.md),
  [Code ofConduct](./CODE_OF_CONDUCT.md), and Changelog)
- Issue and PR templates
- New sensors library to simplify future sensor integrations
- Transmit buffer [#41](i41)
- RTC time synchronization support through LoRaWAN [#48](i48)

### Changed

- Unit tests for fifo library in `test_fifo.c`
- Implementation of fram to a flat address space
- Implementation of fifo to a circular buffer
- Added delay to start of tests to remove requirement of manual reset during testing

[i41]: https://github.com/jlab-sensing/soil-power-sensor-firmware/issues/41
[i48]: https://github.com/jlab-sensing/soil-power-sensor-firmware/issues/48

## [2.1.0] - 2024-02-25

Initial functional release of firmware. The firmware can preform the following:

- Measure voltage
- Read measurement from TEROS-12 over SDI-12
- Transmit measurement to Dirtviz through LoRaWAN