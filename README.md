# IoT_Kits_Firmware
This is a set of firmware sketches for the **IoT KitsÂ©** solar-powered, wireless weather stations. They will work with either the single-board or stacked kits available from [https://W4KRL.com/IoT-Kits](https://w4krl.com/iot-kits/).

There are five sketches:
- **D1M-WX1_Calibration** - Calibrates the analog-to-digital converter
- **D1M-WX1_IoT_REST** - Posts data to ThingSpeak using the REST API
- **D1M-WX1_APRS_REST** - Posts data to ThingSpeak using the REST and to APRS
- **D1M-WX1_IoT_MQTT** - Posts data to ThingSpeak using MQTT
- **D1M-WX1_APRS_MQTT** - Posts data ti ThingSpeak using MQTT and to APRS

This version posts weather data and telemetry to [ThingSpeak](http://www.thingspeak.com) using the REST API. Firmware that adds APRS connectivity is D1M-WX1_APRS_REST. 
## Installation
Click on the **Clone or download** button and select Download ZIP. The file will download as D1-WX1_IoT_REST-master.zip. Unzip the file. Rename the unzipped folder to **D1M-WX1_IoT_REST**. Copy the folder to your Arduino folder.
## Configuration
Open the sketch in the Arduino IDE. Select the ThingSpeak.h tab and edit the information for your station as indicated by the comments within the file. Save the file.
