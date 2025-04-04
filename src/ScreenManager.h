#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include "GPSParser.h"
#include "TCPLogger.h"

// Screen IDs
enum ScreenID {
  SCREEN_VALUES = 0,
  SCREEN_SATELLITES,
  SCREEN_TRACK,
  SCREEN_WAYPOINTS,
  SCREEN_COMPASS,
  SCREEN_SYSTEM,  // System info screen
  SCREEN_COUNT    // Always keep this as the last item
};

// Tab bar configuration
#define TAB_BAR_HEIGHT 40
#define TAB_BUTTON_WIDTH 53  // Adjusted for 6 tabs (320/6)
#define TAB_BUTTON_HEIGHT TAB_BAR_HEIGHT
#define TAB_TEXT_SIZE 1

// Screen area configuration
#define CONTENT_AREA_HEIGHT (240 - TAB_BAR_HEIGHT)  // Screen height minus tab bar height

// Scrolling configuration
#define SCROLL_BAR_WIDTH 10
#define SCROLL_STEP 20

class ScreenManager {
private:
  TFT_eSPI* tft;
  GPSParser* gpsParser;
  TCPLogger* logger;
  String* hostname;
  ScreenID currentScreen;
  
  // Scrolling variables
  int systemScrollOffset = 0;
  int systemMaxScrollOffset = 0;
  
  // Tab bar labels
  const char* tabLabels[SCREEN_COUNT] = {
    "Values",
    "Sats",
    "Track",
    "Waypts",
    "Compass",
    "System"  // System tab
  };
  
  void drawTabBar();
  void drawScreen();
  void drawScrollBar(int offset, int maxOffset, int contentHeight);
  
  // Individual screen drawing functions
  void drawValuesScreen();
  void drawSatellitesScreen();
  void drawTrackScreen();
  void drawWaypointsScreen();
  void drawCompassScreen();
  void drawSystemScreen();
  
public:
  // Updated constructor to include logger and hostname
  ScreenManager(TFT_eSPI* tft, GPSParser* gpsParser, TCPLogger* logger, String* hostname);
  
  void begin();
  void update();
  bool handleTouch(int x, int y);
  void setScreen(ScreenID screen);
  ScreenID getCurrentScreen() { return currentScreen; }
  
  // Scroll methods
  void scrollUp();
  void scrollDown();
};

#endif // SCREEN_MANAGER_H