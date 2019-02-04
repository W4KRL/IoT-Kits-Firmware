// *******************************************************
// ****************** APRS_config.h **********************
// *******************************************************

// This configuration file must reside in the same Arduino
// directory as the weather station sketch

// *******************************************************
// ********************* WIFI LOGON **********************
// *******************************************************

// ENTER YOUR WI-FI SSID
// !!!!!  YOU MUST USE 2.4 GHz WiFi, NOT 5 GHz  !!!!!
const char WIFI_SSID[] = ""; // "your_wifi_ssid";

// ENTER YOUR WI-FI PASSWORD
const char WIFI_PASSWORD[] = ""; // "your_wifi_password";

// *******************************************************
// ***************** APRS CONFIG PARAMS ******************
// *******************************************************
const String APRS_MY_CALL = "";                 // Your call and ssid for display. Suggest -4
const String APRS_THEIR_CALL = "W4KRL-15";      // default - Your call and ssid of your weather station
const char APRS_PASSCODE[6] = "";               // https://apps.magicbug.co.uk/passcode/
const char APRS_FILTER[] = "b/W4KRL-*";         // default value - Change to "b-your call-*"
const char TIME_ZONE[6] = "ET";                 // Change to your time zone from list below.

/* VALID TIMEZONE ABBREVIATIONS
 *  
 *  ET = Eastern US
 *  CT = Central US
 *  MT = Mountain US
 *  PT = Pacific US
 *  AKT = Alaska US
 *  HT = Hawaii US
 */
