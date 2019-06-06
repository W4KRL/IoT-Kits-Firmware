// D1S-Display-IoT-man-config-ezTime.ino

// 06/05/2019 - uses ezTime and ThingSpeak libraries
//            - removed ArduinoJson, Time, TimeZone libs

/*_____________________________________________________________________________
   Copyright(c) 2018-2019 Karl W. Berger dba IoT Kits https://w4krl.com/iot-kits

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
   _____________________________________________________________________________
*/

// *******************************************************
// ******************* INCLUDES **************************
// *******************************************************

// **** Use ESP8266 Core version 2.5.2 ****
// **** Compiled with Arduino IDE 1.8.9 ****

// For WiFi connectivity
// See https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html
#include <ESP8266WiFi.h>                  // [builtin] 

// For ThingSpeak communications
// See https://github.com/mathworks/thingspeak-arduino
#include <ThingSpeak.h>                   // [manager] v1.5.0 

// For Wemos TFT 1.4 display shield
#include <Adafruit_GFX.h>                 // [manager] v1.5.3 Core graphics library
#include <Adafruit_ST7735.h>              // [manager] v1.3.4
#include <SPI.h>                          // [builtin]

// Time functions by Rop Gonggrijp
// See https://github.com/ropg/ezTime
#include <ezTime.h>                       // [manager] v0.7.10 NTP & timezone 

// Your ThingSpeak_config.h file must be located in the sketch folder
#include "ThingSpeak_config.h"

// *******************************************************
// ********** WIRING CONNECTIONS *************************
// *******************************************************
// Wemos TFT 1.4 shield connections
// TFT_CS      D4  GPIO  2
// TFT_DC      D3  GPIO  0
// TFT_RST     -1  use  -1 according to Wemos
// MOSI        D7  GPIO 13
// SCK         D5  GPIO 14
// TFT_LED     NC
// Push Button D1  GPIO  5

// *******************************************************
// ********* INSTANTIATE DEVICES *************************
// *******************************************************
// Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);
Adafruit_ST7735 tft = Adafruit_ST7735(2,  0, -1);
WiFiClient client;
Timezone myTZ;

// *******************************************************
// ******************* GLOBALS ***************************
// *******************************************************
const int pinButton = D1;                  // push button connection
long currentMillis = millis();             // for thingspeak timer
long startMillis = currentMillis - 40000;  // force TD read on first cycle

// structure to hold sensor measurements & calculations
struct {
  float slp;                  // sealevel Pressure (hPa)
  float tempC;                // temperature (°C)
  float tempF;                // temperature (°F)
  float humidity;             // relative humidity (%)
  float timeAwake;            // processor time awake
  long  lightLevel;           // light intensity (lux)
  float cellVoltage;          // volts
  int   dBm;                  // WiFi signal strength (dBm)
  long  entry_id = 0;         // entry id from ThingSpeak - not used
} iotData;

// *******************************************************
// *********** WEMOS TFT 1.4 SETTINGS ********************
// *******************************************************
// display parameters for Wemos TFT 1.4 shield
const int screenW = 128;
const int screenH = 128;
const int screenW2 = screenW / 2;    // half width
const int screenH2 = screenH / 2;    // half height

// analog clock dimensions
const int clockCenterX = screenW2;
// title at top of screen is 16 pixels high so screen available
// is screenH - 16. Must add 16 back to shift center to middle
// of screen available
const int clockCenterY = (( screenH - 16 ) / 2 ) + 16;
const int clockDialRadius = 50;      // outer clock dial

// Color definitions
// RGB565 format
// http://w8bh.net/pi/rgb565.py
const int BLACK      = 0x0000;
const int BROWN      = 0xA145;
const int RED        = 0xF800;
const int ORANGERED  = 0xFA20;
const int ORANGE     = 0xFD20;
const int YELLOW     = 0xFFE0;
const int LIGHTGREEN = 0x9772;
const int LIME       = 0x07E0;
const int CYAN       = 0x07FF;
const int BLUE       = 0x001F;
const int MAGENTA    = 0xF81F;
const int VIOLET     = 0xEC1D;
const int GRAY       = 0x8410;
const int WHITE      = 0xFFFF;
const int GOLD       = 0xFEA0;
const int SILVER     = 0xC618;

