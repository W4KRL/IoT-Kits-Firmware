// *******************************************************
// ********************* config.h ***********************
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

// ************ WIFI & APRS CONFIG PARAMS ****************
// these strings hold the user's response to each parameter
String aprsMyCall = "W4KRL-4";            // call and ssid for display. suggest -4
String aprsTheirCall = "W4KRL-15";        // call and ssid of your weather station
char aprs_passcode[6] = "";               // https://apps.magicbug.co.uk/passcode/
char aprs_filter[] = "b/W4KRL-*";         // default value
char timezone[6] = "ET";                  // default value
