// config.h for D1M-WX1_IoT-APRS_Franklin42.ino

// This configuration file should reside in the same Arduino
// directory as the weather station file D1M-WX1_IoT-APRS_FranklinV3.ino.

// *******************************************************
// ********************* WIFI LOGON **********************
// *******************************************************

// ENTER YOUR WI-FI SSID
// YOU MUST USE 2.4 GHz WiFi, NOT 5 GHz
const char WIFI_SSID[] = "DCMNET";

// ENTER YOUR WI-FI PASSWORD
const char WIFI_PASSWORD[] = "0F1A2D3E4D5G6L7O8R9Y";

// *******************************************************
// **************** STATION FACTORS **********************
// *******************************************************

// If you have performed the voltage calibration steps, enter
// the voltage you measured on your digital multimeter and
// the voltage reported by the ADC on the serial monitor.
// Calibration sketch is D1M-WX1_Calibration.ino
// If you have not performed the calibration, do not change 
// the default values. They should be equal.

const float dmmVoltage = 4.71;         // voltage displayed on your digital multimeter
const float adcVoltage = 4.97;         // voltage reported by the Analog to Digital Converter

// station altitude in meters
// https://www.freemaptools.com/elevation-finder.htm
const float STATION_ELEV = 90.0;

// update interval in seconds
// must be longer than 15 seconds
// suggest 60 seconds for testing, 300 or 600 for use
const long SLEEP_INTERVAL = 60;

// *******************************************************
// ******************** THINGSPEAK ***********************
// *******************************************************

// Open a ThingSpeak account at www.thingspeak.com
/* Define fields:
 *  Field 1 Temperature °C
 *  Field 2 Humidity
 *  Field 3 Time Awake
 *  Field 4 Sea Level Pressure
 *  Field 5 Light Intensity
 *  Field 6 Cell Voltage
 *  Field 7 RSSI
 *  Field 8 Temperature °F
 *  Show Location checked
 *  Show Status checked
 */

// ThingSpeak Channel ID & API Write Key
const long CHANNEL_ID = 190041;
const String API_WRITE_KEY = "DPYSLGVHEAECE9FV";
