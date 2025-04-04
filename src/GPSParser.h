#ifndef GPS_PARSER_H
#define GPS_PARSER_H

#include <Arduino.h>
#include <TinyGPS++.h>

class GPSParser {
private:
    TinyGPSPlus gps;
    bool newDataAvailable;
    float latitude;
    float longitude;
    float speed;
    float course;
    int satellites;
    int hour, minute, second;
    int day, month, year;
    
public:
    GPSParser();
    bool processGPSData(char c);
    bool isNewDataAvailable() const;
    float getLatitude() const;
    float getLongitude() const;
    float getSpeed() const;
    float getCourse() const;
    int getSatellites() const;
    String getTimeString() const;
    String getDateString() const;
    String getPositionString() const;
};

#endif