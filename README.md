# Aperio

*Quod Latet* — ESP32 firmware for a handheld Wi-Fi diagnostic tool. It drives
an ILI9341 TFT over hardware SPI and is navigated entirely with three
push buttons, exposing a menu-driven UI for scanning networks, monitoring
802.11 traffic, inspecting management frames, and transmitting test frames.

## Features

- **Menu-driven UI** rendered on the ILI9341, navigated with three buttons
  (previous / select / next). A long press on **select** acts as *back*.
- **Wi-Fi**
  - **Scan** — list nearby networks (SSID, BSSID, RSSI, channel).
  - **Networks** — browse the last scan and drill into per-network detail.
  - **Channels** — per-channel statistics from the scan.
  - **Monitor** — puts the radio in promiscuous mode:
    - *Passive Monitor* — live frame counters (total, management / control /
      data, beacons, probe requests/responses, deauth, disassociation) and a
      frames-per-second rate, lockable to a single channel.
    - *Management Events* — a rolling history of captured management frames
      (probe request/response, beacon, disassociation, deauthentication) with
      a per-event detail view. A small per-BSSID channel cache lets events be
      attributed to the AP's home channel across channel hops.
    - *Active Survey* — placeholder.
  - **TX Lab** — a raw-frame transmitter for authorized testing: crafts and
    sends probe-request and deauthentication frames on a selectable channel,
    tracking attempted / sent / failed counts.
- **Bluetooth** and **System** entries are present but currently placeholders.

> ⚠️ The TX Lab transmits deauthentication and probe frames. Use it only on
> networks and devices you own or are explicitly authorized to test.
> Transmitting deauth frames against others' networks is illegal in many
> jurisdictions.

## Hardware

- ESP32 dev board (esp32dev / WROOM-32)
- ILI9341 TFT display module (240x320, SPI)
- 3 momentary push buttons (previous / select / next)

### Wiring

Display (SPI uses the ESP32's default VSPI pins):

| Display pin | ESP32 pin | Notes                      |
| ----------- | --------- | -------------------------- |
| VCC         | 3V3       |                            |
| GND         | GND       |                            |
| CS          | GPIO5     |                            |
| RESET       | GPIO27    |                            |
| D/C         | GPIO2     |                            |
| SDI (MOSI)  | GPIO23    | default VSPI MOSI          |
| SCK         | GPIO18    | default VSPI SCK           |
| SDO (MISO)  | GPIO19    | default VSPI MISO          |
| LED         | 3V3       | backlight, always on       |

Buttons (wired to ground, using the internal pull-ups — active low):

| Button   | ESP32 pin | Role                     |
| -------- | --------- | ------------------------ |
| Previous | GPIO25    | move selection up / left |
| Select   | GPIO26    | confirm; hold = back     |
| Next     | GPIO32    | move selection down/right|

All pins are defined in [`include/Config.hpp`](include/Config.hpp).

If the display sits on a breadboard and the panel comes up blank or garbled,
drop `TFT_SPI_FREQUENCY` in `Config.hpp` (currently 20MHz) — jumper wires
introduce enough signal noise at higher clocks to corrupt the init sequence.
Also double check that the breadboard's power rails are electrically
continuous end to end; full-size breadboards commonly split each rail in the
middle, and a `GND` split between the ESP32 and the display causes the exact
same symptom.

## Project layout

```
aperio-firmware/
├── include/
│   ├── AperioApp.hpp         # top-level app: owns input, screens and Wi-Fi
│   ├── Config.hpp            # pin map, SPI frequency, version string
│   ├── input/                # Button + InputManager (debounce, long-press)
│   ├── ui/                   # Screen, ScreenManager, MenuItem, screens/
│   └── wifi/                 # scanner, monitor, transmitter + data types
├── src/                      # implementations mirroring include/
│   └── main.cpp              # entry point: wires up AperioApp, runs loop
├── lib/                      # private, project-specific libraries (empty)
├── test/                     # PlatformIO unit tests (empty)
├── platformio.ini            # build, upload and monitor configuration
├── LICENSE
└── README.md
```

The firmware is structured around a `ScreenManager` that owns a stack of
`Screen` objects; `AperioApp` routes button events to the active screen and
drives `WifiScanner`, `WifiMonitor` and `WifiTransmitter`. The promiscuous-mode
counters in `WifiMonitor` are updated from the Wi-Fi driver's callback (a
separate task) and read from the UI loop, so they are guarded by FreeRTOS
spinlocks.

## Building and flashing

This project uses [PlatformIO](https://platformio.org/).

```bash
# Build
pio run

# Upload to the board
pio run -t upload

# Upload and immediately open the serial monitor (catches the boot log)
pio run -t upload -t monitor

# Open the serial monitor on its own
pio device monitor
```

`upload_port`, `monitor_port` and `monitor_speed` are pinned in
`platformio.ini` to `/dev/ttyUSB0` and 115200 baud. Without
`monitor_speed` set explicitly, `pio device monitor` falls back to 9600
baud and prints garbage against firmware that talks at 115200.

## License

MIT — see [LICENSE](LICENSE).
