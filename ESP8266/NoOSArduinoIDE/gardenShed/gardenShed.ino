  /*******************************************************/
  /*Includes                                             */
  /*******************************************************/
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>  
  #include <DHT.h>  
  #include "SPI.h"
  #include "Adafruit_GFX.h"
  #include "Adafruit_ILI9341.h"
  #include "GfxUi.h"
  #include "WebResource.h"
  #include <ArduinoJson.h>  
  #include <XPT2046_Touchscreen.h>
  #ifdef ESP8266
  extern "C" {
  #include "user_interface.h"
  }
  #endif  
  /*******************************************************/
  /*Fonts                                                */
  /*******************************************************/  
  #include "Liberation_Sqns_16.h"
  #include "URWGothicLDegree_30.h"
  #include "URWGothicLDegree_20.h"
  #include "Digital7_40.h"
  #include "Digital7_14.h"
<<<<<<< HEAD
=======
  /*******************************************************/
  /*Configuration                                        */
  /*******************************************************/
  #include "WiFiConfig.h"
>>>>>>> 12eaa29c7b3103f1ca2ff644f911dedb713daf9a
  /*******************************************************/
  /*System Constans                                      */
  /*******************************************************/
  #define DEBUG true
  int chipId = system_get_chip_id();
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
  /*XPT2046 Touch Settings                               */
  /*******************************************************/
  //#define CS_PIN D8
  //XPT2046_Touchscreen ts(CS_PIN);
  /*******************************************************/
  /*DHT Sensor                                           */
  /*******************************************************/
  #define DHTPIN  D3
  #define DHTTYPE DHT22
  DHT dht(DHTPIN, DHTTYPE);
  /*******************************************************/
  /*WiFI                                                 */
  /*******************************************************/
  const char* ssid = SSID;
  const char* password = WIFI_PASS;
  const char* localHostname = HOSTNAME;
  /*******************************************************/
  /*TCP Server                                           */
  /*******************************************************/
  unsigned int localTcpPort = 4210;
  WiFiServer server(localTcpPort);
  /*******************************************************/
  /*Downloader                                           */
  /*******************************************************/
  WebResource webResource;
  /*******************************************************/
  /*Program Control                                      */
  /*******************************************************/
  unsigned long previousMillis = 0;
  unsigned long prevTFTMillis = 0;
  //Refresh the screen every 15 seconds.
  const long interval = 15000;
  const long tftOff = 4000;
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
  /*******************************************************/
  /*Layout Helper - Clock                                */
  /*******************************************************/ 
  bool isClock = true;
  bool is7Segment = false;
  bool showDate = false;
  bool reprintHourHand = false;
  bool reprintMinHand = false;
  float xCenter = (SCREEN_WIDTH/2.0);
  float yCenter = (SCREEN_HEIGHT/2.0)-25;
  int timeFontHeight = 30;
  int radius = ((SCREEN_HEIGHT-(timeFontHeight*3))/2);
  int curMin = -1;
  int curHour = -1;
  String curStrDate = "NA";
  struct point {
    float x;
    float y;
  }; typedef point Point; 
   
  struct datetime {
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
  }; typedef struct datetime DateTime;
  
  DateTime gDateTime;
 
  
  /*******************************************************/
  /*Function Prototypes                                  */
  /*******************************************************/
  //Network & TCP Server
  void setupWiFi();
  void checkTCP();
  
  //Application Setup & Control
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
  void drawHour(int hours, int minutes);
  void drawMinute(int minutes, int seconds);
  void drawSecond(int seconds);
  void drawClockFace();
  void setupScreenClock ();
  void eraseHand(Point point);
  void printDateTime(DateTime *dateTime);  
  /*******************************************************/
  /*Init Routine                                         */
  /*******************************************************/ 
  void setup() {
    int bootProgress = 0;
    int totalBootEvents = 4;
    int bootInterval = 100 / totalBootEvents;
    //Setup Serial for debug
    if(DEBUG){
      Serial.begin(115200);
      Serial.println("Garden Shed Conditions Server");
      Serial.println("ChipID:" + String(chipId));
    }
    
    //Initialize the DHT Sensor Object and TFT Object
    dht.begin();
    tft.begin();
    //ts.begin();
    
    //Prepare the interface for use.    
    setupScreenBoot();
      
    //Connect to WiFi
    drawProgress(bootProgress, "Connecting to WiFi");
    setupWiFi();

    //Setup DNS
    drawProgress(bootProgress += bootInterval, "Starting mDNS Server");
    if (!MDNS.begin(localHostname)) {
      if(DEBUG){Serial.println("Error setting up MDNS responder!");}
      while(1) { 
        delay(1000);
      }
    }
    if(DEBUG){Serial.println("mDNS responder started");}

    //Start the TCP Server to notify requester the current conditions.
    drawProgress(bootProgress += bootInterval, "Starting TCP Server");
    server.begin();
    
    MDNS.addService("JSON", "tcp", localTcpPort);
    //We will need to uncomment the next three lines to erase the file system and redownload.
//    bootInterval = 100 / (totalBootEvents+1);
//    drawProgress(bootProgress += bootInterval, "Formatting File System");
//    SPIFFS.format();

    //Download necessary external resources.
    drawProgress(bootProgress += bootInterval, "Downloading Resources");
    SPIFFS.begin();
    downloadResources(bootProgress);

    //Nmotify the user we are complete.
    drawProgress(bootProgress += bootInterval, "Completed");
    yield();

    eraseScreen();
    previousMillis = millis();
    if(isClock && getDateTime()){
        setupScreenClock();
    }else{
      //Prepare the screen layout for conditions      
      setupScreenConditions();  
      //Prime the conditions.
      printConditions();  
    }
  }
  /*******************************************************/
  /*Loop Routine                                         */
  /*******************************************************/ 
  void loop(void) {
    getDateTime();
    
    //Always check for a TCP request
    checkTCP();
    yield();
    
    unsigned long currentMillis = millis();    
    if(currentMillis - previousMillis >= interval){
      previousMillis = currentMillis;
      isClock = !isClock;
      if(isClock && getDateTime()){
        eraseScreen();
        setupScreenClock();
      }else{
        //Set this here becuase we might have gooten here becuase the getDateTime() function is down.
        //Also if clock was down, we don't need to full refresh the screen.
        if(isClock){
          isClock = false;
        }else{
          eraseScreen();
        }          
        setupScreenConditions();
        printConditions();
      }
    }
    
    if(isClock){
      printDateTime(gDateTime);
    }else{
      printConditions();
    }
    yield();
  }

  /*******************************************************/
  /*Enable and Connection to WiF                         */
  /*******************************************************/ 
  void setupWiFi(){
    if(DEBUG){Serial.println("Setting Up WiFi");}
    //Need to set hostname prior to DHCP connection.
    
    WiFi.hostname(localHostname);
    if(DEBUG){Serial.println("Hostname: " + String(localHostname));}
    //Connect to Access Point
    if(DEBUG){Serial.print("Connecting to " + String(ssid));}
    WiFi.begin(ssid, password);
    while(WiFi.status() != WL_CONNECTED){
      if(DEBUG){Serial.print(".");}
      delay(500);
    } 
    if(DEBUG){Serial.println("");Serial.println("Connected: " + String(WiFi.localIP()));}   
  }
  
  void printDateTime(DateTime dateTime){
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
        curStrDate = dateTime.strDate;
        tft.fillRect(0,0, SCREEN_WIDTH, timeFontHeight, ILI9341_BLACK);
        ui.drawString(SCREEN_WIDTH/2, timeFontHeight+10, dateTime.strDate);      
      }
  
      if(dateTime.hours12 != curHour || dateTime.minutes != curMin){
        tft.fillRect(0, SCREEN_HEIGHT-timeFontHeight, SCREEN_WIDTH, timeFontHeight, ILI9341_BLACK);
        ui.drawString(SCREEN_WIDTH/2, SCREEN_HEIGHT-15, dateTime.strTime);  
      }
      
      if(dateTime.hours12 != curHour){
        curHour = dateTime.hours12;
        drawHour(dateTime.hours, dateTime.minutes);
      }
  
      if(dateTime.minutes != curMin){
        curMin = dateTime.minutes;
        drawMinute(dateTime.minutes, dateTime.seconds);
      }
  
      //This got a little crazy
      drawSecond(dateTime.seconds);
      
      tft.fillCircle(xCenter, yCenter, 4, GFX_ORANGE);
    }
  }
  /*******************************************************/
  /* Get Current Local time from remote server           */
  /*******************************************************/
  bool getDateTime(){      
    WiFiClient timeClient;
    String JSON = "";
    
    if(!timeClient.connect("littlerichele.com", 3000)){
      Serial.println("Unable to access time server");
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
      Serial.println("Unable to read time data");
      return false;
    }

    //Prase the JSON and display the time on the clock
    StaticJsonBuffer<300> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(JSON);
    if(!root.success()){ 
      Serial.println("Uable to extract time data");
      return false;
    }

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
  
  /*******************************************************/
  /*Helpers for drawing / erasing the hands on the clock */
  /*******************************************************/  
  void drawHour(int hours, int minutes){
    //Draw Hour Hand - 30° per hour 0.5° per minute.
    float hFraction = (float)(3.0/5.0);
    float hRads = (((hours * 30.0) + (float)(minutes * 0.5))*71)/4068.0;
    float xHour = xCenter + radius * sin(hRads);
    float yHour = yCenter - radius * cos(hRads);
    xHour = (xCenter + (hFraction * (xHour - xCenter)));
    yHour = (yCenter + (hFraction * (yHour - yCenter)));
    eraseHand(gDateTime.curHourHand);    
    tft.drawLine(xCenter, yCenter, xHour, yHour, GFX_ORANGE);
    gDateTime.curHourHand = {xHour, yHour};
  }

  void drawMinute(int minutes, int seconds){
    //Draw Minute Hand - 6° per Mins
    float mFraction = (float)(5.0/6.0);
    float mRads = (float)((minutes * 6.0) * 71.0)/4068.0;
    float xMin = xCenter + radius * sin(mRads);
    float yMin = yCenter - radius * cos(mRads);
    xMin = (xCenter + (mFraction * (xMin - xCenter)));
    yMin = (yCenter + (mFraction * (yMin - yCenter)));
    eraseHand(gDateTime.curMinHand);
    drawHour(gDateTime.hours, gDateTime.minutes);
    tft.drawLine(xCenter, yCenter, xMin, yMin, ILI9341_CYAN);  
    gDateTime.curMinHand = {xMin, yMin};
  }

  void drawSecond(int seconds){
    //Draw Second Hand
    float sRads = (float)((seconds * 6.0) * 71.0)/4068.0;
    float xSec = xCenter + radius * sin(sRads);
    float ySec = yCenter - radius * cos(sRads);
    eraseHand(gDateTime.curSecHand);


    float dxc = gDateTime.curMinHand.x - xCenter;
    float dyc = gDateTime.curMinHand.y - yCenter;

    float dxl = xSec - xCenter;
    float dyl = ySec - yCenter;

    if(dxc * dyl - dyc * dxl == 0){
      drawMinute(gDateTime.minutes, gDateTime.seconds); 
    }

    dxc = gDateTime.curHourHand.x - xCenter;
    dyc = gDateTime.curHourHand.y - yCenter;
        
    if(dxc * dyl - dyc * dxl == 0){
     drawHour(gDateTime.hours, gDateTime.minutes);     
    }
    tft.drawLine(xCenter, yCenter, xSec, ySec, ILI9341_RED);  
    gDateTime.curSecHand = {xSec, ySec};
  }

  void eraseHand(Point point){
    tft.drawLine(xCenter, yCenter, point.x, point.y, ILI9341_BLACK);
    drawClockFace();
  }
  
  /*******************************************************/
  /*Check for a TCP request & attempt to respond         */
  /*******************************************************/ 
  void checkTCP(){
      WiFiClient client = server.available();
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
            data = "{\"status\" : " + String(1) + ",\"conditions\" : {\"temp\" : \"" + dht.readTemperature(true) + "\", \"humid\" : \"" + dht.readHumidity() + "\"}}";;
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

  /*******************************************************/
  /*Print conditions from the DHT22 Sensor to the screen */
  /*******************************************************/ 
  void printConditions(){
    //Only do a screen refresh when necessary
    float temp = dht.readTemperature(true);
    if(temp != currentTemp){
      currentTemp = temp;
      //Erase existing Temp and Write New Temp. -16 on y0 and +22 on the height becuase '°' has a y-Axis offset of -60.
      tft.fillRect(textOffset, middleOfTop - halfFontHeight - 16, textBoxWidth, (fontHeight+22), ILI9341_BLACK);     
      ui.drawString(textOffset, middleOfTop + halfFontHeight, String(temp) + "°");           
    }

    //Only do a screen refresh when necessary
    float humid = dht.readHumidity();
    if(humid != currentHumid){
      currentHumid = humid;
      //Erase existing Humidity and Write New Humidity. +2 on font height to covet offset of the '%' sign
      tft.fillRect(textOffset, middleOfBottom - halfFontHeight, textBoxWidth, (fontHeight+2), ILI9341_BLACK);     
      ui.drawString(textOffset, middleOfBottom + halfFontHeight, String(humid) + "%");           
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
    tft.fillScreen(ILI9341_BLACK);
  }
  
  /*******************************************************/
  /*Prepare the screen for boot sequence                 */
  /*******************************************************/
  void setupScreenBoot(){
    eraseScreen();
    tft.setFont(&Liberation_Sans_16);
    tft.setRotation(3);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    //tft.setTextSize(2);
  }

  /*******************************************************/
  /* Prepare the screen for displaying the condition     */
  /*******************************************************/
  void setupScreenConditions(){    
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
  }

  /*******************************************************/
  /* Prepare the screen for displaying the Date/Time     */
  /*******************************************************/
  void setupScreenClock(){
    curStrDate = "NA";
    curHour = -1;
    curMin = -1;
    ui.setTextAlignment(CENTER);
    
    if(is7Segment){
      tft.setFont(&digital_740pt8b );
      tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
    }else{
      tft.setFont(&URWGothicLDegree30pt8b );
      tft.setTextColor(GFX_ORANGE, ILI9341_BLACK);
      drawClockFace();          
    }
  }

  void drawClockFace(){
    //Draw the clock perimiter.
    tft.drawCircle(xCenter, yCenter, radius, GFX_ORANGE);

    //Draw the hour dashes
    for(int i = 0; i < 12; i++){
        float theta = i*2*M_PI/12;
        float xOuter = xCenter + radius * sin(theta);
        float yOuter = yCenter - radius * cos(theta);
        float fraction = 1.0;

        //Make 0/12, 3, 6, 9 longer dashed
        if(i % 3 == 0){
          fraction = (float)(1.0/4.0);
        }else{
          fraction = (float)(1.0/6.0);
        }
        
        float xEnd = (xOuter + (fraction * (xCenter-xOuter)));
        float yEnd = (yOuter + (fraction * (yCenter-yOuter)));
        tft.drawLine(xOuter, yOuter, xEnd, yEnd, GFX_ORANGE);  
    }
  }

  //Callback function to report download progress.
  void downloadCallback(String filename, int16_t bytesDownloaded, int16_t bytesTotal) {
    Serial.println(String(bytesDownloaded) + " / " + String(bytesTotal));
  
    int percentage = 100 * bytesDownloaded / bytesTotal;
    if (percentage % 5 == 0) {
      ui.drawProgressBar(10, 140, 300, 15, percentage, ILI9341_WHITE, ILI9341_GREEN);
    }
  }

  //If the bitmaps don't exist, we need to pull them from the web.
  void downloadResources(uint8_t percentage) {
    char id[5];
    for (int i = 0; i < 21; i++) {
      //sprintf(id, "%02d", i);
      //tft.fillRect(0, 120, 240, 40, ILI9341_BLACK);
      webResource.downloadFile("http://littlerichele.com:3000/images/temperature-mini-crop.bmp", "temperature.bmp", _downloadCallback);
    }
    for (int i = 0; i < 21; i++) {
      //sprintf(id, "%02d", i);
      //tft.fillRect(0, 120, 240, 40, ILI9341_BLACK);
      webResource.downloadFile("http://littlerichele.com:3000/images/humidity-mini-crop.bmp", "humidity.bmp", _downloadCallback);
    }   
  }
  
