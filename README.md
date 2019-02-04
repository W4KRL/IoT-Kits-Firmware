# IoT-Kits-Firmware
This is a set of firmware sketches for the **IoT Kits©** solar-powered, wireless weather stations. They will work with either the single-board or stacked kits available from [W4KRL.com/IoT-Kits](https://w4krl.com/iot-kits/).

There are four sketches for now. Two MQTT sketches are under development:
1. **D1M-WX1_Calibration** - Use this to verify your Arduino/ESP8266 software and to calibrate the analog-to-digital converter in the weather station
2. **D1M-WX1_IoT_REST** - Posts data to ThingSpeak Internet of Things (IoT) service using the REST API
3. **D1M-WX1_APRS_REST** - Posts data to APRS-IS and to ThingSpeak using REST [**NOTE:** For licensed amateur radio operators only.]
4. **D1M-Display-APRS_Manual** - Displays weather station data from your APRS-IS feed. Uses manual configuration rather than WiFiManager config.
- D1M-WX1_IoT_MQTT - *IN DEVELOPMENT* - Posts data to ThingSpeak using MQTT
- D1M-WX1_APRS_MQTT - *IN DEVELOPMENT* - Posts data to APRS and to ThingSpeak using MQTT

## Installation
1. Click on the **Clone or download** button and select Download ZIP. The file will download as **IoT_Kits_Distribution-master.zip**. 
2. Unzip the file. Copy each of the folders within IoT_Kits_Distribution-master to your Arduino folder.
3. The sketches are provided with important library files in subfolders to ensure having a consistent software package. Comments within the sketches identify the library authors, version level, and source.
## Configuration of ThingSpeak_config.h
All weather station sketches need a ThingSpeak_config.h file. It must be located in the same folder as the sketch XXX.ino file.

Open the sketch in the Arduino IDE. Select the ThingSpeak_config.h tab and edit the information for your station as indicated by the comments within the file. 

Information needed:
- Your WiFi SSID
- Your WiFi password
- Station elevation in meters. You can get this at [www.freemaptools.com](https://www.freemaptools.com/elevation-finder.htm)
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
- * Find your location at [www.distancesto.com/](https://www.distancesto.com/coordinates.php)
- CALLSIGN-SSID
- APRS passcode

Save the file.

[I'm a reference-style link][Arbitrary case-insensitive reference text]
