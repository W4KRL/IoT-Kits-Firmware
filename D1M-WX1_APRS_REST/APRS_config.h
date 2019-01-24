// *******************************************************
// ******************** APRS-IS **************************
// *******************************************************

// Enter your station location in decimal degrees
// For example: lat/lon for Washington Monument is 38.8908031, -77.0338225
// Find your location and elevation at
// https://www.distancesto.com/elevation.php
// Use negative numbers for South latitude and West longitude:
const float latitude = 38.8908031;
const float longitude = -77.0338225;

// define your callsign and passcode
// recommend to use SSID -13 for weather stations
// see http://www.aprs.org/aprs11/SSIDs.txt
const String APRScallsign = "your_call-13";

// for passcode www.george-smart.co.uk/aprs/aprs_callpass/
// or http://n5dux.com/ham/aprs-passcode/
const char APRSpasscode[] = "your_passcode";

// interval between transmissions of APRS telemetry definitions in seconds
const int APRS_TELEM_SPAN = 2 * 60 * 60; // once per 2 hours
