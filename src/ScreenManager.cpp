#include "ScreenManager.h"

// Updated constructor
ScreenManager::ScreenManager(TFT_eSPI* tft, GPSParser* gpsParser, TCPLogger* logger, String* hostname) {
  this->tft = tft;
  this->gpsParser = gpsParser;
  this->logger = logger;
  this->hostname = hostname;
  this->currentScreen = SCREEN_VALUES;
  this->systemScrollOffset = 0;
}

void ScreenManager::begin() {
  // Initial draw
  drawTabBar();
  drawScreen();
}

void ScreenManager::update() {
  // Update the current screen with new data
  drawScreen();
}

bool ScreenManager::handleTouch(int x, int y) {
  // Check if touch is in the tab bar area
  if (y >= tft->height() - TAB_BAR_HEIGHT) {
    // Calculate which tab was touched
    int tabIndex = x / TAB_BUTTON_WIDTH;
    
    // Make sure the tab index is valid
    if (tabIndex >= 0 && tabIndex < SCREEN_COUNT) {
      // Only change screen if it's different
      if (tabIndex != currentScreen) {
        setScreen((ScreenID)tabIndex);
        return true;
      }
    }
    return true; // Touch was in tab area
  }
  
  // Handle scrolling for system screen
  if (currentScreen == SCREEN_SYSTEM) {
    // Check if touch is on the scroll bar area
    if (x > tft->width() - SCROLL_BAR_WIDTH) {
      // Calculate new scroll position based on touch point
      float touchPercent = (float)y / (float)(tft->height() - TAB_BAR_HEIGHT);
      systemScrollOffset = touchPercent * systemMaxScrollOffset;
      
      // Constrain scroll offset
      if (systemScrollOffset < 0) systemScrollOffset = 0;
      if (systemScrollOffset > systemMaxScrollOffset) systemScrollOffset = systemMaxScrollOffset;
      
      drawSystemScreen();
      return true;
    }
    // Check for scroll gestures in content area
    else {
      // Simple implementation: top third scrolls up, bottom third scrolls down
      if (y < (tft->height() - TAB_BAR_HEIGHT) / 3) {
        scrollUp();
        return true;
      } 
      else if (y > (tft->height() - TAB_BAR_HEIGHT) * 2 / 3) {
        scrollDown();
        return true;
      }
    }
  }
  
  return false;
}

void ScreenManager::scrollUp() {
  if (currentScreen == SCREEN_SYSTEM && systemScrollOffset > 0) {
    systemScrollOffset -= SCROLL_STEP;
    if (systemScrollOffset < 0) systemScrollOffset = 0;
    drawSystemScreen();
  }
}

void ScreenManager::scrollDown() {
  if (currentScreen == SCREEN_SYSTEM && systemScrollOffset < systemMaxScrollOffset) {
    systemScrollOffset += SCROLL_STEP;
    if (systemScrollOffset > systemMaxScrollOffset) systemScrollOffset = systemMaxScrollOffset;
    drawSystemScreen();
  }
}

void ScreenManager::setScreen(ScreenID screen) {
  if (screen != currentScreen && screen < SCREEN_COUNT) {
    currentScreen = screen;
    
    // Reset scroll position when changing screens
    if (screen == SCREEN_SYSTEM) {
      systemScrollOffset = 0;
    }
    
    // Clear the main screen area (excluding tab bar)
    tft->fillRect(0, 0, tft->width(), tft->height() - TAB_BAR_HEIGHT, TFT_BLACK);
    
    // Redraw the tab bar to show the new active tab
    drawTabBar();
    
    // Draw the new screen
    drawScreen();
  }
}

