# COCO — The Quadruped Spider Robot 🕷️

Welcome to **COCO**, an intelligent, 3-DOF (Degrees of Freedom) per leg quadruped spider robot built on the Arduino platform. This repository contains the complete firmware, electrical circuit diagrams, 3D printing design files, and instructions to build, calibrate, and program your own walking spider robot.

---

## 👤 Author & Credits
- **Author:** Skashyam Bastakoti
- **GitHub:** [@sakxam-xeetri](https://github.com/sakxam-xeetri)

---

## 📋 Table of Contents
1. [Project Overview](#-project-overview)
2. [Hardware Architecture & Components](#-hardware-architecture--components)
3. [Pin Configuration & Wiring Guide](#-pin-configuration--wiring-guide)
4. [Inverse Kinematics (IK) & Software Architecture](#-inverse-kinematics-ik--software-architecture)
5. [Firmware Directory & Features](#-firmware-directory--features)
6. [Bluetooth Control Protocol](#-bluetooth-control-protocol)
7. [Getting Started & Calibration](#-getting-started--calibration)
8. [Libraries Installation](#-libraries-installation)
9. [Chassis & 3D Printing](#-chassis--3d-printing)

---

## 📋 Project Overview

COCO features 4 symmetrical legs, each controlled by 3 micro servos (12 servos total). By using **Inverse Kinematics (IK)**, the controller translates target 3D Cartesian coordinates $(X, Y, Z)$ of the foot tip into precise servo angles. This mathematical framework allows the robot to execute smooth, linear, and synchronized walking gaits, turn in place, perform dynamic gestures (waving, handshaking), dance, and navigate autonomously.

### Core Capabilities:
- **Calibrated Alignment:** Easy assembly alignment using the basic position routine.
- **Wireless Manual Control:** Real-time steering over Bluetooth using a customized mobile serial app.
- **Dynamic Expressiveness:** A front-facing OLED display acting as virtual eyes that change expression based on current motions (happy, sad, angry, blinking, squinting).
- **Autonomous Navigation:** Ultrasonic distance scanning to autonomously steer around obstacles in real-time.
- **Smoothed Gait Interpolation:** Real-time 50Hz timer interrupts to step the legs in straight lines without sudden motor jerks.

---

## 🛠️ Hardware Architecture & Components

To assemble COCO, you will need the following hardware components:

| Component | Description | Qty |
| :--- | :--- | :--- |
| **Microcontroller** | Arduino Uno or Arduino Nano (ATmega328P based) | 1 |
| **Actuators** | SG90 or MG90S Micro Servos (Metal gear MG90S recommended for knee joint loads) | 12 |
| **Distance Sensor** | HC-SR04 Ultrasonic Sensor (Obstacle Avoidance) | 1 |
| **Display** | 0.96" SSD1306 OLED Screen (I2C interface, 128x64 resolution) | 1 |
| **Wireless Module** | HC-05 or HC-06 Bluetooth Transceiver Module | 1 |
| **Power Supply** | 2x 18650 Li-ion batteries (7.4V) or a 2S LiPo battery pack | 1 |
| **Voltage Regulator** | High-current Buck Converter (e.g., LM2596 or XL4015) adjusted to 5V–6V (min 3A output) | 1 |
| **Frame** | 3D Printed Chassis Parts (found in `3D-Print Files.zip`) | 1 set |

> [!WARNING]  
> **Power Supply Precaution:** Do NOT power 12 servos directly from the Arduino's 5V pin. The servos will draw several amps under load, which will damage or instantly reset the Arduino. Always use a dedicated buck converter (stepped down to 5.0V - 6.0V) to feed the servo power bus, sharing a common ground with the Arduino.

---

## 🔌 Pin Configuration & Wiring Guide

COCO's electronics are mapped to the Arduino pins as detailed below. Refer to `COCO- Circuit.png` in the root folder for a visual schematic diagram.

### 1. Servo Motor Mapping
The 12 servos are mapped via a $4 \times 3$ matrix representing: `servo[Leg_ID][Joint_ID]`. 

```
                       [Front]
            Leg 1 (FL)  \     /  Leg 0 (FR)
                         \===/
                         |   |
                         |===|
            Leg 2 (BL)  /     \  Leg 3 (BR)
```

- **Leg 0 (Front-Right / FR):**
  - Alpha (Femur Joint): **Pin 3**
  - Beta (Tibia Joint): **Pin 4**
  - Gamma (Coxa Joint): **Pin 2**
- **Leg 1 (Front-Left / FL):**
  - Alpha (Femur Joint): **Pin 6**
  - Beta (Tibia Joint): **Pin 7**
  - Gamma (Coxa Joint): **Pin 5**
- **Leg 2 (Back-Left / BL):**
  - Alpha (Femur Joint): **Pin 9**
  - Beta (Tibia Joint): **Pin 8**
  - Gamma (Coxa Joint): **Pin 10**
- **Leg 3 (Back-Right / BR):**
  - Alpha (Femur Joint): **Pin 12**
  - Beta (Tibia Joint): **Pin 11**
  - Gamma (Coxa Joint): **Pin 13**

### 2. Peripheral Sensors & Communication Pins
- **HC-SR04 Ultrasonic Sensor:**
  - `Trigger` Pin $\rightarrow$ **A5 (Pin 19)**
  - `Echo` Pin $\rightarrow$ **A4 (Pin 18)**
- **SSD1306 OLED Display (I2C):**
  - `SDA` Pin $\rightarrow$ **A4** (Shared I2C SDA)
  - `SCL` Pin $\rightarrow$ **A5** (Shared I2C SCL)
  - `Address` $\rightarrow$ **`0x3C`**
- **HC-05/HC-06 Bluetooth Module:**
  - `RXD` Pin $\rightarrow$ **TX (Pin 1)** on Arduino (Use a voltage divider to drop 5V to 3.3V logic level)
  - `TXD` Pin $\rightarrow$ **RX (Pin 0)** on Arduino

---

## 📐 Inverse Kinematics (IK) & Software Architecture

### 1. Mechanical Dimensional constants:
The mechanical layout of each leg is parameterized as follows in the firmware:
- `length_a` (Femur): $50.0\text{ mm}$ (or $55.0\text{ mm}$ in autonomous sketches)
- `length_b` (Tibia): $77.1\text{ mm}$ (or $77.5\text{ mm}$ in autonomous sketches)
- `length_c` (Coxa): $27.5\text{ mm}$
- `length_side`: $71.0\text{ mm}$ (distance parameter for body geometry)

### 2. Mathematical Model:
To place the foot at a 3D coordinate $(x, y, z)$ relative to the leg joint origin:

1. **Horizontal Projection ($w$):**
   $$w = \text{sgn}(x) \cdot \sqrt{x^2 + y^2}$$
2. **Coxa-relative Extension ($v$):**
   $$v = w - \text{length\_c}$$
3. **Femur Angle ($\alpha$):**
   $$\alpha = \text{atan2}(z, v) + \text{acos}\left(\frac{\text{length\_a}^2 - \text{length\_b}^2 + v^2 + z^2}{2 \cdot \text{length\_a} \cdot \sqrt{v^2 + z^2}}\right)$$
4. **Tibia-to-Femur Angle ($\beta$):**
   $$\beta = \text{acos}\left(\frac{\text{length\_a}^2 + \text{length\_b}^2 - v^2 - z^2}{2 \cdot \text{length\_a} \cdot \text{length\_b}}\right)$$
5. **Coxa Angle ($\gamma$):**
   $$\gamma = \text{atan2}(y, x)$$

The angles are converted from radians to degrees, mapped to absolute servo orientation per leg (accounting for physical mirroring), and written to the corresponding digital pins.

```cpp
void cartesian_to_polar(volatile float &alpha, volatile float &beta, volatile float &gamma, volatile float x, volatile float y, volatile float z) {
  float v, w;
  w = (x >= 0 ? 1 : -1) * (sqrt(pow(x, 2) + pow(y, 2)));
  v = w - length_c;
  alpha = atan2(z, v) + acos((pow(length_a, 2) - pow(length_b, 2) + pow(v, 2) + pow(z, 2)) / 2 / length_a / sqrt(pow(v, 2) + pow(z, 2)));
  beta = acos((pow(length_a, 2) + pow(length_b, 2) - v * v - z * z) / (2 * length_a * length_b));
  gamma = (w >= 0) ? atan2(y, x) : atan2(-y, -x);
  
  alpha = alpha / pi * 180;
  beta = beta / pi * 180;
  gamma = gamma / pi * 180;
}
```

### 3. Smooth Interpolation Engine (Interrupt-Driven):
A crucial aspect of COCO's software is the use of `FlexiTimer2`. Rather than immediately snapping the servos to target coordinates, the loop defines target expected sites (`site_expect`). A timer interrupt runs at **50Hz** (every 20ms) executing the `servo_service` function:
- Moves coordinates (`site_now`) closer to the target values using a calculated speed multiplier in a linear pathway (linear interpolation).
- Re-calculates Inverse Kinematics for the updated point.
- Updates the servo positions continuously, guaranteeing steady, slow, and cinematic leg motions.

---

## 📂 Firmware Directory & Features

All firmware files are stored inside the `COCO Code/` directory:

*   **`basic postion.ino`**
    *   *Purpose:* Calibration script.
    *   *Features:* Sets all 12 servo channels to an absolute angle of 90 degrees. Used during mechanical assembly to mount leg horns horizontally/perpendicularly.
*   **`Bluetooth control.ino`**
    *   *Purpose:* Manual driving sketch.
    *   *Features:* Listens to serial inputs from a Bluetooth module, displays expressive facial states on the SSD1306 OLED screen, and processes direction commands.
*   **`obstcal avoding.ino`**
    *   *Purpose:* Autonomous pathfinding.
    *   *Features:* Measures front clearance using the HC-SR04. If clearance falls below 20 cm, the robot automatically backs up and selects a random direction (left/right) to steer clear of the obstacle.
*   **`repetitive motion.ino`**
    *   *Purpose:* Demonstration routine.
    *   *Features:* Executes a sequential cyclic loop of actions: Standing, Walking Forward, Stepping Back, Turning Left, Turning Right, Waving Front Legs, Shaking Hands, Sways (Dancing), and Sitting down.

---

## 📲 Bluetooth Control Protocol

When running the `Bluetooth control.ino` sketch, configure your Bluetooth controller app (such as *Arduino Bluetooth Controller* or *Serial Bluetooth Terminal*) to transmit the following ASCII character commands:

| Command | Action | OLED Facial Expression |
| :---: | :--- | :--- |
| **`F`** | Walk Forward (1 step loop) | Happy Eyes (`happy()`) |
| **`B`** | Walk Backward (1 step loop) | Sad Eyes (`triste()`) |
| **`L`** | Turn Left in place | Angry Squint Left (`enfado1()`) |
| **`R`** | Turn Right in place | Angry Squint Right (`enfado()`) |
| **`U`** | Stand Up | Default Eyes |
| **`X`** | Sit Down | Default Eyes |
| **`D`** | Sway/Body Dance | Blinking / Moving Eyes |
| **`S`** | Stop / Idle | Default Eyes |

---

## 🚀 Getting Started & Calibration

To ensure your spider robot moves correctly, follow this step-by-step commissioning checklist:

### Step 1: Mechanical Calibration (Crucial)
1. Build the physical frame, but **do not attach the servo horn arms yet**.
2. Connect all 12 servos to their respective digital pins on the Arduino board.
3. Power the Arduino and upload the **`basic postion.ino`** sketch. This drives all servos exactly to their 90-degree midpoint.
4. Keeping the board powered, carefully push the plastic leg horns onto the servo splines such that the femur and tibia segments are positioned exactly flat (90 degrees relative to each other) and parallel to the body/ground, according to assembly diagrams.
5. Fasten the servo horns securely with screws.

### Step 2: Electric Connections & Power Check
1. Ensure your high-current buck converter output is calibrated to 5.0V - 6.0V before hooking it up to the servo rails.
2. Join all ground wires together (Arduino GND, battery GND, and buck converter OUT GND).
3. Connect the Bluetooth module to the Rx/Tx serial lines. Remember to unplug the Bluetooth module's Tx/Rx line when compiling/uploading code to avoid serial collision.

### Step 3: Flash Application Code
1. Open your choice of controller sketch (`Bluetooth control.ino`, `obstcal avoding.ino`, or `repetitive motion.ino`) in the Arduino IDE.
2. Verify, compile, and upload to your board.
3. Test initial behavior. If legs move in opposite directions, verify the physical mirrored mounting orientation or tweak the `polar_to_servo` mapping arrays inside the code.

---

## 🔧 Libraries Installation

Ensure you download and install the following libraries into your Arduino libraries folder (usually under `Documents/Arduino/libraries/`) or directly via the Arduino Library Manager:

1.  **FlexiTimer2** (Available in `/library/flexitimer2-master.zip` or download online) - *Used for 50Hz servo interpolation interrupt.*
2.  **NewPing** (For HC-SR04 Ultrasonic Distance sensor) - *Ensures non-blocking distance reads.*
3.  **Adafruit SSD1306** & **Adafruit GFX** (Available in `/library/Adafruit_SSD1306-master.zip` and `/library/Adafruit-GFX-Library-master.zip`) - *Used to drive the front OLED eyes display.*

---

## 🔩 Chassis & 3D Printing

You can find the physical structural parts in the root file:
- **`3D-Print Files.zip`**: Unzip this archive to access the STL model files for printing the body plate, femur segments, coxa brackets, and tibia leg endpoints. We recommend printing with:
  - **Infill:** 20% - 30% for high mechanical durability.
  - **Material:** PLA (easy printing) or PETG (better structural strength).
  - **Supports:** Required for coxa joint cavities.

---
*COCO is designed and engineered by Skashyam Bastakoti. Join the repository to contribute updates, optimization math, or custom animation patterns.*