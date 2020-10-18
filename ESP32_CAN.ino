#include <ESP32CAN.h>
#include <CAN_config.h>
#include <WiFi.h>
#include <WebServer.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

#include "webpage.h" //Our HTML webpage contents with javascripts
#include "icons.h"
#include "displaysettings.h"

#define BUILTIN_LED 2
#define MAXCANMESSAGES 50
#define OLED_SDA 21
#define OLED_SCL 22

// Update the below parameters for your project
const char* ssid = "xxx";
const char* password = "yyy";

WiFiClient espClient;
WebServer server(80);                         // Web server
//WiFiServer tcpserver(8040);                   // TCP socket server
Adafruit_SH1106 display(OLED_SDA, OLED_SCL);  // Display setup

// tx_frame is a global variable to store the message to be sent. It is also getting sent 200ms again with off command
// this assumes that no message is sent within 200ms of each other
CAN_frame_t tx_frame;  
String webStat = "";
String webMsg = "";
CAN_frame_t framearray[MAXCANMESSAGES];
String stringarray[MAXCANMESSAGES];
int arraycount = 0;

/* the variable name CAN_cfg is fixed, do not change */
CAN_device_t CAN_cfg;
unsigned int lastchange = 0;
int switchstate = 0;
bool sendupdate = false;
bool autosend = false;
bool statePump, stateRelay2, stateRelay3, stateRelay4;
unsigned long lastMinute, uptime, lastSecond, sec, msgcount;

void refreshStats() {
  char temp[5];
  webStat = "RSSI: ";
  webStat += WiFi.RSSI();
  webStat += "<br/>";
  webStat += "Uptime [min]: ";
  webStat += uptime;
  webStat += "<br/>";
  webStat += "Message count: ";
  webStat += msgcount;
  webStat += "<br/>";
  
}

// Stores new message in String format in an array to be published to the web
void updateMessageArray(String newitem) {
  if (arraycount<MAXCANMESSAGES-1) {
    // there are less than 50 messages
    stringarray[arraycount] = newitem;
    arraycount++;
  } else {
    // shift the array one up
    for (int i=0; i<arraycount-1; i++) {
      stringarray[i] = stringarray[i+1];
    }
    // add the new message to the end
    stringarray[arraycount-1] = newitem;
  }  
  refreshMsg();
}

// Converts the CAN message to string to be displayed on the console and also on the Web
String frameToString(CAN_frame_t rx_frame) {
  String response;
  char temp[200];

  sprintf(temp,"ID 0x%08x, DLC %d | ",rx_frame.MsgID,  rx_frame.FIR.B.DLC);
  response = temp;
  for(int i = 0; i < sizeof(rx_frame.data.u8); i++) {
    sprintf(temp," %02x",rx_frame.data.u8[i]);
    response += temp;
  }
  return response;
}

// Converts the array containing the CAN messages for the HTML format
void refreshMsg() {
  webMsg = "";
  for (int i=0; i<arraycount; i++) {
      webMsg += stringarray[i];
      webMsg += "<br/>";   
  }
}

void handlePointRequest() {
  String message = "";
  int type;
  int address;
  
  if (server.arg("type")== "") {    
    message = "Point type is missing";
  }
  if (server.arg("address")== "") {    
    message = "Point address is missing";
  }
  if (server.arg("state")== "") {    
    message = "Point state is missing";
  }
  if (message=="") {
    if (server.arg("type")== "mm") {
      type = 0x30;
    }
    if (server.arg("type")== "dcc") {
      type = 0x38;
    }
    message = server.arg("address");
    address = message.toInt();
    address--;
    if (server.arg("state")== "0") {
      switchstate = 0x00;
    }    
    if (server.arg("state")== "1") {
      switchstate = 0x01;
    }    
        
    tx_frame.FIR.B.FF = CAN_frame_ext;
    tx_frame.MsgID = 0x0016775e;
    tx_frame.FIR.B.DLC = 6;
    tx_frame.data.u8[0] = 0x00;
    tx_frame.data.u8[1] = 0x00;
    tx_frame.data.u8[2] = type;
    tx_frame.data.u8[3] = address;
    tx_frame.data.u8[4] = switchstate;
    tx_frame.data.u8[5] = 0x01;
    tx_frame.data.u8[6] = 0x00;
    tx_frame.data.u8[7] = 0x00;
    ESP32Can.CANWriteFrame(&tx_frame);
  
    String msgstr = frameToString(tx_frame);
    msgstr = "Send - " + msgstr;
    updateMessageArray(msgstr);
    msgcount ++;
    Serial.println(msgstr);
  
    switchstate = (switchstate == 0) ? 1 : 0;
    lastchange = millis();
    sendupdate = true;  
    message = "Point " + server.arg("address") + " changed to " + server.arg("state");
  }
  // Send dummy response
  server.send(200, "text/html", message);
}

