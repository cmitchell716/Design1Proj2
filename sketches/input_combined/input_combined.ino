#include "MPU9250.h"
#include <Wire.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

TinyGPS gps;
SoftwareSerial ss(4, 5);
MPU9250 mpu;

void setup() {
  Serial.begin(115200);
  ss.begin(9600);
  Wire.begin();

  if (!mpu.setup(0x68)) {  
      while (1) {
          Serial.println(F("MPU connection failed."));
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
  bool newGPS = false;
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      // Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newGPS = true;
    }
  }
  bool newIMU = mpu.available();

  if(newGPS) {
    print_GPS_measurements();
  }
  else Serial.println(F("No GPS Data"));
  if(newIMU){
    mpu.update_accel_gyro();
    mpu.update_mag();
    print_IMU_measurements();
  }
  else Serial.println(F("No IMU Data"));

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

void print_GPS_measurements() {
  float flat, flon;
  unsigned long age, alt;
  gps.f_get_position(&flat, &flon, &age);
  Serial.print("LAT=");
  Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
  Serial.print(" LON=");
  Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
  Serial.print(" ALT=");
  Serial.print(alt == TinyGPS::GPS_INVALID_ALTITUDE ? 0 : gps.altitude());
  Serial.print(" SAT=");
  Serial.println(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
}