void ScreenManager::drawTabBar() {
  // Draw the tab bar background
  tft->fillRect(0, tft->height() - TAB_BAR_HEIGHT, tft->width(), TAB_BAR_HEIGHT, TFT_DARKGREY);
  
  // Draw tab separators
  for (int i = 1; i < SCREEN_COUNT; i++) {
    int x = i * TAB_BUTTON_WIDTH;
    tft->drawLine(x, tft->height() - TAB_BAR_HEIGHT, x, tft->height(), TFT_BLACK);
  }
  
  // Draw each tab
  for (int i = 0; i < SCREEN_COUNT; i++) {
    int x = i * TAB_BUTTON_WIDTH;
    int y = tft->height() - TAB_BAR_HEIGHT;
    
    // Highlight the active tab
    if (i == currentScreen) {
      tft->fillRect(x, y, TAB_BUTTON_WIDTH, TAB_BUTTON_HEIGHT, TFT_NAVY);
    }
    
    // Draw the tab label - using smaller font size
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(1); // Smaller text size for tabs
    
    // Center the text in the tab
    int textWidth = tft->textWidth(tabLabels[i]);
    int textX = x + (TAB_BUTTON_WIDTH - textWidth) / 2;
    int textY = y + (TAB_BUTTON_HEIGHT - 8) / 2; // 8 is approx. text height for size 1
    
    tft->drawString(tabLabels[i], textX, textY);
  }
}

void ScreenManager::drawScrollBar(int offset, int maxOffset, int contentHeight) {
  int scrollBarHeight = CONTENT_AREA_HEIGHT; // Only use content area height
  int scrollBarX = tft->width() - SCROLL_BAR_WIDTH;
  
  // Draw scroll bar background
  tft->fillRect(scrollBarX, 0, SCROLL_BAR_WIDTH, scrollBarHeight, TFT_DARKGREY);
  
  // Calculate thumb size and position
  float viewableRatio = (float)CONTENT_AREA_HEIGHT / (float)contentHeight;
  int thumbHeight = max(20, (int)(scrollBarHeight * viewableRatio));
  
  // Calculate thumb position
  float scrollRatio = (float)offset / (float)maxOffset;
  int thumbY = scrollRatio * (scrollBarHeight - thumbHeight);
  
  // Draw thumb
  tft->fillRect(scrollBarX, thumbY, SCROLL_BAR_WIDTH, thumbHeight, TFT_WHITE);
}

void ScreenManager::drawScreen() {
  // Call the appropriate drawing function based on the current screen
  switch (currentScreen) {
    case SCREEN_VALUES:
      drawValuesScreen();
      break;
    case SCREEN_SATELLITES:
      drawSatellitesScreen();
      break;
    case SCREEN_TRACK:
      drawTrackScreen();
      break;
    case SCREEN_WAYPOINTS:
      drawWaypointsScreen();
      break;
    case SCREEN_COMPASS:
      drawCompassScreen();
      break;
    case SCREEN_SYSTEM:
      drawSystemScreen();
      break;
    default:
      break;
  }
}

