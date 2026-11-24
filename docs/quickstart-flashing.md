# Quickstart: ENTS Board Flashing

There are two microcontrollers on the ENT boards, one for processing the data from the sensors/general computing and the other for networking/SD card management
STM32 = central microprocessor
ESP32 = Network and SDCard

They each have their own version of firmware that we need to flash. 
In general, the STM processor gets more frequent firmware updates, such as adding a new sensor. The ESP firmware is relatively stable. 

> **Note:** That said, each time you build and flash the STM you need to build and flash the ESP.
 
## Pre-requisites
 
### Hardware: 3 open USB ports
 
You need 3 free USB ports on the computer:
 
1. **USB-C** — board power
2. **STM32** — programming/debug connection (requires a specialized connector)
3. **ESP32** — programming/serial connection (requires a specialized connector)
> **TODO:** Add links to where to buy the STM32 and ESP32 connectors.
 
### PlatformIO (PIO) environment (uv-managed Python)

> **Note:** These instructions focus on how to set up PlatformIO in an environment where Python might have a major version upgrade relatively frequently, such as on Fedora. If your 
Python install is more stable you should probably use the default install instructions, which install PIO your Python system libraries: https://docs.platformio.org/en/latest/core/installation/methods/installer-script.html
 
PlatformIO runs from `~/.platformio/penv`. This sets that env up with a uv-managed Python, so Fedora system Python updates don't break it.
 
> **Note:** Python `3.15.2` was recent at the time of writing (2026-11-24). Change it to the appropriate Python version when needed.
 
```bash
uv python install 3.15.2
 
rm -rf ~/.platformio/penv
uv venv --managed-python --python 3.15.2 --seed ~/.platformio/penv
uv pip install --python ~/.platformio/penv/bin/python platformio
 
# Verify it's NOT the system Python: expect a path under ~/.local/share/uv/python/
~/.platformio/penv/bin/python -c 'import sys; print(sys.base_prefix)'
 
# clears bash's PATH cache
hash -r
pio --version
 
# Put pio on PATH: ~/bin/pio -> ~/.platformio/penv/bin/pio
ln -sf ~/.platformio/penv/bin/pio ~/bin/pio
```
 
### Confirm the laptop sees the board
 
With the board attached (all 3 cables plugged in):
 
```bash
# USB vendor IDs: 0483 = STMicro (ST-Link/STM32), 10c4 = Silicon Labs CP210x,
# 0403 = FTDI, 1a86 = CH340, 067b = Prolific PL2303
lsusb | grep -Ei '0483|10c4|0403|1a86|067b'
 
# Stable serial device names; use these to tell which port is which
ls -l /dev/serial/by-id/
 
# Your user must be in the dialout group to open serial ports (no output = not a member)
id -nG | grep -ow dialout
```
 
If you're not in `dialout`: `sudo usermod -aG dialout $USER`, then log out and back in.
 
### Confirm you're on the latest jLab firmware `main`
 
```bash
cd ~/git/ENTS-node-firmware
git fetch origin                # without this, status can't tell you if you're behind
git remote -v                   # shows whether origin is the jLab repo or your fork
git status -sb                  # expect "## main...origin/main" with no [behind N]
git log -1 --oneline            # commit you'll be building from
git describe --tags --always    # nearest release tag (note it when reporting issues)
```
 
If you're behind: `git switch main && git pull --ff-only`.
 
> If `origin` is your fork, being even with `origin/main` doesn't prove you match jLab. The agent's Step 1b (below) checks against upstream jLab directly.
 
### Hand the results to your agent
 
Paste the output of **both** sets of commands above (board detection and git) **along with this document** into your agent. It uses them to write the next set of commands for the board you have attached: the serial port paths, your repo path and the upstream SHA to check against.
 
#### Example agent output (board 3.1.0-039, 2026-11-24)
 
This is an example only. The port paths, board ID, DevEUI and SHA will be different for you. The example continues through the Flashing section below.
 
