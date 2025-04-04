// Kafkar.com

#include <Arduino.h>
#include <HardwareSerial.h>
#include <SPI.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <config.h> // Include the config file for WiFi credentials

// include the installed "TFT_eSPI" library by Bodmer to interface with the TFT Display - https://github.com/Bodmer/TFT_eSPI
#include <TFT_eSPI.h>

// include the installed the "XPT2046_Touchscreen" library by Paul Stoffregen to use the Touchscreen - https://github.com/PaulStoffregen/XPT2046_Touchscreen
#include <XPT2046_Touchscreen.h>

// Include our GPS parser
#include "GPSParser.h"


// WiFi credentials (will be loaded from config)
String ssid;
String password;
String hostname;

// Create a instance of the TFT_eSPI class
TFT_eSPI tft = TFT_eSPI();

// Set the pius of the xpt2046 touchscreen
#define XPT2046_IRQ 36  // T_IRQ
#define XPT2046_MOSI 32 // T_DIN
#define XPT2046_MISO 39 // T_OUT
#define XPT2046_CLK 25  // T_CLK
#define XPT2046_CS 33   // T_CS

// Create a instance of the SPIClass and XPT2046_Touchscreen classes
SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_SIZE 2

// Global Variables
int posX;     // x position of the touch
int posY;     // y position of the touch
int pressure; // pressure of the touch

// Set X and Y coordinates for center of display
int centerX;
int centerY;

// Set the RX and TX pins for the GPS module
#define RXD2 22
#define TXD2 27
// Set the baud rate for the GPS module
#define GPS_BAUD 9600
// Create a instance of the HardwareSerial class
HardwareSerial gpsSerial(2);

// Create an instance of our GPS parser
GPSParser gpsParser;

// Display update timer
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 1000; // Update display every second

// OTA status flag
bool otaInProgress = false;

// Function to load configuration
bool loadConfig() {
  DynamicJsonDocument doc(CONFIG_JSON_BUFFER_SIZE);

  DeserializationError error = deserializeJson(doc, CONFIG_JSON);

  if (error) {
    Serial.print("Failed to parse config JSON: ");
    Serial.println(error.c_str());
    return false;
  }

  // Extract WiFi settings
  if (doc.containsKey("wifi")) {
    JsonObject wifi = doc["wifi"];
    ssid = wifi["ssid"].as<String>();
    password = wifi["password"].as<String>();
    hostname = wifi["hostname"].as<String>();
  } else {
    Serial.println("WiFi configuration not found in config file");
    return false;
  }

  return true;
}

void logTouchData(int posX, int posY, int pressure)
{
  Serial.print("X = ");
  Serial.print(posX);
  Serial.print(" | Y = ");
  Serial.print(posY);
  Serial.print(" | Pressure = ");
  Serial.print(pressure);
  Serial.println();
}

void updateGPSDisplay()
{
  // Clear the data area
  tft.fillRect(0, 60, SCREEN_WIDTH, 130, TFT_BLACK);
  
  // Display GPS data
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("Position: " + gpsParser.getPositionString(), 10, 60);
  
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("Speed: " + String(gpsParser.getSpeed()) + " knots", 10, 80);
  tft.drawString("Course: " + String(gpsParser.getCourse()) + " deg", 10, 100);
  
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Time: " + gpsParser.getTimeString(), 10, 120);
  tft.drawString("Date: " + gpsParser.getDateString(), 10, 140);
  
  tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
  tft.drawString("Satellites: " + String(gpsParser.getSatellites()), 10, 160);
}

