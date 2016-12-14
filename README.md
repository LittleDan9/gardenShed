Garden Shed Sensor Communications
==========================================================================================================================================
I crafted this project as a christmas gift for my wife to monitor conditions in a garden shed that she uses to start seeds. The project is implemented using an ESP8266 and a Raspberry Pi 3 along with a DHT22/AM2302 attached to one of the GPIOs on the ESP8266.

[ESP8266](/ESP8266/) - Lua
----------------------------------------------------------------------------------------------------------------------------------------
The ESP8266 resides in a remote location (the garden shed) and must have access to WiFi. I am utilizing DD-WRT'd routers to range extend my network the necessary distance. The ESP8266 w/ NodeMCU implementation currently loads three files, an [init](/ESP8266/init.lua) routine, [TCP Socket Server](/ESP8266/DHTNetServer.lua), and a [Buffer Generator](/ESP8266/DHTBuffer.lua) for returning the DHT22/AM2302 data in either HTML or JSON format. The operation of these files is further explained in the ESP8266 [README](/ESP8266/README.md). Due to the short time I had to get a working protoype in place I used the clould based [NodeMCU firmware builder](https://nodemcu-build.com "NodeMCU Custom Firmware Builder") rather than building from the Expressif SDK (Future Enhancement).

Short list of the NodeMCU Firmware Modules:

* tmr - For managing the init script.
* file -  To store the files on the internal storage.
* dht - Library to read the DHT/AM2302 Sensor.
* gpio - Needed to communicate the DHT/AM2302 sensor attached to the GPIO.
* net - Used to set up the TCP server.
* node - Not used yet.
* wifi - Used to initizlize the WiFi and/or setup static configuration.
* ssl - Not implmented yet.
* http - Not implemented yet.


Raspbery Pi - Node.js (Express), JavaScript, MySQL
----------------------------------------------------------------------------------------------------------------------------------------


TODO:
----------------------------------------------------------------------------------------------------------------------------------------
- [x] Write Intro
- [x] Write ESP8266 Intro
- [ ] Write RaspberryPi Intro
- [ ] Write ESP8266 README
- [ ] Write nodeServer README
