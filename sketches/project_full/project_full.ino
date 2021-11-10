#include "C:\Users\cmitc\Documents\SeniorDesign1\Project2\repo\headers\MPU9250.h" //must change this line to location on your machine
#include <Wire.h>
#include <SoftwareSerial.h>
#include <TinyGPS_stripped.h>
#include <SPI.h>
#include <SdFat.h>

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_UART.h"
//#include "BluefruitConfig.h"

TinyGPS gps;
SoftwareSerial ss(4, 5);
MPU9250 mpu;
SdFat sd;
SoftwareSerial bluefruitSS = SoftwareSerial(6, 7);
Adafruit_BluefruitLE_UART ble(bluefruitSS, 8,
                      9, 10);
boolean newGPS = false;
boolean newIMU = false;
long outputTimer;

void setup() {
  Serial.begin(115200);
  ss.begin(9600);
  Wire.begin();
  delay(2000);

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
    mpu.setMagneticDeclination(3.08);
  }

  outputTimer = millis();
}

void loop() {
  if (mpu.available()) {
        mpu.update_accel_gyro();
        mpu.update_mag();
        newIMU = true;
  }

  for (unsigned long start = millis(); millis() - start < 5;)
  {
    while (ss.available())
    {
      char c = ss.read();
      if (gps.encode(c))  
        newGPS = true;
    }
  }
  
  if(newIMU && newGPS) {
    print_IMU_measurements();
    newIMU = false;
    print_GPS_measurements();
    newGPS = false;
    outputTimer = millis();
  }
  else if(millis() - outputTimer > 10000){
    if(newIMU){
      print_IMU_measurements();
      Serial.println(F("GPS not Connected"));
      ble.println(F("GPS not Connected"));
      outputTimer = millis();
      newIMU = false;
      newGPS = false;
    }
    else if(newGPS){
      Serial.print(F("IMU not Connected"));
      ble.print(F("IMU not Connected"));
      print_GPS_measurements();
      outputTimer = millis();
      newIMU = false;
      newGPS = false;
    }
    else{
      Serial.println(F("No data available"));
      ble.println(F("No data available"));
      outputTimer = millis();
      newIMU = false;
      newGPS = false;
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

    ble.print("Acceleration X, Y, Z: ");
    ble.print(mpu.getAccX(), 2);
    ble.print(", ");
    ble.print(mpu.getAccY(), 2);
    ble.print(", ");
    ble.print(mpu.getAccZ(), 2);
    ble.print("  ");
    ble.print("Mag X, Y, Z: ");
    ble.print(mpu.getMagX(), 2);
    ble.print(", ");
    ble.print(mpu.getMagY(), 2);
    ble.print(", ");
    ble.print(mpu.getMagZ(), 2);
    ble.print(", ");
    ble.print("Gyro X, Y, Z: ");
    ble.print(mpu.getGyroX(), 2);
    ble.print(", ");
    ble.print(mpu.getGyroY(), 2);
    ble.print(", ");
    ble.println(mpu.getGyroZ(), 2);
}

void print_GPS_measurements() {
  float flat, flon;
  unsigned int alt;
  gps.f_get_position(&flat, &flon );
  Serial.print("LAT=");
  Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
  Serial.print(" LON=");
  Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
  Serial.print(" ALT=");
  Serial.print(alt == TinyGPS::GPS_INVALID_ALTITUDE ? 0 : gps.altitude());
  Serial.print(" SAT=");
  Serial.println(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());

  ble.print("LAT=");
  ble.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
  ble.print(" LON=");
  ble.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
  ble.print(" ALT=");
  ble.print(alt == TinyGPS::GPS_INVALID_ALTITUDE ? 0 : gps.altitude());
  ble.print(" SAT=");
  ble.println(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
}