void handleClearLogRequest() {

  arraycount = 0;
  refreshMsg();
  server.send(200, "text/html", "Log cleared");
}

void handleTestSend() {
  tx_frame.FIR.B.FF = CAN_frame_ext;
  tx_frame.MsgID = 0x0016775e;
  tx_frame.FIR.B.DLC = 6;
  tx_frame.data.u8[0] = 0x00;
  tx_frame.data.u8[1] = 0x00;
  tx_frame.data.u8[2] = 0x38;
  tx_frame.data.u8[3] = 0x00;
  tx_frame.data.u8[4] = switchstate;
  tx_frame.data.u8[5] = 0x01;
  tx_frame.data.u8[6] = 0x00;
  tx_frame.data.u8[7] = 0x00;
  ESP32Can.CANWriteFrame(&tx_frame);

  String msgstr = frameToString(tx_frame);
  msgstr = "Send - " + msgstr;
  updateMessageArray(msgstr);
  msgcount ++;
  Serial.println(msgstr);

  switchstate = (switchstate == 0) ? 1 : 0;
  lastchange = millis();
  sendupdate = true;  
  // Send dummy response
  server.send(200, "text/html", "OK");
}

void handleAccessoryOffMessage() {

  // This method assumes that the original message is still in tx_frame
  tx_frame.data.u8[5] = 0x00;
  ESP32Can.CANWriteFrame(&tx_frame);

  String msgstr = frameToString(tx_frame);
  msgstr = "Send - " + msgstr;
  updateMessageArray(msgstr);
  msgcount ++;
  Serial.println(msgstr);
}

void displayWifiLogo(bool onoff) {
  if (onoff) {
    display.drawBitmap(OLED_WIFI_X, OLED_WIFI_Y, icon_wifi, 16, 16, WHITE);
  } else {
    display.drawBitmap(OLED_WIFI_X, OLED_WIFI_Y, icon_blank, 16, 16, BLACK);
  }
  display.display();
}

void displayCanAnimation(int i) {
  display.drawBitmap(OLED_CANANI_X, OLED_CANANI_Y, icon_blank, 16, 16, BLACK);
  switch (i) {
    case 0: 
      display.drawBitmap(OLED_CANANI_X, OLED_CANANI_Y, icon_ani1, 16, 16, WHITE);
      break;
    case 1: 
      display.drawBitmap(OLED_CANANI_X, OLED_CANANI_Y, icon_ani2, 16, 16, WHITE);
      break;
    case 2: 
      display.drawBitmap(OLED_CANANI_X, OLED_CANANI_Y, icon_ani3, 16, 16, WHITE);
      break;
    case 3: 
      display.drawBitmap(OLED_CANANI_X, OLED_CANANI_Y, icon_ani4, 16, 16, WHITE);
      break;
    case 4: 
      display.drawBitmap(OLED_CANANI_X, OLED_CANANI_Y, icon_ani5, 16, 16, WHITE);
      break;
    case 5: 
      display.drawBitmap(OLED_CANANI_X, OLED_CANANI_Y, icon_ani6, 16, 16, WHITE);
      break;
    case 6: 
      display.drawBitmap(OLED_CANANI_X, OLED_CANANI_Y, icon_ani7, 16, 16, WHITE);
      break;
    case 7: 
      display.drawBitmap(OLED_CANANI_X, OLED_CANANI_Y, icon_ani8, 16, 16, WHITE);
      break;
  }
  display.display();
}

void displayIPAddress() {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(OLED_IP_X,OLED_IP_Y);
  display.println(WiFi.localIP());
  display.display();
}

