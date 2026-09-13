# Aperio

*Quod Latet* — ESP32 firmware that drives an ILI9341 TFT display over hardware SPI.

## Hardware

- ESP32 dev board (esp32dev / WROOM-32)
- ILI9341 TFT display module (240x320, SPI)

### Wiring

| Display pin | ESP32 pin | Notes                          |
| ----------- | --------- | ------------------------------ |
| VCC         | 3V3       |                                 |
| GND         | GND       |                                 |
| CS          | GPIO5     |                                 |
| RESET       | GPIO27    |                                 |
| D/C         | GPIO2     |                                 |
| SDI (MOSI)  | GPIO23    | default VSPI MOSI              |
| SCK         | GPIO18    | default VSPI SCK               |
| SDO (MISO)  | GPIO19    | default VSPI MISO              |
| LED         | 3V3       | backlight, always on           |

If the display sits on a breadboard, keep the SPI clock conservative (this
project uses 8MHz instead of the ESP32 default of 40MHz) — jumper wires
introduce enough signal noise at 40MHz to corrupt the init sequence and
leave the panel blank. Also double check that the breadboard's power rails
are electrically continuous end to end; full-size breadboards commonly
split each rail in the middle, and a `GND` split between the ESP32 and the
display causes the exact same symptom.

## Project layout

```
aperio-firmware/
├── include/          # project header files (empty)
├── lib/              # private, project-specific libraries (empty)
├── src/
│   └── main.cpp      # entry point: initializes the display and draws the title screen
├── test/             # PlatformIO unit tests (empty)
├── platformio.ini    # build, upload and monitor configuration
├── LICENSE
└── README.md
```

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

On boot, the firmware prints the ILI9341's diagnostic registers over
serial (power mode, MADCTL, pixel/image format, self-diagnostic) as a
quick hardware sanity check — a self-diagnostic value of `0xC0` indicates
a healthy panel.

## License

MIT — see [LICENSE](LICENSE).
