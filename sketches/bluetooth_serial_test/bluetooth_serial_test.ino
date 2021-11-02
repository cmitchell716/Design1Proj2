#include <SoftwareSerial.h>


SoftwareSerial BT(2,3); //RX, TX
void setup() {
  // put your setup code here, to run once:
  BT.begin(9600);
  Serial.begin(9600);
  Serial.println("ENTER AT Commands:");
}

void loop() {
  // put your main code here, to run repeatedly:
  //String BTMessage = "Hello from Bluetooth";
  //String serialMessage = "Hello from serial";
  //BT.println(BTMessage);
  //Serial.println(serialMessage);

  //Write data from HC06 to Serial Monitor
  if (BT.available()){
    int message = BT.read();
    Serial.write(message);
  }
  
  //Write from Serial Monitor to HC06
  if (Serial.available()){
    int message2 = Serial.read();
    //Serial.write(message2);
    BT.write(message2);
  }  
}
