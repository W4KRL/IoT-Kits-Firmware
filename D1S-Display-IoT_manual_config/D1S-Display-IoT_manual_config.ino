// D1S-IoT-Remote-Display-Manual-Config.ino

// 02/04/2019 - released as beta
// 02/04/2019 - Added weather and telemetry frames
// 01/30/2019 - ST7735 & ArduinoJson libraries moved to src folder
// 01/28/2019 - Removed WiFiManager
// 01/28/2019 - Mod to D1S-APRS-Remote-Display

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
// For general sketch
#include <ESP8266WiFi.h>                  // [builtin] 
#include <Ticker.h>                       // [builtin] simple task scheduler
#include <WiFiUdp.h>                      // [builtin] for NTP

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Use ArduinoJson version 5.13.4 by Benoit Blanchon as bundled with this sketch
// Do not upgrade to version 6.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "src/ArduinoJson.h"              // [manager] https://github.com/bblanchon/ArduinoJson

// for Wemos TFT 1.4 display shield
#include <Adafruit_GFX.h>                 // [manager] Core graphics library v1.3.6
#include <Fonts/FreeSerif9pt7b.h>         // part of Adafruit GFX
#include "src/Adafruit/Adafruit_ST7735.h" // [manager] v1.2.7
#include <SPI.h>                          // [builtin]


// for TimeLib and TimeZone
#include "src/Timezone/Timezone.h"        // [manager] https://github.com/JChristensen/Timezone
#include <TimeLib.h>                      // [zip] https://github.com/PaulStoffregen/Time

#include "ThingSpeak_config.h"            // your ThingSpeak_config.h file must be located in the sketch folder

// *******************************************************
// ******************* DEFAULTS **************************
// *******************************************************
//            DO NOT CHANGE THESE DEFAULTS
const int NTP_UPDATE_INTERVAL = 3600;               // seconds between NTP synchronization
static const char NTP_SERVER_NAME[] = "us.pool.ntp.org";
const unsigned int UDP_LOCAL_PORT = 8888;           // local port to listen for UDP packets
const char THINGSPEAK_HOST[] = "api.thingspeak.com";
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

Ticker secondPulse;           // one second interval drives display
WiFiClient client;
WiFiUDP udp;                  // NTP
WiFiServer TSserver(80);      // ThingSpeak server

// *******************************************************
// ******************* GLOBALS ***************************
// *******************************************************
const int pinButton = D1;     // push button connection
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
  long  entry_id = 0;         // entry id from ThingSpeak
} iotData;

// *******************************************************
// ******************* TIME SETTINGS *********************
// *******************************************************
time_t utc;
char tzAbbrev[6] = "ET";
int localHour = 0;           // local hour 0 - 23

// TimeChangeRule("time zone abbrev", week, day, month, hour, offset)
// UTC
TimeChangeRule utcRule = {"UTC", Last, Sun, Mar, 1, 0};     // UTC
Timezone UTC(utcRule, utcRule);                             // no DST - use same rule

// CET Central Euopean Time
TimeChangeRule ceST = {"CEST", Last, Sun, Mar, 1, 0};       // Central European Summer Time
TimeChangeRule ceT  = {"CET", Last, Sun, Oct, 1, 60};       // Central Europena Time

// US Eastern Time Zone (New York, Detroit)
TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  // Eastern Daylight Time = UTC - 4 hours
TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   // Eastern Standard Time = UTC - 5 hours
Timezone usET(usEDT, usEST);

// US Central Time Zone (Chicago, Houston)
TimeChangeRule usCDT = {"CDT", Second, Sun, Mar, 2, -300};  // Central Daylight Time = UTC - 5
TimeChangeRule usCST = {"CST", First, Sun, Nov, 2, -360};   // Central Standard Time = UTC - 6
Timezone usCT(usCDT, usCST);

// US Mountain Time Zone (Denver, Salt Lake City)
TimeChangeRule usMDT = {"MDT", Second, Sun, Mar, 2, -360};  // Mountain Daylight Time = UTC - 6
TimeChangeRule usMST = {"MST", First, Sun, Nov, 2, -420};   // Mountain Standard Time = UTC - 7
Timezone usMT(usMDT, usMST);

