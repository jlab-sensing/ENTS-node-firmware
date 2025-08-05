[![Build stm32](https://github.com/jlab-sensing/ents-node-firmware/actions/workflows/stm32.yaml/badge.svg)](https://github.com/jlab-sensing/ents-node-firmware/actions/workflows/stm32.yaml) [![Test stm32](https://github.com/jlab-sensing/ENTS-node-firmware/actions/workflows/stm32-test.yaml/badge.svg)](https://github.com/jlab-sensing/ENTS-node-firmware/actions/workflows/stm32-test.yaml) [![Build esp32](https://github.com/jlab-sensing/ents-node-firmware/actions/workflows/esp32.yaml/badge.svg)](https://github.com/jlab-sensing/ents-node-firmware/actions/workflows/esp32.yaml) [![Doxygen](https://github.com/jlab-sensing/ents-node-firmware/actions/workflows/docs.yaml/badge.svg)](https://github.com/jlab-sensing/ents-node-firmware/actions/workflows/docs.yaml)

# Environmental NeTworked Sensor (ENTS) Firmware Developer Guide

Firmware repository for STM32 and ESP32 microcontrollers on the ENTS-node board along with supporting libraries. This repository acts as the central hub for all repositories related to the project.

## Project Structure

All components of the ENTS project are open source and live across a range of repositories. This repository serves as a central hub for all components for the project including software and hardware.

| Repository | Description |
| --- | --- |
| https://github.com/jlab-sensing/ents-node-firmware | Firmware for microcontrollers |
| https://github.com/jlab-sensing/platform-ststm32 | Fork with our board and modifications to build script |
| https://github.com/jlab-sensing/tool-openocd | For allowing for user defined `openocd` binary |
| https://github.com/jlab-sensing/STM32CubeWL | Platformio library for STMicroelectronics HAL and other libraries |
| https://github.com/jlab-sensing/soil_power_sensor | Hardware design files for the sensor board |

## File structure of repository

The following paths are the major parts of the project. Each individual folder has more information relating to implementation and development.

| Path | Description | Link |
| --- | --- | --- |
| `stm32/` | Platformio project for the stm32-based Wio-E5 mini | [link](stm32/README.md) |
| `esp32/` | Platformio project for the ESP32-C3-MINI-N1 | [link](esp32/README.md) |
| `proto/` | Protobuf definitions and language specific libraries | [link](proto/README.md) |
| `python/` | Python module for interacting with the firmware | [link](python/README.md) |

## Documentation

How to use the *ents-node* is documented in various markdown files. See [docs/README.md](docs/README.md) for links to the documentation.

Online version of the code documentation for the `main` branch is available at https://jlab-sensing.github.io/ENTS-node-firmware/

### Generating Documentation Locally

This project use [Doxygen](https://www.doxygen.nl/) for its code documentation. HTML documentation is automatically generated through Github Actions and is updated whenever there is a change to the `main` branch. To generate documentation locally in the `doxygen/build/html/` folder, run the following from the root directory:

```bash
doxygen doxygen/Doxyfile
```

## Contributing

See [CONTRIBUTING.md](./CONTRIBUTING.md).

If you have a bug or feature request, please create an issue in the [issues tab](https://github.com/jlab-sensing/ENTS-node-firmware/issues). For hardware issues, create an issue in the [hardware repository](https://github.com/jlab-sensing/soil_power_sensor).

We have a public zulip channel for discussion of the project. You can join the channel [here](https://ents.zulipchat.com/). It is primarily used for discussion of the development of the project and is a good place to ask questions about the project. The channel is open to all and we encourage you to join if you are interested in the project.

## Code of Conduct

This project adheres to
[Contributor Covenant](https://www.contributor-covenant.org).
See [Code of Conduct](./CODE_OF_CONDUCT.md) for a local copy.

## License

Code in this repository is licensed under the MIT License unless specified in the file header. See @ref LICENSE for full document.

## Maintainers

- [John Madden](mailto:jtmadden@ucsc.edu) (UCSC)

## Contributors

- [Steve Taylor](mailto:sgtaylor@ucsc.edu) (UCSD)
- Jack Lin (UCSC)
- Ahmed Falah (GSOC)
- Caden Jacobs (UCSC)
- Ella Ferraz (UCSC)
- [Varun Sreedharan](mailto:vasreedh@ucsc.edu) (UCSC)

[pio]: https://platformio.org/
