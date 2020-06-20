#include "gps.h"

//create a GPS object
GPS myGPS(&Serial1);

void setup() {
  Serial1.begin(115200);
  //for communication with PC
  Serial.begin(115200);
}
const char* msg = NULL;
UTCtime localtime;
void loop() {
  msg = myGPS.read();
  if(msg){
      if(myGPS.incoming_msg_type==TYPE_UBX){
        Serial.println("UBX received");
        myGPS.clearMsg();
      } else{
        Serial.print(msg);
        Serial.print(F("GPS satellite no:"));
        Serial.println(myGPS.getSatelliteCount());
        Serial.println(myGPS.isFixed()?F("GPS fixed"):F("GPS not fixed"));
        Serial.print("Lat ");
        Serial.print(myGPS.getLat(),6);
        Serial.print(myGPS.getNS());
        Serial.print('\n');
    
        Serial.print("Lon ");
        Serial.print(myGPS.getLon(),6);
        Serial.println(myGPS.getEW());

        Serial.print("time:");
        localtime = myGPS.getLocalTime();
        Serial.print(localtime.hour);
        Serial.print(':');
        Serial.print(localtime.minute);
        Serial.print(':');
        Serial.println(localtime.second);
        Serial.println();
      }
  }
}
