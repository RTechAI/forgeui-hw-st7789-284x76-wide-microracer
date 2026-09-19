# ForgeUI MicroRacer — ESP32-S3 + ST7789 284×76

ForgeUI MicroRacer is an official ForgeUI Hardware Lab project developed by [RTechAI](https://github.com/RTechAI): a physically tested, joystick-controlled arcade racing and embedded-graphics showcase for the documented ESP32-S3 + ST7789 284×76 wide-display hardware.

It builds on the physically proven [ForgeUI ST7789 284×76 golden baseline](https://github.com/RTechAI/forgeui-hw-st7789-284x76-wide) and belongs to the wider ForgeUI ESP32 hardware project ecosystem.

Physical Hardware Lab validation does not by itself mean this display target is currently integrated into ForgeUI Studio.

![ForgeUI MicroRacer physical game lifecycle](splash_Microracer.png)

**PHYSICAL GAME PASS — ESP32-S3 + ST7789 + ANALOG JOYSTICK**

The hero photo is physical evidence of the complete game lifecycle: **TITLE/START → ACTIVE GAMEPLAY (traffic, HUD, and boost) → CRASH → GAME OVER/RELAUNCH**.

## Gameplay

Start from the ForgeUI MicroRacer title screen with a joystick press, move the player through traffic, hold the button for boost, and race for a higher score. A collision triggers the crash animation, followed by the game-over screen with the current and best scores. Press the joystick again to race again.

The physically proven implementation includes:

- ForgeUI MicroRacer title screen and joystick press edge to start or restart
- Startup joystick centre calibration from 64 samples and a 180-count dead zone
- Joystick X/Y movement within the road area
- Automatically increasing speed from approximately 1.8 to a cap of 4.7
- Full-screen 16-bit `TFT_eSprite` framebuffer rendering
- Scrolling road and lane markers
- Five traffic cars
- Score increases during play, with +100 for passed/recycled traffic; best score persists for the current boot session
- Hold the joystick button to boost: current speed is multiplied by 1.65 while active, the meter drains while active, and recharges otherwise
- Collision detection, 18 crash particles, and an approximately 1.4-second CRASH state
- GAME OVER state and press-to-race-again flow
- Approximately 30 FPS target timing

## Controls

| Input | Action |
|---|---|
| Joystick X/Y | Steer the car |
| Joystick press | Start a race, activate boost while racing, or restart after game over |

## Physical Joystick Proof

![ForgeUI MicroRacer joystick validation](splash_joystick-proof.png)

The analog joystick was independently validated on the physical hardware before integration into MicroRacer. This physical diagnostic shows X/Y readings, switch state, and a position indicator, with display stability also verified.

### Final proven joystick mapping

| Joystick | ESP32-S3 |
|---|---:|
| SW | GPIO4 |
| VRy | GPIO5 |
| VRx | GPIO6 |
| +5V-labelled supply | 3.3V |
| GND | GND |

The joystick module's **+5V-labelled supply pin is intentionally powered from the ESP32-S3 3.3V rail** in this project. GPIO7 remains spare.

## Hardware and display wiring

### MCU

- ESP32-S3 DevKitC-1
- ESP32-S3 silicon revision v0.2
- 8 MB embedded PSRAM physically detected

### Display

- Controller: ST7789
- 2.25-inch IPS TFT
- Native resolution: 76×284
- Game orientation: 284×76 landscape
- SPI at 27 MHz

### Physically proven display wiring

| TFT display | ESP32-S3 | Function |
|---|---:|---|
| GND | GND | Ground |
| VCC | 3.3V | Display power |
| SCL / SCLK | GPIO12 | SPI clock |
| SDA / MOSI | GPIO11 | SPI data |
| RST | GPIO10 | Reset |
| DC | GPIO9 | Data / command |
| CS | GPIO8 | Chip select |
| BL | GND | Active-low backlight enable |
| MISO | Unused | — |

The tested display module has an active-low backlight input, so BL is connected to GND. Other ST7789 modules may use different backlight circuitry.

## Proven display configuration

| Setting | Proven value |
|---|---|
| Driver | ST7789 |
| Native geometry | 76×284 |
| Landscape viewport | 284×76 |
| Rotation | 1 |
| Display inversion | `false` |
| SPI frequency | 27 MHz |
| MOSI / SCLK | GPIO11 / GPIO12 |
| CS / DC / RST | GPIO8 / GPIO9 / GPIO10 |
| Backlight | Active-low, connected to GND |

## Supporting original display bring-up proof

![ForgeUI ST7789 284×76 physical display validation](splash_st7789-284x76-wide.png)

This earlier physical result records the validated display wiring, orientation, and ST7789 configuration that underpin MicroRacer. It is supporting physical hardware evidence for the underlying ST7789 284×76 display baseline; see the related hardware-reference project below.

## Software and build information

This project uses PlatformIO with the Arduino framework for ESP32. The physically proven configuration is held in `platformio.ini`; the game implementation is in `src/main.cpp`.

## ForgeUI Hardware Lab

ForgeUI Hardware Lab is an RTechAI/ForgeUI collection of physically tested ESP32 boards, displays, peripherals, examples, and experimental projects. It establishes reproducible hardware baselines through hardware identification, minimal bring-up, physical proof, and preservation of known-good configurations. Demonstrations and candidate targets can then be evaluated for future ForgeUI Studio workflows.

This project's physical validation does not by itself indicate that the ST7789 284×76 target is currently integrated into ForgeUI Studio.

## Related ForgeUI Projects

- [Golden ST7789 284×76 Wide Display](https://github.com/RTechAI/forgeui-hw-st7789-284x76-wide) — known-good physical ESP32-S3/ST7789 284×76 baseline.
- [ForgeUI MicroDash](https://github.com/RTechAI/forgeui-hw-st7789-284x76-wide-microdash) — compact dashboard/instrumentation graphics showcase for the same display family.
- [ForgeUI Tunnel Run](https://github.com/RTechAI/forgeui-hw-st7789-284x76-wide-tunnelrun) — joystick-controlled procedural tunnel/arcade graphics showcase for the same display family.

## About ForgeUI

ForgeUI is developed by [RTechAI](https://github.com/RTechAI). [ForgeUI](https://forgeui.co.nz) is the wider project and product site, and [ForgeUI Studio](https://studio.forgeui.co.nz) is a visual embedded UI/HMI development environment for supported ESP32 hardware. ForgeUI Hardware Lab preserves physically tested hardware references, demonstrations, and reproducible evidence for evaluating future ForgeUI workflows.

[ForgeUI Hosted Studio](https://studio.forgeui.co.nz) is available for public registration.

## Third-party dependency attribution

This project depends on [TFT_eSPI-ST7789-76x284](https://github.com/atoomnetmarc/TFT_eSPI-ST7789-76x284.git), maintained by atoomnetmarc. It is an external third-party dependency that provides support for this unusual ST7789 panel geometry. ForgeUI does not claim ownership of that library or fork; it retains its own copyright and licence terms.

## License and repository scope

Unless otherwise noted, ForgeUI-authored source and documentation in this repository are available under the [MIT License](LICENSE). Third-party dependencies and reference implementations remain subject to their respective licences and copyright.
