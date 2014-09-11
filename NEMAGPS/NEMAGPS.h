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
#include <SoftwareSerial.h>

#ifndef __NEMA0183parser__NEMAGPS__
#define __NEMA0183parser__NEMAGPS__


class GPS {
    
public:
    GPS(Stream *ts);
    GPS();
    
    int read();
    int parseData();
    int parseData(char* thisBuffer);
    
    uint8_t getYear();
    uint8_t getMonth();
    uint8_t getDay();
    uint8_t getHour();
    uint8_t getMinute();
    double getSecond();
    int setTimezone(uint8_t offset);
    
    uint8_t isFixed();
    double getLat();
    double getLon();
    char getNS();
    char getEW();
    
    double getSpdInKnots();
    double getSpdInMs();
    double getSpdInKMh();
    
    double getTrueCourse();
    double getVariation();
    char getVarEW();
    
    uint8_t getSateNumber();
    double getAlt();
    double getAlt2ellip();
    
    double getPositionDilution();
    double getHorizontalDilution();
    double getVerticalDilution();

//private:
    int parseGGA();//0
    int parseGSA();//1
    int parseRMC();//2
    int parseVTG();//3
    
    int getField(int field,const char* buffer_in,char* buffer_out,const int o_length);
    int getCommaPos(const int number,const char* buffer_in,char*& pos);
    int strcmp(const char* str1,const char* str2);
    int str2db(const char* input,double* output);
    int str2int(const char* input,uint32_t* output);
    int str2int(const char* input,uint8_t* output);
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
    double second;
    int timezone;
    
    
    
    uint8_t isValid;
    double lat,lon;
    char oriNS;
    char oriEW;
    
    
    
    double spdInKnots;
    double spdInMs;
    double trueCourse;
    double variation;
    char varEW;
    
    
    
    //for GGS
    uint8_t sateNumber;
    double alt;//9
    double alt2ellip;//11
    
    
    
    //for VTG
    double spdInKMh;
    
    
    
    //for GSA
    double positionDilution;
    double horizontalDilution;
    double verticalDilution;
    
    
};

#endif /* defined(__NEMA0183parser__NEMAGPS__) */