**Step 1: Set port variables and capture a baseline boot**
 
**1a. Save the variables to an env file.** Shell variables don't carry between terminals, so keep them in `~/ents-ports.env` and source it in **every new terminal**.
 
```bash
cat > ~/ents-ports.env <<'EOF'
export BOARD=3.1.0-039
export STM_CONSOLE=/dev/serial/by-id/usb-Silicon_Labs_CP2102N_USB_to_UART_Bridge_Controller_10bf5728c6f7f011aa451812364a576b-if00-port0
export ESP_PORT=/dev/serial/by-id/usb-1a86_USB_Serial-if00-port0
export FW=~/git/ENTS-node-firmware
EOF
 
source ~/ents-ports.env
```
 
**When you move to another board,** edit `~/ents-ports.env` and change:
 
- `STM_CONSOLE` (the CP2102N line): the path includes that board's serial number. Re-run `ls -l /dev/serial/by-id/` to get the new one.
- `BOARD`: the new board's ID, so its logs get their own filenames.
`ESP_PORT` (the CH340 adapter) and `FW` stay the same.
 
**1b. Confirm the fork matches upstream jLab.** This only reads; nothing changes.
 
```bash
git ls-remote https://github.com/jlab-sensing/ENTS-node-firmware.git refs/heads/main
```
 
The SHA should start with `2cccc7e1` (as of this example). If it doesn't, your fork is behind, so sync it before building.
 
**1c. Baseline boot log.** This shows what's on the board before you overwrite it.
 
Terminal 1:
 
```bash
source ~/ents-ports.env
picocom -b 115200 --imap lfcrlf --logfile ~/ents-$BOARD-baseline-$(date +%Y%m%d-%H%M).log "$STM_CONSOLE"
```

**On the Board** 

Then tap **SW1 RESET** (the button beside the USB-C). Exit picocom with `Ctrl-A`, then `Ctrl-X`.
 
Record from the log:
 
- `Git SHA:` (the firmware currently on the STM32)
- The `######` block: DevEUI, JoinEUI, AppKey
- Any existing user config, or "No existing user configuration"
- Whether the ESP32 responds, or there's an error talking to it
Paste back: the `ls-remote` line and the boot log, **redacted** (see "Safety: boot logs contain secrets" below).
 
**What to copy from the boot log:** everything after picocom's startup banner, up to just before picocom's `Terminating...` message. It starts at the `ENTS LoRa-E5 STM32 firmware, compiled on ...` line. Example (redacted):
 
```text
ENTS LoRa-E5 STM32 firmware, compiled on Nov 23 2026 15:27:03
Git SHA: 5fe2eb917a211c9a3317a84f71fad0b4e253f869
 
###### AppKey:      <redacted>
###### NwkKey:      <redacted>
###### AppSKey:     <redacted>
###### NwkSKey:     <redacted>
###### DevEUI:      00:80:E1:15:06:AB:2F:9E
###### AppEUI:      01:01:01:01:01:01:01:01
###### DevAddr:     06:AB:2F:9E
Encoded Power Command (4): 0x3A020801
 
No existing user configuration in FRAM.
ControllerTransmit() error (1), skipping receive
ControllerTransmit() error (1), skipping receive
 
WiFi AP Info:
---------------
ssid ""
pass "<redacted>"
User Config http:///
WiFi AP MAC: ""
 
Requesting configuration from ESP32...
No configuration to send to ESP32!
```

### Safety: boot logs contain secrets
 
The STM32 boot log prints secrets in plain text:
 
- **LoRaWAN keys:** `AppKey`, `NwkKey`, `AppSKey`, `NwkSKey`. Anyone with these can impersonate the device on the network or decrypt its traffic.
- **WiFi AP password:** the `pass "..."` line.
`DevEUI`, `AppEUI`/`JoinEUI` and `DevAddr` are identifiers, not secrets, and the agent may need them.
 
Rules:
 
