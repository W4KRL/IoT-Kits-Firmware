// D1S-IoT-Remote-Display.ino
// 09/14/2018 Mod to D1S-APRS-Remote-Display

/*_____________________________________________________________________________
   Copyright(c) 2018 Berger Engineering dba IoT Kits

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
// ******* Set true to erase config data *****************
// *******************************************************
// Use only if all else fails.
// If your device is confused or constantly rebooting:
// Set RESET_WIFI = true;
// Upload the firmware:
//    - DO NOT DO ANYTHING ELSE
//    - DO NOT CONFIGURE YOUR DEVICE
// After it has run once, change RESET_WFI = true; and reload.
// Now you can do the normal configuration process.
const boolean RESET_WIFI = false; // erases WiFI & APRS config for testing

// *******************************************************
// ******************* INCLUDES **************************
// *******************************************************
// For general sketch
#include <ESP8266WiFi.h>          // [builtin] 
#include <Ticker.h>               // [builtin] simple task scheduler
#include "ThingSpeak.h"

// for WiFiManager library
#include <FS.h>                   // [builtin] SPIFFS File System
#include <DNSServer.h>            // [builtin] For webserver
#include <ESP8266WebServer.h>     // [builtin] For webserver
#include <WiFiManager.h>          // [manager] https://github.com/tzapu/WiFiManager
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// use ArduinoJson version 5.x.x Do not upgrade to version 6 due at end of 2018
// code was tested with version 5.13.2
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include <ArduinoJson.h>          // [manager] https://github.com/bblanchon/ArduinoJson

// for Wemos TFT 1.4 display shield
#include <Adafruit_GFX.h>         // [manager] Core graphics library
#include <Adafruit_ST7735.h>      // [manager] Hardware-specific library
#include <SPI.h>                  // [builtin]
#include <Fonts/FreeSerif9pt7b.h> // part of GFX for APRS message text

// for TimeLib and TimeZone
#include <Timezone.h>             // https://github.com/JChristensen/Timezone
#include <TimeLib.h>              // https://github.com/PaulStoffregen/Time
#include <WiFiUdp.h>              // needed for NTP

// *******************************************************
// ******************* DEFAULTS **************************
// *******************************************************
//            DO NOT CHANGE THESE DEFAULTS
const long FRAME_INTERVAL = 5;                  // seconds to display frame
const char AP_PORTAL_NAME[] = "IoT Kits";
const int NTP_UPDATE_INTERVAL = 3600;           // seconds between NTP synchronization
static const char ntpServerName[] = "us.pool.ntp.org";
const unsigned int LOCAL_PORT = 8888;           // local port to listen for UDP packets

// *******************************************************
// ********* INSTANTIATE DEVICES *************************
// *******************************************************
// Wemos TFT 1.4 shield connections
// TFT_CS      D4  GPIO  2
// TFT_RST     -1  use  -1 according to Wemos
// TFT_DC      D3  GPIO  0
// MOSI        D7  GPIO 13
// SCK         D5  GPIO 14
// TFT_LED     NC
// Push Button D1  GPIO  5
// Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);
Adafruit_ST7735 tft = Adafruit_ST7735(2,  0, -1);

WiFiClient client;            // Wi-Fi connection

Ticker secondPulse;           // one second interval drives display
Ticker flasher;               // indicates wifiManager mode

WiFiUDP Udp;                  // NTP

// *******************************************************
// ******************* GLOBALS ***************************
// *******************************************************
char APRSage[9] = "";                    // time stamp for received data
const int buttonPin = D1;                // push button connection
boolean configRequest = false;           // set true if user presses button 2 seconds after splash screen

// ************ WIFI & APRS CONFIG PARAMS ****************
// these strings hold the user's response to each parameter
char ts_channel_id[10];
char ts_read_key[20];
char timezone[6] = "ET";                  // default value

// *******************************************************
// ******************* TIME SETTINGS *********************
// *******************************************************
time_t utc;
char tzAbbrev[6] = "ET";
int localHour = 0;        // local hour 0 - 23

// TimeChangeRule("time zone abbrev", week, day, month, hour, offset)
// UTC
TimeChangeRule utcRule = {"UTC", Last, Sun, Mar, 1, 0};          // UTC
Timezone UTC(utcRule, utcRule);                                  // no DST - use same rule

// US Eastern Time Zone (New York, Detroit)
TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};       // Eastern Daylight Time = UTC - 4 hours
TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};        // Eastern Standard Time = UTC - 5 hours
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
Timezone usAZ(usMST, usMST);                                      // Mountain Standard Time = UTC - 7

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
const int BLACK   =  0x0000;
const int BROWN   =  0xA145;
//const int RED     =  0xF800     // official RED
const int RED     =  0xFA20;    // use ORANGERED for RED
const int ORANGE  =  0xFD20;
const int YELLOW  =  0xFFE0;
const int GREEN   =  0x9772;    // given in table as LIGHTGREEN
//const int LIME    =  0x07E0;  // adafruit choice for GREEN
const int BLUE    =  0x001F;
const int VIOLET  =  0xEC1D;
const int GRAY    =  0x8410;
const int WHITE   =  0xFFFF;
const int GOLD    =  0xFEA0;
const int SILVER  =  0xC618;
const int CYAN    =  0x07FF;
const int MAGENTA =  0xF81F;
const int ORANGERED = 0xFA20;  // my choice for RED

// *******************************************************
// *************** WIFIMANAGER ***************************
// *******************************************************

// declaring it here makes it global using more space
// WiFiManager wifiManager;
// flag for saving config data
// WiFiManager will decide if save is needed
bool shouldSaveConfig = false;

// *******************************************************
// ******************** SETUP ****************************
// *******************************************************
void setup() {
  Serial.begin( 115200);
  pinMode(buttonPin, INPUT_PULLUP);   // use internal pullup resistor

  // Initializer for 1.4-in. TFT ST7735S chip, green tab
  tft.initR(INITR_144GREENTAB);

  Serial.println("IoT Kits splash");
  splashScreen();   // stays on until logon is complete
  delay (2000);     // give user time to press reset
  if ( digitalRead(buttonPin) == LOW ) {
    configRequest = true;
  }

  // erase FS config data - used for testing
  if ( RESET_WIFI ) SPIFFS.format();

  // read configuration from FS json
  Serial.println("Open File System");
  openSPIFFS();

  flasher.attach(0.5, flashLED);  // start slow flash

  // starting wifiManager here uses less memory
  WiFiManager wifiManager;

  // set callbacks
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  // ************************
  // add user parameters to config web page
  // ************************
  wifiManager.setCustomHeadElement("<h1 style=\"color:blue;\">IoT Kits</h1>");
  WiFiManagerParameter custom_text_ts("<b>Enter ThingSpeak info:</b>");
  WiFiManagerParameter custom_ts_channel_id("Channel ID", "Channel ID", ts_channel_id, 10);
  WiFiManagerParameter custom_ts_read_key("Read key", "Read Key", ts_read_key, 20);
  WiFiManagerParameter custom_text_tz("<b>Enter time zone:</b>");
  WiFiManagerParameter custom_timezone("Time Zone", "Time Zone (ET)", timezone, 6);

  wifiManager.addParameter(&custom_text_ts);
  wifiManager.addParameter(&custom_ts_channel_id);
  wifiManager.addParameter(&custom_ts_read_key);
  wifiManager.addParameter(&custom_text_tz);
  wifiManager.addParameter(&custom_timezone);

  // reset settings - used for testing
  if ( RESET_WIFI ) wifiManager.resetSettings();

  // forces exit if config takes longer than timeout
  wifiManager.setTimeout(120);        // in seconds

  if ( configRequest == true ) {
    Serial.println("User requested config");
    wifiManager.startConfigPortal(AP_PORTAL_NAME);
  } else {
    Serial.println("Attempt WiFi connection using saved config");
    wifiManager.autoConnect(AP_PORTAL_NAME);
  }

  if ( WiFi.status() != WL_CONNECTED ) {
    Serial.println("Failed to connect within timeout");
    tft.fillScreen(BLACK);            // clear screen
    tft.setCursor(0, 0);
    tft.setTextSize(2);
    tft.setTextColor(RED);
    tft.println("Failed to connect");
    tft.print("Resetting");
    delay(5000);
    ESP.reset();
  }

  // Wi-Fi is connected
  Serial.println("Connected OK");
  flasher.detach();  // stop flasher

  // copy user's config responses
  strcpy(ts_channel_id, custom_ts_channel_id.getValue());
  strcpy(ts_read_key, custom_ts_read_key.getValue());
  strcpy(timezone, custom_timezone.getValue());

  // convert time zone abbreviation to uppercase
  for (int i = 0; i < strlen(timezone); i++) {
    timezone[i] = toupper(timezone[i]);
  }

  // APRS calls must have trailing spaces removed
//  aprsMyCall = aprs_my_call;
//  aprsMyCall.trim();
//  aprsTheirCall = aprs_their_call;
//  aprsTheirCall.trim();

  // save the custom parameters to File System
  if ( shouldSaveConfig ) {
    Serial.println("Save new config");
    saveConfig();
  }

  // ************************
  // proceed with unit's main purpose in life
  // ************************
  tft.fillScreen(BLACK);                 // clear screen
  tft.setCursor(0, 0);
  tft.setTextSize(1);
  tft.setTextColor(YELLOW);
  Serial.print("Local IP: ");            // show Wi-Fi IP
  Serial.println(WiFi.localIP());
  tft.println("Connected to:");
  tft.println( WiFi.localIP() );

  Udp.begin(LOCAL_PORT);                 // for NTP
  setSyncProvider(getNtpTime);           // sync with NTP
  setSyncInterval(NTP_UPDATE_INTERVAL);
  utc = now();                           // get UTC
  setTime(utc);                          // pass it to TimeZone
  getRTCtime();                          // set local time

//  Serial.print("APRS logon");
//  logonToAPRS();
  delay ( 5000 );                        // delay to show connection info

  // start 1-second ticker at end of setup
  secondPulse.attach(1, secondEvents);

  tft.fillScreen(BLACK);                 // clear screen
} //setup()

// *******************************************************
// ******************** LOOP *****************************
// *******************************************************
void loop() {
  // continually check APRS server for incoming data
  String APRSrcvd = APRSreceiveData();

  // ignore comments and short strings (10 is arbitrary)
  if ( APRSrcvd[0] != APRS_ID_COMMENT && APRSrcvd.length() > 10 ) {
    // does stream contain weather data?
    if ( APRSrcvd.indexOf(APRS_ID_WEATHER) > 0 ) {
      if ( APRSdataWeather != APRSrcvd ) { // it has changed so update it
        APRSdataWeather = APRSrcvd;
        // record time data is received in nice format
        sprintf(APRSage, "%02d:%02d:%02d", localHour, minute(), second());
      }
    }
    // does stream contain Telemetry?
    if ( APRSrcvd.indexOf("T#") > 0 ) {  // better than looking for 'T'
      if ( APRSdataTelemetry != APRSrcvd ) {
        APRSdataTelemetry = APRSrcvd;
      }
    }
    // does stream contain a message?
    if ( APRSrcvd.indexOf("::") > 0 ) {      // special case
      // place to detect short press
      APRSdataMessage = APRSrcvd;
      if (aprsMessageFrame() == true) {
        secondPulse.detach();                // pause everything
        // loop here until buttonPin goes LOW
        while ( digitalRead(buttonPin) == HIGH ) {
          yield();                           // make sure the ESP8266 does its stuff
        }
        secondPulse.attach(1, secondEvents); // restart normal tasks
      }
    }
  }
} // loop()

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
      aprsWXFrame( firstTime );
      firstTime = false;
      break;
    case 3:
      aprsTelemetryFrame( firstTime );
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
  displayCenter( "D1S-RCVR", screenW2, topLine,      2 );
  displayCenter( "ThingSpeak",  screenW2, topLine + 20, 2 );
  displayCenter( "Receiver", screenW2, topLine + 40, 2 );
  displayCenter( "IoT Kits", screenW2, topLine + 60, 2 );
  displayCenter( "by W4KRL", screenW2, topLine + 80, 2 );
  for (int i = 0; i < 4; i++) {
    tft.drawRoundRect(12 - 3 * i, 12 - 3 * i, screenW - 12, screenH - 12, 8, YELLOW);
  }
} // splashScreen()

// *******************************************************
// *************** INSTRUCTION SCREEN ********************
// *******************************************************
void displayInstructionScreen() {
  tft.fillScreen(YELLOW);
  tft.setTextSize(2);
  tft.setTextColor(BLUE);
  int topLine = 19;
  displayCenter( "Connect", screenW2, topLine, 2 );
  tft.setTextSize(1);
  displayCenter( "IoT Kits", screenW2, topLine + 20, 1 );
  tft.setTextSize(2);
  displayCenter( "Browse", screenW2, topLine + 40, 2 );
  tft.setTextSize(1);
  displayCenter( "192.168.4.1", screenW2, topLine + 60, 1 );
  tft.setTextSize(2);
  displayCenter( "Fill form", screenW2, topLine + 80, 2 );
  for (int i = 0; i < 2; i++) {
    tft.drawRoundRect(6 - 3 * i, 6 - 3 * i, screenW - 12, screenH - 12, 8, BLUE);
  }
} // displayInstructionScreen()

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
    tft.drawRoundRect(0, 0, screenW, screenH, 8, RED);
    tft.setTextSize( 2 );
    tft.setCursor( 4, 4 );
    tft.setTextColor( WHITE, BLACK );
    tft.print(tzAbbrev);  // local time zone abbreviation
    tft.drawCircle( clockCenterX, clockCenterY, 2, YELLOW );  // hub
    tft.drawCircle( clockCenterX, clockCenterY, clockDialRadius, WHITE );  // dial edge
    tft.setTextSize( 1 );  // for dial numerals
    tft.setTextColor( GREEN, BLACK );
    // add hour ticks & numerals
    for ( int numeral = 1; numeral < 13; numeral++ ) {
      // Begin at 30° and stop at 360° (noon)
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
  tft.drawLine( clockCenterX, clockCenterY, x3, y3, RED );

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
  tft.drawLine( clockCenterX, clockCenterY, x3, y3, GREEN );

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
    tft.setTextColor(RED, BLACK);
    displayCenter("UTC", screenW2, topLine, 2);
    tft.setTextColor(GREEN, BLACK);
    // local time zone
    displayCenter(tzAbbrev, screenW2, topLine + 60, 2);
  }
  // display times
  tft.setTextColor(RED, BLACK);
  tft.setCursor(17, topLine + 20);
  tft.print( utcTime );

  tft.setTextColor(GREEN, BLACK);
  tft.setCursor(17, topLine + 80);
  tft.print( localTime );
} // digitalClockFrame()

// *******************************************************
// ************* APRS WEATHER FRAME **********************
// *******************************************************
void aprsWXFrame( boolean firstRender ) {
  // see APRS Protocol pg 65
  static boolean beenDisplayed = false;
  int index = 0;
  char tempF[4] = "";
  char humid[3] = "";
  char barom[6] = "";
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
    displayCenter(aprs_their_call, screenW2, 23, 2);
  }

  if ( APRSdataWeather.indexOf( APRS_ID_WEATHER ) > 0 ) {
    // remove "waiting" message when weather data is found
    if ( !beenDisplayed ) {
      tft.setTextColor(BLUE, BLUE);
      displayCenter( "Waiting on", screenW2, 50, 2 );
      displayCenter( "Data", screenW2, 70, 2 );
    }
    // temperature
    index = APRSdataWeather.indexOf( 't' );
    for ( int i = 0; i < 3; i++ ) {
      tempF[i] = APRSdataWeather[index + 1 + i];
    }
    int itempF = atoi( tempF );
    int itempC = ( itempF - 32 ) * 5 / 9;

    // humidity
    index = APRSdataWeather.indexOf( 'h', index + 3 );
    for (int i = 0; i < 2; i++) {
      humid[i] = APRSdataWeather[index + 1 + i];
    }
    int iHumid = atoi( humid );

    // barometric pressure
    index = APRSdataWeather.indexOf( 'b', index + 2 );
    for ( int i = 0; i < 5; i++ ) {
      barom[i] = APRSdataWeather[index + 1 + i];
    }
    float Press = atof( barom ) / 10;

    String dispTempF = String( itempF ) + " F ";
    String dispTempC = String( itempC ) + " C ";
    String dispHumid = String( iHumid ) + " % ";
    String dispPress = String( Press, 1) + " mb";

    tft.setTextColor(YELLOW, BLUE);
    // left edge must clear border screenW - 3
    displayFlushRight(dispTempF, 125, 45, 2);
    displayFlushRight(dispTempC, 125, 65, 2);
    displayFlushRight(dispHumid, 125, 85, 2);
    displayFlushRight(dispPress, 125, 105, 2);

    beenDisplayed = true;
  } else {
    tft.setTextColor(YELLOW, BLUE);
    displayCenter( "Waiting on", screenW2, 50, 2 );
    displayCenter( "Data", screenW2, 70, 2 );
  }
} // aprsWXFrame()

// *******************************************************
// ************ APRS TELEMETRY FRAME *********************
// *******************************************************
// hard coded to match telemetry used in D1M-WX1 Solar Weather station
void aprsTelemetryFrame( boolean firstRender ) {
  // see APRS Protocol pg 68
  static boolean beenDisplayed = false;
  int index = 0;
  char vCell[4] = "";
  char lux[4] = "";
  char rssi[4] = "";
  tft.setTextSize(2);
  if ( firstRender ) {
    int headerY = 40;
    int radius = 8;
    tft.fillRoundRect(0, 0, screenW, 2 * radius, radius, BLUE);
    tft.fillRect(0, radius, screenW, headerY - radius, BLUE );
    tft.fillRect(0, headerY, screenW, screenH - headerY - radius, YELLOW );
    tft.fillRoundRect(0, screenH - 2 * radius, screenW, 2 * radius, radius, YELLOW);

    tft.setTextColor(YELLOW, BLUE);
    displayCenter("Telemetry", screenW2, 3, 2);
    displayCenter( aprs_their_call, screenW2, 23, 2 );
  }

  index = APRSdataTelemetry.indexOf( "T#" );
  if ( index > 0 ) {
    // telemetry data is found
    if ( !beenDisplayed ) {
      tft.setTextColor(YELLOW, YELLOW);  // erase waiting text
      displayCenter( "Waiting on", screenW2, 50, 2 );
      displayCenter( "Data", screenW2, 70, 2 );
    }
    index = APRSdataTelemetry.indexOf(',', index + 3);  // comma at end of sequence number
    for (int i = 0; i < 3; i++) {
      vCell[i] = APRSdataTelemetry[index + 1 + i];
    }
    float ivCell = atof(vCell) * 0.01961;  // known scaling factor

    index = APRSdataTelemetry.indexOf(',', index + 2);
    for (int i = 0; i < 3; i++) {
      rssi[i] = APRSdataTelemetry[index + 1 + i];
    }
    int irssi = - atoi(rssi);  // known scaling factor (-1)

    index = APRSdataTelemetry.indexOf(',', index + 2);
    for (int i = 0; i < 3; i++) {
      lux[i] = APRSdataTelemetry[index + 1 + i];
    }
    long ilux = atoi(lux) * 257;  // known scaling factor
    tft.setTextColor(BLUE, YELLOW);
    // align right side of values and left side of units
    String dispVdc = String( ivCell ) + " Vdc";
    String dispdBm = String( irssi ) + " dBm";
    String dispLux = String( ilux ) + " lux";

    displayFlushRight(dispVdc, 125, 45, 2);
    displayFlushRight(dispdBm, 125, 65, 2);
    displayFlushRight(dispLux, 125, 85, 2);

    // show time data was received
    tft.setTextSize(1);
    tft.setTextColor(RED);
    char str[16];
    strcpy(str, "Rcvd @ ");
    strcat(str, APRSage);
    displayCenter(str, screenW2, 108, 1);

    beenDisplayed = true;
  } else {
    tft.setTextColor(BLUE, YELLOW);
    displayCenter( "Waiting on", screenW2, 50, 2 );
    displayCenter( "Data", screenW2, 70, 2 );
  }
} // aprsTelemetryFrame()

// *******************************************************
// ************ APRS MESSAGE FRAME ***********************
// *******************************************************
boolean aprsMessageFrame() {
  // see APRS Protocol pg 71
  // a message can be from 0 to 67 characters
  boolean messageDisplayed = false;
  int index = APRSdataMessage.indexOf(APRS_ID_STATUS);
  String APRSsender = APRSdataMessage.substring(0, index);
  index = APRSdataMessage.indexOf("::");  // special case
  String APRSrcvr = APRSdataMessage.substring(index + 2, index + 11);
  String APRSmessage = APRSdataMessage.substring(index + 12);

  boolean telemFlag = false;
  if (APRSmessage.indexOf("PARM") != -1) telemFlag = true;
  if (APRSmessage.indexOf("UNIT") != -1) telemFlag = true;
  if (APRSmessage.indexOf("EQNS") != -1) telemFlag = true;
  if (APRSmessage.indexOf("BITS") != -1) telemFlag = true;

  if ( telemFlag == false) {
    tft.fillRect(0, 0, screenW, 27, RED );
    tft.fillRect(0, 28, screenW, screenH - 28, BLACK );
    tft.setTextColor(YELLOW, RED);
    tft.setTextSize(1);
    tft.setCursor(0, 0);
    tft.print("Msg from:");
    tft.setTextSize(2);
    tft.setCursor(0, 10);
    tft.print( APRSsender );
    tft.setTextSize(1);
    tft.setCursor(0, 35);
    tft.setTextColor(YELLOW, BLACK);
    tft.setFont(&FreeSerif9pt7b);  // looks nice and permits longer text
    tft.print(APRSmessage);
    messageDisplayed = true;
    index = APRSdataMessage.indexOf(APRS_ID_USER_DEF);  // device should acknowledge message
    if ( index > 0 ) {
      String APRSmsgID = APRSdataMessage.substring(index + 1);
      APRSsendACK( APRSsender, APRSmsgID );
    }
  }
  tft.setFont();                 // return to standard font
  return messageDisplayed;
} // aprsMessageFrame()

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
// *********** WiFi Manager Functions ********************
// *******************************************************
void openSPIFFS() {
  // SPIFFS = Serial Peripheral Inteface Flash File System
  // http://esp8266.github.io/Arduino/versions/2.0.0/doc/filesystem.html
  // from wifiManager example AutoConnectWithFSParameters
  // https://github.com/tzapu/WiFiManager/tree/master/examples/AutoConnectWithFSParameters
  if ( SPIFFS.begin() ) {
    Serial.println("Mount file system");
    if (SPIFFS.exists("/config.json")) {
      // file exists - read and copy to globals
      Serial.println("Read config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if ( configFile ) {
        Serial.println("Opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if ( json.success() ) {
          Serial.println("\nParsed json");
          // copy from json to global strings
          strcpy(aprs_my_call, json["aprs_my_call"]);
          strcpy(aprs_passcode, json["aprs_passcode"]);
          strcpy(aprs_their_call, json["aprs_their_call"]);
          strcpy(aprs_filter, json["aprs_filter"]);
          strcpy(timezone, json["timezone"]);
        } else {
          Serial.println("Failed to load json config");
        }
      }
    }
  } else {
    Serial.println("Failed to mount FS");
  }
  //end read
  Serial.println("Finished spiffs read");
} // openSPIFFS()

void configModeCallback (WiFiManager * myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  displayInstructionScreen();  // shows a reminder on the TFT display
  flasher.attach(0.1, flashLED);  // rapid flash
} // configModeCallback()

// callback when there is need to save config
void saveConfigCallback () {
  Serial.println("Config save needed");
  shouldSaveConfig = true;
} // saveConfigCallback()

// saves user parameters in SPIFFS
void saveConfig() {
  Serial.println("Saving config");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  // copy globals to json
  json["aprs_my_call"] = aprs_my_call;
  json["aprs_passcode"] = aprs_passcode;
  json["aprs_their_call"] = aprs_their_call;
  json["aprs_filter"] = aprs_filter;
  json["timezone"] = timezone;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
  }
  json.printTo(Serial);
  json.printTo(configFile);
  configFile.close();
  //end save
} // saveConfig()

void flashLED() {
  // turns LED on if off and off if on
  // called by Ticker
  int ledState = digitalRead(BUILTIN_LED);
  digitalWrite(BUILTIN_LED, !ledState);
} // flashLED()

void getRTCtime() {
  utc = now();
  TimeChangeRule *tcr;
  time_t t;
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
// ********** Net Time Protocol Functions ****************
// *******************************************************
const int NTP_PACKET_SIZE = 48;       // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE];   // buffer to hold incoming & outgoing packets

time_t getNtpTime() {
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ;     // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
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
  // (see URL above for details on the packets)
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
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
} // sendNTPPacket()
