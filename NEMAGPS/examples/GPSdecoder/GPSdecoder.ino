#include <Wire.h>
#include "NEMAGPS.h"
#include <SoftwareSerial.h>


//create a softwareserial for the gps
SoftwareSerial gpsSerial(10,11);
//create a GPS object
GPS myGPS(&gpsSerial);

void setup() {
  
    //initialize the softwareserial
    gpsSerial.begin(9600);
  //for communication with PC
    Serial.begin(9600);
    
   
}



void loop() {
    
  
    if(myGPS.read()==0){


        
        if (myGPS.isFixed()==0) {

                Serial.println(F("GPS not fixed"));
                Serial.println(myGPS.getSateNumber());
            
            
        }else {
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
    

    
}
