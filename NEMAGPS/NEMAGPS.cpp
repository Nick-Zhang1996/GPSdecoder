//
//  NEMAGPS.cpp
//  NEMA0183parser
//
//  Created by Nick zhang on 14-8-12.
//  Copyright (c) 2014年 Nick Zhang. All rights reserved.
//

#include "NEMAGPS.h"

GPS::GPS(SoftwareSerial *ts){
    thisSerial=ts;
    msgBuffer=serialBuffer;
    *head="$GPGGA";
    *(head+1)="$GPGSA";
    *(head+2)="$GPRMC";
    *(head+3)="$GPVTG";
    //i'm chinese
    timezone=8;
    isValid=0;
}

GPS::GPS(){
    thisSerial=NULL;
    msgBuffer=NULL;
    *head="$GPGGA";
    *(head+1)="$GPGSA";
    *(head+2)="$GPRMC";
    *(head+3)="$GPVTG";
    //i'm chinese
    timezone=8;
    isValid=0;
}

/*return 0 indicates success
note: this is a "semi-blocking" function. If there is no data in the buffer,
 function returns;however,if there is data,the function blocks until
 the whole sentence is read. It waits till the GPS complete the session,most time is wasted. Receiving a sentence typically cost 20-100milliseconds,depending on the sentence type
 
 */
int GPS::read(){
  //this need to be run at a high frequency, for the buffer length of SoftwareSerial port is only 64 bytes, which is relativly short since NEMA sentence is about 100 bytes---i take this back
    if (thisSerial==NULL) {
        return -2;
    }
    
    if (!thisSerial->available()) {
        return -1;
    } else
    
    {
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
    
    
    
}


    parseData();

    return 0;
}

int GPS::parseGGA(){
    //DGPS is scarcely available, lat and long is available from RMC, thus they are not implemented in this program
    
    char buffer[15];
    getField(7, msgBuffer, buffer, 15);
    str2int(buffer, &sateNumber);
    
    //implemented in GSA
    //getField(8, msgBuffer, buffer, 15);
    //str2db(buffer, &horizontalDilution);
    getField(9, msgBuffer, buffer, 15);
    str2db(buffer, &alt);
    getField(11, msgBuffer, buffer, 15);
    str2db(buffer, &alt2ellip);
    
    
    return 0;
}

int GPS::parseVTG(){
    char buffer[10];
    getField(7, msgBuffer, buffer, 10);
    str2db(buffer, &spdInKMh);
    return 0;
}
//note:this function contains no error check methods
int GPS::parseRMC(){
    char buffer[15];
    getField(2, msgBuffer, buffer, 15);
    if (strcmp(buffer, "V")) {
        //GPS is not fixed
        isValid=0;
        return 0;
    }
    
    //GPS is fixed, parse data
    
    isValid=1;
    getField(1, msgBuffer, buffer, 15);
    //4 is an offset to second field
    str2db(buffer+4, &second);
    uint32_t utcTime;
    //get the integer part
    str2int(buffer, &utcTime);
    //get rid of second field
    utcTime/=100;
    minute=utcTime%100;
    utcTime/=100;
    hour=utcTime;
    
    getField(3, msgBuffer, buffer, 15);
    str2db(buffer, &lon);
    getField(4, msgBuffer, buffer, 15);
    oriNS=buffer[0];
    getField(5, msgBuffer, buffer, 15);
    str2db(buffer, &lat);
    getField(6, msgBuffer, buffer, 15);
    oriEW=buffer[0];
    
    getField(7, msgBuffer, buffer, 15);
    str2db(buffer, &spdInKnots);
    spdInMs=spdInKnots*0.5144444444;
    getField(8, msgBuffer, buffer, 15);
    str2db(buffer, &trueCourse);
    
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
    str2db(buffer, &variation);
    getField(11, msgBuffer, buffer, 15);
    varEW=buffer[0];
    
    return 0;
    
}

int GPS::parseGSA(){
    char buffer[10];
    
    //GPS satellite data is not implemented
    
    
    //precision information
    getField(15, msgBuffer, buffer, 10);
    str2db(buffer, &positionDilution);
    getField(16, msgBuffer, buffer, 10);
    str2db(buffer, &horizontalDilution);
    getField(17, msgBuffer, buffer, 10);
    str2db(buffer, &verticalDilution);
    
    return 0;
}

int GPS::parseData(){
    if (msgBuffer==NULL) {
        return 1;
    }
    
    //actual length is 7
    char temp[8];
    getField(0, msgBuffer, temp, 8);
    
    if (strcmp(temp, "$GPGGA")) {
        parseGGA();
    }   else if (strcmp(temp, "$GPGSA")){
        parseGSA();
    }   else if (strcmp(temp, "$GPRMC")){
        parseRMC();
    }   else if (strcmp(temp, "$GPVTG")){
        parseVTG();
    }   else {return 1;}//unknown header
    
    return 0;
    
}

