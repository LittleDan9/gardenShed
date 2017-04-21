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
  //#include "ESP8266WebServer.h"

  #include "ESPAsyncTCP.h"
  #include "ESPAsyncWebServer.h"
  
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
  #include "TFT_ILI9163C.h"
  #include "Gfxui.h"

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
  #include "HTML.h"
  #include "JavaScript.h"
  
  /*******************************************************/
  /*Font Includes                                        */
  /*******************************************************/  
  #include "Liberation_Sans.h"
  #include "URWGothicLDegree.h"  
  #include "Digital7.h"
    
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
    char deviceLocation[30] = "New Location";   
    int roomTemp = 70;
    bool displayClock = true;
    bool displayDate = false;
    int clockType = 0;
    long clockColor = ILI9341_CYAN;
    long clockBkgColor = ILI9341_BLACK;
    bool leadingZero = true;
    bool twentyFourHour = false;
    long interval = 10000;
    int screenRotation = 1;
    char username[30] = "root";
    char password[30] = "esp8266";
    long httpPort = 80;
    long tcpPort = 4210;
    bool deleteFileSystem = false;
    char displayDriver[10] = "ILI9341";
    long displayWidth = 320;
    long displayHeight = 240;    
  };
  
  SysConfig sysConfig;

  #define MAX_LOGIN_ATTEMPTS 3
  #define MAX_SPIFFS_NAME_LEN 30
  int loginAttempts = 0;  
  /*******************************************************/
  /*System Constants                                      */
  /*******************************************************/
  #define DEBUG true
  #define CONFIG_DEBUG true
  #define RELAY_PIN D0
  int chipId = system_get_chip_id();
  ADC_MODE(ADC_VCC); //Allows polling of Supply Voltage
  /*******************************************************/
  /*TFT                                                  */
  /*******************************************************/
  #define TFT_DC  D1
  #define TFT_CS  D2
  #define GFX_ORANGE 0xF4A8
  //#define DISPLAY_IC 9163
  #define DISPLAY_IC 9341

  #if DISPLAY_IC == 9163
    #define SCREEN_WIDTH 128
    #define SCREEN_HEIGHT 128  
    TFT_ILI9163C tft9163C = TFT_ILI9163C(TFT_CS, TFT_DC);
    Adafruit_GFX* tft = &tft9163C;
  #elif DISPLAY_IC == 9341
    #define SCREEN_WIDTH 320
    #define SCREEN_HEIGHT 240  
    Adafruit_ILI9341 tft9341 = Adafruit_ILI9341(TFT_CS, TFT_DC);
    Adafruit_GFX* tft = &tft9341;
  #endif
  GfxUi ui = GfxUi(tft);   
  
  /*******************************************************/
  /*DHT Sensor                                           */
  /*******************************************************/
  #define DHTPIN  D3
  #define DHTTYPE DHT22
  #if SCREEN_WIDTH > 128
    #define TEMP_OFFSET 8
  #else
    #define TEMP_OFFSET 16.62
  #endif
  DHT dht(DHTPIN, DHTTYPE);
  /*******************************************************/
  /*TCP / HTTP Server                                    */
  /*******************************************************/
  WiFiServer JSONServer(sysConfig.tcpPort);
  AsyncWebServer webServer(sysConfig.httpPort);
  AsyncWebSocket ws("/ws"); // access at ws://[esp ip]/ws
  AsyncEventSource events("/events"); // event source (Server-Sent events)  
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
  #if SCREEN_WIDTH > 128
    #define ICON_BOX_WIDTH 56
    #define ICON_OFFSET 20  
    #define TEXT_OFFSET 10
    #define FONT_HEIGHT 46
  #else
    #define ICON_BOX_WIDTH 0
    #define ICON_OFFSET 0
    #define TEXT_OFFSET 6
    #define FONT_HEIGHT 26
  #endif
  
  
  
  //Offset the text 10px from the icons.
  int textOffset = (ICON_OFFSET + ICON_BOX_WIDTH + TEXT_OFFSET);
  int middleOfTop = (SCREEN_HEIGHT/4)*1;
  int middleOfBottom = (SCREEN_HEIGHT/4)*3;
  int textBoxWidth = (SCREEN_WIDTH-textOffset)-6;
  int halfFontHeight = FONT_HEIGHT/2;  
  float currentTemp = -99.00;
  float currentHumid = -99.00;
  bool forceTempUpdate = true;
  bool isNotification = false;
  String txtNotify = "";
  
  /*******************************************************/
  /*Layout Helper - Clock                                */
  /*******************************************************/ 
  #define MAX_FAIL_COUNT 10
  unsigned long lastTimeGetMillis = 0;
  unsigned long lastThermostatMillis = 0;
  unsigned long lastTempReadMillis = -5000;
  unsigned long thermostatInterval = 30000; //(30 Seconds);
  int failCount = 0;
  
  bool isClock = false;
    
  float xCenter = (SCREEN_WIDTH/2.0);
  float yCenter = (SCREEN_HEIGHT/2.0);
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
  void thermostatControl();
  
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
    pinMode(RELAY_PIN, OUTPUT); 
    Serial.begin(115200);
    Serial.println("");
    if(DEBUG){      
      Serial.println("Garden Shed Conditions Server");
      Serial.println("ChipID:" + String(chipId));
    }

    //saveSysConfig();
    loadSysConfig();
    //Initialize the DHT Sensor Object and TFT Object
    SPIFFS.begin();
    dht.begin();
    tft->begin();

    runBootProcess();
    WiFi.scanNetworks(true);

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
  bool toggle = HIGH;
  void loop(void) {     
    isConnected = WiFi.status() == WL_CONNECTED;
    unsigned long currentMillis = millis();    
    if(currentMillis - lastThermostatMillis >= thermostatInterval){
      thermostatControl();        
    }    
    //Check for TCP Request or HTTP Request
    checkTCP();
    //webServer.handleClient();
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
            lastTempReadMillis = 0;
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
        if(isClock){
          eraseScreen();
          setupScreenConditions();
        }
        isClock = false;
        printConditions();
      }
    } else { //Not Connected
      if(isClock){
        //If the clock was displaying, we should end the clock and go to conditions
        isClock = false;   
        lastTempReadMillis = 0;
        eraseScreen();
        setupScreenConditions();        
      }
      //Always print the conditions so we don't just loop trying to connect.
      printConditions();
      //Attempted to Connect
      setupWiFi();          
    }
    yield();
  }
  /*******************************************************/ 
  
  void thermostatControl(){
    lastThermostatMillis = millis();
    
    do{
      delay(5);
      currentTemp = dht.readTemperature(true) - TEMP_OFFSET;
    } while (isnan(currentTemp));
    
    if(DEBUG){
      Serial.println("Temp: " + String(currentTemp));
      Serial.println("Room Temp: " + String(sysConfig.roomTemp));
    }
    if(currentTemp > sysConfig.roomTemp){
      turnHeaterOff();
    }else if(currentTemp < sysConfig.roomTemp - 1){
      turnHeaterOn();
    }
  }
  
  void turnHeaterOn(){    
    if(digitalRead(RELAY_PIN) != LOW){
      if(DEBUG){Serial.println("Turn On");}
      digitalWrite(RELAY_PIN, LOW);
    }
  }

  void turnHeaterOff(){
    if(digitalRead(RELAY_PIN) != HIGH){
      if(DEBUG){Serial.println("Turn Off");}
      digitalWrite(RELAY_PIN, HIGH);
    }
  }


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
    setupAsyncWebServer();

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
    gDateTime.strDate  += ", ";
    gDateTime.strDate  += root["year"].asString();
    gDateTime.strTime   = root["time"].asString();
    gDateTime.hours     = root["hr"];
    gDateTime.hours12   = gDateTime.hours > 12 ? gDateTime.hours - 12 : root["hr"];
    gDateTime.hours12   = gDateTime.hours12 == 0 ? 12 : gDateTime.hours12;
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
    tft->drawLine(xCenter, yCenter, xHour, yHour, ILI9341_WHITE);
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
    tft->drawLine(xCenter, yCenter, xMin, yMin, ILI9341_CYAN);  
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

    tft->drawLine(xCenter, yCenter, xSec, ySec, ILI9341_RED);
  }

  void eraseHand(Point point){
    tft->drawLine(xCenter, yCenter, point.x, point.y, ILI9341_BLACK);
    drawClockFace();
  }
  
  /*******************************************************/
  /*Setup and Configure WiFi Connection                  */
  /*******************************************************/ 
  void setupWiFi(){
    //WiFi.disconnect();
    //WiFi.softAPdisconnect(true);
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
        //tft->println("\n[Client connected]");
        String data = "";
        while (client.connected())
        {
          // read line by line what the client (web browser) is requesting
          if (client.available())
          {
            String line = client.readStringUntil('\r');
            if (line == "TH")
            {  
              float temp =  dht.readTemperature(true) - TEMP_OFFSET;
              while (isnan(temp)){ 
                delay(10);
                temp = dht.readTemperature(true) - TEMP_OFFSET;
              }
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
        //tft->println("[Client disonnected]");
      }   
    }  
  }
    
  /*******************************************************/
  /*HTTP Server Configuration and Response Methods       */
  /*******************************************************/   
  void onRequest(AsyncWebServerRequest *request){
    String url = request->url();
    url.toLowerCase();

    if(url.endsWith(".css")){
      sendCSS(request);
      return;
    }

    if(url.endsWith(".js")){
      sendJS(request);
      return;
    }    
    
    if(url.indexOf("/font") > -1){
      if(url.lastIndexOf("/") == -1){
        request->send(404);
        return;
      }      
      String filename = url.substring(url.lastIndexOf("/"));
      filename.replace("/", "");

      
      if(filename.lastIndexOf(".") == -1){
        request->send(404);
        return;
      }

      String SPIFFSName = filename;
      if(filename.lastIndexOf(".") == -1){
        request->send(404);
        return;
      }
      
      String extension = filename.substring(filename.lastIndexOf("."));                
      if(filename.length() > MAX_SPIFFS_NAME_LEN){
        SPIFFSName = filename.substring(0, MAX_SPIFFS_NAME_LEN - extension.length());
        SPIFFSName += extension;
      }

      if(!SPIFFS.exists("/" + SPIFFSName)){
        request->send(404);
        return;
      }

      extension.replace(".", "");
      String contentType = "";

      if(extension == "otf")
        contentType = "font/opentype";
      else if(extension == "svg")
        contentType = "image/svg+xml";
      else if(extension == "ttf")
        contentType = "application/x-font-ttf";
      else if(extension == "eot")
        contentType = "application/vnd.ms-fontobject";
      else if(extension == "woff")        
        contentType = "application/font-woff";
      else if(extension == "woff2")
        contentType = "application/font-woff2";
      else      
        contentType = "application/" + extension;
      
      //String output = filename + '\n' + extension + '\n' + contentType;
      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/" + SPIFFSName, contentType);
      response->addHeader("Cache-Control","public, max-age=86400");
      request->send(response);
      return;
    }else{   
      //Handle Unknown Request
      request->send(404);
      return;
    }
  }
  
  void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){}
  
  void onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){      

    bool processFile = false;
    filename.toLowerCase();
    String extension = "-1";
    if(filename.lastIndexOf(".") != -1){
      extension = filename.substring(filename.lastIndexOf("."));
      extension.replace(".", "");
    }
    
    if(   filename == "bootstrap.min.css"     || 
          filename == "bootstrap.min.js"      || 
          filename == "jquery.min.js"         || 
          filename == "jquery-ui.min.js"      ||        
          filename == "jquery-ui.min.css"      ||        
          filename == "site.min.js"           ||             
          filename == "site.min.css"          ||           
          filename == "font-awesome.min.css"  ||
          filename == "weather-icons.min.css" ||
          extension == "otf"    ||
          extension == "eot"    ||
          extension == "svg"    ||
          extension == "ttf"    ||
          extension == "woff"   ||
          extension == "woff2"  
        ){
        processFile = true;
    }else{
        processFile = false;
    }

    if(filename.length() > MAX_SPIFFS_NAME_LEN){
      if(filename.lastIndexOf(".") != -1){
        String extension = filename.substring(filename.lastIndexOf("."));
        filename = filename.substring(0, MAX_SPIFFS_NAME_LEN - extension.length());
        filename += extension;
      }else{
        processFile = false;
      }
    }

    if(processFile){
      if(!index){
        bool removed = SPIFFS.remove("/" + filename);   
        if(removed)
          Serial.println("File deleted");
        else
          Serial.println("File not deleted");
        Serial.printf("UploadStart: %s\n", filename.c_str());
      }

      File f = SPIFFS.open("/" + filename, "a+");
      if(!f){
        Serial.println("Unable to open file");
      }else{
        for(size_t i = 0; i < len; i++){
          f.write(data[i]);
        }        
      }
      
      if(final){
        Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index+len);
        Serial.print("File Size: ");
        Serial.println(f.size());
      }
      f.close();      
    }
  }
  
  void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
    //Handle WebSocket event
  }
  
  void setupAsyncWebServer(){
    ws.onEvent(onEvent);
    webServer.addHandler(&ws);    
    
    //attach AsyncEventSource
    webServer.addHandler(&events);
    
    /***********************************************/
    /* Static PROGMEM HTML                         */
    /***********************************************/
    webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      sendHTML(request, HomeHTML);
    });       
    webServer.on("/home", HTTP_GET, [](AsyncWebServerRequest *request){
      sendHTML(request, HomeHTML);
    });   
    webServer.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request){
      sendHTML(request, WiFiConfigHTML);
    });            
    webServer.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
      sendHTML(request, SysConfigHTML);
    });
    webServer.on("/js/sysConfig.js", HTTP_GET, [](AsyncWebServerRequest *request){
      sendJS(request, SysConfigJS);
    });     
    webServer.on("/conditions", HTTP_GET, [](AsyncWebServerRequest *request){
      sendHTML(request, ConditionsHTML);
    });    
    webServer.on("/about", HTTP_GET, [](AsyncWebServerRequest *request){
      sendHTML(request, AboutHTML);
    });

    /***********************************************/
    /* File Uploads                                */
    /***********************************************/
    webServer.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
      request->send(200);
    }, onUpload);


    /***********************************************/
    /* Data Post                                   */
    /***********************************************/
    webServer.on("/wifi/save", HTTP_POST, [](AsyncWebServerRequest *request){
      wifiSaveRequest(request);
    });
    
    webServer.on("/config/save", HTTP_POST, [](AsyncWebServerRequest *request){
      configSaveRequest(request);
    });


    /***********************************************/
    /* APIs                                        */
    /***********************************************/
    webServer.on("/api/getNetworks", HTTP_GET, [](AsyncWebServerRequest *request){
      getNetworksJSON(request);
    });
    
    webServer.on("/api/getNetworkSettings", HTTP_GET, [](AsyncWebServerRequest *request){ 
      getNetworkSettingsJSON(request);
    });
    
    webServer.on("/api/getDeviceDetails", HTTP_GET, [](AsyncWebServerRequest *request){ 
      getDeviceDetailsJSON(request);
    });
    webServer.on("/api/getConditions", HTTP_GET, [](AsyncWebServerRequest *request){ 
      getTempSensorDataJSON(request);
    });
    webServer.on("/api/getSysSettings", HTTP_GET, [](AsyncWebServerRequest *request){ 
      getSystemSettingsJSON(request);
    });

    
    /***********************************************/
    /* Default Handlers                            */
    /***********************************************/
    webServer.onNotFound(onRequest);
    webServer.onFileUpload(onUpload);
    webServer.onRequestBody(onBody);

    //Start the web server!
    webServer.begin();    
  }
  
  void sendHTML(AsyncWebServerRequest *request, const char* HTML){
    if(isHTTPAuthorized(request))
      request->send_P(200, "text/html", HTML);      
  }
  
  void sendJS(AsyncWebServerRequest *request, const char* JS){   
    if(!isHTTPAuthorized(request))
      return;
       
    request->send_P(200, "application/javascript", JS);    
  }

  void sendJS(AsyncWebServerRequest *request){   
    if(!isHTTPAuthorized(request))
      return;      

      String url = request->url();
      url.toLowerCase();
      if(url.lastIndexOf("/") != -1){
        String filename = url.substring(url.lastIndexOf("/"));      
        AsyncWebServerResponse *response = request->beginResponse(SPIFFS, filename, "application/javascript");
        response->addHeader("Cache-Control","public, max-age=86400");
        request->send(response);
      }else{
        request->send(404);
      }      
  }  

  void sendCSS(AsyncWebServerRequest *request, const char* CSS){ 
    if(!isHTTPAuthorized(request))
      return;
       
    request->send_P(200, "text/css", CSS);      
  }
  
  void sendCSS(AsyncWebServerRequest *request){    
    if(!isHTTPAuthorized(request))
      return;    

      String url = request->url();
      url.toLowerCase();
      if(url.lastIndexOf("/") != -1){
        String filename = url.substring(url.lastIndexOf("/"));      
        AsyncWebServerResponse *response = request->beginResponse(SPIFFS, filename, "text/css");
        response->addHeader("Cache-Control","public, max-age=86400");
        request->send(response);
      }else{
        request->send(404);
      }
  } 

  bool isHTTPAuthorized(AsyncWebServerRequest *request){    
    if(!request->authenticate(sysConfig.username, sysConfig.password))
    {
      Serial.print("Login Attempts: ");
      Serial.println(loginAttempts);
      if(loginAttempts >= MAX_LOGIN_ATTEMPTS){
        loginAttempts = 0;
        request->send(200, "text/html", FourOOneHTML); 
        return false;
      }else{
        loginAttempts ++;
        request->requestAuthentication();
        return false;
      }
    }
    return true;
  }

  void getNetworksJSON(AsyncWebServerRequest *request){
    //Make sure authenticated.    
    if(!isHTTPAuthorized(request))
      return;

    String JSON = "{\"networks\":[";
    int networkCount = WiFi.scanComplete();
    if(networkCount == -2){
        WiFi.scanNetworks(true);
    } else if(networkCount){
      for (int i = 0; i < networkCount; i++){
        if(i > 0) JSON += ",";
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
      WiFi.scanDelete();
      if(WiFi.scanComplete() == -2){
        WiFi.scanNetworks(true);
      }
    }
    JSON += "]}";
    request->send(200, "application/json", JSON);
    JSON = String();
  }

  void getNetworkSettingsJSON(AsyncWebServerRequest *request){
    //Make sure authenticated.
    if(!isHTTPAuthorized(request))
      return;
          
    String JSON = "{\"hostname\":\"" + String(wifiConfig.hostname) + "\", \"ssid\":\"" + String(wifiConfig.ssid) + "\"}";
    request->send(200, "application/json", JSON);
  }

  void getDeviceDetailsJSON(AsyncWebServerRequest *request){
    //Make sure authenticated.
    if(!isHTTPAuthorized(request))
      return;
  
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
    request->send(200, "application/json", JSON);
  }

  void getSystemSettingsJSON(AsyncWebServerRequest *request){
    if(!isHTTPAuthorized(request))
      return;
          
    String JSON = "{";
    JSON += "\"deviceName\":\"" + String(sysConfig.deviceName) + "\", ";
    JSON += "\"deviceLocation\":\"" + String(sysConfig.deviceLocation) + "\", ";
    JSON += "\"displayClock\":" + String(sysConfig.displayClock ? "true" : "false") + ", ";
    JSON += "\"displayDate\":" + String(sysConfig.displayDate ? "true" : "false") + ", ";
    JSON += "\"clockType\":" + String(sysConfig.clockType) + ", ";
    JSON += "\"clockColor\":" + String(sysConfig.clockColor) + ", ";
    JSON += "\"clockBackgroundColor\":" + String(sysConfig.clockBkgColor) + ", ";
    JSON += "\"leadingZero\":" +  String(sysConfig.leadingZero ? "true" : "false") + ", ";
    JSON += "\"twentyFourHour\":" + String(sysConfig.twentyFourHour ? "true" : "false") + ", ";
    JSON += "\"interval\":" + String(sysConfig.interval / 1000) + ", ";
    JSON += "\"screenRotation\":" + String(sysConfig.screenRotation) + ", ";
    JSON += "\"username\":\"" + String(sysConfig.username) + "\", ";
    JSON += "\"httpPort\":" + String(sysConfig.httpPort) + ", ";
    JSON += "\"tcpPort\":" + String(sysConfig.tcpPort) + ", ";
    JSON += "\"roomTemperature\":" + String(sysConfig.roomTemp) + ", ";
    JSON += "\"deleteFileSystem\":" + String(sysConfig.deleteFileSystem ? "true" : "false") + ", ";
    JSON += "\"displayDriver\":\"" + String(sysConfig.displayDriver) + "\", ";
    JSON += "\"displayWidth\":" + String(sysConfig.displayWidth) + ", ";
    JSON += "\"displayHeight\":" + String(sysConfig.displayHeight);
    JSON += "}";
    //Serial.println(JSON);
    request->send(200, "application/json", JSON);
  }

  void getTempSensorDataJSON(AsyncWebServerRequest *request){
    //Make sure authenticated.
    if(!isHTTPAuthorized(request))
      return;    
    
    String JSON = "{\"temperature\" : " + String(currentTemp) + ", \"humidity\" : " + String(currentHumid) + "}";    
    request->send(200, "application/json", JSON);
  }
  
  void configSaveRequest(AsyncWebServerRequest *request){
    //bool updateScreen = false;
    
    if(!isHTTPAuthorized(request))
      return;

    int args = request->args();
    for(int i = 0; i < args; i++){
      if(request->argName(i) == "DeviceName"){
        request->arg(i).toCharArray(sysConfig.deviceName, sizeof(sysConfig.deviceName));
      }
      if(request->argName(i) == "DeviceLocation"){
        request->arg(i).toCharArray(sysConfig.deviceLocation, sizeof(sysConfig.deviceLocation));
      }
      if(request->argName(i) == "Username" && request->arg(i).length() > 0){
        request->arg(i).toCharArray(sysConfig.username, sizeof(sysConfig.username));
      }
      if(request->argName(i) == "Password" && request->arg(i).length() > 0){
        request->arg(i).toCharArray(sysConfig.password, sizeof(sysConfig.password));
      }
      if(request->argName(i) == "Interval"){
        char temp[10];
        request->arg(i).toCharArray(temp, 10);
        sysConfig.interval = (atol(temp) * 1000);
      }
      if(request->argName(i) == "HTTPPort"){
        char temp[10];
        request->arg(i).toCharArray(temp, 10);
        sysConfig.httpPort = atol(temp);
      }
      if(request->argName(i) == "TCPPort"){
        char temp[10];
        request->arg(i).toCharArray(temp, 10);
        sysConfig.tcpPort = atol(temp);
      }
      if(request->argName(i) == "ScreenRotation"){
        int temp = request->arg(i).toInt();
        if(temp != sysConfig.screenRotation){
          sysConfig.screenRotation = temp;
          tft->setRotation(sysConfig.screenRotation);
        }
      }
      if(request->argName(i) == "DeleteFileSystem"){
        if(request->arg(i) == "true")
          sysConfig.deleteFileSystem = true;
        else
          sysConfig.deleteFileSystem = false;
      }  
      if(request->argName(i) == "DisplayClock"){
        if(request->arg(i) == "true")
          sysConfig.displayClock = true;
        else{
          sysConfig.displayClock = false;
        }
      }  
      if(request->argName(i) == "DisplayDate"){
        if(request->arg(i) == "true")
          sysConfig.displayDate = true;
        else{
          sysConfig.displayDate = false;
        }
      }  
      if(request->argName(i) == "RoomTemperature"){
        char temp[10];
        request->arg(i).toCharArray(temp, 10);
        sysConfig.roomTemp = atoi(temp);
      }      
      if(request->argName(i) == "ClockType"){
        char temp[10];
        request->arg(i).toCharArray(temp, 10);
        sysConfig.clockType = atoi(temp);
      }      
      if(request->argName(i) == "ClockColor"){
        char temp[16];
        request->arg(i).toCharArray(temp, 10);
        sysConfig.clockColor = atol(temp);
      }      
      if(request->argName(i) == "ClockBackgroundColor"){
        char temp[16];
        request->arg(i).toCharArray(temp, 10);
        sysConfig.clockBkgColor = atol(temp);
      }  
      if(request->argName(i) == "LeadingZero"){
        if(request->arg(i) == "true")
          sysConfig.leadingZero = true;
        else{
          sysConfig.leadingZero = false;
        }
      }  
      if(request->argName(i) == "TwentyFourHour"){
        //bool curVal = sysConfig.twentyFourHour;
        if(request->arg(i) == "true")
          sysConfig.twentyFourHour = true;
        else{
          sysConfig.twentyFourHour = false;
        }
      }

      if(request->argName(i) == "Interval"){
        char temp[10];
        request->arg(i).toCharArray(temp, 10);
        sysConfig.interval = (atol(temp) * 1000);
      }

      if(request->argName(i) == "DisplayDriver"){
        request->arg(i).toCharArray(sysConfig.displayDriver, sizeof(sysConfig.displayDriver));
      }

      if(request->argName(i) == "DisplayWidth"){
        char temp[10];
        request->arg(i).toCharArray(temp, 10);
        sysConfig.displayWidth = (atol(temp));
      }

      if(request->argName(i) == "DisplayHeight"){
        char temp[10];
        request->arg(i).toCharArray(temp, 10);
        sysConfig.displayHeight = (atol(temp));
      }
    }
    
    if(saveSysConfig()){
      request->send(200, "application/json", "{\"success\":true, \"error\":\"\"}");      
    }else{
      request->send(200, "application/json", "{\"success\":false, \"error\":\"Save Failed! Good until reboot!\"}");      
    }
    
    if(CONFIG_DEBUG){
      Serial.println("Config Recevied");
      Serial.println("Size of Sys Config: " + String(sizeof(sysConfig)));
      Serial.println("Device Name: " + String(sysConfig.deviceName));
      Serial.println("Device Location: " + String(sysConfig.deviceLocation));
      Serial.println("Room Temperature: " + String(sysConfig.roomTemp));
      Serial.println("Display Clock: " + String(sysConfig.displayClock));
      Serial.println("Clock Time: " + String(sysConfig.clockType));
      Serial.println("Display Date: " + String(sysConfig.displayDate));
      Serial.println("Clock Color: " + String(sysConfig.clockColor));
      Serial.println("Clock Background Color: " + String(sysConfig.clockBkgColor));
      Serial.println("Leading Zero: " + String(sysConfig.leadingZero));
      Serial.println("24 Hour: " + String(sysConfig.twentyFourHour));
      Serial.println("Interval: " + String(sysConfig.interval));
      Serial.println("Screen Rotation: " + String(sysConfig.screenRotation));
      Serial.println("Username: " + String(sysConfig.username));
      Serial.println("Password: " + String(sysConfig.password));
      Serial.println("HTTP Port: " + String(sysConfig.httpPort));
      Serial.println("TCP Port: " + String(sysConfig.tcpPort));
      Serial.println("Delete File System: " + String(sysConfig.deleteFileSystem));
      Serial.println("Display Driver: " + String(sysConfig.displayDriver));
      Serial.println("Display Width: " + String(sysConfig.displayWidth));
      Serial.println("Display Height: " + String(sysConfig.displayHeight)); 
    }        
  }
 
  void wifiSaveRequest(AsyncWebServerRequest *request){
    String hostname, ssid, password;
    if(!isHTTPAuthorized(request))
      return;

    int args = request->args();
    for(int i = 0; i < args; i++){
      if(request->argName(i) == "Hostname"){
        hostname = request->arg(i);
        //Serial.println("Hostname: " + hostname);
      }
      if(request->argName(i) == "SSID"){
        ssid = request->arg(i);
        //Serial.println("SSID: " + ssid);
      }
      if(request->argName(i) == "Password"){
        password = request->arg(i);
        //Serial.println("Pass: " + password);
      }
    }
    
    if(hostname.length() <= 0 || ssid.length() <= 0 || password.length() <= 0){
      request->send(200, "application/json", "{\"success\":false, \"error\":\"One or more invalid configuration items.\"}");
    }else if (saveWiFiConfig(hostname, ssid, password)) {
      request->send(200, "application/json", "{\"success\":true, \"error\":\"\"}");
      setupWiFi();
    }else{
      hostname.toCharArray(wifiConfig.hostname, 30);
      ssid.toCharArray(wifiConfig.ssid, 30);
      password.toCharArray(wifiConfig.password, 30); 
      request->send(200, "application/json", "{\"success\":false, \"error\":\"Save Failed! Good until reboot!\"}");
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

    for(unsigned int t = 0; t < sizeof(sysConfig.roomTemp); t++)
      EEPROM.write(startAddress++, *((int*)&sysConfig.roomTemp + t));       

    for(unsigned int t = 0; t < sizeof(sysConfig.displayClock); t++)
      EEPROM.write(startAddress++, *((bool*)&sysConfig.displayClock + t));

    for(unsigned int t = 0; t < sizeof(sysConfig.displayDate); t++)
      EEPROM.write(startAddress++, *((bool*)&sysConfig.displayDate + t));      

    for(unsigned int t = 0; t < sizeof(sysConfig.clockType); t++)
      EEPROM.write(startAddress++, *((int*)&sysConfig.clockType + t));                               

    EEPROMWriteLong(sysConfig.clockColor, startAddress);
    
    EEPROMWriteLong(sysConfig.clockBkgColor, startAddress); 

    for(unsigned int t = 0; t < sizeof(sysConfig.leadingZero); t++)
      EEPROM.write(startAddress++, *((bool*)&sysConfig.leadingZero + t));    

    for(unsigned int t = 0; t < sizeof(sysConfig.twentyFourHour); t++)
      EEPROM.write(startAddress++, *((bool*)&sysConfig.twentyFourHour + t));             
    
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

    for(unsigned int t = 0; t < sizeof(sysConfig.displayDriver); t++)
      EEPROM.write(startAddress++, *((char*)&sysConfig.displayDriver + t));  

    EEPROMWriteLong(sysConfig.displayWidth, startAddress);
    
    EEPROMWriteLong(sysConfig.displayHeight, startAddress);
                 

    EEPROM.commit();
    EEPROM.end();
        
    if(CONFIG_DEBUG){
      Serial.println("System Config Write");
      Serial.println("Write Sys Config: " + String(writeSuccess));
      Serial.println("Size of Sys Config: " + String(sizeof(sysConfig)));
      Serial.println("Device Name: " + String(sysConfig.deviceName));
      Serial.println("Device Location: " + String(sysConfig.deviceLocation));
      Serial.println("Room Temperature: " + String(sysConfig.roomTemp));
      Serial.println("Display Clock: " + String(sysConfig.displayClock));
      Serial.println("Clock Time: " + String(sysConfig.clockType));
      Serial.println("Display Date: " + String(sysConfig.displayDate));
      Serial.println("Clock Color: " + String(sysConfig.clockColor));
      Serial.println("Clock Background Color: " + String(sysConfig.clockBkgColor));
      Serial.println("Leading Zero: " + String(sysConfig.leadingZero));
      Serial.println("24 Hour: " + String(sysConfig.twentyFourHour));
      Serial.println("Interval: " + String(sysConfig.interval));
      Serial.println("Screen Rotation: " + String(sysConfig.screenRotation));
      Serial.println("Username: " + String(sysConfig.username));
      Serial.println("Password: " + String(sysConfig.password));
      Serial.println("HTTP Port: " + String(sysConfig.httpPort));
      Serial.println("TCP Port: " + String(sysConfig.tcpPort));
      Serial.println("Delete File System: " + String(sysConfig.deleteFileSystem));
      Serial.println("Display Driver: " + String(sysConfig.displayDriver));
      Serial.println("Display Width: " + String(sysConfig.displayWidth));
      Serial.println("Display Height: " + String(sysConfig.displayHeight));          
    }         
    return writeSuccess;    
  }

  bool loadSysConfig(){   
    EEPROM.begin(512);
    delay(10);       
    if(DEBUG){
      Serial.println("Load Sys Config");
      Serial.println(sizeof(WiFiConfig) + sizeof(sysConfig));
    }
    
    int startAddress = sysAddressStart;

    for(unsigned int t = 0; t < sizeof(sysConfig.deviceName); t++)
      *((char*)&sysConfig.deviceName + t) = EEPROM.read(startAddress++);

    for(unsigned int t = 0; t < sizeof(sysConfig.deviceLocation); t++)
      *((char*)&sysConfig.deviceLocation + t) = EEPROM.read(startAddress++);

    for(unsigned int t = 0; t < sizeof(sysConfig.roomTemp); t++)
      *((int*)&sysConfig.roomTemp + t) = EEPROM.read(startAddress++);        

    for(unsigned int t = 0; t < sizeof(sysConfig.displayClock); t++)
      *((bool*)&sysConfig.displayClock + t) = EEPROM.read(startAddress++);

    for(unsigned int t = 0; t < sizeof(sysConfig.displayDate); t++)
      *((bool*)&sysConfig.displayDate + t) = EEPROM.read(startAddress++);      

    for(unsigned int t = 0; t < sizeof(sysConfig.clockType); t++)
      *((int*)&sysConfig.clockType + t) = EEPROM.read(startAddress++);   

    sysConfig.clockColor = EEPROMReadLong(startAddress);  
    
    sysConfig.clockBkgColor = EEPROMReadLong(startAddress);  

    for(unsigned int t = 0; t < sizeof(sysConfig.leadingZero); t++)
      *((bool*)&sysConfig.leadingZero + t) = EEPROM.read(startAddress++);  

    for(unsigned int t = 0; t < sizeof(sysConfig.twentyFourHour); t++)
      *((bool*)&sysConfig.twentyFourHour + t) = EEPROM.read(startAddress++);          

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

    for(unsigned int t = 0; t < sizeof(sysConfig.displayDriver); t++)
      *((char*)&sysConfig.displayDriver + t) = EEPROM.read(startAddress++);     
       
    sysConfig.displayWidth = EEPROMReadLong(startAddress);

    sysConfig.displayHeight = EEPROMReadLong(startAddress);                 

    EEPROM.end();
    

    if(CONFIG_DEBUG){
      Serial.println("System Config Load");
      Serial.println("Size of Sys Config: " + String(sizeof(sysConfig)));
      Serial.println("Device Name: " + String(sysConfig.deviceName));
      Serial.println("Room Temperature: " + String(sysConfig.roomTemp));
      Serial.println("Device Location: " + String(sysConfig.deviceLocation));
      Serial.println("Display Clock: " + String(sysConfig.displayClock));
      Serial.println("Clock Type: " + String(sysConfig.clockType));
      Serial.println("Clock Color: " + String(sysConfig.clockColor));
      Serial.println("Clock Background Color: " + String(sysConfig.clockBkgColor));
      Serial.println("Leading Zero: " + String(sysConfig.leadingZero));      
      Serial.println("24 Hour: " + String(sysConfig.twentyFourHour));            
      Serial.println("Display Clock: " + String(sysConfig.displayDate));      
      Serial.println("Interval: " + String(sysConfig.interval));
      Serial.println("Screen Rotation: " + String(sysConfig.screenRotation));
      Serial.println("Username: " + String(sysConfig.username));
      Serial.println("Password: " + String(sysConfig.password));
      Serial.println("HTTP Port: " + String(sysConfig.httpPort));
      Serial.println("TCP Port: " + String(sysConfig.tcpPort));
      Serial.println("Delete File System: " + String(sysConfig.deleteFileSystem));
      Serial.println("Display Driver: " + String(sysConfig.displayDriver));
      Serial.println("Display Width: " + String(sysConfig.displayWidth));
      Serial.println("Display Height: " + String(sysConfig.displayHeight));          
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
    //Read temp only every 5 Seconds
    unsigned long currentMillis = millis();
    if(currentMillis - lastTempReadMillis > 5000)
    { 
      Serial.println("Read Temp");   
      lastTempReadMillis = currentMillis;
      //Only do a screen refresh when necessary
      float temp = dht.readTemperature(true) - TEMP_OFFSET;
      float humid = dht.readHumidity();
            
      if(SCREEN_WIDTH > 128){
        printConditionsLarge(temp, humid);
      }else{
        if(!isnan(humid))
          currentHumid = humid;
        printConditionsSmall(temp);
      }
    }
  }
  
  void printConditionsSmall(float temp){
    tft->setFont(&URWGothicLBook18pt8b);
    ui.setTextAlignment(CENTER);
    
    if(temp != currentTemp && !isnan(temp)){
      currentTemp = temp;
      //Erase existing Temp and Write New Temp. -16 on y0 and +22 on the height becuase '°' has a y-Axis offset of -60.
      tft->fillRect(6, 6, SCREEN_WIDTH-12, SCREEN_HEIGHT-12, ILI9341_BLACK);     
      ui.drawString((SCREEN_WIDTH/2), (SCREEN_HEIGHT/2)+10, String(temp) + "°");
    }
  }
  
  void printConditionsLarge(float temp, float humid){
    tft->setFont(&URWGothicLDegree30pt8b );
    if((temp != currentTemp || forceTempUpdate) && !isnan(temp)){
      currentTemp = temp;
      //Erase existing Temp and Write New Temp. -16 on y0 and +22 on the height becuase '°' has a y-Axis offset of -60.
      tft->fillRect(textOffset, middleOfTop - halfFontHeight - 16, textBoxWidth, (FONT_HEIGHT+22), ILI9341_BLACK);     
      ui.drawString(textOffset, middleOfTop + halfFontHeight, String(temp) + "°");           
    }
    
    //Only do a screen refresh when necessary
    if((humid != currentHumid || forceTempUpdate) && !isnan(humid)){
      currentHumid = humid;
      //Erase existing Humidity and Write New Humidity. +2 on font height to covet offset of the '%' sign
      tft->fillRect(textOffset, middleOfBottom - halfFontHeight, textBoxWidth, (FONT_HEIGHT+2), ILI9341_BLACK);     
      ui.drawString(textOffset, middleOfBottom + halfFontHeight, String(humid) + "%");           
    }
  }
  /*******************************************************/
  /*Print Date / Time from remote server                 */
  /*******************************************************/   
  void printDateTime(){
    if(sysConfig.clockType == 1 && (gDateTime.hours12 != curHour || gDateTime.minutes != curMin)){
        curHour = gDateTime.hours12;
        curMin = gDateTime.minutes;      
                    
        String h = "";
        int hours = sysConfig.twentyFourHour ? gDateTime.hours : gDateTime.hours12;
        if(sysConfig.leadingZero || sysConfig.twentyFourHour)
          h = hours  > 9 ? String(hours) : "0" + String(hours);
        else
          h = String(hours);
        
        String m = gDateTime.minutes > 9 ? String(gDateTime.minutes) : "0" + String(gDateTime.minutes);
        String result = h + ":" + m;
        
        
        if(!sysConfig.twentyFourHour)
          result += " " + gDateTime.meridian;
        
        tft->setFont(&digital_740pt8b );      
        tft->fillRect(0, ((SCREEN_HEIGHT/2)-50), SCREEN_WIDTH, 70, ILI9341_BLACK);
        ui.drawString(SCREEN_WIDTH/2, SCREEN_HEIGHT/2+10, result);
      
      //Display Date
      if(!gDateTime.strDate.equals(curStrDate) && sysConfig.displayDate){
        curStrDate = gDateTime.strDate;
        tft->setFont(&digital_714pt8b );
        ui.drawString(SCREEN_WIDTH/2, SCREEN_HEIGHT-60, gDateTime.strDate);
      }
    }
    
    if(sysConfig.clockType == 0){
      //Serial.println("Date Check");
      if(!gDateTime.strDate.equals(curStrDate) && sysConfig.displayDate){
        curStrDate = gDateTime.strDate;
        tft->setFont(&URWGothicLDegree14pt8b);
        ui.drawString(SCREEN_WIDTH/2, 28, gDateTime.strDate);
        tft->setFont(&URWGothicLDegree30pt8b);
      }

      //Serial.println("Print Start Time");
      if(gDateTime.hours12 != curHour || gDateTime.minutes != curMin){
        tft->fillRect(0, SCREEN_HEIGHT-timeFontHeight, SCREEN_WIDTH, timeFontHeight, ILI9341_BLACK);
        //-16
        int printY = SCREEN_HEIGHT;
        if(sysConfig.displayDate)
          printY -= 4;
        else
          printY -= 16;
          
        ui.drawString(SCREEN_WIDTH/2, printY, gDateTime.strTime);  
      }

      //Serial.println("Draw Hour");
      if(gDateTime.hours12 != curHour){
        curHour = gDateTime.hours12;
        drawHour(gDateTime.hours, gDateTime.minutes, true);
      }

      //Serial.println("Draw Minute");
      if(gDateTime.minutes != curMin){
        curMin = gDateTime.minutes;
        drawMinute(gDateTime.minutes, gDateTime.seconds, true);
      }

      //Serial.println("Draw Second");
      if(gDateTime.seconds != curSec){
        curSec = gDateTime.seconds;
        drawSecond(gDateTime.seconds);
      }
    }
    printNotification();
  }
  
  /*******************************************************/
  /*Print Notifications if any exists                    */
  /*******************************************************/  
  void printNotification(){
    if(isNotification){
        tft->setFont(&Liberation_Sans_16);
        ui.setTextAlignment(CENTER);
      if(isClock) {
        ui.drawString(SCREEN_WIDTH/2, 12, txtNotify);
        ui.setTextAlignment(CENTER);
        tft->setFont(&URWGothicLDegree30pt8b);
      }else{
        ui.drawString(SCREEN_WIDTH/2, SCREEN_HEIGHT/2+6, txtNotify);
        //Restore Layout
        ui.setTextAlignment(LEFT);
        tft->setFont(&URWGothicLDegree30pt8b );                 
      }        
      isNotification = false;
      txtNotify = "";
    }
  }

  void eraseNotification(){
    int nFontHeight = 20;
    if(isClock) {
      tft->fillRect(0, 0, SCREEN_WIDTH, nFontHeight, ILI9341_BLACK);
    }else{
      //ui.drawString(SCREEN_WIDTH/2, SCREEN_HEIGHT/2+6, txtNotify);
      tft->fillRect(0, 0, SCREEN_WIDTH, nFontHeight, ILI9341_BLACK);
      tft->drawLine(20, (SCREEN_HEIGHT/2), (SCREEN_WIDTH-20), (SCREEN_HEIGHT/2), GFX_ORANGE);
    }        
  }
  
  /*******************************************************/
  /*Print conditions from the DHT22 Sensor to the screen */
  /*******************************************************/
  void drawProgress(uint8_t percentage, String text) {
    int barWidth = SCREEN_WIDTH - 20;
    int barHeight = 15;
    ui.setTextAlignment(CENTER);
    ui.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
    if(SCREEN_WIDTH == 128 && SCREEN_HEIGHT == 128){
      int imgHeight = 100;
      tft->fillRect(0, imgHeight, SCREEN_WIDTH, SCREEN_HEIGHT-imgHeight, ILI9341_BLACK);
      ui.drawString((SCREEN_WIDTH/2), imgHeight+7, text);
      ui.drawProgressBar(10, SCREEN_HEIGHT-barHeight-3, barWidth, barHeight, percentage, ILI9341_WHITE, ILI9341_BLUE);
      yield();
      delay(500);
    }else{       
      //Clear existing text
      tft->fillRect(0, 0, SCREEN_WIDTH, (SCREEN_HEIGHT/2)-(barHeight/2), ILI9341_BLACK);
      //Draw the Text
      ui.drawString((SCREEN_WIDTH/2), (SCREEN_HEIGHT/2)-barHeight, text);
      //Draw the progress bar
      ui.drawProgressBar(10, (SCREEN_HEIGHT/2)-(barHeight/2), barWidth, barHeight, percentage, ILI9341_WHITE, ILI9341_BLUE);
    }
  }
  
  /*******************************************************/
  /*Erase the screen. Broke out for expabsion */
  /*******************************************************/
  void eraseScreen(){
    txtNotify = "";
    isNotification = false;
    tft->fillScreen(ILI9341_BLACK);
  }
  
  /*******************************************************/
  /*Prepare the screen for boot sequence                 */
  /*******************************************************/
  void setupScreenBoot(){
    tft->setRotation(sysConfig.screenRotation);
    tft->setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    
    eraseScreen();
    switch(SCREEN_WIDTH){
      case 240:
      case 320:
        tft->setFont(&Liberation_Sans_16);
        break;
      case 128:
        ui.drawBmp("chicken.bmp", 0, 0);
        yield();
        delay(2000);
        tft->setFont(&Liberation_Sans_8);
        break;
      default:
        tft->setFont(&Liberation_Sans_16);
        break; 
    }
  }

  /*******************************************************/
  /* Prepare the screen for displaying the condition     */
  /*******************************************************/
  void setupScreenConditions(){
    forceTempUpdate = true;
    ui.setTextAlignment(LEFT);
    //tft->setTextColor(GFX_ORANGE, ILI9341_BLACK);
    ui.setTextColor(GFX_ORANGE, ILI9341_BLACK);
    //Clear the Screen of the progress indicator prep for conditions
    tft->fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ILI9341_BLACK);
    //Boarder Around Screen; Offset by 3 px;
    tft->drawRect(3, 3, (SCREEN_WIDTH-6), (SCREEN_HEIGHT-6), GFX_ORANGE);
    if(SCREEN_WIDTH > 128){      
      //Line arcoss the middle of the screen
      tft->drawLine(20, (SCREEN_HEIGHT/2), (SCREEN_WIDTH-20), (SCREEN_HEIGHT/2), GFX_ORANGE);
      //Midpoint of the top half of the screen minus half the hight of the image. X-Offset by 10px for good looks. 
      int imageHeight = 77;
      int imageWidth = 36;    
      int xOffSet = ((ICON_BOX_WIDTH/2) - (imageWidth/2)) + ICON_OFFSET;
      ui.drawBmp("temperature.bmp", xOffSet, ((SCREEN_HEIGHT/4)*1)-(imageHeight/2));
      //Midpoint of the bottm half of the screen minus half the height of the image. X-Offset by 10px for good looks.   
      imageHeight = 67;
      imageWidth = 53;    
      xOffSet = ((ICON_BOX_WIDTH/2) - (imageWidth/2)) + ICON_OFFSET;
      ui.drawBmp("humidity.bmp", xOffSet, ((SCREEN_HEIGHT/4)*3)-(imageHeight/2)); //53x67
    }
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
    if(sysConfig.clockType == 1){
      tft->fillScreen(sysConfig.clockBkgColor);
      tft->setFont(&digital_740pt8b);
      ui.setTextColor(sysConfig.clockColor, sysConfig.clockBkgColor);
      //tft->setTextColor(sysConfig.clockColor, sysConfig.clockBkgColor);
    }
    
    if(sysConfig.clockType == 0) {
      if(sysConfig.displayDate){
        yCenter = (SCREEN_HEIGHT/2.0) -10; 
      }else{
        yCenter = (SCREEN_HEIGHT/2.0) -25; 
      }      
      tft->setFont(&URWGothicLDegree30pt8b );
      ui.setTextColor(GFX_ORANGE, ILI9341_BLACK);
      //tft->setTextColor(GFX_ORANGE, ILI9341_BLACK);
      
      //Draw the clock perimiter.
      //tft->drawCircle(xCenter, yCenter, radius, GFX_ORANGE);
      //Large Circle
      tft->fillCircle(xCenter, yCenter, radius+4, GFX_ORANGE);
      //Inner Face
      tft->fillCircle(xCenter, yCenter, radius, ILI9341_BLACK);      
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
        tft->drawLine(xOuter, yOuter, xEnd, yEnd, GFX_ORANGE);  
    }
  }

  /*******************************************************/
  /* Web Resources Download / Callback Function          */
  /*******************************************************/  
  void downloadResources(uint8_t percentage) {
    if(SCREEN_WIDTH > 128 || SCREEN_HEIGHT > 128){
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
    } else {
      for (int i = 0; i < 21; i++) {
        //sprintf(id, "%02d", i);
        //tft.fillRect(0, 120, 240, 40, ILI9341_BLACK);
        webResource.downloadFile("http://littlerichele.com/images/chicken.bmp", "chicken.bmp", _downloadCallback);
      }
    }
  }

  void downloadCallback(String filename, int16_t bytesDownloaded, int16_t bytesTotal) {
    Serial.println(String(bytesDownloaded) + " / " + String(bytesTotal));
  
    int percentage = 100 * bytesDownloaded / bytesTotal;
    if (percentage % 5 == 0) {
      ui.drawProgressBar(10, 140, 300, 15, percentage, ILI9341_WHITE, ILI9341_GREEN);
    }
  }

  
