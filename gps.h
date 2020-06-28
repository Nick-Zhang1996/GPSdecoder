//
//  NEMAGPS.h
//  NEMA0183parser
//
//  Created by Nick zhang on 14-8-12.
//  Copyright (c) 2014年 Nick Zhang. All rights reserved.
//

// Brief		Library header
//
// Project	 	GPSdecoder


// Core library - IDE-based

#include "Arduino.h"

#ifndef __NEMA0183parser__NEMAGPS__
#define __NEMA0183parser__NEMAGPS__

#define GPS_RX_BUFFER_SIZE 1024
#define TYPE_NONE 0
#define TYPE_UBX 1
#define TYPE_NMEA 2

#define TYPE_GGA 1
#define TYPE_TVG 2
typedef struct {
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    float second;
} UTCtime;

class GPS {
    
public:
    GPS(Stream *ts);
    
    void init();
    const char* read();
    const char* readNMEA();
    const char*  readUBX();
    int parseNMEA();
    
    int setTimezone(uint8_t offset);
    UTCtime GPS::getUtcTime();
    UTCtime GPS::getLocalTime();
    
    uint8_t isFixed();
    double getLat();
    double getLon();
    char getNS();
    char getEW();
    
    float getSpdInKnots();
    float getSpdInMs();
    float getSpdInKph();
    
    float getTrueCourse();
    float getVariation();
    char getVarEW();
    
    uint8_t getSatelliteCount();
    float getAntennaAltitude();
    float getAltitude();
    
    float getPosition_dilution();
    float getHorizontal_dilution();
    float getVertical_dilution();
    float getGeoidalSeparation();

    uint8_t getError();
    uint8_t clearError();

    uint16_t len;
    uint8_t incoming_msg_type;
    uint8_t incoming_msg_subtype;
    inline void clearMsg(){ incoming_msg_type = TYPE_NONE; }
    inline void debugmsg(const char* msg){return;Serial.println(msg);}
    //inline void debugmsg(const char* msg){Serial.println(msg);}

private:
    GPS();
    int parseGGA();//0
    //int parseGSA();//1
    //int parseRMC();//2
    int parseVTG();//3
    
    int getField(int field,const char* buffer_in,char* buffer_out,const int o_length);
    int getCommaPos(const int number,const char* buffer_in,char*& pos);
    inline int flushSerial();
    
    uint16_t rx_buffer_index;
    char rx_buffer[GPS_RX_BUFFER_SIZE];
    Stream *thisSerial;
    uint8_t error;
    
    //for RMC
    //UTC time
    //FIXME right now the year,month,date is undefined
    UTCtime time;
    int timezone;
    
    uint8_t is_valid;
    //0: no fix, 6:dead reckon, 5:RTK float, 4:RTK fixed, 1/2 2-3D GNSS fix
    uint8_t quality;
    double lat,lon;
    char orientation_NS;
    char orientation_EW;
    
    float speed_kn;
    float speed_ms;
    float true_course;
    float magnetic_course;
    float variation;
    char varEW;
    
    //for GGS
    uint8_t satellite_count;
    float altitude;
    float geoidal_separation;
    
    //for VTG
    float speed_kph;
    
    //for GSA
    float position_dilution;
    float horizontal_dilution;
    float vertical_dilution;
};

#endif /* defined(__NEMA0183parser__NEMAGPS__) */
