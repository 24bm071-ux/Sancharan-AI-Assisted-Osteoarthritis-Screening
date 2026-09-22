# Experimental Methodology

## 1. Overview

Sancharan is an AI-assisted multimodal sensing system designed to investigate functional movement patterns associated with knee osteoarthritis (OA).

The prototype combines four sensing modalities:

- Inertial Measurement Units (IMUs)
- Acoustic sensing
- Surface Electromyography (EMG)
- 24 GHz radar sensing

The system is designed as a functional screening and research platform rather than a standalone diagnostic device.

---

## 2. System Objective

The objective is to investigate whether complementary information from movement, acoustic activity, muscle activation and contactless motion can be combined to generate an OA-related functional screening profile.

The overall pipeline is:

**Patient → Clinical/Functional Information → Multimodal Sensor Acquisition → Signal Processing → Feature Extraction → Machine Learning → Multimodal Fusion → Screening Profile**

---

## 3. Multimodal Sensing Strategy

| Modality | Sensor | Main Information |
|---|---|---|
| Kinematic | 2 × MPU-9250/6500 IMU | Thigh and shank movement |
| Acoustic | INMP441 | Digital acoustic signal |
| Neuromuscular | Surface EMG | Muscle activation |
| Contactless motion | HLK-LD2402 | Distance and radial movement |

Each modality provides complementary information about functional movement.

---

## 4. Patient Registration

Before sensor acquisition, each participant is assigned a study identifier.

Example:

`P001`

The experimental record may contain:

- Participant ID
- Age group
- Sex, if required
- Experimental session
- Test type
- Sensor configuration
- Date and time of experiment

Patient-identifying information should not be included in public datasets.

---

## 5. Experimental Tasks

The prototype can be evaluated using standardized functional tasks.

### 5.1 Static Standing

The participant stands in a stable position.

Purpose:

- Check sensor stability
- Establish baseline signals
- Observe static posture

### 5.2 Knee Flexion and Extension

The participant performs controlled knee movement.

Purpose:

- Measure thigh and shank movement
- Observe muscle activation
- Record acoustic activity
- Evaluate sensor synchronization

### 5.3 Walking

The participant walks along a defined path.

Purpose:

- Evaluate gait-related movement
- Observe thigh and shank motion
- Measure muscle activation
- Evaluate contactless movement using radar

### 5.4 Sit-to-Stand

The participant moves from sitting to standing.

Purpose:

- Evaluate functional lower-limb movement
- Observe muscle activation
- Study movement duration and acceleration patterns

---

# 6. IMU Acquisition

Two IMUs are used in the prototype.

- IMU 1 → Thigh
- IMU 2 → Shank

Both sensors communicate with the ESP32-S3 through the I2C bus.

The two sensors use different I2C addresses:

| Sensor | Location | I2C Address |
|---|---|---|
| IMU 1 | Thigh | 0x68 |
| IMU 2 | Shank | 0x69 |

The accelerometer provides:

- Ax
- Ay
- Az

The gyroscope provides:

- Gx
- Gy
- Gz

---

## 7. IMU Processing

The raw IMU signals are processed before feature extraction.

The general processing pipeline is:

**Raw Accelerometer → Offset Correction → Filtering → Acceleration Features**

**Raw Gyroscope → Offset Correction → Filtering → Angular Velocity Features**

The relative movement of the thigh and shank can be used for future estimation of knee movement.

Potential features include:

- Knee angle
- Range of motion
- Angular velocity
- Acceleration magnitude
- Gait cycle duration
- Step duration
- Stride duration
- Movement symmetry
- Movement variability

---

# 8. Acoustic Acquisition

The INMP441 digital MEMS microphone is connected to the ESP32-S3 using I2S.

Current prototype configuration:

| Parameter | Configuration |
|---|---|
| Sampling frequency | 16 kHz |
| Data format | 32-bit I2S |
| Channel | Left |
| Interface | I2S |

The current prototype demonstrates digital acoustic acquisition.

For detailed research analysis, continuous buffered acoustic signals should be recorded instead of isolated samples.

---

# 9. Acoustic Signal Processing

The proposed acoustic processing pipeline is:

**Raw I2S Signal → Signal Quality Check → Filtering → Windowing → FFT/STFT → Time-Frequency Analysis → Feature Extraction**

Potential features include:

- RMS amplitude
- Peak amplitude
- Signal energy
- Spectral centroid
- Spectral bandwidth
- Frequency-band energy
- Spectrogram characteristics

The current acoustic modality is treated as an experimental acoustic sensing channel.

---

# 10. EMG Acquisition

Surface EMG is used to observe muscle activation during functional movement.

The general signal pathway is:

**Surface Electrodes → EMG Amplifier/AFE → Filtering → ADC/Acquisition → Digital Signal → Signal Processing**

Raw EMG electrodes must not be connected directly to an ESP32 GPIO.

An appropriate biomedical EMG front-end is required.

---

# 11. EMG Signal Processing

