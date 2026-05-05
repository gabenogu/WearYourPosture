# 🧠 WearYourPosture

WearYourPosture is an embedded wearable device designed to improve human posture in real-time using sensor feedback and haptic alerts. It continuously monitors upper body alignment and provides immediate feedback when poor posture is detected, helping users develop healthier posture habits over time.

---

## 📌 Overview

Poor posture is a widespread issue among students, office workers, and gamers who spend long hours sitting. WearYourPosture addresses this by combining embedded sensing, real-time processing, and user feedback into a compact wearable system.

The device uses motion sensors to detect body orientation and determines whether the user is maintaining proper posture. When slouching or incorrect alignment is detected for a sustained period, the system triggers a vibration or alert to prompt correction.

---

## ⚙️ Features

* 📡 Real-time posture monitoring using IMU/accelerometer sensors
* 📉 Detection of slouching vs upright posture
* 🧠 Adjustable sensitivity thresholds for different users
* 🔋 Low-power wearable design for extended use
* 📊 Optional data logging for posture tracking and analysis

---

## 🧩 System Architecture

The system is built around a simple embedded pipeline:

1. **Sensors (IMU / Accelerometer)** capture body orientation data
2. **Microcontroller** processes sensor input in real time
3. **Posture algorithm** compares readings to calibrated baseline posture
4. If deviation exceeds threshold → **trigger feedback system**
5. Optional: data is logged or transmitted for further analysis

---

## 🛠️ Hardware Components

* Microcontroller (ESP32)
* IMU Sensor (MPU6050)
* Battery pack for portability
* Wearable shirt with sensors embedded

---

## 💻 Software Overview

The firmware is written in C++ for embedded systems and includes:

* Sensor initialization and calibration
* Orientation calculation and filtering (Kalman Filter)
* Threshold-based posture classification
* Timing logic to prevent false positives (sustained slouch detection)

---

## 🧪 How It Works

1. The device is worn on the upper back or shoulder region
2. On startup, the system calibrates a baseline "good posture" position
3. Continuous sensor readings track body orientation
4. If posture deviates beyond the allowed threshold:

   * A timer starts to confirm sustained bad posture
6. Once posture is corrected:

   * Feedback stops automatically

---

## 📊 Example Logic

* Upright posture → angle within acceptable range → no alert
* Slouching detected → angle exceeds threshold
* Sustained slouch (> X seconds) → vibration alert triggered
* Correct posture restored → system resets timer

---

## 🚀 Future Improvements

* Machine learning-based posture classification
* Miniaturized PCB for full wearable integration
* Energy optimization for all-day battery life
* Cloud-based posture analytics over time

---

## 🎯 Motivation

This project was created to address a real-world health issue: poor posture caused by prolonged sitting. By providing immediate feedback, WearYourPosture encourages users to develop awareness and long-term improvement in posture habits, reducing the risk of chronic back and neck pain.

---

## 📁 Repository Structure

```
WearYourPosture/
│
├── components/        # Embedded C++ source code
├── frontend/        # Circuit diagrams and schematics
├── main/            # Documentation and design notes
└── README.md        # Project documentation
```

---

## 📜 License

This project is open-source and can be modified or distributed under the MIT License.

---

## 👨‍💻 Author

Developed as an embedded systems project focused on real-time sensing, signal processing, and wearable device design.
