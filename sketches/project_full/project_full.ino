#include "C:\Users\cmitc\Documents\SeniorDesign1\Project2\repo\headers\MPU9250.h" //must change this line to location on your machine
#include <Wire.h>
#include <SoftwareSerial.h>
#include <TinyGPS_stripped.h>
#include <SPI.h>
#include <SdFat.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_UART.h"


TinyGPS gps;
SoftwareSerial ss(3, 4);
MPU9250 mpu;
SdFat sd;
SdFile myFile;
SoftwareSerial bluefruitSS = SoftwareSerial(7, 6);
Adafruit_BluefruitLE_UART ble(bluefruitSS, 9, 8, -1);
boolean newGPS = false;
boolean newIMU = false;
boolean SDavailable = false;
short printBle = 5;
boolean GPSlock = false;
unsigned long outputTimer;
const char filename[] = "out.csv"; 


void setup() {
  Serial.begin(9600);
  ss.begin(9600);
  Wire.begin();
  delay(2000);
  pinMode(2, OUTPUT);

  if (mpu.setup(0x68)) {  
    //mpu.setAccBias(36.19, 3.86, 54.43);
    //mpu.setGyroBias(-1.20, 1.95, -0.08);
    //mpu.setMagBias(128.27, -144.56, -176.65);
    //mpu.setMagScale(1.18, 1.03, 0.85);
    //mpu.setMagneticDeclination(3.08);
   }
  

  SDavailable = sd.begin(10, SPI_HALF_SPEED);
  if(SDavailable){
    create_file();
  }
  
  if ( !ble.begin() )
  {
    Serial.println(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  else{
    ble.echo(false);
    ble.setMode(BLUEFRUIT_MODE_DATA);
  }
  
   //SDavailable = false;
  outputTimer = millis();
}

void loop() {
  
  if (mpu.available()) {
        mpu.update_accel_gyro();
        mpu.update_mag();
        newIMU = true;
  }
  

  ss.listen();
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
  
  if((newIMU && newGPS) || (millis() - outputTimer > 2000)) {
    print_IMU_measurements();
    print_GPS_measurements();
    outputTimer = millis();
    if(printBle == 10){
      printBle = 0;
    }
    else printBle++;
    if(newGPS){
      GPSlock = true;
    }
    else GPSlock = false;
    newGPS = false;
    newIMU = false;
  }
  
}

void create_file(){
  if (myFile.open(filename, O_WRITE | O_CREAT | O_TRUNC)) {
    delay(100);
    myFile.println(F("\"AccX\",\"AccY\",\"AccZ\",\"MagX\",\"MagY\",\"MagZ\",\"GyrX\",\"GyrY\",\"GyrZ\",\"Lat\",\"Lon\",\"Alt\",\"Sat\""));
  }
  myFile.close();
}


void print_IMU_measurements() {
  if(newIMU){
    Serial.print(F("Acc XYZ: "));
    Serial.print(mpu.getAccX(), 2);
    Serial.print(F(", "));
    Serial.print(mpu.getAccY(), 2);
    Serial.print(F(", "));
    Serial.print(mpu.getAccZ(), 2);
    Serial.print(F("  "));
    Serial.print(F("Mag XYZ: "));
    Serial.print(mpu.getMagX(), 2);
    Serial.print(F(", "));
    Serial.print(mpu.getMagY(), 2);
    Serial.print(F(", "));
    Serial.print(mpu.getMagZ(), 2);
    Serial.print(F(", "));
    Serial.print(F("Gyro XYZ: "));
    Serial.print(mpu.getGyroX(), 2);
    Serial.print(F(", "));
    Serial.print(mpu.getGyroY(), 2);
    Serial.print(F(", "));
    Serial.println(mpu.getGyroZ(), 2);

    if(printBle == 10){
      ble.print(F("Acc XYZ: "));
      ble.print(mpu.getAccX(), 2);
      ble.print(F(", "));
      ble.print(mpu.getAccY(), 2);
      ble.print(F(", "));
      ble.print(mpu.getAccZ(), 2);
      ble.print(F("  "));
      ble.print(F("Mag XYZ: "));
      ble.print(mpu.getMagX(), 2);
      ble.print(F(", "));
      ble.print(mpu.getMagY(), 2);
      ble.print(F(", "));
      ble.print(mpu.getMagZ(), 2);
      ble.print(F(", "));
      ble.print(F("Gyro XYZ: "));
      ble.print(mpu.getGyroX(), 2);
      ble.print(F(", "));
      ble.print(mpu.getGyroY(), 2);
      ble.print(F(", "));
      ble.println(mpu.getGyroZ(), 2);
    }

    if(SDavailable){      
      myFile.open(filename, O_WRITE | O_AT_END);
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
    if(printBle == 10){
      ble.println(F("No IMU data"));
    }
    if(SDavailable){      
      myFile.open(filename, O_WRITE | O_AT_END);
      myFile.print(F("\"N/D\",\"N/D\",\"N/D\",\"N/D\",\"N/D\",\"N/D\",\"N/D\",\"N/D\",\"N/D\","));
      myFile.close();     
    }    
  }    
}

void print_GPS_measurements() {
  if(newGPS){
    float flat, flon;
    int alt, sat;
    alt = gps.altitude();
    sat = gps.satellites();
    gps.f_get_position(&flat, &flon );
    Serial.print(F("LAT="));
    Serial.print(flat, 6);
    Serial.print(F(" LON="));
    Serial.print(flon, 6);
    Serial.print(F(" ALT="));
    Serial.print(alt);
    Serial.print(F(" SAT="));
    Serial.println(sat);
  
    
    if(printBle == 10){
      ble.print(F("LAT="));
      ble.print(flat, 6);
      ble.print(F(" LON="));
      ble.print(flon, 6);
      ble.print(F(" ALT="));
      ble.print(alt);
      ble.print(F(" SAT="));
      ble.println(sat);
    }
    

    if(SDavailable){     
      myFile.open(filename, O_WRITE | O_AT_END);
      myFile.print(flat, 6);
      myFile.print(F("\",\""));
      myFile.print(flon, 6); 
      myFile.print(F("\",\""));
      myFile.print(alt);
      myFile.print(F("\",\""));
      myFile.print(sat);
      myFile.println(F("\""));
      myFile.close();      
    }
  }
  else{
    Serial.println(F("No GPS data"));
    if(printBle == 10){
      ble.println(F("No GPS data"));
    }       
    if(SDavailable){      
      myFile.open(filename, O_WRITE | O_AT_END);
      myFile.println(F("\"N/D\",\"N/D\",\"N/D\",\"N/D\""));
      myFile.close();      
    }  
  } 
}
