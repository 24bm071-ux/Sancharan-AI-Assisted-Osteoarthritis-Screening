# Sensor Details

## 1. Dual IMU

### Sensor

MPU-9250 / MPU-6500-compatible 6-axis IMU modules.

### Measurements

- 3-axis acceleration
- 3-axis angular velocity

### Configuration

The two IMUs share the same I2C bus but use different addresses.

```text
Thigh IMU → 0x68
Shank IMU → 0x69
