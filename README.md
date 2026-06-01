# PixMob XIAO ESP32-C6 Controller

Control PixMob "cement V1.1" wristbands using a Seeed XIAO ESP32-C6 and an E07-M1101D-SMA CC1101 RF module, via a web interface over WiFi.

Based on [MajedAbouhatab/Controlling-PIXMOB-Waveband-with-WeMos-D1-Mini-and-CC1101](https://github.com/MajedAbouhatab/Controlling-PIXMOB-Waveband-with-WeMos-D1-Mini-and-CC1101), ported and fixed for the XIAO ESP32-C6.

---

## Hardware

| Component | Model |
|-----------|-------|
| Microcontroller | Seeed XIAO ESP32-C6 |
| RF module | EBYTE E07-M1101D-SMA (CC1101, 868 MHz) |
| Antenna | 868 MHz LoRa antenna (SMA) |

### Wiring

| CC1101 pin | XIAO ESP32-C6 pin |
|------------|-------------------|
| GDO0 | D0 |
| MOSI | D10 |
| SCK | D8 |
| MISO | D9 |
| CSN | D4 |
| VCC | 3.3V |
| GND | GND |

---

## Software

### Arduino IDE board settings

| Setting | Value |
|---------|-------|
| Board | XIAO_ESP32C6 |
| USB CDC On Boot | Enabled |
| CPU Frequency | 160MHz (WiFi) |

### Required libraries

Install via Arduino IDE Library Manager (Sketch → Include Library → Manage Libraries):

| Library | Author | Notes |
|---------|--------|-------|
| ELECHOUSE_CC1101_SRC_DRV | LSatan | Search: `CC1101` |
| Arduino_JSON | Arduino | Search: `Arduino_JSON` |

Install manually via ZIP (Sketch → Include Library → Add .ZIP Library):

| Library | URL | Notes |
|---------|-----|-------|
| ESPAsyncWebServer | https://github.com/ESP32Async/ESPAsyncWebServer | Download ZIP |
| AsyncTCP | https://github.com/ESP32Async/AsyncTCP | Download ZIP |

---

## Setup

1. Clone this repository
2. Open `src/pixmob.ino` in Arduino IDE
3. Edit the WiFi credentials at the top of `pixmob.ino`:
```cpp
const char* WIFI_SSID = "jouw_wifi_naam";
const char* WIFI_PASS = "jouw_wifi_wachtwoord";
```
4. Install all required libraries (see above)
5. Select board: Tools → Board → XIAO_ESP32C6
6. Upload the sketch
7. Open Serial Monitor at 115200 baud to find the IP address
8. Open the IP address in a browser

---

## Usage

The web interface shows sliders for:

| Slider | Range | Description |
|--------|-------|-------------|
| Red | 0–255 | Red channel |
| Green | 0–255 | Green channel |
| Blue | 0–255 | Blue channel |
| Attack | 0–7 | Fade in speed |
| Hold | 0–7 | Hold time (0 = background) |
| Release | 0–7 | Fade out speed (7 = forever) |
| Random | 0–7 | Randomness (0 = straight, 7 = extreme) |
| Group | 0–31 | Target group (0 = all wristbands) |

---

## Technical notes

### ESP32-C6 SPI fix

The ELECHOUSE CC1101 library's `getCC1101()` detection does not work correctly on the ESP32-C6 because the library reinitializes SPI internally with wrong default pins. The fix in `pixmob_cement.cpp` explicitly calls `SPI.begin()` with the correct pins before the library init, and skips the `getCC1101()` check entirely.

### Frequency

PixMob "cement V1.1" wristbands in the EU operate on **868.49 MHz**. The E07-M1101D-SMA module supports 779–928 MHz and covers this frequency.

### Range

Expected range is approximately 1–5 meters depending on antenna and environment. The CC1101 transmits at 10 dBm (10 mW) maximum. For greater range, an external 868 MHz PA module can be added between the CC1101 and antenna.

---

## Credits

- Original PixMob protocol reverse engineering and driver: [Sueppchen and Serge-45](https://github.com/MajedAbouhatab/Controlling-PIXMOB-Waveband-with-WeMos-D1-Mini-and-CC1101)
- Web interface: [MajedAbouhatab](https://github.com/MajedAbouhatab)
- XIAO ESP32-C6 port and SPI fix: this repository

---

## License

BSD — see original repository for license details.
