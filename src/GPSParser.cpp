#include "GPSParser.h"

GPSParser::GPSParser() {
    // Initialize with default values
}

void GPSParser::processGPSData(char c) {
    // If we get a newline, process the current sentence
    if (c == '\n') {
        // Check if it's a valid NMEA sentence (starts with $)
        if (currentSentence.startsWith("$")) {
            // Determine sentence type
            if (currentSentence.startsWith("$GPGGA") || currentSentence.startsWith("$GNGGA")) {
                parseGGA(currentSentence);
            }
            else if (currentSentence.startsWith("$GPRMC") || currentSentence.startsWith("$GNRMC")) {
                parseRMC(currentSentence);
            }
            else if (currentSentence.startsWith("$GPGSA") || currentSentence.startsWith("$GNGSA")) {
                parseGSA(currentSentence);
            }
        }
        
        // Reset for next sentence
        currentSentence = "";
    }
    // Otherwise, add the character to the current sentence
    else if (c != '\r') {
        currentSentence += c;
    }
}

void GPSParser::parseGGA(String sentence) {
    // $GPGGA,time,lat,N/S,lon,E/W,quality,satellites,hdop,altitude,M,geoidSep,M,dgpsAge,dgpsStationId*checksum
    
    // Split the sentence into fields
    int commaIndex = 0;
    int nextCommaIndex = 0;
    
    // Skip the sentence identifier
    commaIndex = sentence.indexOf(',');
    
    // Get time
    nextCommaIndex = sentence.indexOf(',', commaIndex + 1);
    if (nextCommaIndex > commaIndex + 1) {
        time = sentence.substring(commaIndex + 1, nextCommaIndex);
        newData = true;
    }
    commaIndex = nextCommaIndex;
    
    // Get latitude
    nextCommaIndex = sentence.indexOf(',', commaIndex + 1);
    String latStr = sentence.substring(commaIndex + 1, nextCommaIndex);
    commaIndex = nextCommaIndex;
    
    // Get latitude direction (N/S)
    nextCommaIndex = sentence.indexOf(',', commaIndex + 1);
    latDir = sentence.charAt(commaIndex + 1);
    commaIndex = nextCommaIndex;
    
    // Get longitude
    nextCommaIndex = sentence.indexOf(',', commaIndex + 1);
    String lonStr = sentence.substring(commaIndex + 1, nextCommaIndex);
    commaIndex = nextCommaIndex;
    
    // Get longitude direction (E/W)
    nextCommaIndex = sentence.indexOf(',', commaIndex + 1);
    lonDir = sentence.charAt(commaIndex + 1);
    commaIndex = nextCommaIndex;
    
    // Get fix quality
    nextCommaIndex = sentence.indexOf(',', commaIndex + 1);
    int fixQuality = sentence.substring(commaIndex + 1, nextCommaIndex).toInt();
    validPosition = (fixQuality > 0);
    commaIndex = nextCommaIndex;
    
    // Get number of satellites
    nextCommaIndex = sentence.indexOf(',', commaIndex + 1);
    satellites = sentence.substring(commaIndex + 1, nextCommaIndex).toInt();
    commaIndex = nextCommaIndex;
    
    // Get HDOP
    nextCommaIndex = sentence.indexOf(',', commaIndex + 1);
    String hdopStr = sentence.substring(commaIndex + 1, nextCommaIndex);
    if (hdopStr.length() > 0) {
        hdop = hdopStr.toFloat();
        validHDOP = true;
    }
    commaIndex = nextCommaIndex;
    
    // Get altitude
    nextCommaIndex = sentence.indexOf(',', commaIndex + 1);
    String altStr = sentence.substring(commaIndex + 1, nextCommaIndex);
    if (altStr.length() > 0) {
        altitude = altStr.toFloat();
        validAltitude = true;
    }
    
    // Convert latitude and longitude to decimal degrees
    if (latStr.length() > 0 && lonStr.length() > 0) {
        latitude = convertToDecimalDegrees(latStr, latDir);
        longitude = convertToDecimalDegrees(lonStr, lonDir);
    }
}