// *******************************************************
// ******************** SETUP ****************************
// *******************************************************
void setup() {
  Serial.begin( 115200);
  pinMode(pinButton, INPUT_PULLUP);      // use internal pullup resistor

  // Initializer for 1.4-in. TFT ST7735S chip, green tab
  tft.initR(INITR_144GREENTAB);

  Serial.println("IoT Kits splash");
  splashScreen();
  //  delay (5000);

  logonToRouter();                        // logon to WiFi

  tft.fillScreen( BLACK );                // clear screen
  tft.setCursor( 0, 0 );
  tft.setTextSize( 1 );
  tft.setTextColor( YELLOW );
  tft.println("Connected to:");
  tft.println( WiFi.localIP() );

  Serial.print("Local IP: ");             // show Wi-Fi IP
  Serial.println( WiFi.localIP() );

  waitForSync();                          // ezTime lib
  // if timezone has not been cached in EEPROM
  // or user is asking for a new timezone
  // set the timezone
  if ( !myTZ.setCache( 0 ) || myTZ.getOlson() != tzLocation ) {
    myTZ.setLocation( tzLocation );
  }

  ThingSpeak.begin(client);               // Initialize ThingSpeak

  tft.fillScreen(BLACK);                  // clear screen
} //setup()

// *******************************************************
// ******************** LOOP *****************************
// *******************************************************
void loop() {
  // reconnect if WiFi connection has been lost
  if (WiFi.status() != WL_CONNECTED) {
    logonToRouter();
  }

  // update frames when the second has changed
  // this is an ezTime library function
  if ( secondChanged() ) {
    frameUpdate();
  }

  // read ThingSpeak every 30 seconds
  currentMillis = millis();
  if (currentMillis - startMillis > 30000) {
    readTS();
    startMillis = currentMillis;
  }
} // loop()

// *******************************************************
// ************** Logon to your Wi-Fi router *************
// *******************************************************
void logonToRouter() {
  int count = 0;
  WiFi.begin( WIFI_SSID, WIFI_PASSWORD );
  while ( WiFi.status() != WL_CONNECTED ) {
    count++;
    if ( count > 100 ) {
      Serial.println("WiFi connection timeout. Rebooting.");
      ESP.reset();                    // reset ESP8266 if WiFi failed
    }
    delay( 100 );                     // ms delay between reports
    Serial.print(".");
  } // loop while not connected
  // WiFi is sucesfully connected
} // logonToRouter()

// *******************************************************
// **************** FRAME UPDATE *************************
// *******************************************************
void frameUpdate() {
  const int maxFrames = 4;              // number of display frames
  static boolean firstTime = true;      // update frame background first time only
  static int frame = second() % FRAME_INTERVAL;

  if ( second() % FRAME_INTERVAL == 0 ) {
    // erase screen each time a new frame is displayed
    tft.fillScreen(BLACK);
    firstTime = true;
    frame++;
    if ( frame > maxFrames - 1 ) {
      frame = 0;
    }
  }

  // choose frame to display
  switch ( frame ) {
    case 0:
      analogClockFrame( firstTime );
      firstTime = false;
      break;
    case 1:
      digitalClockFrame( firstTime );
      firstTime = false;
      break;
    case 2:
      iotWXFrame( firstTime );
      firstTime = false;
      break;
    case 3:
      iotTelemFrame( firstTime );
      firstTime = false;
      break;
  }
} // frameUpdate()

// *******************************************************
// **************** SPLASH SCREEN ************************
// *******************************************************
void splashScreen() {
  tft.fillScreen( BLUE );
  tft.setTextSize( 2 );
  tft.setTextColor( YELLOW );
  int topLine = 19;
  displayCenter( "D1S-IoT",  screenW2, topLine,      2 );
  displayCenter( "Remote",   screenW2, topLine + 20, 2 );
  displayCenter( "Display",  screenW2, topLine + 40, 2 );
  displayCenter( "IoT Kits", screenW2, topLine + 60, 2 );
  displayCenter( "by W4KRL", screenW2, topLine + 80, 2 );
  for (int i = 0; i < 4; i++) {
    tft.drawRoundRect(12 - 3 * i, 12 - 3 * i, screenW - 12, screenH - 12, 8, YELLOW);
  }
} // splashScreen()

// *******************************************************
// *************** DISPLAY FRAMES ************************
// *******************************************************

