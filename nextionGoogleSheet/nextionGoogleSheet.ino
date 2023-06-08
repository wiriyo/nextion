/*************************************************************************************************
    Created By: Sompoch Tongnamtiang
    Created On: 6 Mar, 2023
    Update On: 31 May, 2023
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
  2. Download and install Modbus-Master, Simple-Timer
    Hoo ya, have fun....
 *************************************************************************************************/
//===========Your WiFi credentials.============//
// Set password to "" for open networks.
char ssid[] = "Hi-Tech FL2@2.4G";  //ชื่อ Wi-Fi รองรับคลื่น 2.4GHz เท่านั้น
char pass[] = "3014030140";  //รหัสเชื่อมต่อ Wi-Fi
int wifiFlag = 0;
#define wifiLed 2  //D2
unsigned long previousMillis = 0;
unsigned long interval = 30000;
//===========Your WiFi credentials.============//

//==============Senddata2GGSheet===============//
#include <WiFi.h>
#include <HTTPClient.h>
const char *host = "script.google.com";
const char *httpsPort = "443";
String GAS_ID = "AKfycbyHrEODp5a7CnjoTr789zw1Rx9BYnpRqLfF6d0w-czD21Cu9CkMJdRjZhpgp-NiafN2";  //Google Script id from deploy app
//==============Senddata2GGSheet===============//

//=========Nextion Library and Object==========//
#include "Nextion.h"  //Nextion libraries
// Nextion Objects, ada 5 tombol
// (page id, component id, component name)
//Page id 3 DualState Button
NexButton bt0 = NexButton(3, 2, "bt0");
NexButton bt1 = NexButton(3, 3, "bt1");

//Page id 4 Text XY-MD02
NexText textemp1 = NexText(4, 2, "texttemp1");
NexText texhumi1 = NexText(4, 3, "texthumi1");

//Page id 5 Progress BarXY-MD02
NexText textemp2 = NexText(5, 4, "texttemp2");
NexText texhumi2 = NexText(5, 5, "texthumi2");
NexProgressBar pgtemp1 = NexProgressBar(5, 2, "pgtemp1");
NexProgressBar pghumi1 = NexProgressBar(5, 3, "pghumi1");

//Page id 6 Guage XY-MD02
NexGauge guagetemp1 = NexGauge(6, 3, "z0");
NexGauge guagehumi1 = NexGauge(6, 4, "z1");

//Page id 7 Guage LDR
// NexGauge guageldr = NexGauge(7, 5, "guageldr");
// NexText textldr = NexText(7, 6, "textldr");

//Page id 8 Modbus Relay
NexButton bt2 = NexButton(7, 2, "bt2");
NexButton bt3 = NexButton(7, 3, "bt3");
NexButton bt4 = NexButton(7, 4, "bt4");
NexButton bt5 = NexButton(7, 5, "bt5");
// NexButton bt6 = NexButton(7, 6, "bt6");
// NexButton bt7 = NexButton(7, 7, "bt7");
// NexButton bt8 = NexButton(7, 8, "bt8");
// NexButton bt9 = NexButton(7, 9, "bt9");

// Declare variable global
bool statusbt0 = false;
bool statusbt1 = false;
//Modbus Relay
bool statusbt2 = false;
bool statusbt3 = false;
bool statusbt4 = false;
bool statusbt5 = false;
// bool statusbt6 = false;
// bool statusbt7 = false;
// bool statusbt8 = false;
// bool statusbt9 = false;
// Register objects to the touch event list
NexTouch *nex_listen_list[] = {
  &bt0,
  &bt1,
  
  &bt2,
  &bt3,
  &bt4,
  &bt5,
  // &bt6,
  // &bt7,
  // &bt8,
  // &bt9,
  NULL
};
//=========Nextion Library and Object==========//

//==========Define IO connect to relay==========//
#define sw1 18
#define sw2 19
//==========Define IO connect to relay==========//

//=======================LDR====================//
//#define INPUT_ldr 34
//=======================LDR====================//