// Arizona is US Mountain Time Zone but does not use DST
Timezone usAZ(usMST, usMST);                                // Mountain Standard Time = UTC - 7

// US Pacific Time Zone (Las Vegas, Los Angeles)
TimeChangeRule usPDT = {"PDT", Second, Sun, Mar, 2, -420};  // Pacific Daylight Time = UTC - 7
TimeChangeRule usPST = {"PST", First, Sun, Nov, 2, -480};   // Pacific Standard Time = UTC - 8
Timezone usPT(usPDT, usPST);

// US Alaska Time Zone
TimeChangeRule usAKDT = {"AKDT", Second, Sun, Mar, 2, -480}; // Alaska Daylight Time = UTC - 8
TimeChangeRule usAKST = {"AKST", First, Sun, Nov, 2, -540};  // Alaska Standard Time = UTC - 7
Timezone usAKT(usAKDT, usAKST);

// US Hawaii Time Zone
TimeChangeRule usHST = {"HST", First, Sun, Nov, 2, -600};    // Hawaii Standard Time = UTC - 10
Timezone usHT(usHST, usHST);

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
  delay (5000);

  logonToRouter();                        // logon to WiFi

  tft.fillScreen( BLACK );                // clear screen
  tft.setCursor( 0, 0 );
  tft.setTextSize( 1 );
  tft.setTextColor( YELLOW );
  tft.println("Connected to:");
  tft.println( WiFi.localIP() );
  
  Serial.print("Local IP: ");             // show Wi-Fi IP
  Serial.println( WiFi.localIP() );
  
  udp.begin( UDP_LOCAL_PORT );            // for NTP
  setSyncProvider( getNtpTime );          // sync with NTP
  setSyncInterval( NTP_UPDATE_INTERVAL );
  utc = now();                            // get UTC
  setTime( utc );                         // pass it to TimeZone
  getRTCtime();                           // set local time

  // start 1-second ticker at end of setup
  secondPulse.attach(1, secondEvents);

  tft.fillScreen(BLACK);                  // clear screen
} //setup()

// *******************************************************
// ******************** LOOP *****************************
// *******************************************************
void loop() {
  RetrieveTSChannelData();
  delay(15000);                           // Delay between requests
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
// **************** SECOND EVENTS ************************
// *******************************************************
// this is a Ticker callback for things to do every second
void secondEvents() {
  getRTCtime();  // get the latest time from the real time clock
  frameUpdate(); // select frame to display
} // secondEvents()

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
  // message frame handled separately
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
  tft.fillScreen(BLUE);
  tft.setTextSize(2);
  tft.setTextColor(YELLOW);
  int topLine = 19;
  displayCenter( "D1S-IoT", screenW2, topLine,      2 );
  displayCenter( "Remote",  screenW2, topLine + 20, 2 );
  displayCenter( "Display", screenW2, topLine + 40, 2 );
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
    tft.print(tzAbbrev);  // local time zone abbreviation
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
      if ( numeral == 12 ) tft.setCursor( x4 - 5, y4 - 4);
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
  int dialHour = localHour;
  if (dialHour > 13 ) dialHour = dialHour - 12;
  // 30 degree increments + adjust for minutes
  angle = localHour * 30 + int(( minute() / 12 ) * 6 );
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
  char utcTime[9];
  char localTime[9];

  sprintf(utcTime, "%02d:%02d:%02d", hour(), minute(), second());
  sprintf(localTime, "%02d:%02d:%02d", localHour, minute(), second());

  tft.setTextSize(2);
  if ( firstRender ) {
    // print labels
    tft.drawRoundRect(0, 0, screenW, screenH, 8, WHITE);
    tft.setTextColor(ORANGERED, BLACK);
    displayCenter("UTC", screenW2, topLine, 2);
    tft.setTextColor(LIGHTGREEN, BLACK);
    // local time zone
    displayCenter(tzAbbrev, screenW2, topLine + 60, 2);
  }
  // display times
  tft.setTextColor(ORANGERED, BLACK);
  tft.setCursor(17, topLine + 20);
  tft.print( utcTime );

  tft.setTextColor(LIGHTGREEN, BLACK);
  tft.setCursor(17, topLine + 80);
  tft.print( localTime );
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
    tft.fillRoundRect(0, 0, screenW, 2 * radius, radius, YELLOW);
    tft.fillRect(0, radius, screenW, headerY - radius, YELLOW );
    tft.fillRect(0, headerY, screenW, screenH - headerY - radius, BLUE );
    tft.fillRoundRect(0, screenH - 2 * radius, screenW, 2 * radius, radius, BLUE );

    tft.setTextSize(2);
    tft.setTextColor( BLUE, YELLOW );
    displayCenter("Weather", screenW2, 3, 2);
  }

  String dispTempF = String( iotData.tempF, 1 ) + " F ";
  String dispTempC = String( iotData.tempC , 1 ) + " C ";
  String dispHumid = String( iotData.humidity, 1 ) + " % ";
  String dispPress = String( iotData.slp, 1 ) + " mb";

  tft.setTextColor(YELLOW, BLUE);
  // left edge must clear border screenW - 3
  displayFlushRight(dispTempF, 125, 45, 2);
  displayFlushRight(dispTempC, 125, 65, 2);
  displayFlushRight(dispHumid, 125, 85, 2);
  displayFlushRight(dispPress, 125, 105, 2);

  beenDisplayed = true;
  //  } else {
  //    tft.setTextColor(YELLOW, BLUE);
  //    displayCenter( "Waiting on", screenW2, 50, 2 );
  //    displayCenter( "Data", screenW2, 70, 2 );
  //  }
} // aprsWXFrame()

