#include <SdFat.h>
#include <SPI.h>

SdFat sd;
SdFile myFile;

const int chipSelect = 10;
char filename[] = "test.txt"; // need to change to date/time

void setup() {
  // begin
  Serial.begin(115200);
  while (!Serial) { 
     ; 
  }
  
  // initialize
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)) sd.initErrorHalt();

  // check if it will open
  if (!myFile.open(filename, O_WRITE | O_CREAT | O_AT_END)) {
    sd.errorHalt("opening test.txt for write failed");
  }

  // write to file
  Serial.println("Writing headers to ");
  Serial.println(filename);
  myFile.println("Latitude, Longitude, Elevation (WHAT MODEL), Number of Satellites, Angular Velocity (rad/s), Acceleration (m/s^2), Magnetic Field (uT)");
  //any other information that needs to be written!  
  Serial.println("Headers written");
  
  // close file
  myFile.close();

  // open file to read
  if (!myFile.open(filename, O_READ)) {
    sd.errorHalt("Error!!!");
  }
  // read and print file data
  int data;
  while ((data = myFile.read()) >= 0) Serial.write(data);

  // close file
  myFile.close();
}

void loop() {
  // put your main code here, to run repeatedly:

}