//=================Modbus-Master================//
#include <ModbusMaster.h>
#define RXD2 16
#define TXD2 17
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
  } else if (statusbt0 == true) {
    digitalWrite(sw1, HIGH);
    statusbt0 = false;
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

// ///=================Modbus-Master================//
// #include <ModbusMaster.h>
// #define RXD2 16
// #define TXD2 17
// // instantiate ModbusMaster object
// ModbusMaster node1;  //Serial2 SlaveID1 XY-MD02
// float humi1 = 0.0f;
// float temp1 = 0.0f;
// // instantiate ModbusMaster object
// ModbusMaster node2;   //Slave ID2 Modbus RTU Relay
// int8_t pool_size1;    //Pool size for Modbus Write command

// //=================Modbus-Master================//

//=================SimpleTimer==================//
// #include <SimpleTimer.h>{
// SimpleTimer timer;
// }
//=================SimpleTimer==================//

// //================ON,OFF Relay1=================//
// void bt0PushCallback(void *ptr) {
//   if (statusbt0 == false) {
//     digitalWrite(sw1, HIGH);
//     statusbt0 = true;
//   } else if (statusbt0 == true) {
//     digitalWrite(sw1, LOW);
//     statusbt0 = false;
//   }
// }
// //================ON,OFF Relay1=================//

// //=================ON,OFF Relay2================//
// void bt1PushCallback(void *ptr) {
//   if (statusbt1 == false) {
//     digitalWrite(sw2, HIGH);
//     statusbt1 = true;
//   } else {
//     digitalWrite(sw2, LOW);
//     statusbt1 = false;
//   }
// }
// //=================ON,OFF Relay2================//


//==========Modbus Relay============//
//==========ON,OFF MRelay1==========//
void bt2PushCallback(void *ptr) {
  if (statusbt2 == false) {
    pool_size1 = node2.writeSingleRegister(0x01,0x0100);
    //relayControl_modbusRTU(2, 1, 1);
    //digitalWrite(sw1, LOW);
    statusbt2 = true;
  } else if (statusbt2 == true) {
     pool_size1 = node2.writeSingleRegister(0x01,0x0200);
    //relayControl_modbusRTU(2, 1, 0);
    //digitalWrite(sw1, HIGH);
    statusbt2 = false;
  }
}
//==========ON,OFF MRelay1==========//

//==========ON,OFF MRelay2==========//
void bt3PushCallback(void *ptr) {
  if (statusbt3 == false) {
    pool_size1 = node2.writeSingleRegister(0x02,0x0100);
    //relayControl_modbusRTU(2, 2, 1);
    //digitalWrite(sw1, LOW);
    statusbt3 = true;
  } else if (statusbt3 == true) {
    pool_size1 = node2.writeSingleRegister(0x02,0x0200);
    //relayControl_modbusRTU(2, 2, 0);
    //digitalWrite(sw1, HIGH);
    statusbt3 = false;
  }
}
//==========ON,OFF MRelay2==========//

//==========ON,OFF MRelay3==========//
void bt4PushCallback(void *ptr) {
  if (statusbt4 == false) {
     pool_size1 = node2.writeSingleRegister(0x03,0x0100);
    //relayControl_modbusRTU(2, 3, 1);
    //digitalWrite(sw1, LOW);
    statusbt4 = true;
  } else if (statusbt4 == true) {
    pool_size1 = node2.writeSingleRegister(0x03,0x0200);
    //relayControl_modbusRTU(2, 3, 0);
    //digitalWrite(sw1, HIGH);
    statusbt4 = false;
  }
}
//==========ON,OFF MRelay3==========//

//==========ON,OFF MRelay4==========//
void bt5PushCallback(void *ptr) {
  if (statusbt5 == false) {
    pool_size1 = node2.writeSingleRegister(0x04,0x0100); 
    //relayControl_modbusRTU(2, 4, 1);
    //digitalWrite(sw1, LOW);
    statusbt5 = true;
  } else if (statusbt5 == true) {
    pool_size1 = node2.writeSingleRegister(0x04,0x0200);
    //relayControl_modbusRTU(2, 4, 0);
    //digitalWrite(sw1, HIGH);
    statusbt5 = false;
  }
}
//==========ON,OFF MRelay4==========//

//==========ON,OFF MRelay5==========//
// void bt6PushCallback(void *ptr) {
//   if (statusbt6 == false) {
//     pool_size1 = node2.writeSingleRegister(0x05,0x0100);
//     //relayControl_modbusRTU(2, 5, 1);
//     //digitalWrite(sw1, LOW);
//     statusbt6 = true;
//   } else if (statusbt6 == true) {
//     pool_size1 = node2.writeSingleRegister(0x05,0x0200);
//     //relayControl_modbusRTU(2, 5, 0);
//     //digitalWrite(sw1, HIGH);
//     statusbt6 = false;
//   }
// }
//==========ON,OFF MRelay5==========//

// //==========ON,OFF MRelay6==========//
// void bt7PushCallback(void *ptr) {
//   if (statusbt7 == false) {
//     pool_size1 = node2.writeSingleRegister(0x06,0x0100);
//     //relayControl_modbusRTU(2, 6, 1);
//     //digitalWrite(sw1, LOW);
//     statusbt7 = true;
//   } else if (statusbt7 == true) {
//     pool_size1 = node2.writeSingleRegister(0x06,0x0200);
//     //relayControl_modbusRTU(2, 6, 0);
//     //digitalWrite(sw1, HIGH);
//     statusbt7 = false;
//   }
// }
// //==========ON,OFF MRelay6==========//

// //==========ON,OFF MRelay7==========//
// void bt8PushCallback(void *ptr) {
//   if (statusbt8 == false) {
//     pool_size1 = node2.writeSingleRegister(0x07,0x0100);
//     //relayControl_modbusRTU(2, 7, 1);
//     //digitalWrite(sw1, LOW);
//     statusbt8 = true;
//   } else if (statusbt8 == true) {
//     pool_size1 = node2.writeSingleRegister(0x07,0x0200);
//     //relayControl_modbusRTU(2, 7, 0);
//     //digitalWrite(sw1, HIGH);
//     statusbt8 = false;
//   }
// }
// //==========ON,OFF MRelay7==========//

// //==========ON,OFF MRelay8==========//
// void bt9PushCallback(void *ptr) {
//   if (statusbt9 == false) {
//     pool_size1 = node2.writeSingleRegister(0x08,0x0100);
//     //relayControl_modbusRTU(2, 8, 1);
//     //digitalWrite(sw1, LOW);
//     statusbt9 = true;
//   } else if (statusbt9 == true) {
//     pool_size1 = node2.writeSingleRegister(0x08,0x0200);
//     //relayControl_modbusRTU(2, 8, 0);
//     //digitalWrite(sw1, HIGH);
//     statusbt9 = false;
//   }
// }
//==========ON,OFF MRelay8==========//
//==========Modbus Relay============//

//==============Wi-Fi Connection================//
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ..");
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    //delay(1000);
  }
  Serial.println(WiFi.localIP());
  digitalWrite(wifiLed, HIGH);
}
//==============Wi-Fi Connection================//

