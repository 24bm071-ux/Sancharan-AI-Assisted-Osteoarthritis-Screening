# Methodology

## 1. Objective

The Sancharan system is designed as an AI-assisted multimodal screening platform for investigating functional signatures associated with knee osteoarthritis.

The system combines movement, acoustic, neuromuscular and contactless sensing.

It is intended for research and screening support rather than definitive clinical diagnosis.

---

# 2. Patient Registration

The software can collect basic information such as:

- Participant ID
- Age
- Sex
- Height
- Weight
- Activity level
- Pain score
- Stiffness information
- Mobility information
- Relevant clinical history

Identifying information should be minimized in research datasets.

---

# 3. Functional Assessment

The participant performs standardized movement tasks.

Possible tasks include:

### Task 1 — Walking

The participant walks along a defined path.

Sensors capture:

- Thigh movement
- Shank movement
- Muscle activity
- Acoustic activity
- Contactless movement information

### Task 2 — Sit-to-Stand

The participant repeatedly transitions from sitting to standing.

This provides information about:

- Movement timing
- Acceleration
- Angular velocity
- Muscle activation

### Task 3 — Knee Flexion/Extension

The participant performs controlled knee movements.

The system records movement and signal characteristics.

---

# 4. Sensor Acquisition

## IMU

Two IMUs are placed on the thigh and shank.

The sensors provide:

- Acceleration
- Angular velocity

The relative movement of the segments can be analyzed to estimate knee movement.

---

## Acoustic

The INMP441 captures digital acoustic samples through the I2S interface.

The acquired signal can be investigated in:

- Time domain
- Frequency domain
- Spectrogram representation

---

## EMG

Surface EMG is acquired using an appropriate biomedical front-end.

The signal is processed using filtering, rectification and envelope extraction.

---

## Radar

The radar module provides contactless movement/presence information through UART.

It can provide complementary information during movement tests.

---

# 5. Signal Processing

Different modalities require different processing pipelines.

## IMU pipeline

```text
Raw acceleration + gyroscope
            ↓
        Calibration
            ↓
          Filtering
            ↓
    Orientation estimation
            ↓
     Relative movement
            ↓
    Gait/motion features