1. **Lock down the log file** right after capture, and keep logs in `~`, never inside the firmware repo or any git repo.
2. **Redact before pasting** the log into an agent, chat, issue or doc. The agent doesn't need key values to plan a flash; it only needs to know they're present.
3. **Treat all keys as secret**, even if they look like defaults (identical keys, `01:01:...` EUIs). Provisioned boards will carry real ones, and the habit should be the same.
Print a redacted copy of the newest log for this board (baseline or flash) to copy from:
 
```bash
source ~/ents-ports.env
LOG=$(ls -t ~/ents-$BOARD-*.log | head -1)
chmod 600 "$LOG"
sed -E -e 's/^(###### (AppKey|NwkKey|AppSKey|NwkSKey):[[:space:]]+).*/\1<redacted>/' \
       -e 's/^(pass ")[^"]*/\1<redacted>/' "$LOG"
```
 
**Step 2: Build (no flashing yet)**
 
Building first separates compile errors from flashing errors. The first build downloads toolchains, so expect several minutes.
 
If picocom is still open, close it with `Ctrl-A`, then `Ctrl-X`.
 
```bash
source ~/ents-ports.env
 
cd "$FW"
git status --short          # must print nothing: the boot SHA should identify the exact code
git rev-parse --short HEAD  # expect 2cccc7e1 (as of this example); this is the SHA you'll see at boot after flashing
 
cd "$FW/stm32"
pio run -e stm32
 
cd "$FW/esp32"
pio run -e release
pio run -e release -t buildfs
```
 
What's being built:
 
- `stm32`: the main STM32 firmware, covering LoRaWAN, sensors and FRAM
- `release`: the ESP32-C3 app, covering WiFi and the config portal
- `buildfs`: the ESP32's LittleFS image, which holds the portal's web files. It's flashed separately from the app.
Expect each command to end with a summary table showing the env as `SUCCESS`.
 
Paste back: the last ~10 lines of each of the three builds. If `git status --short` prints anything, stop and paste that instead.
 

 
## Flashing
 
Continues the example agent output (board 3.1.0-039). Your agent fills in your board's values. Every terminal needs `source ~/ents-ports.env` first.
 
### Step 3a: Flash the STM32
 
**Why the STM32 goes first:** the STM32 firmware drives the ESP32's enable (EN) pin. A running, current STM32 makes the ESP32 flash and test go cleanly. jLab's factory checklist lists the ESP32 first, but on a board that already has STM32 firmware the order doesn't matter much.
 
**Physical setup:**
 
- **Antenna is attached.** Never let the LoRa radio transmit without one.
- Board USB-C is plugged in. It powers the board and carries the STM32 console.
- The STLINK's 10-pin ribbon is on **J4**, beside the USB-C, and the STLINK's USB is in the laptop.
- The CH340 on **J14** can stay connected.
**Terminal 1:** start logging first so you capture the boot right after the flash.
 
```bash
source ~/ents-ports.env
picocom -b 115200 --imap lfcrlf --logfile ~/ents-$BOARD-stm32-flash-$(date +%Y%m%d-%H%M).log "$STM_CONSOLE"
```
 
**Terminal 2:**
 
```bash
source ~/ents-ports.env
cd "$FW/stm32"
pio run -e stm32 -t upload
```
 
**How the upload works:** PlatformIO calls OpenOCD, which talks to the STLINK over USB and writes flash through SWD (`upload_protocol = stlink` in the ini). It doesn't use any `/dev/tty*` port, so no `--upload-port` is needed, and picocom can stay open during the flash.
 
> **Safe to ignore:** OpenOCD prints a halt message before programming that looks like an error:
>
> ```text
> [stm32wlx.cpu0] halted due to debug-request, current mode: Thread
> xPSR: 0x01000000 pc: 0x08007b98 msp: 0x20008000
> ** Programming Started **
> ```
>
> This is OpenOCD stopping the CPU so it can write flash. It is expected. What matters is `SUCCESS` at the end.
 
**Expect:**
 
