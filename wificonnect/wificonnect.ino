#include <WiFi.h>

// char ssid[] = "Hi-Tech FL2@2.4G"; //ชื่อ SSID ที่เราต้องการเชื่อมต่อ        
// char pass[] = "3014030140"; //รหัสผ่าน WI-FI

// char ssid[] = "phonpandao_laptop"; //ชื่อ SSID ที่เราต้องการเชื่อมต่อ        
// char pass[] = "musiccoff"; //รหัสผ่าน WI-FI

char ssid[] = "phonpandao"; //ชื่อ SSID ที่เราต้องการเชื่อมต่อ        
char pass[] = "musiccoff"; //รหัสผ่าน WI-FI

#define Led_wifi 2
void setup()
{
   Serial.begin(9600);
   Serial.println("Starting...");
   WiFi.begin(ssid,pass);
   pinMode(Led_wifi,OUTPUT);
   digitalWrite(Led_wifi,LOW);
   while (WiFi.status() != WL_CONNECTED)
   {
    delay(250);
    Serial.print(".");
   }
  Serial.println("WiFi Connected");
  digitalWrite(Led_wifi,HIGH);
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
 
 }

void loop()
{
 //run program here 
}