The EMG processing pipeline is:

**Raw EMG → Band-pass Filtering → Notch Filtering if Required → Full-wave Rectification → Envelope Extraction → Feature Extraction**

The processed EMG signal can be visualized as:

1. Raw EMG
2. Band-pass filtered EMG
3. Rectified EMG
4. EMG envelope

Potential EMG features include:

### Time-Domain Features

- RMS
- Mean Absolute Value
- Integrated EMG
- Activation duration

### Frequency-Domain Features

- Mean frequency
- Median frequency
- Frequency distribution

### Activation Features

- Muscle activation onset
- Muscle activation offset
- Activation duration
- Co-contraction characteristics

---

# 12. Radar Acquisition

The radar prototype uses an HLK-LD2402 24 GHz radar module.

The current radar test configuration is:

**HLK-LD2402 + ESP32-C3 SuperMini**

Communication is performed using UART.

| Parameter | Configuration |
|---|---|
| Communication | UART |
| Baud rate | 115200 |
| Sensor | HLK-LD2402 |
| Controller | ESP32-C3 SuperMini |

The radar is positioned near the walking area rather than being attached directly to the knee.

---

# 13. Radar Processing

The current firmware processes radar distance information.

The general processing pipeline is:

**Radar → UART Data → Distance Extraction → Distance Filtering → Distance Change → Radial Velocity → Movement Classification**

The prototype can identify movement states such as:

- APPROACHING
- RECEDING
- LOW_RADIAL_MOTION

Potential experimental features include:

- Distance
- Distance variation
- Radial velocity
- Detection duration
- Movement direction
- Temporal movement characteristics

The radar provides complementary contactless movement information and is not treated as a direct measurement of knee joint kinematics.

---

# 14. Signal Synchronization

The four sensing modalities operate at different sampling rates.

Therefore, timestamps are required for multimodal analysis.

The intended synchronization architecture is:

**IMU + Acoustic + EMG + Radar → Timestamping → Time Alignment → Common Experimental Window**

Future versions should use continuous buffered acquisition and synchronized timestamps.

---

# 15. Signal Quality Control

Before feature extraction, each recording should be checked for:

- Missing samples
- Sensor disconnection
- Signal saturation
- Excessive noise
- Motion artifacts
- Incorrect sensor placement
- Communication errors

Poor-quality segments should be identified and handled according to predefined criteria.

---

# 16. Feature Extraction

After preprocessing, features are calculated independently for each modality.

| Modality | Feature Category |
|---|---|
| IMU | Kinematic and movement features |
| Acoustic | Time and frequency features |
| EMG | Muscle activation features |
| Radar | Distance and movement features |

These features form the input to the machine-learning stage.

---

# 17. Individual Modality Analysis

Individual models can first be developed for:

- IMU only
- Acoustic only
- EMG only
- Radar only

This allows investigation of the information contributed by each modality.

---

# 18. Multimodal Fusion

The extracted features can then be combined.

The general architecture is:

**IMU Features + Acoustic Features + EMG Features + Radar Features → Feature Fusion → Machine Learning → Screening Profile**

Possible machine-learning algorithms include:

- Logistic Regression
- Support Vector Machine
- Random Forest
- XGBoost

Model selection should be based on experimental validation.

---

# 19. Model Evaluation

The model can be evaluated using:

- Sensitivity
- Specificity
- Precision
- Recall
- F1-score
- ROC-AUC
- Confusion Matrix
- Calibration

Participant-level separation should be used when creating training and testing datasets.

Repeated recordings from the same participant should not be allowed to leak between training and testing datasets.

---

# 20. Clinical Reference

For future clinical validation, sensor-derived features should be compared with appropriate reference information such as:

- Clinical history
- Pain and stiffness assessment
- Functional assessment
- Physical examination
- Clinician assessment
- Imaging when clinically indicated

The sensors themselves do not directly visualize cartilage degeneration.

---

# 21. Final System Workflow

The complete experimental workflow is:

**Participant Registration → Clinical/Functional Screening → Sensor Placement → Calibration → Static Test → Knee Movement Test → Walking Test → Sit-to-Stand Test → Multimodal Data Acquisition → Signal Quality Check → Signal Processing → Feature Extraction → Individual Modality Analysis → Multimodal Feature Fusion → Machine Learning → Explainable Results → Functional Screening Profile → Report Generation**

---

# 22. Current Prototype Scope

The current prototype demonstrates:

- Dual IMU acquisition
- Digital acoustic acquisition
- EMG signal processing
- Radar distance and movement acquisition
- Initial multimodal system architecture

Further work is required for:

- Continuous synchronized acquisition
- Larger datasets
- Participant-level validation
- Clinical reference labeling
- Multimodal machine-learning validation
- Independent validation

---

# 23. Important Limitation

This project is an AI-assisted research and screening prototype.

It should not be represented as a clinically validated standalone diagnostic device.

Clinical validation is required before diagnostic deployment.
