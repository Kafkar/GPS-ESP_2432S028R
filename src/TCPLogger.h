#ifndef TCP_LOGGER_H
#define TCP_LOGGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class TCPLogger {
private:
    String serverAddress;
    uint16_t serverPort;
    String deviceName;
    bool connected;
    unsigned long lastReconnectAttempt;
    const unsigned long RECONNECT_INTERVAL = 5000; // 5 seconds between reconnection attempts
    
    // Fixed API endpoints
    const String LOG_ENDPOINT = "/api/log";
    const String NMEA_ENDPOINT = "/api/nmea";
    
    bool ensureConnected();

public:
    // Constructor now accepts configuration parameters directly
    TCPLogger(const String& server, uint16_t port, const String& device);
    ~TCPLogger();

    bool begin();
    bool log(const String& message, const String& level = "INFO");
    bool logInfo(const String& message);
    bool logWarning(const String& message);
    bool logError(const String& message);
    bool logDebug(const String& message);
    
    // New method to send raw NMEA data
    bool sendRawNMEA(const String& nmeaData);
};

#endif // TCP_LOGGER_H