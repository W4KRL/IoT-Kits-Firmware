// *******************************************************
// ******************** APRS-IS **************************
// *******************************************************

// Enter your station location in decimal degrees
// For example: lat/lon for Washington Monument is 38.8908031, -77.0338225
// Find your location and elevation at
// http://veloroutes.org/elevation/
const float latitude = 38.8368356;
const float longitude = -77.4796706;

// define your callsign and passcode
// recommend to use SSID -13 for weather stations
// see http://www.aprs.org/aprs11/SSIDs.txt
const String APRScallsign = "W4KRL-13";

// for passcode www.george-smart.co.uk/aprs/aprs_callpass/
// or http://n5dux.com/ham/aprs-passcode/
const char APRSpasscode[] = "9092";

// interval between transmissions of APRS telemetry definitions in seconds
const int APRS_TELEM_SPAN = 2 * 60 * 60; // once per 2 hours