// *******************************************************
// ************* ANALOG CLOCK FRAME **********************
// *******************************************************
void analogClockFrame( boolean firstRender ) {
  // scale dimensions from dial radius
  int numeralR = clockDialRadius - 6 ;
  int outerTickR = numeralR - 6;
  int innerTickR = outerTickR - 6;
  int minHand = innerTickR;       // longest hand
  int secHand = innerTickR;       // use different color
  int hourHand = 2 * minHand / 3; // hour hand is 2/3 minute hand length
  int x2, x3, x4, y2, y3, y4;     // various coordinates

  // draw clock face first time only to speed up graphics
  if ( firstRender == true ) {
    // display time zone and static dial elements
    tft.drawRoundRect(0, 0, screenW, screenH, 8, ORANGERED);
    tft.setTextSize( 2 );
    tft.setCursor( 4, 4 );
    tft.setTextColor( WHITE, BLACK );
    tft.print( myTZ.getTimezoneName() );  // local time zone abbreviation
    tft.drawCircle( clockCenterX, clockCenterY, 2, YELLOW );  // hub
    tft.drawCircle( clockCenterX, clockCenterY, clockDialRadius, WHITE );  // dial edge
    tft.setTextSize( 1 );  // for dial numerals
    tft.setTextColor( LIGHTGREEN, BLACK );
    // add hour ticks & numerals
    for ( int numeral = 1; numeral < 13; numeral++ ) {
      // Begin at 30 degrees and stop at 360 degrees (noon)
      float angle = 30 * numeral;          // convert hour to angle in degrees
      angle = angle / 57.29577951; // Convert degrees to radians
      x2 = ( clockCenterX + ( sin( angle ) * outerTickR ));
      y2 = ( clockCenterY - ( cos( angle ) * outerTickR ));
      x3 = ( clockCenterX + ( sin( angle ) * innerTickR ));
      y3 = ( clockCenterY - ( cos( angle ) * innerTickR ));
      tft.drawLine( x2, y2, x3, y3, YELLOW );      // tick line
      // place numeral a little beyond tick line
      x4 = ( clockCenterX + ( sin(angle) * numeralR ));
      y4 = ( clockCenterY - ( cos(angle) * numeralR ));
      tft.setCursor( x4 - 2, y4 - 4 );  // minus third character width, half height
      if ( numeral == 12 ) tft.setCursor( x4 - 5, y4 - 4 );
      tft.print( numeral );
    }
  } // if firstRender

  // display second hand
  float angle = second() * 6;         // each second advances 6 degrees
  angle = angle  / 57.29577951;       // Convert degrees to radians
  static float oldSecond = angle;
  // erase previous second hand
  x3 = ( clockCenterX + ( sin( oldSecond ) * secHand ));
  y3 = ( clockCenterY - ( cos( oldSecond ) * secHand ));
  tft.drawLine( clockCenterX, clockCenterY, x3, y3, BLACK );
  oldSecond = angle;  // save current angle for erase next time
  // draw new second hand
  x3 = ( clockCenterX + ( sin( angle ) * secHand ));
  y3 = ( clockCenterY - ( cos( angle ) * secHand ));
  tft.drawLine( clockCenterX, clockCenterY, x3, y3, ORANGERED );

  // display minute hand
  angle = minute() * 6;        // each minute advances 6 degrees
  angle = angle / 57.29577951; // Convert degrees to radians
  static float oldMinute = angle;
  // erase old minute hand
  if (angle != oldMinute || firstRender) {
    x3 = ( clockCenterX + ( sin( oldMinute ) * minHand ));
    y3 = ( clockCenterY - ( cos( oldMinute ) * minHand ));
    tft.drawLine( clockCenterX, clockCenterY, x3, y3, BLACK );
    oldMinute = angle;
  }
  // draw new minute hand
  x3 = ( clockCenterX + ( sin(angle ) * minHand ));
  y3 = ( clockCenterY - ( cos(angle ) * minHand ));
  tft.drawLine( clockCenterX, clockCenterY, x3, y3, LIGHTGREEN );

  // display hour hand
  int dialHour = myTZ.hour();
  if (dialHour > 13 ) dialHour = dialHour - 12;
  // 30 degree increments + adjust for minutes
  angle = myTZ.hour() * 30 + int(( minute() / 12 ) * 6 );
  angle = ( angle / 57.29577951 ); //Convert degrees to radians
  static float oldHour = angle;
  if ( angle != oldHour || firstRender ) {
    x3 = ( clockCenterX + ( sin( oldHour ) * hourHand ));
    y3 = ( clockCenterY - ( cos( oldHour ) * hourHand ));
    tft.drawLine( clockCenterX, clockCenterY, x3, y3, BLACK );
    oldHour = angle;
  }
  // draw new hour hand
  x3 = ( clockCenterX + ( sin(angle ) * hourHand ));
  y3 = ( clockCenterY - ( cos(angle ) * hourHand ));
  tft.drawLine( clockCenterX, clockCenterY, x3, y3, YELLOW );
} // analogClockFrame()

// *******************************************************
// ************* DIGITAL CLOCK FRAME *********************
// *******************************************************
void digitalClockFrame( boolean firstRender ) {
  int topLine = 15;                // sets location of top line

  tft.setTextSize(2);
  if ( firstRender ) {
    // print labels
    tft.drawRoundRect( 0, 0, screenW, screenH, 8, WHITE );
    tft.setTextColor( ORANGERED, BLACK );
    displayCenter("UTC", screenW2, topLine, 2 );
    tft.setTextColor( LIGHTGREEN, BLACK );
    // local time zone
    displayCenter( myTZ.getTimezoneName(), screenW2, topLine + 60, 2 );
  }
  // display times
  tft.setTextColor( ORANGERED, BLACK );
  tft.setCursor( 17, topLine + 20 );
  // format definitions at
  tft.print( UTC.dateTime("H~:i~:s") );

  tft.setTextColor( LIGHTGREEN, BLACK );
  tft.setCursor( 17, topLine + 80 );
  tft.print( myTZ.dateTime("H~:i~:s") );
} // digitalClockFrame()