- **Terminal 2:** OpenOCD lines ending in `** Verified OK **` and `** Resetting Target **`, then `SUCCESS`
- **Terminal 1:** the board reboots by itself. If nothing appears, tap **SW1 RESET**. You should see:
  - `Git SHA: 2cccc7e1…` (the SHA from Step 2)
  - The same DevEUI as the baseline (`00:80:E1:15:06:AB:2F:9E` in this example)
  - "No existing user configuration in FRAM"
  - ESP32 errors may still show up. That's fine; the ESP32 hasn't been flashed yet.
**If the upload fails with a libusb or permissions error:** unplug and replug the STLINK, then retry once. On the reference machine (Fedora, 2026-11-24) the STLINK worked with no extra setup. If it still fails, your system may need PlatformIO's udev rules; see [PlatformIO: 99-platformio-udev.rules](https://docs.platformio.org/en/latest/core/installation/udev-rules.html).
 
Paste back: the last ~10 lines of the upload, and the boot log from `Git SHA` through the WiFi AP block, **redacted** (see "Safety: boot logs contain secrets").
 
### Step 3b: Flash the ESP32
 
The ESP32-C3 is programmed over UART through a CH340 serial adapter on **J14**, using `esptool` (PlatformIO calls it for you). Two things are flashed, in separate commands:
 
1. **Filesystem image** (`uploadfs`): the config portal's web files, stored in LittleFS
2. **App** (`upload`): the ESP32 firmware
#### Before you start
 
- Flash the STM32 first (Step 3a). The STM32 powers and enables the ESP32, so the board's USB-C must stay plugged in and the STM32 must be running.
- The STLINK can stay attached to J4. It sits idle unless OpenOCD is running.
- **Don't connect the adapter's VCC pin.** The board is powered from its USB-C.
- **Every terminal needs the port variables**, including the one you flash from, not just the one running picocom. Run `source ~/ents-ports.env` in each (see Step 1a).
#### Wire the serial adapter to J14
 
**Where J14 is:** in the center of the board, next to the network components. The square pad on the silkscreen marks pin 1.
 
| J14 pin | ESP32 signal | Connects to adapter pin |
|---|---|---|
| 4 | TX (ESP32 transmits) | **RX** |
| 6 | RX (ESP32 receives) | **TX** |
| 8 | GND | GND |
| 1–2 | — | nothing |
 
**The wiring is crossover.** Each device's **TX** (transmit) goes to the other device's **RX** (receive), so one side's "talk" wire reaches the other side's "listen" wire. The common mistake is TX→TX and RX→RX. That ties both transmit pins to the same wire and leaves both receive pins listening to nothing. It does no damage, but nothing gets through, and esptool hangs on `Connecting......`. GND is not crossed; GND always goes to GND, which gives both sides a common signal reference.
 
#### Bootloader buttons
 
The ESP32 has its own **BOOT (SW4)** and **RESET (SW3)** buttons. They're next to J14, near the antenna connectors and the SD card slot. They are **not** the STM32's **SW1 RESET**, which is by the USB-C.
 
**To enter bootloader mode** (before *each* flash command):
 
1. Press and **hold SW4 BOOT**.
2. While holding it, press and release **SW3 RESET**.
3. Release SW4.
The ESP32-C3 reads its BOOT pin as it comes out of reset. If the pin is held, the chip waits for esptool instead of running its firmware.
 
#### Flash
 
**Terminal 1:** picocom on the STM32 console, logging (it can stay open the whole time):
 
```bash
source ~/ents-ports.env
picocom -b 115200 --imap lfcrlf --logfile ~/ents-$BOARD-esp32-flash-$(date +%Y%m%d-%H%M).log "$STM_CONSOLE"
```
 
**Terminal 2:**
 
```bash
source ~/ents-ports.env
cd "$FW/esp32"
```
 
Now 
1. Press and **hold SW4 BOOT**.
2. While holding it, press and release **SW3 RESET**.
3. Release SW4.

