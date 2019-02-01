# IoT_Kits_Distribution
This is a set of firmware sketches for the **IoT Kits©** solar-powered, wireless weather stations. They will work with either the single-board or stacked kits available from [https://W4KRL.com/IoT-Kits](https://w4krl.com/iot-kits/).

There are four sketches for now. Two MQTT sketches are under development:
- **D1M-WX1_Calibration** - Calibrates the analog-to-digital converter
- **D1M-WX1_IoT_REST** - Posts data to ThingSpeak using the REST API
- **D1M-WX1_APRS_REST** - Posts data to ThingSpeak using the REST and to APRS
- **D1M-Remote_APRS_Manual_Config** - 
- D1M-WX1_IoT_MQTT *TO BE DEVELOPED* - Posts data to ThingSpeak using MQTT
- D1M-WX1_APRS_MQTT *TO BE DEVELOPED* - Posts data to ThingSpeak using MQTT and to APRS

This version posts weather data and telemetry to [ThingSpeak](http://www.thingspeak.com) using the REST API. Firmware that adds APRS connectivity is D1M-WX1_APRS_REST. 
## Installation
Click on the **Clone or download** button and select Download ZIP. The file will download as IoT_Kits_Distribution-master.zip. Unzip the file. Copy each of the folders within IoT_Kits_Distribution-master to your Arduino folder.
## Configuration of ThingSpeak_config.h
All weather station sketches need a ThingSpeak_config.h file. It muts be located in the same folder as the sketch XXX.ino file.

Open the sketch in the Arduino IDE. Select the ThingSpeak_config.h tab and edit the information for your station as indicated by the comments within the file. 

Information needed:
- Your WiFi SSID
- Your WiFi password
- Station elevation in meters
- Sleep interval in seconds: 60 for testing, 600 for normal service
- ThingSpeak channel ID (a numerical value)
- ThingSpeak API Write Key (alphanumeric between quotes)
- OPTIONAL (Values determined from running D1M-WX1_Calibration.ino)
- - DMM voltage
- - ADC reading

Save the file.
## Configuration of APRS_config.h
Only sketches using APRS need an APRS_config.h file.

Information needed:
- latitude (decimal degrees, positive for north, negative for south)
- longitude (decimal degrees, positive for east, negative for west)
- CALLSIGN-SSID
- APRS passcode

Save the file.
