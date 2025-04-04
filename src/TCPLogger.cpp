#include "TCPLogger.h"

bool TCPLogger::ensureConnected() {
    // For HTTP, we just check if WiFi is connected
    if (WiFi.status() == WL_CONNECTED) {
        connected = true;
        return true;
    }
    
    connected = false;
    return false;
}

// Constructor now accepts configuration parameters directly
TCPLogger::TCPLogger(const String& server, uint16_t port, const String& device) : 
    serverAddress(server),
    serverPort(port),
    deviceName(device),
    connected(false), 
    lastReconnectAttempt(0) {
}

TCPLogger::~TCPLogger() {
    // Nothing to clean up for HTTP client
}

bool TCPLogger::begin() {
    return ensureConnected();
}

bool TCPLogger::log(const String& message, const String& level) {
    if (!ensureConnected()) {
        return false;
    }
    
    // Create a JSON log message
    DynamicJsonDocument logDoc(512);
    logDoc["device"] = deviceName;
    logDoc["timestamp"] = millis();
    logDoc["level"] = level;
    logDoc["message"] = message;
    
    String jsonString;
    serializeJson(logDoc, jsonString);
    
    // Create the full URL
    String url = "http://" + serverAddress + ":" + String(serverPort) + LOG_ENDPOINT;
    
    // Send HTTP POST request
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.POST(jsonString);
    
    if (httpResponseCode > 0) {
        http.end();
        return true;
    } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
        http.end();
        return false;
    }
}

bool TCPLogger::sendRawNMEA(const String& nmeaData) {
    if (!ensureConnected()) {
        return false;
    }
    
    // Create the full URL
    String url = "http://" + serverAddress + ":" + String(serverPort) + NMEA_ENDPOINT;
    
    // Send HTTP POST request with raw NMEA data
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "text/plain");
    
    int httpResponseCode = http.POST(nmeaData);
    
    if (httpResponseCode > 0) {
        http.end();
        return true;
    } else {
        Serial.print("NMEA Error code: ");
        Serial.println(httpResponseCode);
        http.end();
        return false;
    }
}

bool TCPLogger::logInfo(const String& message) {
    return log(message, "INFO");
}

bool TCPLogger::logWarning(const String& message) {
    return log(message, "WARNING");
}

bool TCPLogger::logError(const String& message) {
    return log(message, "ERROR");
}

bool TCPLogger::logDebug(const String& message) {
    return log(message, "DEBUG");
}