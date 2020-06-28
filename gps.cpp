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
    //EST
    timezone=-4;
    is_valid=0;
}

GPS::GPS(){
  // this is illegal
  // an interface must be provided
}

void GPS::init(){
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
    
    if (!(thisSerial->available()>=2)) { return NULL; }

    //check first two bytes (sync byte for message type)
    rx_buffer_index = 0;
    char sync_byte = thisSerial->read();

    // 0x24 = $, NEMA message sync byte
    if (sync_byte == 0x24){
      rx_buffer[rx_buffer_index++] = sync_byte;
      debugmsg("found NMEA header");
      return (const char*)readNMEA();
    }
    // UBX message has two sync bytes
    if (sync_byte == 0xb5 && thisSerial->read() == 0x62){
      rx_buffer[rx_buffer_index++] = 0xb5;
      rx_buffer[rx_buffer_index++] = 0x62;
      debugmsg("found UBX header");
      return (const char*)readUBX();
    }
    
    //unknown message type
    return NULL;
}

const char* GPS::readUBX(){
  // ensure that the data has arrived
  // format of ubx:
  // (#bytes) name
  // 2 sync byte (already processed in read())
  // 1 class
  // 1 id
  // 2 length of payload section in little endian
  // n payload
  // 2 checksum
  while(!thisSerial->available()>=4){;}
  //class
  rx_buffer[rx_buffer_index++] = thisSerial->read();
  //id
  rx_buffer[rx_buffer_index++] = thisSerial->read();
  // len of payload
  rx_buffer[rx_buffer_index++] = thisSerial->read();
  rx_buffer[rx_buffer_index++] = thisSerial->read();
  uint16_t payload_len = rx_buffer[rx_buffer_index-2] + (rx_buffer[rx_buffer_index-1] << 8);
  while(!thisSerial->available()>=payload_len){;}
  for(int i=0;i<payload_len+2;i++){
    rx_buffer[rx_buffer_index++] = thisSerial->read();
    if (rx_buffer_index>=GPS_RX_BUFFER_SIZE){
      error = 1;
      return NULL;
    }
  }

  incoming_msg_type = TYPE_UBX;
  len = rx_buffer_index;
  return rx_buffer;
}


// read the remaining NMEA msg to rx_buffer, starting at rx_buffer_index
const char*  GPS::readNMEA(){
  //this part of the function is blocking
  while (1) {
    if(thisSerial->available()){
      rx_buffer[rx_buffer_index++]=thisSerial->read();
      //actual ending is <CR> <LF> i.e. 13 10,this indicates the end of a sentence
      if (rx_buffer[rx_buffer_index-1]==10) {
        rx_buffer[rx_buffer_index++]='\0';
        debugmsg("found ending 0x10");
        debugmsg(rx_buffer);
        break;
      }

      if (rx_buffer_index>=GPS_RX_BUFFER_SIZE){
        error = 1;
        return NULL;
      }
    }
  }
  debugmsg("parsing NMEA");
  parseNMEA();
  debugmsg("parsing complete");

  incoming_msg_type = TYPE_NMEA;
  len = rx_buffer_index;
  return rx_buffer;
}

// GPS fix data
//  1) Time (UTC)
//  2) Latitude
//  3) N or S (North or South)
//  4) Longitude
//  5) E or W (East or West)
//  6) GPS Quality Indicator,
//  0 - fix not available,
//  1 - GPS fix,
//  2 - Differential GPS fix
//  7) Number of satellites in view, 00 - 12
//  8) Horizontal Dilution of precision
//  9) Antenna Altitude above/below mean-sea-level (geoid)
// 10) Units of antenna altitude, meters
// 11) Geoidal separation, the difference between the WGS-84 earth
//  ellipsoid and mean-sea-level (geoid), "-" means mean-sea-level below ellipsoid
// 12) Units of geoidal separation, meters
// 13) Age of differential GPS data, time in seconds since last SC104
//  type 1 or 9 update, null field when DGPS is not used
// 14) Differential reference station ID, 0000-1023
// 15) Checksum
int GPS::parseGGA(){
    char buffer[15];
    //1 UTC time hhmmss.ss
    getField(1, rx_buffer, buffer, 15);
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
    getField(2, rx_buffer, buffer, 15);
    lat = atof(buffer+2);
    lat /= 60.0;
    degree = atoi(buffer);
    lat += degree/100;

    getField(3, rx_buffer, buffer, 15);
    orientation_NS = buffer[0];
    getField(4, rx_buffer, buffer, 15);
    lon = atof(buffer+3);
    lon /= 60.0;
    degree = atoi(buffer);
    lon += degree/100;

    getField(5, rx_buffer, buffer, 15);
    orientation_EW = buffer[0];
    getField(6, rx_buffer, buffer, 15);
    quality = atoi(buffer);
    if (quality == 1 || quality == 2){
      is_valid = 1;
    } else {
      is_valid = 0;
    }

    getField(7, rx_buffer, buffer, 15);
    satellite_count = atoi(buffer);
    getField(8, rx_buffer, buffer, 15);
    horizontal_dilution = atof(buffer);
    getField(9, rx_buffer, buffer, 15);
    altitude = atof(buffer);
    getField(11, rx_buffer, buffer, 15);
    geoidal_separation = atof(buffer);
    incoming_msg_subtype = TYPE_GGA;

    return 0;
}

int GPS::parseVTG(){
    char buffer[10];
    getField(1, rx_buffer, buffer, 10);
    true_course = atof(buffer);
    getField(3, rx_buffer, buffer, 10);
    magnetic_course = atof(buffer);
    getField(7, rx_buffer, buffer, 10);
    speed_kph = atof(buffer);
    incoming_msg_subtype = TYPE_TVG;
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
    //remove the preceding '$'
    getField(0, rx_buffer+1, temp, 8);
    
    if (!strcmp(temp, "GNGGA")) {
        debugmsg("Parsing GGA");
        parseGGA();
    }   else if (!strcmp(temp, "GNGSA")){
        //parseGSA();
    }   else if (!strcmp(temp, "GNRMC")){
        //parseRMC();
    }   else if (!strcmp(temp, "GNVTG")){
        debugmsg("Parsing VTG");
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
float GPS::getSpdInKph(){
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

float GPS::getAltitude(){
    return altitude;
}

float GPS::getGeoidalSeparation(){
    return geoidal_separation;
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

uint8_t GPS::getError(){
    return error;
}

uint8_t GPS::clearError(){
    return error=0;
}