Then back in the terminal
```bash
pio run -e release -t uploadfs --upload-port "${ESP_PORT:?ESP_PORT not set}"
```

Again 
1. Press and **hold SW4 BOOT**.
2. While holding it, press and release **SW3 RESET**.
3. Release SW4.

Then back in the terminal
```bash
pio run -e release -t upload --upload-port "${ESP_PORT:?ESP_PORT not set}"
```
 
**Always pass `--upload-port`.** `esp32/platformio.ini` hard-codes `upload_port = /dev/ttyUSB1`, which may be the STM32 console on your machine. `${ESP_PORT:?}` makes the shell stop with an error if the variable is empty, instead of silently falling back to the ini's port.
 
#### What success looks like
 
- `Chip is ESP32-C3`
- **uploadfs:** `Writing at 0x00350000…`, then `Hash of data verified`. That's the LittleFS offset from `partitions_2app_reduced_fs.csv`. If it writes at `0x00290000`, an old partition table is in use, so stop.
- **upload:** writes at `0x00000000` (bootloader), `0x00008000` (partition table), `0x0000e000` and `0x00010000` (app), each followed by `Hash of data verified`
#### Troubleshooting
 
| Symptom | Cause | Fix |
|---|---|---|
| `Could not exclusively lock port /dev/ttyUSB1` | `ESP_PORT` not set in this terminal, so PlatformIO fell back to the ini's port, which picocom is holding | Run `source ~/ents-ports.env` in this terminal; use `${ESP_PORT:?}` |
| Hangs on `Connecting......`, then times out | Not in bootloader mode | Redo hold SW4 → tap SW3 → release SW4, then rerun right away |
| Still hangs after a proper bootloader entry | TX/RX not crossed | Swap the TX and RX wires at the adapter; retry once |
 
### Step 4: Verify the whole board
 
With picocom still connected to the STM32 console (Terminal 1 from Step 3b), tap **SW1 RESET** (the STM32 button by the USB-C).
 
You should see the ENTS ASCII-art banner ("Environmentally NeTworked Sensor"), then the boot log. Check all of these:
 
- [ ] **`Git SHA:` matches the commit you built.** Compare it with `git -C "$FW" rev-parse HEAD` (and with the upstream jLab SHA from Step 1b).
- [ ] **DevEUI is unchanged** from the baseline log.
- [ ] **No `ControllerTransmit() error` lines.** The STM32 is talking to the ESP32.
- [ ] **The WiFi AP block is fully filled in:** a real `ssid` (`ents-…`), `User Config http://192.168.4.1/` and a non-empty `WiFi AP MAC`.
Record the AP MAC in the board log.
 
Example of a good boot (board 3.1.0-039, redacted; your SHA, DevEUI, SSID and MAC will differ):
 
```text
ENTS LoRa-E5 STM32 firmware, compiled on Nov 24 2026 11:36:57
Git SHA: 2cccc7e10b77ea7079b3d5028550cf0bd0a4533c
 
###### AppKey:      <redacted>
###### NwkKey:      <redacted>
###### AppSKey:     <redacted>
###### NwkSKey:     <redacted>
###### DevEUI:      00:80:E1:15:06:AB:2F:9E
###### AppEUI:      01:01:01:01:01:01:01:01
###### DevAddr:     06:AB:2F:9E
Encoded Power Command (4): 0x3A020801
 
No existing user configuration in FRAM.
 
WiFi AP Info:
---------------
ssid "ents-06AB2F9E"
pass "<redacted>"
User Config http://192.168.4.1/
WiFi AP MAC: "38:44:BE:93:CA:95"
 
Requesting configuration from ESP32...
No configuration to send to ESP32!
```
 
Compare with the baseline in Step 1c: the `ControllerTransmit()` errors are gone, and `ssid`, `User Config` and `WiFi AP MAC` went from empty to filled in.
 
If you paste this log to your agent, redact it first (see "Safety: boot logs contain secrets").
 
