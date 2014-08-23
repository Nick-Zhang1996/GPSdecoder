// 
// GPSdecoder 
//
// Description of the project
// Developed with [embedXcode](http://embedXcode.weebly.com)
// 
// Author	 	Nick zhang


#include "NEMAGPS.h"
#include <SoftwareSerial.h>
SoftwareSerial gpsSerial(10,11);
GPS myGPS(&gpsSerial);

void setup() {
  
    gpsSerial.begin(9600);
    Serial.begin(9600);
    while (!myGPS.isFixed()) {
        
        Serial.println(F("GPS not fixed"));
        delay(3000);
        myGPS.read();
    }
}


void loop() {
    if(myGPS.read()==0){
    
    
    Serial.println(F("GPS fixed"));
    Serial.println(myGPS.getLat());
    Serial.println(myGPS.getLon());
    
    }
    
    
}