void ScreenManager::drawValuesScreen() {
  // Clear the data area (excluding tab bar)
  tft->fillRect(0, 0, tft->width(), tft->height() - TAB_BAR_HEIGHT, TFT_BLACK);
  
  // Draw title
  tft->setTextSize(1);
  tft->setTextColor(TFT_WHITE);
  tft->drawString("GPS Values", 10, 10);
  
  // Display GPS data with smaller font
  int lineHeight = 16; // Reduced line height for smaller font
  int startY = 30;
  
  // Position data
  tft->setTextColor(TFT_GREEN);
  tft->drawString("Position:", 10, startY);
  tft->drawString(gpsParser->getPositionString(), 80, startY);
  
  tft->drawString("Latitude:", 10, startY + lineHeight);
  tft->drawString(String(gpsParser->getLatitude(), 6) + "°", 80, startY + lineHeight);
  
  tft->drawString("Longitude:", 10, startY + lineHeight*2);
  tft->drawString(String(gpsParser->getLongitude(), 6) + "°", 80, startY + lineHeight*2);
  
  // Navigation data
  tft->setTextColor(TFT_YELLOW);
  tft->drawString("Speed:", 10, startY + lineHeight*3);
  tft->drawString(String(gpsParser->getSpeed()) + " knots", 80, startY + lineHeight*3);
  
  tft->drawString("Course:", 10, startY + lineHeight*4);
  tft->drawString(String(gpsParser->getCourse()) + "°", 80, startY + lineHeight*4);
  
  // Time data
  tft->setTextColor(TFT_CYAN);
  tft->drawString("UTC Time:", 10, startY + lineHeight*5);
  tft->drawString(gpsParser->getTimeString(), 80, startY + lineHeight*5);
  
  tft->drawString("Date:", 10, startY + lineHeight*6);
  tft->drawString(gpsParser->getDateString(), 80, startY + lineHeight*6);
  
  // Satellite data
  tft->setTextColor(TFT_MAGENTA);
  tft->drawString("Satellites:", 10, startY + lineHeight*7);
  tft->drawString(String(gpsParser->getSatellites()), 80, startY + lineHeight*7);
  
  // Fix data
  tft->setTextColor(TFT_WHITE);
  tft->drawString("Fix Quality:", 10, startY + lineHeight*8);
  
  // Display fix quality based on satellites
  String fixQuality;
  int satellites = gpsParser->getSatellites();
  if (satellites == 0) {
    fixQuality = "No Fix";
    tft->setTextColor(TFT_RED);
  } else if (satellites < 4) {
    fixQuality = "Poor";
    tft->setTextColor(TFT_ORANGE);
  } else if (satellites < 7) {
    fixQuality = "Good";
    tft->setTextColor(TFT_YELLOW);
  } else {
    fixQuality = "Excellent";
    tft->setTextColor(TFT_GREEN);
  }
  tft->drawString(fixQuality, 80, startY + lineHeight*8);
  
  // Add altitude if available
  tft->setTextColor(TFT_WHITE);
  tft->drawString("Altitude:", 10, startY + lineHeight*9);
  // Assuming GPSParser has getAltitude method, otherwise you'll need to add it
  if (gpsParser->hasAltitude()) {
    tft->drawString(String(gpsParser->getAltitude()) + " m", 80, startY + lineHeight*9);
  } else {
    tft->setTextColor(TFT_DARKGREY);
    tft->drawString("N/A", 80, startY + lineHeight*9);
  }
  
  // Add HDOP (Horizontal Dilution of Precision) if available
  tft->setTextColor(TFT_WHITE);
  tft->drawString("HDOP:", 10, startY + lineHeight*10);
  // Assuming GPSParser has getHDOP method, otherwise you'll need to add it
  if (gpsParser->hasHDOP()) {
    tft->drawString(String(gpsParser->getHDOP()), 80, startY + lineHeight*10);
  } else {
    tft->setTextColor(TFT_DARKGREY);
    tft->drawString("N/A", 80, startY + lineHeight*10);
  }
}

