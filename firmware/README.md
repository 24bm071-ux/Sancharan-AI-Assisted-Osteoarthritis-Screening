# Firmware

This folder contains the proof-of-concept firmware developed for validating the sensing modules used in the Sancharan multimodal osteoarthritis screening system.

## 1. IMU + Acoustic Sensor Firmware

**File:** `imu_acoustic_test.ino`

This firmware runs on the **ESP32-S3** and interfaces with two MPU-9250/6500 IMUs and an INMP441 digital acoustic sensor.

The two IMUs are placed on the **thigh and shank** to capture acceleration and angular velocity during knee movement. The INMP441 captures digital acoustic signals generated during movement. The firmware reads and displays the sensor data through the Serial Monitor for initial hardware and signal validation.

**Main outputs:**
- Thigh accelerometer and gyroscope data
- Shank accelerometer and gyroscope data
- Acoustic sensor samples
- Timestamp for each acquisition cycle

The collected data will later be processed to extract movement, knee kinematic, gait, and acoustic features for machine-learning analysis.

---

## 2. Radar Firmware

**File:** `radar_test.ino`

This firmware runs on an **ESP32-C3 SuperMini** and interfaces with the **HLK-LD2402 24 GHz radar module** through UART.

The radar is used for contactless detection of human movement. The firmware receives distance information from the radar, applies basic filtering, calculates radial movement, and classifies the detected movement into simple states such as **approaching, receding, or low radial motion**.

**Main outputs:**
- Detection status
- Distance from radar
- Filtered distance
- Estimated radial velocity
- Movement direction/state
- Detection duration and related test parameters

The radar data will later be investigated as an additional complementary modality for movement assessment in the multimodal osteoarthritis screening system.

---

### Firmware Status

Both programs are currently **proof-of-concept sensor validation firmware**. They are intended to confirm hardware communication and basic data acquisition before implementing synchronized multimodal recording, signal processing, feature extraction, and AI-based analysis.
