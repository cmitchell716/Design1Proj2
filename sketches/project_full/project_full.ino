#include "C:\Users\cmitc\Documents\SeniorDesign1\Project2\repo\headers\MPU9250.h" //must change this line to location on your machine
#include <Wire.h>
#include <SoftwareSerial.h>
#include <TinyGPS_stripped.h>
#include <SPI.h>
#include <SD.h>

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_UART.h"

TinyGPS gps;
SoftwareSerial ss(3, 4);
MPU9250 mpu;
File myFile;
SoftwareSerial bluefruitSS = SoftwareSerial(7, 6);
Adafruit_BluefruitLE_UART ble(bluefruitSS, 9, 8, -1);
boolean newGPS = false;
boolean newIMU = false;
boolean SDavailable = false;
boolean printBle = true;
boolean GPSlock = false;
unsigned long outputTimer;

char filename[] = "Output_Record.csv"; // need to change to date/time

void setup() {
  Serial.begin(115200);
  ss.begin(9600);
  Wire.begin();
  delay(2000);
  pinMode(2, OUTPUT);

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

  if ( !ble.begin() )
  {
    Serial.println(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  else{
    ble.echo(false);
    ble.setMode(BLUEFRUIT_MODE_DATA);
  }

  SDavailable = SD.begin(10);
  if(SDavailable){
    create_file();
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

  if(GPSlock){
    digitalWrite(2, HIGH);
  }
  else digitalWrite(2, LOW);
  
  if((newIMU && newGPS) || (millis() - outputTimer > 1000)) {
    print_IMU_measurements();
    newIMU = false;
    print_GPS_measurements();
    newGPS = false;
    outputTimer = millis();
    if(printBle){
      printBle = false;
    }
    else printBle = true;

    if(newGPS){
      GPSlock = true;
    }
    else GPSlock = false;
  }

}

void create_file(){
  if (SD.exists(filename)) {
    SD.remove(filename);
  }
  myFile = SD.open(filename, FILE_WRITE);
  if (myFile) {
    myFile.println(F("\"AccX\",\"AccY\",\"AccZ\",\"MagX\",\"MagY\",\"MagZ\",\"GyrX\",\"GyrY\",\"GyrZ\",\"Lat\",\"Lon\",\"Alt\",\"Sat\""));
  }
  myFile.close();
}


void print_IMU_measurements() {
  if(newIMU){
    Serial.print(F("Acceleration X, Y, Z: "));
    Serial.print(mpu.getAccX(), 2);
    Serial.print(", ");
    Serial.print(mpu.getAccY(), 2);
    Serial.print(F(", "));
    Serial.print(mpu.getAccZ(), 2);
    Serial.print(F("  "));
    Serial.print(F("Mag X, Y, Z: "));
    Serial.print(mpu.getMagX(), 2);
    Serial.print(F(", "));
    Serial.print(mpu.getMagY(), 2);
    Serial.print(F(", "));
    Serial.print(mpu.getMagZ(), 2);
    Serial.print(F(", "));
    Serial.print(F("Gyro X, Y, Z: "));
    Serial.print(mpu.getGyroX(), 2);
    Serial.print(F(", "));
    Serial.print(mpu.getGyroY(), 2);
    Serial.print(F(", "));
    Serial.println(mpu.getGyroZ(), 2);

    
    if(printBle){
      ble.print(F("Acceleration X, Y, Z: "));
      ble.print(mpu.getAccX(), 2);
      ble.print(", ");
      ble.print(mpu.getAccY(), 2);
      ble.print(F(", "));
      ble.print(mpu.getAccZ(), 2);
      ble.print(F("  "));
      ble.print(F("Mag X, Y, Z: "));
      ble.print(mpu.getMagX(), 2);
      ble.print(F(", "));
      ble.print(mpu.getMagY(), 2);
      ble.print(F(", "));
      ble.print(mpu.getMagZ(), 2);
      ble.print(F(", "));
      ble.print(F("Gyro X, Y, Z: "));
      ble.print(mpu.getGyroX(), 2);
      ble.print(F(", "));
      ble.print(mpu.getGyroY(), 2);
      ble.print(F(", "));
      ble.println(mpu.getGyroZ(), 2);
    }
    

    if(SDavailable){
      myFile = SD.open(filename, FILE_WRITE);
      myFile.print(F("\""));
      myFile.print(mpu.getAccX(), 2);
      myFile.print(F("\",\""));
      myFile.print(mpu.getAccY(), 2);
      myFile.print(F("\",\""));
      myFile.print(mpu.getAccZ(), 2);
      myFile.print(F("\",\""));
      myFile.print(mpu.getMagX(), 2);
      myFile.print(F("\",\""));
      myFile.print(mpu.getMagY(), 2);
      myFile.print(F("\",\""));
      myFile.print(mpu.getMagZ(), 2);
      myFile.print(F("\",\""));
      myFile.print(mpu.getGyroX(), 2);
      myFile.print(F("\",\""));
      myFile.print(mpu.getGyroY(), 2);
      myFile.print(F("\",\""));
      myFile.print(mpu.getGyroZ(), 2);
      myFile.print(F("\",\""));
      myFile.close();
    }
    
  }
  else{
    Serial.println(F("No IMU data"));
    ble.println(F("No IMU data"));
    if(SDavailable){
      myFile = SD.open(filename, FILE_WRITE);
      myFile.print(F("\"N/D\",\"N/D\",\"N/D\",\"N/D\",\"N/D\",\"N/D\",\"N/D\",\"N/D\",\"N/D\","));
      myFile.close();
    }    
  }    
}

void print_GPS_measurements() {
  if(newGPS){
    float flat, flon;
    gps.f_get_position(&flat, &flon );
    Serial.print(F("LAT="));
    Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    Serial.print(F(" LON="));
    Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
    Serial.print(F(" ALT="));
    Serial.print(gps.altitude() == TinyGPS::GPS_INVALID_ALTITUDE ? 0 : gps.altitude());
    Serial.print(F(" SAT="));
    Serial.println(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
  
    
    if(printBle){
      ble.print(F("LAT="));
      ble.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
      ble.print(F(" LON="));
      ble.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
      ble.print(F(" ALT="));
      ble.print(gps.altitude() == TinyGPS::GPS_INVALID_ALTITUDE ? 0 : gps.altitude());
      ble.print(F(" SAT="));
      ble.println(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
    }
    

    if(SDavailable){
      myFile = SD.open(filename, FILE_WRITE);
      myFile.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
      myFile.print(F("\",\""));
      myFile.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
      myFile.print(F("\",\""));
      myFile.print(gps.altitude() == TinyGPS::GPS_INVALID_ALTITUDE ? 0 : gps.altitude());
      myFile.print(F("\",\""));
      myFile.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
      myFile.println(F("\""));
      myFile.close();
    }
  }
  else{
    Serial.println(F("No GPS data"));
    ble.println(F("No GPS data"));
    if(SDavailable){
      myFile = SD.open(filename, FILE_WRITE);
      myFile.println(F("\"N/D\",\"N/D\",\"N/D\",\"N/D\""));
      myFile.close();
    }  
  } 
}