int GPS::parseData(char* thisBuffer){
    msgBuffer=thisBuffer;
    if (msgBuffer==NULL) {
        return 1;
    }
    
    //actual length is 7
    char temp[8];
    getField(0, msgBuffer, temp, 8);
    
    if (strcmp(temp, "$GPGGA")) {
        parseGGA();
    }   else if (strcmp(temp, "$GPGSA")){
        parseGSA();
    }   else if (strcmp(temp, "$GPRMC")){
        parseRMC();
    }   else if (strcmp(temp, "$GPVTG")){
        parseVTG();
    }   else {return 1;}//unknown header
    
    return 0;
    
}

//1 indicates true;
int GPS::strcmp(const char* str1,const char* str2){
    int i=0;
    while (*(str1+i)!='\0' && *(str2+i)!='\0') {
        if (*(str1+i)!=*(str2+i)) {
            return 0;
        }
        i++;
    }
    
    if (*(str1+i)!='\0' || *(str2+i)!='\0') {
        return 0;
    }
    
    return 1;
}
//retrive the FIELDth field seperated by ',', store the retrived string in BUFFER_OUT
int GPS::getField(const int field,const char* buffer_in,char* buffer_out,const int o_length){
    char* start;
    char* end;
    int rval=getCommaPos(field, buffer_in, start);
    if (rval) {
        return -1;
    }
    rval=getCommaPos(field+1, buffer_in,end);
    if (rval) {
        return -1;
    }
    
    //not enough space to store,rarely happens
    if (o_length<end-start) {
        return -1;
    }
    
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

uint8_t GPS::exp10(int e){
    if (e<0) {
        return -1;
    }
    
    uint8_t rval=1;
    
    for (int i=0; i<e; i++) {
        rval=rval*10;
    }
    
    return rval;
}

//whether the char C is a number or dicimal point
bool GPS::isLegalFigure(char c){
    if ((c<='9' && c>='0') || c=='.') {
        return true;
    } else {return false;}
}

int GPS::char2dec(char c){
    return c-48;
}

//convert a string to double,capable of dealing decimal point
int GPS::str2db(const char* input,double* output){
    int afterP=0;
    char* thisFigure=const_cast<char*>(input);
    double inte=0.0;
    double frac=0.0;
    
    while (1) {
        if (*thisFigure=='\0') {
            break;
        }
        if (!isLegalFigure(*thisFigure)) {
            *output=0;
            return -1;
        }
        
        if (*thisFigure=='.') {
            if (afterP) {
                //there are more than one decimal point!
                *output=0;
                return -1;
            }
            afterP=1;
            thisFigure++;
            continue;
        }
        
        if (afterP) {
            frac+=static_cast<double>(char2dec(*thisFigure))/static_cast<double>(exp10(afterP));
            
            afterP++;
        } else {
            inte=inte*10+char2dec(*thisFigure);
        }
        
        thisFigure++;
        
    }
    
    *output=inte+frac;
    return 0;
    
}

int GPS::str2int(const char* input,uint32_t* output){
    
    char* thisFigure=const_cast<char*>(input);
    uint32_t inte=0;
    
    
    while (1) {
        //if INPUT contains a decimal point, parse only the integer part
        if (*thisFigure=='\0'|| *thisFigure=='.') {
            break;
        }
        if (!isLegalFigure(*thisFigure)) {
            *output=0;
            return -1;
        }
        
        inte=inte*10+char2dec(*thisFigure);
        thisFigure++;
        
    }
    
    *output=inte;
    return 0;
}

int GPS::str2int(const char* input,uint8_t* output){
    
    char* thisFigure=const_cast<char*>(input);
    uint32_t inte=0;
    
    
    while (1) {
        //if INPUT contains a decimal point, parse only the integer part
        if (*thisFigure=='\0'|| *thisFigure=='.') {
            break;
        }
        if (!isLegalFigure(*thisFigure)) {
            *output=0;
            return -1;
        }
        
        inte=inte*10+char2dec(*thisFigure);
        thisFigure++;
        
    }
    
    *output=inte;
    return 0;
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


uint8_t GPS::getYear(){
    return year;
}

uint8_t GPS::getMonth(){
    return month;
}
uint8_t GPS::getDay(){
    return day;
}
uint8_t GPS::getHour(){
    return hour;
}
uint8_t GPS::getMinute(){
    return minute;
}
double GPS::getSecond(){
    return second;
}
int GPS::setTimezone(uint8_t offset){
    timezone=offset;
    return 0;
}

uint8_t GPS::isFixed(){
    return isValid;
}
double GPS::getLat(){
    return lat;
}
double GPS::getLon(){
    return lon;
}
char GPS::getNS(){
    return oriNS;
}
char GPS::getEW(){
    return oriEW;
}

double GPS::getSpdInKnots(){
    return spdInKnots;
}
double GPS::getSpdInMs(){
    return spdInMs;
}
double GPS::getSpdInKMh(){
    return spdInKMh;
}

double GPS::getTrueCourse(){
    return trueCourse;
}
double GPS::getVariation(){
    return variation;
}
char GPS::getVarEW(){
    return varEW;
}

uint8_t GPS::getSateNumber(){
    return sateNumber;
}
double GPS::getAlt(){
    return alt;
}
double GPS::getAlt2ellip(){
    return alt2ellip;
}

double GPS::getPositionDilution(){
    return positionDilution;
}
double GPS::getHorizontalDilution(){
    return horizontalDilution;
}
double GPS::getVerticalDilution(){
    return verticalDilution;
}