void setupOTA() {
  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Display WiFi info on screen
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("WiFi: " + String(ssid), 10, 220);
  tft.drawString("IP: " + WiFi.localIP().toString(), 180, 220);

  // Set up mDNS responder
  if (!MDNS.begin(hostname)) {
    Serial.println("Error setting up MDNS responder!");
  } else {
    Serial.println("mDNS responder started");
    Serial.print("Device name: ");
    Serial.println(hostname);
  }

  // Configure OTA
  ArduinoOTA.setHostname(hostname.c_str());
  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {
      type = "filesystem";
    }
    
    // Set flag to prevent other operations during update
    otaInProgress = true;
    
    // Clear screen and show update message
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawCentreString("OTA Update Starting", centerX, centerY - 20, FONT_SIZE);
    tft.drawCentreString("Type: " + type, centerX, centerY, FONT_SIZE);
    
    Serial.println("Start updating " + type);
  });
  
  ArduinoOTA.onEnd([]() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawCentreString("Update Complete!", centerX, centerY, FONT_SIZE);
    tft.drawCentreString("Rebooting...", centerX, centerY + 30, FONT_SIZE);
    Serial.println("\nEnd");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    unsigned int percentage = (progress / (total / 100));
    
    // Update progress bar
    tft.fillRect(centerX - 100, centerY + 40, 200, 20, TFT_BLACK);
    tft.drawRect(centerX - 100, centerY + 40, 200, 20, TFT_WHITE);
    tft.fillRect(centerX - 98, centerY + 42, percentage * 196 / 100, 16, TFT_BLUE);
    
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawCentreString(String(percentage) + "%", centerX, centerY + 70, FONT_SIZE);
    
    Serial.printf("Progress: %u%%\r", percentage);
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    String errorMsg;
    
    if (error == OTA_AUTH_ERROR) {
      errorMsg = "Auth Failed";
    } else if (error == OTA_BEGIN_ERROR) {
      errorMsg = "Begin Failed";
    } else if (error == OTA_CONNECT_ERROR) {
      errorMsg = "Connect Failed";
    } else if (error == OTA_RECEIVE_ERROR) {
      errorMsg = "Receive Failed";
    } else if (error == OTA_END_ERROR) {
      errorMsg = "End Failed";
    }
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawCentreString("Update Error!", centerX, centerY - 20, FONT_SIZE);
    tft.drawCentreString(errorMsg, centerX, centerY + 10, FONT_SIZE);
    
    Serial.println(errorMsg);
    
    // Reset flag after error
    otaInProgress = false;
    
    // Restart the display after 3 seconds
    delay(3000);
    ESP.restart();
  });
  
  // Start OTA service
  ArduinoOTA.begin();
  Serial.println("OTA ready");
}

void setup()
{
    Serial.begin(115200);
  
    // Load configuration
    if (!loadConfig()) {
      Serial.println("Failed to load configuration. Using default values.");
      ssid = "DEFAULT_SSID";
      password = "DEFAULT_PASSWORD";
      hostname = "GPS-ESP32";
    }

  // Start the touchscreen component and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);

  // Set the Touchscreen rotation in landscape mode
  touchscreen.setRotation(1);

  // Start the GPS module
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);

  // Start the tft display
  tft.init();

  // Set the TFT display rotation in landscape mode
  tft.setRotation(1);

  // Clear the screen and display a message
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // Set X and Y coordinates for center of display
  centerX = SCREEN_WIDTH / 2;
  centerY = SCREEN_HEIGHT / 2;

  tft.drawCentreString("Hello, Kafkar.com!", centerX, 30, FONT_SIZE);
  tft.drawCentreString("Marin GPS", centerX, 200, FONT_SIZE);
  
  // Setup OTA after display is initialized
  setupOTA();
}

void loop()
{
  // Handle OTA updates
  ArduinoOTA.handle();
  
  // Skip other operations if OTA is in progress
  if (otaInProgress) {
    return;
  }

  // Read GPS data from Serial2 (UART2)
  while (gpsSerial.available() > 0) {
    // get the byte data from the GPS
    char gpsData = gpsSerial.read();
    
    // Process the GPS data with our parser
    gpsParser.processGPSData(gpsData);
    
    // Echo to serial monitor for debugging
    Serial.print(gpsData);
  }

  // Update display if we have new data and enough time has passed
  if (gpsParser.isNewDataAvailable() && (millis() - lastDisplayUpdate > DISPLAY_UPDATE_INTERVAL)) {
    updateGPSDisplay();
    lastDisplayUpdate = millis();
  }

  // Checks if Touchscreen is touched
  if (touchscreen.tirqTouched() && touchscreen.touched())
  {
    // Get Touchscreen points
    TS_Point p = touchscreen.getPoint();
    // Calibrate Touchscreen points with map function to the correct width and height
    posX = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    posY = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    pressure = p.z;

    logTouchData(posX, posY, pressure);

    delay(100);
  }
}
