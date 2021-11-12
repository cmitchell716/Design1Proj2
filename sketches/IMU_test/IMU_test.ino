#include "MPU9250.h"
#include <Wire.h>

MPU9250 mpu;

void setup() {
    Serial.begin(115200);
    Wire.begin();
    delay(2000);

    if (!mpu.setup(0x68)) {  // change to your own address
        while (1) {
            Serial.println("MPU connection failed. Please check your connection.");
            delay(5000);
        }
    }
    else {
      mpu.setAccBias(36.19, 3.86, 54.43);
      mpu.setGyroBias(-1.20, 1.95, -0.08);
      mpu.setMagBias(128.27, -144.56, -176.65);
      mpu.setMagScale(1.18, 1.03, 0.85);
      //mpu.setmagneticDeclination();
    }
}

void loop() {
    if (mpu.available()) {
        mpu.update_accel_gyro();
        mpu.update_mag();
        static uint32_t prev_ms = millis();
        if (millis() > prev_ms + 100) {
            print_IMU_measurements();
            prev_ms = millis();
        }
    }
}

void print_IMU_measurements() {
    Serial.print("Acceleration X, Y, Z: ");
    Serial.print(mpu.getAccX(), 2);
    Serial.print(", ");
    Serial.print(mpu.getAccY(), 2);
    Serial.print(", ");
    Serial.print(mpu.getAccZ(), 2);
    Serial.print("  ");
    Serial.print("Mag X, Y, Z: ");
    Serial.print(mpu.getMagX(), 2);
    Serial.print(", ");
    Serial.print(mpu.getMagY(), 2);
    Serial.print(", ");
    Serial.print(mpu.getMagZ(), 2);
    Serial.print(", ");
    Serial.print("Gyro X, Y, Z: ");
    Serial.print(mpu.getGyroX(), 2);
    Serial.print(", ");
    Serial.print(mpu.getGyroY(), 2);
    Serial.print(", ");
    Serial.println(mpu.getGyroZ(), 2);
}
