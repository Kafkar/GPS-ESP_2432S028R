#include "GPSParser.h"

GPSParser::GPSParser() : 
    newDataAvailable(false),
    latitude(0.0),
    longitude(0.0),
    speed(0.0),
    course(0.0),
    satellites(0),
    hour(0), minute(0), second(0),
    day(0), month(0), year(0) {
}

bool GPSParser::processGPSData(char c) {
    if (gps.encode(c)) {
        if (gps.location.isUpdated()) {
            latitude = gps.location.lat();
            longitude = gps.location.lng();
            newDataAvailable = true;
        }
        
        if (gps.speed.isUpdated()) {
            speed = gps.speed.knots();
        }
        
        if (gps.course.isUpdated()) {
            course = gps.course.deg();
        }
        
        if (gps.satellites.isUpdated()) {
            satellites = gps.satellites.value();
        }
        
        if (gps.time.isUpdated() && gps.time.isValid()) {
            hour = gps.time.hour();
            minute = gps.time.minute();
            second = gps.time.second();
        }
        
        if (gps.date.isUpdated() && gps.date.isValid()) {
            day = gps.date.day();
            month = gps.date.month();
            year = gps.date.year();
        }
        
        return newDataAvailable;
    }
    return false;
}

bool GPSParser::isNewDataAvailable() const {
    return newDataAvailable;
}

float GPSParser::getLatitude() const {
    return latitude;
}

float GPSParser::getLongitude() const {
    return longitude;
}

float GPSParser::getSpeed() const {
    return speed;
}

float GPSParser::getCourse() const {
    return course;
}

int GPSParser::getSatellites() const {
    return satellites;
}

String GPSParser::getTimeString() const {
    char buffer[9];
    sprintf(buffer, "%02d:%02d:%02d", hour, minute, second);
    return String(buffer);
}

String GPSParser::getDateString() const {
    char buffer[11];
    sprintf(buffer, "%02d/%02d/%04d", day, month, year);
    return String(buffer);
}

String GPSParser::getPositionString() const {
    char buffer[30];
    sprintf(buffer, "%.6f, %.6f", latitude, longitude);
    return String(buffer);
}