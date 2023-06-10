/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 6 Mar, 2023
    Update On: 8 June, 2023
    Facebook : https://www.facebook.com/smfthailand
    YouTube Channel : https://www.youtube.com/bugkuska
 *  *********************************************************************************************
    Preferences--> Aditional boards Manager URLs :
    For ESP32 (2.0.3):
    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *  *********************************************************************************************
    Install the following libraries :
    1.Download and install ITEADLIB_Arduino_Nextion library:
    https://github.com/itead/ITEADLIB_Arduino_Nextion
  *  *********************************************************************************************
    Wiring Nextion to ESP32
    5V >> 5V
    GND >> GND
    RX >> TX0
    TX >> RX0, ถ้าแก้ไขไฟล์ NexConfig.h ใน Library ITEADLIB_Arduino_Nextion สามารถเปลี่ยนมาใช้ RX0 ได้
  *Note : ระหว่าง upload source-code ให้ถอดสาย RX2 ออกจากบอร์ด ESP32 พอ upload source-code ค่อยต่อเข้าไปใหม่
  2. Download and install Modbus-Master, Blynk and Simple-Timer
    Hoo ya, have fun....
 *************************************************************************************************/
/* Fill-in your Template ID (only if using Blynk.Cloud) */
#define BLYNK_TEMPLATE_ID "TMPL6WfmTVWA1"
#define BLYNK_TEMPLATE_NAME "NextionDsd5"
#define BLYNK_AUTH_TOKEN "TRMU82p0K9R2_76YxWKEtXGAYF65wfLg"
#define BLYNK_FIRMWARE_VERSION "0.1.0"
bool fetch_blynk_state = true;  //true or false
//#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
char auth[] = BLYNK_AUTH_TOKEN;
//================Wi-Fi Manager================//
#include <FS.h>  //this needs to be first, or it all crashes and burns...
#include <SPIFFS.h>
//needed for library
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>  //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>  //Ver 5.13.4   //https://github.com/bblanchon/ArduinoJson
//Callback notifying us of the need to save config
bool shouldSaveConfig = false;
void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
//================Wi-Fi Manager================//

//==============Check Wi-Fi State==============//
int wifiFlag = 0;
#define wifiLed 2  //D2
unsigned long previousMillis = 0;
unsigned long interval = 30000;
//==============Check Wi-Fi State==============//

//==============Senddata2GGSheet===============//
#include <WiFi.h>
#include <HTTPClient.h>
const char *host = "script.google.com";
const char *httpsPort = "443";
String GAS_ID = "AKfycbyHrEODp5a7CnjoTr789zw1Rx9BYnpRqLfF6d0w-czD21Cu9CkMJdRjZhpgp-NiafN2";  //Google Script id from deploy app
//==============Senddata2GGSheet===============//

