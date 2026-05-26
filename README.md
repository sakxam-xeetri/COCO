# COCO - Quad Spider Arduino-Based Robot 🕷️

Welcome to **COCO**, the Quad Spider Robot project! This repository contains the complete codebase and instructions for building, programming, and operating a versatile quadruped spider robot powered by Arduino.

## 👤 Author & Credits
- **Author:** skashyam bastakoti
- **GitHub:** [sakxam-xeetri](https://github.com/sakxam-xeetri)

## 📋 Project Overview
This project features an intelligent 4-legged spider robot with 3 servos per leg (12 servos total), allowing for dynamic movement and advanced features like obstacle avoidance and Bluetooth control. The code is modularly broken down into multiple `.ino` scripts to test different functionalities independently.

## 📂 File Structure and Core Features

- **`basic postion.ino`**
  Used to calibrate and set the robot into its default resting/standing posture. Ideal for initial servo alignment during assembly.

- **`Bluetooth-controlling_spider_robot_Robot_Lk.ino`**
  Provides remote control functionality over Bluetooth. You can connect to your robot using a mobile app to steer it in real-time. This sketch also supports an OLED display (`Adafruit_SSD1306`) for visual feedback or expressive "eyes".

- **`obstcal avoding.ino`**
  Implements autonomous navigation! Utilizes an Ultrasonic Distance Sensor (`NewPing.h`) to detect objects in the robot's path and autonomously walk around them.

- **`repetitive motion.ino`**
  Contains hardcoded sequences for cyclic animations, leg stretches, and repetitive walking cycles using `FlexiTimer2` to synchronize multi-servo movements smoothly.

## 🛠️ Hardware Requirements
- **Microcontroller:** Arduino Uno/Nano/Mega (Depending on available pins and extensions)
- **Actuators:** 12x Micro Servos (e.g., SG90 or MG90S)
- **Power Supply:** High-current battery pack (LiPo or 18650s) and a reliable step-down buck converter to power all servos safely.
- **Sensors:** Ultrasonic Distance Sensor (HC-SR04)
- **Connectivity:** HC-05 / HC-06 Bluetooth Module
- **Optional:** OLED display (I2C) for visual feedback
- **Chassis:** 3D Printed or Laser-cut Quadruped Robot Frame

## 🔧 Libraries Used
Make sure you install the following libraries via the Arduino Library Manager before compiling:
- `Servo.h` (Built-in)
- `FlexiTimer2.h`
- `NewPing.h` (For Ultrasonic Sensor)
- `Wire.h` (Built-in I2C)
- `Adafruit_SSD1306.h` & `Adafruit_GFX.h` (For OLED Display)

## 🚀 How to Use / Setup Instructions
1. **Calibration:** Upload `basic postion.ino` and securely attach all servo horns at the specified straight angles. Calibration is an essential first step.
2. **Wiring:** Wire the system matching your circuit diagram. Ensure correct logic levels for TX/RX if using Bluetooth and provide a dedicated power supply for the servos (do NOT power 12 servos directly from the Arduino 5V pin!).
3. **Run Code:** 
   - Want it to roam freely exploring? Upload `obstcal avoding.ino`.
   - Prefer manual control over your phone? Upload the `Bluetooth-controlling` sketch.
4. **Bluetooth Remote:** Configure your Bluetooth app to send your steering buttons to the HC-05/HC-06 receiver.

---
*Developed by skashyam bastakoti.*