//=================Setup Function===============//
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
  // bt6.attachPush(bt6PushCallback, &bt6);
  // bt7.attachPush(bt7PushCallback, &bt7);
  // bt8.attachPush(bt8PushCallback, &bt8);
  // bt9.attachPush(bt9PushCallback, &bt9);

  // Set IO pinMode for relay
  pinMode(sw1, OUTPUT);
  pinMode(sw2, OUTPUT);
  pinMode(wifiLed, OUTPUT);
  //Set default relay status at boot
  digitalWrite(sw1, HIGH);
  digitalWrite(sw2, HIGH);
  digitalWrite(wifiLed, LOW);

  initWiFi();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  timer.setInterval(20000L, xymdtoNex);          //อ่านค่าเซ็นเซอร์และส่งไปยังจอ Nextion ทุกๆ 5 วินาที
  // timer.setInterval(5000L, sendldrtoNex);       //อ่านค่าเซ็นเซอร์และส่งไปยังจอ Nextion ทุกๆ 5 วินาที
  timer.setInterval(300000L, sendData2GGSheet);  //ส่งค่าเซ็นเซอร์ขึ้น google sheet ทุกๆ 10 วินาที
}
//=================Setup Function===============//

//==============XY-MD02 to Nextion==============//
void xymdtoNex() {
  uint8_t result1;
  float temp1 = (node1.getResponseBuffer(0) / 10.0f);
  float humi1 = (node1.getResponseBuffer(1) / 10.0f);

  Serial.println("Get XY-MD02 Data:");
  result1 = node1.readInputRegisters(0x0001, 2);  //Function 04, Read 2 registers starting at 2)
  if (result1 == node1.ku8MBSuccess) {
    Serial.print("Temp1: ");
    Serial.println(node1.getResponseBuffer(0) / 10.0f);
    Serial.print("Humi1: ");
    Serial.println(node1.getResponseBuffer(1) / 10.0f);
  }

  //Page 3 Text XY-MD02
  String command = "texttemp1.txt=\"" + String(temp1) + "\"";
  Serial.print(command);
  endNextionCommand();

  String command1 = "texttemp1.txt=\"" + String(temp1) + "\"";
  Serial.print(command1);
  endNextionCommand();

  String command2 = "texthumi1.txt=\"" + String(humi1) + "\"";
  Serial.print(command2);
  endNextionCommand();

  //Page 4 Progress Bar XY-MD02
  String command3 = "texttemp2.txt=\"" + String(temp1) + "\"";
  Serial.print(command3);
  endNextionCommand();

  String command4 = "texttemp2.txt=\"" + String(temp1) + "\"";
  Serial.print(command4);
  endNextionCommand();

  String command5 = "texthumi2.txt=\"" + String(humi1) + "\"";
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
  int val1 = map(temp1, 0, 100, 0, 180);
  Serial.print("z0.val=");  //Send the object tag
  Serial.print(val1);               //Send the value
  endNextionCommand();

  int val2 = map(humi1, 0, 100, 0, 180);
  Serial.print("z1.val=");  //Send the object tag
  Serial.print(val2);               //Send the value
  endNextionCommand();
}
//==============XY-MD02 to Nextion==============//

