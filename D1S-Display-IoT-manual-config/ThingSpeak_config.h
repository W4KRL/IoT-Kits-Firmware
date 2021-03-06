// *******************************************************
// ************** ThingSpeak_config.h ********************
// *******************************************************

// This configuration file should reside in the same Arduino
// directory as the weather station file D1S-Display-IoT-man-config-ezTime.ino.

// *******************************************************
// ****************** WIFI LOGON *************************
// *******************************************************

// ENTER YOUR WI-FI SSID
// YOU MUST USE 2.4 GHz WiFi, NOT 5 GHz
const char WIFI_SSID[] = ""; // "your_wifi_ssid";

// ENTER YOUR WI-FI PASSWORD
const char WIFI_PASSWORD[] = ""; // "your_wifi_password";

// *******************************************************
// ****************** THINGSPEAK *************************
// *******************************************************

// Open a ThingSpeak account at www.thingspeak.com
/* Define fields:
    Field 1 Temperature °C
    Field 2 Humidity
    Field 3 Time Awake
    Field 4 Sea Level Pressure
    Field 5 Light Intensity
    Field 6 Cell Voltage
    Field 7 RSSI
    Field 8 Temperature °F
    Show Location checked
    Show Status checked
*/

// ThingSpeak Channel ID & API Read Key
const unsigned long CHANNEL_ID = ; // enter channel number
const String API_READ_KEY = "";    // enter read key between quotes

const long FRAME_INTERVAL = 5;                     // seconds to display frame

// uncomment only your location
// or add your location using time zones definitions from:
// https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
String tzLocation = "America/New_York";            // Eastern
//String tzLocation = "America/Chicago";             // Central
//String tzLocation = "America/Denver";              // Mountain
//String tzLocation = "America/Los_Angeles";         // Pacific
//String tzLocation = "Pacific/Honolulu";            // Hawaii
//String tzLocation = "America/Anchorage";           // Alaska
//String tzLocation = "America/New_York";            // UTC
//String tzLocation = "Europe/Zurich";               // Central Europe
//String tzLocation = "Asia/Tokyo";                 // Japan