void ScreenManager::drawSatellitesScreen() {
  // Clear the data area (excluding tab bar)
  tft->fillRect(0, 0, tft->width(), tft->height() - TAB_BAR_HEIGHT, TFT_BLACK);
  
  // Draw title
  tft->setTextSize(1);
  tft->setTextColor(TFT_WHITE);
  tft->drawString("Satellites", 10, 10);
  
  // Display satellite information
  tft->setTextColor(TFT_MAGENTA);
  tft->drawString("Satellites in view: " + String(gpsParser->getSatellites()), 10, 30);
  
  // Draw a satellite view diagram
  int centerX = 160;
  int centerY = 100;
  int outerRadius = 80;
  int midRadius = 60;
  int innerRadius = 40;
  
  // Draw concentric circles representing elevation
  tft->drawCircle(centerX, centerY, outerRadius, TFT_DARKGREY);
  tft->drawCircle(centerX, centerY, midRadius, TFT_DARKGREY);
  tft->drawCircle(centerX, centerY, innerRadius, TFT_DARKGREY);
  
  // Draw compass points on the satellite view
  tft->setTextColor(TFT_DARKGREY);
  tft->drawString("N", centerX - 3, centerY - outerRadius - 10);
  tft->drawString("E", centerX + outerRadius + 5, centerY - 3);
  tft->drawString("S", centerX - 3, centerY + outerRadius + 5);
  tft->drawString("W", centerX - outerRadius - 10, centerY - 3);
  
  // Draw crosshairs
  tft->drawLine(centerX - outerRadius, centerY, centerX + outerRadius, centerY, TFT_DARKGREY);
  tft->drawLine(centerX, centerY - outerRadius, centerX, centerY + outerRadius, TFT_DARKGREY);
  
  // Draw satellites
  // In a real implementation, you would get actual satellite positions from the GPS
  // For now, we'll simulate satellites with random positions
  int numSatellites = gpsParser->getSatellites();
  
  for (int i = 0; i < numSatellites; i++) {
    // In a real implementation, you would get actual azimuth and elevation
    // For now, we'll use random positions
    float azimuth = random(0, 360) * PI / 180.0;
    float elevation = random(0, 90); // 0-90 degrees
    
    // Convert elevation to radius (90° = center, 0° = edge)
    float radius = outerRadius * (1.0 - elevation / 90.0);
    
    // Calculate x,y position
    int x = centerX + cos(azimuth) * radius;
    int y = centerY - sin(azimuth) * radius;
    
    // Determine color based on simulated signal strength
    uint16_t color;
    int signalStrength = random(0, 100);
    if (signalStrength > 70) {
      color = TFT_GREEN;
    } else if (signalStrength > 40) {
      color = TFT_YELLOW;
    } else {
      color = TFT_RED;
    }
    
    // Draw the satellite
    tft->fillCircle(x, y, 4, color);
    
    // Draw satellite ID
    tft->setTextColor(TFT_WHITE);
    tft->drawString(String(i+1), x-3, y-3);
  }
  
  // Add satellite signal strength bars at the bottom
  int barWidth = 15;
  int barSpacing = 5;
  int barMaxHeight = 50;
  int barY = 170;
  
  tft->setTextColor(TFT_WHITE);
  tft->drawString("Satellite Signal Strength", 10, barY - 15);
  
  // Only show bars for visible satellites
  int maxBars = min(numSatellites, (tft->width() - 20) / (barWidth + barSpacing));
  
  for (int i = 0; i < maxBars; i++) {
    int x = 10 + i * (barWidth + barSpacing);
    // In a real implementation, use actual signal strength
    int strength = random(30, 100);
    int barHeight = (strength * barMaxHeight) / 100;
    
    // Color based on strength
    uint16_t color;
    if (strength > 70) {
      color = TFT_GREEN;
    } else if (strength > 40) {
      color = TFT_YELLOW;
    } else {
      color = TFT_RED;
    }
    
    // Draw the bar
    tft->fillRect(x, barY + (barMaxHeight - barHeight), barWidth, barHeight, color);
    tft->drawRect(x, barY, barWidth, barMaxHeight, TFT_DARKGREY);
    
    // Draw the satellite number
    tft->setTextColor(TFT_WHITE);
    tft->drawString(String(i+1), x + 3, barY + barMaxHeight + 5);
  }
}

