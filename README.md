# IoT_Kits_Distribution
This is a set of firmware sketches for the **IoT Kits©** solar-powered, wireless weather stations. They will work with either the single-board or stacked kits available from [https://W4KRL.com/IoT-Kits](https://w4krl.com/iot-kits/).

There are four sketches for now. Two MQTT sketches are under development:
- **D1M-WX1_Calibration** - Calibrates the analog-to-digital converter
- **D1M-WX1_IoT_REST** - Posts data to ThingSpeak using the REST API
- **D1M-WX1_APRS_REST** - Posts data to ThingSpeak using the REST and to APRS
- **D1S-IoT-Remote-Display** - Sketch under development to read & display ThngSpeak feed
- D1M-WX1_IoT_MQTT - TO BE DEVELOPED Posts data to ThingSpeak using MQTT
- D1M-WX1_APRS_MQTT - TO BE DEVELOPED Posts data to ThingSpeak using MQTT and to APRS

This version posts weather data and telemetry to [ThingSpeak](http://www.thingspeak.com) using the REST API. Firmware that adds APRS connectivity is D1M-WX1_APRS_REST. 
## Installation
Click on the **Clone or download** button and select Download ZIP. The file will download as IoT_Kits_Distribution-master.zip. Unzip the file. Copy each of the folders within IoT_Kits_Distribution-master to your Arduino folder.
## Configuration of ThingSpeak.h
Open the sketch in the Arduino IDE. Select the ThingSpeak.h tab and edit the information for your station as indicated by the comments within the file. Save the file.
## Configuration of APRS.h

