#ifndef GPS_PARSER_H
#define GPS_PARSER_H

#include <Arduino.h>

class GPSParser {
private:
    // Position data
    float latitude = 0.0;
    float longitude = 0.0;
    char latDir = 'N';
    char lonDir = 'E';
    
    // Navigation data
    float speed = 0.0;
    float course = 0.0;
    float altitude = 0.0;
    
    // Time and date
    String time = "000000.000";
    String date = "010100";
    
    // Satellite data
    int satellites = 0;
    float hdop = 99.99;  // Horizontal dilution of precision
    
    // Status flags
    bool newData = false;
    bool validPosition = false;
    bool validAltitude = false;
    bool validHDOP = false;
    
    // NMEA sentence parsing
    String currentSentence = "";
    bool parsingGGA = false;
    bool parsingRMC = false;
    bool parsingGSA = false;
    
    // Helper methods
    void parseGGA(String sentence);
    void parseRMC(String sentence);
    void parseGSA(String sentence);
    float convertToDecimalDegrees(String pos, char dir);

public:
    GPSParser();
    
    void processGPSData(char c);
    bool isNewDataAvailable();
    void clearNewDataFlag();
    
    // Position getters
    float getLatitude();
    float getLongitude();
    String getPositionString();
    
    // Navigation getters
    float getSpeed();
    float getCourse();
    float getAltitude();
    bool hasAltitude();
    
    // Time and date getters
    String getTimeString();
    String getDateString();
    
    // Satellite data getters
    int getSatellites();
    float getHDOP();
    bool hasHDOP();
    
    // Status getters
    bool hasValidPosition();
    bool hasValidFix();
    int getFixQuality();  // 0=no fix, 1=GPS fix, 2=DGPS fix
    
    // For satellite view screen
    struct SatelliteInfo {
        int id;
        int elevation;  // 0-90 degrees
        int azimuth;    // 0-359 degrees
        int snr;        // Signal-to-noise ratio (0-99 dB)
        bool used;      // Whether satellite is used in position fix
    };
    
    // Return array of visible satellites (up to maxCount)
    int getSatelliteInfo(SatelliteInfo* satArray, int maxCount);

    // Additional GPS data getters
    float getVDOP();
    float getPDOP();
    float getGeoidSeparation();
    String getFixTypeString();
};

#endif // GPS_PARSER_H
