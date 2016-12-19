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
  #ifdef ESP8266
  extern "C" {
  #include "user_interface.h"
  }
  #endif  
  /*******************************************************/
  /*Fonts                                                */
  /*******************************************************/  
  #include "Liberation_Sqns_16.h"
  #include "URWGothiLDegree_30.h"
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
  /*DHT Sensor                                           */
  /*******************************************************/
  #define DHTPIN  D3
  #define DHTTYPE DHT22
  DHT dht(DHTPIN, DHTTYPE);
  /*******************************************************/
  /*WiFI                                                 */
  /*******************************************************/
  const char* ssid = "TimeCapsule";
  const char* password = "df6g87df";
  const char* localHostname = "GardenShedConditions";
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
  //Refresh the screen every 30 seconds.
  const long interval = 10000;
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
  /*Function Prototypes                                  */
  /*******************************************************/
  void setupWiFi();
  void checkTCP();
  void printConditions();
  void drawProgress(uint8_t percentage, String text);
  void setupScreenBoot();
  void setupScreenConditions();  
  void downloadResources(uint8_t percentage);  
  void downloadCallback(String filename, int16_t bytesDownloaded, int16_t bytesTotal);
  ProgressCallback _downloadCallback = downloadCallback;
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
    //bootInterval = 100 / (totalBootEvents+1)
    //drawProgress(bootProgress += bootInterval, "Formatting File System");
    //SPIFFS.format();

    //Download necessary external resources.
    drawProgress(bootProgress += bootInterval, "Downloading Resources");
    SPIFFS.begin();
    downloadResources(bootProgress);

    //Nmotify the user we are complete.
    drawProgress(bootProgress += bootInterval, "Completed");
    yield();

    //Prepare the screen layout for conditions
    setupScreenConditions();  
    //Prime the conditions.
    printConditions();  
  }
  
  /*******************************************************/
  /*Loop Routine                                         */
  /*******************************************************/ 
  void loop(void) {    
    //Always check for a TCP request
    yield();
    checkTCP();
    yield(); 
    //Build a timmered loop to print the conditions.
    unsigned long currentMillis = millis();    
    if(currentMillis - previousMillis >= interval){
      previousMillis = currentMillis;
      printConditions();
    }
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

  //Prepare the screen for boot.
  void setupScreenBoot(){
    tft.fillScreen(ILI9341_BLACK);
    tft.setFont(&Liberation_Sans_16);
    tft.setRotation(3);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    //tft.setTextSize(2);
  }

  //Prepare the TFT for displaying the conditions.
  void setupScreenConditions(){
    
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
  