//=========Nextion Library and Object==========//
#include "Nextion.h"  //Nextion libraries
// (page id, component id, component name)
//Page id 3 DualState Button
NexButton bt0 = NexButton(3, 2, "bt0");
NexButton bt1 = NexButton(3, 3, "bt1");
//Page id 4 Text XY-MD02
NexText textemp1 = NexText(4, 2, "textemp1");
NexText texhumi1 = NexText(4, 3, "texhumi1");
//Page id 5 Progress BarXY-MD02
NexText textemp2 = NexText(5, 2, "textemp2");
NexText texhumi2 = NexText(5, 3, "texhumi2");
NexProgressBar pgtemp1 = NexProgressBar(5, 4, "pgtemp1");
NexProgressBar pghumi1 = NexProgressBar(5, 5, "pghumi1");
//Page id 6 Guage XY-MD02
NexGauge guagetemp1 = NexGauge(6, 2, "guagetemp1");
NexGauge guagehumi1 = NexGauge(6, 3, "guagehumi1");
//Page id 7 Modbus Relay
NexButton bt2 = NexButton(7, 2, "bt2");
NexButton bt3 = NexButton(7, 3, "bt3");
NexButton bt4 = NexButton(7, 4, "bt4");
NexButton bt5 = NexButton(7, 5, "bt5");
NexButton bt6 = NexButton(7, 6, "bt6");
NexButton bt7 = NexButton(7, 7, "bt7");
NexButton bt8 = NexButton(7, 8, "bt8");
NexButton bt9 = NexButton(7, 9, "bt9");
// Declare variable global
bool statusbt0 = false;
bool statusbt1 = false;
//Modbus Relay
bool statusbt2 = false;
bool statusbt3 = false;
bool statusbt4 = false;
bool statusbt5 = false;
bool statusbt6 = false;
bool statusbt7 = false;
bool statusbt8 = false;
bool statusbt9 = false;
// Register objects to the touch event list
NexTouch *nex_listen_list[] = {
  //2CH Relay
  &bt0,
  &bt1,
  //Modbus Relay
  &bt2,
  &bt3,
  &bt4,
  &bt5,

  NULL
};
//=========Nextion Library and Object==========//
//==========Define IO connect to relay==========//
#define sw1 18
#define sw2 19
//#define sw1 18
//#define sw2 19
//==========Define IO connect to relay==========//
//=================Modbus-Master================//
#include <ModbusMaster.h>
#define RXD2 16  //RXD2
#define TXD2 17  //TXD2
// instantiate ModbusMaster object
ModbusMaster node1;  //Serial2 SlaveID1 XY-MD02
float humi1 = 0.0f;
float temp1 = 0.0f;
// instantiate ModbusMaster object
ModbusMaster node2;  //Slave ID2 Modbus RTU Relay
int8_t pool_size1;   //Pool size for Modbus Write command
//=================Modbus-Master================//
//=================SimpleTimer==================//
#include <SimpleTimer.h>
SimpleTimer timer;
//=================SimpleTimer==================//
//================ON,OFF Relay1=================//
void bt0PushCallback(void *ptr) {
  if (statusbt0 == false) {
    digitalWrite(sw1, LOW);
    statusbt0 = true;
    //Blynk.virtualWrite(V3, 1);
  } else if (statusbt0 == true) {
    digitalWrite(sw1, HIGH);
    statusbt0 = false;
    //Blynk.virtualWrite(V3, 0);
  }
}
//================ON,OFF Relay1=================//
//=================ON,OFF Relay2================//
void bt1PushCallback(void *ptr) {
  if (statusbt1 == false) {
    digitalWrite(sw2, LOW);
    statusbt1 = true;
  } else {
    digitalWrite(sw2, HIGH);
    statusbt1 = false;
  }
}
//=================ON,OFF Relay2================//
//================Modbus Relay==================//
//=================ON,OFF MRelay1===============//
void bt2PushCallback(void *ptr) {
  if (statusbt2 == false) {
    pool_size1 = node2.writeSingleRegister(0x01, 0x0100);
    //relayControl_modbusRTU(2, 1, 1);
    //digitalWrite(sw1, LOW);
    statusbt2 = true;
  } else if (statusbt2 == true) {
    pool_size1 = node2.writeSingleRegister(0x01, 0x0200);
    //relayControl_modbusRTU(2, 1, 0);
    //digitalWrite(sw1, HIGH);
    statusbt2 = false;
  }
}
//=================ON,OFF MRelay1===============//
//=================ON,OFF MRelay2===============//
void bt3PushCallback(void *ptr) {
  if (statusbt3 == false) {
    pool_size1 = node2.writeSingleRegister(0x02, 0x0100);
    //relayControl_modbusRTU(2, 2, 1);
    //digitalWrite(sw1, LOW);
    statusbt3 = true;
  } else if (statusbt3 == true) {
    pool_size1 = node2.writeSingleRegister(0x02, 0x0200);
    //relayControl_modbusRTU(2, 2, 0);
    //digitalWrite(sw1, HIGH);
    statusbt3 = false;
  }
}
//=================ON,OFF MRelay2===============//
//=================ON,OFF MRelay3===============//
void bt4PushCallback(void *ptr) {
  if (statusbt4 == false) {
    pool_size1 = node2.writeSingleRegister(0x03, 0x0100);
    //relayControl_modbusRTU(2, 3, 1);
    //digitalWrite(sw1, LOW);
    statusbt4 = true;
  } else if (statusbt4 == true) {
    pool_size1 = node2.writeSingleRegister(0x03, 0x0200);
    //relayControl_modbusRTU(2, 3, 0);
    //digitalWrite(sw1, HIGH);
    statusbt4 = false;
  }
}
//=================ON,OFF MRelay3===============//
//=================ON,OFF MRelay4===============//
void bt5PushCallback(void *ptr) {
  if (statusbt5 == false) {
    pool_size1 = node2.writeSingleRegister(0x04, 0x0100);
    //relayControl_modbusRTU(2, 4, 1);
    //digitalWrite(sw1, LOW);
    statusbt5 = true;
  } else if (statusbt5 == true) {
    pool_size1 = node2.writeSingleRegister(0x04, 0x0200);
    //relayControl_modbusRTU(2, 4, 0);
    //digitalWrite(sw1, HIGH);
    statusbt5 = false;
  }
}
//=================ON,OFF MRelay4===============//
//=================ON,OFF MRelay5===============//
void bt6PushCallback(void *ptr) {
  if (statusbt6 == false) {
    pool_size1 = node2.writeSingleRegister(0x05, 0x0100);
    //relayControl_modbusRTU(2, 5, 1);
    //digitalWrite(sw1, LOW);
    statusbt6 = true;
  } else if (statusbt6 == true) {
    pool_size1 = node2.writeSingleRegister(0x05, 0x0200);
    //relayControl_modbusRTU(2, 5, 0);
    //digitalWrite(sw1, HIGH);
    statusbt6 = false;
  }
}
//=================ON,OFF MRelay5===============//
//=================ON,OFF MRelay6===============//
void bt7PushCallback(void *ptr) {
  if (statusbt7 == false) {
    pool_size1 = node2.writeSingleRegister(0x06, 0x0100);
    //relayControl_modbusRTU(2, 6, 1);
    //digitalWrite(sw1, LOW);
    statusbt7 = true;
  } else if (statusbt7 == true) {
    pool_size1 = node2.writeSingleRegister(0x06, 0x0200);
    //relayControl_modbusRTU(2, 6, 0);
    //digitalWrite(sw1, HIGH);
    statusbt7 = false;
  }
}
//=================ON,OFF MRelay6===============//
//=================ON,OFF MRelay7===============//
void bt8PushCallback(void *ptr) {
  if (statusbt8 == false) {
    pool_size1 = node2.writeSingleRegister(0x07, 0x0100);
    //relayControl_modbusRTU(2, 7, 1);
    //digitalWrite(sw1, LOW);
    statusbt8 = true;
  } else if (statusbt8 == true) {
    pool_size1 = node2.writeSingleRegister(0x07, 0x0200);
    //relayControl_modbusRTU(2, 7, 0);
    //digitalWrite(sw1, HIGH);
    statusbt8 = false;
  }
}
//=================ON,OFF MRelay7===============//
//=================ON,OFF MRelay8===============//
void bt9PushCallback(void *ptr) {
  if (statusbt9 == false) {
    pool_size1 = node2.writeSingleRegister(0x08, 0x0100);
    //relayControl_modbusRTU(2, 8, 1);
    //digitalWrite(sw1, LOW);
    statusbt9 = true;
  } else if (statusbt9 == true) {
    pool_size1 = node2.writeSingleRegister(0x08, 0x0200);
    //relayControl_modbusRTU(2, 8, 0);
    //digitalWrite(sw1, HIGH);
    statusbt9 = false;
  }
}
//=================ON,OFF MRelay8===============//
//================Modbus Relay==================//
//==============initWi-Fi Manager===============//
void initWiFiManager() {
  //read configuration from FS json
  Serial.println("mounting FS...");  //แสดงข้อความใน Serial Monitor
  if (SPIFFS.begin(true)) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
        } else {
          Serial.println("failed to load json config");  //แสดงข้อความใน Serial Monitor
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");  //แสดงข้อความใน Serial Monitor
  }
  //end read
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  /*
  for (int i = 5; i > -1; i--) {  // นับเวลาถอยหลัง 5 วินาทีก่อนกดปุ่ม AP Config
    //digitalWrite(ledbb, HIGH);
    delay(500);
    //digitalWrite(ledbb, LOW);
    // delay(500);
    Serial.print (String(i) + " ");//แสดงข้อความใน Serial Monitor
   
  }
  */
  /*
  if (digitalRead(AP_Config) == LOW) {
   // Serial.println("Button Pressed");//แสดงข้อความใน Serial Monitor
    // wifiManager.resetSettings();//ให้ล้างค่า SSID และ Password ที่เคยบันทึกไว้
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); //load the flash-saved configs
    esp_wifi_init(&cfg); //initiate and allocate wifi resources (does not matter if connection fails)
    delay(2000); //wait a bit
    if (esp_wifi_restore() != ESP_OK)
    {
      Serial.println("WiFi is not initialized by esp_wifi_init ");
    } else {
      Serial.println("WiFi Configurations Cleared!");
    }
    //continue
    //delay(1000);
    //esp_restart(); //just my reset configs routine...
  }
*/
  wifiManager.setTimeout(120);
  //ใช้ได้ 2 กรณี
  //1. เมื่อกดปุ่มเพื่อ Config ค่า AP แล้ว จะขึ้นชื่อ AP ที่เราตั้งขึ้น
  //   ช่วงนี้ให้เราทำการตั้งค่า SSID+Password หรืออื่นๆทั้งหมด ภายใน 60 วินาที ก่อน AP จะหมดเวลา
  //   ไม่เช่นนั้น เมื่อครบเวลา 60 วินาที MCU จะ Reset เริ่มต้นใหม่ ให้เราตั้งค่าอีกครั้งภายใน 60 วินาที
  //2. ช่วงไฟดับ Modem router + MCU จะดับทั้งคู่ และเมื่อมีไฟมา ทั้งคู่ก็เริ่มทำงานเช่นกัน
  //   โดยปกติ Modem router จะ Boot ช้ากว่า  MCU ทำให้ MCU กลับไปเป็น AP รอให้เราตั้งค่าใหม่
  //   ดังนั้น AP จะรอเวลาให้เราตั้งค่า 60 วินาที ถ้าไม่มีการตั้งค่าใดๆ เมื่อครบ 60 วินาที MCU จะ Reset อีกครั้ง
  //   ถ้า Modem router  Boot และใช้งานได้ภายใน 60 วินาที และหลังจากที่ MCU Resset และเริ่มทำงานใหม่
  //   ก็จะสามารถเชื่อมต่อกับ  Modem router ที่ Boot และใช้งานได้แล้ว  ได้  ระบบจะทำงานปกติ
  if (!wifiManager.autoConnect("Aroon", "password00")) {    //ชื่อ Soft AP ที่เราต้องเชื่อมต่อเพื่อเข้าหน้าตั้งค่าการเชื่อมต่อ Wi-Fi
    Serial.println("failed to connect and hit timeout");  //แสดงข้อความใน Serial Monitor
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();  //แก้ เดิม ESP.reset(); ใน Esp8266
    //delay(5000);
  }
  Serial.println("Connected.......OK!)");  //แสดงข้อความใน Serial Monitor
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");  //แสดงข้อความใน Serial Monitor
    }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
  Serial.println("local ip");  //แสดงข้อความใน Serial Monitor
  delay(100);
  Serial.println(WiFi.localIP());  //แสดงข้อความใน Serial Monitor
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
}
//==============initWi-Fi Manager===============//
//========Check Blynk connected Status==========//
void checkBlynkStatus() {  // called every 10 seconds by SimpleTimer
  bool isconnected = Blynk.connected();
  if (isconnected == false) {
    wifiFlag = 1;
    Serial.println("Blynk Not Connected");
    digitalWrite(wifiLed, LOW);
  }
  if (isconnected == true) {
    wifiFlag = 0;
    if (!fetch_blynk_state) {
      Blynk.syncAll();
    }
    digitalWrite(wifiLed, HIGH);
    Serial.println("Blynk Connected");
  }
}
//========Check Blynk connected Status==========//
//===============Blynk Connected================//
BLYNK_CONNECTED() {
  // Request the latest state from the server
  if (fetch_blynk_state) {
    digitalWrite(wifiLed, HIGH);
    Blynk.syncAll();
  }
}
//===============Blynk Connected================//
//==============Setup function==================//
void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  node1.begin(1, Serial2);  //Slave ID1 XY-MD02
  node2.begin(2, Serial2);
  //Initialize Nextion Library
  nexInit();
  // Register the push/pop event callback function
  //2CH Relay
  bt0.attachPush(bt0PushCallback, &bt0);
  bt1.attachPush(bt1PushCallback, &bt1);
  //Modbus Relay
  bt2.attachPush(bt2PushCallback, &bt2);
  bt3.attachPush(bt3PushCallback, &bt3);
  bt4.attachPush(bt4PushCallback, &bt4);
  bt5.attachPush(bt5PushCallback, &bt5);
  bt6.attachPush(bt6PushCallback, &bt6);
  bt7.attachPush(bt7PushCallback, &bt7);
  bt8.attachPush(bt8PushCallback, &bt8);
  bt9.attachPush(bt9PushCallback, &bt9);
  // Set IO pinMode for relay
  pinMode(sw1, OUTPUT);
  pinMode(sw2, OUTPUT);
  pinMode(wifiLed, OUTPUT);
  //Set default relay status at boot
  digitalWrite(sw1, HIGH);
  digitalWrite(sw2, HIGH);
  digitalWrite(wifiLed, LOW);

  initWiFiManager();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  timer.setInterval(10000L, xymdtoNex);          //อ่านค่าเซ็นเซอร์และส่งไปยังจอ Nextion ทุกๆ 5 วินาที
  timer.setInterval(300000L, sendData2GGSheet);  //ส่งค่าเซ็นเซอร์ขึ้น google sheet ทุกๆ 10 วินาที
  timer.setInterval(30000L, xymdtoBlynk);
  Blynk.config(auth);
  delay(1000);
  if (!fetch_blynk_state) {
    Blynk.virtualWrite(V3, HIGH);
    Blynk.virtualWrite(V4, HIGH);
  }
}
//==============Setup function==================//
//==============XY-MD02 to Nextion==============//
void xymdtoNex() {
  uint8_t result1;
  float temp1 = (node1.getResponseBuffer(0) / 10.0f);
  float humi1 = (node1.getResponseBuffer(1) / 10.0f);
  Serial.println("Get XY-MD02 Data:");
  result1 = node1.readInputRegisters(0x0001, 2);  //Function 04, Read 2 registers starting at 2)
  //result1 = node1.readHoldingRegisters(0x0000, 2); // Read 2 registers starting at 1)
  if (result1 == node1.ku8MBSuccess) {
    Serial.print("Temp1: ");
    Serial.println(node1.getResponseBuffer(0) / 10.0f);
    Serial.print("Humi1: ");
    Serial.println(node1.getResponseBuffer(1) / 10.0f);
  }
  //Page 3 Text XY-MD02
  String command = "textemp1.txt=\"" + String(temp1) + "\"";
  Serial.print(command);
  endNextionCommand();

  String command1 = "textemp1.txt=\"" + String(temp1) + "\"";
  Serial.print(command1);
  endNextionCommand();

  String command2 = "texhumi1.txt=\"" + String(humi1) + "\"";
  Serial.print(command2);
  endNextionCommand();

  //Page 4 Progress Bar XY-MD02
  String command3 = "textemp2.txt=\"" + String(temp1) + "\"";
  Serial.print(command3);
  endNextionCommand();

  String command4 = "textemp2.txt=\"" + String(temp1) + "\"";
  Serial.print(command4);
  endNextionCommand();

  String command5 = "texhumi2.txt=\"" + String(humi1) + "\"";
  Serial.print(command5);
  endNextionCommand();

  pgtemp1.setValue(temp1);
  Serial.print("pgtemp1.val=");
  Serial.print(temp1);
  endNextionCommand();

  pghumi1.setValue(humi1);
  Serial.print("pghumi1.val=");
  Serial.print(humi1);
  endNextionCommand();

  //Page 5 Guage XY-MD02
  int val1 = map(temp1, 0, 100, 0, 223);
  Serial.print("guagetemp1.val=");  //Send the object tag
  Serial.print(val1);               //Send the value
  endNextionCommand();

  int val2 = map(humi1, 0, 100, 0, 223);
  Serial.print("guagehumi1.val=");  //Send the object tag
  Serial.print(val2);               //Send the value
  endNextionCommand();
}
//==============XY-MD02 to Nextion==============//
//=============SendData2GGSheet=================//
void sendData2GGSheet() {
  //XY-MD02
  uint8_t result1;
  float temp1 = (node1.getResponseBuffer(0) / 10.0f);
  float humi1 = (node1.getResponseBuffer(1) / 10.0f);
  delay(1000);
  HTTPClient http;
  String url = "https://script.google.com/macros/s/" + GAS_ID + "/exec?temp1=" + temp1 + "&humi1=" + humi1 + "&lux=";
  //Serial.print(url);
  Serial.println("Posting Temperature and humidity data to Google Sheet");
  //---------------------------------------------------------------------
  //starts posting data to google sheet
  http.begin(url.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  Serial.print("HTTP Status Code: ");
  Serial.println(httpCode);
  //---------------------------------------------------------------------
  //getting response from google sheet
  String payload;
  if (httpCode > 0) {
    payload = http.getString();
    Serial.println("Payload: " + payload);
  }
  //---------------------------------------------------------------------
  http.end();
}
//=============SendData2GGSheet=================//
//==============endNextionCommand===============//
void endNextionCommand() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}
//==============endNextionCommand===============//
//==============XY-MD02 to Blynk 2.0============//
void xymdtoBlynk() {
  uint8_t result1;
  float temp1 = (node1.getResponseBuffer(0) / 10.0f);
  float humi1 = (node1.getResponseBuffer(1) / 10.0f);

  Serial.println("Get XY-MD02 Data:");
  result1 = node1.readInputRegisters(0x0001, 2);  //Function 04, Read 2 registers starting at 2)
  //result1 = node1.readHoldingRegisters(0x0000, 2); // Read 2 registers starting at 1)
  if (result1 == node1.ku8MBSuccess) {
    Serial.print("Temp1: ");
    Serial.println(node1.getResponseBuffer(0) / 10.0f);
    Serial.print("Humi1: ");
    Serial.println(node1.getResponseBuffer(1) / 10.0f);
  }

  delay(1000);
  Blynk.virtualWrite(V1, temp1);
  Blynk.virtualWrite(V2, humi1);
}
//==============XY-MD02 to Blynk 2.0============//
//================Loop function=================//
void loop() {
  //When push/pop event occured execute component in touch event list
  nexLoop(nex_listen_list);
  Blynk.run();
  timer.run();
  //Check Wi-Fi
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }
}
//================Loop function=================//
//======BTN Blynk ON-OFF SW1 Digtal Relay1======//
BLYNK_WRITE(V3) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    digitalWrite(sw1, HIGH);
  } else {
    digitalWrite(sw1, LOW);
  }
}
//======BTN Blynk ON-OFF SW1 Digtal Relay1======//
//======BTN Blynk ON-OFF SW2 Digtal Relay2======//
BLYNK_WRITE(V4) {
  int valuebtn2 = param.asInt();
  if (valuebtn2 == 1) {
    digitalWrite(sw2, HIGH);
  } else {
    digitalWrite(sw2, LOW);
  }
}
//======BTN Blynk ON-OFF SW2 Digtal Relay2======//
//=======BTN Blynk ON-OFF SW3 Mbrelay1==========//
BLYNK_WRITE(V5) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    pool_size1 = node2.writeSingleRegister(0x01, 0x0100);
  } else {
    pool_size1 = node2.writeSingleRegister(0x01, 0x0200);
  }
}
//=======BTN Blynk ON-OFF SW3 Mbrelay1==========//
//========BTN Blynk ON-OFF SW4 Mbrelay2=========//
BLYNK_WRITE(V6) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    pool_size1 = node2.writeSingleRegister(0x02, 0x0100);
  } else {
    pool_size1 = node2.writeSingleRegister(0x02, 0x0200);
  }
}
//========BTN Blynk ON-OFF SW4 Mbrelay2=========//
//========BTN Blynk ON-OFF SW5 Mbrelay3=========//
BLYNK_WRITE(V7) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    pool_size1 = node2.writeSingleRegister(0x03, 0x0100);
  } else {
    pool_size1 = node2.writeSingleRegister(0x03, 0x0200);
  }
}
//========BTN Blynk ON-OFF SW5 Mbrelay3=========//
//========BTN Blynk ON-OFF SW6 Mbrelay4=========//
BLYNK_WRITE(V8) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    pool_size1 = node2.writeSingleRegister(0x04, 0x0100);
  } else {
    pool_size1 = node2.writeSingleRegister(0x04, 0x0200);
  }
}
//========BTN Blynk ON-OFF SW6 Mbrelay4=========//
//========BTN Blynk ON-OFF SW7 Mbrelay5=========//
BLYNK_WRITE(V9) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    pool_size1 = node2.writeSingleRegister(0x05, 0x0100);
  } else {
    pool_size1 = node2.writeSingleRegister(0x05, 0x0200);
  }
}
//========BTN Blynk ON-OFF SW7 Mbrelay5=========//
//========BTN Blynk ON-OFF SW8 Mbrelay6=========//
BLYNK_WRITE(V10) {
  int valuebtn1 = param.asInt();
  if (valuebtn1 == 1) {
    pool_size1 = node2.writeSingleRegister(0x06, 0x0100);
  } else {
    pool_size1 = node2.writeSingleRegister(0x06, 0x0200);
  }
}
//========BTN Blynk ON-OFF SW8 Mbrelay6=========//