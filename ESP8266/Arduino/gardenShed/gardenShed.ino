  /********************************************************
  * gardenShed.ino
  * Description: Main runtime script for the garden shed 
  *              conditions/time module.
  * Author:  Daniel R. Little
  * Company: Little Squared, Inc.
  * Created: 12/03/2016
  ********************************************************/
  
  /*******************************************************/  
  /*ESP Includes                                         */
  /*******************************************************/  
  #ifdef ESP8266
    extern "C" {
    #include "user_interface.h"
    }
  #endif  
  #include "ESP8266WiFi.h"
  #include "ESP8266mDNS.h"
  #include "ESP8266WebServer.h"
  
  /*******************************************************/
  /*Sensor Includes                                      */  
  /*******************************************************/  
  #include "DHT.h"
  #include "SPI.h"

  /*******************************************************/
  /*Graphics Includes                                    */  
  /*******************************************************/    
  #include "Adafruit_GFX.h"
  #include "Adafruit_ILI9341.h"
  #include "GfxUi.h"

  /*******************************************************/
  /*Resource Includes                                    */  
  /*******************************************************/  
  #include "WebResource.h"
  #include "ArduinoJson.h"
  #include "FS.h"
  #include "EEPROM.h"

  /*******************************************************/
  /*HTML Includes                                        */
  /*******************************************************/
  #include "HomeHTML.h"
  #include "WiFiConfigHTML.h"
  #include "SysConfigHTML.h"
  #include "ConditionsHTML.h"
  #include "AboutHTML.h"
  #include "JQuery.h"
  #include "Bootstrap.h"
  
  
  /*******************************************************/
  /*Font Includes                                        */
  /*******************************************************/  
  #include "Liberation_Sqns_16.h"
  #include "URWGothicLDegree_30.h"
  #include "Digital7_40.h"
  #include "Digital7_14.h"
    
  /*******************************************************/
  /*Configurations                                       */
  /*******************************************************/
  int wifiAddressStart = 0;
  struct WiFiConfig {
    char hostname[30] = "";
    char ssid[30] = "";
    char password[30] = "";
  };
  WiFiConfig wifiConfig;
  
  //This works because the addresses are 0 based
  int sysAddressStart = sizeof(wifiConfig); 
  struct SysConfig{
    char deviceName[30] = "New Device";
    char deviceLocation[60] = "New Location";
    bool displayClock = true;
    long interval = 10000;
    int screenRotation = 1;
    char username[30] = "root";
    char password[30] = "esp8266";
    long httpPort = 80;
    long tcpPort = 4210;
    bool deleteFileSystem = false;
    //Raspberry Pi Stuff        
  };
  SysConfig sysConfig;
  
  /*******************************************************/
  /*System Constants                                      */
  /*******************************************************/
  #define DEBUG true
  int chipId = system_get_chip_id();
  ADC_MODE(ADC_VCC); //Allows polling of Supply Voltage
  /*******************************************************/
  /*TFT                                                  */
  /*******************************************************/
  #define TFT_DC  D1
  #define TFT_CS  D2
  #define GFX_ORANGE 0xF4A8
  #define SCREEN_WIDTH 320
  #define SCREEN_HEIGHT 240
  Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
  GfxUi ui = GfxUi(&tft);
  
  /*******************************************************/
  /*DHT Sensor                                           */
  /*******************************************************/
  #define DHTPIN  D3
  #define DHTTYPE DHT22
  #define TEMP_OFFSET 8
  DHT dht(DHTPIN, DHTTYPE);
  /*******************************************************/
  /*TCP / HTTP Server                                    */
  /*******************************************************/
  WiFiServer JSONServer(sysConfig.tcpPort);
  ESP8266WebServer webServer(sysConfig.httpPort);
  /*******************************************************/
  /*Downloader                                           */
  /*******************************************************/
  WebResource webResource;
  /*******************************************************/
  /*Program Control                                      */
  /*******************************************************/
  unsigned long previousMillis = 0;
  unsigned long prevTFTMillis = 0;
  bool isConnected = false;
  /*******************************************************/
  /*Layout Heplper                                       */
  /*******************************************************/
  #define iconBoxWidth 56
  #define iconOffset 20
  #define fontHeight 46
  
  //Offset the text 10px from the icons.
  int textOffset = (iconOffset + iconBoxWidth + 10);
  int middleOfTop = (SCREEN_HEIGHT/4)*1;
  int middleOfBottom = (SCREEN_HEIGHT/4)*3;
  int textBoxWidth = (SCREEN_WIDTH-textOffset)-6;
  int halfFontHeight = fontHeight/2;  
  float currentTemp = 0.00;
  float currentHumid = 0.00;
  bool isNotification = false;
  String txtNotify = "";
  
  /*******************************************************/
  /*Layout Helper - Clock                                */
  /*******************************************************/ 
  #define MAX_FAIL_COUNT 10
  unsigned long lastTimeGetMillis = 0;
  int failCount = 0;
  
  bool isClock = false;
  bool is7Segment = false;
  bool showDate = false;

  
  bool reprintHourHand = false;
  bool reprintMinHand = false;
  float xCenter = (SCREEN_WIDTH/2.0);
  float yCenter = (SCREEN_HEIGHT/2.0)-25;
  int timeFontHeight = 30;
  int radius = ((SCREEN_HEIGHT-(timeFontHeight*3))/2)-5;
  int curMin = -1;
  int curHour = -1;
  int curSec = -1;
  String curStrDate = "NA";
  struct point {
    float x;
    float y;
  }; typedef point Point; 

  
  struct DateTime {
    String strDate;
    String strTime;
    int hours = -1;
    int hours12 = hours > 12 ? hours - 12 : hours;
    int minutes;
    int seconds;
    int day;
    int month;
    int year;
    String meridian = hours > 12 ? "PM" : "AM";
    Point curHourHand;
    Point curMinHand;
    Point curSecHand;
  };  
  DateTime gDateTime;
  
  /*******************************************************/
  /*Function Prototypes                                  */
  /*******************************************************/
  //Network, TCP & HTTP Server
  void setupWiFi();
  void checkTCP();
  void setupHTTP();
  void getNetworksJSON();
  void getNetworkSettingsJSON();
  void getDeviceDetailsJSON();
  void getTempSensorDataJSON();  
  void sendHTML(char* HTML);
  void sendCSS(char* CSS);
  void sendJS(char* JS);
  void wifiSaveRequest();
    
  //Application Setup & Control
  void runBootProcess();
  bool saveWiFiConfig(String hostname, String ssid, String password);
  bool loadWiFiConfig();
  void printNotification();
  void drawProgress(uint8_t percentage, String text);
  void setupScreenBoot();
  void eraseScreen(); 
  void downloadResources(uint8_t percentage);  
  void downloadCallback(String filename, int16_t bytesDownloaded, int16_t bytesTotal);
  ProgressCallback _downloadCallback = downloadCallback;
  
  //Conditions 
  void printConditions();
  void setupScreenConditions();
  
  //Clock
  bool getDateTime();
  void drawHour(int hours, int minutes, bool erase);
  void drawMinute(int minutes, int seconds, bool erase);
  void drawSecond(int seconds);
  void drawClockFace();
  void setupScreenClock();
  void eraseHand(Point point);
  void printDateTime(); 
  void addSeconds();
  /*******************************************************/
  /*Init Routine                                         */
  /*******************************************************/ 
  void setup() {  
    //Setup Serial for debug
    if(DEBUG){
      Serial.begin(115200);
      Serial.println("");
      Serial.println("Garden Shed Conditions Server");
      Serial.println("ChipID:" + String(chipId));
    }
    
    loadSysConfig();
    
    //Initialize the DHT Sensor Object and TFT Object
    dht.begin();
    tft.begin();
            
    runBootProcess();

    previousMillis = millis();
    //We will always start with condition. Being that this is the primary purpose.
    eraseScreen();
    setupScreenConditions();  
    printConditions();
    yield();
  }
  /*******************************************************/
  /*Loop Routine                                         */
  /*******************************************************/ 
  void loop(void) { 
    isConnected = WiFi.status() == WL_CONNECTED;
    unsigned long currentMillis = millis();    
    //Check for TCP Request or HTTP Request
    checkTCP();
    webServer.handleClient();
    //If we are not connected to a network, we need to setup the WiFi
    if(isConnected){
      //We must be connected
      if(sysConfig.displayClock)
      {
        if(currentMillis - previousMillis >= sysConfig.interval){
          previousMillis = currentMillis;
          //Time to flip/flop
          if(!isClock && getDateTime()){ //Not Clock and successfully got time - Setup Clock
            isClock = true;
            eraseScreen();
            setupScreenClock();          
          }else{ //Is Clock - Setup Conditions
            isClock = false;
            eraseScreen();
            setupScreenConditions();
          }
        }
        
        if(isClock){
          addSeconds();
          printDateTime();
        }else{
          printConditions();        
        }
      }else{
        isClock = false;
        printConditions();
      }
    } else { //Not Connected
      if(isClock){
        //If the clock was displaying, we should end the clock and go to conditions
        isClock = false;        
        eraseScreen();
        setupScreenConditions();        
      }
      //Always print the conditions so we don't just loop trying to connect.
      printConditions();
      //Attempted to Connect
      setupWiFi();          
    }
  }
  /*******************************************************/ 

  void runBootProcess(){
    int bootProgress = 0;
    int totalBootEvents = 5;
    if(sysConfig.deleteFileSystem)
      totalBootEvents++;
    int bootInterval = 100 / totalBootEvents;   
    
    //Prepare the interface for use.    
    setupScreenBoot();

    //Connect to WiFi
    drawProgress(bootProgress, "Connecting to WiFi");
    setupWiFi();
    
    //Start mDNS
    drawProgress(bootProgress += bootInterval, "Starting mDNS Server");
    if (!MDNS.begin(wifiConfig.hostname)) {
      if(DEBUG){Serial.println("Error setting up MDNS responder!");}
    }else{
      if(DEBUG){Serial.println("mDNS responder started");}    
    }      
    
    //Start the TCP Server to notify requester the current conditions.
    drawProgress(bootProgress += bootInterval, "Starting TCP Server");
    JSONServer.begin();      
    MDNS.addService("JSON", "tcp", sysConfig.tcpPort);
    
    //Start web server
    drawProgress(bootProgress += bootInterval, "Starting HTTP Server");
    setupHTTP();
    webServer.begin();

    if(sysConfig.deleteFileSystem){
      //We will need to uncomment the next three lines to erase the file system and redownload.
      bootInterval = 100 / (totalBootEvents+1);
      drawProgress(bootProgress += bootInterval, "Formatting File System");
      SPIFFS.format();
      sysConfig.deleteFileSystem = false;
      saveSysConfig();
    }   
    
    //Download necessary external resources.
    drawProgress(bootProgress += bootInterval, "Downloading Resources");
    downloadResources(bootProgress); 
    //Notify the user we are complete.
    drawProgress(bootProgress += bootInterval, "Completed");
    delay(2);
  }
  
  
  /*******************************************************/
  /* Helpers methods for getting / managing time         */
  /* Drawing / erasing the hands on the clock            */
  /*******************************************************/
  bool getDateTime(){
    bool isTimeConnected = false;
    WiFiClient timeClient;
    String JSON = "";

    for(int i = 0; i < 10; i++){
      if(timeClient.connect("littlerichele.com", 80)){
        isTimeConnected = true;
        break;
      }      
    }
    
    if(isTimeConnected){
      failCount = 0;
    }else{
      if(DEBUG){Serial.println("Unable to access time server");};
      failCount++;
      if(failCount > MAX_FAIL_COUNT || !isClock){
        isNotification = true;
        txtNotify = "Unable to access time server";
        failCount = 0;
      }
      return false;
    }
         
    timeClient.print(String("GET ") + "/DateTimeNow" + " HTTP/1.1\r\n" +
               "Host: littlerichele.com\r\n" +
               "Connection: close\r\n" +
               "\r\n"
              );
              
    while (timeClient.connected()){
      if(timeClient.available()){
        String line = timeClient.readStringUntil('\n');
        if(line.startsWith("{")){
          JSON = line;
          timeClient.flush();
          break;
        }
      }
    }
    timeClient.stop();
    yield();
      
    if(JSON.length() <= 0){
      //This is not good, but we connected, so there is an issues on the server
      //or there is a timing issue where we tried to process the response but did not
      //receive it, meaning the network is acting slow.
      if(DEBUG){Serial.println("Unable to read time data");}
      isNotification = true;
      txtNotify = "No time data received";
      return false;
    }

    //Prase the JSON and display the time on the clock
    StaticJsonBuffer<300> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(JSON);
    if(!root.success()){ 
      if(DEBUG){Serial.println("Unable to extract time data");}
      isNotification = true;
      txtNotify = "Invalid time data";
      return false;
    }
    
    //When did we last get the time?
    lastTimeGetMillis = millis();

    gDateTime.strDate   = root["date"].asString();
    gDateTime.strTime   = root["time"].asString();
    gDateTime.hours     = root["hr"];
    gDateTime.hours12   = gDateTime.hours > 12 ? gDateTime.hours - 12 : root["hr"];
    gDateTime.meridian  = gDateTime.hours > 12 ? "PM" : "AM";
    gDateTime.minutes   = root["min"];
    gDateTime.seconds   = root["sec"];
    gDateTime.month     = root["month"];
    gDateTime.month++;  
    gDateTime.day       = root["day"];
    gDateTime.year      = root["year"];    
    return true;
  }

  void addSeconds(){
    //See if we need to add time to the current time.
    int seconds = 0;
    unsigned long currentMillis = millis();
    int diff = (int)(currentMillis - lastTimeGetMillis);
    if(diff > 1000){
      lastTimeGetMillis = currentMillis;
      seconds = (diff / 1000) == 0 ? 1 : (diff/1000);
    }
        
    if(gDateTime.seconds + seconds > 59){
      gDateTime.seconds = gDateTime.seconds + seconds - 60;
      if(gDateTime.minutes == 59){
        gDateTime.minutes = 0;
        if(gDateTime.hours == 12){
          gDateTime.hours = 1;
        }
      }else{
        gDateTime.minutes += 1;
      }
    }else{
      gDateTime.seconds += seconds;
    }
  }
  
  void drawHour(int hours, int minutes, bool erase){
    //Draw Hour Hand - 30° per hour 0.5° per minute.
    float hFraction = (float)(3.0/5.0);
    float hRads = (((hours * 30.0) + (float)(minutes * 0.5))*71)/4068.0;
    float xHour = xCenter + radius * sin(hRads);
    float yHour = yCenter - radius * cos(hRads);
    xHour = (xCenter + (hFraction * (xHour - xCenter)));
    yHour = (yCenter + (hFraction * (yHour - yCenter)));
    if(erase)
      eraseHand(gDateTime.curHourHand);    
    tft.drawLine(xCenter, yCenter, xHour, yHour, ILI9341_WHITE);
    gDateTime.curHourHand = {xHour, yHour};
  }

  void drawMinute(int minutes, int seconds, bool erase){
    //Draw Minute Hand - 6° per Mins
    float mFraction = (float)(5.0/6.0);
    float mRads = (float)((minutes * 6.0) * 71.0)/4068.0;
    float xMin = xCenter + radius * sin(mRads);
    float yMin = yCenter - radius * cos(mRads);
    xMin = (xCenter + (mFraction * (xMin - xCenter)));
    yMin = (yCenter + (mFraction * (yMin - yCenter)));

    if(erase)
      eraseHand(gDateTime.curMinHand);
    
    drawHour(gDateTime.hours, gDateTime.minutes, true);
    tft.drawLine(xCenter, yCenter, xMin, yMin, ILI9341_CYAN);  
    gDateTime.curMinHand = {xMin, yMin};
  }

  void drawSecond(int seconds){
    //Draw Second Hand
    float sRads = (float)((seconds * 6.0) * 71.0)/4068.0;
    float xSec = xCenter + radius * sin(sRads);
    float ySec = yCenter - radius * cos(sRads);
    eraseHand(gDateTime.curSecHand);
    gDateTime.curSecHand = {xSec, ySec};
    
    if(gDateTime.seconds > gDateTime.minutes -2 && gDateTime.seconds < gDateTime.minutes +2 ){
      drawMinute(gDateTime.minutes, gDateTime.seconds, false); 
    }
    
    drawHour(gDateTime.hours, gDateTime.minutes, false);

    tft.drawLine(xCenter, yCenter, xSec, ySec, ILI9341_RED);
  }

  void eraseHand(Point point){
    tft.drawLine(xCenter, yCenter, point.x, point.y, ILI9341_BLACK);
    drawClockFace();
  }
  
  /*******************************************************/
  /*Setup and Configure WiFi Connection                  */
  /*******************************************************/ 
  void setupWiFi(){
    WiFi.disconnect();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    loadWiFiConfig();
    if(DEBUG){Serial.println("Setting Up WiFi");}
    //Need to set hostname prior to DHCP connection.
    WiFi.hostname(wifiConfig.hostname);
    if(DEBUG){Serial.println("Hostname: " + String(wifiConfig.hostname));}
    //Connect to Access Point
    int MAX_ATTEMPTS = 1;
    int count = 0;
    while(WiFi.status() != WL_CONNECTED && count < MAX_ATTEMPTS && String(wifiConfig.ssid).length() > 0){
      count ++;
      if(DEBUG){Serial.println("Count: " + String(count));}
      if(DEBUG){Serial.println("Connecting to '" + String(wifiConfig.ssid) + "' w\\ Password '" + String(wifiConfig.password) + "'");}

      WiFi.begin(wifiConfig.ssid, wifiConfig.password);
      for(int j = 0; j < 10; j++) {
        if(DEBUG){Serial.print(".");}
          delay(1000);
          if(WiFi.status() == WL_CONNECTED){
            break;
          }
      }
      if(DEBUG){Serial.println("");}
      
      if(WiFi.status() != WL_CONNECTED){
        WiFi.mode(WIFI_AP);
        if(DEBUG){Serial.println("Failed connecting to '" + String(wifiConfig.ssid) + "'");}
        bool softAPConnected = WiFi.softAP("ESP8266SetupAP");
        if(DEBUG){
          if(softAPConnected){
            Serial.print("AP Started - IP Address: ");
            Serial.println(WiFi.softAPIP());
          }else{
            Serial.print("AP Failed");
          }
        }
      }else{
        if(DEBUG){
          Serial.print("Connected to WiFi " + String(wifiConfig.ssid) + " - IP Address: ");
          Serial.println(WiFi.localIP());
        }
        isConnected = true;
      }
    }
  }
  
  /*******************************************************/
  /*Check for a TCP request & attempt to respond         */
  /*******************************************************/ 
  void checkTCP(){
    //Only check for TCP requests when connect to a network
    if(WiFi.status() == WL_CONNECTED){
      WiFiClient client = JSONServer.available();
      // wait for a client to connect
      if (client)
      {
        //tft.println("\n[Client connected]");
        String data = "";
        while (client.connected())
        {
          // read line by line what the client (web browser) is requesting
          if (client.available())
          {
            String line = client.readStringUntil('\r');
            if (line == "TH")
            {  
              float temp = dht.readTemperature(true) - TEMP_OFFSET;
              data = "{\"status\" : " + String(1) + ",\"conditions\" : {\"temp\" : \"" + temp + "\", \"humid\" : \"" + dht.readHumidity() + "\"}}";;
              break;
            }else{
              data = "{\"status\":-1, \"message\":\"Unknown Request\"}";
              break;
            }
          }
        }
        //Send the response
        client.print("{\"chipID\":\"" + String(chipId) + "\", \"data\":" + data + "}");
        //Wait a moment for the requester to recevied the data
        delay(1);
        // close the connection:
        client.stop();
        //tft.println("[Client disonnected]");
      }   
    }  
  }

  /*******************************************************/
  /*HTTP Server Configuration and Response Methods       */
  /*******************************************************/ 
  void setupHTTP(){ 

    //HTML URLs
    webServer.on("/", [](){
      sendHTML(HomeHTML);
    });    
    webServer.on("/wifi", [](){
      sendHTML(WiFiConfigHTML);
    });            
    webServer.on("/config", [](){
      sendHTML(SysConfigHTML);
    });
    webServer.on("/conditions", [](){
      sendHTML(ConditionsHTML);
    });    
    webServer.on("/about", [](){
      sendHTML(AboutHTML);
    });    

    //Data Post
    webServer.on("/wifi/save/", wifiSaveRequest);
    webServer.on("/config/save/", configSaveRequest);

    //API URLs
    webServer.on("/api/getNetworks", getNetworksJSON);
    webServer.on("/api/getNetworkSettings", getNetworkSettingsJSON);
    webServer.on("/api/getDeviceDetails", getDeviceDetailsJSON);
    webServer.on("/api/getConditions", getTempSensorDataJSON);
  }
  
  void getNetworksJSON(){
    //Make sure authenticated.
    if(!webServer.authenticate(sysConfig.username, sysConfig.password))
      return webServer.requestAuthentication();
  
    int networkCount = WiFi.scanNetworks();
    String JSON = "{\"networks\":[";
    for (int i = 0; i < networkCount; i++)
    {
      if(i > 0){
        JSON += ",";
      }
      JSON += "{";
      JSON += "\"SSID\":\"" + String(WiFi.SSID(i)) + "\",";
      JSON += "\"security\":\"" + String(WiFi.encryptionType(i)) + "\",";
      JSON += "\"RSSI\":\"" + String(WiFi.RSSI(i)) + "\",";
      JSON += "\"BSSID\":\"" + WiFi.BSSIDstr(i) + "\",";
      JSON += "\"channel\":\"" + String(WiFi.channel(i)) + "\",";
      JSON += "\"isHidden\":" + String(WiFi.isHidden(i) ? "true" : "false") + ",";
      JSON += "\"selected\":" + String(String(wifiConfig.ssid) == String(WiFi.SSID(i)) ? "true" : "false");
      JSON += "}";      
    }
    JSON += "]}";
    webServer.send(200, "application/json", JSON);
  }

  void getNetworkSettingsJSON(){
    //Make sure authenticated.
    if(!webServer.authenticate(sysConfig.username, sysConfig.password))
      return webServer.requestAuthentication();
          
    String JSON = "{\"hostname\":\"" + String(wifiConfig.hostname) + "\", \"ssid\":\"" + String(wifiConfig.ssid) + "\"}";
    webServer.send(200, "application/json", JSON);
  }

  void getDeviceDetailsJSON(){
    //Make sure authenticated.
    if(!webServer.authenticate(sysConfig.username, sysConfig.password))
      return webServer.requestAuthentication();
  
    String JSON;
    JSON  = "{";
    JSON += "\"chipId\":\"" + String(ESP.getChipId()) + "\",";
    JSON += "\"deviceName\":\"" + String(sysConfig.deviceName) + "\",";
    JSON += "\"deviceLocation\":\"" + String(sysConfig.deviceLocation) + "\",";
    JSON += "\"resetReason\":\"" + String(ESP.getResetReason()) + "\",";
    JSON += "\"freeHeap\":\"" + String(ESP.getFreeHeap()) + "\",";
    JSON += "\"freeHeapUnit\":\"b\",";
    JSON += "\"flashChipId\":\"" + String(ESP.getFlashChipId()) + "\",";
    JSON += "\"flashChipSize\":\"" + String(ESP.getFlashChipSize()) + "\",";
    JSON += "\"flashChipSizeUnit\":\"b\",";
    JSON += "\"flashChipRealSize\":\"" + String(ESP.getFlashChipRealSize()) + "\",";
    JSON += "\"flashChipRealSizeUnit\":\"b\",";
    JSON += "\"flashChipFreq\":\"" + String(((float)ESP.getFlashChipSpeed() / (float)1000000.0)) + "\",";
    JSON += "\"flashChipFreqUnit\":\"MHz\",";
    JSON += "\"supplyVoltage\":\"" + String((float)ESP.getVcc() / (float)1000.0) + "\",";
    JSON += "\"supplyVoltageUnit\":\"V\"";
    JSON += "}";      
    webServer.send(200, "application/json", JSON);    
  }

  void getTempSensorDataJSON(){
    //Make sure authenticated.
    if(!webServer.authenticate(sysConfig.username, sysConfig.password))
      return webServer.requestAuthentication();
      
    float temp = dht.readTemperature(true) - TEMP_OFFSET;
    float humid = dht.readHumidity();    
    String JSON = "{\"temperature\" : " + String(temp) + ", \"humidity\" : " + String(humid) + "}";
    webServer.send(200, "application/json", JSON);
  }

  void sendHTML(const char* HTML){
     if(!webServer.authenticate(sysConfig.username, sysConfig.password))
        return webServer.requestAuthentication();
     webServer.send(200, "text/html", HTML);   
  }

  void sendCSS(const char* CSS){    
     webServer.send(200, "text/css", CSS);   
  }

  void sendJS(const char* JS){     
     webServer.send(200, "application/javascript", JS);   
  }    

  void configSaveRequest(){
    if(!webServer.authenticate(sysConfig.username, sysConfig.password))
      return webServer.requestAuthentication();


    SysConfig tempConfig;
    
    for(int i = 0; i < webServer.args(); i++){
      if(webServer.argName(i) == "DeviceName"){
        webServer.arg(i).toCharArray(tempConfig.deviceName, sizeof(tempConfig.deviceName));  
      }
      
      if(webServer.argName(i) == "DeviceLocation"){
        
      }      
      
    }



//    char deviceName[30] = "New Device";
//    char deviceLocation[60] = "New Location";
//    bool displayClock = true;
//    long interval = 10000;
//    int screenRotation = 1;
//    char username[30] = "root";
//    char password[30] = "esp8266";
//    unsigned int httpPort = 80;
//    unsigned int tcpPort = 4210;
//    bool deleteFileSystem = false;
//    
//    if(hostname.length() <= 0 || ssid.length() <= 0 || password.length() <= 0){
//      webServer.send(200, "application/json", "{\"success\":false, \"error\":\"One or more invalid configuration items.\"}");
//    }else if (saveWiFiConfig(hostname, ssid, password)) {
//      webServer.send(200, "application/json", "{\"success\":true, \"error\":\"\"}");
//      setupWiFi();
//    }else{
//      hostname.toCharArray(wifiConfig.hostname, 30);
//      ssid.toCharArray(wifiConfig.ssid, 30);
//      password.toCharArray(wifiConfig.password, 30); 
//      webServer.send(200, "application/json", "{\"success\":false, \"error\":\"Save Failed! Good until reboot!\"}");
//    }        
    
  }
  
  void wifiSaveRequest(){
    String hostname, ssid, password;
    if(!webServer.authenticate(sysConfig.username, sysConfig.password))
        return webServer.requestAuthentication();
    for(int i = 0; i < webServer.args(); i++){
      if(webServer.argName(i) == "Hostname"){
        hostname = webServer.arg(i);
        //Serial.println("Hostname: " + hostname);
      }

      if(webServer.argName(i) == "SSID"){
        ssid = webServer.arg(i);
        //Serial.println("SSID: " + ssid);
      }

      if(webServer.argName(i) == "Password"){
        password = webServer.arg(i);
        //Serial.println("Pass: " + password);
      }
    }
    
    
    if(hostname.length() <= 0 || ssid.length() <= 0 || password.length() <= 0){
      webServer.send(200, "application/json", "{\"success\":false, \"error\":\"One or more invalid configuration items.\"}");
    }else if (saveWiFiConfig(hostname, ssid, password)) {
      webServer.send(200, "application/json", "{\"success\":true, \"error\":\"\"}");
      setupWiFi();
    }else{
      hostname.toCharArray(wifiConfig.hostname, 30);
      ssid.toCharArray(wifiConfig.ssid, 30);
      password.toCharArray(wifiConfig.password, 30); 
      webServer.send(200, "application/json", "{\"success\":false, \"error\":\"Save Failed! Good until reboot!\"}");
    }    
  }
  
  /*******************************************************/
  /*WiFi Configuration                                   */
  /*******************************************************/ 
  bool saveWiFiConfig(String hostname, String ssid, String password){
    bool writeSuccess = false;
    if(hostname.length() > 0 && ssid.length() > 0 && password.length() > 0){

      if(DEBUG){Serial.println("Writting values to Config Variable");}
      hostname.toCharArray(wifiConfig.hostname, sizeof(wifiConfig.hostname));
      ssid.toCharArray(wifiConfig.ssid, sizeof(wifiConfig.ssid));
      password.toCharArray(wifiConfig.password, sizeof(wifiConfig.password));
      
      //Write config struct to EEPROM
      EEPROM.begin(512);
      delay(1);      
      writeSuccess = true;
      for (unsigned int t = 0; t <sizeof(wifiConfig); t++)
      { 
        //Write Data to EEPROM
        EEPROM.write(wifiAddressStart + t, *((char*)&wifiConfig + t));
        //Verify Write
        if (EEPROM.read(wifiAddressStart + t) != *((char*)&wifiConfig + t))
        {
          if(DEBUG){Serial.println("Error Writting value to EEPROM");}
          writeSuccess = false;
          break;
        }
      }
      EEPROM.commit();
      EEPROM.end();
    }    
    return writeSuccess;
  }

  bool loadWiFiConfig(){  
    EEPROM.begin(512);
    delay(1);    
    if(DEBUG){Serial.println("Load Config");}
    for (unsigned int t = wifiAddressStart; t < sizeof(wifiConfig); t++){
      *((char*)&wifiConfig + t) = EEPROM.read(wifiAddressStart + t);
    }
    EEPROM.end();
  }

  /*******************************************************/
  /*System Configuration                                 */
  /*******************************************************/
  bool saveSysConfig(){
    EEPROM.begin(512);
    delay(10);
    bool writeSuccess = true; 
    int startAddress = sysAddressStart;

    for(unsigned int t = 0; t < sizeof(sysConfig.deviceName); t++)
      EEPROM.write(startAddress++, *((char*)&sysConfig.deviceName + t));

    for(unsigned int t = 0; t < sizeof(sysConfig.deviceLocation); t++)
      EEPROM.write(startAddress++, *((char*)&sysConfig.deviceLocation + t));

    for(unsigned int t = 0; t < sizeof(sysConfig.displayClock); t++)
      EEPROM.write(startAddress++, *((bool*)&sysConfig.displayClock + t));

    EEPROMWriteLong(sysConfig.interval, startAddress);
    
    for(unsigned int t = 0; t < sizeof(sysConfig.screenRotation); t++)
      EEPROM.write(startAddress++, *((int*)&sysConfig.screenRotation + t)); 

    for(unsigned int t = 0; t < sizeof(sysConfig.username); t++)
      EEPROM.write(startAddress++, *((char*)&sysConfig.username + t)); 

    for(unsigned int t = 0; t < sizeof(sysConfig.password); t++)
      EEPROM.write(startAddress++, *((char*)&sysConfig.password + t)); 

    EEPROMWriteLong(sysConfig.httpPort, startAddress);
    
    EEPROMWriteLong(sysConfig.tcpPort, startAddress);

    for(unsigned int t = 0; t < sizeof(sysConfig.deleteFileSystem); t++)
      EEPROM.write(startAddress++, *((bool*)&sysConfig.deleteFileSystem + t)); 

    EEPROM.commit();
    EEPROM.end();
        
    if(DEBUG){Serial.println("Write Sys Config: " + String(writeSuccess));}
    return writeSuccess;    
  }
  
  bool saveSysConfig(bool displayClock, long interval, String username, String password, unsigned int httpPort, unsigned int tcpPort, String deviceName, String deviceLocation){
    
    sysConfig.displayClock = displayClock;
    if(interval > 1000)
      sysConfig.interval = interval;
    if(deviceName.length() > 0)
      deviceName.toCharArray(sysConfig.username, sizeof(sysConfig.deviceName));
    if(deviceLocation.length() > 0)
      deviceLocation.toCharArray(sysConfig.deviceLocation, sizeof(sysConfig.deviceLocation));
    if(username.length() > 0)
      username.toCharArray(sysConfig.username, sizeof(sysConfig.username));
    if(password.length() > 0)
      password.toCharArray(sysConfig.password, sizeof(sysConfig.password));      
    if(httpPort > 0)
       sysConfig.httpPort = httpPort;
    if(tcpPort > 0 )
      sysConfig.tcpPort = tcpPort;   
    
    return saveSysConfig();
  }

  bool loadSysConfig(){   
    EEPROM.begin(512);
    delay(10);       

    int startAddress = sysAddressStart;

    for(unsigned int t = 0; t < sizeof(sysConfig.deviceName); t++)
      *((char*)&sysConfig.deviceName + t) = EEPROM.read(startAddress++);

    for(unsigned int t = 0; t < sizeof(sysConfig.deviceLocation); t++)
      *((char*)&sysConfig.deviceLocation + t) = EEPROM.read(startAddress++);

    for(unsigned int t = 0; t < sizeof(sysConfig.displayClock); t++)
      *((bool*)&sysConfig.displayClock + t) = EEPROM.read(startAddress++);

    sysConfig.interval = EEPROMReadLong(startAddress);

    for(unsigned int t = 0; t < sizeof(sysConfig.screenRotation); t++)
      *((int*)&sysConfig.screenRotation + t) = EEPROM.read(startAddress++);

    for(unsigned int t = 0; t < sizeof(sysConfig.username); t++)
      *((char*)&sysConfig.username + t) = EEPROM.read(startAddress++);

    for(unsigned int t = 0; t < sizeof(sysConfig.password); t++)
      *((char*)&sysConfig.password + t) = EEPROM.read(startAddress++);

    sysConfig.httpPort = EEPROMReadLong(startAddress);

    sysConfig.tcpPort = EEPROMReadLong(startAddress);

    for(unsigned int t = 0; t < sizeof(sysConfig.deleteFileSystem); t++)
      *((bool*)&sysConfig.deleteFileSystem + t) = EEPROM.read(startAddress++);

    EEPROM.end();
    

    if(DEBUG){
      Serial.println("Size of Sys Config: " + String(sizeof(sysConfig)));
      Serial.println("Device Name: " + String(sysConfig.deviceName));
      Serial.println("Device Location: " + String(sysConfig.deviceLocation));
      Serial.println("Display Clock: " + String(sysConfig.displayClock));
      Serial.println("Interval: " + String(sysConfig.interval));
      Serial.println("Screen Rotation: " + String(sysConfig.screenRotation));
      Serial.println("Username: " + String(sysConfig.username));
      Serial.println("Password: " + String(sysConfig.password));
      Serial.println("HTTP Port: " + String(sysConfig.httpPort));
      Serial.println("TCP Port: " + String(sysConfig.tcpPort));
      Serial.println("Delete File System: " + String(sysConfig.deleteFileSystem));
    }
  }
    
  void EEPROMWriteLong(long value, int &address){
    byte four  = (value & 0xFF);
    byte three = ((value >> 8) & 0xFF);
    byte two   = ((value >> 16) & 0xFF);
    byte one   = ((value >> 24) & 0xFF);

    //Write the 4 bytes into the eeprom memory.
    EEPROM.write(address++, four);
    EEPROM.write(address++, three);
    EEPROM.write(address++, two);
    EEPROM.write(address++, one);    
  }

  long EEPROMReadLong(int &address){
    //Read the 4 bytes from the eeprom memory.
    long four  = EEPROM.read(address++);
    long three = EEPROM.read(address++);
    long two   = EEPROM.read(address++);
    long one   = EEPROM.read(address++);

    //Return the recomposed long by using bitshift.
    return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
  }
  
  /*******************************************************/
  /*Print conditions from the DHT22 Sensor to the screen */
  /*******************************************************/ 
  void printConditions(){
    //Only do a screen refresh when necessary
    float temp = dht.readTemperature(true);
    //The temperature in the enclouser gets a little warmer due to the electronics.
    temp -= TEMP_OFFSET;  
    
    if(temp != currentTemp && !isnan(temp)){
      currentTemp = temp;
      //Erase existing Temp and Write New Temp. -16 on y0 and +22 on the height becuase '°' has a y-Axis offset of -60.
      tft.fillRect(textOffset, middleOfTop - halfFontHeight - 16, textBoxWidth, (fontHeight+22), ILI9341_BLACK);     
      ui.drawString(textOffset, middleOfTop + halfFontHeight, String(temp) + "°");           
    }

    //Only do a screen refresh when necessary
    float humid = dht.readHumidity();
    if(humid != currentHumid && !isnan(humid)){
      currentHumid = humid;
      //Erase existing Humidity and Write New Humidity. +2 on font height to covet offset of the '%' sign
      tft.fillRect(textOffset, middleOfBottom - halfFontHeight, textBoxWidth, (fontHeight+2), ILI9341_BLACK);     
      ui.drawString(textOffset, middleOfBottom + halfFontHeight, String(humid) + "%");           
    }
  }
  /*******************************************************/
  /*Print Date / Time from remote server                 */
  /*******************************************************/   
  void printDateTime(){        
    eraseNotification(); 
    if(is7Segment){
      if(gDateTime.hours12 != curHour || gDateTime.minutes != curMin){
        tft.setFont(&URWGothicLDegree30pt8b );
        curHour = gDateTime.hours12;
        curMin = gDateTime.minutes;
        String h = gDateTime.hours12   > 9 ? String(gDateTime.hours12)   : "0" + String(gDateTime.hours12);
        String m = gDateTime.minutes > 9 ? String(gDateTime.minutes) : "0" + String(gDateTime.minutes);
        String result = h + ":" + m + " " + gDateTime.meridian;
        ui.drawString(SCREEN_WIDTH/2, SCREEN_HEIGHT/2+20, result);
      }

      if(!gDateTime.strDate.equals(curStrDate) && showDate){
        curStrDate = gDateTime.strDate;
        tft.setFont(&digital_714pt8b );
        ui.drawString(SCREEN_WIDTH/2, SCREEN_HEIGHT-10, gDateTime.strDate);
      }
    }else{
      if(!gDateTime.strDate.equals(curStrDate) && showDate){
        curStrDate = gDateTime.strDate;
        tft.fillRect(0,0, SCREEN_WIDTH, timeFontHeight, ILI9341_BLACK);
        ui.drawString(SCREEN_WIDTH/2, timeFontHeight+10, gDateTime.strDate);      
      }
  
      if(gDateTime.hours12 != curHour || gDateTime.minutes != curMin){
        tft.fillRect(0, SCREEN_HEIGHT-timeFontHeight, SCREEN_WIDTH, timeFontHeight, ILI9341_BLACK);
        ui.drawString(SCREEN_WIDTH/2, SCREEN_HEIGHT-15, gDateTime.strTime);  
      }
      
      if(gDateTime.hours12 != curHour){
        curHour = gDateTime.hours12;
        drawHour(gDateTime.hours, gDateTime.minutes, true);
      }
  
      if(gDateTime.minutes != curMin){
        curMin = gDateTime.minutes;
        drawMinute(gDateTime.minutes, gDateTime.seconds, true);
      }
  
      if(gDateTime.seconds != curSec){
        curSec = gDateTime.seconds;
        drawSecond(gDateTime.seconds);
      }
      
      tft.fillCircle(xCenter, yCenter, 4, GFX_ORANGE);
    }
    printNotification();
  }
  
  /*******************************************************/
  /*Print Notifications if any exists                    */
  /*******************************************************/  
  void printNotification(){
    if(isNotification){
        tft.setFont(&Liberation_Sans_16);
        ui.setTextAlignment(CENTER);
      if(isClock) {
        ui.drawString(SCREEN_WIDTH/2, 12, txtNotify);
        ui.setTextAlignment(CENTER);
        tft.setFont(&URWGothicLDegree30pt8b);
      }else{
        ui.drawString(SCREEN_WIDTH/2, SCREEN_HEIGHT/2+6, txtNotify);
        //Restore Layout
        ui.setTextAlignment(LEFT);
        tft.setFont(&URWGothicLDegree30pt8b );                 
      }        
      isNotification = false;
      txtNotify = "";
    }
  }

  void eraseNotification(){
    int nFontHeight = 20;
    if(isClock) {
      tft.fillRect(0, 0, SCREEN_WIDTH, nFontHeight, ILI9341_BLACK);
    }else{
      //ui.drawString(SCREEN_WIDTH/2, SCREEN_HEIGHT/2+6, txtNotify);
      tft.fillRect(0, 0, SCREEN_WIDTH, nFontHeight, ILI9341_BLACK);
      tft.drawLine(20, (SCREEN_HEIGHT/2), (SCREEN_WIDTH-20), (SCREEN_HEIGHT/2), GFX_ORANGE);
    }        
  }
  
  /*******************************************************/
  /*Print conditions from the DHT22 Sensor to the screen */
  /*******************************************************/
  void drawProgress(uint8_t percentage, String text) {
    ui.setTextAlignment(CENTER);
    ui.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
    //Clear existing text
    tft.fillRect(0, 80, 320, 35, ILI9341_BLACK);
    ui.drawString(160, 110, text);
    ui.drawProgressBar(10, 120, 300, 15, percentage, ILI9341_WHITE, ILI9341_BLUE);
    yield();
  }
  
  /*******************************************************/
  /*Erase the screen. Broke out for expabsion */
  /*******************************************************/
  void eraseScreen(){
    txtNotify = "";
    isNotification = false;
    tft.fillScreen(ILI9341_BLACK);
  }
  
  /*******************************************************/
  /*Prepare the screen for boot sequence                 */
  /*******************************************************/
  void setupScreenBoot(){
    eraseScreen();
    tft.setFont(&Liberation_Sans_16);
    tft.setRotation(sysConfig.screenRotation);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    //tft.setTextSize(2);
  }

  /*******************************************************/
  /* Prepare the screen for displaying the condition     */
  /*******************************************************/
  void setupScreenConditions(){  
    SPIFFS.begin(); 
    currentTemp = -99;
    currentHumid = -99;
    tft.setFont(&URWGothicLDegree30pt8b );
    ui.setTextAlignment(LEFT);
    tft.setTextColor(GFX_ORANGE, ILI9341_BLACK);
    
    //Clear the Screen of the progress indicator prep for conditions
    tft.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ILI9341_BLACK);
    //Boarder Around Screen; Offset by 3 px;
    tft.drawRect(3, 3, (SCREEN_WIDTH-6), (SCREEN_HEIGHT-6), GFX_ORANGE);
    //Line arcoss the middle of the screen
    tft.drawLine(20, (SCREEN_HEIGHT/2), (SCREEN_WIDTH-20), (SCREEN_HEIGHT/2), GFX_ORANGE);
    //Midpoint of the top half of the screen minus half the hight of the image. X-Offset by 10px for good looks.
    int imageHeight = 77;
    int imageWidth = 36;    
    int xOffSet = ((iconBoxWidth/2) - (imageWidth/2)) + iconOffset;
    ui.drawBmp("temperature.bmp", xOffSet, ((SCREEN_HEIGHT/4)*1)-(imageHeight/2));
    
    //Midpoint of the bottm half of the screen minus half the height of the image. X-Offset by 10px for good looks.   
    imageHeight = 67;
    imageWidth = 53;    
    xOffSet = ((iconBoxWidth/2) - (imageWidth/2)) + iconOffset;
    ui.drawBmp("humidity.bmp", xOffSet, ((SCREEN_HEIGHT/4)*3)-(imageHeight/2)); //53x67
    SPIFFS.end();
  }

  /*******************************************************/
  /* Prepare the screen for displaying the Date/Time     */
  /*******************************************************/
  void setupScreenClock(){
    curStrDate = "NA";
    curHour = -1;
    curMin = -1;
    curSec = -1;
    ui.setTextAlignment(CENTER);
    
    if(is7Segment){
      tft.setFont(&digital_740pt8b );
      tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
    }else{
      tft.setFont(&URWGothicLDegree30pt8b );
      tft.setTextColor(GFX_ORANGE, ILI9341_BLACK);
      
      //Draw the clock perimiter.
      //tft.drawCircle(xCenter, yCenter, radius, GFX_ORANGE);
      //Large Circle
      tft.fillCircle(xCenter, yCenter, radius+4, GFX_ORANGE);
      //Inner Face
      tft.fillCircle(xCenter, yCenter, radius, ILI9341_BLACK);      
      drawClockFace();          
    }
  }
  
  /*******************************************************/
  /* Draw the face of a clock on the screen              */
  /*******************************************************/
  void drawClockFace(){
    //Draw the minute dashes
    for(int i = 0; i < 60; i++){
        float theta = i*2*M_PI/60;
        float xOuter = xCenter + radius * sin(theta);
        float yOuter = yCenter - radius * cos(theta);
        float fraction = (float)(1.0);

        //Make 0/12, 3, 6, 9 longer dashed        
        if(i % 15 == 0){ //Main Hours 0/12, 3, 6, 8
          fraction = (float)(1.0/3.0);
        }else if(i % 5 == 0) { //Sub Hours 1, 2, 4, 5, 7, 8, 10, 11
          fraction = (float)(1.0/5.0);
        } else { //minutes marks
          fraction = (float)(1.0/10.0);
        }

        float xEnd = (xOuter + (fraction * (xCenter-xOuter)));
        float yEnd = (yOuter + (fraction * (yCenter-yOuter)));
        tft.drawLine(xOuter, yOuter, xEnd, yEnd, GFX_ORANGE);  
    }
  }

  /*******************************************************/
  /* Web Resources Download / Callback Function          */
  /*******************************************************/  
  void downloadResources(uint8_t percentage) {
    SPIFFS.begin();
    //char id[5];
    for (int i = 0; i < 21; i++) {
      //sprintf(id, "%02d", i);
      //tft.fillRect(0, 120, 240, 40, ILI9341_BLACK);
      webResource.downloadFile("http://littlerichele.com/images/temperature-mini-crop.bmp", "temperature.bmp", _downloadCallback);
    }
    for (int i = 0; i < 21; i++) {
      //sprintf(id, "%02d", i);
      //tft.fillRect(0, 120, 240, 40, ILI9341_BLACK);
      webResource.downloadFile("http://littlerichele.com/images/humidity-mini-crop.bmp", "humidity.bmp", _downloadCallback);
    }
    SPIFFS.end();   
  }

  void downloadCallback(String filename, int16_t bytesDownloaded, int16_t bytesTotal) {
    Serial.println(String(bytesDownloaded) + " / " + String(bytesTotal));
  
    int percentage = 100 * bytesDownloaded / bytesTotal;
    if (percentage % 5 == 0) {
      ui.drawProgressBar(10, 140, 300, 15, percentage, ILI9341_WHITE, ILI9341_GREEN);
    }
  }
  