void GPSParser::parseRMC(String sentence) {
    // $GPRMC,time,status,lat,N/S,lon,E/W,speed,course,date,magnetic variation,E/W*checksum
    
    // Split the sentence into fields
    int commaIndex = 0;
    int nextCommaIndex = 0;
    
    // Skip the sentence identifier
    commaIndex = sentence.indexOf(',');
    
    // Get time
    nextCommaIndex = sentence.indexOf(',', commaIndex + 1);
    if (nextCommaIndex > commaIndex + 1) {
        time = sentence.substring(commaIndex + 1, nextCommaIndex);
        newData = true;
    }
    commaIndex = nextCommaIndex;
    
    // Get status (A=active, V=void)
    nextCommaIndex = sentence.indexOf(',', commaIndex + 1);
    char status = sentence.charAt(commaIndex + 1);
    validPosition = (status == 'A');
    commaIndex = nextCommaIndex;
    
    // Get latitude
    nextCommaIndex = sentence.indexOf(',', commaIndex + 1);
    String latStr = sentence.substring(commaIndex + 1, nextCommaIndex);
    commaIndex = nextCommaIndex;
    
    // Get latitude direction (N/S)
    nextCommaIndex = sentence.indexOf(',', commaIndex + 1);
    latDir = sentence.charAt(commaIndex + 1);
    commaIndex = nextCommaIndex;
    
    // Get longitude
    nextCommaIndex = sentence.indexOf(',', commaIndex + 1);
    String lonStr = sentence.substring(commaIndex + 1, nextCommaIndex);
    commaIndex = nextCommaIndex;
    
    // Get longitude direction (E/W)
    nextCommaIndex = sentence.indexOf(',', commaIndex + 1);
    lonDir = sentence.charAt(commaIndex + 1);
    commaIndex = nextCommaIndex;
    
    // Get speed
    nextCommaIndex = sentence.indexOf(',', commaIndex + 1);
    String speedStr = sentence.substring(commaIndex + 1, nextCommaIndex);
    if (speedStr.length() > 0) {
        speed = speedStr.toFloat();
    }
    commaIndex = nextCommaIndex;
    
    // Get course
    nextCommaIndex = sentence.indexOf(',', commaIndex + 1);
    String courseStr = sentence.substring(commaIndex + 1, nextCommaIndex);
    if (courseStr.length() > 0) {
        course = courseStr.toFloat();
    }
    commaIndex = nextCommaIndex;
    
    // Get date
    nextCommaIndex = sentence.indexOf(',', commaIndex + 1);
    if (nextCommaIndex > commaIndex + 1) {
        date = sentence.substring(commaIndex + 1, nextCommaIndex);
    }
    
    // Convert latitude and longitude to decimal degrees
    if (latStr.length() > 0 && lonStr.length() > 0) {
        latitude = convertToDecimalDegrees(latStr, latDir);
        longitude = convertToDecimalDegrees(lonStr, lonDir);
    }
}

void GPSParser::parseGSA(String sentence) {
    // $GPGSA,mode,fixType,satellites used...,PDOP,HDOP,VDOP*checksum
    
    // This function would parse GSA sentences for more detailed satellite info
    // For now, we'll just extract HDOP as a simple implementation
    
    // Split the sentence into fields
    int commaIndex = 0;
    int nextCommaIndex = 0;
    
    // Skip to the HDOP field (15th field)
    for (int i = 0; i < 15; i++) {
        commaIndex = sentence.indexOf(',', commaIndex + 1);
        if (commaIndex == -1) return; // Not enough fields
    }
    
    nextCommaIndex = sentence.indexOf(',', commaIndex + 1);
    if (nextCommaIndex == -1) {
        // Last field might end with * instead of ,
        nextCommaIndex = sentence.indexOf('*', commaIndex + 1);
    }
    
    if (nextCommaIndex > commaIndex + 1) {
        String hdopStr = sentence.substring(commaIndex + 1, nextCommaIndex);
        if (hdopStr.length() > 0) {
            hdop = hdopStr.toFloat();
            validHDOP = true;
        }
    }
}

