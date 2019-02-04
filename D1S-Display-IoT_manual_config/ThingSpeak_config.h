// *******************************************************
// ************** ThingSpeak_config.h ********************
// *******************************************************

// This configuration file should reside in the same Arduino
// directory as the weather station file D1M-WX1_IoT_REST.ino.

// *******************************************************
// ****************** WIFI LOGON *************************
// *******************************************************

// ENTER YOUR WI-FI SSID
// YOU MUST USE 2.4 GHz WiFi, NOT 5 GHz
const char WIFI_SSID[] = "DCMNET"; // "your_wifi_ssid";

// ENTER YOUR WI-FI PASSWORD
const char WIFI_PASSWORD[] = "0F1A2D3E4D5G6L7O8R9Y"; // "your_wifi_password";

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

// ThingSpeak Channel ID & API Write Key
const String CHANNEL_ID = "286120"; //your_channel_id as a String value
const String API_READ_KEY = "25PKMG5G3YS7QJDT";    // between quotes

const long FRAME_INTERVAL = 5;                     // seconds to display frame

char timezone[6] = "ET";                // default value
/*
 * Eastern Europe:
 * Europe: UTC, CET
 * Africa: 
 * North America: ET, CT, MT, PT, AT, HT
 * South America:
 * Asia:
 */

// flags reserved for future development
const bool ANALOG_CLOCK_LOCAL = true;

const bool CHART_TEMP_C = true;         // false for F
const bool CHART_PRES_MB = true;        // false for inHg
const bool CHART_TREND_ONE_DAY = true;
const bool CHART_TREND_3HR = true;
const bool CHART_HUMID = true;

const bool TABLE_TEMP_C = false;        // false for F
const bool TABLE_PRES_MB = true;        // false for inHG
