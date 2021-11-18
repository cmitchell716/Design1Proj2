#include <SoftwareSerial.h>
#include <TinyGPS_stripped.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_UART.h"
#include <SPI.h>
#include <SdFat.h>

TinyGPS gps;
SoftwareSerial ss(3, 4);
SoftwareSerial bluefruitSS = SoftwareSerial(7, 6);
Adafruit_BluefruitLE_UART ble(bluefruitSS, 9, 8, -1);
SdFat sd;
SdFile myFile;
boolean newGPS = false;
boolean SDavailable = false;
char filename[] = "testingcsv.csv"; 

void setup() {
  Serial.begin(9600);
  ss.begin(9600);
  delay(2000);
  pinMode(2, OUTPUT);

  if ( !ble.begin() )
  {
    Serial.println(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  else{
    ble.echo(false);
    ble.setMode(BLUEFRUIT_MODE_DATA);
  }
  
  SDavailable = sd.begin(10, SPI_HALF_SPEED);
  if(SDavailable){
    create_file();
  }
  digitalWrite(2, HIGH);
}

void loop() {
  ss.listen();
  for (unsigned long start = millis(); millis() - start < 20;)
  {
    while (ss.available())
    {
      char c = ss.read();
      if (gps.encode(c))  
        newGPS = true;
    }
  }

  if(newGPS){
    print_GPS_measurements();
  }
}

void create_file(){
  if (myFile.open(filename, O_WRITE | O_CREAT | O_TRUNC)) {
    delay(100);
    myFile.println(F("\"AccX\",\"AccY\",\"AccZ\",\"MagX\",\"MagY\",\"MagZ\",\"GyrX\",\"GyrY\",\"GyrZ\",\"Lat\",\"Lon\",\"Alt\",\"Sat\""));
  }
  myFile.close();
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
  

    ble.print(F("LAT="));
    ble.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    ble.print(F(" LON="));
    ble.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
    ble.print(F(" ALT="));
    ble.print(gps.altitude() == TinyGPS::GPS_INVALID_ALTITUDE ? 0 : gps.altitude());
    ble.print(F(" SAT="));
    ble.println(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());

    if(SDavailable){     
      myFile.open(filename, O_WRITE | O_AT_END);
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
  }
} 
