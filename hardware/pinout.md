# Hardware Pin Connections

This document describes the electrical connections used in the Sancharan AI-assisted multimodal osteoarthritis screening prototype.

## 1. ESP32-S3 + Dual IMU

Two MPU-9250/MPU-6500-compatible IMU modules are connected to the same I2C bus.

The two IMUs use different I2C addresses through the AD0 pin.

### Common I2C connections

| ESP32-S3 | IMU |
|---|---|
| 3.3V | VCC |
| GND | GND |
| GPIO 21 | SDA |
| GPIO 7 | SCL |

### Thigh IMU

| IMU Pin | Connection |
|---|---|
| VCC | ESP32-S3 3.3V |
| GND | Common GND |
| SDA | GPIO 21 |
| SCL | GPIO 7 |
| AD0 | GND |
| NCS | 3.3V |
| FSYNC | GND |

I2C address:

`0x68`

### Shank IMU

| IMU Pin | Connection |
|---|---|
| VCC | ESP32-S3 3.3V |
| GND | Common GND |
| SDA | GPIO 21 |
| SCL | GPIO 7 |
| AD0 | 3.3V |
| NCS | 3.3V |
| FSYNC | GND |

I2C address:

`0x69`

### IMU placement

The first IMU is mounted on the thigh segment and the second IMU is mounted on the shank segment.

The relative orientation of the two IMUs can be used to estimate knee movement and derive movement-related features such as:

- Knee angle
- Range of motion
- Angular velocity
- Gait timing
- Movement symmetry
- Segment acceleration

---

# 2. ESP32-S3 + INMP441 Acoustic Sensor

The INMP441 is a digital I2S MEMS microphone.

It provides digital acoustic samples to the ESP32-S3.

| INMP441 Pin | ESP32-S3 |
|---|---|
| VDD | 3.3V |
| GND | GND |
| SCK / BCLK | GPIO 4 |
| WS / LRCLK | GPIO 5 |
| SD / DATA | GPIO 6 |
| L/R | GND |

### I2S configuration

- Sampling frequency: 16 kHz
- Data format: 32-bit I2S
- Channel: Left
- Interface: I2S

### Important note

The INMP441 is a digital microphone and should not automatically be described as a validated vibroarthrography sensor.

For future structure-borne knee vibration measurements, a contact piezoelectric or contact accelerometer sensing arrangement with appropriate analog front-end electronics may be evaluated.

---

# 3. EMG Interface

The EMG interface depends on the exact EMG acquisition board being used.

The EMG electrodes must connect to a suitable biomedical EMG front-end/amplifier.

### Important safety note

Raw electrodes should not be connected directly to an ESP32 GPIO.

A suitable biomedical analog front-end and appropriate electrical safety/isolation must be used.

### Typical signal path

```text
EMG electrodes
      ↓
Biomedical EMG amplifier / AFE
      ↓
Filtering
      ↓
ADC
      ↓
ESP32 / Data acquisition system
      ↓
Signal processing