// *******************************************************
// ************** IOT TELEMTRY FRAME *********************
// *******************************************************
void iotTelemFrame( boolean firstRender ) {
  static boolean beenDisplayed = false;
  // print static labels the first time
  if ( firstRender ) {
    int headerY = 40;
    int radius = 8;
    // top panel
    tft.fillRoundRect(0, 0, screenW, 2 * radius, radius, BLUE);
    tft.fillRect(0, radius, screenW, headerY - radius, BLUE );
    tft.fillRect(0, headerY, screenW, screenH - headerY - radius, YELLOW );
    tft.fillRoundRect(0, screenH - 2 * radius, screenW, 2 * radius, radius, YELLOW );

    tft.setTextSize(2);
    tft.setTextColor( YELLOW, BLUE );
    displayCenter("Telemetry", screenW2, 3, 2);
  }

  String dispvCell = String( iotData.cellVoltage ) + " V ";
  String dispdBm = String( iotData.dBm ) + " dBm ";
  String dispAwake = String( iotData.timeAwake ) + " s ";
  //      String dispPress = String( iotData.slp, 1) + " mb";

  tft.setTextColor( BLUE, YELLOW );
  // left edge must clear border screenW - 3
  displayFlushRight(dispvCell, 125, 45, 2);
  displayFlushRight(dispdBm, 125, 65, 2);
  displayFlushRight(dispAwake, 125, 85, 2);
  //      displayFlushRight(dispPress, 125, 105, 2);

  beenDisplayed = true;
  //  } else {
  //    tft.setTextColor( BLUE, YELLOW );
  //    displayCenter( "Waiting on", screenW2, 50, 2 );
  //    displayCenter( "Data", screenW2, 70, 2 );
  //  }
} // aprsWXFrame()

// *******************************************************
// *********** DISPLAY TEXT FORMATTING *******************
// *******************************************************
void displayFlushRight( String text, int column, int line, int size ) {
  int numChars = text.length();
  int widthText = size * ( 6 * numChars - 1 );
  tft.setCursor(column - widthText, line);
  tft.print( text );
} // displayFlushRight()

void displayCenter( String text, int column, int line, int size ) {
  int numChars = text.length();
  int widthText = size * ( 6 * numChars - 1 );
  tft.setCursor(column - widthText / 2, line);
  tft.print( text );
} // displayCenter()

// *******************************************************
// ****************** FLASH LED **************************
// *******************************************************
void flashLED() {
  // flashes LED
  // called by Ticker
  digitalWrite(LED_BUILTIN, LOW);
  delay (100);
  digitalWrite(LED_BUILTIN, HIGH);
} // flashLED()

// *******************************************************
// *************** GET RTC TIME **************************
// *******************************************************

