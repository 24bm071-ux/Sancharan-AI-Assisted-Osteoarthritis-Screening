# Sensor Details

This document describes the sensors used in the Sancharan multimodal osteoarthritis screening prototype.

---

# 1. MPU-9250/6500 IMU

## Purpose

Two IMUs are used to measure movement of the lower limb.

One sensor is placed on the thigh and the other on the shank.

The relative movement between the two segments can be used for future knee kinematic analysis.

## Measurements

The IMU provides:

- 3-axis acceleration
- 3-axis angular velocity
- Additional inertial sensing capabilities depending on the module configuration

The current prototype primarily uses accelerometer and gyroscope measurements.

## Configuration

| Parameter | Configuration |
|---|---|
| Number of sensors | 2 |
| Interface | I2C |
| Supply | 3.3 V |
| Thigh address | 0x68 |
| Shank address | 0x69 |
| SDA | GPIO 21 |
| SCL | GPIO 7 |

## Placement

```text
        THIGH
          │
     [ IMU 1 ]
          │
       KNEE JOINT
          │
     [ IMU 2 ]
          │
        SHANK
```

The IMUs should be securely attached to minimize unwanted movement relative to the limb.

## Potential Features

- Acceleration magnitude
- Angular velocity
- Knee angle
- Range of motion
- Movement duration
- Gait timing
- Movement symmetry
- Movement variability

---

# 2. INMP441 Digital Microphone

## Purpose

The INMP441 is used for experimental digital acoustic sensing.

It provides a digital I2S audio signal to the ESP32-S3.

## Interface

| Parameter | Configuration |
|---|---|
| Sensor | INMP441 |
| Interface | I2S |
| Sampling frequency | 16 kHz |
| Data format | 32-bit |
| Channel | Left |
| Supply | 3.3 V |

## ESP32-S3 Connections

| INMP441 | ESP32-S3 |
|---|---|
| VDD | 3.3 V |
| GND | GND |
| SCK | GPIO 4 |
| WS | GPIO 5 |
| SD | GPIO 6 |
| L/R | GND |

## Processing

The acoustic processing pipeline is:

```text
I2S Audio
    ↓
Buffering
    ↓
Signal Quality Check
    ↓
Filtering
    ↓
Windowing
    ↓
FFT / STFT
    ↓
Feature Extraction
```

Potential features include:

- RMS
- Peak amplitude
- Signal energy
- Spectral centroid
- Spectral bandwidth
- Frequency-band energy
- Spectrogram characteristics

### Important Limitation

The INMP441 is a digital MEMS microphone.

It should therefore be described in this prototype as an **acoustic sensing channel**, rather than automatically being described as a validated vibroarthrography sensor.

---

# 3. Surface EMG

## Purpose

Surface EMG measures electrical activity associated with skeletal muscle activation during movement.

It provides information that is complementary to kinematic measurements from the IMUs.

## Signal Path

```text
Muscle
  ↓
Surface Electrodes
  ↓
EMG Front-End / Amplifier
  ↓
Filtering
  ↓
ADC
  ↓
Digital Signal
  ↓
Signal Processing
```

## Processing Pipeline

```text
Raw EMG
   ↓
Band-pass Filtering
   ↓
Notch Filtering if Required
   ↓
Rectification
   ↓
Envelope Extraction
   ↓
Feature Extraction
```

## Potential Features

### Time Domain

- RMS
- Mean Absolute Value
- Integrated EMG
- Activation duration

### Frequency Domain

- Mean frequency
- Median frequency
- Frequency distribution

### Activation

- Activation onset
- Activation offset
- Activation duration
- Muscle activity pattern

## Hardware Note

The exact EMG acquisition hardware should be documented separately once the final EMG module/AFE is confirmed.

Raw EMG electrodes should not be connected directly to an ESP32 GPIO.

---

# 4. HLK-LD2402 Radar

## Purpose

The radar provides contactless movement information.

The prototype was tested using:

**HLK-LD2402 + ESP32-C3 SuperMini**

## Interface

| Parameter | Configuration |
|---|---|
| Radar | HLK-LD2402 |
| Frequency | 24 GHz class radar |
| Communication | UART |
| Baud rate | 115200 |
| Controller | ESP32-C3 SuperMini |

## Measured Information

The current firmware processes:

- Distance
- Distance change
- Radial velocity
- Movement direction
- Detection duration

## Movement Classification

The experimental firmware can classify radial movement as:

- APPROACHING
- RECEDING
- LOW_RADIAL_MOTION

## Placement

The radar is positioned near the walking/test area.

It is not intended to replace the thigh and shank IMUs for detailed knee angle measurement.

---

# 5. Multimodal Sensor Complementarity

The four sensing modalities provide different types of information.

| Modality | Main Information |
|---|---|
| IMU | Limb movement and kinematics |
| Acoustic | Acoustic activity |
| EMG | Muscle activation |
| Radar | Contactless movement |

The multimodal system investigates whether combining these complementary signals can improve functional screening.

---

# 6. Sensor Integration Architecture

```text
                  PATIENT
                     │
        ┌────────────┼────────────┐
        │            │            │
      THIGH        SHANK        MUSCLE
       IMU           IMU          EMG
        │            │            │
        └───────┬────┴────────────┘
                │
             ESP32-S3
                │
          Data Acquisition
                │
        ┌───────┴────────┐
        │                │
    Acoustic           Data
    INMP441           Transfer
        │                │
        └────────┬───────┘
                 │
        Signal Processing
                 │
        Feature Extraction
                 │
          Machine Learning
                 │
        Multimodal Analysis
```

The radar prototype is currently tested separately using an ESP32-C3.

Future versions can integrate synchronized radar data into the multimodal acquisition pipeline.

---

# 7. Design Philosophy

The prototype follows a multimodal sensing approach.

No individual sensor is assumed to diagnose osteoarthritis.

Instead, each modality contributes complementary information related to:

- Movement
- Functional performance
- Muscle activation
- Acoustic activity
- Contactless motion

The final objective is an AI-assisted functional screening profile that can be investigated against appropriate clinical reference data.
