# Quickstart

## Flashing

The Wio-E5 (stm32 based) and the esp32 have different methods of flashing but both use the [PlatformIO](pio) system. The VSCode extension is the most intuitive to use with setup instructions available [here](https://platformio.org/install/ide?install=vscode) and quick start guide available [here](https://docs.platformio.org/en/latest/integration/ide/vscode.html#quick-start). There is also a CLI interface that is similar to the `Make` build system with installation instructions varying depending on you OS. The [quick start guide](https://docs.platformio.org/en/latest/core/quickstart.html#process-project) is a good reference for common commands.

In VSCode PlatformIO extension requires a folder with a `platformio.ini` file for the project configuration. We recommend opening the root folder `ents-node-firmware` in VSCode than adding the `esp32` and `stm32` folders with *File/Add Folder to Workspace...*, then saving the workspace to the root project folder. The `.code-workspace` file should be automatically excluded from git. After all environments in both `esp32` and `stm32` should be available.

> Following the instructions [here](configuring-ports.md) to set your local ports in `platformio.ini` for both the `stm32` and `esp32` environments.

The Wio-E5 relies on a ST-Link JTAG interface with detailed instructions available at [stm/README.md](../stm32/README.md). Using the cli you can flash the firmware with the following command:

```bash
pio run -e stm32 -t upload -t monitor
```

The esp32 uses a built in a bootloader that can be accessed over UART, detailed instructions are available at [eps32/README.md](../esp32/README.md).

```bash
pio run -e release -t upload -t monitor
```

## User Config

Its likely the device is failed to upload due to missing the user configuration. You can open our desktop applications for modifying the configuration by installing the python package with the following from the root directory of the project:

```bash
python -m venv .venv
pip install ents
gui
```

