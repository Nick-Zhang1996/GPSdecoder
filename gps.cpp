//
//  NEMAGPS.cpp
//  NEMA0183parser
//
//  Created by Nick zhang on 14-8-12.
//  Copyright (c) 2014年 Nick Zhang. All rights reserved.
//

#include "gps.h"

GPS::GPS(Stream *ts){
    thisSerial=ts;
    msgBuffer=serialBuffer;
    //EST
    timezone=-4;
    is_valid=0;
}

GPS::GPS(){
  // this is illegal
  // an interface must be provided
}

/*return 0 indicates success
note: this is a "semi-blocking" function. If there is no data in the buffer,
 function returns;however,if there is data,the function blocks until
 the whole sentence is read. It waits till the GPS complete the session,most time is wasted. Receiving a sentence typically cost 20-100milliseconds,depending on the sentence type
 
 */
// return msgbuffer if a valid NMEA sentence is received
const char* GPS::read(){
  //this need to be run at a high frequency, for the buffer length of SoftwareSerial port is only 64 bytes, which is relativly short since NEMA sentence is about 100 bytes---i take this back
    if (thisSerial==NULL) { return NULL; }
    
    if (!thisSerial->available()) { return NULL; }

    //check first two bytes (sync byte for message type)
    char sync_byte = thisSerial->read();

    // 0x24 = $, NEMA message sync byte
    if (sync_byte == 0x24){
      readNMEA();
      return (const char*)msgBuffer;
    }
    // UBX message has two sync bytes
    if (sync_byte == 0xb5 && thisSerial->read() == 0x62){
      readUBX();
      return NULL;
    }
    
    //unknown message type
    return NULL;
}

void GPS::readNMEA(){
  int i=0;
  
  //this part of the function is blocking
  while (1) {
    if(thisSerial->available()){
      *(msgBuffer+i)=thisSerial->read();
      //actual ending is <CR> <LF> i.e. 13 10,this indicates the end of a sentence
      if (*(msgBuffer+i)==10) {
        *(msgBuffer+i)='\0';
        break;
      }
      i++;
    }
  }
  //Serial.println(msgBuffer);
  parseNMEA();
}

// GPS fix data
int GPS::parseGGA(){
    char buffer[15];
    //1 UTC time hhmmss.ss
    getField(1, msgBuffer, buffer, 15);
    //4 is an offset to second field
    time.second = atof(buffer+4);
    uint32_t utc_time;
    //get the integer part
    utc_time = atof(buffer);
    //get rid of second field
    utc_time/=100;
    time.minute=utc_time%100;
    utc_time/=100;
    time.hour=utc_time;

    int degree=0;
    getField(2, msgBuffer, buffer, 15);
    lat = atof(buffer+2);
    lat /= 60.0;
    degree = atoi(buffer);
    lat += degree/100;

    getField(3, msgBuffer, buffer, 15);
    orientation_NS = buffer[0];
    getField(4, msgBuffer, buffer, 15);
    lon = atof(buffer+3);
    lon /= 60.0;
    degree = atoi(buffer);
    lon += degree/100;

    getField(5, msgBuffer, buffer, 15);
    orientation_EW = buffer[0];
    getField(6, msgBuffer, buffer, 15);
    quality = atoi(buffer);
    if (quality == 1 || quality == 2){
      is_valid = 1;
    } else {
      is_valid = 0;
    }

    getField(7, msgBuffer, buffer, 15);
    satellite_count = atoi(buffer);
    getField(8, msgBuffer, buffer, 15);
    horizontal_dilution = atof(buffer);
    getField(9, msgBuffer, buffer, 15);
    antenna_altitude = atof(buffer);
    getField(11, msgBuffer, buffer, 15);
    altitude = atof(buffer);

    return 0;
}

int GPS::parseVTG(){
    char buffer[10];
    getField(1, msgBuffer, buffer, 10);
    true_course = atof(buffer);
    getField(3, msgBuffer, buffer, 10);
    magnetic_course = atof(buffer);
    getField(7, msgBuffer, buffer, 10);
    speed_kph = atof(buffer);
    return 0;
}