void displayRSSI() {
  display.fillRect(17, 0, 20, 16, BLACK);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(OLED_RSSI_X,OLED_RSSI_Y);
  display.println(WiFi.RSSI());
  display.display();  
}

void configSaved()
{
  Serial.println("Configuration was updated.");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Marklin CAN-Wifi Gateway");
  
  // initialize with the I2C addr 0x3C
  display.begin(SH1106_SWITCHCAPVCC, 0x3C); 
  display.clearDisplay();  
  display.drawBitmap(-2, 0, icon_background, 128, 64, WHITE);
  display.display();

  pinMode(BUILTIN_LED, OUTPUT);

  uptime = 0;
  msgcount = 0;

  delay(100);
  Serial.println();
  Serial.print("Connecting to wifi network");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    displayWifiLogo(uptime % 2 == 0);
    digitalWrite(BUILTIN_LED, uptime % 2 == 0);
    uptime++;
  }
  uptime = 0;
  displayWifiLogo(true);
  digitalWrite(BUILTIN_LED, LOW);

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  displayIPAddress();
  Serial.print("Signal [RSSI]: ");
  Serial.println(WiFi.RSSI());
  displayRSSI();

  //if (mdns.begin("poolheater", WiFi.localIP())) {
  //  Serial.println("MDNS responder started");
  //}  
  
  server.on("/", [](){                        // main page
     String s = webPage; //Read HTML contents
     server.send(200, "text/html", s); //Send web page
  });
  server.on("/getlog", [](){                  // Log callback
    server.send(200, "text/html", webMsg+"#"+webStat);
  });
  server.on("/testsend", handleTestSend);
  server.on("/point", handlePointRequest);              
  server.on("/clearlog", handleClearLogRequest);              
  //server.on("/relay", handleRelayCommand);  
   
  server.begin();
  Serial.println("HTTP server started");

  Serial.println("Setting up CAN...");
  /* set CAN pins and baudrate */
  CAN_cfg.speed=CAN_SPEED_250KBPS;
  CAN_cfg.tx_pin_id = GPIO_NUM_5;
  CAN_cfg.rx_pin_id = GPIO_NUM_4;
  /* create a queue for CAN receiving */
  CAN_cfg.rx_queue = xQueueCreate(10,sizeof(CAN_frame_t));
  //initialize CAN Module
  ESP32Can.CANInit();
  Serial.println("CAN initialized");
  refreshStats();

  
  //tcpserver.begin();
  Serial.println("TCP server started");
  
}

void loop() {
  CAN_frame_t rx_frame;


  // This sends out the off message after 200ms
  if ((sendupdate)&&(millis()-lastchange>200)) {
    handleAccessoryOffMessage();
    sendupdate = false;
  }
   
    
  //receive next CAN frame from queue
  if(xQueueReceive(CAN_cfg.rx_queue,&rx_frame, 3*portTICK_PERIOD_MS)==pdTRUE){
    digitalWrite(BUILTIN_LED, HIGH);
    String msgstr = frameToString(rx_frame);
    msgstr = "Recv - " + msgstr;
    updateMessageArray(msgstr);
    msgcount ++;
    Serial.println(msgstr);
    displayCanAnimation(msgcount % 8);
    digitalWrite(BUILTIN_LED, LOW);
  }

 
  // Handle HTTP server requests
  server.handleClient();

  // Uptime calculation (every minute)
  if (millis() - lastMinute >= 60000) {            
    lastMinute = millis();            
    uptime++;            
  }   

  // second calculation
  if (millis() - lastSecond >= 1000) {            
    lastSecond = millis();            
    sec++;   
    if (sec%10==0) {
      refreshStats();  
      displayRSSI();       
    }
  }   
/*
  // listen for client 
  WiFiClient client = tcpserver.available(); 
  uint8_t data[30]; 
  if (client) {                   
    Serial.println("new client");         
    // check client is connected        
    while (client.connected()) {          
        if (client.available()) {
            int len = client.read(data, 30);
            if(len < 30){
                data[len] = '\0';  
            }else {
                data[30] = '\0';
            }    
            Serial.print("client sent: ");            
            Serial.println((char *)data); 
        }
    } 
  }
*/
    
}
