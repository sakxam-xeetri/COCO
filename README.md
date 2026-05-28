# COCO — The Quadruped Spider Robot 🕷️

Welcome to **COCO**, an intelligent, 3-DOF (Degrees of Freedom) per leg quadruped spider robot built on the Arduino platform. This repository contains the complete firmware, electrical circuit diagrams, 3D printing design files, and instructions to build, calibrate, and program your own walking spider robot.

---

## 👤 Author & Credits
- **Author:** Skashyam Bastakoti
- **GitHub:** [@sakxam-xeetri](https://github.com/sakxam-xeetri)

---

# COCO Spider Robot — ESP32-CAM Controller

Comprehensive, professional documentation for the COCO Spider Robot project. This README covers hardware, wiring, software structure, flashing, usage, troubleshooting, and contribution guidelines for the ESP32-CAM based web controller and the robot controller sketches contained in this repository.

---

## Table of Contents
- **Overview** — Project summary and goals
- **Highlights** — Key features and capabilities
- **Hardware** — Parts, wiring and pinout
- **Software** — Sketches, files and how they interact
- **Web Controller** — AP, endpoints, UI behavior
- **Flashing** — Arduino IDE and PlatformIO instructions
- **Usage** — Operation examples and command reference
- **Troubleshooting** — Common problems and fixes
- **Contributing** — How to help and development notes
- **License & Safety** — Legal and safety information

---

## Overview
The COCO Spider Robot repository provides firmware and documentation to control a quadruped "spider" robot. It includes two main flows:

- An ESP32-CAM web controller that creates a Wi‑Fi AP and serves a touch-optimized control UI.
- A robot controller (Arduino-style) that accepts single-character serial commands and actuates the robot's motions.

This project is designed to be modular: use the ESP32 web controller to transmit commands over TTL serial to the robot controller, or use Bluetooth/serial sketches for local control.

## Highlights
- Low-latency command transport via single-byte serial commands
- Mobile-first web UI with hold-to-move behavior and pose buttons
- Minimal dependencies — camera module not required for controller use
- Clear wiring and safe power recommendations for multi-servo installations

## Hardware
**Recommended board:** AI Thinker ESP32-CAM (or any ESP32 with accessible U0 pins)

**Essential components:**
- ESP32-CAM board
- Robot controller board (TTL serial input)
- Jumper wires, 3.3V logic compatible
- Power for servos (dedicated supply, min 3A recommended)

**Suggested wiring:**
- `ESP32 U0T / GPIO1 -> Robot RX` (ESP32 TX to robot RX)
- `ESP32 U0R / GPIO3 <- Robot TX` (optional, for robot replies)
- `GND -> Robot GND` (common ground)

**Serial settings:** `9600` baud (constant used in controller)

Safety note: never power servo banks from the ESP32's 5V pin. Use a separate regulated power rail and common ground.

## Software overview
Key files in this repository:
- `COCO Code/ESP32_CAM_Controller/ESP32_CAM_Controller.ino` — ESP32 AP and web UI that forwards commands over serial
- `COCO Code/Bluetooth-controlling_spider_robot_Robot_Lk.ino` — robot-side command handling (example)
- Other sketches in `COCO Code/` provide calibration, demo routines and autonomous behaviors

ESP32 behavior summary:
- Hosts AP `SpiderRobot` (password `12345678`) with static IP `192.168.4.1`
- Serves the control UI at `/`
- Accepts `GET /cmd?go=<CHAR>` and forwards `<CHAR>` to the robot via serial

## Web Controller (behavior and endpoints)
- Access Point: **SpiderRobot** (password `12345678`)
- Static IP: **192.168.4.1**
- Endpoints:
  - `GET /` — web UI
  - `GET /cmd?go=<CHAR>` — forward single-character command to robot

UI behavior summary:
- Hold a direction button to repeatedly send a movement command (e.g., `F`), release to send `S` (stop)
- Pose and gesture buttons send single-shot commands (e.g., `P`, `Q`, `U`)
- LED cycle button toggles `X` -> `O` -> `K` modes (off / on / blink)

## Command Reference
Supported single-character commands forwarded to the robot:
- `F` — forward
- `B` — back
- `L` — left
- `R` — right
- `S` — stop
- `P` — basic position (pose)
- `Q` — spider position (pose)
- `U` — hand shake
- `W` — hand wave
- `V` — body dance
- `O` — LED on
- `X` — LED off
- `K` — LED blink

These characters are sent verbatim with `Serial.write()` at 9600 baud.

## Flashing & Build Instructions
Arduino IDE (quick):
1. Add ESP32 board URL to Preferences: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
2. Install ESP32 boards via Board Manager and select `AI Thinker ESP32-CAM`.
3. Open `ESP32_CAM_Controller.ino`, set the correct COM port, and upload.

PlatformIO (VS Code):
Add to `platformio.ini`:
```ini
[env:esp32-cam]
platform = espressif32
board = esp32cam
framework = arduino
monitor_speed = 9600
```
Upload with `platformio run -t upload`.

Notes:
- Disconnect robot TX/RX from ESP32 while uploading to avoid serial collisions.
- Avoid debug `Serial.print` output on U0 when it is used for robot communications.

## Usage example
1. Power both robot and ESP32-CAM (ensure common ground).
2. Connect a phone to Wi‑Fi SSID `SpiderRobot` (password `12345678`).
3. Browse to `http://192.168.4.1/` and use the touch UI.
4. Optionally call commands programmatically:
```bash
curl "http://192.168.4.1/cmd?go=F"
```

## Troubleshooting
- AP not visible: verify board is powered and sketch is running
- Robot unresponsive: verify TX->RX wiring and common ground; confirm 9600 baud on robot
- Upload problems: unplug external RX/TX during upload; select correct board in Arduino IDE

## Contributing
- Fork, open feature branches, and submit PRs to `main`.
- Describe hardware changes and calibration steps in PR descriptions.

## Safety & License
- **Safety:** Keep body parts away from moving servos. Use a current-limited supply during tests. Securely fasten servo horns.
- **License:** None specified in repository. Add a `LICENSE` file (e.g., MIT) if you want to grant reuse rights.

---

If you want: wiring diagrams (SVG/PNG), a PlatformIO CI example, or an illustrated troubleshooting flowchart, say which you'd like and I will add them.

## 🔩 Chassis & 3D Printing

You can find the physical structural parts in the root file:
- **`3D-Print Files.zip`**: Unzip this archive to access the STL model files for printing the body plate, femur segments, coxa brackets, and tibia leg endpoints. We recommend printing with:
  - **Infill:** 20% - 30% for high mechanical durability.
  - **Material:** PLA (easy printing) or PETG (better structural strength).
  - **Supports:** Required for coxa joint cavities.

---
*COCO is designed and engineered by Skashyam Bastakoti. Join the repository to contribute updates, optimization math, or custom animation patterns.*