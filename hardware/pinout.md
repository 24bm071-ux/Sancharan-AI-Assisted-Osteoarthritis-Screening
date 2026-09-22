# Hardware Pin Connections

This document describes the electrical connections used in the current prototype.

## 1. ESP32-S3 + Dual IMU + INMP441

The ESP32-S3 acts as the primary controller for the dual IMU and acoustic sensing prototype.

### ESP32-S3 Pin Summary

| ESP32-S3 GPIO | Function | Connected Device |
|---|---|---|
| GPIO 21 | I2C SDA | Both IMUs |
| GPIO 7 | I2C SCL | Both IMUs |
| GPIO 4 | I2S BCLK | INMP441 |
| GPIO 5 | I2S WS/LRCLK | INMP441 |
| GPIO 6 | I2S DATA | INMP441 |

---

# 2. Thigh IMU

The first MPU-9250/6500 is placed on the thigh.

| IMU Pin | ESP32-S3 |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 7 |
| AD0 | GND |
| NCS | 3.3V |
| FSYNC | GND |

### I2C Address

`0x68`

---

# 3. Shank IMU

The second MPU-9250/6500 is placed on the shank.

| IMU Pin | ESP32-S3 |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 7 |
| AD0 | 3.3V |
| NCS | 3.3V |
| FSYNC | GND |

### I2C Address

`0x69`

---

# 4. Why Two I2C Addresses Are Used

Both IMUs share the same I2C bus.

The AD0 pin is used to select different I2C addresses.

```text
Thigh IMU
AD0 → GND
Address = 0x68

Shank IMU
AD0 → 3.3V
Address = 0x69
```

This allows both IMUs to communicate with the same ESP32-S3 without requiring two separate I2C buses.

---

# 5. INMP441 Acoustic Sensor

The INMP441 is a digital I2S MEMS microphone.

| INMP441 Pin | ESP32-S3 |
|---|---|
| VDD | 3.3V |
| GND | GND |
| SCK / BCLK | GPIO 4 |
| WS / LRCLK | GPIO 5 |
| SD / DATA | GPIO 6 |
| L/R | GND |

### I2S Configuration

| Parameter | Value |
|---|---|
| Interface | I2S |
| Sampling frequency | 16 kHz |
| Data format | 32-bit |
| Channel | Left |

---

# 6. Radar Prototype

The radar prototype was tested separately using:

**HLK-LD2402 + ESP32-C3 SuperMini**

The radar communicates through UART.

| Radar Pin | ESP32-C3 SuperMini |
|---|---|
| 3.3V | 3.3V |
| GND | GND |
| UART TX | GPIO 4 |
| UART RX | GPIO 5 |
| IO / Status | GPIO 6 |

### UART Configuration

| Parameter | Value |
|---|---|
| Baud rate | 115200 |
| Data format | 8N1 |
| Interface | UART |

---

# 7. Radar Wiring Logic

UART communication uses crossed TX/RX connections.

```text
Radar TX  → ESP32-C3 RX
Radar RX  ← ESP32-C3 TX
```

Therefore:

```text
Radar TX → GPIO 4
Radar RX → GPIO 5
```

---

# 8. Common Ground

All sensors connected to the same controller must share a common electrical ground.

For the ESP32-S3 prototype:

```text
ESP32 GND
   ├── Thigh IMU GND
   ├── Shank IMU GND
   └── INMP441 GND
```

The radar test uses the ESP32-C3 ground:

```text
ESP32-C3 GND → Radar GND
```

---

# 9. Sensor Placement

### Thigh IMU

The first IMU is mounted firmly on the thigh.

### Shank IMU

The second IMU is mounted firmly on the lower leg/shank.

Both sensors should maintain a consistent orientation throughout the experiment.

### Acoustic Sensor

The INMP441 is used as an experimental acoustic sensing channel.

Its placement should remain consistent between trials.

### Radar

The radar is positioned beside the walking/test area.

It is not attached directly to the knee.

---

# 10. Important Hardware Notes

- Use appropriate 3.3 V logic for the sensors.
- Ensure all devices share a common ground.
- Avoid loose sensor connections during movement experiments.
- Keep IMU orientation consistent between trials.
- Record the sensor configuration for every experiment.
- EMG electrodes must interface through an appropriate biomedical EMG front-end.
- Do not connect raw EMG electrodes directly to an ESP32 GPIO.