void ScreenManager::drawTrackScreen() {
  // Clear the data area (excluding tab bar)
  tft->fillRect(0, 0, tft->width(), tft->height() - TAB_BAR_HEIGHT, TFT_BLACK);
  
  // Draw title
  tft->setTextSize(1);
  tft->setTextColor(TFT_WHITE);
  tft->drawString("Track", 10, 10);
  
  // Draw a grid
  tft->setTextColor(TFT_DARKGREY);
  for (int i = 0; i < tft->width(); i += 40) {
    tft->drawLine(i, 40, i, tft->height() - TAB_BAR_HEIGHT, TFT_DARKGREY);
  }
  
  for (int i = 40; i < tft->height() - TAB_BAR_HEIGHT; i += 40) {
    tft->drawLine(0, i, tft->width(), i, TFT_DARKGREY);
  }
  
  // Draw current position
  tft->fillCircle(160, 100, 5, TFT_RED);
  
  // Display track information
  tft->setTextColor(TFT_YELLOW);
  tft->drawString("Current Position: " + gpsParser->getPositionString(), 10, 170);
  tft->drawString("Speed: " + String(gpsParser->getSpeed()) + " knots", 10, 185);
  tft->drawString("Course: " + String(gpsParser->getCourse()) + "°", 160, 185);
  
  // Add track statistics
  tft->setTextColor(TFT_CYAN);
  tft->drawString("Track Distance: 0.0 nm", 10, 200);
  tft->drawString("Elapsed Time: 00:00:00", 160, 200);
}

void ScreenManager::drawWaypointsScreen() {
  // Clear the data area (excluding tab bar)
  tft->fillRect(0, 0, tft->width(), tft->height() - TAB_BAR_HEIGHT, TFT_BLACK);
  
  // Draw title
  tft->setTextSize(1);
  tft->setTextColor(TFT_WHITE);
  tft->drawString("Waypoints", 10, 10);
  
  // Draw a table header for waypoints
  tft->drawLine(10, 30, tft->width() - 10, 30, TFT_DARKGREY);
  tft->drawLine(10, 45, tft->width() - 10, 45, TFT_DARKGREY);
  
  tft->setTextColor(TFT_CYAN);
  tft->drawString("Name", 15, 35);
  tft->drawString("Latitude", 80, 35);
  tft->drawString("Longitude", 160, 35);
  tft->drawString("Dist", 240, 35);
  
  // This is a placeholder - you would need to implement waypoint storage
  tft->setTextColor(TFT_SILVER);
  tft->drawString("No waypoints stored", 80, 100);
  
  // Draw buttons for waypoint management
  tft->fillRoundRect(10, 160, 140, 25, 5, TFT_DARKGREEN);
  tft->setTextColor(TFT_WHITE);
  tft->drawString("Add Current Position", 20, 167);
  
  tft->fillRoundRect(170, 160, 140, 25, 5, TFT_RED);
  tft->drawString("Clear Waypoints", 190, 167);
  
  // Add navigation button
  tft->fillRoundRect(90, 195, 140, 25, 5, TFT_NAVY);
  tft->drawString("Navigate to Selected", 100, 202);
}

void ScreenManager::drawCompassScreen() {
  // Clear the data area (excluding tab bar)
  tft->fillRect(0, 0, tft->width(), tft->height() - TAB_BAR_HEIGHT, TFT_BLACK);
  
  // Draw title
  tft->setTextSize(1);
  tft->setTextColor(TFT_WHITE);
  tft->drawString("Compass", 10, 10);
  
  // Draw compass
  int centerX = tft->width() / 2;
  int centerY = 110;
  int radius = 80;
  
  // Draw compass circle
  tft->drawCircle(centerX, centerY, radius, TFT_WHITE);
  tft->drawCircle(centerX, centerY, radius + 1, TFT_WHITE);
  
  // Draw cardinal points
  tft->setTextColor(TFT_RED);
  tft->drawString("N", centerX - 3, centerY - radius - 10);
  tft->setTextColor(TFT_WHITE);
  tft->drawString("E", centerX + radius + 5, centerY - 3);
  tft->drawString("S", centerX - 3, centerY + radius + 5);
  tft->drawString("W", centerX - radius - 15, centerY - 3);
  
  // Draw heading needle based on course
  float course = gpsParser->getCourse() * PI / 180.0; // Convert to radians
  int needleLength = radius - 10;
  
  int needleX = centerX + sin(course) * needleLength;
  int needleY = centerY - cos(course) * needleLength;
  
  tft->drawLine(centerX, centerY, needleX, needleY, TFT_RED);
  tft->fillTriangle(
    needleX, needleY,
    centerX + sin(course + 0.2) * (needleLength - 15), centerY - cos(course + 0.2) * (needleLength - 15),
    centerX + sin(course - 0.2) * (needleLength - 15), centerY - cos(course - 0.2) * (needleLength - 15),
    TFT_RED
  );
  
  // Display course information
  tft->setTextColor(TFT_YELLOW);
  tft->drawString("Course: " + String(gpsParser->getCourse()) + "°", 10, 180);
  tft->drawString("Speed: " + String(gpsParser->getSpeed()) + " knots", 160, 180);
}