float GPSParser::convertToDecimalDegrees(String pos, char dir) {
    // NMEA format: DDMM.MMMM
    // Need to convert to decimal degrees: DD.DDDDDD
    
    float value = 0.0;
    
    // Find the decimal point
    int decimalPoint = pos.indexOf('.');
    if (decimalPoint == -1) return 0.0;
    
    // Extract degrees (characters before decimal point minus 2)
    int degrees = pos.substring(0, decimalPoint - 2).toInt();
    
    // Extract minutes (remaining characters)
    float minutes = pos.substring(decimalPoint - 2).toFloat();
    
    // Convert to decimal degrees
    value = degrees + (minutes / 60.0);
    
    // Apply direction
    if (dir == 'S' || dir == 'W') {
        value = -value;
    }
    
    return value;
}

bool GPSParser::isNewDataAvailable() {
    return newData;
}

void GPSParser::clearNewDataFlag() {
    newData = false;
}

float GPSParser::getLatitude() {
    return latitude;
}

float GPSParser::getLongitude() {
    return longitude;
}

String GPSParser::getPositionString() {
    if (!validPosition) {
        return "No Fix";
    }
    
    char buffer[30];
    char latDir = (latitude >= 0) ? 'N' : 'S';
    char lonDir = (longitude >= 0) ? 'E' : 'W';
    float absLat = abs(latitude);
    float absLon = abs(longitude);
    
    sprintf(buffer, "%02d°%05.2f'%c %03d°%05.2f'%c", 
            (int)absLat, (absLat - (int)absLat) * 60, latDir,
            (int)absLon, (absLon - (int)absLon) * 60, lonDir);
    
    return String(buffer);
}

float GPSParser::getSpeed() {
    return speed;
}

float GPSParser::getCourse() {
    return course;
}

float GPSParser::getAltitude() {
    return altitude;
}

bool GPSParser::hasAltitude() {
    return validAltitude;
}

String GPSParser::getTimeString() {
    // Format: HHMMSS.SSS
    if (time.length() < 6) {
        return "00:00:00";
    }
    
    int hour = time.substring(0, 2).toInt();
    int minute = time.substring(2, 4).toInt();
    int second = time.substring(4, 6).toInt();
    
    char buffer[9];
    sprintf(buffer, "%02d:%02d:%02d", hour, minute, second);
    return String(buffer);
}

String GPSParser::getDateString() {
    // Format: DDMMYY
    if (date.length() < 6) {
        return "01/01/00";
    }
    
    int day = date.substring(0, 2).toInt();
    int month = date.substring(2, 4).toInt();
    int year = date.substring(4, 6).toInt();
    
    char buffer[9];
    sprintf(buffer, "%02d/%02d/%02d", day, month, year);
    return String(buffer);
}

int GPSParser::getSatellites() {
    return satellites;
}

float GPSParser::getHDOP() {
    return hdop;
}

bool GPSParser::hasHDOP() {
    return validHDOP;
}

bool GPSParser::hasValidPosition() {
    return validPosition && satellites >= 4;
}

bool GPSParser::hasValidFix() {
    return validPosition;
}

int GPSParser::getFixQuality() {
    if(!validPosition) return 0;
    if(satellites >= 7) return 2; // DGPS fix
    if(satellites >= 4) return 1; // GPS fix
    return 0; // No fix
}

int GPSParser::getSatelliteInfo(SatelliteInfo* satArray, int maxCount) {
    // Placeholder implementation - would parse GSV sentences for detailed sat info
    // For now just return 0 satellites found
    return 0;
}

float GPSParser::getVDOP() {
    // Placeholder - would parse GSA sentence for VDOP
    return 0.0;
}

float GPSParser::getPDOP() {
    // Placeholder - would parse GSA sentence for PDOP
    return 0.0;
}

float GPSParser::getGeoidSeparation() {
    // Placeholder - would parse GGA sentence for geoid separation
    return 0.0;
}

String GPSParser::getFixTypeString() {
    switch(getFixQuality()) {
        case 0: return "No Fix";
        case 1: return "GPS Fix";
        case 2: return "DGPS Fix";
        default: return "Unknown";
    }
}
