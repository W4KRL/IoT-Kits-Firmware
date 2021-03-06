// *******************************************************
// ************** ThingSpeak_config.h ********************
// *******************************************************

// This configuration file must reside in the same Arduino
// directory as the weather station sketch.

// *******************************************************
// ********************* WIFI LOGON **********************
// *******************************************************

// ENTER YOUR WI-FI SSID
// !!!!!  YOU MUST USE 2.4 GHz WiFi, NOT 5 GHz !!!!!
const char WIFI_SSID[] = "your_wifi_ssid";

// ENTER YOUR WI-FI PASSWORD
const char WIFI_PASSWORD[] = "your_wifi_password";

// *******************************************************
// ****************** STATION FACTORS ********************
// *******************************************************

// If you have performed the voltage calibration steps, enter
// the voltage you measured on your digital multimeter and
// the voltage reported by the ADC on the serial monitor.
// Calibration sketch is D1M-WX1_Calibration.ino
// If you have not performed the calibration, do not change 
// the default values. They should be equal.

const float dmmVoltage = 4.20;  // voltage displayed on your digital multimeter 
const float adcVoltage = 4.20;  // voltage reported by the D1 Mini Analog to Digital Converter

// Enter your station altitude in meters
// https://www.distancesto.com/elevation.php
const float STATION_ELEV = 0.0;

// Enter the update interval in seconds
// The interval must be longer than 15 seconds
// Use 60 seconds for testing, 600 or 900 for normal use
const long SLEEP_INTERVAL = 600;

// *******************************************************
// ******************** THINGSPEAK ***********************
// *******************************************************

// Open a ThingSpeak account at www.thingspeak.com
// Configure your channel fields as follows:
/* Define fields:
 *  Field 1 Temperature ï¿½C
 *  Field 2 Humidity
 *  Field 3 Time Awake
 *  Field 4 Sea Level Pressure
 *  Field 5 Light Intensity
 *  Field 6 Cell Voltage
 *  Field 7 RSSI
 *  Field 8 Temperature ï¿½F
 *  Show Location checked
 *  Show Status checked
 */
 
// Enter your ThingSpeak channel information here
// ThingSpeak Channel ID & API Write Key
const long CHANNEL_ID = 00000; // numerical value
const String API_WRITE_KEY = "your_api_write_key";  // between quotes

// *******************************************************
// ***** CHANGE ONLY IF YOU KNOW WHAT YOU ARE DOING! *****
// *******************************************************

const long  MIN_RSSI = -80;                       // warning level for weak WiFi
const float MIN_VCELL = 3.0;                      // warning level for low cell voltage
const char  IOT_SERVER[] = "api.thingspeak.com";  // ThingSpeak Server
