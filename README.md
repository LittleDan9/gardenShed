# Sensor'd up Garden Shed Project
Current implementation uses:<br/>
<ol>
<li>ESP8266 to monitor sensors.</li>
<li>Raspberry Pi 3 to read via UDP senors data from the ESP8266, Node.js & Express website to view sensor data, MySQL to log sensor data</li>
<li>MySQL logs sensor data and provide address information to the ESP8266, scaled for multiple ESP8266s</li>
</ol>