// //=================LDRtoNextion=================//
// void sendldrtoNex() {
//   float ldr_percentage1;
//   int sensor_analog1;
//   sensor_analog1 = analogRead(INPUT_ldr);
//   ldr_percentage1 = (100 - ((sensor_analog1 / 4095.00) * 100));

//   Serial.print("LDR Percentage 1 = ");
//   Serial.print(ldr_percentage1);
//   Serial.print("%\n\n");

//   //Page 6 Guage LDR
//   String command3 = "textldr.txt=\"" + String(ldr_percentage1) + "\"";
//   Serial.print(command3);
//   endNextionCommand();

//   String command4 = "textldr.txt=\"" + String(ldr_percentage1) + "\"";
//   Serial.print(command4);
//   endNextionCommand();

//   int val = map(ldr_percentage1, 0, 100, 0, 222);
//   Serial.print("guageldr.val=");  //Send the object tag
//   Serial.print(val);              //Send the value
//   endNextionCommand();
// }
// //=================LDRtoNextion=================//

//=============SendData2GGSheet=================//
void sendData2GGSheet() {
  //XY-MD02
  uint8_t result1;
  float temp1 = (node1.getResponseBuffer(0) / 10.0f);
  float humi1 = (node1.getResponseBuffer(1) / 10.0f);

 // delay(200);

  HTTPClient http;
  String url = "https://script.google.com/macros/s/" + GAS_ID + "/exec?temp1=" + temp1 + "&humi1=" + humi1; 
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

//===============endNextionCommand==============//
void endNextionCommand() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}
//===============endNextionCommand==============//

//==================Loop Function===============//
void loop() {
  //When push/pop event occured execute component in touch event list
  nexLoop(nex_listen_list);
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
//==================Loop Function===============//