void ScreenManager::drawSystemScreen() {
  // Clear the data area (excluding tab bar)
  tft->fillRect(0, 0, tft->width() - SCROLL_BAR_WIDTH, tft->height() - TAB_BAR_HEIGHT, TFT_BLACK);
  
  // Define content parameters
  int lineHeight = 16;
  int totalLines = 25; // Increased number of lines for more content
  int contentHeight = totalLines * lineHeight;
  
  // Calculate max scroll offset
  systemMaxScrollOffset = max(0, contentHeight - CONTENT_AREA_HEIGHT);
  
  // Draw title - fixed at top
  tft->setTextSize(1);
  tft->setTextColor(TFT_WHITE);
  tft->drawString("System Information", 10, 10 - systemScrollOffset);
  
  // Draw horizontal separator
  if (10 + lineHeight - systemScrollOffset >= 0 && 10 + lineHeight - systemScrollOffset < CONTENT_AREA_HEIGHT) {
    tft->drawLine(10, 25 - systemScrollOffset, tft->width() - SCROLL_BAR_WIDTH - 10, 25 - systemScrollOffset, TFT_DARKGREY);
  }
  
  int startY = 30;
  
  // Function to draw a line of text if it's in the visible area
  auto drawInfoLine = [this, startY, lineHeight](int lineNum, const String& label, const String& value, uint16_t labelColor, uint16_t valueColor) {
    int y = startY + lineNum * lineHeight - systemScrollOffset;
    
    // Only draw if line is visible
    if (y >= 0 && y < CONTENT_AREA_HEIGHT) {
      tft->setTextColor(labelColor);
      tft->drawString(label, 10, y);
      tft->setTextColor(valueColor);
      tft->drawString(value, 100, y);
    }
  };
  
  // WiFi Information
  drawInfoLine(0, "WiFi Status:", WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected", 
               TFT_CYAN, WiFi.status() == WL_CONNECTED ? TFT_GREEN : TFT_RED);
  
  drawInfoLine(1, "SSID:", WiFi.SSID(), TFT_CYAN, TFT_WHITE);
  drawInfoLine(2, "IP Address:", WiFi.localIP().toString(), TFT_CYAN, TFT_WHITE);
  drawInfoLine(3, "MAC Address:", WiFi.macAddress(), TFT_CYAN, TFT_WHITE);
  drawInfoLine(4, "Hostname:", *hostname, TFT_CYAN, TFT_WHITE);
  drawInfoLine(5, "Signal Strength:", String(WiFi.RSSI()) + " dBm", TFT_CYAN, TFT_WHITE);
  
  // Draw horizontal separator
  if (startY + 6 * lineHeight - systemScrollOffset >= 0 && 
      startY + 6 * lineHeight - systemScrollOffset < CONTENT_AREA_HEIGHT) {
    tft->drawLine(10, startY + 6 * lineHeight - systemScrollOffset, 
                 tft->width() - SCROLL_BAR_WIDTH - 10, 
                 startY + 6 * lineHeight - systemScrollOffset, TFT_DARKGREY);
  }
  
  // GPS Information
  drawInfoLine(7, "GPS Status:", gpsParser->getSatellites() > 0 ? "Active" : "No Fix", 
               TFT_GREEN, gpsParser->getSatellites() > 0 ? TFT_GREEN : TFT_RED);
  drawInfoLine(8, "Satellites:", String(gpsParser->getSatellites()), TFT_GREEN, TFT_WHITE);
  drawInfoLine(9, "Position:", gpsParser->getPositionString(), TFT_GREEN, TFT_WHITE);
  drawInfoLine(10, "Last Update:", gpsParser->getTimeString(), TFT_GREEN, TFT_WHITE);
  
  // Draw horizontal separator
  if (startY + 11 * lineHeight - systemScrollOffset >= 0 && 
      startY + 11 * lineHeight - systemScrollOffset < CONTENT_AREA_HEIGHT) {
    tft->drawLine(10, startY + 11 * lineHeight - systemScrollOffset, 
                 tft->width() - SCROLL_BAR_WIDTH - 10, 
                 startY + 11 * lineHeight - systemScrollOffset, TFT_DARKGREY);
  }
  
  // System Information
  // Format uptime as days, hours, minutes, seconds
  unsigned long uptime = millis() / 1000; // Convert to seconds
  int days = uptime / 86400;
  int hours = (uptime % 86400) / 3600;
  int minutes = (uptime % 3600) / 60;
  int seconds = uptime % 60;
  
  String uptimeStr = "";
  if (days > 0) uptimeStr += String(days) + "d ";
  uptimeStr += String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s";
  
  drawInfoLine(13, "System Uptime:", uptimeStr, TFT_YELLOW, TFT_WHITE);
  drawInfoLine(14, "Free Heap:", String(ESP.getFreeHeap() / 1024) + " KB", TFT_YELLOW, TFT_WHITE);
  drawInfoLine(15, "ESP32 Chip:", String(ESP.getChipModel()) + " Rev" + String(ESP.getChipRevision()), TFT_YELLOW, TFT_WHITE);
  drawInfoLine(16, "CPU Freq:", String(ESP.getCpuFreqMHz()) + " MHz", TFT_YELLOW, TFT_WHITE);
  drawInfoLine(17, "Flash Size:", String(ESP.getFlashChipSize() / (1024 * 1024)) + " MB", TFT_YELLOW, TFT_WHITE);
  
  // Draw horizontal separator
  if (startY + 18 * lineHeight - systemScrollOffset >= 0 && 
      startY + 18 * lineHeight - systemScrollOffset < CONTENT_AREA_HEIGHT) {
    tft->drawLine(10, startY + 18 * lineHeight - systemScrollOffset, 
                 tft->width() - SCROLL_BAR_WIDTH - 10, 
                 startY + 18 * lineHeight - systemScrollOffset, TFT_DARKGREY);
  }
  
  // GPS Information
  drawInfoLine(20, "GPS Satellites:", String(gpsParser->getSatellites()), TFT_CYAN, TFT_WHITE);
  drawInfoLine(21, "GPS Fix:", gpsParser->getFixTypeString(), TFT_CYAN, TFT_WHITE);
  drawInfoLine(22, "HDOP:", String(gpsParser->getHDOP()), TFT_CYAN, TFT_WHITE);
  drawInfoLine(23, "VDOP:", String(gpsParser->getVDOP()), TFT_CYAN, TFT_WHITE);
  drawInfoLine(24, "PDOP:", String(gpsParser->getPDOP()), TFT_CYAN, TFT_WHITE);
  drawInfoLine(25, "Altitude:", String(gpsParser->getAltitude()) + " m", TFT_CYAN, TFT_WHITE);
  drawInfoLine(26, "Geoid Separation:", String(gpsParser->getGeoidSeparation()) + " m", TFT_CYAN, TFT_WHITE);
  
  // Draw scroll bar
  drawScrollBar(systemScrollOffset, systemMaxScrollOffset, contentHeight);
}
