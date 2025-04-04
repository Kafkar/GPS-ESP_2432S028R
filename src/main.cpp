// Kafkar.com

#include <Arduino.h>
#include <HardwareSerial.h>

#include <SPI.h>


// include the installed "TFT_eSPI" library by Bodmer to interface with the TFT Display - https://github.com/Bodmer/TFT_eSPI
#include <TFT_eSPI.h>

// include the installed the "XPT2046_Touchscreen" library by Paul Stoffregen to use the Touchscreen - https://github.com/PaulStoffregen/XPT2046_Touchscreen
#include <XPT2046_Touchscreen.h>

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

void setup()
{
  Serial.begin(115200);

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

}

void loop()
{
  // Read GPS data from Serial2 (UART2)
  while (gpsSerial.available() > 0){
    // get the byte data from the GPS
    char gpsData = gpsSerial.read();
    Serial.print(gpsData);

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
