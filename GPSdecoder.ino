#include "gps.h"

//create a GPS object
GPS myGPS(&Serial1);

void setup() {
  Serial1.begin(115200);
  //for communication with PC
  Serial.begin(115200);
}

void loop() {
  if(myGPS.read()==0){
      Serial.print(F("GPS satellite:"));
      Serial.println(myGPS.getSatelliteCount());
      Serial.println(F("GPS fixed"));
      Serial.print("Lat ");
      Serial.print(myGPS.getLat());
      Serial.print(myGPS.getNS());
      Serial.print('\n');
  
      Serial.print("Lon ");
      Serial.print(myGPS.getLon());
      Serial.println(myGPS.getEW());
  }
}
