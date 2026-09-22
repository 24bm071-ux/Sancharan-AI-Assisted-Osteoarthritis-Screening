# Experimental Results

## 1. Overview

Initial experiments were performed to verify the operation of the four sensing modalities:

1. Dual IMU
2. Acoustic sensing
3. Surface EMG
4. Radar

The results presented here demonstrate sensor-level acquisition and signal-processing feasibility.

They are not clinical validation results.

---

# 2. Dual IMU Results

Two IMUs were connected to the ESP32-S3.

- Thigh IMU → I2C address 0x68
- Shank IMU → I2C address 0x69

The sensors provide:

- Accelerometer: Ax, Ay, Az
- Gyroscope: Gx, Gy, Gz

## Sample Measurements

| Time (ms) | Thigh Ax | Thigh Ay | Thigh Az | Thigh Gx | Thigh Gy | Thigh Gz | Shank Ax | Shank Ay | Shank Az | Shank Gx | Shank Gy | Shank Gz |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 416647 | -10788 | 2064 | 11900 | 499 | -332 | -469 | 15304 | -1412 | 5476 | -652 | 145 | -197 |
| 417660 | -10292 | 3216 | 12052 | 139 | -310 | 21 | 15284 | -384 | 5692 | -307 | 158 | -73 |
| 418673 | -10320 | 3476 | 11872 | 221 | -244 | -61 | 15204 | -252 | 5784 | -450 | 191 | -120 |
| 419686 | -10352 | 3876 | 11832 | -384 | -46 | 537 | 15108 | -292 | 6452 | 387 | 349 | 290 |

These values are raw sensor readings produced by the prototype firmware.

---

# 3. IMU Interpretation

The two IMUs provide simultaneous information from the thigh and shank.

The difference in movement between the two body segments can be used for future estimation of:

- Knee angle
- Range of motion
- Angular velocity
- Gait timing
- Movement symmetry
- Movement variability

The current readings demonstrate successful communication with both IMUs.

---

# 4. Acoustic Results

The INMP441 digital microphone was interfaced with the ESP32-S3 through I2S.

Example digital samples obtained during testing:

| Time (ms) | Acoustic Sample |
|---:|---:|
| 416647 | 17456 |
| 417660 | 15828 |
| 418673 | -10276 |
| 419686 | 8047 |

These values represent individual digital samples.

They should not be interpreted individually as OA biomarkers.

Continuous signal recording is required for time-frequency analysis.

---

# 5. Acoustic Processing

The planned processing pipeline is:

**I2S Audio → Continuous Buffer → Filtering → Windowing → FFT/STFT → Feature Extraction**

Potential features include:

| Feature | Purpose |
|---|---|
| RMS | Signal magnitude |
| Peak | Maximum amplitude |
| Energy | Signal activity |
| Spectral centroid | Frequency distribution |
| Spectral bandwidth | Frequency spread |
| Band energy | Energy in selected frequency ranges |
| Spectrogram | Time-frequency representation |

---

# 6. EMG Results

Surface EMG processing was demonstrated using the following sequence:

**Raw EMG → Band-pass Filtered EMG → Rectified EMG → EMG Envelope**

## Output Stages

| Stage | Description |
|---|---|
| Raw EMG | Original acquired signal |
| Band-pass EMG | Reduced unwanted frequency components |
| Rectified EMG | Absolute magnitude of EMG |
| Envelope | Smoothed muscle activation pattern |

The EMG dashboard/output image will be added to the repository after anonymization.

Numerical EMG data will be added from experimental recordings.

---

# 7. EMG Features

Potential features include:

| Category | Features |
|---|---|
| Time domain | RMS |
| Time domain | Mean Absolute Value |
| Time domain | Integrated EMG |
| Activation | Onset |
| Activation | Offset |
| Activation | Duration |
| Frequency domain | Mean frequency |
| Frequency domain | Median frequency |

No numerical EMG values are fabricated in this repository.

Actual values will be added from experimental recordings.

---

# 8. Radar Results

The radar prototype used:

**HLK-LD2402 + ESP32-C3 SuperMini**

Communication:

**UART at 115200 baud**

Representative distance measurements included:

| Sequence | Distance |
|---:|---:|
| 1 | 107 cm |
| 2 | 91 cm |
| 3 | 82 cm |
| 4 | 74 cm |
| 5 | 67 cm |
| 6 | 63 cm |
| 7 | 59 cm |
| 8 | 52 cm |
| 9 | 47 cm |
| 10 | 43 cm |
| 11 | 39 cm |

The decreasing distance sequence indicates movement toward the radar during this test.

---

# 9. Radar Movement States

The prototype firmware calculates radial motion from changes in distance.

Observed states include:

- APPROACHING
- RECEDING
- LOW_RADIAL_MOTION

Example experimental sequence:

**RECEDING → RECEDING → LOW_RADIAL_MOTION → APPROACHING → APPROACHING → LOW_RADIAL_MOTION**

The radar provides complementary contactless movement information.

It is not used as a direct measurement of knee joint kinematics.

---

# 10. Four-Modality Experimental Summary

| Modality | Sensor | Output | Status |
|---|---|---|---|
| IMU | 2 × MPU-9250/6500 | Acceleration + gyroscope | Demonstrated |
| Acoustic | INMP441 | Digital I2S samples | Demonstrated |
| EMG | EMG acquisition system | Raw + processed signal | Demonstrated |
| Radar | HLK-LD2402 | Distance + radial motion | Demonstrated |

---

# 11. Current Experimental Evidence

The current experiments demonstrate:

### IMU

Successful acquisition from two sensors using different I2C addresses.

### Acoustic

Successful digital I2S acquisition from the INMP441.

### EMG

Successful signal-processing pipeline from raw signal to envelope.

### Radar

Successful distance and radial-motion acquisition.

---

# 12. Acquisition Limitation

The current demonstration firmware prints IMU and acoustic values approximately once per second.

This is sufficient for demonstrating sensor operation but is not sufficient for complete gait waveform analysis.

Future firmware will separate:

**High-rate sensor acquisition → Buffering → Timestamping → Data logging/transmission**

from lower-rate serial monitoring.

This will preserve the sensor information required for detailed analysis.

---

# 13. Next Experimental Dataset

The next stage will collect synchronized multimodal recordings during:

1. Static standing
2. Knee flexion-extension
3. Walking
4. Sit-to-stand

Each trial will contain:

**Timestamp + IMU + Acoustic + EMG + Radar**

The recordings will then be processed to generate modality-specific features.

---

# 14. Multimodal Analysis

The feature sets will be evaluated separately and in combination.

Potential configurations include:

- IMU
- Acoustic
- EMG
- Radar
- IMU + Acoustic
- IMU + EMG
- IMU + Radar
- Acoustic + EMG
- Acoustic + Radar
- EMG + Radar
- IMU + Acoustic + EMG
- IMU + Acoustic + Radar
- IMU + EMG + Radar
- Acoustic + EMG + Radar
- IMU + Acoustic + EMG + Radar

The purpose is to investigate the contribution of complementary sensing modalities.

---

# 15. Validation

Future model evaluation will use participant-level separation.

Evaluation metrics may include:

- Sensitivity
- Specificity
- Precision
- Recall
- F1-score
- ROC-AUC
- Confusion Matrix
- Calibration

No clinical accuracy claim is made from the current sensor demonstrations.

---

# 16. Research Status

| Component | Status |
|---|---|
| Hardware integration | Completed |
| IMU acquisition | Demonstrated |
| Acoustic acquisition | Demonstrated |
| EMG processing | Demonstrated |
| Radar acquisition | Demonstrated |
| Continuous synchronization | In progress |
| Feature extraction | In progress |
| Multimodal dataset | Planned |
| Machine learning | Planned |
| Clinical validation | Future work |

---

# 17. Conclusion

The experimental results demonstrate the feasibility of acquiring complementary signals from multiple sensing modalities.

The next stage focuses on synchronized continuous acquisition, standardized functional tasks, feature extraction and multimodal machine-learning analysis.

The prototype is intended for research and functional screening development and is not currently a clinically validated diagnostic device.