// *******************************************************
// *************** IOT WEATHER FRAME *********************
// *******************************************************
void iotWXFrame( boolean firstRender ) {
  static boolean beenDisplayed = false;
  // print static labels the first time
  if ( firstRender ) {
    int headerY = 40;
    int radius = 8;
    // top panel
    tft.fillRoundRect( 0, 0, screenW, 2 * radius, radius, YELLOW );
    tft.fillRect( 0, radius, screenW, headerY - radius, YELLOW );
    tft.fillRect( 0, headerY, screenW, screenH - headerY - radius, BLUE );
    tft.fillRoundRect( 0, screenH - 2 * radius, screenW, 2 * radius, radius, BLUE );

    tft.setTextSize( 2 );
    tft.setTextColor( BLUE, YELLOW );
    displayCenter("Weather", screenW2, 3, 2);
  }

  String dispTempF = String( iotData.tempF, 1 ) + " F ";
  String dispTempC = String( iotData.tempC , 1 ) + " C ";
  String dispHumid = String( iotData.humidity, 1 ) + " % ";
  String dispPress = String( iotData.slp, 1 ) + " mb";

  tft.setTextColor(YELLOW, BLUE);
  // left edge must clear border screenW - 3
  displayFlushRight( dispTempF, 125,  45, 2 );
  displayFlushRight( dispTempC, 125,  65, 2 );
  displayFlushRight( dispHumid, 125,  85, 2 );
  displayFlushRight( dispPress, 125, 105, 2 );

  beenDisplayed = true;
} // iotWXFrame()

// *******************************************************
// ************** IOT TELEMETRY FRAME ********************
// *******************************************************
void iotTelemFrame( boolean firstRender ) {
  static boolean beenDisplayed = false;
  // print static labels the first time
  if ( firstRender ) {
    int headerY = 40;
    int radius = 8;
    // top panel
    tft.fillRoundRect( 0, 0, screenW, 2 * radius, radius, BLUE );
    tft.fillRect( 0, radius, screenW, headerY - radius, BLUE );
    tft.fillRect( 0, headerY, screenW, screenH - headerY - radius, YELLOW );
    tft.fillRoundRect( 0, screenH - 2 * radius, screenW, 2 * radius, radius, YELLOW );

    tft.setTextSize(2);
    tft.setTextColor( YELLOW, BLUE );
    displayCenter("Telemetry", screenW2, 3, 2 );
  }

  String dispvCell = String( iotData.cellVoltage ) + " V  ";
  String dispdBm = String( iotData.dBm ) + " dBm";
  String dispAwake = String( iotData.timeAwake ) + " s  ";

  tft.setTextColor( BLUE, YELLOW );
  // left edge must clear border screenW - 3
  displayFlushRight( dispvCell, 125, 45, 2 );
  displayFlushRight( dispdBm,   125, 65, 2 );
  displayFlushRight( dispAwake, 125, 85, 2 );

  beenDisplayed = true;
} // iotTelemFrame()

// *******************************************************
// *********** DISPLAY TEXT FORMATTING *******************
// *******************************************************
void displayFlushRight( String text, int column, int line, int size ) {
  int numChars = text.length();
  int widthText = size * ( 6 * numChars - 1 );
  tft.setCursor( column - widthText, line );
  tft.print( text );
} // displayFlushRight()

void displayCenter( String text, int column, int line, int size ) {
  int numChars = text.length();
  int widthText = size * ( 6 * numChars - 1 );
  tft.setCursor( column - widthText / 2, line );
  tft.print( text );
} // displayCenter()

// *******************************************************
// ************* Retrive ThingSpeak Data *****************
// *******************************************************
void readTS() {
  iotData.tempC       = ThingSpeak.readFloatField(CHANNEL_ID, 1);
  iotData.humidity    = ThingSpeak.readFloatField(CHANNEL_ID, 2);
  iotData.timeAwake   = ThingSpeak.readFloatField(CHANNEL_ID, 3);
  iotData.slp         = ThingSpeak.readFloatField(CHANNEL_ID, 4);
  iotData.lightLevel  = ThingSpeak.readFloatField(CHANNEL_ID, 5);
  iotData.cellVoltage = ThingSpeak.readFloatField(CHANNEL_ID, 6);
  iotData.dBm         = ThingSpeak.readIntField  (CHANNEL_ID, 7);
  iotData.tempF       = ThingSpeak.readFloatField(CHANNEL_ID, 8);
} // readTS()
