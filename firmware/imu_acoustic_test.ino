#include <Wire.h>
#include "driver/i2s.h"

// =====================================================
// ESP32-S3
// THIGH MPU + SHANK MPU + INMP441
// =====================================================


// =====================================================
// I2C PINS
// =====================================================

#define SDA_PIN 21
#define SCL_PIN 7

// MPU addresses
#define THIGH_IMU 0x68
#define SHANK_IMU 0x69


// =====================================================
// INMP441 I2S PINS
// =====================================================

#define I2S_PORT I2S_NUM_0

#define I2S_BCLK 4
#define I2S_WS   5
#define I2S_DATA 6


// =====================================================
// MPU-9250 / MPU-6500 REGISTERS
// =====================================================

#define WHO_AM_I      0x75
#define PWR_MGMT_1    0x6B
#define SMPLRT_DIV    0x19
#define CONFIG_REG    0x1A
#define GYRO_CONFIG   0x1B
#define ACCEL_CONFIG  0x1C
#define ACCEL_XOUT_H  0x3B


// =====================================================
// MPU WRITE
// =====================================================

void writeMPU(uint8_t address, uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(address);

    Wire.write(reg);
    Wire.write(value);

    uint8_t error = Wire.endTransmission();

    if (error != 0)
    {
        Serial.print("I2C WRITE ERROR 0x");
        Serial.println(error, HEX);
    }
}


// =====================================================
// MPU READ REGISTER
// =====================================================

uint8_t readRegister(uint8_t address, uint8_t reg)
{
    Wire.beginTransmission(address);

    Wire.write(reg);

    uint8_t error = Wire.endTransmission(false);

    if (error != 0)
    {
        return 0xFF;
    }

    Wire.requestFrom(address, (uint8_t)1);

    if (Wire.available())
    {
        return Wire.read();
    }

    return 0xFF;
}


// =====================================================
// INITIALIZE MPU
// =====================================================

bool initMPU(uint8_t address)
{
    uint8_t who = readRegister(address, WHO_AM_I);

    Serial.print("Checking MPU at 0x");
    Serial.print(address, HEX);

    Serial.print(" | WHO_AM_I = 0x");
    Serial.println(who, HEX);


    if (who == 0xFF)
    {
        Serial.println("ERROR: MPU NOT FOUND");

        return false;
    }


    // Wake MPU
    writeMPU(
        address,
        PWR_MGMT_1,
        0x00
    );

    delay(100);


    // Sample rate divider
    writeMPU(
        address,
        SMPLRT_DIV,
        0x04
    );


    // Digital low-pass filter
    writeMPU(
        address,
        CONFIG_REG,
        0x03
    );


    // Gyroscope ±250 dps
    writeMPU(
        address,
        GYRO_CONFIG,
        0x00
    );


    // Accelerometer ±2g
    writeMPU(
        address,
        ACCEL_CONFIG,
        0x00
    );


    delay(50);


    Serial.print("MPU 0x");
    Serial.print(address, HEX);

    Serial.println(" initialized");

    return true;
}


// =====================================================
// READ MPU
// =====================================================

void readMPU(
    uint8_t address,

    int16_t &ax,
    int16_t &ay,
    int16_t &az,

    int16_t &gx,
    int16_t &gy,
    int16_t &gz
)
{
    Wire.beginTransmission(address);

    Wire.write(ACCEL_XOUT_H);

    Wire.endTransmission(false);

    Wire.requestFrom(
        address,
        (uint8_t)14
    );


    if (Wire.available() < 14)
    {
        ax = 0;
        ay = 0;
        az = 0;

        gx = 0;
        gy = 0;
        gz = 0;

        return;
    }


    // Accelerometer

    ax =
        (Wire.read() << 8) |
        Wire.read();

    ay =
        (Wire.read() << 8) |
        Wire.read();

    az =
        (Wire.read() << 8) |
        Wire.read();


    // Temperature
    Wire.read();
    Wire.read();


    // Gyroscope

    gx =
        (Wire.read() << 8) |
        Wire.read();

    gy =
        (Wire.read() << 8) |
        Wire.read();

    gz =
        (Wire.read() << 8) |
        Wire.read();
}


// =====================================================
// INMP441 SETUP
// =====================================================

void setupI2S()
{
    i2s_config_t i2s_config =
    {
        .mode =
            (i2s_mode_t)(
                I2S_MODE_MASTER |
                I2S_MODE_RX
            ),

        .sample_rate = 16000,

        .bits_per_sample =
            I2S_BITS_PER_SAMPLE_32BIT,

        .channel_format =
            I2S_CHANNEL_FMT_ONLY_LEFT,

        .communication_format =
            I2S_COMM_FORMAT_I2S,

        .intr_alloc_flags =
            ESP_INTR_FLAG_LEVEL1,

        .dma_buf_count = 8,

        .dma_buf_len = 64,

        .use_apll = false,

        .tx_desc_auto_clear = false,

        .fixed_mclk = 0
    };


    i2s_pin_config_t pin_config =
    {
        .bck_io_num = I2S_BCLK,

        .ws_io_num = I2S_WS,

        .data_out_num =
            I2S_PIN_NO_CHANGE,

        .data_in_num =
            I2S_DATA
    };


    i2s_driver_install(
        I2S_PORT,
        &i2s_config,
        0,
        NULL
    );


    i2s_set_pin(
        I2S_PORT,
        &pin_config
    );


    i2s_zero_dma_buffer(
        I2S_PORT
    );


    Serial.println("INMP441 initialized");
}