/*
//note:this function contains no error check methods
int GPS::parseRMC(){
    char buffer[15];
    getField(2, msgBuffer, buffer, 15);
    if (strcmp(buffer, "V")) {
        //GPS is not fixed
        is_valid=0;
        return 0;
    }
    
    //GPS is fixed, parse data
    
    is_valid=1;
    getField(1, msgBuffer, buffer, 15);
    //4 is an offset to second field
    str2float<float>(buffer+4, &second);
    uint32_t utcTime;
    //get the integer part
    str2int(buffer, &utcTime);
    //get rid of second field
    utcTime/=100;
    minute=utcTime%100;
    utcTime/=100;
    hour=utcTime;
    
    getField(3, msgBuffer, buffer, 15);
    str2float<double>(buffer, &lon);
    getField(4, msgBuffer, buffer, 15);
    orientation_NS=buffer[0];
    getField(5, msgBuffer, buffer, 15);
    str2float<double>(buffer, &lat);
    getField(6, msgBuffer, buffer, 15);
    oriEW=buffer[0];
    
    getField(7, msgBuffer, buffer, 15);
    str2float<float>(buffer, &speed_kn);
    speed_ms=speed_kn*0.5144444444;
    getField(8, msgBuffer, buffer, 15);
    str2float<float>(buffer, &true_course);
    
    //get the utc date ddmmyy
    getField(9, msgBuffer, buffer, 15);
    uint32_t utcDate;
    str2int(buffer, &utcDate);
    
    year=utcDate%100;
    utcDate/=100;
    month=utcDate%100;
    utcDate/=100;
    day=utcDate;
    
    getField(10, msgBuffer, buffer, 15);
    str2float<float>(buffer, &variation);
    getField(11, msgBuffer, buffer, 15);
    varEW=buffer[0];
    
    return 0;
    
}

int GPS::parseGSA(){
    char buffer[10];
    //precision information
    getField(15, msgBuffer, buffer, 10);
    str2db(buffer, &position_dilution);
    getField(16, msgBuffer, buffer, 10);
    str2db(buffer, &horizontal_dilution);
    getField(17, msgBuffer, buffer, 10);
    str2db(buffer, &vertical_dilution);
    
    return 0;
}
*/

// parse the sentence in msgBuffer, call appropriate decoder
int GPS::parseNMEA(){
    char temp[8];
    getField(0, msgBuffer, temp, 8);
    
    if (!strcmp(temp, "GNGGA")) {
        //Serial.println("gga parsing");
        parseGGA();
    }   else if (!strcmp(temp, "GNGSA")){
        //parseGSA();
    }   else if (!strcmp(temp, "GNRMC")){
        //parseRMC();
    }   else if (!strcmp(temp, "GNVTG")){
        //Serial.println("vtg parsing");
        parseVTG();
    }   else {return 1;}//unknown header
    
    return 0;
    
}

//retrive the FIELDth field seperated by ',', store the retrived string in BUFFER_OUT
//0th field = message ID (GPGGA, etc)
int GPS::getField(const int field,const char* buffer_in,char* buffer_out,const int o_length){
    char* start;
    char* end;
    int rval=getCommaPos(field, buffer_in, start);
    if (rval) { return -1; }
    rval=getCommaPos(field+1, buffer_in,end);
    if (rval) { return -1; }
    
    //not enough space to store,rarely happens
    if (o_length<end-start) { return -1; }
    
    long offset;
    for (char* i=start+1; i<end; i++) {
        offset=i-start-1;
        *(buffer_out+offset)=*i;
    }
    
    offset=end-start-1;
    *(buffer_out+offset)='\0';
    return 0;
}


//get the position of number NUMBER comma in BUFFER_IN, and store the pointer in POS. if an error occurs, set POS to NULL and return -1
int GPS::getCommaPos(const int number,const char* buffer_in,char*& pos){
    int count=0;
    pos=NULL;
    
    if (number<1) {
        // for compatibility
        if (number==0) {
            pos=const_cast<char*>(buffer_in)-1;
            return 0;
        }
        return -1;
    }
    
    char* i=const_cast<char*>(buffer_in);
    
    while ((*i)!='\0') {
        if ((*i)==',') {
            count++;
            if (count==number) {
                pos=i;
                return 0;
            }
        }
        i++;
    }
    // for compatibility
    if (++count==number) {
        pos=i;
        return 0;
    }
    return -1;
}

int GPS::flushSerial(){
    if (thisSerial==NULL) {
        return -1;
    }
    
    //clear the buffer
    while (thisSerial->available()) {
        thisSerial->read();
    }
}

UTCtime GPS::getUtcTime(){
    return time;
}

UTCtime GPS::getLocalTime(){
    UTCtime local_time = time;
    local_time.hour += 24;
    local_time.hour += timezone;
    local_time.hour %= 24;
    return local_time;
}

int GPS::setTimezone(uint8_t offset){
    timezone=offset;
    return 0;
}

uint8_t GPS::isFixed(){
    return is_valid;
}

double GPS::getLat(){
    return lat;
}
double GPS::getLon(){
    return lon;
}
char GPS::getNS(){
    return orientation_NS;
}
char GPS::getEW(){
    return orientation_EW;
}

float GPS::getSpdInKnots(){
    return speed_kn;
}
float GPS::getSpdInMs(){
    return speed_ms;
}
float GPS::getSpdInKMh(){
    return speed_kph;
}

float GPS::getTrueCourse(){
    return true_course;
}
float GPS::getVariation(){
    return variation;
}
char GPS::getVarEW(){
    return varEW;
}

uint8_t GPS::getSatelliteCount(){
    return satellite_count;
}
float GPS::getAntennaAltitude(){
    return antenna_altitude;
}
float GPS::getAltitude(){
    return altitude;
}

float GPS::getPosition_dilution(){
    return position_dilution;
}
float GPS::getHorizontal_dilution(){
    return horizontal_dilution;
}
float GPS::getVertical_dilution(){
    return vertical_dilution;
}