void getRTCtime() {
  utc = now();
  TimeChangeRule *tcr;
  time_t t;
  // DST rule is found for the users timezone
  if (strcmp(timezone, "ET") == 0) t = usET.toLocal(utc, &tcr);
  else if (strcmp(timezone, "CT") == 0) t = usCT.toLocal(utc, &tcr);
  else if (strcmp(timezone, "MT") == 0) t = usMT.toLocal(utc, &tcr);
  else if (strcmp(timezone, "PT") == 0) t = usPT.toLocal(utc, &tcr);
  else if (strcmp(timezone, "AKT") == 0) t = usAKT.toLocal(utc, &tcr);
  else if (strcmp(timezone, "HT") == 0) t = usHT.toLocal(utc, &tcr);
  else t = UTC.toLocal(utc, &tcr);    // unrecognized time zone - default to UTC

  strcpy(tzAbbrev, tcr -> abbrev);
  localHour = hour(t);
} // getRTCtime()

// *******************************************************
// ********** NET TIME PROTOCOL FUNCTIONS ****************
// *******************************************************
const int NTP_PACKET_SIZE = 48;       // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE];   // buffer to hold incoming & outgoing packets

time_t getNtpTime() {
  IPAddress ntpServerIP; // NTP server's ip address

  while (udp.parsePacket() > 0) ;     // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(NTP_SERVER_NAME, ntpServerIP);
  Serial.print(NTP_SERVER_NAME);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL;   // We want UTC - do not adjust for time zone
    }
  }
  Serial.println("No NTP Response");
  return 0; // return 0 if unable to get the time
} // getNtpTime()

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
} // sendNTPPacket()

// *******************************************************
// ************* Retrive ThingSpeak Data *****************
// *******************************************************
void RetrieveTSChannelData() {          // Receive data from Thingspeak
  static char responseBuffer[3 * 1024]; // Buffer for received data
  client = TSserver.available();
  if (!client.connect(THINGSPEAK_HOST, 80)) {
    Serial.println("connection failed");
    return;
  }
  String readRequest = "GET /channels/" + CHANNEL_ID;
  readRequest += "/feeds.json?results=1";  // 5 is the results request number, so 5 are returned, 1 woudl return the last result received
  client.println(readRequest + " HTTP/1.1");
  client.println("Host: " + (String)THINGSPEAK_HOST);
  client.println("Connection: close");
  client.println("");
  while (!skipResponseHeaders());                      // Wait until there is some data and skip headers
  while (client.available()) {                         // Now receive the data
    String line = client.readStringUntil('\n');
    if ( line.indexOf('{', 0) >= 0 ) {                 // Ignore data that is not likely to be JSON formatted, so must contain a '{'
      line.toCharArray(responseBuffer, line.length()); // Convert to char array for the JSON decoder
      Serial.print("JSON: "); Serial.println(line);    // Show the text received
      parseJSON(responseBuffer);                       // Decode the JSON text
    }
  }
  client.stop();
}

bool skipResponseHeaders() {
  char endOfHeaders[] = "\r\n\r\n"; // HTTP headers end with an empty line
  client.setTimeout( 10000 );
  bool ok = client.find( endOfHeaders );
  if (!ok) {
    Serial.println("No response or invalid response!");
  }
  return ok;
}

// JSON Parsing
void parseJSON(char *json) {
  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(10) + JSON_OBJECT_SIZE(17) + 620;
  DynamicJsonBuffer jsonBuffer(capacity);

  JsonObject& root = jsonBuffer.parseObject(json);

  //  JsonObject& channel = root["channel"];
  //  long channel_id = channel["id"];

  JsonObject& feeds0 = root["feeds"][0];

  const char* feeds0_created_at = feeds0["created_at"];
  iotData.entry_id    = feeds0["entry_id"];
  iotData.tempC       = feeds0["field1"];
  iotData.humidity    = feeds0["field2"];
  iotData.timeAwake   = feeds0["field3"];
  iotData.slp         = feeds0["field4"];
  iotData.lightLevel  = feeds0["field5"];
  iotData.cellVoltage = feeds0["field6"];
  iotData.dBm         = feeds0["field7"];
  iotData.tempF       = feeds0["field8"];
  Serial.println(feeds0_created_at);
  Serial.println(iotData.entry_id);
  Serial.print("Temp  F: "); Serial.println(iotData.tempF);
  Serial.print("Pres mb: "); Serial.println(iotData.slp);
}