// =====================================================
// READ INMP441
// =====================================================

int32_t readMicrophone()
{
    int32_t sample = 0;

    size_t bytesRead = 0;


    i2s_read(
        I2S_PORT,

        &sample,

        sizeof(sample),

        &bytesRead,

        100
    );


    if (bytesRead != sizeof(sample))
    {
        return 0;
    }


    // Convert 24-bit microphone data
    sample = sample >> 8;


    return sample;
}


// =====================================================
// SETUP
// =====================================================

void setup()
{
    Serial.begin(115200);

    delay(2000);


    Serial.println();
    Serial.println("========================================");
    Serial.println(" ESP32-S3 SENSOR NODE");
    Serial.println(" THIGH + SHANK IMU + INMP441");
    Serial.println("========================================");


    // =================================================
    // I2C
    // =================================================

    Serial.println();
    Serial.println("Starting I2C...");


    Wire.begin(
        SDA_PIN,
        SCL_PIN
    );


    Wire.setClock(400000);


    // =================================================
    // THIGH MPU
    // =================================================

    Serial.println();
    Serial.println("Initializing THIGH MPU...");


    if (initMPU(THIGH_IMU))
    {
        Serial.println("THIGH MPU -> OK");
    }
    else
    {
        Serial.println("THIGH MPU -> FAILED");
    }


    // =================================================
    // SHANK MPU
    // =================================================

    Serial.println();
    Serial.println("Initializing SHANK MPU...");


    if (initMPU(SHANK_IMU))
    {
        Serial.println("SHANK MPU -> OK");
    }
    else
    {
        Serial.println("SHANK MPU -> FAILED");
    }


    // =================================================
    // INMP441
    // =================================================

    Serial.println();
    Serial.println("Initializing INMP441...");


    setupI2S();


    // =================================================
    // DONE
    // =================================================

    Serial.println();
    Serial.println("========================================");
    Serial.println(" ALL SENSORS INITIALIZED");
    Serial.println("========================================");
    Serial.println();
}


// =====================================================
// LOOP
// =====================================================

void loop()
{
    unsigned long timestamp = millis();


    // =================================================
    // THIGH VARIABLES
    // =================================================

    int16_t thighAX;
    int16_t thighAY;
    int16_t thighAZ;

    int16_t thighGX;
    int16_t thighGY;
    int16_t thighGZ;


    // =================================================
    // SHANK VARIABLES
    // =================================================

    int16_t shankAX;
    int16_t shankAY;
    int16_t shankAZ;

    int16_t shankGX;
    int16_t shankGY;
    int16_t shankGZ;


    // =================================================
    // READ THIGH IMU
    // =================================================

    readMPU(
        THIGH_IMU,

        thighAX,
        thighAY,
        thighAZ,

        thighGX,
        thighGY,
        thighGZ
    );


    // =================================================
    // READ SHANK IMU
    // =================================================

    readMPU(
        SHANK_IMU,

        shankAX,
        shankAY,
        shankAZ,

        shankGX,
        shankGY,
        shankGZ
    );


    // =================================================
    // READ MICROPHONE
    // =================================================

    int32_t acoustic =
        readMicrophone();


    // =================================================
    // SERIAL OUTPUT
    // =================================================

    Serial.println(
        "----------------------------------------"
    );


    // Timestamp

    Serial.print("TIME: ");
    Serial.print(timestamp);
    Serial.println(" ms");


    // =================================================
    // THIGH
    // =================================================

    Serial.println("THIGH IMU");

    Serial.print("ACC: ");

    Serial.print(thighAX);
    Serial.print(", ");

    Serial.print(thighAY);
    Serial.print(", ");

    Serial.println(thighAZ);


    Serial.print("GYRO: ");

    Serial.print(thighGX);
    Serial.print(", ");

    Serial.print(thighGY);
    Serial.print(", ");

    Serial.println(thighGZ);


    // =================================================
    // SHANK
    // =================================================

    Serial.println("SHANK IMU");

    Serial.print("ACC: ");

    Serial.print(shankAX);
    Serial.print(", ");

    Serial.print(shankAY);
    Serial.print(", ");

    Serial.println(shankAZ);


    Serial.print("GYRO: ");

    Serial.print(shankGX);
    Serial.print(", ");

    Serial.print(shankGY);
    Serial.print(", ");

    Serial.println(shankGZ);


    // =================================================
    // ACOUSTIC
    // =================================================

    Serial.print("ACOUSTIC: ");

    Serial.println(acoustic);


    delay(1000);
}