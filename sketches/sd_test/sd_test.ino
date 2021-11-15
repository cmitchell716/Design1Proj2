#include <SPI.h>
#include <SD.h>

File myFile;

void setup() {
  Serial.begin(115200);
  while (!Serial) { 
     ; 
  }
  if (!SD.begin(10)) {
    Serial.println("Initialization failed!");
    while (1);
  }
  Serial.println("Initialization done.");

  String filename = "SD_write.txt"; // need to change to date/time
  
  // check if old file exists
  if (SD.exists(filename)) {
    SD.remove(filename);
    Serial.println("Old file removed.");
  }
  
  // create new file
  myFile = SD.open(filename, FILE_WRITE);
  if (myFile) {
    Serial.println("Writing headers to " + filename);
    //myFile.println("Latitude, Longitude, Elevation (WHAT MODEL), Number of Satellites, Angular Velocity (rad/s), Acceleration (m/s^2), Magnetic Field (uT)");
    
    //any other information that needs to be written!
    
    Serial.println("Headers written");
  }
  myFile.close();
  
  // read sd card file
  myFile = SD.open(filename);
  if (myFile) {
    while(myFile.available()) {
      Serial.write(myFile.read());
    }
  }
  else {
    Serial.println("error!!!");  
  }
  myFile.close();
}

void loop() {
}
