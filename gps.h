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


class GPS {
    
public:
    GPS(Stream *ts);
    
    int read();
    void readNMEA();
    int parseNMEA();
    void readUBX();
    
    uint8_t getYear();
    uint8_t getMonth();
    uint8_t getDay();
    uint8_t getHour();
    uint8_t getMinute();
    float getSecond();
    int setTimezone(uint8_t offset);
    
    uint8_t isFixed();
    double getLat();
    double getLon();
    char getNS();
    char getEW();
    
    float getSpdInKnots();
    float getSpdInMs();
    float getSpdInKMh();
    
    float getTrueCourse();
    float getVariation();
    char getVarEW();
    
    uint8_t getSatelliteCount();
    float getAntennaAltitude();
    float getAltitude();
    
    float getPosition_dilution();
    float getHorizontal_dilution();
    float getVertical_dilution();

private:
    GPS();
    int parseGGA();//0
    //int parseGSA();//1
    //int parseRMC();//2
    int parseVTG();//3
    
    int getField(int field,const char* buffer_in,char* buffer_out,const int o_length);
    int getCommaPos(const int number,const char* buffer_in,char*& pos);
    int strcmp(const char* str1,const char* str2);

    template <class floatType>
    int str2float(const char* input,floatType* output);

    template <class intType>
    int str2int(const char* input,intType* output);

    inline uint8_t exp10(int);
    inline int char2dec(char);
    inline bool isLegalFigure(char);
    inline int flushSerial();
    
    char* msgBuffer;
    const char* head[4];
    char serialBuffer[120];
    Stream *thisSerial;
    
    //for RMC
    //UTC time
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    float second;
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
    float antenna_altitude;//9
    float altitude;//11
    
    //for VTG
    float speed_kph;
    
    //for GSA
    float position_dilution;
    float horizontal_dilution;
    float vertical_dilution;
};

#endif /* defined(__NEMA0183parser__NEMAGPS__) */
