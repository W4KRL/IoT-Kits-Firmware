# IoT-Kits-Firmware
This is a set of firmware sketches for the **IoT Kits©** solar-powered, wireless weather stations. They will work with either the single-board or stacked kits available from [W4KRL.com/IoT-Kits](https://w4krl.com/iot-kits/).

There are five active sketches:
1. **D1M-WX1-Calibration** - Use this to verify that you have installed the ESP8266 core in the Arduino IDE. It can also calibrate the analog-to-digital converter in the weather station.
2. **D1M-WX1-IoT-REST** - Posts data to ThingSpeak Internet of Things (IoT) service using the REST API.
3. **D1M-WX1-APRS-REST** - Posts data to APRS-IS and to ThingSpeak using REST [**NOTE:** For licensed amateur radio operators only.]
4. **D1S-Display-APRS-manual-config** - Displays weather station data from your APRS-IS feed. Uses manual configuration rather than WiFiManager captive page configuration.
5. **D1S-Display-IoT-manual-config** - This displays weather station data from your ThingSpeak channel and uses manual configuration rather than WiFiManager captive page configuration.

**NOTE:** *As of July 2021 some versions of the ESP8266 Core are deffective. Use version 2.7.4. Use Arduino IDE Tools | Boards | Board Manager... and search for ESP8266. Select version 2.7.4 to install.*

## Installation
1. Click on the **Clone or download** button and select Download ZIP. The file will download as **IoT-Kits-Firmware-master.zip**. 
2. Unzip the file. Change the name of the extracted folder to **IoT-Kits**. Copy this to your Arduino sketchbook folder. If you need to find your sketchbook folder, open the Arduino IDE and use menu File | Preferences. The first line tells you where the sketchbook resides on your computer. 
3. The sketches are provided with important library files in subfolders to ensure having a consistent software package. Comments within the sketches identify the library authors, version levels, and sources.

## Configuration of ThingSpeak_config.h
All weather station sketches need a *ThingSpeak_config.h* file. It must be located in the same folder as the sketch *XXX.ino* file.

Open the sketch in the Arduino IDE. Select the *ThingSpeak_config.h* or appropriate tab and edit the information for your station as indicated by the comments within the file. 

### Information needed:
- Your WiFi SSID **(You must use 2.4 GHz not 5 GHz.)**
- Your WiFi password
- Station elevation in meters. You can get this at [www.freemaptools.com](https://www.freemaptools.com/elevation-finder.htm)
- Sleep interval in seconds: 60 for testing, 600 or longer for normal service
- ThingSpeak channel ID (a numerical value)
- ThingSpeak API Write Key (alphanumeric between quotes)
- For the remote display kits, you must select the appropriate timezone by uncommenting your location.
- OPTIONAL (Values determined from running *D1M-WX1_Calibration.ino*)
  - DMM voltage
  - ADC reading

Save the sketch. Set the PROG/RUN switch to **PROG** and upload to the microcontroller. **Return the switch to RUN after a sucessful upload.**

## Configuration of APRS_config.h
Only sketches using APRS need an *APRS_config.h* file. You must have a valid amateur radio license to use APRS.

### Information needed:
- Find your location at [www.distancesto.com/](https://www.distancesto.com/coordinates.php)
  - latitude (decimal degrees, positive for north, negative for south)
  - longitude (decimal degrees, positive for east, negative for west)
- CALLSIGN-SSID
- APRS passcode
- For the remote display kits, you must select the appropriate timezone by uncommenting your location.

Save the sketch. Set the **PROG/RUN** switch to PROG and upload to the microcontroller. **Return the switch to RUN after a sucessful upload.**
